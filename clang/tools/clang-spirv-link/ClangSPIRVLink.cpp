//===-- ClangSPIRVLink.cpp - C++ shader linking and legalization ----------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/ScopeExit.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/IR/ReplaceConstant.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Linker/Linker.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/KnownBits.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/GlobalDCE.h"
#include "llvm/Transforms/Scalar/SROA.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Local.h"
#include <cstdint>
#include <set>

using namespace llvm;
static cl::list<std::string> Inputs(cl::Positional, cl::OneOrMore,
                                    cl::desc("C++ shader bitcode inputs"));
static cl::opt<std::string> Output("o", cl::Required,
                                   cl::desc("Output SPIR-V"));
static cl::opt<std::string> EntryName("entry",
                                      cl::desc("Selected entry point"));
static cl::opt<std::string> Validator("validator", cl::desc("spirv-val path"));
static cl::opt<bool> EmitLLVM("emit-llvm", cl::desc("Emit legalized LLVM IR"));
static cl::opt<unsigned> CodeGenOptimizationLevel(
    "codegen-opt-level", cl::desc("SPIR-V code generation optimization level"),
    cl::init(2));

static bool fail(const Twine &Message) {
  errs() << "clang-spirv-link: error: " << Message << '\n';
  return false;
}
static Metadata *number(LLVMContext &C, unsigned V) {
  return ConstantAsMetadata::get(ConstantInt::get(Type::getInt32Ty(C), V));
}
static unsigned mdNumber(const MDNode *Node, unsigned Index) {
  return mdconst::extract<ConstantInt>(Node->getOperand(Index))->getZExtValue();
}
static void decorate(GlobalVariable *G, unsigned Kind, unsigned Value) {
  auto &C = G->getContext();
  G->setMetadata(
      "spirv.Decorations",
      MDNode::get(C, MDNode::get(C, {number(C, Kind), number(C, Value)})));
}

static bool checkCalls(Function &F, DenseSet<Function *> &Active,
                       DenseSet<Function *> &Done) {
  if (Done.contains(&F))
    return true;
  if (!Active.insert(&F).second)
    return fail("runtime C++ recursion involving '" + F.getName() + "'");
  for (Instruction &I : instructions(F)) {
    auto *Call = dyn_cast<CallBase>(&I);
    if (!Call)
      continue;
    if (Call->isInlineAsm())
      return fail(
          "unexpected raw LLVM inline assembly; compile shader asm with -fspirv-shader");
    auto *Callee =
        dyn_cast<Function>(Call->getCalledOperand()->stripPointerCasts());
    if (!Callee)
      return fail("unresolved indirect call in '" + F.getName() + "'");
    if (Callee->isIntrinsic() || Callee->hasFnAttribute("spirv.cxx.intrinsic"))
      continue;
    if (Callee->isDeclaration())
      return fail("unresolved shader function '" + Callee->getName() + "'");
    if (!checkCalls(*Callee, Active, Done))
      return false;
  }
  Active.erase(&F);
  Done.insert(&F);
  return true;
}

static bool materializeInvocationValues(Module &M, Function &Entry) {
  IRBuilder<> B(&*Entry.getEntryBlock().getFirstInsertionPt());
  SmallVector<GlobalVariable *> Globals;
  for (auto &G : M.globals())
    if (G.getAddressSpace() == 0 && !G.use_empty())
      Globals.push_back(&G);
  for (GlobalVariable *G : Globals) {
    convertUsersOfConstantsToInstructions({G});
    for (User *Use : G->users())
      if (!isa<Instruction>(Use) ||
          cast<Instruction>(Use)->getFunction() != &Entry)
        return fail("global address escapes invocation storage: '" +
                    G->getName() + "'");
    Type *T = G->getValueType();
    auto *Snapshot = B.CreateAlloca(T, nullptr, G->getName() + ".invocation");
    Snapshot->setAlignment(G->getAlign().valueOrOne());
    if (auto *Builtin = G->getMetadata("spirv.cxx.builtin")) {
      unsigned ID = mdNumber(Builtin, 0);
      StringRef Stage =
          Entry.getFnAttribute("spirv.cxx.stage").getValueAsString();
      bool Allowed =
          ID == 36 || ID == 41 ||
          (Stage == "compute" &&
           ((ID >= 24 && ID <= 29) || ID == 38 || ID == 40)) ||
          (Stage == "vertex" &&
           (ID == 42 || ID == 43 || ID == 4424 || ID == 4425 || ID == 4426)) ||
          (Stage == "fragment" &&
           (ID == 7 || (ID >= 15 && ID <= 19) || ID == 23)) ||
          ((Stage == "vertex" || Stage == "fragment") && ID == 4440);
      if (!Allowed)
        return fail("builtin '" + G->getName() + "' is unavailable in " +
                    Stage + " entry '" + Entry.getName() + "'");
      auto *Array = dyn_cast<ArrayType>(T);
      // Tessellation levels are arrays; other scope arrays expose vector
      // inputs.
      bool Vector = Array && ID != 11 && ID != 12;
      Type *InterfaceType =
          Vector ? static_cast<Type *>(FixedVectorType::get(
                       Array->getElementType(), Array->getNumElements()))
                 : T;
      if (ID == 17 || ID == 23)
        InterfaceType = Type::getInt1Ty(M.getContext());
      Value *V = nullptr;
      if (ID == 25) {
        SmallVector<Constant *> Dimensions;
        SmallVector<StringRef> Parts;
        Entry.getFnAttribute("hlsl.numthreads")
            .getValueAsString()
            .split(Parts, ',');
        if (Parts.size() != 3)
          return fail("workgroup_size is only available in workgroup stages");
        for (StringRef Part : Parts) {
          unsigned Dim;
          if (Part.getAsInteger(10, Dim))
            return fail("invalid workgroup dimensions");
          Dimensions.push_back(ConstantInt::get(Array->getElementType(), Dim));
        }
        V = ConstantVector::get(Dimensions);
      } else {
        auto *Input = new GlobalVariable(M, InterfaceType, false,
                                         GlobalValue::ExternalLinkage, nullptr,
                                         G->getName() + ".input", nullptr,
                                         GlobalVariable::NotThreadLocal, 7);
        decorate(Input, 11, ID);
        if (Entry.getFnAttribute("spirv.cxx.stage").getValueAsString() ==
                "fragment" &&
            InterfaceType->isIntegerTy() && !InterfaceType->isIntegerTy(1))
          Input->setMetadata(
              "spirv.Decorations",
              MDNode::get(
                  M.getContext(),
                  {MDNode::get(M.getContext(), {number(M.getContext(), 11),
                                                number(M.getContext(), ID)}),
                   MDNode::get(M.getContext(), {number(M.getContext(), 14)})}));
        V = B.CreateLoad(InterfaceType, Input);
        if (InterfaceType != T && !Vector)
          V = B.CreateZExt(V, T);
      }
      if (Vector) {
        for (unsigned I = 0; I != Array->getNumElements(); ++I) {
          auto *Element =
              B.CreateInBoundsGEP(T, Snapshot, {B.getInt32(0), B.getInt32(I)});
          B.CreateStore(B.CreateExtractElement(V, I), Element);
        }
      } else {
        B.CreateStore(V, Snapshot);
      }
    } else if (G->hasInitializer()) {
      B.CreateStore(G->getInitializer(), Snapshot);
    } else {
      return fail("unresolved external shader variable '" + G->getName() + "'");
    }
    G->replaceAllUsesWith(Snapshot);
    G->eraseFromParent();
  }
  return true;
}

static json::Object typeLayout(Type *T, const DataLayout &DL,
                               unsigned Depth = 0) {
  json::Object R{
      {"size", static_cast<int64_t>(DL.getTypeAllocSize(T))},
      {"alignment", static_cast<int64_t>(DL.getABITypeAlign(T).value())}};
  if (auto *P = dyn_cast<PointerType>(T)) {
    R["kind"] = P->getAddressSpace() == 14 ? "device_address" : "pointer";
    R["address_space"] = P->getAddressSpace();
  } else if (auto *S = dyn_cast<StructType>(T)) {
    R["kind"] = "record";
    if (S->hasName())
      R["name"] = S->getName().str();
    json::Array Fields;
    if (Depth < 16) {
      const StructLayout *Layout = DL.getStructLayout(S);
      for (unsigned I = 0; I != S->getNumElements(); ++I) {
        auto Field = typeLayout(S->getElementType(I), DL, Depth + 1);
        Field["offset"] = static_cast<int64_t>(Layout->getElementOffset(I));
        Fields.push_back(std::move(Field));
      }
    }
    R["members"] = std::move(Fields);
  } else if (auto *A = dyn_cast<ArrayType>(T)) {
    R["kind"] = "array";
    R["count"] = static_cast<int64_t>(A->getNumElements());
    R["stride"] =
        static_cast<int64_t>(DL.getTypeAllocSize(A->getElementType()));
    if (Depth < 16)
      R["element"] = typeLayout(A->getElementType(), DL, Depth + 1);
  } else {
    R["kind"] = T->isFloatingPointTy() ? "float" : "integer";
    R["bits"] = static_cast<int64_t>(T->getPrimitiveSizeInBits());
  }
  return R;
}

static json::Array reflectInterfaces(Module &M) {
  json::Array Interfaces;
  for (GlobalVariable &G : M.globals()) {
    unsigned AS = G.getAddressSpace();
    if (AS != 2 && AS != 7 && AS != 8 && AS != 12 && AS != 13)
      continue;
    Type *SourceType = G.getValueType();
    if (auto *N = G.getMetadata("spirv.cxx.source_interface"))
      SourceType = cast<ValueAsMetadata>(N->getOperand(0))->getType();
    auto R = AS == 2 ? json::Object{} : typeLayout(SourceType, M.getDataLayout());
    if (auto Alignment = G.getAlign())
      R["alignment"] = static_cast<int64_t>(Alignment->value());
    R["name"] = G.getName().str();
    if (auto *Info = G.getMetadata("spirv.cxx.resource")) {
      R["resource_type"] =
          cast<MDString>(Info->getOperand(0))->getString().str();
      R["dimension"] = mdNumber(Info, 1);
      R["arrayed"] = bool(mdNumber(Info, 2));
      R["multisampled"] = bool(mdNumber(Info, 3));
      R["sampled"] = mdNumber(Info, 4);
      R["image_format"] = mdNumber(Info, 5);
      if (Info->getNumOperands() > 6 &&
          cast<MDString>(Info->getOperand(0))->getString() ==
              "descriptor_array") {
        R["descriptor_count"] = mdNumber(Info, 6);
        R["element_resource_type"] =
            cast<MDString>(Info->getOperand(7))->getString().str();
      }
    }
    R["kind"] = AS == 2    ? "descriptor"
                : AS == 7  ? "input"
                : AS == 8  ? "output"
                : AS == 12 ? "uniform"
                           : "push_constant";
    if (auto *D = G.getMetadata("spirv.Decorations"))
      for (const auto &Op : D->operands()) {
        auto *Node = cast<MDNode>(Op.get());
        unsigned Decoration = mdNumber(Node, 0);
        const char *Name = Decoration == 30   ? "location"
                           : Decoration == 33 ? "binding"
                           : Decoration == 34 ? "set"
                           : Decoration == 11 ? "builtin"
                                              : nullptr;
        if (Name)
          R[Name] = mdNumber(Node, 1);
        if (Decoration == 13 || Decoration == 14)
          R["interpolation"] = Decoration == 14 ? "flat" : "noperspective";
        if (Decoration == 16 || Decoration == 17)
          R["interpolation_sampling"] =
              Decoration == 16 ? "centroid" : "sample";
      }
    Interfaces.push_back(std::move(R));
  }
  return Interfaces;
}

