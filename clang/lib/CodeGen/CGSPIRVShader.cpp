//===-- CGSPIRVShader.cpp - Native C++ shader entry interfaces
//---------------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "CodeGenModule.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/RecordLayout.h"
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
                          unsigned Value) {
  auto &C = GV->getContext();
  SmallVector<llvm::Metadata *> Nodes;
  if (auto *Old = GV->getMetadata("spirv.Decorations"))
    for (const auto &Op : Old->operands())
      Nodes.push_back(Op.get());
  Nodes.push_back(
      llvm::MDNode::get(C, {integerMD(C, Decoration), integerMD(C, Value)}));
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
  if (Implementation->arg_size() != D->getNumParams()) {
    Fail("entry parameters must have a direct scalar or pointer ABI");
    return;
  }
  llvm::Module &M = getModule();
  llvm::LLVMContext &C = getLLVMContext();
  std::string EntryName = D->getNameAsString();
  if (Implementation->getName() == EntryName)
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
  Entry->addFnAttr(llvm::Attribute::Convergent);
  if (Size)
    Entry->addFnAttr(
        "hlsl.numthreads",
        llvm::formatv("{0},{1},{2}", Size->getX(), Size->getY(), Size->getZ())
            .str());
  llvm::IRBuilder<> B(llvm::BasicBlock::Create(C, "entry", Entry));
  SmallVector<llvm::Value *> Args;
  unsigned PushConstants = 0;
  for (unsigned I = 0; I != D->getNumParams(); ++I) {
    const ParmVarDecl *P = D->getParamDecl(I);
    QualType T = P->getType();
    bool IsPointer = T->isPointerType();
    QualType Element = IsPointer ? T->getPointeeType() : T;
    unsigned AS = Context.getTargetAddressSpace(Element.getAddressSpace());
    if (P->hasAttr<SPIRVInputAttr>())
      AS = 7;
    if (P->hasAttr<SPIRVOutputAttr>()) {
      AS = 8;
      if (!IsPointer) {
        Fail("output parameters must be pointers to interface values");
        Entry->eraseFromParent();
        return;
      }
    }
    if (AS != 7 && AS != 8 && AS != 12 && AS != 13) {
      Fail("entry parameters must be qualified inputs, outputs, uniforms, or "
           "push constants");
      Entry->eraseFromParent();
      return;
    }
    if (AS == 13 && ++PushConstants > 1) {
      Fail("an entry point can have only one push-constant interface");
      Entry->eraseFromParent();
      return;
    }
    auto *Binding = P->getAttr<SPIRVBindingAttr>();
    if (AS == 12 && !Binding) {
      Fail("uniform interfaces require __binding__(set, binding)");
      Entry->eraseFromParent();
      return;
    }
    if (Binding && AS != 12) {
      Fail("__binding__ applies to descriptor resources");
      Entry->eraseFromParent();
      return;
    }
    auto *Ty = getTypes().ConvertTypeForMem(Element);
    auto *GV = new llvm::GlobalVariable(
        M, Ty, false, llvm::GlobalValue::ExternalLinkage, nullptr,
        "__spirv_cxx_" + EntryName + "_arg" + llvm::Twine(I), nullptr,
        llvm::GlobalVariable::NotThreadLocal, AS);
    GV->setAlignment(Context.getTypeAlignInChars(Element).getAsAlign());
    EmitSPIRVShaderGlobal(P, GV);
    if (IsPointer)
      Args.push_back(GV);
    else
      Args.push_back(B.CreateLoad(Ty, GV));
    auto *Info = M.getOrInsertNamedMetadata("spirv.cxx.interfaces");
    Info->addOperand(llvm::MDNode::get(
        C, {llvm::MDString::get(C, EntryName), llvm::ValueAsMetadata::get(GV),
            integerMD(C, AS),
            integerMD(C, Context.getTypeSizeInChars(Element).getQuantity()),
            integerMD(C, Context.getTypeAlignInChars(Element).getQuantity())}));
  }
  auto *Convergence = B.CreateCall(llvm::Intrinsic::getOrInsertDeclaration(
      &M, llvm::Intrinsic::experimental_convergence_entry));
  llvm::OperandBundleDef Bundle("convergencectrl", Convergence);
  auto *Call = B.CreateCall(Implementation, Args, {Bundle});
  Call->setCallingConv(Implementation->getCallingConv());
  Call->setAttributes(Implementation->getAttributes());
  B.CreateRetVoid();
  if (!M.getNamedMetadata("spirv.MemoryModel"))
    M.getOrInsertNamedMetadata("spirv.MemoryModel")
        ->addOperand(
            llvm::MDNode::get(C, {integerMD(C, 5348), integerMD(C, 3)}));
}
