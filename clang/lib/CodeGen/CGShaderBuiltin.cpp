//===-- CGShaderBuiltin.cpp - Native shader intrinsics ------------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "CGRecordLayout.h"
#include "CodeGenFunction.h"
#include "clang/AST/SPIRVShader.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/IR/IntrinsicsSPIRV.h"

using namespace clang;
using namespace clang::CodeGen;

RValue CodeGenFunction::EmitShaderBuiltin(unsigned ID, const CallExpr *E,
                                          ReturnValueSlot ReturnValue) {
  llvm::StringRef Name = shaderIntrinsicName(ID);
  auto &C = getContext();
  auto &LC = getLLVMContext();
  auto *Void = llvm::Type::getVoidTy(LC);
  auto I32 = [&](unsigned V) { return Builder.getInt32(V); };
  auto Scalar = [&](unsigned I) { return EmitScalarExpr(E->getArg(I)); };
  auto ResourceHandle = [&](unsigned I) -> llvm::Value * {
    const Expr *Value = E->getArg(I);
    if (!Value->getType()->isRecordType())
      return Scalar(I);
    Address Storage = Value->isLValue()
                          ? EmitLValue(Value).getAddress()
                          : Address(CreateMemTemp(Value->getType()));
    if (!Value->isLValue())
      EmitAnyExprToMem(Value, Storage, Qualifiers(), true);
    return Builder.CreateLoad(Builder.CreateStructGEP(Storage, 0));
  };
  auto Native = [&](llvm::StringRef Op, llvm::Type *Return,
                    ArrayRef<llvm::Value *> Args,
                    bool Convergent = false) -> llvm::Value * {
    // Use a valid Itanium template name to distinguish every LLVM signature.
    // The backend extracts the operation name and reads types from the IR.
    std::string Signature;
    llvm::raw_string_ostream OS(Signature);
    Return->print(OS);
    SmallVector<llvm::Type *> Types;
    for (auto *V : Args) {
      Types.push_back(V->getType());
      V->getType()->print(OS);
    }
    OS.flush();
    uint64_t Hash = 14695981039346656037ULL;
    for (unsigned char Ch : Signature)
      Hash = (Hash ^ Ch) * 1099511628211ULL;
    std::string Base =
        (Op == "SpecConstant" ? "__spirv_" : "__spirv_shader_") + Op.str();
    std::string Mangled = "_Z" + std::to_string(Base.size()) + Base + "ILy" +
                          std::to_string(Hash) + "EEvv";
    auto Callee = CGM.getModule().getOrInsertFunction(
        Mangled, llvm::FunctionType::get(Return, Types, false));
    auto *F = cast<llvm::Function>(Callee.getCallee());
    F->addFnAttr("spirv.cxx.intrinsic", Op);
    F->addFnAttr(llvm::Attribute::NoUnwind);
    if (Convergent)
      F->addFnAttr(llvm::Attribute::Convergent);
    auto *Call = Builder.CreateCall(Callee, Args);
    if (Convergent)
      Call = cast<llvm::CallInst>(addConvergenceControlToken(Call));
    return Call;
  };
  std::function<void(Address, QualType, SmallVectorImpl<Address> &,
                     SmallVectorImpl<bool> *)>
      Leaves = [&](Address A, QualType T, SmallVectorImpl<Address> &Out,
                   SmallVectorImpl<bool> *Volatile) {
        if (auto *AT = C.getAsConstantArrayType(T)) {
          for (unsigned I = 0; I < AT->getSize().getZExtValue(); ++I)
            Leaves(Builder.CreateConstArrayGEP(A, I),
                   T.isVolatileQualified() ? AT->getElementType().withVolatile()
                                           : AT->getElementType(),
                   Out, Volatile);
        } else if (auto *RD = T->getAsCXXRecordDecl()) {
          const auto &Layout = CGM.getTypes().getCGRecordLayout(RD);
          for (auto *F : RD->fields())
            Leaves(Builder.CreateStructGEP(A, Layout.getLLVMFieldNo(F)),
                   T.isVolatileQualified() ? F->getType().withVolatile()
                                           : F->getType(),
                   Out, Volatile);
        } else {
          Out.push_back(A);
          if (Volatile)
            Volatile->push_back(T.isVolatileQualified());
        }
      };
  auto Addresses = [&](unsigned I) {
    SmallVector<Address> A;
    const Expr *Value = E->getArg(I);
    Address Storage = Value->isLValue()
                          ? EmitLValue(Value).getAddress()
                          : Address(CreateMemTemp(Value->getType()));
    if (!Value->isLValue())
      EmitAnyExprToMem(Value, Storage, Qualifiers(), true);
    SmallVector<bool> Volatile;
    Leaves(Storage, Value->getType(), A, &Volatile);
    return std::make_pair(A, Volatile);
  };
  auto LoadComponents = [&](unsigned I) {
    SmallVector<llvm::Value *> Values;
    auto [A, Volatile] = Addresses(I);
    for (unsigned N = 0; N < A.size(); ++N) {
      auto *Value = Builder.CreateLoad(A[N]);
      Value->setVolatile(Volatile[N]);
      Values.push_back(Value);
    }
    return Values;
  };
  auto Pack = [&](ArrayRef<llvm::Value *> Values) -> llvm::Value * {
    if (Values.size() == 1)
      return Values[0];
    llvm::Value *V = llvm::PoisonValue::get(
        llvm::FixedVectorType::get(Values[0]->getType(), Values.size()));
    for (unsigned I = 0; I < Values.size(); ++I)
      V = Builder.CreateInsertElement(V, Values[I], I32(I));
    return V;
  };
  auto Load = [&](unsigned I) -> llvm::Value * {
    if (E->getArg(I)->getType()->isScalarType())
      return Scalar(I);
    return Pack(LoadComponents(I));
  };
  auto ResultComponents = [&](ArrayRef<llvm::Value *> Values) -> RValue {
    if (E->getType()->isScalarType())
      return RValue::get(Values.front());
    Address Storage = ReturnValue.isNull()
                          ? Address(CreateMemTemp(E->getType()))
                          : ReturnValue.getAddress();
    SmallVector<Address> Out;
    Leaves(Storage, E->getType(), Out, nullptr);
    assert(Out.size() == Values.size() && "result component count mismatch");
    for (unsigned I = 0; I < Out.size(); ++I)
      Builder.CreateStore(Values[I], Out[I]);
    return RValue::getAggregate(Storage);
  };
  auto Result = [&](llvm::Value *V) -> RValue {
    SmallVector<llvm::Value *> Values;
    unsigned Count = shaderShape(C, E->getType()).Count;
    for (unsigned I = 0; I < Count; ++I)
      Values.push_back(V->getType()->isVectorTy()
                           ? Builder.CreateExtractElement(V, I32(I))
                           : V);
    return ResultComponents(Values);
  };
  auto Dot = [&](llvm::Value *A, llvm::Value *B) -> llvm::Value * {
    if (!getLangOpts().ShaderHostMath)
      return Builder.CreateIntrinsic(A->getType()->getScalarType(),
                                     llvm::Intrinsic::spv_fdot, {A, B},
                                     nullptr);
    auto *V = cast<llvm::FixedVectorType>(A->getType());
    llvm::Value *Sum = llvm::ConstantFP::get(V->getElementType(), 0);
    for (unsigned I = 0; I < V->getNumElements(); ++I)
      Sum = Builder.CreateFAdd(
          Sum, Builder.CreateFMul(Builder.CreateExtractElement(A, I32(I)),
                                  Builder.CreateExtractElement(B, I32(I))));
    return Sum;
  };
  auto Splat = [&](llvm::Value *V, llvm::Type *T) -> llvm::Value * {
    if (!T->isVectorTy())
      return V;
    return Builder.CreateVectorSplat(
        cast<llvm::FixedVectorType>(T)->getElementCount(), V);
  };
  auto Constant = [&](llvm::Type *T, double V) -> llvm::Value * {
    return Splat(llvm::ConstantFP::get(T->getScalarType(), V), T);
  };
  auto Sqrt = [&](llvm::Value *V) {
    return Builder.CreateUnaryIntrinsic(llvm::Intrinsic::sqrt, V);
  };
  if (Name == "add_carry" || Name == "sub_borrow" ||
      Name == "mul_extended") {
    auto *A = Load(0);
    auto *B = Load(1);
    auto S = shaderShape(C, E->getArg(0)->getType());
    llvm::StringRef Op = Name == "add_carry"
                             ? (S.Count == 1 ? "IAddCarryS" : "IAddCarryV")
                         : Name == "sub_borrow"
                             ? (S.Count == 1 ? "ISubBorrowS" : "ISubBorrowV")
                         : S.Element->isSignedIntegerType() ? "SMulExtended"
                                                            : "UMulExtended";
    auto *RT = llvm::StructType::get(LC, {A->getType(), A->getType()});
    auto *V = Native(Op, RT, {A, B});
    SmallVector<llvm::Value *> Fields;
    for (unsigned I = 0; I != 2; ++I) {
      auto *Field = Builder.CreateExtractValue(V, I);
      for (unsigned J = 0; J != S.Count; ++J)
        Fields.push_back(S.Count == 1
                             ? Field
                             : Builder.CreateExtractElement(Field, I32(J)));
    }
    return ResultComponents(Fields);
  }
  if (Name == "is_nan" || Name == "is_inf" || Name == "is_finite" ||
      Name == "is_normal" || Name == "sign_bit") {
    auto *V = Load(0);
    unsigned Bits = V->getType()->getScalarSizeInBits();
    unsigned ExponentBits = Bits == 16 ? 5 : Bits == 32 ? 8 : 11;
    unsigned FractionBits = Bits - ExponentBits - 1;
    llvm::Type *IntType = Builder.getIntNTy(Bits);
    if (auto *Vector = llvm::dyn_cast<llvm::FixedVectorType>(V->getType()))
      IntType = llvm::FixedVectorType::get(IntType, Vector->getNumElements());
    auto IntConstant = [&](uint64_t Value) -> llvm::Constant * {
      llvm::Constant *C = llvm::ConstantInt::get(Builder.getIntNTy(Bits), Value);
      if (auto *Vector = llvm::dyn_cast<llvm::FixedVectorType>(IntType))
        C = llvm::ConstantVector::getSplat(Vector->getElementCount(), C);
      return C;
    };
    auto *Raw = Builder.CreateBitCast(V, IntType);
    auto *Zero = IntConstant(0);
    auto ClassResult = [&](llvm::Value *Predicate) -> RValue {
      if (auto *Vector =
              llvm::dyn_cast<llvm::FixedVectorType>(Predicate->getType()))
        Predicate = Builder.CreateZExt(
            Predicate, llvm::FixedVectorType::get(Builder.getInt32Ty(),
                                                  Vector->getNumElements()));
      return Result(Predicate);
    };
    if (Name == "sign_bit")
      return ClassResult(Builder.CreateICmpNE(
          Builder.CreateLShr(Raw, IntConstant(Bits - 1)), Zero));
    auto *Exponent = Builder.CreateAnd(
        Builder.CreateLShr(Raw, IntConstant(FractionBits)),
        IntConstant((uint64_t(1) << ExponentBits) - 1));
    auto *AllOnes = IntConstant((uint64_t(1) << ExponentBits) - 1);
    if (Name == "is_finite")
      return ClassResult(Builder.CreateICmpNE(Exponent, AllOnes));
    if (Name == "is_normal")
      return ClassResult(Builder.CreateAnd(
          Builder.CreateICmpNE(Exponent, Zero),
          Builder.CreateICmpNE(Exponent, AllOnes)));
    auto *Fraction = Builder.CreateAnd(
        Raw, IntConstant((uint64_t(1) << FractionBits) - 1));
    return ClassResult(Builder.CreateAnd(
        Builder.CreateICmpEQ(Exponent, AllOnes),
        Name == "is_nan" ? Builder.CreateICmpNE(Fraction, Zero)
                         : Builder.CreateICmpEQ(Fraction, Zero)));
  }
  if (Name == "modf" || Name == "frexp" || Name == "ldexp") {
    auto *A = Load(0);
    auto *T = A->getType();
    if (Name == "ldexp") {
      auto *Exponent = Load(1);
      if (!getLangOpts().ShaderHostMath)
        return Result(Native("Ldexp", T, {A, Exponent}));
      auto *F = llvm::Intrinsic::getOrInsertDeclaration(
          &CGM.getModule(), llvm::Intrinsic::ldexp, {T, Exponent->getType()});
      return Result(Builder.CreateCall(F, {A, Exponent}));
    }
    llvm::Type *Exponent = Builder.getInt32Ty();
    if (T->isVectorTy())
      Exponent = llvm::FixedVectorType::get(
          Exponent, cast<llvm::FixedVectorType>(T)->getNumElements());
    auto *RT = llvm::StructType::get(LC, {T, Name == "frexp" ? Exponent : T});
    llvm::Value *V;
    if (!getLangOpts().ShaderHostMath)
      V = Native(Name == "frexp" ? "FrexpStruct" : "ModfStruct", RT, {A});
    else {
      auto ID =
          Name == "frexp" ? llvm::Intrinsic::frexp : llvm::Intrinsic::modf;
      auto *F = llvm::Intrinsic::getOrInsertDeclaration(
          &CGM.getModule(), ID,
          Name == "frexp" ? ArrayRef<llvm::Type *>({T, Exponent})
                          : ArrayRef<llvm::Type *>({T}));
      V = Builder.CreateCall(F, {A});
    }
    SmallVector<llvm::Value *> Fields;
    unsigned Count = shaderShape(C, E->getArg(0)->getType()).Count;
    for (unsigned I = 0; I < 2; ++I) {
      auto *Field = Builder.CreateExtractValue(V, I);
      for (unsigned J = 0; J < Count; ++J)
        Fields.push_back(
            Count == 1 ? Field : Builder.CreateExtractElement(Field, I32(J)));
    }
    return ResultComponents(Fields);
  }
  if (Name == "demote" || Name == "shader_clock" || Name == "memory_barrier" ||
      Name == "control_barrier") {
    auto Op = llvm::StringSwitch<llvm::StringRef>(Name)
                  .Case("demote", "DemoteToHelperInvocation")
                  .Case("shader_clock", "ReadClockKHR")
                  .Case("memory_barrier", "MemoryBarrier")
                  .Case("control_barrier", "ControlBarrier")
                  .Default("");
    SmallVector<llvm::Value *> Args;
    for (unsigned I = 0; I < E->getNumArgs(); ++I)
      Args.push_back(Scalar(I));
    return RValue::get(
        Native(Op, Name == "shader_clock" ? ConvertType(E->getType()) : Void,
               Args, true));
  }
  if (Name == "outer_product" ||
      (Name == "mul" && (shaderShape(C, E->getArg(0)->getType()).Rows ||
                         shaderShape(C, E->getArg(1)->getType()).Rows))) {
    auto AS = shaderShape(C, E->getArg(0)->getType());
    auto BS = shaderShape(C, E->getArg(1)->getType());
    SmallVector<llvm::Value *> A, B, R;
    if (E->getArg(0)->getType()->isScalarType())
      A.push_back(Scalar(0));
    else
      A = LoadComponents(0);
    if (E->getArg(1)->getType()->isScalarType())
      B.push_back(Scalar(1));
    else
      B = LoadComponents(1);
    if (Name == "outer_product") {
      for (auto *X : A)
        for (auto *Y : B)
          R.push_back(Builder.CreateFMul(X, Y));
    } else if (E->getArg(0)->getType()->isScalarType() ||
               E->getArg(1)->getType()->isScalarType()) {
      auto &M = AS.Rows ? A : B;
      auto *Scale = AS.Rows ? B[0] : A[0];
      for (auto *X : M)
        R.push_back(Builder.CreateFMul(X, Scale));
    } else {
      unsigned Rows = AS.Rows ? AS.Rows : 1;
      unsigned Inner = AS.Rows ? AS.Columns : AS.Count;
      unsigned Columns = BS.Rows ? BS.Columns : 1;
      for (unsigned I = 0; I < Rows; ++I)
        for (unsigned J = 0; J < Columns; ++J) {
          if (Inner == 1) {
            R.push_back(Builder.CreateFMul(A[I], B[J]));
            continue;
          }
          SmallVector<llvm::Value *> Row, Column;
          for (unsigned K = 0; K < Inner; ++K) {
            Row.push_back(A[I * Inner + K]);
            Column.push_back(B[K * Columns + J]);
          }
          R.push_back(Dot(Pack(Row), Pack(Column)));
        }
    }
    return ResultComponents(R);
  }
  if (Name == "pack_snorm4x8") {
    // Pack each signed lane independently. Mask before shifting so negative
    // values cannot sign-extend into adjacent bytes on Vulkan implementations.
    auto *V = Load(0);
    llvm::Value *Packed = I32(0);
    for (unsigned I = 0; I < 4; ++I) {
      auto *X = Builder.CreateExtractElement(V, I32(I));
      auto *Lo = Builder.CreateBinaryIntrinsic(llvm::Intrinsic::maxnum, X,
                                               Constant(X->getType(), -1));
      auto *Clamped = Builder.CreateBinaryIntrinsic(llvm::Intrinsic::minnum, Lo,
                                                    Constant(X->getType(), 1));
      auto *Scaled = Builder.CreateFMul(Clamped, Constant(X->getType(), 127));
      auto *Rounded =
          Builder.CreateUnaryIntrinsic(llvm::Intrinsic::roundeven, Scaled);
      auto *Bits = Builder.CreateAnd(
          Builder.CreateFPToSI(Rounded, Builder.getInt32Ty()), I32(255));
      Packed = Builder.CreateOr(Packed, Builder.CreateShl(Bits, I32(I * 8)));
    }
    return RValue::get(Packed);
  }
  if (Name.starts_with("pack_") || Name.starts_with("unpack_")) {
    auto Op = llvm::StringSwitch<llvm::StringRef>(Name)
                  .Case("pack_snorm4x8", "PackSnorm4x8")
                  .Case("pack_unorm4x8", "PackUnorm4x8")
                  .Case("pack_snorm2x16", "PackSnorm2x16")
                  .Case("pack_unorm2x16", "PackUnorm2x16")
                  .Case("pack_half2x16", "PackHalf2x16")
                  .Case("pack_double2x32", "PackDouble2x32")
                  .Case("unpack_snorm4x8", "UnpackSnorm4x8")
                  .Case("unpack_unorm4x8", "UnpackUnorm4x8")
                  .Case("unpack_snorm2x16", "UnpackSnorm2x16")
                  .Case("unpack_unorm2x16", "UnpackUnorm2x16")
                  .Case("unpack_half2x16", "UnpackHalf2x16")
                  .Case("unpack_double2x32", "UnpackDouble2x32")
                  .Default("");
    auto S = shaderShape(C, E->getType());
    auto *T = ConvertType(S.Element);
    if (S.Count > 1)
      T = llvm::FixedVectorType::get(T, S.Count);
    return Result(Native(Op, T, {Load(0)}));
  }
  if (Name.starts_with("bit_") || Name == "find_lsb" || Name == "find_msb") {
    auto *V = Load(0);
    auto S = shaderShape(C, E->getArg(0)->getType());
    if (C.getTypeSize(S.Element) == 64) {
      auto *Wide = V->getType();
      llvm::Type *Narrow = Builder.getInt32Ty();
      if (S.Count > 1)
        Narrow = llvm::FixedVectorType::get(Narrow, S.Count);
      auto W = [&](uint64_t N) { return llvm::ConstantInt::get(Wide, N); };
      auto N = [&](uint64_t X) {
        return llvm::ConstantInt::get(Narrow, uint32_t(X));
      };
      auto *Lo = Builder.CreateTrunc(V, Narrow);
      auto *Hi = Builder.CreateTrunc(Builder.CreateLShr(V, W(32)), Narrow);
      if (Name == "bit_count") {
        auto *Count = Builder.CreateAdd(Native("BitCount", Narrow, {Lo}),
                                        Native("BitCount", Narrow, {Hi}));
        return Result(Builder.CreateZExt(Count, Wide));
      }
      if (Name == "bit_reverse") {
        auto *A = Builder.CreateZExt(Native("BitReverse", Narrow, {Lo}), Wide);
        auto *B = Builder.CreateZExt(Native("BitReverse", Narrow, {Hi}), Wide);
        return Result(Builder.CreateOr(Builder.CreateShl(A, W(32)), B));
      }
      if (Name == "find_lsb" || Name == "find_msb") {
        if (Name == "find_msb" && !S.Element->isUnsignedIntegerType()) {
          V = Builder.CreateSelect(Builder.CreateICmpSLT(V, W(0)),
                                   Builder.CreateNot(V), V);
          Lo = Builder.CreateTrunc(V, Narrow);
          Hi = Builder.CreateTrunc(Builder.CreateLShr(V, W(32)), Narrow);
        }
        auto Op = Name == "find_lsb" ? "FindILsb" : "FindUMsb";
        auto *L = Native(Op, Narrow, {Lo});
        auto *H = Native(Op, Narrow, {Hi});
        auto *Adjusted = Builder.CreateAdd(H, N(32));
        llvm::Value *Index =
            Name == "find_lsb"
                ? Builder.CreateSelect(Builder.CreateICmpNE(Lo, N(0)), L,
                                       Adjusted)
                : Builder.CreateSelect(Builder.CreateICmpNE(Hi, N(0)), Adjusted,
                                       L);
        return Result(
            Builder.CreateSelect(Builder.CreateICmpEQ(V, W(0)), N(-1), Index));
      }
      unsigned Start = Name == "bit_insert" ? 2 : 1;
      auto *Offset =
          Splat(Builder.CreateZExt(Scalar(Start), Builder.getInt64Ty()), Wide);
      auto *Count = Splat(
          Builder.CreateZExt(Scalar(Start + 1), Builder.getInt64Ty()), Wide);
      auto *Mask = Builder.CreateSelect(
          Builder.CreateICmpEQ(Count, W(64)), W(-1),
          Builder.CreateSub(Builder.CreateShl(W(1), Count), W(1)));
      llvm::Value *Value;
      if (Name == "bit_insert") {
        Mask = Builder.CreateShl(Mask, Offset);
        Value = Builder.CreateOr(
            Builder.CreateAnd(V, Builder.CreateNot(Mask)),
            Builder.CreateAnd(Builder.CreateShl(Load(1), Offset), Mask));
      } else {
        Value = Builder.CreateAnd(Builder.CreateLShr(V, Offset), Mask);
        if (!S.Element->isUnsignedIntegerType()) {
          auto *Shift = Builder.CreateSub(W(64), Count);
          Value = Builder.CreateAShr(Builder.CreateShl(Value, Shift), Shift);
        }
      }
      return Result(Builder.CreateSelect(Builder.CreateICmpEQ(Count, W(0)),
                                         Name == "bit_insert" ? V : W(0),
                                         Value));
    }
    auto Op =
        llvm::StringSwitch<llvm::StringRef>(Name)
            .Case("bit_count", "BitCount")
            .Case("bit_reverse", "BitReverse")
            .Case("bit_extract", S.Element->isUnsignedIntegerType()
                                     ? "BitFieldUExtract"
                                     : "BitFieldSExtract")
            .Case("bit_insert", "BitFieldInsert")
            .Case("find_lsb", "FindILsb")
            .Case("find_msb",
                  S.Element->isUnsignedIntegerType() ? "FindUMsb" : "FindSMsb")
            .Default("");
    SmallVector<llvm::Value *> A{V};
    for (unsigned I = 1; I < E->getNumArgs(); ++I)
      A.push_back(Name == "bit_insert" && I == 1 ? Load(I) : Scalar(I));
    auto *T = ConvertType(shaderShape(C, E->getType()).Element);
    if (S.Count > 1)
      T = llvm::FixedVectorType::get(T, S.Count);
    return Result(Native(Op, T, A));
  }
  if (Name == "dot")
    return RValue::get(Dot(Load(0), Load(1)));
  if (Name == "length" || Name == "distance") {
    auto *V = Load(0);
    if (Name == "distance")
      V = Builder.CreateFSub(V, Load(1));
    return RValue::get(Sqrt(Dot(V, V)));
  }
  if (shaderComponentMath(Name) &&
      !(Name == "mul" && shaderShape(C, E->getArg(0)->getType()).Rows)) {
    unsigned Start = 0;
    llvm::Value *A = Load(Start), *V = nullptr;
    auto Op = Name;
    if (!getLangOpts().ShaderHostMath &&
        llvm::StringSwitch<bool>(Op)
            .Cases({"sin", "cos", "tan", "asin", "acos", "atan", "atan2"}, true)
            .Cases({"sinh", "cosh", "tanh", "asinh", "acosh", "atanh",
                    "radians", "degrees"},
                   true)
            .Default(false) &&
        A->getType()->getScalarType()->isDoubleTy()) {
      CGM.Error(E->getExprLoc(), "shader trigonometric intrinsics require "
                                 "16- or 32-bit floating-point components");
      return Result(Constant(A->getType(), 0));
    }
    if (!getLangOpts().ShaderHostMath && (Op == "exp2" || Op == "log2") &&
        A->getType()->getScalarType()->isDoubleTy()) {
      CGM.Error(
          E->getExprLoc(),
          "shader exp2/log2 require 16- or 32-bit floating-point components");
      return Result(Constant(A->getType(), 0));
    }
    bool Boolean =
        shaderShape(C, E->getArg(Start)->getType()).Element->isBooleanType();
    auto Promote = [&](llvm::Value *Value) -> llvm::Value * {
      if (!Boolean)
        return Value;
      Value = Builder.CreateICmpNE(
          Value, llvm::Constant::getNullValue(Value->getType()));
      auto *Type = Value->getType()->isVectorTy()
                       ? llvm::FixedVectorType::get(
                             Builder.getInt32Ty(),
                             cast<llvm::FixedVectorType>(Value->getType())
                                 ->getNumElements())
                       : static_cast<llvm::Type *>(Builder.getInt32Ty());
      return Builder.CreateZExt(Value, Type);
    };
    A = Promote(A);
    bool Integer = A->getType()->getScalarType()->isIntegerTy();
    bool Unsigned = shaderShape(C, E->getArg(Start)->getType())
                        .Element->isUnsignedIntegerType();
    llvm::Intrinsic::ID IID = llvm::StringSwitch<llvm::Intrinsic::ID>(Op)
                                  .Case("sqrt", llvm::Intrinsic::sqrt)
                                  .Case("abs", llvm::Intrinsic::fabs)
                                  .Case("floor", llvm::Intrinsic::floor)
                                  .Case("ceil", llvm::Intrinsic::ceil)
                                  .Case("sin", llvm::Intrinsic::sin)
                                  .Case("cos", llvm::Intrinsic::cos)
                                  .Case("acos", llvm::Intrinsic::acos)
                                  .Case("asin", llvm::Intrinsic::asin)
                                  .Case("atan", llvm::Intrinsic::atan)
                                  .Case("tan", llvm::Intrinsic::tan)
                                  .Case("sinh", llvm::Intrinsic::sinh)
                                  .Case("cosh", llvm::Intrinsic::cosh)
                                  .Case("tanh", llvm::Intrinsic::tanh)
                                  .Case("exp", llvm::Intrinsic::exp)
                                  .Case("log", llvm::Intrinsic::log)
                                  .Default(llvm::Intrinsic::not_intrinsic);
    if (Op == "round" || Op == "round_even" || Op == "trunc" || Op == "exp2" ||
        Op == "log2") {
      auto ID = llvm::StringSwitch<llvm::Intrinsic::ID>(Op)
                    .Case("round", llvm::Intrinsic::round)
                    .Case("round_even", llvm::Intrinsic::roundeven)
                    .Case("trunc", llvm::Intrinsic::trunc)
                    .Case("exp2", llvm::Intrinsic::exp2)
                    .Case("log2", llvm::Intrinsic::log2)
                    .Default(llvm::Intrinsic::not_intrinsic);
      V = Builder.CreateUnaryIntrinsic(ID, A);
    } else if (Op == "fract") {
      V = Builder.CreateFSub(
          A, Builder.CreateUnaryIntrinsic(llvm::Intrinsic::floor, A));
    } else if (Op == "inverse_sqrt") {
      V = Builder.CreateFDiv(Constant(A->getType(), 1), Sqrt(A));
    } else if (Op == "sign") {
      auto *Zero = Integer ? llvm::Constant::getNullValue(A->getType())
                           : Constant(A->getType(), 0);
      auto *Positive = Integer ? Builder.CreateICmpSGT(A, Zero)
                               : Builder.CreateFCmpOGT(A, Zero);
      auto *Negative = Integer ? Builder.CreateICmpSLT(A, Zero)
                               : Builder.CreateFCmpOLT(A, Zero);
      auto *One = Integer ? llvm::ConstantInt::get(A->getType(), 1)
                          : Constant(A->getType(), 1);
      auto *MinusOne = Integer ? llvm::ConstantInt::get(A->getType(), -1, true)
                               : Constant(A->getType(), -1);
      if (Integer && Unsigned)
        V = Builder.CreateSelect(Builder.CreateICmpNE(A, Zero), One, Zero);
      else
        V = Builder.CreateSelect(
            Positive, One, Builder.CreateSelect(Negative, MinusOne, Zero));
    } else if (Op == "step") {
      V = Builder.CreateSelect(Builder.CreateFCmpOLT(Load(1), A),
                               Constant(A->getType(), 0),
                               Constant(A->getType(), 1));
    } else if (Op == "smoothstep") {
      auto *T = Builder.CreateFDiv(Builder.CreateFSub(Load(2), A),
                                   Builder.CreateFSub(Load(1), A));
      T = Builder.CreateBinaryIntrinsic(llvm::Intrinsic::maxnum, T,
                                        Constant(A->getType(), 0));
      T = Builder.CreateBinaryIntrinsic(llvm::Intrinsic::minnum, T,
                                        Constant(A->getType(), 1));
      V = Builder.CreateFMul(
          Builder.CreateFMul(T, T),
          Builder.CreateFSub(Constant(A->getType(), 3),
                             Builder.CreateFMul(Constant(A->getType(), 2), T)));
    } else if (Op == "fma") {
      V = Builder.CreateIntrinsic(llvm::Intrinsic::fma, {A->getType()},
                                  {A, Load(1), Load(2)});
    } else if (Op == "mod") {
      auto *B = Load(1);
      V = Builder.CreateFSub(
          A, Builder.CreateFMul(
                 B, Builder.CreateUnaryIntrinsic(llvm::Intrinsic::floor,
                                                 Builder.CreateFDiv(A, B))));
    } else if (Op == "radians" || Op == "degrees") {
      if (getLangOpts().ShaderHostMath)
        V = Builder.CreateFMul(
            A, Constant(A->getType(), Op == "radians" ? 0.017453292519943295
                                                      : 57.29577951308232));
      else
        V = Builder.CreateUnaryIntrinsic(Op == "radians"
                                             ? llvm::Intrinsic::spv_radians
                                             : llvm::Intrinsic::spv_degrees,
                                         A);
    } else if (Op == "asinh" || Op == "acosh" || Op == "atanh") {
      if (!getLangOpts().ShaderHostMath) {
        V = Native(llvm::StringSwitch<llvm::StringRef>(Op)
                       .Case("asinh", "Asinh")
                       .Case("acosh", "Acosh")
                       .Case("atanh", "Atanh")
                       .Default(""),
                   A->getType(), {A});
      } else {
        SmallVector<llvm::Value *> Values;
        unsigned N =
            A->getType()->isVectorTy()
                ? cast<llvm::FixedVectorType>(A->getType())->getNumElements()
                : 1;
        for (unsigned I = 0; I < N; ++I) {
          llvm::Value *X = N == 1 ? A : Builder.CreateExtractElement(A, I32(I));
          llvm::Type *Type = X->getType();
          bool Narrow = Type->isHalfTy();
          if (Narrow) {
            Type = Builder.getFloatTy();
            X = Builder.CreateFPExt(X, Type);
          }
          std::string Name = Op.str() + (Type->isFloatTy() ? "f" : "");
          auto Callee = CGM.getModule().getOrInsertFunction(
              Name, llvm::FunctionType::get(Type, {Type}, false));
          llvm::Value *Y = Builder.CreateCall(Callee, {X});
          if (Narrow)
            Y = Builder.CreateFPTrunc(Y, Builder.getHalfTy());
          Values.push_back(Y);
        }
        V = Pack(Values);
      }
    } else if (Op == "abs" && Integer) {
      V = Unsigned ? A
                   : Builder.CreateBinaryIntrinsic(llvm::Intrinsic::abs, A,
                                                   Builder.getFalse());
    } else if (IID != llvm::Intrinsic::not_intrinsic) {
      V = Builder.CreateUnaryIntrinsic(IID, A);
    } else {
      auto *B = Promote(Load(Start + 1));
      auto Min =
          Integer ? (Unsigned ? llvm::Intrinsic::umin : llvm::Intrinsic::smin)
                  : llvm::Intrinsic::minnum;
      auto Max =
          Integer ? (Unsigned ? llvm::Intrinsic::umax : llvm::Intrinsic::smax)
                  : llvm::Intrinsic::maxnum;
      if (Op == "add")
        V = Integer ? Builder.CreateAdd(A, B) : Builder.CreateFAdd(A, B);
      else if (Op == "sub")
        V = Integer ? Builder.CreateSub(A, B) : Builder.CreateFSub(A, B);
      else if (Op == "mul")
        V = Integer ? Builder.CreateMul(A, B) : Builder.CreateFMul(A, B);
      else if (Op == "div")
        V = Integer ? (Unsigned ? Builder.CreateUDiv(A, B)
                                : Builder.CreateSDiv(A, B))
                    : Builder.CreateFDiv(A, B);
      else if (Op == "pow")
        V = Builder.CreateBinaryIntrinsic(llvm::Intrinsic::pow, A, B);
      else if (Op == "atan2")
        V = Builder.CreateBinaryIntrinsic(llvm::Intrinsic::atan2, A, B);
      else if (Op == "min")
        V = Builder.CreateBinaryIntrinsic(Min, A, B);
      else if (Op == "max")
        V = Builder.CreateBinaryIntrinsic(Max, A, B);
      else if (Op == "clamp")
        V = Builder.CreateBinaryIntrinsic(
            Min, Builder.CreateBinaryIntrinsic(Max, A, B),
            Promote(Load(Start + 2)));
      else if (Op == "lerp")
        V = Builder.CreateFAdd(
            A, Builder.CreateFMul(Builder.CreateFSub(B, A), Load(Start + 2)));
    }
    if (Boolean) {
      V = Builder.CreateICmpNE(V, llvm::Constant::getNullValue(V->getType()));
      auto *MemoryType =
          V->getType()->isVectorTy()
              ? llvm::FixedVectorType::get(
                    Builder.getInt8Ty(),
                    cast<llvm::FixedVectorType>(V->getType())->getNumElements())
              : static_cast<llvm::Type *>(Builder.getInt8Ty());
      V = Builder.CreateZExt(V, MemoryType);
    }
    return Result(V);
  }
  if (Name == "normalize" || Name == "reflect" || Name == "refract" ||
      Name == "faceforward" || Name == "cross") {
    llvm::Value *A = Load(0), *V = nullptr;
    if (Name == "normalize")
      V = Builder.CreateFDiv(A, Splat(Sqrt(Dot(A, A)), A->getType()));
    else if (Name == "reflect" || Name == "refract") {
      auto *N = Load(1);
      auto *D = Dot(N, A);
      if (Name == "reflect")
        V = Builder.CreateFSub(
            A, Builder.CreateFMul(
                   N, Splat(Builder.CreateFMul(D, Constant(D->getType(), 2)),
                            N->getType())));
      else {
        auto *Eta = Scalar(2);
        auto *K = Builder.CreateFSub(
            Constant(D->getType(), 1),
            Builder.CreateFMul(Builder.CreateFMul(Eta, Eta),
                               Builder.CreateFSub(Constant(D->getType(), 1),
                                                  Builder.CreateFMul(D, D))));
        auto *R = Builder.CreateFSub(
            Builder.CreateFMul(A, Splat(Eta, A->getType())),
            Builder.CreateFMul(
                N,
                Splat(Builder.CreateFAdd(Builder.CreateFMul(Eta, D), Sqrt(K)),
                      N->getType())));
        V = Builder.CreateSelect(
            Builder.CreateFCmpOLT(K, Constant(K->getType(), 0)),
            llvm::Constant::getNullValue(A->getType()), R);
      }
    } else if (Name == "faceforward") {
      auto *D = Dot(Load(2), Load(1));
      V = Builder.CreateSelect(
          Builder.CreateFCmpOLT(D, Constant(D->getType(), 0)), A,
          Builder.CreateFNeg(A));
    } else if (Name == "cross") {
      auto *B = Load(1);
      SmallVector<llvm::Value *> X;
      for (unsigned I = 0; I < 3; ++I) {
        unsigned J = (I + 1) % 3, K = (I + 2) % 3;
        X.push_back(Builder.CreateFSub(
            Builder.CreateFMul(Builder.CreateExtractElement(A, I32(J)),
                               Builder.CreateExtractElement(B, I32(K))),
            Builder.CreateFMul(Builder.CreateExtractElement(A, I32(K)),
                               Builder.CreateExtractElement(B, I32(J)))));
      }
      V = Pack(X);
    }
    return Result(V);
  }
  if (Name == "mul" || Name == "transpose" || Name == "determinant" ||
      Name == "inverse") {
    unsigned Input = 0;
    auto S = shaderShape(C, E->getArg(Input)->getType());
    auto A = LoadComponents(Input);
    SmallVector<llvm::Value *> R;
    auto *T = A[0]->getType();
    std::function<llvm::Value *(ArrayRef<llvm::Value *>, unsigned)> Det =
        [&](ArrayRef<llvm::Value *> M, unsigned N) -> llvm::Value * {
      if (N == 1)
        return M[0];
      llvm::Value *D = Constant(T, 0);
      for (unsigned Col = 0; Col < N; ++Col) {
        SmallVector<llvm::Value *> Minor;
        for (unsigned I = 1; I < N; ++I)
          for (unsigned J = 0; J < N; ++J)
            if (J != Col)
              Minor.push_back(M[I * N + J]);
        auto *Term = Builder.CreateFMul(M[Col], Det(Minor, N - 1));
        D = Col % 2 ? Builder.CreateFSub(D, Term) : Builder.CreateFAdd(D, Term);
      }
      return D;
    };
    if (Name == "determinant")
      return RValue::get(Det(A, S.Rows));
    if (Name == "transpose") {
      for (unsigned J = 0; J < S.Columns; ++J)
        for (unsigned I = 0; I < S.Rows; ++I)
          R.push_back(A[I * S.Columns + J]);
    } else if (Name == "mul") {
      auto B = LoadComponents(1);
      auto BS = shaderShape(C, E->getArg(1)->getType());
      for (unsigned I = 0; I < S.Rows; ++I)
        for (unsigned J = 0; J < BS.Columns; ++J) {
          llvm::Value *Sum = Constant(T, 0);
          for (unsigned K = 0; K < S.Columns; ++K)
            Sum = Builder.CreateFAdd(Sum,
                                     Builder.CreateFMul(A[I * S.Columns + K],
                                                        B[K * BS.Columns + J]));
          R.push_back(Sum);
        }
    } else {
      auto *D = Det(A, S.Rows);
      for (unsigned I = 0; I < S.Rows; ++I)
        for (unsigned J = 0; J < S.Columns; ++J) {
          SmallVector<llvm::Value *> Minor;
          for (unsigned Y = 0; Y < S.Rows; ++Y)
            if (Y != J)
              for (unsigned X = 0; X < S.Columns; ++X)
                if (X != I)
                  Minor.push_back(A[Y * S.Columns + X]);
          llvm::Value *V =
              S.Rows == 1 ? Constant(T, 1) : Det(Minor, S.Rows - 1);
          if ((I + J) % 2)
            V = Builder.CreateFNeg(V);
          R.push_back(Builder.CreateFDiv(V, D));
        }
    }
    return ResultComponents(R);
  }
  if (Name == "specialization_constant") {
    Expr::EvalResult Id, Default;
    E->getArg(0)->EvaluateAsInt(Id, C);
    E->getArg(1)->EvaluateAsRValue(Default, C);
    llvm::Type *T = ConvertType(E->getType());
    llvm::Value *V = Default.Val.isInt()
                         ? static_cast<llvm::Value *>(
                               llvm::ConstantInt::get(T, Default.Val.getInt()))
                         : llvm::ConstantFP::get(LC, Default.Val.getFloat());
    auto *Call = cast<llvm::CallInst>(
        Native("SpecConstant", T, {I32(Id.Val.getInt().getZExtValue()), V}));
    Call->setMetadata(
        "spirv.cxx.specialization.type",
        llvm::MDNode::get(
            LC, llvm::MDString::get(
                    LC, E->getType().getCanonicalType().getAsString())));
    return RValue::get(Call);
  }
  if (Name.starts_with("atomic")) {
    auto *P = Scalar(0);
    auto T = E->getArg(0)->getType()->getPointeeType();
    unsigned BaseCount = shaderIntrinsicArity(ID);
    bool Explicit = E->getNumArgs() != BaseCount;
    auto EnumValue = [&](unsigned Index) {
      Expr::EvalResult V;
      E->getArg(Index)->EvaluateAsInt(V, C);
      return unsigned(V.Val.getInt().getZExtValue());
    };
    bool Shared = C.getTargetAddressSpace(T.getAddressSpace()) == 3;
    unsigned Scope = Explicit ? EnumValue(E->getNumArgs() - 1)
                              : Shared ? 2 : 1;
    unsigned Memory = Shared ? 0x100 : 0x40;
    unsigned Order = Explicit ? EnumValue(BaseCount)
                     : Name == "atomicLoad"  ? 2
                     : Name == "atomicStore" ? 4
                                             : 8;
    auto Semantics = [&](unsigned RequestedOrder) {
      return I32(RequestedOrder ? Memory | RequestedOrder : 0);
    };
    if (Name == "atomicLoad")
      return RValue::get(Native("AtomicLoad", ConvertType(T),
                                {P, I32(Scope), Semantics(Order)}));
    if (Name == "atomicStore") {
      Native("AtomicStore", Void,
             {P, I32(Scope), Semantics(Order), Scalar(1)},
             true);
      return RValue::get(nullptr);
    }
    auto *V = Scalar(1);
    if (T->isRealFloatingType()) {
      auto Op = Name == "atomicMin"   ? "AtomicFMinEXT"
                : Name == "atomicMax" ? "AtomicFMaxEXT"
                                      : "AtomicFAddEXT";
      if (Name == "atomicSub")
        V = Builder.CreateFNeg(V);
      return RValue::get(
          Native(Op, V->getType(), {P, I32(Scope), Semantics(Order), V}));
    }
    if (Name == "atomicCAS")
      return RValue::get(Native(
          "AtomicCompareExchange", V->getType(),
          {P, I32(Scope), Semantics(Order),
           Semantics(Explicit ? EnumValue(BaseCount + 1) : 2), Scalar(2),
           V}));
    auto Inst =
        llvm::StringSwitch<llvm::StringRef>(Name)
            .Case("atomicAdd", "AtomicIAdd")
            .Case("atomicSub", "AtomicISub")
            .Case("atomicExch", "AtomicExchange")
            .Case("atomicAnd", "AtomicAnd")
            .Case("atomicOr", "AtomicOr")
            .Case("atomicXor", "AtomicXor")
            .Case("atomicMin",
                  T->isUnsignedIntegerType() ? "AtomicUMin" : "AtomicSMin")
            .Case("atomicMax",
                  T->isUnsignedIntegerType() ? "AtomicUMax" : "AtomicSMax")
            .Default("");
    return RValue::get(
        Native(Inst, V->getType(), {P, I32(Scope), Semantics(Order), V}));
  }
  if (Name == "workgroup_barrier" || Name == "subgroup_barrier" ||
      Name == "device_memory_barrier") {
    if (Name == "device_memory_barrier")
      Native("MemoryBarrier", Void, {I32(1), I32(0x948)}, true);
    else {
      unsigned Scope = Name == "workgroup_barrier" ? 2 : 3;
      Native("ControlBarrier", Void, {I32(Scope), I32(Scope), I32(0x108)},
             true);
    }
    return RValue::get(nullptr);
  }
  if (Name.starts_with("subgroup_")) {
    auto Op = Name.drop_front(9);
    if (Op == "elect")
      return RValue::get(
          Native("GroupNonUniformElect", Builder.getInt1Ty(), {I32(3)}, true));
    if (Op == "ballot_bit_count" || Op == "ballot_inclusive_bit_count" ||
        Op == "ballot_exclusive_bit_count") {
      unsigned Group = Op == "ballot_inclusive_bit_count"   ? 1
                       : Op == "ballot_exclusive_bit_count" ? 2
                                                            : 0;
      return RValue::get(Native("GroupNonUniformBallotBitCount",
                                Builder.getInt32Ty(),
                                {I32(3), I32(Group), Load(0)}, true));
    }
    if (Op == "inverse_ballot" || Op == "ballot_bit_extract" ||
        Op == "ballot_find_lsb" || Op == "ballot_find_msb") {
      auto Inst =
          llvm::StringSwitch<llvm::StringRef>(Op)
              .Case("inverse_ballot", "GroupNonUniformInverseBallot")
              .Case("ballot_bit_extract", "GroupNonUniformBallotBitExtract")
              .Case("ballot_find_lsb", "GroupNonUniformBallotFindLSB")
              .Case("ballot_find_msb", "GroupNonUniformBallotFindMSB")
              .Default("");
      SmallVector<llvm::Value *> A{I32(3), Load(0)};
      if (E->getNumArgs() == 2)
        A.push_back(Scalar(1));
      return RValue::get(Native(Inst, ConvertType(E->getType()), A, true));
    }
    if (Op.starts_with("quad_")) {
      auto *V = Load(0);
      auto *Index = Op == "quad_broadcast"
                        ? Scalar(1)
                        : I32(Op == "quad_swap_horizontal" ? 0
                              : Op == "quad_swap_vertical" ? 1
                                                           : 2);
      return Result(Native(Op == "quad_broadcast"
                               ? "GroupNonUniformQuadBroadcast"
                               : "GroupNonUniformQuadSwap",
                           V->getType(), {I32(3), V, Index}, true));
    }
    if (Op == "ballot") {
      auto *V = Native("GroupNonUniformBallot",
                       llvm::FixedVectorType::get(Builder.getInt32Ty(), 4),
                       {I32(3), Scalar(0)}, true);
      return Result(V);
    }
    SmallVector<llvm::Value *> Args{I32(3)};
    unsigned GroupOp = Op.starts_with("inclusive_")   ? 1
                       : Op.starts_with("exclusive_") ? 2
                       : Op.starts_with("clustered_") ? 3
                                                      : 0;
    if (GroupOp)
      Op = Op.drop_front(10);
    auto *V = Load(0);
    bool FP = V->getType()->getScalarType()->isFloatingPointTy();
    bool Unsigned = shaderShape(C, E->getArg(0)->getType())
                        .Element->isUnsignedIntegerType();
    llvm::StringRef Inst =
        llvm::StringSwitch<llvm::StringRef>(Op)
            .Case("all", "GroupNonUniformAll")
            .Case("any", "GroupNonUniformAny")
            .Case("all_equal", "GroupNonUniformAllEqual")
            .Case("broadcast", "GroupNonUniformBroadcast")
            .Case("broadcast_first", "GroupNonUniformBroadcastFirst")
            .Case("shuffle", "GroupNonUniformShuffle")
            .Case("shuffle_xor", "GroupNonUniformShuffleXor")
            .Case("shuffle_up", "GroupNonUniformShuffleUp")
            .Case("shuffle_down", "GroupNonUniformShuffleDown")
            .Case("add", FP ? "GroupNonUniformFAdd" : "GroupNonUniformIAdd")
            .Case("mul", FP ? "GroupNonUniformFMul" : "GroupNonUniformIMul")
            .Case("min", FP         ? "GroupNonUniformFMin"
                         : Unsigned ? "GroupNonUniformUMin"
                                    : "GroupNonUniformSMin")
            .Case("max", FP         ? "GroupNonUniformFMax"
                         : Unsigned ? "GroupNonUniformUMax"
                                    : "GroupNonUniformSMax")
            .Case("and", "GroupNonUniformBitwiseAnd")
            .Case("or", "GroupNonUniformBitwiseOr")
            .Case("xor", "GroupNonUniformBitwiseXor")
            .Case("logical_and", "GroupNonUniformLogicalAnd")
            .Case("logical_or", "GroupNonUniformLogicalOr")
            .Case("logical_xor", "GroupNonUniformLogicalXor")
            .Default("");
    if (Op == "add" || Op == "mul" || Op == "min" || Op == "max" ||
        Op == "and" || Op == "or" || Op == "xor" ||
        Op.starts_with("logical_"))
      Args.push_back(I32(GroupOp));
    Args.push_back(V);
    if (E->getNumArgs() == 2)
      Args.push_back(Scalar(1));
    return Result(Native(Inst,
                         Op == "all_equal" ? Builder.getInt1Ty() : V->getType(),
                         Args, true));
  }
  if (Name.starts_with("ddx") || Name.starts_with("ddy") || Name == "fwidth") {
    auto Op = llvm::StringSwitch<llvm::StringRef>(Name)
                  .Case("ddx", "DPdx")
                  .Case("ddy", "DPdy")
                  .Case("fwidth", "Fwidth")
                  .Case("ddx_fine", "DPdxFine")
                  .Case("ddy_fine", "DPdyFine")
                  .Case("ddx_coarse", "DPdxCoarse")
                  .Case("ddy_coarse", "DPdyCoarse")
                  .Default("");
    return RValue::get(
        Native(Op, ConvertType(E->getType()), {Scalar(0)}, true));
  }
  if (Name == "discard") {
    Builder.CreateCall(llvm::Intrinsic::getOrInsertDeclaration(
        &CGM.getModule(), llvm::Intrinsic::spv_discard));
    return RValue::get(nullptr);
  }
  if (Name == "nonuniform") {
    auto *V = Scalar(0);
    // A copy gives this decoration its own SSA result instead of decorating
    // a literal or an unrelated use of the original index.
    auto *Copy = Native("CopyObject", V->getType(), {V});
    auto *MD = llvm::MDNode::get(
        LC, llvm::MDNode::get(LC, llvm::ConstantAsMetadata::get(I32(5300))));
    Builder.CreateIntrinsic(llvm::Intrinsic::spv_assign_decoration,
                            {Copy->getType()},
                            {Copy, llvm::MetadataAsValue::get(LC, MD)});
    return RValue::get(Copy);
  }
  if (Name == "descriptor_subscript") {
    unsigned ArrayArg =
        shaderResource(E->getArg(0)->getType()).Name == "descriptor_array" ? 0
                                                                           : 1;
    auto R = shaderResource(E->getArg(ArrayArg)->getType());
    auto *T = CGM.GetShaderResourceType(R.Descriptor);
    // C++17 sequences the left operand before the right operand, including
    // the equivalent index[array] spelling. Evaluate each exactly once.
    llvm::Value *Operands[2];
    for (unsigned I = 0; I != 2; ++I) {
      if (I != ArrayArg) {
        Operands[I] = Scalar(I);
        continue;
      }
      const Expr *Array = E->getArg(I);
      llvm::Value *Carrier =
          Array->getType()->isArrayType()
              ? EmitLValue(Array).getAddress().emitRawPointer(*this)
              : Scalar(I);
      llvm::Type *CarrierType = ConvertTypeForMem(R.Descriptor);
      Address Storage(Carrier, CarrierType,
                      C.getTypeAlignInChars(R.Descriptor));
      Operands[I] = Builder.CreateLoad(R.Descriptor->isRecordType()
                                           ? Builder.CreateStructGEP(Storage, 0)
                                           : Storage);
    }
    auto *P = Builder.CreateGEP(T, Operands[ArrayArg], Operands[1 - ArrayArg]);
    // Descriptor indices can vary between invocations. Decorate the access
    // itself so the backend requests the resource-specific indexing capability.
    auto *MD = llvm::MDNode::get(
        LC, llvm::MDNode::get(LC, llvm::ConstantAsMetadata::get(I32(5300))));
    Builder.CreateIntrinsic(llvm::Intrinsic::spv_assign_decoration,
                            {P->getType()},
                            {P, llvm::MetadataAsValue::get(LC, MD)});
    if (E->getType()->isRecordType()) {
      Address Storage = ReturnValue.isNull()
                            ? Address(CreateMemTemp(E->getType()))
                            : ReturnValue.getAddress();
      Builder.CreateStore(P, Builder.CreateStructGEP(Storage, 0));
      return RValue::getAggregate(Storage);
    }
    return RValue::get(P);
  }
  if (Name.starts_with("texture_")) {
    bool Offset = Name.ends_with("_offset");
    llvm::StringRef BaseName = Offset ? Name.drop_back(7) : Name;
    bool Gather = BaseName == "texture_gather";
    auto R = shaderResource(E->getArg(0)->getType());
    auto *ImageTy = CGM.GetShaderResourceType(E->getArg(0)->getType());
    auto *Image = Builder.CreateLoad(
        Address(ResourceHandle(0), ImageTy, CharUnits::One()));
    auto TS = shaderShape(C, R.Element);
    auto *Texel = llvm::FixedVectorType::get(ConvertType(TS.Element), 4);
    bool Sample = Name.starts_with("texture_sample") || Gather ||
                  Name == "texture_lod";
    unsigned Width = (R.Dim == 3 ? 3 : R.Dim == 5 ? 1 : R.Dim + 1) + R.Arrayed;
    bool Mip = R.Sampled == 1 && !R.MS && R.Dim != 5;
    auto Components = [&](unsigned Start, unsigned Count) {
      SmallVector<llvm::Value *> Values;
      for (unsigned I = 0; I < Count; ++I)
        Values.push_back(Scalar(Start + I));
      return Pack(Values);
    };
    if (Name == "texture_levels" || Name == "texture_samples")
      return RValue::get(Native(Name == "texture_levels" ? "ImageQueryLevels"
                                                         : "ImageQuerySamples",
                                Builder.getInt32Ty(), {Image}));
    if (Name == "texture_dimensions") {
      auto S = shaderShape(C, E->getType());
      llvm::Type *T = ConvertType(S.Element);
      if (S.Count > 1)
        T = llvm::FixedVectorType::get(T, S.Count);
      SmallVector<llvm::Value *> A{Image};
      if (Mip)
        A.push_back(E->getNumArgs() == 2 ? Scalar(1) : I32(0));
      return Result(Native(Mip ? "ImageQuerySizeLod" : "ImageQuerySize", T, A));
    }
    llvm::Value *Combined = nullptr;
    if (Sample) {
      auto *SamplerTy = CGM.GetShaderResourceType(E->getArg(1)->getType());
      auto *Sampler =
          Builder.CreateLoad(Address(Scalar(1), SamplerTy, CharUnits::One()));
      auto *IT = cast<llvm::TargetExtType>(ImageTy);
      auto *CombinedTy = llvm::TargetExtType::get(
          LC, "spirv.SampledImage", IT->type_params(), IT->int_params());
      Combined = Native("SampledImage", CombinedTy, {Image, Sampler});
    }
    unsigned Start = Sample ? 2 : 1;
    auto *Coordinates = Components(Start, Width);
    if (Name == "texture_lod")
      return Result(Native("ImageQueryLod",
                           llvm::FixedVectorType::get(Builder.getFloatTy(), 2),
                           {Combined, Coordinates}, true));
    if (Name == "texture_load") {
      auto *Selector = E->getNumArgs() > 1 + Width ? Scalar(1 + Width) : I32(0);
      SmallVector<llvm::Value *> A{Image, Coordinates};
      if (Mip) {
        A.push_back(I32(2));
        A.push_back(Selector);
      }
      if (R.MS) {
        A.push_back(I32(64));
        A.push_back(Selector);
      }
      return Result(
          Native(R.Sampled == 2 ? "ImageRead" : "ImageFetch", Texel, A));
    }
    if (Name == "texture_store") {
      bool ExplicitSample = R.MS && E->getNumArgs() == 2 + Width + TS.Count;
      SmallVector<llvm::Value *> Values;
      for (unsigned I = 0; I < TS.Count; ++I)
        Values.push_back(Scalar(1 + Width + unsigned(ExplicitSample) + I));
      while (Values.size() < 4)
        Values.push_back(llvm::Constant::getNullValue(Values[0]->getType()));
      SmallVector<llvm::Value *> A{Image, Coordinates, Pack(Values)};
      if (R.MS) {
        A.push_back(I32(64));
        A.push_back(ExplicitSample ? Scalar(1 + Width) : I32(0));
      }
      Native("ImageWrite", Void, A);
      return RValue::get(nullptr);
    }
    SmallVector<llvm::Value *> A{Combined, Coordinates};
    llvm::StringRef Op = "ImageSampleImplicitLod";
    unsigned Spatial = Width - R.Arrayed;
    unsigned BaseExtra = 0;
    if (BaseName == "texture_sample_bias") {
      A.push_back(I32(Offset ? 9 : 1));
      A.push_back(Scalar(Start + Width));
      BaseExtra = 1;
    } else if (BaseName == "texture_sample_lod") {
      Op = "ImageSampleExplicitLod";
      A.push_back(I32(Offset ? 10 : 2));
      A.push_back(Scalar(Start + Width));
      BaseExtra = 1;
    } else if (BaseName == "texture_sample_grad") {
      Op = "ImageSampleExplicitLod";
      A.push_back(I32(Offset ? 12 : 4));
      A.push_back(Components(Start + Width, Spatial));
      A.push_back(Components(Start + Width + Spatial, Spatial));
      BaseExtra = Spatial * 2;
    } else if (Gather) {
      Op = "ImageGather";
      A.push_back(Scalar(Start + Width));
      BaseExtra = 1;
    }
    if (Offset) {
      if (BaseName == "texture_sample" || Gather)
        A.push_back(I32(8));
      A.push_back(Components(Start + Width + BaseExtra, Spatial));
    }
    return Result(Native(Op, Texel, A, true));
  }
  if (Name == "make_ray_query") {
    auto *Handle =
        Builder.CreateAlloca(llvm::TargetExtType::get(LC, "spirv.RayQueryKHR"));
    Address Storage = ReturnValue.isNull()
                          ? Address(CreateMemTemp(E->getType()))
                          : ReturnValue.getAddress();
    Builder.CreateStore(Handle, Builder.CreateStructGEP(Storage, 0));
    return RValue::getAggregate(Storage);
  }
  if (Name.starts_with("ray_query_")) {
    auto Op =
        llvm::StringSwitch<llvm::StringRef>(Name)
            .Case("ray_query_initialize", "RayQueryInitializeKHR")
            .Case("ray_query_proceed", "RayQueryProceedKHR")
            .Case("ray_query_terminate", "RayQueryTerminateKHR")
            .Case("ray_query_confirm_intersection",
                  "RayQueryConfirmIntersectionKHR")
            .Case("ray_query_generate_intersection",
                  "RayQueryGenerateIntersectionKHR")
            .Case("ray_query_intersection_type",
                  "RayQueryGetIntersectionTypeKHR")
            .Case("ray_query_intersection_t", "RayQueryGetIntersectionTKHR")
            .Case("ray_query_primitive_idx",
                  "RayQueryGetIntersectionPrimitiveIndexKHR")
            .Case("ray_query_instance_custom_idx",
                  "RayQueryGetIntersectionInstanceCustomIndexKHR")
            .Case("ray_query_geometry_idx",
                  "RayQueryGetIntersectionGeometryIndexKHR")
            .Case("ray_query_ray_t_min", "RayQueryGetRayTMinKHR")
            .Case("ray_query_ray_flags", "RayQueryGetRayFlagsKHR")
            .Case("ray_query_instance_idx",
                  "RayQueryGetIntersectionInstanceIdKHR")
            .Case("ray_query_instance_sbt_offset",
                  "RayQueryGetIntersectionInstanceShaderBindingTableRecordOffse"
                  "tKHR")
            .Case("ray_query_barycentrics",
                  "RayQueryGetIntersectionBarycentricsKHR")
            .Case("ray_query_candidate_aabb_opaque",
                  "RayQueryGetIntersectionCandidateAABBOpaqueKHR")
            .Case("ray_query_object_ray_direction",
                  "RayQueryGetIntersectionObjectRayDirectionKHR")
            .Case("ray_query_object_ray_origin",
                  "RayQueryGetIntersectionObjectRayOriginKHR")
            .Case("ray_query_world_ray_direction",
                  "RayQueryGetWorldRayDirectionKHR")
            .Case("ray_query_world_ray_origin", "RayQueryGetWorldRayOriginKHR")
            .Case("ray_query_front_facing",
                  "RayQueryGetIntersectionFrontFaceKHR")
            .Default("");
    SmallVector<llvm::Value *> A{ResourceHandle(0)};
    if (Name == "ray_query_initialize") {
      A.push_back(Builder.CreateLoad(
          Address(Scalar(1), CGM.GetShaderResourceType(E->getArg(1)->getType()),
                  CharUnits::One())));
      A.push_back(Scalar(2));
      A.push_back(Scalar(3));
      A.push_back(Pack({Scalar(4), Scalar(5), Scalar(6)}));
      A.push_back(Scalar(7));
      A.push_back(Pack({Scalar(8), Scalar(9), Scalar(10)}));
      A.push_back(Scalar(11));
    } else {
      for (unsigned I = 1; I < E->getNumArgs(); ++I)
        A.push_back(Scalar(I));
    }
    auto S = shaderShape(C, E->getType());
    llvm::Type *T =
        E->getType()->isVoidType() ? Void : ConvertType(E->getType());
    if (S && S.Count > 1)
      T = llvm::FixedVectorType::get(ConvertType(S.Element), S.Count);
    auto *V = Native(Op, T, A, true);
    return S && S.Count > 1 ? Result(V) : RValue::get(V);
  }
  llvm_unreachable("shader intrinsic has no lowering");
}