static bool canonicalizeSpecializations(Function &Entry) {
  DenseMap<unsigned, CallInst *> Constants;
  SmallVector<CallInst *> Calls;
  for (Instruction &I : instructions(Entry))
    if (auto *Call = dyn_cast<CallInst>(&I))
      if (auto *F = Call->getCalledFunction();
          F && F->getFnAttribute("spirv.cxx.intrinsic").getValueAsString() ==
                   "SpecConstant")
        Calls.push_back(Call);
  auto Insert = Entry.getEntryBlock().getFirstInsertionPt();
  for (auto *Call : Calls) {
    unsigned ID = cast<ConstantInt>(Call->getArgOperand(0))->getZExtValue();
    auto [It, Added] = Constants.try_emplace(ID, Call);
    if (Added) {
      Call->moveBefore(Entry.getEntryBlock(), Insert);
    } else {
      auto *First = It->second;
      if (First->getArgOperand(1) != Call->getArgOperand(1) ||
          First->getMetadata("spirv.cxx.specialization.type") !=
              Call->getMetadata("spirv.cxx.specialization.type"))
        return fail("conflicting specialization constant ID " + Twine(ID));
      Call->replaceAllUsesWith(First);
      Call->eraseFromParent();
    }
  }
  return true;
}

static json::Array reflectSpecializations(Function &Entry) {
  json::Array Result;
  for (Instruction &I : instructions(Entry)) {
    auto *Call = dyn_cast<CallInst>(&I);
    if (!Call)
      continue;
    auto *Type = Call->getMetadata("spirv.cxx.specialization.type");
    if (!Type)
      continue;
    json::Object R{
        {"id", cast<ConstantInt>(Call->getArgOperand(0))->getZExtValue()},
        {"type", cast<MDString>(Type->getOperand(0))->getString().str()}};
    auto *Default = Call->getArgOperand(1);
    if (auto *V = dyn_cast<ConstantInt>(Default)) {
      R["bit_width"] = V->getBitWidth();
      SmallString<32> Bits;
      V->getValue().toString(Bits, 16, false);
      R["default_bits"] = Bits.str().str();
      if (V->getBitWidth() == 1)
        R["default"] = !V->isZero();
      else if (cast<MDString>(Type->getOperand(0))
                   ->getString()
                   .contains("unsigned"))
        R["default"] = V->getZExtValue();
      else
        R["default"] = V->getSExtValue();
    } else {
      auto &FP = cast<ConstantFP>(Default)->getValueAPF();
      if (FP.isFinite())
        R["default"] = FP.convertToDouble();
      auto Bits = FP.bitcastToAPInt();
      R["bit_width"] = Bits.getBitWidth();
      SmallString<32> Text;
      Bits.toString(Text, 16, false);
      R["default_bits"] = Text.str().str();
    }
    Result.push_back(std::move(R));
  }
  return Result;
}

// Function pointers cannot participate in SPIR-V OpPhi without variable
// pointers. Keep finite local addresses as scalar base tags and byte offsets,
// then reconstruct and branch at each memory access.
static bool lowerLocalPointerPhis(Function &F) {
  SmallVector<PHINode *> Phis;
  for (Instruction &I : instructions(F))
    if (auto *P = dyn_cast<PHINode>(&I))
      if (P->getType()->isPointerTy() &&
          P->getType()->getPointerAddressSpace() == 0)
        Phis.push_back(P);
  if (Phis.empty())
    return true;
  const DataLayout &DL = F.getParent()->getDataLayout();
  SmallVector<AllocaInst *> Bases;
  DenseMap<Value *, std::pair<Value *, Value *>> Encoded;
  DenseSet<Value *> Affected;
  SmallVector<Value *> Work(Phis.begin(), Phis.end());
  while (!Work.empty()) {
    Value *V = Work.pop_back_val();
    if (!Affected.insert(V).second)
      continue;
    for (User *U : V->users())
      if (isa<GetElementPtrInst, SelectInst, PHINode>(U))
        Work.push_back(U);
  }
  bool Valid = true;
  std::function<std::pair<Value *, Value *>(Value *)> Encode = [&](Value *V) {
    if (auto It = Encoded.find(V); It != Encoded.end())
      return It->second;
    IRBuilder<> B(F.getContext());
    std::pair<Value *, Value *> R;
    if (auto *A = dyn_cast<AllocaInst>(V)) {
      auto It = llvm::find(Bases, A);
      unsigned Tag = It == Bases.end() ? Bases.size() : It - Bases.begin();
      if (It == Bases.end())
        Bases.push_back(A);
      R = {B.getInt32(Tag), B.getInt64(0)};
    } else if (auto *P = dyn_cast<PHINode>(V)) {
      auto *Tag = PHINode::Create(B.getInt32Ty(), P->getNumIncomingValues(),
                                  "local.base", P->getIterator());
      auto *Offset = PHINode::Create(B.getInt64Ty(), P->getNumIncomingValues(),
                                     "local.offset", P->getIterator());
      Encoded[V] = {Tag, Offset};
      for (unsigned I = 0; I != P->getNumIncomingValues(); ++I) {
        auto Incoming = Encode(P->getIncomingValue(I));
        Tag->addIncoming(Incoming.first, P->getIncomingBlock(I));
        Offset->addIncoming(Incoming.second, P->getIncomingBlock(I));
      }
      return Encoded[V];
    } else if (auto *G = dyn_cast<GetElementPtrInst>(V)) {
      auto Parent = Encode(G->getPointerOperand());
      B.SetInsertPoint(G);
      SmallMapVector<Value *, APInt, 4> Variables;
      APInt Constant(64, 0);
      if (!G->collectOffset(DL, 64, Variables, Constant))
        Valid = false;
      Value *Offset = B.getInt64(Constant.getZExtValue());
      for (auto &[Index, Stride] : Variables)
        Offset = B.CreateAdd(
            Offset, B.CreateMul(B.CreateSExtOrTrunc(Index, B.getInt64Ty()),
                                B.getInt64(Stride.getZExtValue())));
      R = {Parent.first, B.CreateAdd(Parent.second, Offset)};
    } else if (auto *S = dyn_cast<SelectInst>(V)) {
      auto T = Encode(S->getTrueValue()), E = Encode(S->getFalseValue());
      B.SetInsertPoint(S);
      R = {B.CreateSelect(S->getCondition(), T.first, E.first),
           B.CreateSelect(S->getCondition(), T.second, E.second)};
    } else {
      Valid = false;
      R = {B.getInt32(0), B.getInt64(0)};
    }
    Encoded[V] = R;
    return R;
  };
  for (auto *P : Phis)
    Encode(P);
  SmallVector<ICmpInst *> Comparisons;
  for (Instruction &I : instructions(F))
    if (auto *Compare = dyn_cast<ICmpInst>(&I))
      if (Affected.contains(Compare->getOperand(0)) ||
          Affected.contains(Compare->getOperand(1))) {
        Encode(Compare->getOperand(0));
        Encode(Compare->getOperand(1));
        Comparisons.push_back(Compare);
      }
  SmallVector<Instruction *> Memory;
  for (Instruction &I : instructions(F)) {
    Value *Address = isa<LoadInst>(I) ? cast<LoadInst>(I).getPointerOperand()
                     : isa<StoreInst>(I)
                         ? cast<StoreInst>(I).getPointerOperand()
                         : nullptr;
    if (Address && Affected.contains(Address)) {
      Encode(Address);
      Memory.push_back(&I);
    }
  }
  for (Value *V : Affected)
    for (User *U : V->users())
      if (!Affected.contains(U) && !is_contained(Memory, U) &&
          !is_contained(Comparisons, U))
        Valid = false;
  if (!Valid)
    return fail(
        "local pointer loop escapes or has an unsupported address base");
  for (auto *Compare : Comparisons) {
    auto A = Encoded[Compare->getOperand(0)];
    auto E = Encoded[Compare->getOperand(1)];
    IRBuilder<> B(Compare);
    Value *SameBase = B.CreateICmpEQ(A.first, E.first);
    Value *Offsets = B.CreateICmp(Compare->getPredicate(), A.second, E.second);
    Value *Result;
    if (Compare->getPredicate() == CmpInst::ICMP_EQ)
      Result = B.CreateAnd(SameBase, Offsets);
    else if (Compare->getPredicate() == CmpInst::ICMP_NE)
      Result = B.CreateOr(B.CreateNot(SameBase), Offsets);
    else
      Result = B.CreateSelect(SameBase, Offsets,
                              B.CreateICmp(Compare->getPredicate(), A.first, E.first));
    Compare->replaceAllUsesWith(Result);
    Compare->eraseFromParent();
  }
  for (Instruction *I : Memory) {
    unsigned Operand = isa<LoadInst>(I) ? 0 : 1;
    auto [Tag, Offset] = Encoded[I->getOperand(Operand)];
    IRBuilder<> B(I);
    SmallVector<Value *> Addresses;
    Type *AccessType = isa<LoadInst>(I)
                           ? I->getType()
                           : cast<StoreInst>(I)->getValueOperand()->getType();
    for (AllocaInst *A : Bases) {
      Type *T = A->getAllocatedType();
      Value *Remaining = Offset;
      SmallVector<Value *> Indices{B.getInt32(0)};
      while (T != AccessType) {
        if (auto *Array = dyn_cast<ArrayType>(T)) {
          T = Array->getElementType();
          uint64_t Stride = DL.getTypeAllocSize(T);
          Indices.push_back(B.CreateSDiv(Remaining, B.getInt64(Stride)));
          Remaining = B.CreateSRem(Remaining, B.getInt64(Stride));
        } else if (auto *S = dyn_cast<StructType>(T)) {
          if (S->getNumElements() == 1 &&
              DL.getStructLayout(S)->getElementOffset(0) == 0) {
            Indices.push_back(B.getInt32(0));
            T = S->getElementType(0);
            continue;
          }
          // A dynamic offset into a heterogeneous record requires a separate
          // field dispatch. Reject it rather than silently choosing member 0.
          auto *C = dyn_cast<ConstantInt>(Remaining);
          if (!C)
            return fail("local pointer loop has a dynamic record-field offset");
          auto *Layout = DL.getStructLayout(S);
          unsigned Field =
              Layout->getElementContainingOffset(C->getZExtValue());
          Indices.push_back(B.getInt32(Field));
          Remaining =
              B.getInt64(C->getZExtValue() - Layout->getElementOffset(Field));
          T = S->getElementType(Field);
        } else {
          return fail("local pointer loop has incompatible pointee types");
        }
      }
      Addresses.push_back(B.CreateGEP(A->getAllocatedType(), A, Indices));
    }
    Value *Address = Addresses.back();
    for (unsigned N = Addresses.size() - 1; N != 0; --N)
      Address = B.CreateSelect(B.CreateICmpEQ(Tag, B.getInt32(N - 1)),
                               Addresses[N - 1], Address);
    I->setOperand(Operand, Address);
  }
  // Remove the pointer SSA cycle only after all memory uses have been replaced.
  SmallVector<Instruction *> Dead;
  for (Value *V : Affected)
    Dead.push_back(cast<Instruction>(V));
  for (Instruction *I : Dead)
    I->dropAllReferences();
  for (Instruction *I : Dead)
    I->eraseFromParent();
  return true;
}

