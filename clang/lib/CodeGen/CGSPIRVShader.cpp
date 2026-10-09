//===-- CGSPIRVShader.cpp - Native C++ shader entry interfaces ------------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "CGCall.h"
#include "CGRecordLayout.h"
#include "CodeGenFunction.h"
#include "CodeGenModule.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecordLayout.h"
#include "clang/AST/SPIRVShader.h"
#include "clang/Basic/DiagnosticSema.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/FormatVariadic.h"

using namespace clang;
using namespace clang::CodeGen;

static llvm::Metadata *integerMD(llvm::LLVMContext &C, unsigned V) {
  return llvm::ConstantAsMetadata::get(
      llvm::ConstantInt::get(llvm::Type::getInt32Ty(C), V));
}

static void addDecoration(llvm::GlobalVariable *GV, unsigned Decoration,
                          std::optional<unsigned> Value = std::nullopt) {
  auto &C = GV->getContext();
  SmallVector<llvm::Metadata *> Nodes;
  if (auto *Old = GV->getMetadata("spirv.Decorations"))
    for (const auto &Op : Old->operands())
      Nodes.push_back(Op.get());
  SmallVector<llvm::Metadata *> Operands{integerMD(C, Decoration)};
  if (Value)
    Operands.push_back(integerMD(C, *Value));
  Nodes.push_back(llvm::MDNode::get(C, Operands));
  GV->setMetadata("spirv.Decorations", llvm::MDNode::get(C, Nodes));
}

void CodeGenModule::EmitSPIRVShaderGlobal(const VarDecl *D,
                                          llvm::GlobalVariable *GV) {
  auto &C = GV->getContext();
  if (auto *A = D->getAttr<SPIRVBuiltinVariableAttr>()) {
    // The linker materializes one invocation-lifetime C array after helpers
    // have been linked and legalized. Do not mark this snapshot LLVM-constant.
    GV->setConstant(false);
    GV->setMetadata("spirv.cxx.builtin",
                    llvm::MDNode::get(C, integerMD(C, A->getBuiltin())));
  }
  if (auto *A = D->getAttr<SPIRVBindingAttr>()) {
    addDecoration(GV, 34, A->getSet());
    addDecoration(GV, 33, A->getBinding());
  }
  if (auto *A = D->getAttr<SPIRVInputAttr>())
    addDecoration(GV, 30, A->getLocation());
  if (auto *A = D->getAttr<SPIRVOutputAttr>())
    addDecoration(GV, 30, A->getLocation());
  if (D->hasAttr<SPIRVPositionAttr>())
    addDecoration(GV, 11, 0);
  if (D->hasAttr<SPIRVNoPerspectiveAttr>())
    addDecoration(GV, 13);
  if (D->hasAttr<SPIRVFlatAttr>())
    addDecoration(GV, 14);
  if (D->hasAttr<SPIRVCentroidAttr>())
    addDecoration(GV, 16);
  if (D->hasAttr<SPIRVSampleAttr>())
    addDecoration(GV, 17);
}