// A decayed descriptor array borrows a native range through a private handle
// slot, so advancing it advances the native descriptor address, not the slot.
llvm::Value *CodeGenFunction::EmitShaderDescriptorOffset(QualType PointerType,
                                                         llvm::Value *Carrier,
                                                         llvm::Value *Index) {
  auto R = shaderResource(PointerType);
  QualType Element = R.Descriptor;
  if (Element->isArrayType()) {
    CGM.Error(SourceLocation(),
              "multidimensional descriptor arrays are unsupported");
    return Carrier;
  }
  Address Source(Carrier, ConvertTypeForMem(Element),
                 getContext().getTypeAlignInChars(Element));
  auto *Base = Builder.CreateLoad(
      Element->isRecordType() ? Builder.CreateStructGEP(Source, 0) : Source);
  auto *Next =
      Builder.CreateGEP(CGM.GetShaderResourceType(Element), Base, Index);
  auto &C = getLLVMContext();
  auto *NonUniform =
      llvm::MDNode::get(C, llvm::MDNode::get(C, llvm::ConstantAsMetadata::get(
                                                    Builder.getInt32(5300))));
  Builder.CreateIntrinsic(llvm::Intrinsic::spv_assign_decoration,
                          {Next->getType()},
                          {Next, llvm::MetadataAsValue::get(C, NonUniform)});
  Address Result = CreateMemTemp(Element);
  Builder.CreateStore(Next, Element->isRecordType()
                                ? Builder.CreateStructGEP(Result, 0)
                                : Result);
  return Result.emitRawPointer(*this);
}