// A conditional logical address is lowered at its dereference. Reading both
// alternatives speculatively is incorrect for volatile or inaccessible memory.
static bool lowerLogicalSelections(Function &F) {
  bool Changed;
  do {
    Changed = false;
    SmallVector<Instruction *> Memory;
    for (Instruction &I : instructions(F))
      if (isa<LoadInst, StoreInst>(I))
        Memory.push_back(&I);
    for (Instruction *I : Memory) {
      Value *Address = isa<LoadInst>(I)
                           ? cast<LoadInst>(I)->getPointerOperand()
                           : cast<StoreInst>(I)->getPointerOperand();
      if (Address->getType()->getPointerAddressSpace() == 14)
        continue;
      SmallVector<GetElementPtrInst *> Path;
      Value *Root = Address;
      while (auto *G = dyn_cast<GetElementPtrInst>(Root)) {
        Path.push_back(G);
        Root = G->getPointerOperand();
      }
      auto *S = dyn_cast<SelectInst>(Root);
      if (!S)
        continue;
      Instruction *Then, *Else;
      SplitBlockAndInsertIfThenElse(S->getCondition(), I, &Then, &Else);
      auto Copy = [&](Value *Base, Instruction *Term) -> Instruction * {
        IRBuilder<NoFolder> B(Term);
        for (auto *G : reverse(Path)) {
          SmallVector<Value *> Indices(G->indices());
          Base = B.CreateGEP(G->getSourceElementType(), Base, Indices);
        }
        auto *Op = I->clone();
        Op->setOperand(isa<LoadInst>(Op) ? 0 : 1, Base);
        Op->insertBefore(Term->getIterator());
        return Op;
      };
      auto *T = Copy(S->getTrueValue(), Then);
      auto *E = Copy(S->getFalseValue(), Else);
      if (!I->getType()->isVoidTy()) {
        auto *P = PHINode::Create(I->getType(), 2, "selected.value",
                                  I->getIterator());
        P->addIncoming(T, Then->getParent());
        P->addIncoming(E, Else->getParent());
        I->replaceAllUsesWith(P);
      }
      I->eraseFromParent();
      RecursivelyDeleteTriviallyDeadInstructions(Address);
      Changed = true;
    }
  } while (Changed);
  return true;
}