void CodeGenModule::EmitSPIRVShaderEntryPoint(const FunctionDecl *D,
                                              llvm::Function *Implementation) {
  auto *Stage = D->getAttr<SPIRVEntryPointAttr>();
  if (!Stage)
    return;
  auto Fail = [&](StringRef Message) {
    Diags.Report(D->getLocation(), diag::err_spirv_shader) << Message;
  };
  auto *Size = D->getAttr<SPIRVWorkgroupSizeAttr>();
  bool HasWorkgroup = Stage->getStage() == SPIRVEntryPointAttr::Compute ||
                      Stage->getStage() == SPIRVEntryPointAttr::Task ||
                      Stage->getStage() == SPIRVEntryPointAttr::Mesh;
  if (HasWorkgroup != (Size != nullptr)) {
    Fail(HasWorkgroup ? "this stage requires __workgroup_size__(x, y, z)"
                      : "this stage does not have workgroup dimensions");
    return;
  }
  llvm::Module &M = getModule();
  llvm::LLVMContext &C = getLLVMContext();
  std::string EntryName = Implementation->getName().str();
  Implementation->setName(EntryName + ".implementation");
  if (M.getNamedValue(EntryName)) {
    Fail("entry points cannot overload their exported shader name");
    return;
  }
  auto *Entry = llvm::Function::Create(
      llvm::FunctionType::get(llvm::Type::getVoidTy(C), false),
      llvm::GlobalValue::ExternalLinkage, EntryName, M);
  static constexpr const char *StageNames[] = {"compute",
                                               "vertex",
                                               "fragment",
                                               "tessellation_control",
                                               "tessellation_evaluation",
                                               "task",
                                               "mesh",
                                               "ray_generation",
                                               "intersection",
                                               "any_hit",
                                               "closest_hit",
                                               "miss",
                                               "callable"};
  StringRef StageName = StageNames[Stage->getStage()];
  // Preserve backend compatibility with the shared shader infrastructure.
  StringRef BackendStage = llvm::StringSwitch<StringRef>(StageName)
                               .Case("fragment", "pixel")
                               .Case("tessellation_control", "hull")
                               .Case("tessellation_evaluation", "domain")
                               .Case("task", "amplification")
                               .Case("ray_generation", "raygeneration")
                               .Case("any_hit", "anyhit")
                               .Case("closest_hit", "closesthit")
                               .Default(StageName);
  Entry->addFnAttr("hlsl.shader", BackendStage);
  Entry->addFnAttr("spirv.cxx.stage", StageName);
  Entry->addFnAttr("spirv.cxx.source_name", D->getQualifiedNameAsString());
  Entry->addFnAttr(llvm::Attribute::Convergent);
  if (Size)
    Entry->addFnAttr(
        "hlsl.numthreads",
        llvm::formatv("{0},{1},{2}", Size->getX(), Size->getY(), Size->getZ())
            .str());
  CodeGenFunction CGF(*this);
  CGF.StartFunction(GlobalDecl(), Context.VoidTy, Entry,
                    getTypes().arrangeNullaryFunction(), {});
  auto &B = CGF.Builder;
  CallArgList Args;
  unsigned PushConstants = 0;
  unsigned Positions = 0;
  for (unsigned I = 0; I != D->getNumParams(); ++I) {
    const ParmVarDecl *P = D->getParamDecl(I);
    QualType T = P->getType();
    bool IsReference = T->isReferenceType();
    QualType ValueType = IsReference ? T->getPointeeType() : T;
    if (auto *R = ValueType->getAsCXXRecordDecl();
        R && !shaderResource(ValueType) &&
        (!R->isStandardLayout() || !R->isTriviallyCopyable() || R->isUnion() ||
         R->getNumBases())) {
      Fail("entry record values must be trivially copyable standard-layout "
           "records without bases or unions");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    bool IsPointer = ValueType->isPointerType();
    QualType Element = IsPointer ? ValueType->getPointeeType() : ValueType;
    unsigned AS = Context.getTargetAddressSpace(Element.getAddressSpace());
    if (P->hasAttr<SPIRVInputAttr>())
      AS = 7;
    if (P->hasAttr<SPIRVOutputAttr>() || P->hasAttr<SPIRVPositionAttr>()) {
      AS = 8;
      if (!IsPointer) {
        Fail("output parameters must be pointers to interface values");
        CGF.FinishFunction();
        Entry->eraseFromParent();
        return;
      }
    }
    QualType ResourceType = ValueType;
    if (P->getOriginalType()->isArrayType())
      ResourceType = P->getOriginalType();
    auto Resource = shaderResource(ResourceType);
    if (Resource)
      AS = 2;
    if (AS != 7 && AS != 8 && AS != 12 && AS != 13 && !(AS == 2 && Resource)) {
      Fail("entry parameters must be qualified inputs, outputs, uniforms, or "
           "push constants");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    bool Interpolation =
        P->hasAttr<SPIRVFlatAttr>() || P->hasAttr<SPIRVNoPerspectiveAttr>() ||
        P->hasAttr<SPIRVCentroidAttr>() || P->hasAttr<SPIRVSampleAttr>();
    if (P->hasAttr<SPIRVPositionAttr>()) {
      const auto *Array = Context.getAsConstantArrayType(Element);
      if (Stage->getStage() != SPIRVEntryPointAttr::Vertex || ++Positions > 1 ||
          P->hasAttr<SPIRVInputAttr>() || P->hasAttr<SPIRVOutputAttr>() ||
          Interpolation || !Array || Array->getSize() != 4 ||
          !Context.hasSameUnqualifiedType(Array->getElementType(),
                                          Context.FloatTy) ||
          Element.isConstQualified()) {
        Fail("__position__ requires one vertex output pointer to float[4], "
             "without location or interpolation qualifiers");
        CGF.FinishFunction();
        Entry->eraseFromParent();
        return;
      }
    }
    bool Varying =
        (AS == 7 && Stage->getStage() == SPIRVEntryPointAttr::Fragment) ||
        (AS == 8 && Stage->getStage() == SPIRVEntryPointAttr::Vertex);
    if (Interpolation && !Varying) {
      Fail(
          "interpolation qualifiers require vertex outputs or fragment inputs");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    auto ContainsInteger = [&](auto &&Self, QualType Q) -> bool {
      Q = Context.getBaseElementType(Q);
      if (const auto *R = Q->getAsRecordDecl()) {
        for (const auto *F : R->fields())
          if (Self(Self, F->getType()))
            return true;
        return false;
      }
      return Q->isIntegerType();
    };
    auto FloatingOnly = [&](auto &&Self, QualType Q) -> bool {
      Q = Context.getBaseElementType(Q);
      if (const auto *R = Q->getAsRecordDecl()) {
        for (const auto *F : R->fields())
          if (!Self(Self, F->getType()))
            return false;
        return true;
      }
      return Q->isRealFloatingType();
    };
    if (AS == 7 && Stage->getStage() == SPIRVEntryPointAttr::Fragment &&
        ContainsInteger(ContainsInteger, Element) &&
        !P->hasAttr<SPIRVFlatAttr>()) {
      Fail("integer fragment inputs require __flat__");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    if (Interpolation && !P->hasAttr<SPIRVFlatAttr>() &&
        !FloatingOnly(FloatingOnly, Element)) {
      Fail("interpolated inputs and outputs require floating-point values");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    if (AS == 13 && ++PushConstants > 1) {
      Fail("an entry point can have only one push-constant interface");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    auto *Binding = P->getAttr<SPIRVBindingAttr>();
    if ((AS == 12 || AS == 2) && !Binding) {
      Fail("uniform interfaces require __binding__(set, binding)");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    if (Binding && AS != 12 && AS != 2) {
      Fail("__binding__ applies to descriptor resources");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    if (Resource.Name == "descriptor_array") {
      const auto *Array = Context.getAsConstantArrayType(ResourceType);
      if ((Array && Array->getSize().isZero()) ||
          Resource.Count > 0xffffffffULL) {
        Fail("fixed descriptor arrays require a positive 32-bit bound; use [] "
             "for runtime arrays");
        CGF.FinishFunction();
        Entry->eraseFromParent();
        return;
      }
    }
    if (Resource) {
      auto Descriptor = Resource.Name == "descriptor_array"
                            ? shaderResource(Resource.Descriptor)
                            : Resource;
      bool Valid = bool(Descriptor) && Descriptor.Name != "descriptor_array" &&
                   Descriptor.Name != "ray_query";
      if (!Descriptor.Element.isNull()) {
        auto Shape = shaderShape(Context, Descriptor.Element);
        Valid &= bool(Shape) && !Shape.Rows && Shape.Count <= 4 &&
                 Context.getTypeSize(Shape.Element) == 32 &&
                 (Shape.Element->isIntegerType() ||
                  Shape.Element->isRealFloatingType());
      }
      if (Valid && Descriptor.Sampled == 2) {
        unsigned F = Descriptor.Format;
        auto Shape = shaderShape(Context, Descriptor.Element);
        // SPIR-V image formats 1..20 are floating/normalized, 21..29
        // signed integer, and 30..39 unsigned integer. 64-bit formats
        // require a separate API surface and are not accepted here.
        Valid = F < 40 &&
                (F == 0 || (F <= 20 && Shape.Element->isRealFloatingType()) ||
                 (F >= 21 && F <= 29 && Shape.Element->isSignedIntegerType()) ||
                 (F >= 30 && Shape.Element->isUnsignedIntegerType()));
        if (!Valid)
          Fail("storage image format does not match its 32-bit texel type");
      }
      if (!Valid) {
        Fail("native descriptors require a resource with 32-bit scalar or "
             "homogeneous texel components");
        CGF.FinishFunction();
        Entry->eraseFromParent();
        return;
      }
    }
    auto *Ty = Resource ? GetShaderResourceType(ResourceType)
                        : getTypes().ConvertTypeForMem(Element);
    // Keep C++ pointee information separately from the opaque-pointer ABI.
    // The backend consumes this tree after ordinary LLVM optimization.
    auto *GV = new llvm::GlobalVariable(
        M, Ty, false, llvm::GlobalValue::ExternalLinkage, nullptr,
        "__spirv_cxx_" + EntryName + "_arg" + llvm::Twine(I), nullptr,
        llvm::GlobalVariable::NotThreadLocal, AS);
    GV->setAlignment(Resource
                         ? llvm::Align(1)
                         : Context.getTypeAlignInChars(Element).getAsAlign());
    if (Resource)
      GV->setMetadata(
          "spirv.cxx.resource",
          llvm::MDNode::get(
              C, {llvm::MDString::get(C, Resource.Name),
                  integerMD(C, Resource.Dim), integerMD(C, Resource.Arrayed),
                  integerMD(C, Resource.MS), integerMD(C, Resource.Sampled),
                  integerMD(C, Resource.Format), integerMD(C, Resource.Count),
                  llvm::MDString::get(
                      C, Resource.Name == "descriptor_array"
                             ? shaderResource(Resource.Descriptor).Name
                             : "")}));
    if (!Resource)
      GV->setMetadata(
          "spirv.cxx.pointee",
          llvm::MDNode::get(
              C, llvm::ConstantAsMetadata::get(
                     llvm::PoisonValue::get(GetShaderPointeeType(Element)))));
    EmitSPIRVShaderGlobal(P, GV);
    if (Resource.Name == "descriptor_array") {
      // Borrow a descriptor range through an invocation-local carrier. Its
      // private first slot contains the native descriptor-array base handle;
      // subscript lowering creates native resource values from that handle.
      QualType CarrierType = Resource.Descriptor;
      Address Storage = CGF.CreateMemTemp(CarrierType);
      B.CreateStore(GV, CarrierType->isRecordType()
                            ? B.CreateStructGEP(Storage, 0)
                            : Storage);
      llvm::Value *Carrier = Storage.emitRawPointer(CGF);
      if (IsReference && ValueType->isPointerType()) {
        Address PointerSlot = CGF.CreateMemTemp(ValueType);
        B.CreateStore(Carrier, PointerSlot);
        Carrier = PointerSlot.emitRawPointer(CGF);
      }
      Args.add(RValue::get(Carrier), T);
    } else if (Resource && ValueType->isRecordType()) {
      Address Storage = CGF.CreateMemTemp(ValueType);
      B.CreateStore(GV, B.CreateStructGEP(Storage, 0));
      Args.add(IsReference ? RValue::get(Storage.emitRawPointer(CGF))
                           : RValue::getAggregate(Storage),
               T);
    } else if (IsReference) {
      Address Storage = CGF.CreateMemTemp(ValueType);
      B.CreateStore(
          Resource
              ? static_cast<llvm::Value *>(GV)
              : B.CreateLoad(Address(
                    GV, Ty, CharUnits::fromQuantity(GV->getAlign()->value()))),
          Storage);
      Args.add(RValue::get(Storage.emitRawPointer(CGF)), T);
    } else if (IsPointer) {
      Args.add(RValue::get(GV), T);
    } else if (ValueType->isRecordType()) {
      Address Storage = CGF.CreateMemTemp(ValueType);
      B.CreateStore(
          B.CreateLoad(Address(
              GV, Ty, CharUnits::fromQuantity(GV->getAlign()->value()))),
          Storage);
      Args.add(RValue::getAggregate(Storage), T);
    } else {
      Args.add(RValue::get(B.CreateLoad(Address(
                   GV, Ty, CharUnits::fromQuantity(GV->getAlign()->value())))),
               T);
    }
    auto *Info = M.getOrInsertNamedMetadata("spirv.cxx.interfaces");
    Info->addOperand(llvm::MDNode::get(
        C,
        {llvm::MDString::get(C, EntryName), llvm::ValueAsMetadata::get(GV),
         integerMD(C, AS),
         integerMD(C, Resource
                          ? 0
                          : Context.getTypeSizeInChars(Element).getQuantity()),
         integerMD(C,
                   Resource
                       ? 0
                       : Context.getTypeAlignInChars(Element).getQuantity())}));
  }
  llvm::CallBase *Call = nullptr;
  CGF.EmitCall(getTypes().arrangeFunctionDeclaration(GlobalDecl(D)),
               CGCallee::forDirect(Implementation, GlobalDecl(D)),
               ReturnValueSlot(), Args, &Call);
  CGF.FinishFunction();
  if (!M.getNamedMetadata("spirv.MemoryModel"))
    M.getOrInsertNamedMetadata("spirv.MemoryModel")
        ->addOperand(
            llvm::MDNode::get(C, {integerMD(C, 5348), integerMD(C, 3)}));
}

llvm::Type *CodeGenModule::GetShaderPointeeType(QualType Q) {
  auto &C = getLLVMContext();
  const Type *Key = Q.getCanonicalType().getTypePtr();
  if (auto It = ShaderPointeeTypes.find(Key); It != ShaderPointeeTypes.end())
    return It->second;
  if (Q->isPointerType()) {
    QualType E = Q->getPointeeType();
    return llvm::TargetExtType::get(
        C, "spirv.$TypedPointerType", {GetShaderPointeeType(E)},
        {Context.getTargetAddressSpace(E.getAddressSpace())});
  }
  if (auto *A = Context.getAsConstantArrayType(Q))
    return llvm::ArrayType::get(GetShaderPointeeType(A->getElementType()),
                                A->getSize().getZExtValue());
  if (const auto *R = Q->getAsRecordDecl()) {
    getTypes().ConvertTypeForMem(Q);
    const auto &Layout = getTypes().getCGRecordLayout(R);
    auto *Original = Layout.getLLVMType();
    auto *Described = llvm::StructType::create(C);
    ShaderPointeeTypes[Key] = Described;
    SmallVector<llvm::Type *> Fields(Original->elements());
    SmallVector<bool> IsField(Fields.size(), false);
    for (const auto *F : R->fields())
      if (!F->isBitField()) {
        unsigned Index = Layout.getLLVMFieldNo(F);
        IsField[Index] = true;
        Fields[Index] = GetShaderPointeeType(F->getType());
      }
    // ABI padding is not user-visible 8-bit storage. Preserve its byte size
    // using 32-bit words where the original offset and length permit it.
    const auto *Offsets = getDataLayout().getStructLayout(Original);
    for (unsigned I = 0; I != Fields.size(); ++I)
      if (!IsField[I])
        if (auto *Padding = dyn_cast<llvm::ArrayType>(Fields[I]))
          if (Padding->getElementType()->isIntegerTy(8) &&
              Padding->getNumElements() % 4 == 0 &&
              Offsets->getElementOffset(I) % 4 == 0)
            Fields[I] = llvm::ArrayType::get(llvm::Type::getInt32Ty(C),
                                             Padding->getNumElements() / 4);
    Described->setBody(Fields, Original->isPacked());
    return Described;
  }
  return getTypes().ConvertTypeForMem(Q);
}

llvm::Type *CodeGenModule::GetShaderResourceType(QualType T) {
  auto R = shaderResource(T);
  auto &C = getLLVMContext();
  if (R.Name == "descriptor_array")
    return llvm::ArrayType::get(GetShaderResourceType(R.Descriptor), R.Count);
  if (R.Name == "sampler" || R.Name == "sampler_comparison")
    return llvm::TargetExtType::get(C, "spirv.Sampler");
  if (R.Name == "acceleration_structure")
    return llvm::TargetExtType::get(C, "spirv.AccelerationStructureKHR");
  if (R.Name == "ray_query")
    return llvm::TargetExtType::get(C, "spirv.RayQueryKHR");
  auto S = shaderShape(Context, R.Element);
  return llvm::TargetExtType::get(
      C, "spirv.Image", {getTypes().ConvertType(S.Element)},
      {R.Dim, 0, R.Arrayed, R.MS, R.Sampled, R.Format});
}
