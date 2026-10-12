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

struct ShaderInterpolation {
  unsigned Mode = 0;
  unsigned Sampling = 0;

  explicit operator bool() const { return Mode || Sampling; }
};

static ShaderInterpolation
shaderInterpolation(const Decl *Declaration, ShaderInterpolation Parent = {}) {
  if (Declaration->hasAttr<SPIRVFlatAttr>()) {
    Parent.Mode = 14;
    Parent.Sampling = 0;
  } else if (Declaration->hasAttr<SPIRVVaryingAttr>())
    Parent.Mode = 1;
  else if (Declaration->hasAttr<SPIRVNoPerspectiveAttr>())
    Parent.Mode = 13;
  if (Declaration->hasAttr<SPIRVCentroidAttr>())
    Parent.Sampling = 16;
  else if (Declaration->hasAttr<SPIRVSampleAttr>())
    Parent.Sampling = 17;
  return Parent;
}

static std::optional<unsigned> namedBuiltinInterface(const Decl *Declaration) {
  if (Declaration->hasAttr<SPIRVPrimitiveIdAttr>())
    return 7;
  if (Declaration->hasAttr<SPIRVInvocationIdAttr>())
    return 8;
  if (Declaration->hasAttr<SPIRVLayerAttr>())
    return 9;
  if (Declaration->hasAttr<SPIRVViewportIndexAttr>())
    return 10;
  if (Declaration->hasAttr<SPIRVTessLevelOuterAttr>())
    return 11;
  if (Declaration->hasAttr<SPIRVTessLevelInnerAttr>())
    return 12;
  if (Declaration->hasAttr<SPIRVTessCoordAttr>())
    return 13;
  if (Declaration->hasAttr<SPIRVPatchVerticesAttr>())
    return 14;
  if (Declaration->hasAttr<SPIRVPrimitivePointIndicesAttr>())
    return 5294;
  if (Declaration->hasAttr<SPIRVPrimitiveLineIndicesAttr>())
    return 5295;
  if (Declaration->hasAttr<SPIRVPrimitiveTriangleIndicesAttr>())
    return 5296;
  if (Declaration->hasAttr<SPIRVCullPrimitiveAttr>())
    return 5299;
  return std::nullopt;
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
    if (A->getLocation() != UINT32_MAX)
      addDecoration(GV, 30, A->getLocation());
  if (auto *A = D->getAttr<SPIRVOutputAttr>())
    if (A->getLocation() != UINT32_MAX)
      addDecoration(GV, 30, A->getLocation());
  if (D->hasAttr<SPIRVPositionAttr>())
    addDecoration(GV, 11, 0);
  if (D->hasAttr<SPIRVFragDepthAttr>())
    addDecoration(GV, 11, 22);
  if (D->hasAttr<SPIRVStencilRefAttr>())
    addDecoration(GV, 11, 5014);
  if (D->hasAttr<SPIRVSampleMaskAttr>())
    addDecoration(GV, 11, 20);
  if (D->hasAttr<SPIRVPointSizeAttr>())
    addDecoration(GV, 11, 1);
  if (D->hasAttr<SPIRVClipDistanceAttr>())
    addDecoration(GV, 11, 3);
  if (D->hasAttr<SPIRVCullDistanceAttr>())
    addDecoration(GV, 11, 4);
  if (D->hasAttr<SPIRVNoPerspectiveAttr>())
    addDecoration(GV, 13);
  if (D->hasAttr<SPIRVFlatAttr>())
    addDecoration(GV, 14);
  if (D->hasAttr<SPIRVCentroidAttr>())
    addDecoration(GV, 16);
  if (D->hasAttr<SPIRVSampleAttr>())
    addDecoration(GV, 17);
  auto Builtin = namedBuiltinInterface(D);
  if (Builtin)
    addDecoration(GV, 11, *Builtin);
  if (D->hasAttr<SPIRVPatchAttr>())
    addDecoration(GV, 15);
  if (D->hasAttr<SPIRVPerPrimitiveAttr>() || (Builtin && *Builtin >= 5294))
    addDecoration(GV, 5271);
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
  auto *SizeIDs = D->getAttr<SPIRVWorkgroupSizeIdAttr>();
  bool HasWorkgroup = Stage->getStage() == SPIRVEntryPointAttr::Compute ||
                      Stage->getStage() == SPIRVEntryPointAttr::Task ||
                      Stage->getStage() == SPIRVEntryPointAttr::Mesh;
  if (HasWorkgroup != (Size != nullptr || SizeIDs != nullptr)) {
    Fail(HasWorkgroup ? "this stage requires workgroup dimensions"
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
                                               "callable",
                                               "geometry"};
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
  auto AddMode = [&](unsigned Mode,
                     std::optional<unsigned> Literal = std::nullopt) {
    SmallVector<llvm::Metadata *> Operands{llvm::ValueAsMetadata::get(Entry),
                                           integerMD(C, Mode)};
    if (Literal)
      Operands.push_back(integerMD(C, *Literal));
    M.getOrInsertNamedMetadata("spirv.ExecutionMode")
        ->addOperand(llvm::MDNode::get(C, Operands));
  };
  auto *Geometry = D->getAttr<SPIRVGeometryModeAttr>();
  auto *Invocations = D->getAttr<SPIRVGeometryInvocationsAttr>();
  auto *Vertices = D->getAttr<SPIRVOutputVerticesAttr>();
  auto *Tessellation = D->getAttr<SPIRVTessellationModeAttr>();
  auto *Mesh = D->getAttr<SPIRVMeshOutputAttr>();
  if ((StageName == "geometry") != bool(Geometry) ||
      (Invocations && StageName != "geometry") ||
      (StageName == "tessellation_control") != bool(Vertices) ||
      (StageName == "tessellation_evaluation") != bool(Tessellation) ||
      (StageName == "mesh") != bool(Mesh)) {
    Fail("advanced stage requires its matching execution mode annotation");
    Entry->eraseFromParent();
    return;
  }
  if (Geometry) {
    AddMode(Geometry->getInput());
    AddMode(Geometry->getOutput());
    AddMode(26, Geometry->getVertices());
    AddMode(0, Invocations ? Invocations->getCount() : 1);
  }
  if (Vertices)
    AddMode(26, Vertices->getCount());
  if (Tessellation) {
    AddMode(Tessellation->getDomain());
    AddMode(Tessellation->getSpacing());
    AddMode(Tessellation->getOrder());
  }
  if (Mesh) {
    AddMode(Mesh->getTopology());
    AddMode(26, Mesh->getVertices());
    AddMode(5270, Mesh->getPrimitives());
  }
  if (auto *Interlock = D->getAttr<SPIRVFragmentInterlockAttr>()) {
    if (Stage->getStage() != SPIRVEntryPointAttr::Fragment) {
      Fail("fragment interlock requires a fragment entry point");
      Entry->eraseFromParent();
      return;
    }
    Entry->addFnAttr("spirv.cxx.interlock_mode",
                     std::to_string(Interlock->getMode()));
  }
  Entry->addFnAttr(llvm::Attribute::Convergent);
  if (Size)
    Entry->addFnAttr(
        "hlsl.numthreads",
        llvm::formatv("{0},{1},{2}", Size->getX(), Size->getY(), Size->getZ())
            .str());
  if (SizeIDs) {
    Entry->addFnAttr("hlsl.numthreads",
                     llvm::formatv("{0},{1},{2}", SizeIDs->getXDefault(),
                                   SizeIDs->getYDefault(),
                                   SizeIDs->getZDefault())
                         .str());
    Entry->addFnAttr("spirv.cxx.workgroup_size_ids",
                     llvm::formatv("{0},{1},{2}", SizeIDs->getXId(),
                                   SizeIDs->getYId(), SizeIDs->getZId())
                         .str());
  }
  CodeGenFunction CGF(*this);
  CGF.StartFunction(GlobalDecl(), Context.VoidTy, Entry,
                    getTypes().arrangeNullaryFunction(), {});
  auto &B = CGF.Builder;
  if (SizeIDs) {
    auto *I32 = llvm::Type::getInt32Ty(C);
    auto Callee =
        M.getOrInsertFunction("_Z20__spirv_SpecConstantILy0EEvv",
                              llvm::FunctionType::get(I32, {I32, I32}, false));
    auto *F = llvm::cast<llvm::Function>(Callee.getCallee());
    F->addFnAttr("spirv.cxx.intrinsic", "SpecConstant");
    F->addFnAttr(llvm::Attribute::NoUnwind);
    for (auto [ID, Default] :
         {std::pair{SizeIDs->getXId(), SizeIDs->getXDefault()},
          std::pair{SizeIDs->getYId(), SizeIDs->getYDefault()},
          std::pair{SizeIDs->getZId(), SizeIDs->getZDefault()}}) {
      auto *Call = B.CreateCall(F, {B.getInt32(ID), B.getInt32(Default)});
      Call->setMetadata(
          "spirv.cxx.specialization.type",
          llvm::MDNode::get(C, llvm::MDString::get(C, "unsigned int")));
    }
  }
  CallArgList Args;
  unsigned PushConstants = 0;
  unsigned Positions = 0;
  unsigned InputPositions = 0;
  unsigned FragDepths = 0;
  unsigned StencilRefs = 0;
  unsigned SampleMasks = 0;
  unsigned InputSampleMasks = 0;
  unsigned PointSizes = 0;
  unsigned ClipDistances = 0;
  unsigned CullDistances = 0;
  unsigned TaskPayloads = 0;
  auto ContainsInteger = [&](auto &&Self, QualType Type) -> bool {
    Type = Context.getBaseElementType(Type);
    if (const auto *Record = Type->getAsRecordDecl()) {
      for (const auto *Field : Record->fields())
        if (Self(Self, Field->getType()))
          return true;
      return false;
    }
    return Type->isIntegerType();
  };
  auto FloatingOnly = [&](auto &&Self, QualType Type) -> bool {
    Type = Context.getBaseElementType(Type);
    if (const auto *Record = Type->getAsRecordDecl()) {
      for (const auto *Field : Record->fields())
        if (!Self(Self, Field->getType()))
          return false;
      return true;
    }
    return Type->isRealFloatingType();
  };
  auto ContainsDouble = [&](auto &&Self, QualType Type) -> bool {
    Type = Context.getBaseElementType(Type);
    if (const auto *Record = Type->getAsRecordDecl()) {
      for (const auto *Field : Record->fields())
        if (Self(Self, Field->getType()))
          return true;
      return false;
    }
    return Type->isRealFloatingType() && Context.getTypeSize(Type) == 64;
  };
  struct LocationRange {
    unsigned AddressSpace;
    uint64_t Begin;
    uint64_t End;
  };
  SmallVector<LocationRange> Locations;
  auto ScalarComponent = [&](QualType Type) {
    return Type->isRealFloatingType() ||
           (Type->isIntegerType() && !Type->isBooleanType() &&
            (Context.getTypeSize(Type) == 32 ||
             Context.getTypeSize(Type) == 64));
  };
  auto VectorRecord = [&](const CXXRecordDecl *Record) {
    if (!Record->isStandardLayout() || !Record->isTriviallyCopyable() ||
        Record->isUnion() || Record->getNumBases() ||
        Record->getNumFields() < 2 || Record->getNumFields() > 4)
      return false;
    QualType Component = (*Record->field_begin())->getType();
    if (!ScalarComponent(Component))
      return false;
    for (const auto *Field : Record->fields())
      if (Field->isBitField() || Field->hasAttr<SPIRVLocationAttr>() ||
          shaderInterpolation(Field) ||
          !Context.hasSameUnqualifiedType(Field->getType(), Component))
        return false;
    return Context.getTypeSize(Context.getCanonicalTagType(Record)) ==
           Context.getTypeSize(Component) * Record->getNumFields();
  };
  auto NeedsMemberLocations = [&](auto &&Self, QualType Type) -> bool {
    if (const auto *Array = Context.getAsConstantArrayType(Type))
      return Self(Self, Array->getElementType());
    const auto *Record = Type->getAsCXXRecordDecl();
    if (!Record)
      return false;
    if (VectorRecord(Record))
      return false;
    if (Record->getNumFields() != 1)
      return true;
    const auto *Field = *Record->field_begin();
    return Field->hasAttr<SPIRVLocationAttr>() || shaderInterpolation(Field) ||
           Self(Self, Field->getType());
  };
  auto LocationCount = [&](auto &&Self, QualType Type) -> uint64_t {
    if (const auto *Array = Context.getAsConstantArrayType(Type)) {
      uint64_t Count = Array->getSize().getZExtValue();
      if (Count >= 2 && Count <= 4 && ScalarComponent(Array->getElementType()))
        return (Count * Context.getTypeSize(Array->getElementType()) + 127) /
               128;
      return Count * Self(Self, Array->getElementType());
    }
    if (const auto *Record = Type->getAsCXXRecordDecl()) {
      if (VectorRecord(Record))
        return (Context.getTypeSize((*Record->field_begin())->getType()) *
                    Record->getNumFields() +
                127) /
               128;
      uint64_t Count = 0;
      for (const auto *Field : Record->fields())
        Count += Self(Self, Field->getType());
      return Count;
    }
    return Type->isIntegerType() || Type->isRealFloatingType() ? 1 : 0;
  };
  auto ClaimLocation = [&](unsigned AddressSpace, uint32_t Location,
                           QualType Type) {
    uint64_t Count = LocationCount(LocationCount, Type);
    uint64_t End = uint64_t(Location) + Count;
    if (!Count || End > UINT32_MAX)
      return false;
    for (const auto &Range : Locations)
      if (Range.AddressSpace == AddressSpace && Location < Range.End &&
          Range.Begin < End)
        return false;
    Locations.push_back({AddressSpace, Location, End});
    return true;
  };
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
    if (namedBuiltinInterface(P))
      AS = Element.isConstQualified() ? 7 : 8;
    if (P->hasAttr<SPIRVSampleMaskAttr>() && Element.isConstQualified())
      AS = 7;
    if (P->hasAttr<SPIRVOutputAttr>() || P->hasAttr<SPIRVPositionAttr>() ||
        P->hasAttr<SPIRVFragDepthAttr>() || P->hasAttr<SPIRVStencilRefAttr>() ||
        (P->hasAttr<SPIRVSampleMaskAttr>() && !Element.isConstQualified()) ||
        P->hasAttr<SPIRVPointSizeAttr>() ||
        P->hasAttr<SPIRVClipDistanceAttr>() ||
        P->hasAttr<SPIRVCullDistanceAttr>()) {
      AS = 8;
      if (!IsPointer) {
        Fail("output parameters must be pointers to interface values");
        CGF.FinishFunction();
        Entry->eraseFromParent();
        return;
      }
    }
    if ((P->hasAttr<SPIRVClipDistanceAttr>() ||
         P->hasAttr<SPIRVCullDistanceAttr>() ||
         P->hasAttr<SPIRVPositionAttr>()) &&
        Element.isConstQualified())
      AS = 7;
    QualType ResourceType = ValueType;
    if (P->getOriginalType()->isArrayType())
      ResourceType = P->getOriginalType();
    auto Resource = shaderResource(ResourceType);
    if (Resource)
      AS = Resource.Name == "tile_image" ? 16 : 2;
    if (AS != 7 && AS != 8 && AS != 12 && AS != 13 && !(AS >= 17 && AS <= 23) &&
        !((AS == 2 || AS == 16) && Resource)) {
      Fail("entry parameters must be qualified inputs, outputs, uniforms, or "
           "push constants");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    ShaderInterpolation ParameterInterpolation = shaderInterpolation(P);
    bool Interpolation = bool(ParameterInterpolation);
    bool Varying =
        (AS == 7 && StageName == "fragment") ||
        (AS == 8 &&
         (StageName == "vertex" || StageName == "geometry" ||
          StageName == "tessellation_evaluation" || StageName == "mesh"));
    const auto *InputLocation = P->getAttr<SPIRVInputAttr>();
    const auto *OutputLocation = P->getAttr<SPIRVOutputAttr>();
    const auto *LocationAttribute =
        InputLocation ? static_cast<const Attr *>(InputLocation)
                      : static_cast<const Attr *>(OutputLocation);
    unsigned Location = InputLocation    ? InputLocation->getLocation()
                        : OutputLocation ? OutputLocation->getLocation()
                                         : UINT32_MAX;
    bool FieldInterface = LocationAttribute && Location == UINT32_MAX;
    SmallVector<llvm::Metadata *> FieldInterfaces;
    bool VertexArrayInterface =
        !P->hasAttr<SPIRVPatchAttr>() &&
        ((AS == 7 &&
          (StageName == "geometry" || StageName == "tessellation_control" ||
           StageName == "tessellation_evaluation")) ||
         (AS == 8 &&
          (StageName == "tessellation_control" || StageName == "mesh")));
    QualType LocatedType = Element;
    if (VertexArrayInterface)
      if (const auto *Array = Context.getAsConstantArrayType(LocatedType))
        LocatedType = Array->getElementType();
    if (FieldInterface) {
      SmallVector<unsigned> Path;
      StringRef FieldError;
      auto CollectFields = [&](auto &&Self, QualType Type,
                               ShaderInterpolation Parent) -> bool {
        const auto *Record = Type->getAsCXXRecordDecl();
        if (!Record || !Record->isStandardLayout() ||
            !Record->isTriviallyCopyable() || Record->isUnion() ||
            Record->getNumBases() || Record->field_empty())
          return false;
        const auto &Layout = getTypes().getCGRecordLayout(Record);
        for (const auto *Field : Record->fields()) {
          if (Field->isBitField())
            return false;
          ShaderInterpolation Qualifiers = shaderInterpolation(Field, Parent);
          Path.push_back(Layout.getLLVMFieldNo(Field));
          if (const auto *Attribute = Field->getAttr<SPIRVLocationAttr>()) {
            if (NeedsMemberLocations(NeedsMemberLocations, Field->getType()) ||
                !ClaimLocation(AS, Attribute->getLocation(), Field->getType()))
              return false;
            if (Qualifiers && !Varying)
              FieldError =
                  "interpolation qualifiers require vertex outputs or "
                  "fragment inputs, or other rasterization-stage outputs";
            else if (Qualifiers.Mode == 14 && Qualifiers.Sampling)
              FieldError = "conflicting interpolation qualifiers";
            else if (AS == 7 && StageName == "fragment" &&
                     ContainsInteger(ContainsInteger, Field->getType()) &&
                     Qualifiers.Mode != 14)
              FieldError = "integer fragment inputs require __flat";
            else if (AS == 7 && StageName == "fragment" &&
                     ContainsDouble(ContainsDouble, Field->getType()) &&
                     Qualifiers.Mode != 14)
              FieldError = "double-precision fragment inputs require __flat";
            else if (Qualifiers && Qualifiers.Mode != 14 &&
                     !FloatingOnly(FloatingOnly, Field->getType()))
              FieldError = "interpolated inputs and outputs require "
                           "floating-point values";
            if (!FieldError.empty())
              return false;
            SmallVector<llvm::Metadata *> Decorations;
            for (unsigned Decoration : {Qualifiers.Mode, Qualifiers.Sampling})
              if (Decoration > 1)
                Decorations.push_back(
                    llvm::MDNode::get(C, integerMD(C, Decoration)));
            SmallVector<llvm::Metadata *> Description{
                integerMD(C, Attribute->getLocation()),
                llvm::MDString::get(C, Field->getName()),
                llvm::MDNode::get(C, Decorations)};
            for (unsigned Index : Path)
              Description.push_back(integerMD(C, Index));
            FieldInterfaces.push_back(llvm::MDNode::get(C, Description));
          } else if (!Self(Self, Field->getType(), Qualifiers))
            return false;
          Path.pop_back();
        }
        return true;
      };
      if (!CollectFields(CollectFields, LocatedType, ParameterInterpolation)) {
        Fail(FieldError.empty()
                 ? "interface record members require explicit, non-overlapping "
                   "__location(N) annotations on scalar or vector values"
                 : FieldError);
        CGF.FinishFunction();
        Entry->eraseFromParent();
        return;
      }
    } else if (LocationAttribute &&
               NeedsMemberLocations(NeedsMemberLocations, LocatedType)) {
      Fail("interface record members require explicit __location(N); "
           "omit the parameter location");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    } else if (LocationAttribute && !ClaimLocation(AS, Location, LocatedType)) {
      Fail("shader interface locations overlap or have an invalid type or "
           "range");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    bool RayStage = Stage->getStage() >= SPIRVEntryPointAttr::RayGeneration &&
                    Stage->getStage() <= SPIRVEntryPointAttr::Callable;
    auto Equivalent = [&](QualType Type, QualType Component, unsigned Count) {
      auto Shape = shaderShape(Context, Type);
      return Shape && !Shape.Rows && Shape.Count == Count &&
             Context.hasSameUnqualifiedType(Shape.Element, Component) &&
             Context.getTypeSize(Type) ==
                 Context.getTypeSize(Component) * Count &&
             shaderCanCopyFields(Context, Type) &&
             (!Type->isRecordType() ||
              Type->getAsCXXRecordDecl()->isTriviallyCopyable());
    };
    auto IntegerShape = [&](QualType Type) {
      auto Shape = shaderShape(Context, Type);
      return Shape && !Shape.Rows && Shape.Count == 1 &&
             Shape.Element->isIntegerType() &&
             !Shape.Element->isBooleanType() &&
             Context.getTypeSize(Shape.Element) == 32 &&
             Equivalent(Type, Shape.Element, 1);
    };
    auto InterfaceArray = [&](QualType Type) {
      while (const auto *Record = Type->getAsCXXRecordDecl()) {
        if (!Record->isStandardLayout() || !Record->isTriviallyCopyable() ||
            Record->isUnion() || Record->getNumBases() ||
            Record->getNumFields() != 1 ||
            !shaderCanCopyFields(Context, Type) ||
            Context.getTypeSize(Type) !=
                Context.getTypeSize((*Record->field_begin())->getType()))
          break;
        Type = (*Record->field_begin())->getType();
      }
      return Context.getAsConstantArrayType(Type);
    };
    auto FloatArrayShape = [&](QualType Type) {
      auto Shape = shaderShape(Context, Type);
      return Shape && !Shape.Rows &&
                     Equivalent(Type, Context.FloatTy, Shape.Count)
                 ? Shape.Count
                 : 0u;
    };
    auto *FloatType = llvm::Type::getFloatTy(C);
    auto *IntegerType = llvm::Type::getInt32Ty(C);
    llvm::Type *BuiltinInterfaceType = nullptr;
    if (auto Builtin = namedBuiltinInterface(P)) {
      unsigned ID = *Builtin;
      auto *Array = InterfaceArray(Element);
      bool Valid = false;
      if (ID == 11 || ID == 12)
        Valid = Equivalent(Element, Context.FloatTy, ID == 11 ? 4 : 2) &&
                P->hasAttr<SPIRVPatchAttr>() &&
                ((StageName == "tessellation_control" && AS == 8) ||
                 (StageName == "tessellation_evaluation" && AS == 7));
      if (ID == 11 || ID == 12)
        BuiltinInterfaceType =
            llvm::ArrayType::get(FloatType, ID == 11 ? 4 : 2);
      if (ID == 7 || ID == 8 || ID == 9 || ID == 10 || ID == 14)
        Valid = IntegerShape(Element) &&
                ((AS == 7 && (StageName == "geometry" ||
                              StageName == "tessellation_control" ||
                              StageName == "tessellation_evaluation" ||
                              StageName == "fragment")) ||
                 (AS == 8 && (StageName == "geometry" || StageName == "mesh") &&
                  (ID == 7 || ID == 9 || ID == 10)));
      if (ID == 7 || ID == 8 || ID == 9 || ID == 10 || ID == 14)
        BuiltinInterfaceType = IntegerType;
      if (ID == 13) {
        auto Shape = shaderShape(Context, Element);
        Valid = StageName == "tessellation_evaluation" && AS == 7 && Shape &&
                Shape.Count == 3 && !Shape.Rows &&
                Context.hasSameUnqualifiedType(Shape.Element, Context.FloatTy);
        Valid &= Equivalent(Element, Context.FloatTy, 3);
        BuiltinInterfaceType = llvm::FixedVectorType::get(FloatType, 3);
      }
      if ((ID == 7 || ID == 9 || ID == 10) && StageName == "mesh")
        Valid = AS == 8 && Array && IntegerShape(Array->getElementType()) &&
                Array->getSize().getZExtValue() == Mesh->getPrimitives() &&
                P->hasAttr<SPIRVPerPrimitiveAttr>();
      if ((ID == 7 || ID == 9 || ID == 10) && StageName == "mesh" && Array)
        BuiltinInterfaceType =
            llvm::ArrayType::get(IntegerType, Array->getSize().getZExtValue());
      if (ID >= 5294 && ID <= 5296) {
        unsigned Components = ID - 5293;
        auto Shape = Array ? shaderShape(Context, Array->getElementType())
                           : ShaderShape{};
        Valid = StageName == "mesh" && AS == 8 && Array && Shape &&
                Shape.Count == Components && !Shape.Rows &&
                Shape.Element->isUnsignedIntegerType() &&
                Context.getTypeSize(Shape.Element) == 32 &&
                Array->getSize().getZExtValue() == Mesh->getPrimitives();
        Valid &= Array && Equivalent(Array->getElementType(),
                                     Context.UnsignedIntTy, Components);
        if (Array)
          BuiltinInterfaceType = llvm::ArrayType::get(
              Components == 1
                  ? IntegerType
                  : static_cast<llvm::Type *>(
                        llvm::FixedVectorType::get(IntegerType, Components)),
              Array->getSize().getZExtValue());
      }
      if (ID == 5299)
        Valid = StageName == "mesh" && AS == 8 && Array &&
                Equivalent(Array->getElementType(), Context.BoolTy, 1) &&
                Array->getSize().getZExtValue() == Mesh->getPrimitives();
      if (ID == 5299 && Array)
        BuiltinInterfaceType = llvm::ArrayType::get(
            llvm::Type::getInt1Ty(C), Array->getSize().getZExtValue());
      if (!Valid || !IsPointer || P->hasAttr<SPIRVInputAttr>() ||
          P->hasAttr<SPIRVOutputAttr>()) {
        Fail("builtin interface has an invalid type, direction or stage");
        CGF.FinishFunction();
        Entry->eraseFromParent();
        return;
      }
    }
    bool PayloadAllowed =
        AS < 17 ||
        (AS == 17 &&
         (StageName == "task" ||
          (StageName == "mesh" && Element.isConstQualified())) &&
         ++TaskPayloads == 1) ||
        (AS == 18 && (StageName == "ray_generation" ||
                      StageName == "closest_hit" || StageName == "miss")) ||
        (AS == 19 && (StageName == "any_hit" || StageName == "closest_hit" ||
                      StageName == "miss")) ||
        (AS == 20 && (StageName == "intersection" ||
                      ((StageName == "any_hit" || StageName == "closest_hit") &&
                       Element.isConstQualified()))) ||
        (AS == 21 &&
         (StageName == "ray_generation" || StageName == "closest_hit" ||
          StageName == "miss" || StageName == "callable")) ||
        (AS == 22 && StageName == "callable") ||
        (AS == 23 && RayStage && Element.isConstQualified());
    if (!PayloadAllowed ||
        (P->hasAttr<SPIRVPatchAttr>() && StageName != "tessellation_control" &&
         StageName != "tessellation_evaluation") ||
        (P->hasAttr<SPIRVPerPrimitiveAttr>() &&
         !((StageName == "mesh" && AS == 8) ||
           (StageName == "fragment" && AS == 7)))) {
      Fail("shader payload or interface qualifier is invalid for this stage");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    if (P->hasAttr<SPIRVPositionAttr>()) {
      QualType PositionElement = Element;
      bool VertexArray =
          StageName == "tessellation_control" || StageName == "mesh" ||
          (AS == 7 &&
           (StageName == "geometry" || StageName == "tessellation_evaluation"));
      const auto *Outer =
          VertexArray ? InterfaceArray(PositionElement) : nullptr;
      if (VertexArray)
        if (Outer)
          PositionElement = Outer->getElementType();
      bool PositionStage = StageName == "vertex" || StageName == "geometry" ||
                           StageName == "tessellation_control" ||
                           StageName == "tessellation_evaluation" ||
                           StageName == "mesh";
      if (!PositionStage || ++(AS == 7 ? InputPositions : Positions) > 1 ||
          P->hasAttr<SPIRVInputAttr>() || P->hasAttr<SPIRVOutputAttr>() ||
          Interpolation || !Equivalent(PositionElement, Context.FloatTy, 4) ||
          (VertexArray && !Outer) || (Element.isConstQualified() && AS != 7)) {
        Fail("__position requires one vertex output pointer to float[4] "
             "or an equivalent four-float record, "
             "without location or interpolation qualifiers");
        CGF.FinishFunction();
        Entry->eraseFromParent();
        return;
      }
      BuiltinInterfaceType = llvm::FixedVectorType::get(FloatType, 4);
      if (Outer)
        BuiltinInterfaceType = llvm::ArrayType::get(
            BuiltinInterfaceType, Outer->getSize().getZExtValue());
    }
    if (P->hasAttr<SPIRVFragDepthAttr>() &&
        (Stage->getStage() != SPIRVEntryPointAttr::Fragment ||
         ++FragDepths > 1 || P->hasAttr<SPIRVInputAttr>() ||
         P->hasAttr<SPIRVOutputAttr>() || Interpolation ||
         !Equivalent(Element, Context.FloatTy, 1) ||
         Element.isConstQualified())) {
      Fail("__frag_depth requires one fragment output pointer to float "
           "or an equivalent record "
           "without location or interpolation qualifiers");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    if (P->hasAttr<SPIRVFragDepthAttr>())
      BuiltinInterfaceType = FloatType;
    if (P->hasAttr<SPIRVStencilRefAttr>() &&
        (Stage->getStage() != SPIRVEntryPointAttr::Fragment ||
         ++StencilRefs > 1 || P->hasAttr<SPIRVInputAttr>() ||
         P->hasAttr<SPIRVOutputAttr>() || Interpolation ||
         !IntegerShape(Element) || Element.isConstQualified())) {
      Fail("__stencil_ref requires one fragment output pointer to a "
           "writable 32-bit integer or an equivalent record without location "
           "or interpolation "
           "qualifiers");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    if (P->hasAttr<SPIRVStencilRefAttr>())
      BuiltinInterfaceType = IntegerType;
    if (P->hasAttr<SPIRVSampleMaskAttr>() &&
        (Stage->getStage() != SPIRVEntryPointAttr::Fragment ||
         ++(Element.isConstQualified() ? InputSampleMasks : SampleMasks) > 1 ||
         P->hasAttr<SPIRVInputAttr>() || P->hasAttr<SPIRVOutputAttr>() ||
         Interpolation || !Equivalent(Element, Context.UnsignedIntTy, 1))) {
      Fail("__sample_mask requires at most one fragment input pointer to "
           "const unsigned[1] and one output pointer to unsigned[1], "
           "or equivalent records, "
           "without location or interpolation qualifiers");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    if (P->hasAttr<SPIRVSampleMaskAttr>())
      BuiltinInterfaceType = llvm::ArrayType::get(IntegerType, 1);
    if (P->hasAttr<SPIRVPointSizeAttr>() &&
        (Stage->getStage() != SPIRVEntryPointAttr::Vertex || ++PointSizes > 1 ||
         P->hasAttr<SPIRVInputAttr>() || P->hasAttr<SPIRVOutputAttr>() ||
         Interpolation || !Equivalent(Element, Context.FloatTy, 1) ||
         Element.isConstQualified())) {
      Fail("__point_size requires one vertex output pointer to float "
           "or an equivalent record "
           "without location or interpolation qualifiers");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    if (P->hasAttr<SPIRVPointSizeAttr>())
      BuiltinInterfaceType = FloatType;
    if (P->hasAttr<SPIRVClipDistanceAttr>() ||
        P->hasAttr<SPIRVCullDistanceAttr>()) {
      bool Clip = P->hasAttr<SPIRVClipDistanceAttr>();
      unsigned DistanceCount = FloatArrayShape(Element);
      unsigned &Count = Clip ? ClipDistances : CullDistances;
      bool VertexOutput = Stage->getStage() == SPIRVEntryPointAttr::Vertex &&
                          !Element.isConstQualified();
      bool FragmentInput = Stage->getStage() == SPIRVEntryPointAttr::Fragment &&
                           Element.isConstQualified();
      if (++Count > 1 || !(VertexOutput || FragmentInput) ||
          P->hasAttr<SPIRVInputAttr>() || P->hasAttr<SPIRVOutputAttr>() ||
          Interpolation || !DistanceCount) {
        Fail(
            Clip
                ? "__clip_distance requires a vertex output float array "
                  "or fragment input const float array, or equivalent records"
                : "__cull_distance requires a vertex output float array "
                  "or fragment input const float array, or equivalent records");
        CGF.FinishFunction();
        Entry->eraseFromParent();
        return;
      }
      BuiltinInterfaceType = llvm::ArrayType::get(FloatType, DistanceCount);
    }
    if (Interpolation && !Varying) {
      Fail(
          "interpolation qualifiers require vertex outputs or fragment inputs, "
          "or other rasterization-stage outputs");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    if (!FieldInterface && AS == 7 &&
        Stage->getStage() == SPIRVEntryPointAttr::Fragment &&
        ContainsInteger(ContainsInteger, Element) &&
        !P->hasAttr<SPIRVFlatAttr>() && !P->hasAttr<SPIRVSampleMaskAttr>() &&
        !namedBuiltinInterface(P)) {
      Fail("integer fragment inputs require __flat");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    if (!FieldInterface && AS == 7 && StageName == "fragment" &&
        ContainsDouble(ContainsDouble, Element) &&
        !P->hasAttr<SPIRVFlatAttr>()) {
      Fail("double-precision fragment inputs require __flat");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    if (!FieldInterface && Interpolation && !P->hasAttr<SPIRVFlatAttr>() &&
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
    if (Resource.Name == "tile_image" &&
        Stage->getStage() != SPIRVEntryPointAttr::Fragment) {
      Fail("tile images require a fragment entry point");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    if ((AS == 12 || AS == 2) && !Binding) {
      Fail("uniform interfaces require __binding(set, binding)");
      CGF.FinishFunction();
      Entry->eraseFromParent();
      return;
    }
    if (Binding && AS != 12 && AS != 2) {
      Fail("__binding applies to descriptor resources");
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
                   Descriptor.Name != "ray_query" &&
                   !(Resource.Name == "descriptor_array" &&
                     Descriptor.Name == "tile_image");
      if (!Descriptor.Element.isNull()) {
        auto Shape = shaderShape(Context, Descriptor.Element);
        Valid &= bool(Shape) && !Shape.Rows && Shape.Count <= 4 &&
                 ((Context.getTypeSize(Shape.Element) == 32 &&
                   (Shape.Element->isIntegerType() ||
                    Shape.Element->isRealFloatingType())) ||
                  (Context.getTypeSize(Shape.Element) == 64 &&
                   Descriptor.Sampled == 2 && Shape.Count == 1 &&
                   ((Descriptor.Format == 40 &&
                     Shape.Element->isUnsignedIntegerType()) ||
                    (Descriptor.Format == 41 &&
                     Shape.Element->isSignedIntegerType()))));
        if (Valid && Descriptor.Name.starts_with("depth_"))
          Valid = Shape.Count == 1 && Context.hasSameUnqualifiedType(
                                          Shape.Element, Context.FloatTy);
      }
      if (Valid && Descriptor.Sampled == 2) {
        unsigned F = Descriptor.Name == "input_attachment" ||
                             Descriptor.Name == "tile_image"
                         ? 0
                         : Descriptor.Format;
        auto Shape = shaderShape(Context, Descriptor.Element);
        Valid =
            (F < 40 &&
             (F == 0 || (F <= 20 && Shape.Element->isRealFloatingType()) ||
              (F >= 21 && F <= 29 && Shape.Element->isSignedIntegerType()) ||
              (F >= 30 && Shape.Element->isUnsignedIntegerType()))) ||
            (F == 40 && Shape.Element->isUnsignedIntegerType()) ||
            (F == 41 && Shape.Element->isSignedIntegerType());
        if (!Valid)
          Fail("storage image format does not match its texel type");
      }
      if (!Valid) {
        Fail("native descriptors require a matching 32-bit or R64 texel type");
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
                  integerMD(C, Resource.Name == "input_attachment" ||
                                       Resource.Name == "tile_image"
                                   ? 0
                                   : Resource.Format),
                  integerMD(C, Resource.Count),
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
    if (FieldInterface)
      GV->setMetadata("spirv.cxx.field_interfaces",
                      llvm::MDNode::get(C, FieldInterfaces));
    if (BuiltinInterfaceType)
      GV->setMetadata("spirv.cxx.builtin_interface",
                      llvm::MDNode::get(C, llvm::ConstantAsMetadata::get(
                                               llvm::PoisonValue::get(
                                                   BuiltinInterfaceType))));
    if (Context.getAsConstantArrayType(Element) &&
        !P->hasAttr<SPIRVPatchAttr>() &&
        ((AS == 7 &&
          (StageName == "geometry" || StageName == "tessellation_control" ||
           StageName == "tessellation_evaluation")) ||
         (AS == 8 &&
          (StageName == "tessellation_control" || StageName == "mesh"))))
      GV->setMetadata("spirv.cxx.array_interface", llvm::MDNode::get(C, {}));
    if (Resource.Name == "input_attachment")
      addDecoration(GV, 43, Resource.Format);
    if (Resource.Name == "tile_image")
      addDecoration(GV, 30, Resource.Format);
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
  if (FragDepths)
    M.getOrInsertNamedMetadata("spirv.ExecutionMode")
        ->addOperand(llvm::MDNode::get(
            C, {llvm::ValueAsMetadata::get(Entry), integerMD(C, 12)}));
  if (StencilRefs)
    M.getOrInsertNamedMetadata("spirv.ExecutionMode")
        ->addOperand(llvm::MDNode::get(
            C, {llvm::ValueAsMetadata::get(Entry), integerMD(C, 5027)}));
  bool DepthGreater = D->hasAttr<SPIRVDepthGreaterAttr>();
  bool DepthLess = D->hasAttr<SPIRVDepthLessAttr>();
  bool DepthUnchanged = D->hasAttr<SPIRVDepthUnchangedAttr>();
  if (D->hasAttr<SPIRVEarlyFragmentTestsAttr>() ||
      D->hasAttr<SPIRVPostDepthCoverageAttr>() ||
      D->hasAttr<SPIRVNoncoherentColorAttachmentReadAttr>() ||
      D->hasAttr<SPIRVNoncoherentDepthAttachmentReadAttr>() ||
      D->hasAttr<SPIRVNoncoherentStencilAttachmentReadAttr>() || DepthGreater ||
      DepthLess || DepthUnchanged) {
    if (Stage->getStage() != SPIRVEntryPointAttr::Fragment) {
      Fail("fragment execution modes require a fragment entry point");
      Entry->eraseFromParent();
      return;
    }
    if (unsigned(DepthGreater) + unsigned(DepthLess) +
            unsigned(DepthUnchanged) >
        1) {
      Fail("fragment depth modes are mutually exclusive");
      Entry->eraseFromParent();
      return;
    }
    if ((DepthGreater || DepthLess || DepthUnchanged) && !FragDepths) {
      Fail("fragment depth modes require a __frag_depth output");
      Entry->eraseFromParent();
      return;
    }
    auto AddMode = [&](unsigned Mode) {
      M.getOrInsertNamedMetadata("spirv.ExecutionMode")
          ->addOperand(llvm::MDNode::get(
              C, {llvm::ValueAsMetadata::get(Entry), integerMD(C, Mode)}));
    };
    if (D->hasAttr<SPIRVEarlyFragmentTestsAttr>())
      AddMode(9);
    if (D->hasAttr<SPIRVPostDepthCoverageAttr>())
      AddMode(4446);
    if (D->hasAttr<SPIRVNoncoherentColorAttachmentReadAttr>())
      AddMode(4169);
    if (D->hasAttr<SPIRVNoncoherentDepthAttachmentReadAttr>())
      AddMode(4170);
    if (D->hasAttr<SPIRVNoncoherentStencilAttachmentReadAttr>())
      AddMode(4171);
    if (DepthGreater)
      AddMode(14);
    if (DepthLess)
      AddMode(15);
    if (DepthUnchanged)
      AddMode(16);
  }
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
  llvm::Type *Element = getTypes().ConvertType(S.Element);
  if (R.Name == "tile_image" && S.Element->isSignedIntegerType())
    Element =
        llvm::TargetExtType::get(C, "spirv.SignedImageInteger", {Element});
  return llvm::TargetExtType::get(
      C, "spirv.Image", {Element},
      {R.Dim, unsigned(R.Name.starts_with("depth_")), R.Arrayed, R.MS,
       R.Sampled,
       R.Name == "input_attachment" || R.Name == "tile_image" ? 0u : R.Format});
}