static bool legalize(Module &M, Function &Entry) {
  PassBuilder PB;
  LoopAnalysisManager LAM;
  FunctionAnalysisManager FAM;
  CGSCCAnalysisManager CGAM;
  ModuleAnalysisManager MAM;
  PB.registerModuleAnalyses(MAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);
  // Resolve invocation-local, statically known function pointers before the
  // call graph check. Dynamic indirect calls remain unsupported.
  for (Function &F : M)
    F.removeFnAttr(Attribute::OptimizeNone);
  FunctionPassManager Resolve;
  Resolve.addPass(SROAPass(SROAOptions::ModifyCFG));
  Resolve.addPass(SimplifyCFGPass());
  ModulePassManager ResolveCalls;
  ResolveCalls.addPass(createModuleToFunctionPassAdaptor(std::move(Resolve)));
  ResolveCalls.run(M, MAM);
  DenseSet<Function *> Active, Reachable;
  if (!checkCalls(Entry, Active, Reachable))
    return false;
  if (M.getNamedGlobal("llvm.global_ctors"))
    return fail("dynamic global initialization is unavailable in shaders");
  for (Function &F : M) {
    if (F.isDeclaration() || &F == &Entry)
      continue;
    F.setLinkage(GlobalValue::InternalLinkage);
    F.setComdat(nullptr);
    F.removeFnAttr(Attribute::NoInline);
    F.removeFnAttr(Attribute::OptimizeNone);
    F.removeFnAttr("hlsl.shader");
    F.removeFnAttr("spirv.cxx.stage");
    if (Reachable.contains(&F))
      F.addFnAttr(Attribute::AlwaysInline);
  }
  for (GlobalVariable &G : M.globals())
    if (G.getAddressSpace() == 0 && G.hasInitializer()) {
      G.setLinkage(GlobalValue::InternalLinkage);
      G.setComdat(nullptr);
    }
  for (Function &F : M)
    for (BasicBlock &BB : F)
      for (Instruction &I : BB)
        if (auto *Call = dyn_cast<CallBase>(&I)) {
          Call->removeFnAttr(Attribute::NoInline);
          Call->removeFnAttr(Attribute::OptimizeNone);
        }
  ModulePassManager Inline;
  Inline.addPass(AlwaysInlinerPass());
  Inline.addPass(GlobalDCEPass());
  Inline.run(M, MAM);
  if (!canonicalizeSpecializations(Entry))
    return false;
  if (!materializeInvocationValues(M, Entry))
    return false;
  FunctionPassManager Functions;
  Functions.addPass(SROAPass(SROAOptions::ModifyCFG));
  Functions.addPass(SimplifyCFGPass());
  ModulePassManager Cleanup;
  Cleanup.addPass(createModuleToFunctionPassAdaptor(std::move(Functions)));
  Cleanup.addPass(GlobalDCEPass());
  Cleanup.run(M, MAM);
  SmallVector<GlobalVariable *> UnusedGlobals;
  for (GlobalVariable &G : M.globals())
    if (G.use_empty())
      UnusedGlobals.push_back(&G);
  for (auto *G : UnusedGlobals)
    G->eraseFromParent();
  for (GlobalVariable &G : M.globals())
    convertUsersOfConstantsToInstructions({&G});
  if (!lowerLocalPointerPhis(Entry))
    return false;
  // LLVM can combine loads from record fields into a byte-offset select.
  // Expand the address expression before constructing logical access chains.
  SmallVector<GetElementPtrInst *> ConditionalOffsets;
  for (Instruction &I : instructions(Entry))
    if (auto *G = dyn_cast<GetElementPtrInst>(&I))
      if (G->getPointerAddressSpace() != 14 && G->getNumIndices() == 1 &&
          isa<SelectInst>(G->getOperand(1)))
        ConditionalOffsets.push_back(G);
  for (auto *G : ConditionalOffsets) {
    auto *S = cast<SelectInst>(G->getOperand(1));
    IRBuilder<NoFolder> B(G);
    auto *T = B.CreateGEP(G->getSourceElementType(), G->getPointerOperand(),
                          S->getTrueValue());
    auto *E = B.CreateGEP(G->getSourceElementType(), G->getPointerOperand(),
                          S->getFalseValue());
    G->replaceAllUsesWith(B.CreateSelect(S->getCondition(), T, E));
    G->eraseFromParent();
  }
  if (!lowerLogicalSelections(Entry))
    return false;
  SmallVector<Instruction *> PhysicalChoices;
  for (Instruction &I : instructions(Entry)) {
    if (auto *Cmp = dyn_cast<ICmpInst>(&I)) {
      if (Cmp->getOperand(0)->getType()->isPointerTy() &&
          Cmp->getOperand(0)->getType()->getPointerAddressSpace() == 14) {
        IRBuilder<NoFolder> B(Cmp);
        for (unsigned N = 0; N != 2; ++N) {
          Value *V = Cmp->getOperand(N);
          Cmp->setOperand(N, isa<ConstantPointerNull>(V->stripPointerCasts())
                                 ? static_cast<Value *>(B.getInt64(0))
                                 : B.CreatePtrToInt(V, B.getInt64Ty()));
        }
      }
    }
    if (I.getType()->isPointerTy() &&
        I.getType()->getPointerAddressSpace() == 14 && isa<SelectInst>(I))
      PhysicalChoices.push_back(&I);
  }
  for (auto *I : PhysicalChoices) {
    auto *S = cast<SelectInst>(I);
    IRBuilder<NoFolder> B(S);
    auto Address = [&](Value *V) -> Value * {
      return isa<ConstantPointerNull>(V->stripPointerCasts())
                 ? static_cast<Value *>(B.getInt64(0))
                 : B.CreatePtrToInt(V, B.getInt64Ty());
    };
    auto *V = B.CreateSelect(S->getCondition(), Address(S->getTrueValue()),
                             Address(S->getFalseValue()));
    auto *P = B.CreateIntToPtr(V, S->getType());
    S->replaceAllUsesWith(P);
    S->eraseFromParent();
  }
  // A physical null is an integer address converted in the function body;
  // SPIR-V forbids OpConstantNull for PhysicalStorageBuffer pointers.
  for (Instruction &I : instructions(Entry))
    for (unsigned N = 0; N < I.getNumOperands(); ++N)
      if (auto *Null = dyn_cast<ConstantPointerNull>(I.getOperand(N)))
        if (Null->getType()->getPointerAddressSpace() == 14) {
          Instruction *Insert = &I;
          if (auto *P = dyn_cast<PHINode>(&I))
            Insert = P->getIncomingBlock(N)->getTerminator();
          IRBuilder<NoFolder> B(Insert);
          I.setOperand(N, B.CreateIntToPtr(B.getInt64(0), Null->getType()));
        }
  // Optimizing a constant descriptor index can produce byte GEPs. Restore
  // typed indices explicitly, including for an unsized runtime descriptor
  // array whose zero LLVM bound must never be used as an access bound.
  auto IsByteType = [](Type *T) {
    while (auto *A = dyn_cast<ArrayType>(T))
      T = A->getElementType();
    return T->isIntegerTy(8);
  };
  SmallVector<GetElementPtrInst *> DescriptorBytes;
  for (Instruction &I : instructions(Entry))
    if (auto *G = dyn_cast<GetElementPtrInst>(&I))
      if (G->getPointerAddressSpace() == 2 &&
          IsByteType(G->getSourceElementType()))
        DescriptorBytes.push_back(G);
  for (auto *G : reverse(DescriptorBytes)) {
    IRBuilder<> B(G);
    Value *Bytes = B.getInt64(0);
    Value *Base = G;
    while (auto *P = dyn_cast<GetElementPtrInst>(Base)) {
      if (!IsByteType(P->getSourceElementType()))
        break;
      SmallMapVector<Value *, APInt, 4> Variables;
      APInt Constant(64, 0);
      if (!P->collectOffset(M.getDataLayout(), 64, Variables, Constant))
        return fail("unsupported descriptor byte offset");
      Bytes = B.CreateAdd(Bytes, B.getInt64(Constant.getZExtValue()));
      for (auto &[Index, Stride] : Variables)
        Bytes = B.CreateAdd(
            Bytes, B.CreateMul(B.CreateSExtOrTrunc(Index, B.getInt64Ty()),
                               B.getInt64(Stride.getZExtValue())));
      Base = P->getPointerOperand();
    }
    auto *Global = dyn_cast<GlobalVariable>(Base);
    auto *Array =
        Global ? dyn_cast<ArrayType>(Global->getValueType()) : nullptr;
    if (!Array || !Global->getMetadata("spirv.cxx.resource"))
      continue;
    uint64_t Size = M.getDataLayout().getTypeAllocSize(Array->getElementType());
    if (!isPowerOf2_64(Size) ||
        computeKnownBits(Bytes, M.getDataLayout()).countMinTrailingZeros() <
            Log2_64(Size))
      return fail("descriptor byte offset is not aligned to its element type");
    Value *Index = B.CreateSDiv(Bytes, B.getInt64(Size));
    IRBuilder<NoFolder> Typed(G);
    G->replaceAllUsesWith(
        Typed.CreateGEP(Array, Global, {B.getInt32(0), Index}));
    G->eraseFromParent();
  }
  // Descriptor iterator arithmetic and subscripting use the same borrowed
  // native base. Combine offsets so logical storage never needs PtrAccessChain.
  for (Instruction &I : instructions(Entry)) {
    auto *G = dyn_cast<GetElementPtrInst>(&I);
    if (!G || G->getPointerAddressSpace() != 2 || G->getNumIndices() != 1 ||
        !isa<TargetExtType>(G->getSourceElementType()))
      continue;
    while (auto *P = dyn_cast<GetElementPtrInst>(G->getPointerOperand())) {
      if (P->getSourceElementType() != G->getSourceElementType() ||
          P->getNumIndices() != 1)
        break;
      IRBuilder<> B(G);
      Value *Offset =
          B.CreateAdd(B.CreateSExtOrTrunc(P->getOperand(1), B.getInt64Ty()),
                      B.CreateSExtOrTrunc(G->getOperand(1), B.getInt64Ty()));
      G->setOperand(0, P->getPointerOperand());
      G->setOperand(1, Offset);
    }
  }
  // Byte addressing on physical storage is integer address arithmetic. An
  // intermediate i8 pointer would unnecessarily require 8-bit shader/storage
  // features even when every accessed value is a float or a wider integer.
  SmallVector<GetElementPtrInst *> PhysicalBytes;
  for (Instruction &I : instructions(Entry))
    if (auto *G = dyn_cast<GetElementPtrInst>(&I))
      if (G->getPointerAddressSpace() == 14 && G->getNumIndices() == 1 &&
          G->getSourceElementType()->isIntegerTy(8))
        PhysicalBytes.push_back(G);
  for (auto *G : PhysicalBytes) {
    IRBuilder<> B(G);
    Value *Address = B.CreatePtrToInt(G->getPointerOperand(), B.getInt64Ty());
    Address = B.CreateAdd(
        Address, B.CreateSExtOrTrunc(G->getOperand(1), B.getInt64Ty()));
    G->replaceAllUsesWith(B.CreateIntToPtr(Address, G->getType()));
    G->eraseFromParent();
  }
  // Recover logical byte offsets as typed aggregate paths. LLVM can fold
  // nested arrays into byte strides even when no C++ reinterpretation exists.
  SmallVector<GetElementPtrInst *> BytePaths;
  for (Instruction &I : instructions(Entry))
    if (auto *G = dyn_cast<GetElementPtrInst>(&I))
      if (G->getPointerAddressSpace() != 14 &&
          G->getPointerAddressSpace() != 2 &&
          IsByteType(G->getSourceElementType()))
        BytePaths.push_back(G);
  for (auto *G : reverse(BytePaths)) {
    // Fold the complete byte path before reconstructing its typed base. In
    // particular, an optimized shared array can have a [4 x i8] dynamic stride
    // followed by an i8 constant offset. Rewriting only the inner GEP loses the
    // enclosing array needed to turn the outer offset into an AccessChain.
    Value *Root = G;
    while (auto *P = dyn_cast<GetElementPtrInst>(Root)) {
      if (!IsByteType(P->getSourceElementType()))
        break;
      Root = P->getPointerOperand();
    }
    Type *RootType = nullptr;
    if (auto *P = dyn_cast<GetElementPtrInst>(Root))
      RootType = P->getResultElementType();
    else if (auto *V = dyn_cast<GlobalVariable>(Root))
      RootType = V->getValueType();
    else if (auto *A = dyn_cast<AllocaInst>(Root))
      RootType = A->getAllocatedType();
    const auto *Array = dyn_cast_or_null<ArrayType>(RootType);
    if (!Array || Array->getElementType()->isAggregateType()) {
      // Preserve row/field boundaries in nested aggregates. Their separate
      // byte strides are reconstructed one level at a time below; flattening
      // row and column offsets would require division by an entire row size.
      if (!G->getSourceElementType()->isIntegerTy(8) ||
          G->getNumIndices() != 1)
        continue;
      while (auto *P = dyn_cast<GetElementPtrInst>(G->getPointerOperand())) {
        if (!P->getSourceElementType()->isIntegerTy(8) ||
            P->getNumIndices() != 1)
          break;
        IRBuilder<> B(G);
        Value *Offset = B.CreateAdd(
            B.CreateSExtOrTrunc(P->getOperand(1), B.getInt64Ty()),
            B.CreateSExtOrTrunc(G->getOperand(1), B.getInt64Ty()));
        G->setOperand(0, P->getPointerOperand());
        G->setOperand(1, Offset);
      }
      continue;
    }
    IRBuilder<> B(G);
    Value *Bytes = B.getInt64(0);
    Value *Base = G;
    while (auto *P = dyn_cast<GetElementPtrInst>(Base)) {
      if (!IsByteType(P->getSourceElementType()))
        break;
      SmallMapVector<Value *, APInt, 4> Variables;
      APInt Constant(64, 0);
      if (!P->collectOffset(M.getDataLayout(), 64, Variables, Constant))
        return fail("unsupported logical byte offset");
      Bytes = B.CreateAdd(Bytes, B.getInt64(Constant.getZExtValue()));
      for (auto &[Index, Stride] : Variables)
        Bytes = B.CreateAdd(
            Bytes, B.CreateMul(B.CreateSExtOrTrunc(Index, B.getInt64Ty()),
                               B.getInt64(Stride.getZExtValue())));
      Base = P->getPointerOperand();
    }
    IRBuilder<NoFolder> Typed(G);
    G->replaceAllUsesWith(Typed.CreateGEP(B.getInt8Ty(), Base, Bytes));
    G->eraseFromParent();
  }
  BytePaths.clear();
  for (Instruction &I : instructions(Entry))
    if (auto *G = dyn_cast<GetElementPtrInst>(&I))
      if (G->getPointerAddressSpace() != 14 &&
          G->getPointerAddressSpace() != 2 && G->getNumIndices() == 1)
        BytePaths.push_back(G);
  for (auto *G : BytePaths) {
    uint64_t Stride = 0;
    Type *Source = G->getSourceElementType();
    if (Source->isIntegerTy(8))
      Stride = 1;
    else if (auto *A = dyn_cast<ArrayType>(Source))
      if (A->getElementType()->isIntegerTy(8))
        Stride = A->getNumElements();
    if (!Stride)
      continue;
    Value *Base = G->getPointerOperand();
    Type *T = nullptr;
    if (auto *P = dyn_cast<GetElementPtrInst>(Base))
      T = P->getResultElementType();
    else if (auto *V = dyn_cast<GlobalVariable>(Base))
      T = V->getValueType();
    else if (auto *A = dyn_cast<AllocaInst>(Base))
      T = A->getAllocatedType();
    if (!T || !T->isAggregateType())
      continue;
    IRBuilder<NoFolder> B(G);
    SmallVector<Value *> Indices{B.getInt32(0)};
    Type *RootType = T;
    if (auto *C = dyn_cast<ConstantInt>(G->getOperand(1))) {
      uint64_t Offset = C->getZExtValue() * Stride;
      if (Offset >= M.getDataLayout().getTypeAllocSize(T))
        continue;
      do {
        if (auto *S = dyn_cast<StructType>(T)) {
          const auto *L = M.getDataLayout().getStructLayout(S);
          unsigned N = L->getElementContainingOffset(Offset);
          Offset -= L->getElementOffset(N);
          Indices.push_back(B.getInt32(N));
          T = S->getElementType(N);
        } else if (auto *A = dyn_cast<ArrayType>(T)) {
          uint64_t Size =
              M.getDataLayout().getTypeAllocSize(A->getElementType());
          Indices.push_back(B.getInt64(Offset / Size));
          Offset %= Size;
          T = A->getElementType();
        } else
          break;
      } while (Offset != 0);
      if (Offset)
        continue;
    } else {
      while (auto *S = dyn_cast<StructType>(T)) {
        if (!S->getNumElements())
          break;
        Indices.push_back(B.getInt32(0));
        T = S->getElementType(0);
      }
      auto *A = dyn_cast<ArrayType>(T);
      if (!A)
        continue;
      uint64_t ElementSize =
          M.getDataLayout().getTypeAllocSize(A->getElementType());
      Value *Index = G->getOperand(1);
      if (ElementSize != Stride) {
        Index = B.CreateSExtOrTrunc(Index, B.getInt64Ty());
        Value *Bytes = B.CreateMul(Index, B.getInt64(Stride));
        if (!isPowerOf2_64(ElementSize) ||
            computeKnownBits(Bytes, M.getDataLayout()).countMinTrailingZeros() <
                Log2_64(ElementSize))
          return fail("logical byte offset is not aligned to its element type");
        Index = B.CreateSDiv(Bytes, B.getInt64(ElementSize));
      }
      Indices.push_back(Index);
    }
    auto *Typed = B.CreateGEP(RootType, Base, Indices, G->getName() + ".path");
    G->replaceAllUsesWith(Typed);
    G->eraseFromParent();
  }
  // Opaque-pointer optimization can erase any number of zero-offset record
  // members. Restore the typed path before SPIR-V constructs access chains.
  SmallVector<GetElementPtrInst *> Paths;
  for (Instruction &I : instructions(Entry))
    if (auto *G = dyn_cast<GetElementPtrInst>(&I))
      Paths.push_back(G);
  for (auto *G : Paths) {
    Value *Base = G->getPointerOperand();
    Type *Actual = nullptr;
    if (auto *Parent = dyn_cast<GetElementPtrInst>(Base))
      Actual = Parent->getResultElementType();
    else if (auto *Global = dyn_cast<GlobalVariable>(Base))
      Actual = Global->getValueType();
    else if (auto *Local = dyn_cast<AllocaInst>(Base))
      Actual = Local->getAllocatedType();
    SmallVector<Type *> Prefix;
    while (Actual && Actual != G->getSourceElementType()) {
      if (auto *R = dyn_cast<StructType>(Actual)) {
        if (R->getNumElements() == 0 ||
            M.getDataLayout().getStructLayout(R)->getElementOffset(0) != 0)
          break;
        Prefix.push_back(R);
        Actual = R->getElementType(0);
      } else if (auto *A = dyn_cast<ArrayType>(Actual)) {
        Prefix.push_back(A);
        Actual = A->getElementType();
      } else
        break;
    }
    if (Actual != G->getSourceElementType())
      continue;
    IRBuilder<NoFolder> B(G);
    for (Type *R : Prefix)
      Base =
          B.CreateGEP(R, Base, {B.getInt32(0), B.getInt32(0)}, "first.member");
    G->setOperand(0, Base);
  }
  // Keep aggregate paths together so backend type inference cannot collapse a
  // zero-offset first member into a pointer to the containing record.
  SmallVector<GetElementPtrInst *> AggregateGEPs;
  for (Instruction &I : instructions(Entry))
    if (auto *GEP = dyn_cast<GetElementPtrInst>(&I))
      AggregateGEPs.push_back(GEP);
  for (auto *GEP : AggregateGEPs) {
    auto *Parent = dyn_cast<GetElementPtrInst>(GEP->getPointerOperand());
    auto *First = dyn_cast<ConstantInt>(GEP->getOperand(1));
    if (!Parent || !First || !First->isZero() ||
        Parent->getResultElementType() != GEP->getSourceElementType())
      continue;
    SmallVector<Value *> Indices(Parent->indices());
    llvm::append_range(Indices, llvm::drop_begin(GEP->indices()));
    IRBuilder<NoFolder> B(GEP);
    auto *Combined =
        B.CreateGEP(Parent->getSourceElementType(), Parent->getPointerOperand(),
                    Indices, GEP->getName() + ".aggregate");
    GEP->replaceAllUsesWith(Combined);
    GEP->eraseFromParent();
  }
  // Generic LLVM optimization may spell scalar indexing as [sizeof(T) x i8].
  // Recover the element type when every use agrees and the stride is identical.
  SmallVector<GetElementPtrInst *> ByteGEPs;
  for (Instruction &I : instructions(Entry))
    if (auto *GEP = dyn_cast<GetElementPtrInst>(&I))
      if (GEP->getPointerAddressSpace() == 14 && GEP->getNumIndices() == 1)
        if (auto *Array = dyn_cast<ArrayType>(GEP->getSourceElementType()))
          if (Array->getElementType()->isIntegerTy(8))
            ByteGEPs.push_back(GEP);
  for (auto *GEP : ByteGEPs) {
    Type *Element = nullptr;
    bool Compatible = true;
    for (User *U : GEP->users()) {
      Type *Access = nullptr;
      if (auto *Load = dyn_cast<LoadInst>(U))
        Access = Load->getType();
      else if (auto *Store = dyn_cast<StoreInst>(U))
        if (Store->getPointerOperand() == GEP)
          Access = Store->getValueOperand()->getType();
      if (!Access || (Element && Access != Element)) {
        Compatible = false;
        break;
      }
      Element = Access;
    }
    if (Compatible && Element && Element->isSized() &&
        M.getDataLayout().getTypeAllocSize(Element) ==
            M.getDataLayout().getTypeAllocSize(GEP->getSourceElementType())) {
      IRBuilder<NoFolder> B(GEP);
      auto *Typed = B.CreateGEP(Element, GEP->getPointerOperand(),
                                GEP->getOperand(1), GEP->getName() + ".typed");
      GEP->replaceAllUsesWith(Typed);
      GEP->eraseFromParent();
    }
  }
  // Recover pointer-load hints after LLVM merges loads with different
  // metadata. Interface type trees remain authoritative across optimization.
  DenseSet<Value *> Describing;
  auto DescribeAddress = [&](auto &&Self, Value *V) -> Type * {
    if (!Describing.insert(V).second)
      return nullptr;
    auto Done = scope_exit([&] { Describing.erase(V); });
    if (auto *G = dyn_cast<GlobalVariable>(V)) {
      if (auto *N = G->getMetadata("spirv.cxx.pointee"))
        return cast<ValueAsMetadata>(N->getOperand(0))->getType();
      return G->getValueType();
    }
    if (auto *L = dyn_cast<LoadInst>(V)) {
      if (auto *N = L->getMetadata("spirv.cxx.pointee"))
        return cast<ValueAsMetadata>(N->getOperand(0))->getType();
      Type *T = Self(Self, L->getPointerOperand());
      while (T) {
        if (auto *P = dyn_cast<TargetExtType>(T))
          if (P->getName() == "spirv.$TypedPointerType")
            return P->getTypeParameter(0);
        if (auto *S = dyn_cast<StructType>(T); S && S->getNumElements())
          T = S->getElementType(0);
        else if (auto *A = dyn_cast<ArrayType>(T))
          T = A->getElementType();
        else
          break;
      }
    }
    if (auto *G = dyn_cast<GetElementPtrInst>(V)) {
      Type *T = Self(Self, G->getPointerOperand());
      for (Value *I : drop_begin(G->indices())) {
        if (auto *S = dyn_cast_or_null<StructType>(T)) {
          auto *C = dyn_cast<ConstantInt>(I);
          if (!C || C->getZExtValue() >= S->getNumElements())
            return nullptr;
          T = S->getElementType(C->getZExtValue());
        } else if (auto *A = dyn_cast_or_null<ArrayType>(T))
          T = A->getElementType();
        else
          return nullptr;
      }
      return T;
    }
    return nullptr;
  };
  for (Instruction &I : instructions(Entry))
    if (auto *L = dyn_cast<LoadInst>(&I))
      if (L->getType()->isPointerTy() &&
          L->getType()->getPointerAddressSpace() == 14)
        if (Type *T = DescribeAddress(DescribeAddress, L))
          L->setMetadata("spirv.cxx.pointee",
                         MDNode::get(M.getContext(), ConstantAsMetadata::get(
                                                         PoisonValue::get(T))));
  AssumptionCache AC(Entry);
  DominatorTree DT(Entry);
  for (Instruction &I : instructions(Entry)) {
    if (auto *Load = dyn_cast<LoadInst>(&I))
      Load->setAlignment(
          std::max(Load->getAlign(),
                   getKnownAlignment(Load->getPointerOperand(),
                                     M.getDataLayout(), Load, &AC, &DT)));
    else if (auto *Store = dyn_cast<StoreInst>(&I))
      Store->setAlignment(
          std::max(Store->getAlign(),
                   getKnownAlignment(Store->getPointerOperand(),
                                     M.getDataLayout(), Store, &AC, &DT)));
  }
  if (!M.getNamedMetadata("spirv.MemoryModel"))
    M.getOrInsertNamedMetadata("spirv.MemoryModel")
        ->addOperand(MDNode::get(M.getContext(), {number(M.getContext(), 5348),
                                                  number(M.getContext(), 3)}));
  // C++ records/C arrays expose vector components without requiring a public
  // vector class. Stage interfaces must nevertheless use SPIR-V vectors:
  // separate scalar struct members would each consume an entire location.
  DenseMap<Type *, Type *> PackedTypes;
  auto PackType = [&](auto &&Self, Type *T) -> Type * {
    if (auto *Known = PackedTypes.lookup(T))
      return Known;
    SmallVector<Type *> Members;
    if (auto *S = dyn_cast<StructType>(T)) {
      if (S->isOpaque())
        return T;
      Members.append(S->element_begin(), S->element_end());
    } else if (auto *A = dyn_cast<ArrayType>(T)) {
      if (A->getNumElements() >= 2 && A->getNumElements() <= 4)
        Members.assign(A->getNumElements(), A->getElementType());
      else
        return T;
    } else
      return T;
    Type *Result = T;
    if (Members.size() >= 2 && Members.size() <= 4 &&
        (Members[0]->isFloatingPointTy() || Members[0]->isIntegerTy(32) ||
         Members[0]->isIntegerTy(64)) &&
        llvm::all_of(Members, [&](Type *E) { return E == Members[0]; })) {
      Result = FixedVectorType::get(Members[0], Members.size());
    } else if (auto *S = dyn_cast<StructType>(T)) {
      bool Changed = false;
      for (Type *&E : Members) {
        Type *New = Self(Self, E);
        Changed |= New != E;
        E = New;
      }
      if (Changed)
        Result = StructType::get(M.getContext(), Members, S->isPacked());
    }
    PackedTypes[T] = Result;
    return Result;
  };
  SmallVector<GlobalVariable *> Interfaces;
  for (auto &G : M.globals())
    if ((G.getAddressSpace() == 7 || G.getAddressSpace() == 8) &&
        PackType(PackType, G.getValueType()) != G.getValueType())
      Interfaces.push_back(&G);
  for (auto *G : Interfaces) {
    Type *OldType = G->getValueType();
    Type *NewType = PackType(PackType, OldType);
    auto *Output = new GlobalVariable(M, NewType, false, G->getLinkage(),
                                      nullptr, G->getName() + ".interface",
                                      nullptr, GlobalVariable::NotThreadLocal,
                                      G->getAddressSpace());
    Output->copyMetadata(G, 0);
    Output->setAlignment(G->getAlign());
    Output->setMetadata(
        "spirv.cxx.source_interface",
        MDNode::get(M.getContext(),
                    ConstantAsMetadata::get(PoisonValue::get(OldType))));
    Output->setMetadata(
        "spirv.cxx.pointee",
        MDNode::get(M.getContext(),
                    ConstantAsMetadata::get(PoisonValue::get(NewType))));
    auto ConvertValue = [&](auto &&Self, IRBuilder<> &B, Value *V,
                            Type *To) -> Value * {
      if (V->getType() == To)
        return V;
      Type *From = V->getType();
      unsigned Count = isa<StructType>(From)
                           ? cast<StructType>(From)->getNumElements()
                       : isa<ArrayType>(From)
                           ? cast<ArrayType>(From)->getNumElements()
                           : cast<FixedVectorType>(From)->getNumElements();
      Value *Result = PoisonValue::get(To);
      for (unsigned I = 0; I < Count; ++I) {
        Value *Part = From->isVectorTy() ? B.CreateExtractElement(V, I)
                                         : B.CreateExtractValue(V, I);
        Type *Element = To->isVectorTy()     ? To->getScalarType()
                        : isa<ArrayType>(To) ? To->getArrayElementType()
                                             : To->getStructElementType(I);
        Part = Self(Self, B, Part, Element);
        Result = To->isVectorTy() ? B.CreateInsertElement(Result, Part, I)
                                  : B.CreateInsertValue(Result, Part, I);
      }
      return Result;
    };
    SmallVector<Value *> Pointers{G};
    while (!Pointers.empty()) {
      Value *Pointer = Pointers.pop_back_val();
      Type *OldPointee =
          Pointer == G
              ? OldType
              : cast<GetElementPtrInst>(Pointer)->getResultElementType();
      Type *NewPointee = PackType(PackType, OldPointee);
      SmallVector<User *> Users(Pointer->users());
      for (User *U : Users) {
        if (auto *P = dyn_cast<GetElementPtrInst>(U)) {
          Pointers.push_back(P);
        } else if (auto *L = dyn_cast<LoadInst>(U)) {
          IRBuilder<> B(L);
          Value *Address = Pointer == G ? Output : Pointer;
          Type *Access = L->getType();
          SmallVector<Value *> Path{B.getInt32(0)};
          Type *Leaf = NewPointee;
          while (
              Leaf != PackType(PackType, Access) &&
              (Leaf->isStructTy() || Leaf->isArrayTy() || Leaf->isVectorTy())) {
            Path.push_back(B.getInt32(0));
            Leaf = Leaf->isVectorTy()  ? Leaf->getScalarType()
                   : Leaf->isArrayTy() ? Leaf->getArrayElementType()
                                       : Leaf->getStructElementType(0);
          }
          if (Leaf != PackType(PackType, Access))
            return fail("unsupported stage interface load");
          if (Path.size() > 1)
            Address = B.CreateGEP(NewPointee, Address, Path);
          auto *Loaded = B.CreateAlignedLoad(Leaf, Address, L->getAlign(),
                                             L->isVolatile());
          L->replaceAllUsesWith(ConvertValue(ConvertValue, B, Loaded, Access));
          L->eraseFromParent();
        } else if (auto *S = dyn_cast<StoreInst>(U)) {
          IRBuilder<> B(S);
          Value *Address = Pointer == G ? Output : Pointer;
          Type *Access = S->getValueOperand()->getType();
          SmallVector<Value *> Path{B.getInt32(0)};
          Type *Leaf = NewPointee;
          while (
              Leaf != PackType(PackType, Access) &&
              (Leaf->isStructTy() || Leaf->isArrayTy() || Leaf->isVectorTy())) {
            Path.push_back(B.getInt32(0));
            Leaf = Leaf->isVectorTy()  ? Leaf->getScalarType()
                   : Leaf->isArrayTy() ? Leaf->getArrayElementType()
                                       : Leaf->getStructElementType(0);
          }
          if (Leaf != PackType(PackType, Access))
            return fail("unsupported stage interface store");
          if (Path.size() > 1)
            Address = B.CreateGEP(NewPointee, Address, Path);
          B.CreateAlignedStore(
              ConvertValue(ConvertValue, B, S->getValueOperand(), Leaf),
              Address, S->getAlign(), S->isVolatile());
          S->eraseFromParent();
        } else
          return fail("unsupported stage interface pointer escape");
      }
    }
    // Update typed GEPs only after all original pointees have been inspected.
    for (Instruction &I : instructions(Entry))
      if (auto *P = dyn_cast<GetElementPtrInst>(&I)) {
        Value *Root = P->getPointerOperand();
        while (auto *Parent = dyn_cast<GetElementPtrInst>(Root))
          Root = Parent->getPointerOperand();
        if (Root == G || Root == Output) {
          P->setSourceElementType(
              PackType(PackType, P->getSourceElementType()));
          P->setResultElementType(
              PackType(PackType, P->getResultElementType()));
        }
      }
    G->replaceAllUsesWith(Output);
    G->eraseFromParent();
  }
  // The backend cannot form a component pointer with a dynamic vector index.
  // Keep the C-array spelling by loading/extracting or inserting/storing the
  // interface vector instead. Each stage interface is invocation-local.
  SmallVector<GetElementPtrInst *> DynamicComponents;
  for (Instruction &I : instructions(Entry))
    if (auto *P = dyn_cast<GetElementPtrInst>(&I)) {
      if (P->getPointerAddressSpace() != 7 && P->getPointerAddressSpace() != 8)
        continue;
      SmallVector<Value *> Prefix(P->indices());
      if (Prefix.size() < 2 || isa<ConstantInt>(Prefix.back()))
        continue;
      Prefix.pop_back();
      if (isa_and_nonnull<FixedVectorType>(GetElementPtrInst::getIndexedType(
              P->getSourceElementType(), Prefix)))
        DynamicComponents.push_back(P);
    }
  for (auto *P : DynamicComponents) {
    SmallVector<Value *> Prefix(P->indices());
    Value *Index = Prefix.pop_back_val();
    Type *Vector =
        GetElementPtrInst::getIndexedType(P->getSourceElementType(), Prefix);
    IRBuilder<> B(P);
    Value *Base =
        B.CreateGEP(P->getSourceElementType(), P->getPointerOperand(), Prefix);
    SmallVector<User *> Users(P->users());
    for (User *U : Users) {
      auto *I = dyn_cast<Instruction>(U);
      if (!I)
        return fail("unsupported dynamic stage component pointer escape");
      B.SetInsertPoint(I);
      if (auto *L = dyn_cast<LoadInst>(I)) {
        auto *V = B.CreateAlignedLoad(Vector, Base, Align(4), L->isVolatile());
        L->replaceAllUsesWith(B.CreateExtractElement(V, Index));
        L->eraseFromParent();
      } else if (auto *S = dyn_cast<StoreInst>(I)) {
        if (S->getPointerOperand() != P)
          return fail("unsupported dynamic stage component pointer escape");
        auto *V = B.CreateAlignedLoad(Vector, Base, Align(4), S->isVolatile());
        B.CreateAlignedStore(
            B.CreateInsertElement(V, S->getValueOperand(), Index), Base,
            Align(4), S->isVolatile());
        S->eraseFromParent();
      } else
        return fail("unsupported dynamic stage component pointer escape");
    }
    P->eraseFromParent();
  }
  // LLVM -O1 can scalarize a local record into a byte array while retaining
  // wider loads and stores through its elements. That mixed-width layout has
  // no logical SPIR-V pointer representation, and the backend's single-offset
  // recovery cannot legalize the resulting two-index GEP.
  for (Instruction &I : instructions(Entry))
    if (auto *P = dyn_cast<GetElementPtrInst>(&I))
      if (auto *Array = dyn_cast<ArrayType>(P->getSourceElementType()))
        if (Array->getElementType()->isIntegerTy(8) &&
            P->getNumIndices() != 1 && P->getPointerAddressSpace() != 14) {
          bool MixedWidth = llvm::any_of(P->users(), [](User *U) {
            if (auto *L = dyn_cast<LoadInst>(U))
              return !L->getType()->isIntegerTy(8);
            if (auto *S = dyn_cast<StoreInst>(U))
              return !S->getValueOperand()->getType()->isIntegerTy(8);
            return false;
          });
          if (MixedWidth)
            return fail("mixed-width access to a byte-addressed local "
                        "aggregate is unsupported in logical SPIR-V");
        }
  if (verifyModule(M, &errs()))
    return fail("invalid LLVM IR after C++ shader legalization");
  return true;
}

