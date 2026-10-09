//===-- CGShaderAsm.inc - Direct SPIR-V extended asm ---------------------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

static void
EmitShaderAsm(CodeGenFunction &CGF, CodeGenModule &CGM, const GCCAsmStmt &S,
              llvm::function_ref<llvm::CallBase *(llvm::CallBase *)> Converge) {
  auto &C = CGF.getContext();
  auto &B = CGF.Builder;
  auto &LC = CGM.getLLVMContext();
  auto Error = [&](StringRef Text) { CGM.Error(S.getAsmLoc(), Text); };
  SmallVector<llvm::Value *> Args{B.getInt32(0)};
  SmallVector<llvm::Type *> Results;
  SmallVector<LValue> Destinations;
  SmallVector<SmallVector<Address>> OutputLeaves;
  SmallVector<llvm::Metadata *> Operands;
  SmallVector<unsigned> ReadWrite;
  bool Memory = false;
  auto Number = [&](int V) -> llvm::Metadata * {
    return llvm::ConstantAsMetadata::get(B.getInt32(V));
  };
  std::function<void(Address, QualType, SmallVectorImpl<Address> &)> Leaves =
      [&](Address A, QualType T, SmallVectorImpl<Address> &Out) {
        if (auto *AT = C.getAsConstantArrayType(T)) {
          for (unsigned I = 0; I < AT->getSize().getZExtValue(); ++I)
            Leaves(B.CreateConstArrayGEP(A, I), AT->getElementType(), Out);
        } else if (auto *R = T->getAsCXXRecordDecl()) {
          const auto &Layout = CGM.getTypes().getCGRecordLayout(R);
          for (auto *F : R->fields())
            Leaves(B.CreateStructGEP(A, Layout.getLLVMFieldNo(F)), F->getType(),
                   Out);
        } else {
          Out.push_back(A);
        }
      };
  auto LoadVector = [&](const Expr *E) -> llvm::Value * {
    Address A = E->isLValue() ? CGF.EmitLValue(E).getAddress()
                              : Address(CGF.CreateMemTemp(E->getType()));
    if (!E->isLValue())
      CGF.EmitAnyExprToMem(E, A, Qualifiers(), true);
    SmallVector<Address> Addresses;
    Leaves(A, E->getType(), Addresses);
    llvm::Value *V = llvm::PoisonValue::get(llvm::FixedVectorType::get(
        CGF.ConvertType(shaderShape(C, E->getType()).Element),
        Addresses.size()));
    for (unsigned I = 0; I < Addresses.size(); ++I) {
      auto *Load = B.CreateLoad(Addresses[I]);
      Load->setVolatile(E->getType().isVolatileQualified());
      V = B.CreateInsertElement(V, Load, B.getInt32(I));
    }
    return V;
  };
  unsigned Count = S.getNumOutputs() + S.getNumInputs();
  for (unsigned I = 0; I < Count; ++I) {
    bool Output = I < S.getNumOutputs();
    const Expr *E =
        Output ? S.getOutputExpr(I) : S.getInputExpr(I - S.getNumOutputs());
    std::string Constraint = Output
                                 ? S.getOutputConstraint(I)
                                 : S.getInputConstraint(I - S.getNumOutputs());
    bool RW = Output && StringRef(Constraint).starts_with("+");
    StringRef Kind(Constraint);
    if (Output)
      Kind = Kind.drop_front();
    if (Kind.starts_with("&"))
      Kind = Kind.drop_front();
    if (!Output && (Kind.starts_with("[") || isDigit(Kind.front()))) {
      unsigned Tied = 0;
      if (Kind.starts_with("[")) {
        int N = S.getNamedOperand(Kind.drop_front().drop_back());
        if (N < 0 || unsigned(N) >= S.getNumOutputs()) {
          Error("invalid tied SPIR-V asm operand");
          return;
        }
        Tied = N;
      } else if (Kind.getAsInteger(10, Tied) || Tied >= S.getNumOutputs()) {
        Error("invalid tied SPIR-V asm operand");
        return;
      }
      Constraint = S.getOutputConstraint(Tied);
      Kind = StringRef(Constraint).drop_front();
      if (Kind.starts_with("&"))
        Kind = Kind.drop_front();
    }
    if (Kind != "r" && Kind != "v" && Kind != "m" &&
        !(Kind == "i" && !Output)) {
      Error("SPIR-V asm constraints must be r, v, m or an input i");
      return;
    }
    int Arg = -1, Result = -1;
    unsigned Alignment = 0;
    std::string Literal;
    llvm::Metadata *Pointee = llvm::MDString::get(LC, "");
    if (Kind == "i") {
      Expr::EvalResult Evaluation;
      if (!E->EvaluateAsInt(Evaluation, C)) {
        Error("SPIR-V asm i operands must be integer constant expressions");
        return;
      }
      Literal = llvm::toString(Evaluation.Val.getInt(), 10);
      Arg = Args.size();
      Args.push_back(CGF.EmitScalarExpr(E));
    } else if (Kind == "m") {
      Address A = CGF.EmitLValue(E).getAddress();
      Arg = Args.size();
      auto *Pointer = A.emitRawPointer(CGF);
      Pointee = llvm::ConstantAsMetadata::get(
          llvm::PoisonValue::get(A.getElementType()));
      Args.push_back(Pointer);
      Alignment = A.getAlignment().getQuantity();
      Memory = true;
    } else {
      auto Shape = shaderShape(C, E->getType());
      if ((Kind == "r" && !E->getType()->isScalarType()) ||
          (Kind == "v" &&
           (!Shape || Shape.Rows || Shape.Count < 2 || Shape.Count > 4))) {
        Error("SPIR-V asm r requires a scalar/pointer; v requires 2-4 "
              "homogeneous components");
        return;
      }
      if (Output) {
        LValue LV = CGF.EmitLValue(E);
        Destinations.push_back(LV);
        SmallVector<Address> Addresses;
        if (Kind == "v")
          Leaves(LV.getAddress(), E->getType(), Addresses);
        OutputLeaves.push_back(std::move(Addresses));
        Result = Results.size();
        Results.push_back(
            Kind == "r" ? CGF.ConvertType(E->getType())
                        : llvm::FixedVectorType::get(
                              CGF.ConvertType(Shape.Element), Shape.Count));
      }
      if (!Output || RW) {
        Arg = Args.size();
        if (Output && RW) {
          if (Kind == "r") {
            Args.push_back(
                CGF.EmitLoadOfLValue(Destinations.back(), E->getExprLoc())
                    .getScalarVal());
          } else {
            const auto &Addresses = OutputLeaves.back();
            llvm::Value *V = llvm::PoisonValue::get(Results.back());
            for (unsigned J = 0; J < Addresses.size(); ++J) {
              auto *Load = B.CreateLoad(Addresses[J]);
              Load->setVolatile(Destinations.back().isVolatileQualified());
              V = B.CreateInsertElement(V, Load, B.getInt32(J));
            }
            Args.push_back(V);
          }
        } else {
          Args.push_back(Kind == "r" ? CGF.EmitScalarExpr(E) : LoadVector(E));
        }
      }
    }
    Operands.push_back(llvm::MDNode::get(
        LC, {llvm::MDString::get(LC, Kind), Number(Arg), Number(Result),
             Number(Alignment), llvm::MDString::get(LC, Literal), Pointee}));
    if (RW)
      ReadWrite.push_back(I);
  }
  for (unsigned I : ReadWrite)
    Operands.push_back(Operands[I]);
  for (unsigned I = 0; I < S.getNumClobbers(); ++I) {
    if (S.getClobber(I) != "memory") {
      Error("SPIR-V asm supports only the memory clobber");
      return;
    }
    Memory = true;
  }
  llvm::Type *Return = Results.empty() ? B.getInt32Ty()
                       : Results.size() == 1
                           ? Results[0]
                           : llvm::StructType::get(LC, Results);
  SmallVector<llvm::Type *> Types;
  std::string Signature;
  llvm::raw_string_ostream OS(Signature);
  Return->print(OS);
  for (auto *A : Args) {
    Types.push_back(A->getType());
    A->getType()->print(OS);
  }
  OS << S.generateAsmString(C);
  for (unsigned I = 0; I < S.getNumOutputs(); ++I)
    OS << S.getOutputConstraint(I);
  for (unsigned I = 0; I < S.getNumInputs(); ++I)
    OS << S.getInputConstraint(I);
  OS.flush();
  uint64_t Hash = 14695981039346656037ULL;
  for (unsigned char Ch : Signature)
    Hash = (Hash ^ Ch) * 1099511628211ULL;
  std::string Base = "__spirv_shader_InlineAsm";
  auto Callee = CGM.getModule().getOrInsertFunction(
      "_Z" + std::to_string(Base.size()) + Base + "ILy" + std::to_string(Hash) +
          "EEvv",
      llvm::FunctionType::get(Return, Types, false));
  auto *F = cast<llvm::Function>(Callee.getCallee());
  F->addFnAttr("spirv.cxx.intrinsic", "InlineAsm");
  F->addFnAttr(llvm::Attribute::NoUnwind);
  F->addFnAttr(llvm::Attribute::Convergent);
  auto *Call = B.CreateCall(Callee, Args);
  Call->setConvergent();
  Call = cast<llvm::CallInst>(Converge(Call));
  if (!S.isVolatile() && !Memory && !Results.empty())
    Call->setMemoryEffects(llvm::MemoryEffects::none());
  Call->setMetadata(
      "spirv.cxx.asm",
      llvm::MDNode::get(LC, {llvm::MDString::get(LC, S.generateAsmString(C)),
                             llvm::MDNode::get(LC, Operands),
                             Number(S.getNumOutputs())}));
  for (unsigned I = 0; I < Results.size(); ++I) {
    llvm::Value *V = Results.size() == 1 ? Call : B.CreateExtractValue(Call, I);
    if (OutputLeaves[I].empty()) {
      CGF.EmitStoreThroughLValue(RValue::get(V), Destinations[I]);
    } else {
      for (unsigned J = 0; J < OutputLeaves[I].size(); ++J) {
        auto *Store = B.CreateStore(B.CreateExtractElement(V, B.getInt32(J)),
                                    OutputLeaves[I][J]);
        Store->setVolatile(Destinations[I].isVolatileQualified());
      }
    }
  }
}
