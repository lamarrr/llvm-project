//===-- ClangSPIRVLink.cpp - C++ shader linking and legalization ----------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/ScopeExit.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Bitcode/BitcodeWriter.h"
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
#include "llvm/Support/MemoryBuffer.h"
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
      return fail("unexpected raw LLVM inline assembly; compile shader asm "
                  "with -fspirv-shader");
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
  Instruction *Insert = &*Entry.getEntryBlock().getFirstInsertionPt();
  while (auto *Call = dyn_cast<CallInst>(Insert)) {
    auto *F = Call->getCalledFunction();
    if (!F || F->getFnAttribute("spirv.cxx.intrinsic").getValueAsString() !=
                  "SpecConstant")
      break;
    Insert = Insert->getNextNode();
  }
  IRBuilder<> B(Insert);
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
          ((Stage == "compute" || Stage == "task" || Stage == "mesh") &&
           ((ID >= 24 && ID <= 29) || ID == 38 || ID == 40)) ||
          (Stage == "vertex" &&
           (ID == 42 || ID == 43 || ID == 4424 || ID == 4425 || ID == 4426)) ||
          (Stage == "fragment" &&
           (ID == 7 || (ID >= 15 && ID <= 19) || ID == 23 || ID == 4444 ||
            ID == 5286 || ID == 5287)) ||
          (Stage == "geometry" && (ID == 7 || ID == 8)) ||
          (Stage == "tessellation_control" &&
           (ID == 7 || ID == 8 || ID == 14)) ||
          (Stage == "tessellation_evaluation" &&
           (ID == 7 || ID == 11 || ID == 12 || ID == 13 || ID == 14)) ||
          ((Stage == "ray_generation" || Stage == "intersection" ||
            Stage == "any_hit" || Stage == "closest_hit" || Stage == "miss" ||
            Stage == "callable") &&
           (ID == 5319 || ID == 5320)) ||
          ((Stage == "intersection" || Stage == "any_hit" ||
            Stage == "closest_hit" || Stage == "miss") &&
           (ID == 5321 || ID == 5322 || ID == 5325 || ID == 5326 ||
            ID == 5351)) ||
          ((Stage == "intersection" || Stage == "any_hit" ||
            Stage == "closest_hit") &&
           (ID == 7 || (ID >= 5323 && ID <= 5324) || ID == 5327 ||
            ID == 5352)) ||
          ((Stage == "any_hit" || Stage == "closest_hit") && ID == 5333) ||
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
        if (auto SizeIDsAttr =
                Entry.getFnAttribute("spirv.cxx.workgroup_size_ids");
            SizeIDsAttr.isValid()) {
          SmallVector<StringRef> IDs;
          SizeIDsAttr.getValueAsString().split(IDs, ',');
          if (IDs.size() != 3)
            return fail("invalid workgroup specialization IDs");
          SmallVector<Value *> Components;
          for (StringRef IDText : IDs) {
            unsigned SpecID;
            if (IDText.getAsInteger(10, SpecID))
              return fail("invalid workgroup specialization ID");
            CallInst *Component = nullptr;
            for (Instruction &I : instructions(Entry))
              if (auto *Call = dyn_cast<CallInst>(&I))
                if (auto *F = Call->getCalledFunction())
                  if (F->getFnAttribute("spirv.cxx.intrinsic")
                              .getValueAsString() == "SpecConstant" &&
                      cast<ConstantInt>(Call->getArgOperand(0))
                              ->getZExtValue() == SpecID) {
                    Component = Call;
                    break;
                  }
            if (!Component)
              return fail("missing workgroup specialization constant");
            Components.push_back(Component);
          }
          Type *VectorType = FixedVectorType::get(Array->getElementType(), 3);
          auto Callee =
              M.getOrInsertFunction("_Z29__spirv_SpecConstantCompositeILy0EEvv",
                                    FunctionType::get(VectorType,
                                                      {Array->getElementType(),
                                                       Array->getElementType(),
                                                       Array->getElementType()},
                                                      false));
          auto *F = cast<Function>(Callee.getCallee());
          F->addFnAttr("spirv.cxx.intrinsic", "SpecConstantComposite");
          F->addFnAttr(Attribute::NoUnwind);
          V = B.CreateCall(F, Components);
        }
      } else {
        auto *Input = new GlobalVariable(M, InterfaceType, false,
                                         GlobalValue::ExternalLinkage, nullptr,
                                         G->getName() + ".input", nullptr,
                                         GlobalVariable::NotThreadLocal, 7);
        decorate(Input, 11, ID);
        if (ID == 11 || ID == 12)
          Input->setMetadata(
              "spirv.Decorations",
              MDNode::get(
                  M.getContext(),
                  {MDNode::get(M.getContext(), {number(M.getContext(), 11),
                                                number(M.getContext(), ID)}),
                   MDNode::get(M.getContext(), {number(M.getContext(), 15)})}));
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
    if (AS != 2 && AS != 7 && AS != 8 && AS != 12 && AS != 13 && AS != 16 &&
        !(AS >= 17 && AS <= 23))
      continue;
    Type *SourceType = G.getValueType();
    if (auto *N = G.getMetadata("spirv.cxx.source_interface"))
      SourceType = cast<ValueAsMetadata>(N->getOperand(0))->getType();
    auto R = AS == 2 || AS == 16 ? json::Object{}
                                 : typeLayout(SourceType, M.getDataLayout());
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
    R["kind"] = AS == 17   ? "task_payload"
                : AS == 18 ? "ray_payload"
                : AS == 19 ? "incoming_ray_payload"
                : AS == 20 ? "hit_attribute"
                : AS == 21 ? "callable_data"
                : AS == 22 ? "incoming_callable_data"
                : AS == 23 ? "shader_record"
                : AS == 16 ? "tile_image"
                : AS == 2  ? "descriptor"
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
                           : Decoration == 43 ? "input_attachment_index"
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

static bool specializeWorkgroupSize(SmallVectorImpl<char> &Binary,
                                    const Function &Entry) {
  auto Attribute = Entry.getFnAttribute("spirv.cxx.workgroup_size_ids");
  if (!Attribute.isValid())
    return true;
  SmallVector<StringRef> Parts;
  Attribute.getValueAsString().split(Parts, ',');
  if (Parts.size() != 3)
    return fail("invalid workgroup specialization IDs");
  uint32_t IDs[3];
  for (unsigned I = 0; I != 3; ++I)
    if (Parts[I].getAsInteger(10, IDs[I]))
      return fail("invalid workgroup specialization ID");

  DenseMap<uint32_t, uint32_t> SpecConstants;
  size_t LocalSizeOffset = 0;
  for (size_t Offset = 20; Offset < Binary.size();) {
    uint32_t Header = support::endian::read32le(Binary.data() + Offset);
    unsigned Count = Header >> 16, Opcode = Header & 0xffff;
    if (!Count || Offset + Count * 4 > Binary.size())
      return fail("malformed SPIR-V instruction");
    if (Opcode == 71 && Count == 4 &&
        support::endian::read32le(Binary.data() + Offset + 8) == 1)
      SpecConstants[support::endian::read32le(Binary.data() + Offset + 12)] =
          support::endian::read32le(Binary.data() + Offset + 4);
    if (Opcode == 16 && Count == 6 &&
        support::endian::read32le(Binary.data() + Offset + 8) == 17)
      LocalSizeOffset = Offset;
    Offset += Count * 4;
  }
  if (!LocalSizeOffset)
    return fail("workgroup specialization requires LocalSize execution mode");
  for (unsigned I = 0; I != 3; ++I)
    if (!SpecConstants.count(IDs[I]))
      return fail("missing workgroup specialization constant ID " +
                  Twine(IDs[I]));
  support::endian::write32le(Binary.data() + LocalSizeOffset,
                             (6u << 16) | 331u);
  support::endian::write32le(Binary.data() + LocalSizeOffset + 8, 38u);
  for (unsigned I = 0; I != 3; ++I)
    support::endian::write32le(Binary.data() + LocalSizeOffset + 12 + I * 4,
                               SpecConstants.lookup(IDs[I]));
  return true;
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
      Result = B.CreateSelect(
          SameBase, Offsets,
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

static bool legalizeFieldInterfaces(Module &M) {
  SmallVector<GlobalVariable *> Groups;
  for (GlobalVariable &Global : M.globals())
    if (Global.getMetadata("spirv.cxx.field_interfaces"))
      Groups.push_back(&Global);
  for (GlobalVariable *Group : Groups) {
    Type *Original = Group->getValueType();
    bool PerVertex = Group->getMetadata("spirv.cxx.array_interface") != nullptr;
    auto *Outer = PerVertex ? dyn_cast<ArrayType>(Original) : nullptr;
    if (PerVertex && !Outer)
      return fail("per-vertex interface records require an outer array");
    struct FieldInterface {
      SmallVector<unsigned> Path;
      GlobalVariable *Global;
    };
    SmallVector<FieldInterface> Fields;
    for (const auto &Operand :
         Group->getMetadata("spirv.cxx.field_interfaces")->operands()) {
      auto *Description = cast<MDNode>(Operand.get());
      auto *MemberDecorations = dyn_cast<MDNode>(Description->getOperand(2));
      Type *Leaf = Outer ? Outer->getElementType() : Original;
      FieldInterface Field;
      for (unsigned Index = MemberDecorations ? 3 : 2;
           Index != Description->getNumOperands(); ++Index) {
        unsigned Member = mdNumber(Description, Index);
        Field.Path.push_back(Member);
        auto *Record = dyn_cast<StructType>(Leaf);
        if (!Record || Member >= Record->getNumElements())
          return fail("invalid explicit interface member path");
        Leaf = Record->getElementType(Member);
      }
      Type *FieldType =
          Outer ? ArrayType::get(Leaf, Outer->getNumElements()) : Leaf;
      StringRef Name = cast<MDString>(Description->getOperand(1))->getString();
      Field.Global = new GlobalVariable(
          M, FieldType, false, Group->getLinkage(), nullptr,
          Group->getName() + "." + Name + "." + Twine(Fields.size()), nullptr,
          GlobalVariable::NotThreadLocal, Group->getAddressSpace());
      Field.Global->copyMetadata(Group, 0);
      Field.Global->setMetadata("spirv.cxx.field_interfaces", nullptr);
      Field.Global->setAlignment(M.getDataLayout().getABITypeAlign(Leaf));
      Field.Global->setMetadata(
          "spirv.cxx.pointee",
          MDNode::get(M.getContext(),
                      ConstantAsMetadata::get(PoisonValue::get(FieldType))));
      SmallVector<Metadata *> Decorations;
      if (auto *Existing = Group->getMetadata("spirv.Decorations"))
        for (const auto &Decoration : Existing->operands()) {
          unsigned Kind = mdNumber(cast<MDNode>(Decoration.get()), 0);
          if (MemberDecorations &&
              (Kind == 13 || Kind == 14 || Kind == 16 || Kind == 17))
            continue;
          Decorations.push_back(Decoration.get());
        }
      if (MemberDecorations)
        for (const auto &Decoration : MemberDecorations->operands())
          Decorations.push_back(Decoration.get());
      Decorations.push_back(MDNode::get(
          M.getContext(), {number(M.getContext(), 30),
                           number(M.getContext(), mdNumber(Description, 0))}));
      Field.Global->setMetadata("spirv.Decorations",
                                MDNode::get(M.getContext(), Decorations));
      Fields.push_back(std::move(Field));
    }
    auto ResolveAddress =
        [&](IRBuilder<> &Builder,
            ArrayRef<Value *> Path) -> std::pair<Value *, Type *> {
      unsigned Start = Outer ? 1 : 0;
      for (const auto &Field : Fields) {
        if (Path.size() < Start + Field.Path.size())
          continue;
        bool Match = true;
        for (unsigned Index = 0; Index != Field.Path.size(); ++Index) {
          auto *Member = dyn_cast<ConstantInt>(Path[Start + Index]);
          Match &= Member && Member->getZExtValue() == Field.Path[Index];
        }
        if (!Match)
          continue;
        SmallVector<Value *> Indices{Builder.getInt32(0)};
        if (Outer)
          Indices.push_back(Path[0]);
        Indices.append(Path.begin() + Start + Field.Path.size(), Path.end());
        Type *Leaf = GetElementPtrInst::getIndexedType(
            Field.Global->getValueType(), Indices);
        if (!Leaf)
          return {nullptr, nullptr};
        if (Indices.size() == 1)
          return {Field.Global, Leaf};
        IRBuilder<NoFolder> Typed(Builder.GetInsertPoint());
        return {Typed.CreateGEP(Field.Global->getValueType(), Field.Global,
                                Indices),
                Leaf};
      }
      return {nullptr, nullptr};
    };
    DenseMap<Value *, SmallVector<Value *>> Paths;
    Paths[Group] = {};
    SmallVector<Value *> Pointers{Group};
    SmallVector<GetElementPtrInst *> OldPointers;
    while (!Pointers.empty()) {
      Value *Pointer = Pointers.pop_back_val();
      SmallVector<Value *> Base = Paths.lookup(Pointer);
      SmallVector<User *> Users(Pointer->users());
      for (User *User : Users) {
        if (auto *GEP = dyn_cast<GetElementPtrInst>(User)) {
          SmallVector<Value *> Path = Base;
          auto *First = dyn_cast<ConstantInt>(*GEP->idx_begin());
          if (!First || !First->isZero()) {
            if (Path.empty())
              return fail("explicit interface record pointer arithmetic is "
                          "unsupported");
            SmallVector<Value *> Prefix{
                ConstantInt::get(Type::getInt32Ty(M.getContext()), 0)};
            Prefix.append(Path.begin(), Path.end() - 1);
            Type *Parent = GetElementPtrInst::getIndexedType(Original, Prefix);
            if (!Parent || (!Parent->isArrayTy() && !Parent->isVectorTy()))
              return fail(
                  "explicit interface member pointer escapes its array");
            IRBuilder<> Builder(GEP);
            Path.back() = Builder.CreateAdd(
                Builder.CreateIntCast(Path.back(), Builder.getInt32Ty(), false),
                Builder.CreateIntCast(*GEP->idx_begin(), Builder.getInt32Ty(),
                                      false));
          }
          for (Value *Index : drop_begin(GEP->indices()))
            Path.push_back(Index);
          Paths[GEP] = std::move(Path);
          Pointers.push_back(GEP);
          OldPointers.push_back(GEP);
          continue;
        }
        auto *Load = dyn_cast<LoadInst>(User);
        auto *Store = dyn_cast<StoreInst>(User);
        if (!Load && (!Store || Store->getPointerOperand() != Pointer))
          return fail("unsupported explicit interface record pointer escape");
        IRBuilder<> Builder(cast<Instruction>(User));
        Value *Stored = Store ? Store->getValueOperand() : nullptr;
        Type *Access = Load ? Load->getType() : Stored->getType();
        bool Valid = true;
        auto Transfer = [&](auto &&Self, Type *ValueType, Value *Stored,
                            SmallVector<Value *> Path) -> Value * {
          SmallVector<Value *> SourceIndices{Builder.getInt32(0)};
          SourceIndices.append(Path.begin(), Path.end());
          Type *Cursor =
              GetElementPtrInst::getIndexedType(Original, SourceIndices);
          while (Cursor && Cursor != ValueType &&
                 (Cursor->isStructTy() || Cursor->isArrayTy() ||
                  Cursor->isVectorTy())) {
            if (ValueType->isVectorTy() && Cursor->isArrayTy() &&
                Cursor->getArrayNumElements() ==
                    cast<FixedVectorType>(ValueType)->getNumElements() &&
                Cursor->getArrayElementType() == ValueType->getScalarType())
              break;
            Path.push_back(Builder.getInt32(0));
            Cursor = Cursor->isStructTy()  ? Cursor->getStructElementType(0)
                     : Cursor->isArrayTy() ? Cursor->getArrayElementType()
                                           : Cursor->getScalarType();
          }
          if (ValueType->isIntegerTy() || ValueType->isFloatingPointTy()) {
            auto [Address, Leaf] = ResolveAddress(Builder, Path);
            if (!Address || !Leaf->isSingleValueType()) {
              Valid = false;
              return PoisonValue::get(ValueType);
            }
            auto Convert = [&](Value *Value, Type *To) {
              if (Value->getType() == To)
                return Value;
              if (Value->getType()->isIntegerTy() && To->isIntegerTy())
                return Builder.CreateIntCast(Value, To, false);
              return Builder.CreateBitCast(Value, To);
            };
            Align Alignment =
                std::min(Store ? Store->getAlign() : Load->getAlign(),
                         M.getDataLayout().getABITypeAlign(Leaf));
            if (Store) {
              Builder.CreateAlignedStore(Convert(Stored, Leaf), Address,
                                         Alignment, Store->isVolatile());
              return nullptr;
            }
            return Convert(Builder.CreateAlignedLoad(Leaf, Address, Alignment,
                                                     Load->isVolatile()),
                           ValueType);
          }
          if (!ValueType->isStructTy() && !ValueType->isArrayTy() &&
              !ValueType->isVectorTy()) {
            Valid = false;
            return PoisonValue::get(ValueType);
          }
          unsigned Count =
              ValueType->isStructTy() ? ValueType->getStructNumElements()
              : ValueType->isArrayTy()
                  ? ValueType->getArrayNumElements()
                  : cast<FixedVectorType>(ValueType)->getNumElements();
          Value *Result = PoisonValue::get(ValueType);
          for (unsigned Index = 0; Index != Count; ++Index) {
            Type *Element =
                ValueType->isStructTy() ? ValueType->getStructElementType(Index)
                : ValueType->isArrayTy() ? ValueType->getArrayElementType()
                                         : ValueType->getScalarType();
            Value *Part =
                Store ? (ValueType->isVectorTy()
                             ? Builder.CreateExtractElement(Stored, Index)
                             : Builder.CreateExtractValue(Stored, Index))
                      : nullptr;
            Path.push_back(Builder.getInt32(Index));
            Part = Self(Self, Element, Part, Path);
            Path.pop_back();
            if (Load)
              Result = ValueType->isVectorTy()
                           ? Builder.CreateInsertElement(Result, Part, Index)
                           : Builder.CreateInsertValue(Result, Part, Index);
          }
          return Result;
        };
        Value *Result = Transfer(Transfer, Access, Stored, Base);
        if (!Valid)
          return fail("explicit interface record access has no located member");
        if (Load)
          Load->replaceAllUsesWith(Result);
        cast<Instruction>(User)->eraseFromParent();
      }
    }
    for (auto *Pointer : reverse(OldPointers))
      Pointer->eraseFromParent();
    if (auto *Interfaces = M.getNamedMetadata("spirv.cxx.interfaces")) {
      SmallVector<MDNode *> Descriptions;
      for (MDNode *Description : Interfaces->operands()) {
        auto *Reference =
            dyn_cast_or_null<ValueAsMetadata>(Description->getOperand(1));
        if (!Reference)
          continue;
        if (Reference->getValue() != Group) {
          Descriptions.push_back(Description);
          continue;
        }
        for (const auto &Field : Fields)
          Descriptions.push_back(MDNode::get(
              M.getContext(),
              {Description->getOperand(0), ValueAsMetadata::get(Field.Global),
               Description->getOperand(2),
               number(M.getContext(), M.getDataLayout().getTypeAllocSize(
                                          Field.Global->getValueType())),
               number(M.getContext(), Field.Global->getAlign()->value())}));
      }
      Interfaces->clearOperands();
      for (MDNode *Description : Descriptions)
        Interfaces->addOperand(Description);
    }
    Group->eraseFromParent();
  }
  return true;
}

static bool legalizeBuiltinInterfaces(Module &M) {
  auto Components = [&](auto &&Self, Type *ValueType) -> uint64_t {
    if (auto *Record = dyn_cast<StructType>(ValueType)) {
      uint64_t Count = 0;
      for (auto *Element : Record->elements())
        Count += Self(Self, Element);
      return Count;
    }
    if (auto *Array = dyn_cast<ArrayType>(ValueType))
      return Array->getNumElements() * Self(Self, Array->getElementType());
    if (auto *Vector = dyn_cast<FixedVectorType>(ValueType))
      return Vector->getNumElements() * Self(Self, Vector->getElementType());
    return ValueType->isIntegerTy() || ValueType->isFloatingPointTy() ? 1 : 0;
  };
  SmallVector<GlobalVariable *> Interfaces;
  for (GlobalVariable &Global : M.globals())
    if (Global.getMetadata("spirv.cxx.builtin_interface"))
      Interfaces.push_back(&Global);
  for (GlobalVariable *Global : Interfaces) {
    Type *Original = Global->getValueType();
    auto *Metadata = Global->getMetadata("spirv.cxx.builtin_interface");
    Type *Native = cast<ConstantAsMetadata>(Metadata->getOperand(0))
                       ->getValue()
                       ->getType();
    if (Original == Native)
      continue;
    if (!Components(Components, Original) ||
        Components(Components, Original) != Components(Components, Native))
      return fail("builtin record layout does not match its semantic shape");
    std::string Suffix = ".builtin";
    if (auto *Record = dyn_cast<StructType>(Original))
      if (Record->getNumElements() == 1)
        if (auto *Array = dyn_cast<ArrayType>(Record->getElementType(0)))
          if (Array->getNumElements() == 4 &&
              Array->getElementType()->isFloatTy() &&
              Native ==
                  FixedVectorType::get(Type::getFloatTy(M.getContext()), 4))
            Suffix = ".position.interface";
    auto *Interface = new GlobalVariable(
        M, Native, false, Global->getLinkage(), nullptr,
        Global->getName() + Suffix, nullptr, GlobalVariable::NotThreadLocal,
        Global->getAddressSpace());
    Interface->copyMetadata(Global, 0);
    Interface->setAlignment(Global->getAlign());
    Interface->setMetadata(
        "spirv.cxx.source_interface",
        MDNode::get(M.getContext(),
                    ConstantAsMetadata::get(PoisonValue::get(Original))));
    Interface->setMetadata(
        "spirv.cxx.pointee",
        MDNode::get(M.getContext(),
                    ConstantAsMetadata::get(PoisonValue::get(Native))));
    struct ComponentOffset {
      uint64_t Constant = 0;
      SmallVector<std::pair<Value *, uint64_t>> Terms;
    };
    auto Materialize = [&](IRBuilder<> &Builder,
                           const ComponentOffset &Offset) {
      Value *Result = Builder.getInt32(Offset.Constant);
      for (auto [Index, Stride] : Offset.Terms)
        Result = Builder.CreateAdd(
            Result, Builder.CreateMul(Builder.CreateIntCast(
                                          Index, Builder.getInt32Ty(), false),
                                      Builder.getInt32(Stride)));
      return Result;
    };
    auto Address = [&](IRBuilder<> &Builder, ComponentOffset Offset) {
      SmallVector<Value *> Indices{Builder.getInt32(0)};
      Type *Cursor = Native;
      while (Cursor->isArrayTy() || Cursor->isVectorTy()) {
        Type *Element = Cursor->isArrayTy() ? Cursor->getArrayElementType()
                                            : Cursor->getScalarType();
        uint64_t Stride = Components(Components, Element);
        bool Divisible = llvm::all_of(
            Offset.Terms, [&](auto Term) { return Term.second % Stride == 0; });
        if (Divisible) {
          ComponentOffset Index;
          Index.Constant = Offset.Constant / Stride;
          for (auto [Term, Factor] : Offset.Terms)
            Index.Terms.push_back({Term, Factor / Stride});
          Indices.push_back(Materialize(Builder, Index));
          Offset.Constant %= Stride;
          Offset.Terms.clear();
        } else {
          Value *Flat = Materialize(Builder, Offset);
          Indices.push_back(Builder.CreateUDiv(Flat, Builder.getInt32(Stride)));
          Offset.Constant = 0;
          Offset.Terms = {
              {Builder.CreateURem(Flat, Builder.getInt32(Stride)), 1}};
        }
        Cursor = Element;
      }
      Value *Pointer = Interface;
      if (Indices.size() > 1) {
        IRBuilder<NoFolder> Typed(Builder.GetInsertPoint());
        Pointer = Typed.CreateGEP(Native, Interface, Indices);
      }
      return std::pair{Pointer, Cursor};
    };
    auto Convert = [&](IRBuilder<> &Builder, Value *Value, Type *To) {
      return Value->getType() == To ? Value
                                    : Builder.CreateIntCast(Value, To, false);
    };
    DenseMap<Value *, ComponentOffset> Offsets;
    Offsets[Global] = {};
    SmallVector<Value *> Pointers{Global};
    SmallVector<GetElementPtrInst *> OldPointers;
    while (!Pointers.empty()) {
      Value *Pointer = Pointers.pop_back_val();
      ComponentOffset Base = Offsets.lookup(Pointer);
      SmallVector<User *> Users(Pointer->users());
      for (User *User : Users) {
        if (auto *GEP = dyn_cast<GetElementPtrInst>(User)) {
          Type *Cursor = GEP->getSourceElementType();
          ComponentOffset Offset = Base;
          unsigned Position = 0;
          for (Value *Index : GEP->indices()) {
            uint64_t Stride = 0;
            if (Position++ == 0)
              Stride = Components(Components, Cursor);
            else if (auto *Record = dyn_cast<StructType>(Cursor)) {
              unsigned Field = cast<ConstantInt>(Index)->getZExtValue();
              uint64_t Prefix = 0;
              for (unsigned Member = 0; Member != Field; ++Member)
                Prefix +=
                    Components(Components, Record->getElementType(Member));
              Offset.Constant += Prefix;
              Cursor = Record->getElementType(Field);
              continue;
            } else {
              Cursor = Cursor->isArrayTy() ? Cursor->getArrayElementType()
                                           : Cursor->getScalarType();
              Stride = Components(Components, Cursor);
            }
            if (auto *Constant = dyn_cast<ConstantInt>(Index))
              Offset.Constant += Constant->getZExtValue() * Stride;
            else
              Offset.Terms.push_back({Index, Stride});
          }
          Offsets[GEP] = Offset;
          Pointers.push_back(GEP);
          OldPointers.push_back(GEP);
          continue;
        }
        auto *Load = dyn_cast<LoadInst>(User);
        auto *Store = dyn_cast<StoreInst>(User);
        if (!Load && (!Store || Store->getPointerOperand() != Pointer))
          return fail("unsupported builtin record pointer escape");
        IRBuilder<> Builder(cast<Instruction>(User));
        Value *ValueToStore = Store ? Store->getValueOperand() : nullptr;
        Type *Access = Load ? Load->getType() : ValueToStore->getType();
        uint64_t Component = 0;
        auto Transfer = [&](auto &&Self, Type *ValueType,
                            Value *ValueToStore) -> Value * {
          if (ValueType->isIntegerTy() || ValueType->isFloatingPointTy()) {
            ComponentOffset Offset = Base;
            Offset.Constant += Component++;
            auto [Pointer, Element] = Address(Builder, Offset);
            Align Alignment =
                std::min(Store ? Store->getAlign() : Load->getAlign(),
                         M.getDataLayout().getABITypeAlign(Element));
            if (Store) {
              Builder.CreateAlignedStore(
                  Convert(Builder, ValueToStore, Element), Pointer, Alignment,
                  Store->isVolatile());
              return nullptr;
            }
            return Convert(Builder,
                           Builder.CreateAlignedLoad(
                               Element, Pointer, Alignment, Load->isVolatile()),
                           ValueType);
          }
          unsigned Count =
              ValueType->isStructTy() ? ValueType->getStructNumElements()
              : ValueType->isArrayTy()
                  ? ValueType->getArrayNumElements()
                  : cast<FixedVectorType>(ValueType)->getNumElements();
          Value *Result = PoisonValue::get(ValueType);
          for (unsigned Index = 0; Index != Count; ++Index) {
            Type *Element =
                ValueType->isStructTy() ? ValueType->getStructElementType(Index)
                : ValueType->isArrayTy() ? ValueType->getArrayElementType()
                                         : ValueType->getScalarType();
            Value *Part =
                Store ? (ValueType->isVectorTy()
                             ? Builder.CreateExtractElement(ValueToStore, Index)
                             : Builder.CreateExtractValue(ValueToStore, Index))
                      : nullptr;
            Part = Self(Self, Element, Part);
            if (Load)
              Result = ValueType->isVectorTy()
                           ? Builder.CreateInsertElement(Result, Part, Index)
                           : Builder.CreateInsertValue(Result, Part, Index);
          }
          return Result;
        };
        Value *Result = Transfer(Transfer, Access, ValueToStore);
        if (Load)
          Load->replaceAllUsesWith(Result);
        cast<Instruction>(User)->eraseFromParent();
      }
    }
    for (auto *Pointer : reverse(OldPointers))
      Pointer->eraseFromParent();
    Global->replaceAllUsesWith(Interface);
    Global->eraseFromParent();
  }
  return true;
}

static bool legalize(Module &M, Function &Entry) {
  if (auto *Modes = M.getNamedMetadata("spirv.ExecutionMode")) {
    SmallVector<MDNode *> Selected;
    for (MDNode *Mode : Modes->operands())
      if (Mode->getOperand(0) == ValueAsMetadata::get(&Entry))
        Selected.push_back(Mode);
    Modes->clearOperands();
    for (MDNode *Mode : Selected)
      Modes->addOperand(Mode);
  }
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
  bool BeginInterlock = false, EndInterlock = false;
  for (Instruction &I : instructions(Entry))
    if (auto *Call = dyn_cast<CallBase>(&I))
      if (auto *F = Call->getCalledFunction()) {
        StringRef Op =
            F->getFnAttribute("spirv.cxx.intrinsic").getValueAsString();
        StringRef Stage =
            Entry.getFnAttribute("spirv.cxx.stage").getValueAsString();
        bool GeometryOp = Op == "EmitVertex" || Op == "EndPrimitive" ||
                          Op == "EmitStreamVertex" ||
                          Op == "EndStreamPrimitive";
        bool TraceOp = Op == "TraceRayKHR";
        bool CallableOp = Op == "ExecuteCallableKHR";
        bool HitOp = Op == "IgnoreIntersectionKHR" || Op == "TerminateRayKHR";
        if ((GeometryOp && Stage != "geometry") ||
            (Op == "SetMeshOutputsEXT" && Stage != "mesh") ||
            (Op == "EmitMeshTasksEXT" && Stage != "task") ||
            (TraceOp && Stage != "ray_generation" && Stage != "closest_hit" &&
             Stage != "miss") ||
            (CallableOp && Stage != "ray_generation" &&
             Stage != "closest_hit" && Stage != "miss" &&
             Stage != "callable") ||
            (Op == "ReportIntersectionKHR" && Stage != "intersection") ||
            (HitOp && Stage != "any_hit"))
          return fail("stage operation '" + Op + "' is unavailable in " +
                      Stage);
        if (TraceOp || CallableOp ||
            (Op == "EmitMeshTasksEXT" && Call->arg_size() == 4)) {
          unsigned Index = TraceOp ? 10 : CallableOp ? 1 : 3;
          if (!isa<GlobalVariable>(
                  Call->getArgOperand(Index)->stripPointerCasts()))
            return fail(
                "stage payload operands must name an interface variable");
        }
        BeginInterlock |= Op == "BeginInvocationInterlockEXT";
        EndInterlock |= Op == "EndInvocationInterlockEXT";
      }
  auto InterlockMode = Entry.getFnAttribute("spirv.cxx.interlock_mode");
  if (BeginInterlock || EndInterlock || InterlockMode.isValid()) {
    if (Entry.getFnAttribute("spirv.cxx.stage").getValueAsString() !=
        "fragment")
      return fail("fragment interlock requires a fragment entry");
    if (BeginInterlock != EndInterlock)
      return fail("fragment interlock requires both begin and end calls");
    unsigned Mode = 5366;
    if (InterlockMode.isValid() &&
        (InterlockMode.getValueAsString().getAsInteger(10, Mode) ||
         Mode < 5366 || Mode > 5371))
      return fail("invalid fragment interlock mode");
    M.getOrInsertNamedMetadata("spirv.ExecutionMode")
        ->addOperand(MDNode::get(
            M.getContext(), {ValueAsMetadata::get(&Entry),
                             ConstantAsMetadata::get(ConstantInt::get(
                                 Type::getInt32Ty(M.getContext()), Mode))}));
  }
  bool EarlyTests = false;
  if (auto *Modes = M.getNamedMetadata("spirv.ExecutionMode"))
    for (auto *Mode : Modes->operands())
      EarlyTests |= mdNumber(Mode, 1) == 9;
  for (Instruction &Instruction : instructions(Entry))
    if (auto *Call = dyn_cast<CallBase>(&Instruction))
      if (auto *Callee = Call->getCalledFunction()) {
        StringRef Operation =
            Callee->getFnAttribute("spirv.cxx.intrinsic").getValueAsString();
        bool DepthOrStencil = Operation == "DepthAttachmentReadEXT" ||
                              Operation == "StencilAttachmentReadEXT";
        if ((DepthOrStencil || Operation == "ColorAttachmentReadEXT") &&
            Entry.getFnAttribute("spirv.cxx.stage").getValueAsString() !=
                "fragment")
          return fail("tile attachment reads require a fragment entry");
        if (DepthOrStencil && EarlyTests)
          return fail("depth and stencil attachment reads cannot use early "
                      "fragment tests");
      }
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
      if (!G->getSourceElementType()->isIntegerTy(8) || G->getNumIndices() != 1)
        continue;
      while (auto *P = dyn_cast<GetElementPtrInst>(G->getPointerOperand())) {
        if (!P->getSourceElementType()->isIntegerTy(8) ||
            P->getNumIndices() != 1)
          break;
        IRBuilder<> B(G);
        Value *Offset =
            B.CreateAdd(B.CreateSExtOrTrunc(P->getOperand(1), B.getInt64Ty()),
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
  if (!legalizeFieldInterfaces(M) || !legalizeBuiltinInterfaces(M))
    return false;
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
      Type *Element = Self(Self, A->getElementType());
      if (Element != A->getElementType()) {
        Type *Result = ArrayType::get(Element, A->getNumElements());
        PackedTypes[T] = Result;
        return Result;
      }
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
      if (Members.size() == 1 && Members[0]->isVectorTy())
        Result = Members[0];
      else if (Changed)
        Result = StructType::get(M.getContext(), Members, S->isPacked());
    }
    PackedTypes[T] = Result;
    return Result;
  };
  SmallVector<GlobalVariable *> Interfaces;
  DenseMap<GlobalVariable *, Type *> InterfaceTypes;
  for (auto &G : M.globals())
    if (auto *Decorations = G.getMetadata("spirv.Decorations"))
      for (const auto &Operand : Decorations->operands()) {
        auto *Decoration = cast<MDNode>(Operand.get());
        if (mdNumber(Decoration, 0) == 11 && mdNumber(Decoration, 1) == 5299)
          PackedTypes[Type::getInt8Ty(M.getContext())] =
              Type::getInt1Ty(M.getContext());
      }
  for (auto &G : M.globals()) {
    auto *Decorations = G.getMetadata("spirv.Decorations");
    bool DistanceArray =
        Decorations &&
        llvm::any_of(Decorations->operands(), [](const MDOperand &Op) {
          auto *D = cast<MDNode>(Op.get());
          return mdNumber(D, 0) == 11 &&
                 (mdNumber(D, 1) == 3 || mdNumber(D, 1) == 4 ||
                  mdNumber(D, 1) == 11 || mdNumber(D, 1) == 12 ||
                  mdNumber(D, 1) == 20 || mdNumber(D, 1) == 5294);
        });
    Type *NewType = PackType(PackType, G.getValueType());
    if (G.getMetadata("spirv.cxx.array_interface"))
      if (auto *Array = dyn_cast<ArrayType>(G.getValueType()))
        NewType = ArrayType::get(PackType(PackType, Array->getElementType()),
                                 Array->getNumElements());
    if ((G.getAddressSpace() == 7 || G.getAddressSpace() == 8) &&
        !DistanceArray && !G.getMetadata("spirv.cxx.builtin_interface") &&
        NewType != G.getValueType()) {
      InterfaceTypes[&G] = NewType;
      Interfaces.push_back(&G);
    }
  }
  for (auto *G : Interfaces) {
    Type *OldType = G->getValueType();
    Type *OrdinaryPacked = PackType(PackType, OldType);
    bool Interpolated = llvm::any_of(G->users(), [](User *Use) {
      auto *Call = dyn_cast<CallBase>(Use);
      return Call && Call->getCalledFunction() &&
             Call->getCalledFunction()
                 ->getFnAttribute("spirv.cxx.intrinsic")
                 .getValueAsString()
                 .starts_with("InterpolateAt");
    });
    if (Interpolated)
      if (auto *Record = dyn_cast<StructType>(OrdinaryPacked))
        if (Record->getNumElements() == 1 &&
            isa<FixedVectorType>(Record->getElementType(0)))
          PackedTypes[OldType] = Record->getElementType(0);
    if (G->getMetadata("spirv.cxx.array_interface"))
      PackedTypes[OldType] = InterfaceTypes.lookup(G);
    Type *NewType = PackType(PackType, OldType);
    auto *Output = new GlobalVariable(M, NewType, false, G->getLinkage(),
                                      nullptr, G->getName() + ".interface",
                                      nullptr, GlobalVariable::NotThreadLocal,
                                      G->getAddressSpace());
    Output->copyMetadata(G, 0);
    Output->setAlignment(G->getAlign());
    if (!Output->getMetadata("spirv.cxx.source_interface"))
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
      if (From->isIntegerTy() && To->isIntegerTy())
        return B.CreateIntCast(V, To, false);
      if (auto *Record = dyn_cast<StructType>(From))
        if (Record->getNumElements() == 1 && To->isVectorTy())
          return Self(Self, B, B.CreateExtractValue(V, 0), To);
      if (auto *Record = dyn_cast<StructType>(To))
        if (Record->getNumElements() == 1 && From->isVectorTy())
          return B.CreateInsertValue(
              PoisonValue::get(To), Self(Self, B, V, Record->getElementType(0)),
              0);
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
        } else if (auto *Call = dyn_cast<CallBase>(U);
                   Call && Pointer == G && Call->getCalledFunction() &&
                   Call->getCalledFunction()
                       ->getFnAttribute("spirv.cxx.intrinsic")
                       .getValueAsString()
                       .starts_with("InterpolateAt")) {
          IRBuilder<> B(Call);
          Value *Interpolant = Output;
          Type *InterpolantType = NewPointee;
          while (auto *Record = dyn_cast<StructType>(InterpolantType)) {
            if (Record->getNumElements() != 1)
              return fail("interpolation requires a homogeneous interface");
            Interpolant = B.CreateStructGEP(InterpolantType, Interpolant, 0);
            InterpolantType = Record->getElementType(0);
          }
          if (InterpolantType != Call->getType())
            return fail("interpolation result does not match its input");
          for (unsigned I = 0; I < Call->arg_size(); ++I)
            if (Call->getArgOperand(I) == G)
              Call->setArgOperand(I, Interpolant);
        } else
          return fail("unsupported stage interface pointer escape");
      }
    }
    // Update typed GEPs only after all original pointees have been inspected.
    for (Instruction &I : make_early_inc_range(instructions(Entry)))
      if (auto *P = dyn_cast<GetElementPtrInst>(&I)) {
        Value *Root = P->getPointerOperand();
        while (auto *Parent = dyn_cast<GetElementPtrInst>(Root))
          Root = Parent->getPointerOperand();
        if (Root == G || Root == Output) {
          Type *Cursor = P->getSourceElementType();
          SmallVector<Value *> Indices;
          Indices.push_back(P->getOperand(1));
          for (Value *Index : drop_begin(P->indices())) {
            if (auto *Record = dyn_cast<StructType>(Cursor)) {
              unsigned Field = cast<ConstantInt>(Index)->getZExtValue();
              if (!PackType(PackType, Cursor)->isVectorTy())
                Indices.push_back(Index);
              Cursor = Record->getElementType(Field);
            } else {
              Indices.push_back(Index);
              Cursor = Cursor->isArrayTy() ? Cursor->getArrayElementType()
                                           : Cursor->getScalarType();
            }
          }
          if (Indices.size() != P->getNumIndices()) {
            IRBuilder<NoFolder> Builder(P);
            auto *Rewritten =
                Builder.CreateGEP(PackType(PackType, P->getSourceElementType()),
                                  P->getPointerOperand(), Indices);
            P->replaceAllUsesWith(Rewritten);
            P->eraseFromParent();
            continue;
          }
          P->setSourceElementType(
              PackType(PackType, P->getSourceElementType()));
          P->setResultElementType(
              PackType(PackType, P->getResultElementType()));
        }
      }
    G->replaceAllUsesWith(Output);
    G->eraseFromParent();
    PackedTypes[OldType] = OrdinaryPacked;
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
    IRBuilder<NoFolder> B(P);
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
  Arguments.push_back(
      "-spirv-ext=+SPV_KHR_shader_clock,+SPV_EXT_demote_to_helper_invocation,+"
      "SPV_KHR_physical_storage_buffer,+SPV_KHR_"
      "vulkan_memory_model,+SPV_EXT_descriptor_indexing,+SPV_"
      "KHR_shader_draw_parameters,+SPV_KHR_multiview,+SPV_KHR_"
      "ray_query,+SPV_KHR_zero_initialize_workgroup_memory,+"
      "SPV_KHR_8bit_storage,+SPV_KHR_16bit_storage,+SPV_EXT_shader_atomic_"
      "float_add,+SPV_EXT_shader_atomic_float_min_max,+SPV_KHR_integer_dot_"
      "product,+SPV_KHR_fragment_shader_barycentric,+SPV_KHR_fragment_shading_"
      "rate,+SPV_EXT_fragment_shader_interlock,+SPV_KHR_cooperative_matrix,+"
      "SPV_EXT_shader_image_int64,+SPV_KHR_post_depth_coverage,+"
      "SPV_EXT_shader_stencil_export,+SPV_EXT_shader_tile_image,+"
      "SPV_EXT_mesh_shader,+SPV_KHR_ray_tracing,+"
      "SPV_EXT_shader_atomic_float16_add");
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
                .contains("Atomic")) {
          StringRef Op =
              F->getFnAttribute("spirv.cxx.intrinsic").getValueAsString();
          bool Image = Op.consume_front("Image");
          if (!Op.starts_with("Atomic"))
            continue;
          Type *ValueType = Call->getType();
          if (Op == "AtomicStore")
            ValueType = Call->getArgOperand(Call->arg_size() - 1)->getType();
          if (ValueType->isFloatingPointTy()) {
            bool Shared =
                Call->getArgOperand(0)->getType()->getPointerAddressSpace() ==
                3;
            unsigned Width = ValueType->getScalarSizeInBits();
            StringRef Policy = Op == "AtomicFAddEXT" ? "AtomicAdd"
                               : Op == "AtomicFMinEXT" || Op == "AtomicFMaxEXT"
                                   ? "AtomicMinMax"
                                   : "Atomics";
            AtomicFeatures.insert((Twine(Image    ? "shaderImageFloat"
                                         : Shared ? "shaderSharedFloat"
                                                  : "shaderBufferFloat") +
                                   Twine(Width) + Policy)
                                      .str());
          }
          bool Wide = Call->getType()->isIntegerTy(64);
          for (Value *A : Call->args())
            Wide |= A->getType()->isIntegerTy(64);
          if (Wide && Call->getArgOperand(0)->getType()->isPointerTy())
            AtomicFeatures.insert(
                Image ? "shaderImageInt64Atomics"
                : Call->getArgOperand(0)->getType()->getPointerAddressSpace() ==
                        3
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
  std::unique_ptr<TargetMachine> TM(
      T->createTargetMachine(M->getTargetTriple(), "", "", Options,
                             std::nullopt, std::nullopt, OptLevel));
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
  if (!specializeWorkgroupSize(Binary, *Entry))
    return 1;
  json::Array Capabilities, Extensions;
  std::set<std::string> Features{"bufferDeviceAddress", "descriptorIndexing",
                                 "vulkanMemoryModel", "scalarBlockLayout"};
  Features.insert(AtomicFeatures.begin(), AtomicFeatures.end());
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
      unsigned Scope = ConstantValues.lookup(
          support::endian::read32le(Binary.data() + Offset + 12));
      Features.insert(Scope == 1 ? "shaderDeviceClock" : "shaderSubgroupClock");
    }
    if (Opcode == 71 && Count == 4 &&
        support::endian::read32le(Binary.data() + Offset + 8) == 11) {
      unsigned Builtin = support::endian::read32le(Binary.data() + Offset + 12);
      if (Builtin == 3)
        Features.insert("shaderClipDistance");
      if (Builtin == 4)
        Features.insert("shaderCullDistance");
    }
    if (Opcode == 17 && Count == 2) {
      unsigned Cap = support::endian::read32le(Binary.data() + Offset + 4);
      Capabilities.push_back(Cap);
      switch (Cap) {
      case 2:
        Features.insert("geometryShader");
        break;
      case 3:
        Features.insert("tessellationShader");
        break;
      case 5283:
        Features.insert("meshShader");
        if (Stage == "task")
          Features.insert("taskShader");
        break;
      case 4479:
        Features.insert("rayTracingPipeline");
        Features.insert("accelerationStructure");
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
      case 6019:
        Features.insert("shaderIntegerDotProduct");
        break;
      case 6022:
        Features.insert("cooperativeMatrix");
        break;
      case 5016:
        Features.insert("shaderImageInt64Atomics");
        break;
      case 41:
        Features.insert("sparseBinding");
        Features.insert("sparseResidencyImage2D");
        break;
      case 5284:
        Features.insert("fragmentShaderBarycentric");
        break;
      case 4422:
        Features.insert("pipelineFragmentShadingRate");
        break;
      case 5378:
        Features.insert("fragmentShaderPixelInterlock");
        break;
      case 5363:
        Features.insert("fragmentShaderSampleInterlock");
        break;
      case 5372:
        Features.insert("fragmentShaderShadingRateInterlock");
        break;
      case 4166:
        Features.insert("shaderTileImageColorReadAccess");
        break;
      case 4167:
        Features.insert("shaderTileImageDepthReadAccess");
        break;
      case 4168:
        Features.insert("shaderTileImageStencilReadAccess");
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