static json::Object reflectLimits(Module &M, Function &Entry) {
  uint64_t PushBytes = 0, SharedBytes = 0;
  const auto &DL = M.getDataLayout();
  for (auto &G : M.globals()) {
    if (G.getAddressSpace() == 13)
      PushBytes = std::max(
          PushBytes, DL.getTypeAllocSize(G.getValueType()).getFixedValue());
    if (G.getAddressSpace() == 3) {
      SharedBytes = alignTo(SharedBytes, G.getAlign().valueOrOne());
      SharedBytes += DL.getTypeAllocSize(G.getValueType());
    }
  }
  json::Object R{{"maxPushConstantsSize", PushBytes},
                 {"maxComputeSharedMemorySize", SharedBytes}};
  if (Entry.hasFnAttribute("hlsl.numthreads")) {
    SmallVector<StringRef> Parts;
    Entry.getFnAttribute("hlsl.numthreads")
        .getValueAsString()
        .split(Parts, ',');
    json::Array Size;
    uint64_t Product = 1;
    for (StringRef P : Parts) {
      unsigned D = 0;
      P.getAsInteger(10, D);
      Product *= D;
      Size.push_back(D);
    }
    R["maxComputeWorkGroupSize"] = std::move(Size);
    R["maxComputeWorkGroupInvocations"] = Product;
  }
  return R;
}

