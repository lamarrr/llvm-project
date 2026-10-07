//===-- ClangSPIRVLink.cpp - C++ shader linking and legalization
//------------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/ScopeExit.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Constants.h"
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
#include "llvm/Transforms/Utils/Local.h"
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
          "direct SPIR-V inline assembly lowering is not implemented yet");
    auto *Callee =
        dyn_cast<Function>(Call->getCalledOperand()->stripPointerCasts());
    if (!Callee)
      return fail("unresolved indirect call in '" + F.getName() + "'");
    if (Callee->isIntrinsic())
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
    if (AS != 7 && AS != 8 && AS != 12 && AS != 13)
      continue;
    auto R = typeLayout(G.getValueType(), M.getDataLayout());
    if (auto Alignment = G.getAlign())
      R["alignment"] = static_cast<int64_t>(Alignment->value());
    R["name"] = G.getName().str();
    R["kind"] = AS == 7    ? "input"
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
      }
    Interfaces.push_back(std::move(R));
  }
  return Interfaces;
}

static bool legalize(Module &M, Function &Entry) {
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
  ModulePassManager Inline;
  Inline.addPass(AlwaysInlinerPass());
  Inline.addPass(GlobalDCEPass());
  Inline.run(M, MAM);
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
  // A zero-offset first-member GEP can disappear during SROA. Recover that
  // member before indexing a nested array through its containing record.
  SmallVector<GetElementPtrInst *> ArrayGEPs;
  for (Instruction &I : instructions(Entry))
    if (auto *GEP = dyn_cast<GetElementPtrInst>(&I))
      if (GEP->getSourceElementType()->isArrayTy())
        ArrayGEPs.push_back(GEP);
  for (auto *GEP : ArrayGEPs) {
    Value *Base = GEP->getPointerOperand();
    Type *Actual = nullptr;
    if (auto *Parent = dyn_cast<GetElementPtrInst>(Base))
      Actual = Parent->getResultElementType();
    else if (auto *Global = dyn_cast<GlobalVariable>(Base))
      Actual = Global->getValueType();
    auto *Record = dyn_cast_or_null<StructType>(Actual);
    if (Record && Record->getNumElements() &&
        Record->getElementType(0) == GEP->getSourceElementType() &&
        M.getDataLayout().getStructLayout(Record)->getElementOffset(0) == 0) {
      IRBuilder<NoFolder> B(GEP);
      GEP->setOperand(0, B.CreateStructGEP(Record, Base, 0, "first.member"));
    }
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
  for (Instruction &I : instructions(Entry)) {
    if (auto *Load = dyn_cast<LoadInst>(&I))
      Load->setAlignment(std::max(Load->getAlign(),
                                  getKnownAlignment(Load->getPointerOperand(),
                                                    M.getDataLayout(), Load)));
    else if (auto *Store = dyn_cast<StoreInst>(&I))
      Store->setAlignment(std::max(
          Store->getAlign(), getKnownAlignment(Store->getPointerOperand(),
                                               M.getDataLayout(), Store)));
  }
  if (!M.getNamedMetadata("spirv.MemoryModel"))
    M.getOrInsertNamedMetadata("spirv.MemoryModel")
        ->addOperand(MDNode::get(M.getContext(), {number(M.getContext(), 5348),
                                                  number(M.getContext(), 3)}));
  if (verifyModule(M, &errs()))
    return fail("invalid LLVM IR after C++ shader legalization");
  return true;
}

int main(int Argc, char **Argv) {
  InitLLVM X(Argc, Argv);
  InitializeAllTargetInfos();
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmPrinters();
  SmallVector<const char *> Arguments(Argv, Argv + Argc);
  Arguments.push_back("-spirv-ext=+SPV_KHR_physical_storage_buffer,+SPV_KHR_"
                      "vulkan_memory_model,+SPV_EXT_descriptor_indexing,+SPV_"
                      "KHR_shader_draw_parameters,+SPV_KHR_multiview");
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
  Function *Entry = nullptr;
  for (Function &F : *M) {
    if (!F.hasFnAttribute("spirv.cxx.stage") ||
        (!EntryName.empty() && F.getName() != EntryName))
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
  json::Object Reflection{{"schema_version", 1},
                          {"entry", Entry->getName().str()},
                          {"stage", Stage},
                          {"target", "spirv1.4-unknown-vulkan1.3"},
                          {"interfaces", reflectInterfaces(*M)}};
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
  std::string Error;
  const Target *T = TargetRegistry::lookupTarget(M->getTargetTriple(), Error);
  if (!T) {
    fail(Error);
    return 1;
  }
  TargetOptions Options;
  std::unique_ptr<TargetMachine> TM(T->createTargetMachine(
      M->getTargetTriple(), "", "", Options, std::nullopt, std::nullopt));
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
  json::Array Capabilities, Extensions;
  std::set<std::string> Features{"bufferDeviceAddress", "descriptorIndexing",
                                 "vulkanMemoryModel", "scalarBlockLayout"};
  for (size_t Offset = 20; Offset < Binary.size();) {
    uint32_t Header = support::endian::read32le(Binary.data() + Offset);
    unsigned Count = Header >> 16, Opcode = Header & 0xffff;
    if (!Count || Offset + Count * 4 > Binary.size()) {
      fail("malformed SPIR-V instruction");
      return 1;
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
      case 12:
        Features.insert("shaderBufferInt64Atomics");
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
      default:
        break;
      }
    } else if (Opcode == 10) {
      Extensions.push_back(std::string(Binary.data() + Offset + 4));
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