#include "ShaderInlineAsm.inc"

// Lower each entry independently: stage interfaces and invocation storage are
// specific to an entry, while SPIR-V Tools preserves those interfaces when it
// combines the validated modules.
static int emitAllEntries(Module &M, ArrayRef<std::string> EntryNames,
                          const char *Executable) {
  SmallString<256> Directory;
  if (std::error_code EC =
          sys::fs::createUniqueDirectory("clang-spirv-entries", Directory)) {
    fail(EC.message());
    return 1;
  }
  auto Cleanup = scope_exit([&] { sys::fs::remove_directories(Directory); });
  auto Path = [&](StringRef Name) {
    SmallString<256> Result(Directory);
    sys::path::append(Result, Name);
    return Result.str().str();
  };
  std::string Source = Path("linked.bc");
  {
    std::error_code EC;
    ToolOutputFile File(Source, EC, sys::fs::OF_None);
    if (EC) {
      fail(EC.message());
      return 1;
    }
    WriteBitcodeToFile(M, File.os());
    File.keep();
  }
  std::string Self =
      sys::fs::getMainExecutable(Executable, reinterpret_cast<void *>(&fail));
  SmallVector<std::string> Inputs;
  json::Array Entries;
  std::set<int64_t> Capabilities;
  std::set<std::string> Extensions, Features;
  for (size_t I = 0; I < EntryNames.size(); ++I) {
    std::string Binary = Path("entry-" + std::to_string(I) + ".spv");
    std::string Selection = "--entry=" + EntryNames[I];
    SmallVector<StringRef> Args{Self, Source, Selection, "-o", Binary};
    std::string ValidatorSelection;
    if (!Validator.empty()) {
      ValidatorSelection = "--validator=" + Validator.getValue();
      Args.push_back(ValidatorSelection);
    }
    if (sys::ExecuteAndWait(Self, Args) != 0)
      return 1;
    auto JSON = MemoryBuffer::getFile(Binary + ".json");
    if (!JSON) {
      fail("cannot read entry reflection for '" + EntryNames[I] + "'");
      return 1;
    }
    auto Parsed = json::parse((*JSON)->getBuffer());
    if (!Parsed || !Parsed->getAsObject()) {
      fail("invalid entry reflection for '" + EntryNames[I] + "'");
      return 1;
    }
    auto *Object = Parsed->getAsObject();
    if (auto *Array = Object->getArray("spirv_capabilities"))
      for (const auto &Value : *Array)
        if (auto N = Value.getAsInteger())
          Capabilities.insert(*N);
    if (auto *Array = Object->getArray("spirv_extensions"))
      for (const auto &Value : *Array)
        if (auto S = Value.getAsString())
          Extensions.insert(S->str());
    if (auto *Array = Object->getArray("required_features"))
      for (const auto &Value : *Array)
        if (auto S = Value.getAsString())
          Features.insert(S->str());
    Entries.push_back(std::move(*Object));
    Inputs.push_back(std::move(Binary));
  }

  SmallString<256> ToolDirectory(Self);
  sys::path::remove_filename(ToolDirectory);
  auto Linker = sys::findProgramByName("spirv-link", {ToolDirectory});
  if (!Linker)
    Linker = sys::findProgramByName("spirv-link");
  if (!Linker) {
    fail("spirv-link is required for multiple entries; install SPIR-V Tools "
         "beside Clang or on PATH");
    return 1;
  }
  std::string Merged = Path("merged.spv");
  SmallVector<StringRef> LinkArgs{*Linker, "--target-env", "vulkan1.3", "-o",
                                  Merged};
  for (const std::string &Input : Inputs)
    LinkArgs.push_back(Input);
  if (sys::ExecuteAndWait(*Linker, LinkArgs) != 0) {
    fail("cannot combine shader entry points");
    return 1;
  }
  auto Binary = MemoryBuffer::getFile(Merged, /*IsText=*/false);
  if (!Binary || (*Binary)->getBufferSize() < 20 ||
      support::endian::read32le((*Binary)->getBufferStart()) != 0x07230203 ||
      support::endian::read32le((*Binary)->getBufferStart() + 4) !=
          0x00010400) {
    fail("combined module is not SPIR-V 1.4");
    return 1;
  }
  auto ValidatorProgram =
      Validator.empty() ? sys::findProgramByName("spirv-val", {ToolDirectory})
                        : ErrorOr<std::string>(Validator.getValue());
  if (!ValidatorProgram && Validator.empty())
    ValidatorProgram = sys::findProgramByName("spirv-val");
  if (!ValidatorProgram) {
    fail("spirv-val is required for Vulkan shader validation");
    return 1;
  }
  SmallVector<StringRef> ValidateArgs{*ValidatorProgram, "--target-env",
                                      "vulkan1.3", "--scalar-block-layout",
                                      Merged};
  if (sys::ExecuteAndWait(*ValidatorProgram, ValidateArgs) != 0) {
    fail("combined Vulkan shader validation failed; output was not published");
    return 1;
  }
  json::Array CapabilityArray, ExtensionArray, FeatureArray;
  for (int64_t Value : Capabilities)
    CapabilityArray.push_back(Value);
  for (const std::string &Value : Extensions)
    ExtensionArray.push_back(Value);
  for (const std::string &Value : Features)
    FeatureArray.push_back(Value);
  json::Object Reflection{{"schema_version", 2},
                          {"target", "spirv1.4-unknown-vulkan1.3"},
                          {"entries", std::move(Entries)},
                          {"spirv_capabilities", std::move(CapabilityArray)},
                          {"spirv_extensions", std::move(ExtensionArray)},
                          {"required_features", std::move(FeatureArray)}};
  SmallString<256> Temp;
  int FD;
  if (std::error_code EC =
          sys::fs::createUniqueFile(Output + ".%%%%%%.tmp", FD, Temp)) {
    fail(EC.message());
    return 1;
  }
  auto Remove = scope_exit([&] { sys::fs::remove(Temp); });
  {
    raw_fd_ostream File(FD, true);
    File.write((*Binary)->getBufferStart(), (*Binary)->getBufferSize());
  }
  std::error_code EC;
  ToolOutputFile JSONFile(Output + ".json", EC, sys::fs::OF_Text);
  if (EC) {
    fail(EC.message());
    return 1;
  }
  JSONFile.os() << formatv("{0:2}\n", json::Value(std::move(Reflection)));
  JSONFile.os().close();
  if (JSONFile.os().has_error()) {
    fail("cannot write shader reflection");
    return 1;
  }
  if (auto RenameError = sys::fs::rename(Temp, Output)) {
    fail(RenameError.message());
    return 1;
  }
  JSONFile.keep();
  return 0;
}

int main(int Argc, char **Argv) {
  InitLLVM X(Argc, Argv);
  InitializeAllTargetInfos();
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmPrinters();
  SmallVector<const char *> Arguments(Argv, Argv + Argc);
  Arguments.push_back("-spirv-ext=+SPV_KHR_shader_clock,+SPV_EXT_demote_to_helper_invocation,+SPV_KHR_physical_storage_buffer,+SPV_KHR_"
                      "vulkan_memory_model,+SPV_EXT_descriptor_indexing,+SPV_"
                      "KHR_shader_draw_parameters,+SPV_KHR_multiview,+SPV_KHR_"
                      "ray_query,+SPV_KHR_zero_initialize_workgroup_memory,+"
                      "SPV_KHR_8bit_storage,+SPV_KHR_16bit_storage,+SPV_EXT_shader_atomic_float_add,+SPV_EXT_shader_atomic_float_min_max");
  cl::ParseCommandLineOptions(Arguments.size(), Arguments.data(),
                              "C++ Vulkan shader linker\n");
  LLVMContext C;
  auto M = std::make_unique<Module>("C++ shader", C);
  Linker L(*M);
  for (const std::string &Input : Inputs) {
    SMDiagnostic D;
    auto Part = parseIRFile(Input, D, C);
    if (!Part) {
      D.print(Argv[0], errs());
      return 1;
    }
    if (Part->getTargetTriple() != Triple("spirv1.4-unknown-vulkan1.3")) {
      fail("input '" + Input + "' has an incompatible target triple");
      return 1;
    }
    if (L.linkInModule(std::move(Part)))
      return 1;
  }
  if (EntryName.empty() && !EmitLLVM) {
    SmallVector<std::string> EntryNames;
    for (Function &F : *M)
      if (F.hasFnAttribute("spirv.cxx.stage"))
        EntryNames.push_back(F.getName().str());
    if (EntryNames.size() > 1)
      return emitAllEntries(*M, EntryNames, Argv[0]);
  }
  Function *Entry = nullptr;
  Function *ExactEntry = M->getFunction(EntryName);
  if (ExactEntry && !ExactEntry->hasFnAttribute("spirv.cxx.stage"))
    ExactEntry = nullptr;
  for (Function &F : *M) {
    if (!F.hasFnAttribute("spirv.cxx.stage") ||
        (!EntryName.empty() && F.getName() != EntryName &&
         (ExactEntry ||
          F.getFnAttribute("spirv.cxx.source_name").getValueAsString() !=
              EntryName)))
      continue;
    if (Entry) {
      fail("multiple entry points; select one with -fspirv-entry=<name>");
      return 1;
    }
    Entry = &F;
  }
  if (!Entry) {
    fail("no matching C++ shader entry point");
    return 1;
  }
  std::string Stage =
      Entry->getFnAttribute("spirv.cxx.stage").getValueAsString().str();
  if (Stage != "compute" && Stage != "vertex" && Stage != "fragment") {
    fail("backend lowering for stage '" + Stage + "' is not implemented yet");
    return 1;
  }
  if (!legalize(*M, *Entry))
    return 1;
  if (EmitLLVM) {
    std::error_code EC;
    ToolOutputFile File(Output, EC, sys::fs::OF_Text);
    if (EC) {
      fail(EC.message());
      return 1;
    }
    M->print(File.os(), nullptr);
    File.keep();
    return 0;
  }
  json::Object Reflection{
      {"schema_version", 1},
      {"entry", Entry->getName().str()},
      {"source_entry",
       Entry->getFnAttribute("spirv.cxx.source_name").getValueAsString().str()},
      {"stage", Stage},
      {"target", "spirv1.4-unknown-vulkan1.3"},
      {"interfaces", reflectInterfaces(*M)},
      {"required_limits", reflectLimits(*M, *Entry)},
      {"specialization_constants", reflectSpecializations(*Entry)}};
  if (Entry->hasFnAttribute("hlsl.numthreads")) {
    SmallVector<StringRef> Dimensions;
    Entry->getFnAttribute("hlsl.numthreads")
        .getValueAsString()
        .split(Dimensions, ',');
    json::Array Size;
    for (auto Dim : Dimensions) {
      unsigned N = 0;
      Dim.getAsInteger(10, N);
      Size.push_back(N);
    }
    Reflection["workgroup_size"] = std::move(Size);
  }
  std::set<std::string> AtomicFeatures;
  for (Instruction &I : instructions(*Entry))
    if (auto *Call = dyn_cast<CallInst>(&I))
      if (auto *F = Call->getCalledFunction())
        if (F->getFnAttribute("spirv.cxx.intrinsic")
                .getValueAsString()
                .starts_with("Atomic")) {
          StringRef Op = F->getFnAttribute("spirv.cxx.intrinsic").getValueAsString();
          if (Op.starts_with("AtomicF") && Call->getType()->isFloatingPointTy()) {
            bool Shared = Call->getArgOperand(0)->getType()->getPointerAddressSpace() == 3;
            unsigned Width = Call->getType()->getScalarSizeInBits();
            AtomicFeatures.insert((Twine(Shared ? "shaderSharedFloat" : "shaderBufferFloat") + Twine(Width) + (Op == "AtomicFAddEXT" ? "AtomicAdd" : "AtomicMinMax")).str());
          }
          bool Wide = Call->getType()->isIntegerTy(64);
          for (Value *A : Call->args())
            Wide |= A->getType()->isIntegerTy(64);
          if (Wide && Call->getArgOperand(0)->getType()->isPointerTy())
            AtomicFeatures.insert(
                Call->getArgOperand(0)->getType()->getPointerAddressSpace() == 3
                    ? "shaderSharedInt64Atomics"
                    : "shaderBufferInt64Atomics");
        }
  SmallVector<ShaderAsmSite> AsmSites;
  if (!collectShaderAsm(*Entry, AsmSites))
    return 1;
  std::string Error;
  const Target *T = TargetRegistry::lookupTarget(M->getTargetTriple(), Error);
  if (!T) {
    fail(Error);
    return 1;
  }
  TargetOptions Options;
  if (CodeGenOptimizationLevel > 3) {
    fail("codegen-opt-level must be between 0 and 3");
    return 1;
  }
  CodeGenOptLevel OptLevel = CodeGenOptLevel::Default;
  if (CodeGenOptimizationLevel == 0)
    OptLevel = CodeGenOptLevel::None;
  else if (CodeGenOptimizationLevel == 1)
    OptLevel = CodeGenOptLevel::Less;
  else if (CodeGenOptimizationLevel == 3)
    OptLevel = CodeGenOptLevel::Aggressive;
  std::unique_ptr<TargetMachine> TM(T->createTargetMachine(
      M->getTargetTriple(), "", "", Options, std::nullopt, std::nullopt,
      OptLevel));
  SmallVector<char> Binary;
  raw_svector_ostream Stream(Binary);
  legacy::PassManager Codegen;
  Codegen.add(new TargetLibraryInfoWrapperPass(
      TargetLibraryInfoImpl(M->getTargetTriple())));
  if (TM->addPassesToEmitFile(Codegen, Stream, nullptr,
                              CodeGenFileType::ObjectFile)) {
    fail("SPIR-V backend cannot emit an object file");
    return 1;
  }
  Codegen.run(*M);
  if (Binary.size() < 20 ||
      support::endian::read32le(Binary.data()) != 0x07230203 ||
      support::endian::read32le(Binary.data() + 4) != 0x00010400) {
    fail("backend did not produce SPIR-V 1.4");
    return 1;
  }
  SmallString<256> AssemblyToolDirectory(
      sys::fs::getMainExecutable(Argv[0], reinterpret_cast<void *>(&fail)));
  sys::path::remove_filename(AssemblyToolDirectory);
  if (!expandShaderAsm(Binary, AsmSites, AssemblyToolDirectory))
    return 1;
  json::Array Capabilities, Extensions;
  std::set<std::string> Features{"bufferDeviceAddress", "descriptorIndexing",
                                 "vulkanMemoryModel", "scalarBlockLayout"};
  DenseMap<unsigned, uint32_t> ConstantValues;
  for (size_t Offset = 20; Offset < Binary.size();) {
    uint32_t Header = support::endian::read32le(Binary.data() + Offset);
    unsigned Count = Header >> 16, Opcode = Header & 0xffff;
    if (!Count || Offset + Count * 4 > Binary.size()) {
      fail("malformed SPIR-V instruction");
      return 1;
    }
    if (Opcode == 43 && Count == 4)
      ConstantValues[support::endian::read32le(Binary.data() + Offset + 8)] =
          support::endian::read32le(Binary.data() + Offset + 12);
    if (Opcode == 5056 && Count == 4) {
      unsigned Scope = ConstantValues.lookup(support::endian::read32le(Binary.data() + Offset + 12));
      Features.insert(Scope == 1 ? "shaderDeviceClock" : "shaderSubgroupClock");
    }
    if (Opcode == 17 && Count == 2) {
      unsigned Cap = support::endian::read32le(Binary.data() + Offset + 4);
      Capabilities.push_back(Cap);
      switch (Cap) {
      case 2:
        Features.insert("geometryShader");
        break;
      case 4427:
        Features.insert("shaderDrawParameters");
        break;
      case 4439:
        Features.insert("multiview");
        break;
      case 9:
        Features.insert("shaderFloat16");
        break;
      case 10:
        Features.insert("shaderFloat64");
        break;
      case 11:
        Features.insert("shaderInt64");
        break;
      case 6033:
      case 6034:
      case 5612:
      case 5613:
      case 12:
        Features.insert(AtomicFeatures.begin(), AtomicFeatures.end());
        break;
      case 22:
        Features.insert("shaderInt16");
        break;
      case 35:
        Features.insert("sampleRateShading");
        break;
      case 39:
        Features.insert("shaderInt8");
        break;
      case 5346:
        Features.insert("vulkanMemoryModelDeviceScope");
        break;
      case 5302:
        Features.insert("runtimeDescriptorArray");
        break;
      case 5306:
        Features.insert("shaderUniformBufferArrayNonUniformIndexing");
        break;
      case 5307:
        Features.insert("shaderSampledImageArrayNonUniformIndexing");
        break;
      case 5308:
        Features.insert("shaderStorageBufferArrayNonUniformIndexing");
        break;
      case 5309:
        Features.insert("shaderStorageImageArrayNonUniformIndexing");
        break;
      case 5310:
        Features.insert("shaderInputAttachmentArrayNonUniformIndexing");
        break;
      case 5311:
        Features.insert("shaderUniformTexelBufferArrayNonUniformIndexing");
        break;
      case 5312:
        Features.insert("shaderStorageTexelBufferArrayNonUniformIndexing");
        break;
      case 5379:
        Features.insert("shaderDemoteToHelperInvocation");
        break;
      case 4472:
        Features.insert("rayQuery");
        Features.insert("accelerationStructure");
        break;
      case 49:
        Features.insert("shaderStorageImageExtendedFormats");
        break;
      case 55:
        Features.insert("shaderStorageImageReadWithoutFormat");
        break;
      case 56:
        Features.insert("shaderStorageImageWriteWithoutFormat");
        break;
      case 4433:
        Features.insert("storageBuffer16BitAccess");
        break;
      case 4434:
        Features.insert("uniformAndStorageBuffer16BitAccess");
        break;
      case 4435:
        Features.insert("storagePushConstant16");
        break;
      case 4436:
        Features.insert("storageInputOutput16");
        break;
      case 4448:
        Features.insert("storageBuffer8BitAccess");
        break;
      case 4449:
        Features.insert("uniformAndStorageBuffer8BitAccess");
        break;
      case 4450:
        Features.insert("storagePushConstant8");
        break;
      case 34:
      case 45:
        Features.insert("imageCubeArray");
        break;
      case 27:
        Features.insert("shaderStorageImageMultisample");
        break;

      default:
        break;
      }
    } else if (Opcode == 10) {
      std::string Extension(Binary.data() + Offset + 4);
      if (Extension == "SPV_KHR_zero_initialize_workgroup_memory")
        Features.insert("shaderZeroInitializeWorkgroupMemory");
      Extensions.push_back(std::move(Extension));
    }
    Offset += Count * 4;
  }
  Reflection["spirv_capabilities"] = std::move(Capabilities);
  Reflection["spirv_extensions"] = std::move(Extensions);
  json::Array RequiredFeatures;
  for (const auto &Feature : Features)
    RequiredFeatures.push_back(Feature);
  Reflection["required_features"] = std::move(RequiredFeatures);

  SmallString<256> Temp;
  int FD;
  if (std::error_code EC =
          sys::fs::createUniqueFile(Output + ".%%%%%%.tmp", FD, Temp)) {
    fail(EC.message());
    return 1;
  }
  auto Remove = scope_exit([&] { sys::fs::remove(Temp); });
  {
    raw_fd_ostream File(FD, true);
    File.write(Binary.data(), Binary.size());
  }
  SmallString<256> OwnDir(
      sys::fs::getMainExecutable(Argv[0], reinterpret_cast<void *>(&fail)));
  sys::path::remove_filename(OwnDir);
  auto Program = Validator.empty()
                     ? sys::findProgramByName("spirv-val", {OwnDir})
                     : ErrorOr<std::string>(Validator.getValue());
  if (!Program) {
    Program = sys::findProgramByName("spirv-val");
    if (!Program) {
      fail("spirv-val is required; install SPIR-V Tools beside Clang or on "
           "PATH");
      return 1;
    }
  }
  SmallVector<StringRef> ValidateArgs{*Program, "--target-env", "vulkan1.3",
                                      "--scalar-block-layout", Temp};
  if (sys::ExecuteAndWait(*Program, ValidateArgs) != 0) {
    fail("Vulkan shader validation failed; output was not published");
    return 1;
  }
  std::error_code EC;
  ToolOutputFile JSONFile(Output + ".json", EC, sys::fs::OF_Text);
  if (EC) {
    fail(EC.message());
    return 1;
  }
  JSONFile.os() << formatv("{0:2}\n", json::Value(std::move(Reflection)));
  JSONFile.os().close();
  if (JSONFile.os().has_error()) {
    fail("cannot write shader reflection");
    return 1;
  }
  if (auto EC = sys::fs::rename(Temp, Output)) {
    fail(EC.message());
    return 1;
  }
  JSONFile.keep();
  return 0;
}
