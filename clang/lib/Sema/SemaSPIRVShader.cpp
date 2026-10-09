//===-- SemaSPIRVShader.cpp - Native C++ shaders --------------------------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "clang/AST/SPIRVShader.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Basic/Builtins.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Sema/ParsedAttr.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/SemaSPIRV.h"
#include "llvm/ADT/StringSwitch.h"
#include <initializer_list>

using namespace clang;

void SemaSPIRV::InitializeShaderCXX() {
  ASTContext &C = SemaRef.Context;
  DeclContext *TU = C.getTranslationUnitDecl();
  // CUDA-style scalar math names are native builtins, available without math.h.
#define SHADER_MATH(Name)                                                      \
  C.Idents.get(#Name).setBuiltinID(Builtin::BI__builtin_##Name)
  SHADER_MATH(sqrtf);
  SHADER_MATH(sinf);
  SHADER_MATH(cosf);
  SHADER_MATH(expf);
  SHADER_MATH(exp2f);
  SHADER_MATH(logf);
  SHADER_MATH(log2f);
  SHADER_MATH(fabsf);
  SHADER_MATH(floorf);
  SHADER_MATH(ceilf);
  SHADER_MATH(truncf);
  SHADER_MATH(fminf);
  SHADER_MATH(fmaxf);
  SHADER_MATH(fmaf);
#undef SHADER_MATH
  auto AddMathOverloads = [&](llvm::StringRef Name, unsigned ID) {
    auto *T = TemplateTypeParmDecl::Create(C, TU, {}, {}, 0, 0,
                                           &C.Idents.get("Args"), false, true);
    QualType TT = C.getTemplateTypeParmType(0, 0, true, T);
    QualType Pack =
        C.getPackExpansionType(C.getRValueReferenceType(TT), std::nullopt);
    auto *Params = TemplateParameterList::Create(C, {}, {}, {T}, {}, nullptr);
    FunctionProtoType::ExtProtoInfo Info;
    QualType FunctionType = C.getFunctionType(C.VoidTy, {Pack}, Info);
    auto *FD =
        FunctionDecl::Create(C, TU, {}, {}, &C.Idents.get(Name), FunctionType,
                             C.getTrivialTypeSourceInfo(FunctionType), SC_None);
    FD->setImplicit();
    auto *P =
        ParmVarDecl::Create(C, FD, {}, {}, &C.Idents.get("args"), Pack,
                            C.getTrivialTypeSourceInfo(Pack), SC_None, nullptr);
    FD->setParams({P});
    FD->getTypeSourceInfo()
        ->getTypeLoc()
        .castAs<FunctionProtoTypeLoc>()
        .setParam(0, P);
    auto *BuiltinName = &C.Idents.get(("__builtin_shader_" + Name).str());
    BuiltinName->setBuiltinID(ID);
    FD->addAttr(BuiltinAliasAttr::CreateImplicit(C, BuiltinName));
    auto *FT =
        FunctionTemplateDecl::Create(C, TU, {}, FD->getDeclName(), Params, FD);
    FT->setImplicit();
    FD->setDescribedFunctionTemplate(FT);
    SemaRef.PushOnScopeChains(FT, SemaRef.TUScope);
  };
#define SPIRV_SHADER_INTRINSIC(Name, Arity)                                    \
  if (!llvm::StringRef(#Name).starts_with("texture_") &&                       \
      !llvm::StringRef(#Name).starts_with("ray_query_") &&                     \
      llvm::StringRef(#Name) != "descriptor_subscript")                        \
    AddMathOverloads(#Name, Builtin::BI__builtin_shader_##Name);
#include "clang/Basic/SPIRVShaderIntrinsics.def"
  if (SemaRef.getLangOpts().ShaderHostMath)
    return;
  auto AddBuiltin = [&](llvm::StringRef Name, QualType Element, unsigned Count,
                        unsigned Builtin) {
    IdentifierInfo *ID = &C.Idents.get(Name);
    if (!TU->lookup(ID).empty())
      return;
    QualType T = Element.withConst();
    if (Count)
      T = C.getConstantArrayType(
          T, llvm::APInt(C.getTypeSize(C.getSizeType()), Count), nullptr,
          ArraySizeModifier::Normal, 0);
    auto *V = VarDecl::Create(C, TU, {}, {}, ID, T,
                              C.getTrivialTypeSourceInfo(T), SC_Extern);
    V->setImplicit();
    V->addAttr(SPIRVBuiltinVariableAttr::CreateImplicit(C, Builtin));
    SemaRef.PushOnScopeChains(V, SemaRef.TUScope);
  };
#define SPIRV_SHADER_BUILTIN(Name, Type, Count, Builtin)                       \
  AddBuiltin(#Name, C.Type##Ty, Count, Builtin);
#include "clang/Basic/SPIRVShaderBuiltins.def"

  // A real unscoped enum; enumerators are available without qualification.
  IdentifierInfo *ID = &C.Idents.get("format");
  if (!TU->lookup(ID).empty())
    return;
  auto *E = EnumDecl::Create(C, TU, {}, {}, ID, nullptr, false, false, true);
  E->setImplicit();
  E->setIntegerType(C.UnsignedIntTy);
  E->startDefinition();
  QualType ET = C.getCanonicalTagType(E);
  static constexpr const char *Names[] = {
      "unknown",        "rgba32f",     "rgba16f",    "r32f",
      "rgba8",          "rgba8_snorm", "rg32f",      "rg16f",
      "r11f_g11f_b10f", "r16f",        "rgba16",     "rgb10_a2",
      "rg16",           "rg8",         "r16",        "r8",
      "rgba16_snorm",   "rg16_snorm",  "rg8_snorm",  "r16_snorm",
      "r8_snorm",       "rgba32i",     "rgba16i",    "rgba8i",
      "r32i",           "rg32i",       "rg16i",      "rg8i",
      "r16i",           "r8i",         "rgba32ui",   "rgba16ui",
      "rgba8ui",        "r32ui",       "rgb10_a2ui", "rg32ui",
      "rg16ui",         "rg8ui",       "r16ui",      "r8ui",
      "r64ui",          "r64i"};
  for (unsigned I = 0; I != std::size(Names); ++I) {
    auto *EC =
        EnumConstantDecl::Create(C, E, {}, &C.Idents.get(Names[I]), ET, nullptr,
                                 llvm::APSInt(llvm::APInt(32, I), true));
    EC->setImplicit();
    E->addDecl(EC);
    SemaRef.PushOnScopeChains(EC, SemaRef.TUScope, /*AddToContext=*/false);
  }
  E->completeDefinition(C.UnsignedIntTy, C.UnsignedIntTy, 32, 0);
  SemaRef.PushOnScopeChains(E, SemaRef.TUScope);
  auto AddAtomicEnum = [&](llvm::StringRef Name,
                           std::initializer_list<std::pair<llvm::StringRef,
                                                           unsigned>> Values) {
    auto *Enum = EnumDecl::Create(C, TU, {}, {}, &C.Idents.get(Name), nullptr,
                                  true, true, true);
    Enum->setImplicit();
    Enum->setIntegerType(C.UnsignedIntTy);
    Enum->startDefinition();
    QualType EnumType = C.getCanonicalTagType(Enum);
    for (auto [Member, Value] : Values) {
      auto *Constant = EnumConstantDecl::Create(
          C, Enum, {}, &C.Idents.get(Member), EnumType, nullptr,
          llvm::APSInt(llvm::APInt(32, Value), true));
      Constant->setImplicit();
      Enum->addDecl(Constant);
    }
    Enum->completeDefinition(C.UnsignedIntTy, C.UnsignedIntTy, 32, 0);
    SemaRef.PushOnScopeChains(Enum, SemaRef.TUScope);
  };
  AddAtomicEnum("memory_order", {{"relaxed", 0},
                                  {"acquire", 2},
                                  {"release", 4},
                                  {"acquire_release", 8},
                                  {"seq_cst", 16}});
  AddAtomicEnum("memory_scope", {{"device", 1},
                                  {"workgroup", 2},
                                  {"subgroup", 3}});
  auto AddResource = [&](llvm::StringRef Name, bool Template,
                         QualType ValueParameter = QualType()) {
    auto *R = CXXRecordDecl::Create(C, TagTypeKind::Struct, TU, {}, {},
                                    &C.Idents.get(("__shader_" + Name).str()));
    R->setImplicit();
    R->startDefinition();
    bool Texture = Name.contains("texture");
    bool Query = Name == "ray_query";
    if (Texture || Query) {
      QualType Handle = C.getPointerType(C.getAddrSpaceQualType(
          C.VoidTy, getLangASFromTargetAS(Texture ? 2 : 0)));
      auto *Field = FieldDecl::Create(
          C, R, {}, {}, &C.Idents.get("__handle"), Handle,
          C.getTrivialTypeSourceInfo(Handle), nullptr, false, ICIS_NoInit);
      Field->setImplicit();
      Field->setAccess(AS_private);
      R->addDecl(Field);
      auto AddMethod = [&](llvm::StringRef MethodName, bool Const) {
        FunctionProtoType::ExtProtoInfo Info;
        Info.Variadic = true;
        if (Const)
          Info.TypeQuals.addConst();
        QualType MethodType = C.getFunctionType(C.VoidTy, {}, Info);
        auto *Method = CXXMethodDecl::Create(
            C, R, {}, DeclarationNameInfo(&C.Idents.get(MethodName), {}),
            MethodType, C.getTrivialTypeSourceInfo(MethodType), SC_None, false,
            false, ConstexprSpecKind::Unspecified, {}, {});
        Method->setImplicit();
        Method->setAccess(AS_public);
        R->addDecl(Method);
      };
      if (Texture) {
        for (llvm::StringRef MethodName :
             {"load", "store", "sample", "sample_lod", "sample_grad", "gather",
              "dimensions", "levels", "samples", "lod", "sample_bias",
              "sample_offset", "sample_lod_offset", "sample_grad_offset",
              "sample_bias_offset", "gather_offset"})
          AddMethod(MethodName, true);
      } else {
#define SPIRV_SHADER_INTRINSIC(Name, Arity)                                    \
  if (llvm::StringRef(#Name).starts_with("ray_query_")) {                      \
    llvm::StringRef MethodName = llvm::StringRef(#Name).drop_front(10);        \
    AddMethod(MethodName, shaderRayQueryGetter(MethodName));                   \
  }
#include "clang/Basic/SPIRVShaderIntrinsics.def"
      }
    }
    R->completeDefinition();
    QualType Proxy = C.getCanonicalTagType(R);
    TemplateParameterList *Params = nullptr;
    if (Template) {
      auto *T = TemplateTypeParmDecl::Create(C, TU, {}, {}, 0, 0,
                                             &C.Idents.get("T"), false, false);
      QualType TT = C.getTemplateTypeParmType(0, 0, false, T);
      SmallVector<NamedDecl *> Parameters{T};
      SmallVector<TemplateArgument> Arguments{TemplateArgument(TT)};
      if (!ValueParameter.isNull()) {
        auto *N = NonTypeTemplateParmDecl::Create(
            C, TU, {}, {}, 0, 1, &C.Idents.get("N"), ValueParameter, false,
            C.getTrivialTypeSourceInfo(ValueParameter));
        Parameters.push_back(N);
        Arguments.emplace_back(
            DeclRefExpr::Create(C, {}, {}, N, false,
                                DeclarationNameInfo(N->getDeclName(), {}),
                                ValueParameter, VK_LValue),
            false);
      }
      Params =
          TemplateParameterList::Create(C, {}, {}, Parameters, {}, nullptr);
      auto *CT =
          ClassTemplateDecl::Create(C, TU, {}, R->getIdentifier(), Params, R);
      CT->setImplicit();
      R->setDescribedClassTemplate(CT);
      TU->addDecl(CT);
      SmallVector<TemplateArgument> Canonical;
      for (const auto &Arg : Arguments)
        Canonical.push_back(C.getCanonicalTemplateArgument(Arg));
      Proxy = C.getTemplateSpecializationType(
          ElaboratedTypeKeyword::None, TemplateName(CT), Arguments, Canonical);
    } else {
      TU->addDecl(R);
    }
    if (!Texture && Name != "ray_query")
      Proxy =
          C.getAddrSpaceQualType(Proxy.withConst(), getLangASFromTargetAS(2));
    auto *Alias = TypeAliasDecl::Create(
        C, TU, {}, {}, &C.Idents.get(Name),
        C.getTrivialTypeSourceInfo(
            (Texture || Query) ? Proxy : C.getPointerType(Proxy)));
    Alias->setImplicit();
    if (Template) {
      auto *AT = TypeAliasTemplateDecl::Create(
          C, TU, {}, Alias->getIdentifier(), Params, Alias);
      AT->setImplicit();
      Alias->setDescribedAliasTemplate(AT);
      SemaRef.PushOnScopeChains(AT, SemaRef.TUScope);
    } else {
      SemaRef.PushOnScopeChains(Alias, SemaRef.TUScope);
    }
  };
  for (llvm::StringRef Name :
       {"texture_1d", "texture_2d", "texture_3d", "texture_cube",
        "texture_1d_layers", "texture_2d_layers", "texture_cube_layers",
        "texture_2d_ms", "texture_2d_ms_layers", "texture_buffer"})
    AddResource(Name, true);
  for (llvm::StringRef Name :
       {"storage_texture_1d", "storage_texture_2d", "storage_texture_3d",
        "storage_texture_1d_layers", "storage_texture_2d_layers",
        "storage_texture_2d_ms", "storage_texture_2d_ms_layers",
        "storage_texture_buffer"})
    AddResource(Name, true, ET);
  AddResource("sampler", false);
  AddResource("sampler_comparison", false);
  AddResource("acceleration_structure", false);
  AddResource("ray_query", false);
}

void SemaSPIRV::HandleShaderAttribute(Decl *D, const ParsedAttr &AL) {
  ASTContext &C = SemaRef.Context;
  auto Error = [&](llvm::StringRef Message) {
    SemaRef.Diag(AL.getLoc(), diag::err_spirv_shader) << Message;
  };
  if (AL.getKind() == ParsedAttr::AT_SPIRVEntryPoint) {
    auto *FD = cast<FunctionDecl>(D);
    if (isa<CXXMethodDecl>(FD) || !FD->getReturnType()->isVoidType() ||
        FD->isVariadic()) {
      Error("entry points must be non-variadic free functions returning void");
      return;
    }
    auto Stage = static_cast<SPIRVEntryPointAttr::StageKind>(
        AL.getAttributeSpellingListIndex());
    if (auto *Old = FD->getAttr<SPIRVEntryPointAttr>()) {
      if (Old->getStage() != Stage)
        Error("conflicting entry-point stages");
      return;
    }
    FD->addAttr(new (C) SPIRVEntryPointAttr(C, AL, Stage));
    return;
  }
  // Interface placement is checked after all parameter attributes are known,
  // so qualifier order does not change the meaning of a declaration.
  switch (AL.getKind()) {
  case ParsedAttr::AT_SPIRVFlat:
  case ParsedAttr::AT_SPIRVNoPerspective:
  case ParsedAttr::AT_SPIRVCentroid:
  case ParsedAttr::AT_SPIRVSample:
    if (!isa<ParmVarDecl>(D)) {
      Error("interpolation qualifiers must be declared on entry-point "
            "parameters");
      return;
    }
    if (AL.getKind() == ParsedAttr::AT_SPIRVFlat)
      D->addAttr(new (C) SPIRVFlatAttr(C, AL));
    else if (AL.getKind() == ParsedAttr::AT_SPIRVNoPerspective)
      D->addAttr(new (C) SPIRVNoPerspectiveAttr(C, AL));
    else if (AL.getKind() == ParsedAttr::AT_SPIRVCentroid)
      D->addAttr(new (C) SPIRVCentroidAttr(C, AL));
    else
      D->addAttr(new (C) SPIRVSampleAttr(C, AL));
    if ((D->hasAttr<SPIRVFlatAttr>() &&
         (D->hasAttr<SPIRVNoPerspectiveAttr>() ||
          D->hasAttr<SPIRVCentroidAttr>() || D->hasAttr<SPIRVSampleAttr>())) ||
        (D->hasAttr<SPIRVCentroidAttr>() && D->hasAttr<SPIRVSampleAttr>()))
      Error("conflicting interpolation qualifiers");
    return;
  default:
    break;
  }
  uint32_t Values[3] = {};
  for (unsigned I = 0; I != AL.getNumArgs(); ++I)
    if (!SemaRef.checkUInt32Argument(AL, AL.getArgAsExpr(I), Values[I], I + 1,
                                     true))
      return;
  switch (AL.getKind()) {
  case ParsedAttr::AT_SPIRVWorkgroupSize:
    if (!Values[0] || !Values[1] || !Values[2]) {
      Error("workgroup dimensions must be positive");
      return;
    }
    if (auto *Old = D->getAttr<SPIRVWorkgroupSizeAttr>()) {
      if (Old->getX() != Values[0] || Old->getY() != Values[1] ||
          Old->getZ() != Values[2])
        Error("conflicting workgroup sizes");
      return;
    }
    D->addAttr(
        new (C) SPIRVWorkgroupSizeAttr(C, AL, Values[0], Values[1], Values[2]));
    break;
  case ParsedAttr::AT_SPIRVBinding:
    if (!isa<ParmVarDecl>(D)) {
      Error("shader bindings must be declared on entry-point parameters; "
            "global resources are not supported");
      return;
    }
    if (auto *Old = D->getAttr<SPIRVBindingAttr>()) {
      if (Old->getSet() != Values[0] || Old->getBinding() != Values[1])
        Error("conflicting descriptor bindings");
      return;
    }
    D->addAttr(new (C) SPIRVBindingAttr(C, AL, Values[0], Values[1]));
    break;
  case ParsedAttr::AT_SPIRVInput:
  case ParsedAttr::AT_SPIRVOutput:
  case ParsedAttr::AT_SPIRVPosition: {
    auto *V = cast<VarDecl>(D);
    if (!isa<ParmVarDecl>(V)) {
      Error("shader inputs and outputs must be declared on entry-point "
            "parameters; global interfaces are not supported");
      return;
    }
    bool Input = AL.getKind() == ParsedAttr::AT_SPIRVInput;
    if (Input) {
      V->addAttr(new (C) SPIRVInputAttr(C, AL, Values[0]));
    } else if (AL.getKind() == ParsedAttr::AT_SPIRVPosition) {
      V->addAttr(new (C) SPIRVPositionAttr(C, AL));
    } else {
      V->addAttr(new (C) SPIRVOutputAttr(C, AL, Values[0]));
    }
    // Parameter pointers have interface-space pointees, while the pointer
    // variable and scalar value parameters are ordinary C++.
    QualType T = V->getType();
    bool Pointer = T->isPointerType();
    QualType Element = Pointer ? T->getPointeeType() : T;
    if (Pointer)
      Element =
          C.getAddrSpaceQualType(Element, getLangASFromTargetAS(Input ? 7 : 8));
    if (Input)
      Element = Element.withConst();
    V->setType(Pointer ? C.getPointerType(Element) : Element);
    break;
  }
  default:
    llvm_unreachable("unexpected C++ shader attribute");
  }
}

QualType SemaSPIRV::GetShaderResultType(QualType Element, unsigned Count,
                                        unsigned Rows, unsigned Columns,
                                        bool ForceAggregate) {
  ASTContext &C = SemaRef.Context;
  Element = Element.getUnqualifiedType();
  if (Count == 1 && !Rows && !ForceAggregate)
    return Element;
  std::string Name = ("__shader_result_" +
                      llvm::Twine(Element->castAs<BuiltinType>()->getKind()) +
                      "_" + llvm::Twine(Count) + "_" + llvm::Twine(Rows) + "_" +
                      llvm::Twine(Columns))
                         .str();
  auto *TU = C.getTranslationUnitDecl();
  auto *ID = &C.Idents.get(Name);
  auto Found = TU->lookup(ID);
  if (!Found.empty())
    return C.getCanonicalTagType(cast<CXXRecordDecl>(*Found.begin()));
  auto *R = CXXRecordDecl::Create(C, TagTypeKind::Struct, TU, {}, {}, ID);
  R->setImplicit();
  R->startDefinition();
  auto Array = [&](QualType T, unsigned N) {
    return C.getConstantArrayType(
        T, llvm::APInt(C.getTypeSize(C.getSizeType()), N), nullptr,
        ArraySizeModifier::Normal, 0);
  };
  QualType DataType =
      Rows ? Array(Array(Element, Columns), Rows) : Array(Element, Count);
  auto *Data = FieldDecl::Create(C, R, {}, {}, &C.Idents.get("data"), DataType,
                                 C.getTrivialTypeSourceInfo(DataType), nullptr,
                                 false, ICIS_NoInit);
  Data->setImplicit();
  Data->setAccess(AS_public);
  R->addDecl(Data);
  R->completeDefinition();
  TU->addDecl(R);
  return C.getCanonicalTagType(R);
}

ExprResult SemaSPIRV::BuildShaderResultSubscript(Expr *Base, Expr *Index,
                                                 SourceLocation LBracketLoc,
                                                 SourceLocation RBracketLoc) {
  ASTContext &C = SemaRef.Context;
  if (Base->isPRValue()) {
    ExprResult Materialized = SemaRef.TemporaryMaterializationConversion(Base);
    if (Materialized.isInvalid())
      return ExprError();
    Base = Materialized.get();
  }
  auto *Data = *Base->getType()->getAsCXXRecordDecl()->field_begin();
  QualType T = Data->getType();
  if (Base->getType().isConstQualified())
    T = C.getConstType(T);
  auto *Member = MemberExpr::CreateImplicit(C, Base, false, Data, T,
                                            Base->getValueKind(), OK_Ordinary);
  return SemaRef.CreateBuiltinArraySubscriptExpr(Member, LBracketLoc, Index,
                                                 RBracketLoc);
}

bool SemaSPIRV::CheckShaderBuiltin(unsigned ID, CallExpr *Call) {
  ASTContext &C = SemaRef.Context;
  llvm::StringRef Name = shaderIntrinsicName(ID);
  auto Error = [&](llvm::StringRef Message) {
    SemaRef.Diag(Call->getExprLoc(), diag::err_spirv_shader) << Message;
    return true;
  };
  if (!SemaRef.getLangOpts().SPIRVShader &&
      !SemaRef.getLangOpts().ShaderHostMath)
    return Error(
        "shader intrinsics require -fspirv-shader or -fshader-host-math");
  if (!Name.starts_with("texture_") && !Name.starts_with("atomic") &&
      SemaRef.checkArgCount(Call, shaderIntrinsicArity(ID)))
    return true;
  if (Call->isTypeDependent())
    return false;
  auto Type = [&](unsigned I) { return Call->getArg(I)->getType(); };
  auto Shape = [&](unsigned I) { return shaderShape(C, Type(I)); };
  auto Scalar = [&](unsigned I, QualType T) {
    auto V = SemaRef.DefaultFunctionArrayLvalueConversion(Call->getArg(I));
    if (V.isInvalid())
      return false;
    V = SemaRef.PerformImplicitConversion(V.get(), T,
                                          AssignmentAction::Passing);
    if (V.isInvalid())
      return false;
    Call->setArg(I, V.get());
    return true;
  };
  bool Math = shaderComponentMath(Name) || Name == "dot" || Name == "length" ||
              Name == "distance" || Name == "normalize" || Name == "cross" ||
              Name == "reflect" || Name == "refract" || Name == "faceforward" ||
              Name == "transpose" || Name == "determinant" ||
              Name == "inverse" || Name == "outer_product" || Name == "modf" ||
              Name == "frexp" || Name == "ldexp";
  Math |= Name == "is_nan" || Name == "is_inf" || Name == "is_finite" ||
          Name == "is_normal" || Name == "sign_bit";
  if (SemaRef.getLangOpts().ShaderHostMath && !Math)
    return Error("host math mode does not provide shader resources or device "
                 "operations");
  if (Name == "add_carry" || Name == "sub_borrow" ||
      Name == "mul_extended") {
    auto A = Shape(0), B = Shape(1);
    if (!A || !B || A.Rows || B.Rows || A.Count > 4 ||
        A.Count != B.Count || !C.hasSameType(A.Element, B.Element) ||
        !A.Element->isIntegerType() ||
        (C.getTypeSize(A.Element) != 32 && C.getTypeSize(A.Element) != 64) ||
        (Name != "mul_extended" && !A.Element->isUnsignedIntegerType()))
      return Error("extended integer arithmetic requires matching 32-bit or "
                   "64-bit integer scalars or vectors; carry and borrow "
                   "require unsigned elements");
    for (unsigned I = 0; I != 2; ++I)
      if (Type(I)->isScalarType() && !Scalar(I, A.Element))
        return true;
    std::string RecordName =
        ("__cxx_shader_" + Name + "_" +
         llvm::Twine(A.Element->castAs<BuiltinType>()->getKind()) + "_" +
         llvm::Twine(A.Count))
            .str();
    auto *TU = C.getTranslationUnitDecl();
    auto *Ident = &C.Idents.get(RecordName);
    auto Existing = TU->lookup(Ident);
    if (!Existing.empty()) {
      Call->setType(
          C.getCanonicalTagType(cast<CXXRecordDecl>(*Existing.begin())));
      return false;
    }
    auto *Record =
        CXXRecordDecl::Create(C, TagTypeKind::Struct, TU, {}, {}, Ident);
    Record->setImplicit();
    Record->startDefinition();
    QualType FieldType = GetShaderResultType(A.Element, A.Count);
    for (llvm::StringRef FieldName :
         {Name == "mul_extended" ? "low" : "value",
          Name == "add_carry"   ? "carry"
          : Name == "sub_borrow" ? "borrow"
                                 : "high"}) {
      auto *Field = FieldDecl::Create(
          C, Record, {}, {}, &C.Idents.get(FieldName), FieldType,
          C.getTrivialTypeSourceInfo(FieldType), nullptr, false, ICIS_NoInit);
      Field->setImplicit();
      Field->setAccess(AS_public);
      Record->addDecl(Field);
    }
    Record->completeDefinition();
    TU->addDecl(Record);
    Call->setType(C.getCanonicalTagType(Record));
    return false;
  }
  if (Name == "modf" || Name == "frexp" || Name == "ldexp") {
    auto S = Shape(0);
    if (!S || S.Rows || S.Count > 4 || !S.Element->isRealFloatingType())
      return Error("decomposition intrinsics require floating scalars or "
                   "vectors of at most four components");
    if (Type(0)->isScalarType() && !Scalar(0, S.Element))
      return true;
    if (Name == "ldexp") {
      auto Exponent = Shape(1);
      if (!Exponent || Exponent.Rows || Exponent.Count != S.Count ||
          !C.hasSameType(Exponent.Element, C.IntTy))
        return Error(
            "ldexp requires signed 32-bit exponents matching the value shape");
      if (Type(1)->isScalarType() && !Scalar(1, C.IntTy))
        return true;
      Call->setType(GetShaderResultType(S.Element, S.Count));
    } else {
      QualType First = GetShaderResultType(S.Element, S.Count);
      QualType Second =
          Name == "frexp" ? GetShaderResultType(C.IntTy, S.Count) : First;
      std::string RecordName =
          ("__cxx_shader_" + Name + "_" +
           llvm::Twine(S.Element->castAs<BuiltinType>()->getKind()) + "_" +
           llvm::Twine(S.Count))
              .str();
      auto *TU = C.getTranslationUnitDecl();
      auto *Ident = &C.Idents.get(RecordName);
      auto Existing = TU->lookup(Ident);
      if (!Existing.empty()) {
        Call->setType(
            C.getCanonicalTagType(cast<CXXRecordDecl>(*Existing.begin())));
        return false;
      }
      auto *Record =
          CXXRecordDecl::Create(C, TagTypeKind::Struct, TU, {}, {}, Ident);
      Record->setImplicit();
      Record->startDefinition();
      for (auto [FieldName, T] :
           {std::pair<llvm::StringRef, QualType>{
                Name == "frexp" ? "significand" : "fraction", First},
            {Name == "frexp" ? "exponent" : "whole", Second}}) {
        auto *Field = FieldDecl::Create(
            C, Record, {}, {}, &C.Idents.get(FieldName), T,
            C.getTrivialTypeSourceInfo(T), nullptr, false, ICIS_NoInit);
        Field->setImplicit();
        Field->setAccess(AS_public);
        Record->addDecl(Field);
      }
      Record->completeDefinition();
      TU->addDecl(Record);
      Call->setType(C.getCanonicalTagType(Record));
    }
    return false;
  }
  if (Name == "is_nan" || Name == "is_inf" || Name == "is_finite" ||
      Name == "is_normal" || Name == "sign_bit") {
    auto S = Shape(0);
    if (!S || S.Rows || S.Count > 4 || !S.Element->isRealFloatingType())
      return Error("floating classification requires a floating scalar or "
                   "vector of at most four components");
    if (Type(0)->isScalarType() && !Scalar(0, S.Element))
      return true;
    Call->setType(GetShaderResultType(
        S.Count == 1 ? C.BoolTy : C.UnsignedIntTy, S.Count));
    return false;
  }
  if (Name == "shader_clock" || Name == "memory_barrier" ||
      Name == "control_barrier") {
    for (unsigned I = 0; I < Call->getNumArgs(); ++I) {
      Expr::EvalResult Value;
      if (!Call->getArg(I)->EvaluateAsInt(Value, C) ||
          Value.Val.getInt().isNegative() ||
          Value.Val.getInt().getActiveBits() > 32)
        return Error("scope and memory semantics must be constant unsigned "
                     "32-bit integers");
      if (!Scalar(I, C.UnsignedIntTy))
        return true;
    }
    if (Name == "shader_clock") {
      Expr::EvalResult Scope;
      Call->getArg(0)->EvaluateAsInt(Scope, C);
      if (Scope.Val.getInt() != 1 && Scope.Val.getInt() != 3)
        return Error("shader_clock scope must be 1 (device) or 3 (subgroup)");
    }
    Call->setType(Name == "shader_clock" ? C.UnsignedLongLongTy : C.VoidTy);
    return false;
  }
  if (Name == "outer_product" ||
      (Name == "mul" && (Shape(0).Rows || Shape(1).Rows))) {
    auto A = Shape(0), B = Shape(1);
    bool Scale = Name == "mul" &&
                 (Type(0)->isArithmeticType() || Type(1)->isArithmeticType());
    QualType Element = Scale ? (A.Rows ? A.Element : B.Element) : A.Element;
    if (!A || !B || !Element->isRealFloatingType() ||
        (!Scale && !C.hasSameType(A.Element, B.Element)) || A.Count > 16 ||
        B.Count > 16 || A.Rows > 4 || A.Columns > 4 || B.Rows > 4 ||
        B.Columns > 4)
      return Error("matrix multiplication requires matching floating element "
                   "types and dimensions of at most four");
    unsigned Rows = 0, Columns = 0, Count = 0;
    if (Name == "outer_product") {
      if (A.Rows || B.Rows || A.Count < 2 || A.Count > 4 || B.Count < 2 ||
          B.Count > 4)
        return Error(
            "outer_product requires two vectors of two to four components");
      Rows = A.Count;
      Columns = B.Count;
      Count = Rows * Columns;
    } else if (Scale) {
      auto M = A.Rows ? A : B;
      Rows = M.Rows;
      Columns = M.Columns;
      Count = M.Count;
      if (!Scalar(Type(0)->isArithmeticType() ? 0 : 1, M.Element))
        return true;
    } else if (A.Rows && B.Rows) {
      if (A.Columns != B.Rows)
        return Error("matrix multiplication dimensions do not match");
      Rows = A.Rows;
      Columns = B.Columns;
      Count = Rows * Columns;
    } else if (A.Rows) {
      if (A.Columns != B.Count || B.Count > 4)
        return Error("matrix-vector multiplication dimensions do not match");
      Count = A.Rows;
    } else {
      if (A.Count != B.Rows || A.Count > 4)
        return Error("vector-matrix multiplication dimensions do not match");
      Count = B.Columns;
    }
    Call->setType(
        GetShaderResultType(Element, Count, Rows, Columns, Count == 1));
    return false;
  }
  if (Name.starts_with("pack_") || Name.starts_with("unpack_")) {
    bool Unpack = Name.starts_with("unpack_");
    bool Double = Name.ends_with("double2x32");
    unsigned Count = Name.ends_with("4x8") ? 4 : 2;
    QualType Element = Double ? C.UnsignedIntTy : C.FloatTy;
    if (Unpack) {
      QualType Input = Double ? C.DoubleTy : C.UnsignedIntTy;
      if (!Type(0)->isScalarType() ||
          !C.hasSameType(Type(0).getUnqualifiedType(), Input))
        return Error("unpack requires an unsigned 32-bit scalar (double for "
                     "unpack_double2x32)");
      if (!Scalar(0, Input))
        return true;
      Call->setType(GetShaderResultType(Element, Count));
    } else {
      auto S = Shape(0);
      if (!S || S.Rows || S.Count != Count ||
          !C.hasSameType(S.Element, Element))
        return Error("pack requires float[2/4] or unsigned[2] for "
                     "pack_double2x32, or an equivalent homogeneous record");
      Call->setType(Double ? C.DoubleTy : C.UnsignedIntTy);
    }
    return false;
  }
  if (Name.starts_with("bit_") || Name == "find_lsb" || Name == "find_msb") {
    auto S = Shape(0);
    if (!S || S.Rows || S.Count > 4 || !S.Element->isIntegerType() ||
        (C.getTypeSize(S.Element) != 32 && C.getTypeSize(S.Element) != 64))
      return Error(
          "bit operations require 32-bit or 64-bit integer scalars or vectors");
    if (Type(0)->isScalarType() && !Scalar(0, S.Element))
      return true;
    unsigned Start = 1;
    if (Name == "bit_insert") {
      auto B = Shape(1);
      if (!B || B.Rows || B.Count != S.Count ||
          !C.hasSameType(B.Element, S.Element))
        return Error("bit_insert base and inserted value types must match");
      if (Type(1)->isScalarType() && !Scalar(1, S.Element))
        return true;
      Start = 2;
    }
    for (unsigned I = Start; I < Call->getNumArgs(); ++I) {
      if (!Type(I)->isIntegerType())
        return Error("bit offset and count must be integer scalars");
      Expr::EvalResult V;
      if (Call->getArg(I)->EvaluateAsInt(V, C) &&
          (V.Val.getInt().isNegative() ||
           V.Val.getInt().getLimitedValue() > C.getTypeSize(S.Element)))
        return Error("bit offset or count exceeds the element width");
      if (!Scalar(I, C.UnsignedIntTy))
        return true;
    }
    if (Name == "bit_extract" || Name == "bit_insert") {
      Expr::EvalResult Offset, Count;
      if (Call->getArg(Start)->EvaluateAsInt(Offset, C) &&
          Call->getArg(Start + 1)->EvaluateAsInt(Count, C) &&
          Offset.Val.getInt().getLimitedValue() +
                  Count.Val.getInt().getLimitedValue() >
              C.getTypeSize(S.Element))
        return Error("bit offset plus count exceeds the element width");
    }
    QualType Element =
        Name == "find_lsb" || Name == "find_msb" ? C.IntTy : S.Element;
    Call->setType(GetShaderResultType(Element, S.Count));
    return false;
  }
  bool ScalarMath = shaderComponentMath(Name) && Type(0)->isArithmeticType();
  if (ScalarMath) {
    for (unsigned I = 0; I < Call->getNumArgs(); ++I)
      if (!Type(I)->isArithmeticType())
        return Error("scalar math overloads require arithmetic operands");
    ExprResult L = SemaRef.UsualUnaryConversions(Call->getArg(0));
    if (L.isInvalid())
      return true;
    QualType Common = L.get()->getType().getUnqualifiedType();
    for (unsigned I = 1; I < Call->getNumArgs(); ++I) {
      ExprResult R = Call->getArg(I);
      Common = SemaRef.UsualArithmeticConversions(L, R, Call->getExprLoc(),
                                                  ArithConvKind::Arithmetic);
      if (Common.isNull())
        return true;
    }
    if (!shaderIntegerMath(Name) && Common->isIntegerType())
      Common = C.DoubleTy;
    for (unsigned I = 0; I < Call->getNumArgs(); ++I)
      if (!Scalar(I, Common))
        return true;
    Call->setType(Common);
    return false;
  }
  if (Math) {
    bool Matrix = (Name == "mul" && Shape(0).Rows) || Name == "transpose" ||
                  Name == "determinant" || Name == "inverse";
    bool ReturnsScalar = Name == "dot" || Name == "length" ||
                         Name == "distance" || Name == "determinant";
    ShaderShape S = Shape(0);
    if (!S ||
        (!S.Element->isRealFloatingType() &&
         !(!Matrix && shaderIntegerMath(Name) && S.Element->isIntegerType())) ||
        (Matrix ? (!S.Rows || S.Rows > 4 || S.Columns > 4)
                : (S.Rows || S.Count < 2 || S.Count > 4)))
      return Error("math operands must be floating-point C arrays or "
                   "homogeneous records of the required shape");
    for (unsigned I = 1; I < Call->getNumArgs(); ++I) {
      if (Name == "refract" && I == 2) {
        if (!Scalar(I, S.Element))
          return true;
        continue;
      }
      ShaderShape Other = Shape(I);
      if (!Other || !C.hasSameType(S.Element, Other.Element))
        return Error("math operand element types must match");
      if (Matrix && (!Other.Rows || Other.Rows > 4 || Other.Columns > 4))
        return Error(
            "matrix operands must have two dimensions of at most four");
      if (!Matrix && (Other.Count != S.Count || Other.Rows))
        return Error("vector operand dimensions must match");
    }
    if (Name == "cross" && S.Count != 3)
      return Error("cross requires three components");
    if (Matrix && Name == "mul" && S.Columns != Shape(1).Rows)
      return Error("matrix multiplication dimensions do not match");
    if ((Name == "inverse" || Name == "determinant") && S.Rows != S.Columns)
      return Error("inverse and determinant require matching square matrices");
    if (ReturnsScalar)
      Call->setType(S.Element);
    else if (Matrix) {
      unsigned Rows = Name == "transpose" ? S.Columns : S.Rows;
      unsigned Columns = Name == "transpose" ? S.Rows
                         : Name == "mul"     ? Shape(1).Columns
                                             : S.Columns;
      Call->setType(
          GetShaderResultType(S.Element, Rows * Columns, Rows, Columns));
    } else
      Call->setType(Type(0)->isRecordType()
                        ? Type(0).getUnqualifiedType()
                        : GetShaderResultType(S.Element, S.Count));
    return false;
  }
  if (Name == "specialization_constant") {
    Expr::EvalResult Id, Default;
    if (!Call->getArg(0)->EvaluateAsInt(Id, C) ||
        Id.Val.getInt().isNegative() || Id.Val.getInt().getActiveBits() > 32 ||
        !Call->getArg(1)->EvaluateAsRValue(Default, C) ||
        (!Default.Val.isInt() && !Default.Val.isFloat()) ||
        (!Type(1)->isIntegerType() && !Type(1)->isRealFloatingType()))
      return Error("specialization_constant requires a constant unsigned "
                   "32-bit ID and a constant scalar default");
    if (!Scalar(1, Type(1).getUnqualifiedType()))
      return true;
    Call->setType(Type(1).getUnqualifiedType());
    return false;
  }
  if (Name.starts_with("atomic")) {
    unsigned BaseCount = shaderIntrinsicArity(ID);
    bool Explicit = Call->getNumArgs() ==
                    BaseCount + (Name == "atomicCAS" ? 3u : 2u);
    if (Call->getNumArgs() != BaseCount && !Explicit)
      return Error("atomic operation requires its existing arguments or "
                   "explicit order and scope arguments");
    if (!Type(0)->isPointerType())
      return Error("atomic operations require a pointer");
    QualType T = Type(0)->getPointeeType();
    unsigned AS = C.getTargetAddressSpace(T.getAddressSpace());
    if ((!T->isIntegerType() && !T->isRealFloatingType()) ||
        (C.getTypeSize(T) != 32 && C.getTypeSize(T) != 64) ||
        (AS != 3 && AS != 14) || T.isConstQualified())
      return Error("atomic operations require writable 32-bit or 64-bit shared "
                   "or storage integers");
    if (T->isRealFloatingType() && Name != "atomicAdd" && Name != "atomicSub" &&
        Name != "atomicMin" && Name != "atomicMax")
      return Error("floating atomic overloads provide atomicAdd, atomicSub, "
                   "atomicMin and atomicMax");
    for (unsigned I = 1; I < BaseCount; ++I)
      if (!Scalar(I, T.getUnqualifiedType()))
        return true;
    if (Explicit) {
      auto EnumType = [&](llvm::StringRef EnumName) {
        auto Lookup =
            C.getTranslationUnitDecl()->lookup(&C.Idents.get(EnumName));
        return C.getCanonicalTagType(cast<EnumDecl>(*Lookup.begin()));
      };
      QualType OrderType = EnumType("memory_order");
      QualType ScopeType = EnumType("memory_scope");
      unsigned ScopeIndex = Call->getNumArgs() - 1;
      unsigned FailureIndex = Name == "atomicCAS" ? BaseCount + 1 : 0;
      unsigned Order = 0, Failure = 0, Scope = 0;
      auto ReadEnum = [&](unsigned Index, QualType Expected,
                          unsigned &Value) {
        Expr::EvalResult Result;
        if (!C.hasSameUnqualifiedType(Type(Index), Expected) ||
            !Call->getArg(Index)->EvaluateAsInt(Result, C))
          return false;
        Value = Result.Val.getInt().getZExtValue();
        return true;
      };
      if (!ReadEnum(BaseCount, OrderType, Order) ||
          !ReadEnum(ScopeIndex, ScopeType, Scope) ||
          (FailureIndex && !ReadEnum(FailureIndex, OrderType, Failure)))
        return Error("atomic order and scope must be constant memory_order "
                     "and memory_scope values");
      if (Order == 16 || (FailureIndex && Failure == 16))
        return Error("sequentially consistent atomics are unavailable in "
                     "Vulkan shaders");
      bool ValidOrder = Name == "atomicLoad"
                            ? Order == 0 || Order == 2 || Order == 16
                        : Name == "atomicStore"
                            ? Order == 0 || Order == 4 || Order == 16
                            : Order == 0 || Order == 2 || Order == 4 ||
                                  Order == 8 || Order == 16;
      if (!ValidOrder)
        return Error("memory order is invalid for this atomic operation");
      if (FailureIndex &&
          !((Failure == 0) ||
            (Failure == 2 && (Order == 2 || Order == 8 || Order == 16)) ||
            (Failure == 16 && Order == 16)))
        return Error("compare-exchange failure order cannot release or be "
                     "stronger than success order");
      if ((AS == 3 && Scope != 2 && Scope != 3) ||
          (AS == 14 && Scope != 1 && Scope != 2 && Scope != 3))
        return Error("atomic memory scope is invalid for the pointer space");
    }
    Call->setType(Name == "atomicStore" ? C.VoidTy : T.getUnqualifiedType());
    return false;
  }
  if (Name.starts_with("subgroup_")) {
    Call->setType(C.VoidTy);
    if (Name == "subgroup_barrier")
      return false;
    if (Name == "subgroup_elect") {
      Call->setType(C.BoolTy);
      return false;
    }
    if (Name == "subgroup_ballot") {
      if (!Type(0)->isArithmeticType())
        return Error("subgroup_ballot requires a scalar condition");
      Call->setType(GetShaderResultType(C.UnsignedIntTy, 4));
      return !Scalar(0, C.BoolTy);
    }
    if (Name == "subgroup_all" || Name == "subgroup_any") {
      Call->setType(C.BoolTy);
      return !Scalar(0, C.BoolTy);
    }
    if (Name == "subgroup_ballot_bit_count" || Name.contains("ballot_") ||
        Name == "subgroup_inverse_ballot") {
      auto S = Shape(0);
      if (!S || S.Count != 4 || !C.hasSameType(S.Element, C.UnsignedIntTy))
        return Error("ballot bit count requires unsigned[4]");
      Call->setType(Name == "subgroup_inverse_ballot" ||
                            Name.ends_with("bit_extract")
                        ? C.BoolTy
                        : C.UnsignedIntTy);
      if (Call->getNumArgs() == 2 && !Scalar(1, C.UnsignedIntTy))
        return true;
      return false;
    }
    auto ValueShape = Shape(0);
    if (!ValueShape || ValueShape.Rows || ValueShape.Count > 4 ||
        (!ValueShape.Element->isIntegerType() &&
         !ValueShape.Element->isRealFloatingType()))
      return Error("subgroup operations require arithmetic scalars or vectors "
                   "of at most four components");
    bool Logical = Name.contains("logical_");
    if (Logical && !ValueShape.Element->isBooleanType())
      return Error("logical subgroup operations require bool elements");
    if (!Logical && ValueShape.Element->isBooleanType() &&
        (Name.ends_with("_add") || Name.ends_with("_mul") ||
         Name.ends_with("_min") || Name.ends_with("_max") ||
         Name.ends_with("_and") || Name.ends_with("_or") ||
         Name.ends_with("_xor")))
      return Error("arithmetic and bitwise subgroup operations do not accept "
                   "bool");
    if (!Logical && (Name.ends_with("_and") || Name.ends_with("_or") ||
                     Name.ends_with("_xor")) &&
        !ValueShape.Element->isIntegerType())
      return Error("this subgroup operation requires integer elements");
    if (Type(0)->isScalarType() && !Scalar(0, ValueShape.Element))
      return true;
    if (Name.starts_with("subgroup_clustered_")) {
      Expr::EvalResult Cluster;
      if (!Call->getArg(1)->EvaluateAsInt(Cluster, C) ||
          Cluster.Val.getInt().isNegative() ||
          !Cluster.Val.getInt().isPowerOf2())
        return Error("subgroup cluster size must be a positive constant "
                     "power of two");
    }
    if (Call->getNumArgs() == 2 && !Scalar(1, C.UnsignedIntTy))
      return true;
    Call->setType(
        Name == "subgroup_all_equal" ? C.BoolTy
        : Type(0)->isScalarType()    ? ValueShape.Element
        : Type(0)->isRecordType()
            ? Type(0).getUnqualifiedType()
            : GetShaderResultType(ValueShape.Element, ValueShape.Count));
    return false;
  }
  if (Name == "nonuniform") {
    if (!Type(0)->isIntegerType())
      return Error("nonuniform requires an integer descriptor index");
    if (!Scalar(0, Type(0).getUnqualifiedType()))
      return true;
    Call->setType(Type(0));
    return false;
  }
  if (Name == "descriptor_subscript") {
    unsigned ArrayArg =
        shaderResource(Type(0)).Name == "descriptor_array" ? 0 : 1;
    unsigned IndexArg = 1 - ArrayArg;
    auto R = shaderResource(Type(ArrayArg));
    if (R.Name != "descriptor_array")
      return Error("descriptor subscript requires a native resource C array");
    auto Element = shaderResource(R.Descriptor);
    if (!Element || Element.Name == "descriptor_array" ||
        Element.Name == "ray_query")
      return Error(
          "descriptor arrays must contain native descriptor resources");
    if (!Type(IndexArg)->isIntegralOrUnscopedEnumerationType())
      return Error("descriptor array subscript must have an integral type");
    ExprResult Index = SemaRef.UsualUnaryConversions(Call->getArg(IndexArg));
    if (Index.isInvalid())
      return true;
    Call->setArg(IndexArg, Index.get());
    Call->setType(R.Descriptor);
    return false;
  }
  if (Name.starts_with("texture_")) {
    bool Offset = Name.ends_with("_offset");
    llvm::StringRef BaseName = Offset ? Name.drop_back(7) : Name;
    bool Gather = BaseName == "texture_gather";
    auto R = shaderResource(Type(0));
    auto Texel = shaderShape(C, R.Element);
    if (!R || !R.Name.contains("texture") || !Texel || Texel.Rows ||
        Texel.Count > 4 ||
        (!Texel.Element->isRealFloatingType() &&
         !Texel.Element->isIntegerType()) ||
        C.getTypeSize(Texel.Element) != 32)
      return Error("texture operations require a native texture with 32-bit "
                   "scalar or up to four homogeneous texel components");
    bool Sample = Name.starts_with("texture_sample") || Gather ||
                  Name == "texture_lod";
    unsigned Width = (R.Dim == 3 ? 3 : R.Dim == 5 ? 1 : R.Dim + 1) + R.Arrayed;
    unsigned Spatial = Width - R.Arrayed;
    bool Mip = R.Sampled == 1 && !R.MS && R.Dim != 5;
    if (Offset && (R.Dim == 3 || R.Dim == 5))
      return Error("coordinate offsets require a non-cube, non-buffer image");
    if (Name == "texture_levels" || Name == "texture_samples") {
      if (Call->getNumArgs() != 1 || (Name == "texture_levels" ? !Mip : !R.MS))
        return Error("levels requires a sampled mipmapped image; samples "
                     "requires a multisampled image, with no arguments");
      Call->setType(C.UnsignedIntTy);
      return false;
    }
    if (Name == "texture_dimensions") {
      if (Call->getNumArgs() != 1 && !(Mip && Call->getNumArgs() == 2))
        return Error("dimensions requires no arguments or one mip level for a "
                     "sampled texture");
      if (Call->getNumArgs() == 2 && !Scalar(1, C.UnsignedIntTy))
        return true;
      unsigned N = (R.Dim == 3 ? 2 : R.Dim == 5 ? 1 : R.Dim + 1) + R.Arrayed;
      Call->setType(GetShaderResultType(C.UnsignedIntTy, N));
      return false;
    }
    unsigned BaseExtra = BaseName == "texture_sample_lod" ||
                                 BaseName == "texture_sample_bias" || Gather
                         ? 1
                     : BaseName == "texture_sample_grad" ? Spatial * 2
                     : BaseName == "texture_store"       ? Texel.Count
                                                     : 0;
    unsigned Extra = BaseExtra + (Offset ? Spatial : 0);
    unsigned Expected = 1 + unsigned(Sample) + Width + Extra;
    bool AccessSelector = ((Name == "texture_load" && (Mip || R.MS)) ||
                           (Name == "texture_store" && R.MS)) &&
                          Call->getNumArgs() == Expected + 1;
    if (AccessSelector)
      ++Expected;
    if (Call->getNumArgs() != Expected)
      return Error(("texture method " + Name.drop_front(8) + " requires " +
                    llvm::Twine(Expected - 1) + " scalar arguments")
                       .str());
    if (Sample &&
        (R.Sampled != 1 || R.MS || shaderResource(Type(1)).Name != "sampler"))
      return Error("sampling requires a sampled non-multisampled texture and a "
                   "native sampler");
    if (Name == "texture_store" && R.Sampled != 2)
      return Error("texture_store requires a storage texture");
    unsigned Start = Sample ? 2 : 1;
    for (unsigned I = Start; I < Expected; ++I) {
      QualType Target = I < Start + Width ? (Sample ? C.FloatTy : C.IntTy)
                        : Offset && I >= Start + Width + BaseExtra ? C.IntTy
                        : Gather                  ? C.UnsignedIntTy
                        : Name == "texture_store" ? Texel.Element
                                                   : C.FloatTy;
      if (AccessSelector &&
          I == (Name == "texture_load" ? Expected - 1 : Start + Width))
        Target = C.UnsignedIntTy;
      if (!Type(I)->isArithmeticType())
        return Error("texture coordinates, gradients and texel components must "
                     "be scalars");
      if (Offset && I >= Start + Width + BaseExtra) {
        Expr::EvalResult Value;
        if (!Call->getArg(I)->EvaluateAsInt(Value, C) ||
            Value.Val.getInt().getSignificantBits() > 32)
          return Error("texture coordinate offsets must be constant signed "
                       "32-bit integers");
      }
      if (!Scalar(I, Target))
        return true;
    }
    Call->setType(
        Name == "texture_lod"      ? GetShaderResultType(C.FloatTy, 2)
        : Name == "texture_store"  ? C.VoidTy
        : Gather                  ? GetShaderResultType(Texel.Element, 4)
        : R.Element->isArrayType()
            ? GetShaderResultType(Texel.Element, Texel.Count, 0, 0, true)
            : R.Element.getUnqualifiedType());
    return false;
  }
  if (Name == "make_ray_query") {
    auto Result =
        C.getTranslationUnitDecl()->lookup(&C.Idents.get("ray_query"));
    Call->setType(cast<TypeAliasDecl>(*Result.begin())->getUnderlyingType());
    return false;
  }
  if (Name.starts_with("ray_query_")) {
    if (shaderResource(Type(0)).Name != "ray_query")
      return Error("ray query operations require a native ray_query");
    bool Getter = shaderRayQueryGetter(Name);
    if (!Getter && Type(0).isConstQualified())
      return Error("ray query mutation requires a non-const receiver");
    if (Name == "ray_query_initialize") {
      if (shaderResource(Type(1)).Name != "acceleration_structure")
        return Error(
            "ray_query_initialize requires a native acceleration_structure");
      for (unsigned I : {2u, 3u})
        if (!Scalar(I, C.UnsignedIntTy))
          return true;
      for (unsigned I = 4; I < 12; ++I) {
        if (!Type(I)->isArithmeticType())
          return Error("ray origin, direction and distances must be scalars");
        if (!Scalar(I, C.FloatTy))
          return true;
      }
    }
    if (Name == "ray_query_generate_intersection" && !Scalar(1, C.FloatTy))
      return true;
    if (Getter && Call->getNumArgs() == 2) {
      Expr::EvalResult K;
      if (!Call->getArg(1)->EvaluateAsInt(K, C) ||
          K.Val.getInt().getZExtValue() > 1)
        return Error("ray intersection selector must be the constant 0 "
                     "(candidate) or 1 (committed)");
      if (!Scalar(1, C.UnsignedIntTy))
        return true;
    }
    QualType Result = C.VoidTy;
    if (Name == "ray_query_proceed" || Name.ends_with("front_facing") ||
        Name.ends_with("candidate_aabb_opaque"))
      Result = C.BoolTy;
    else if (Name.ends_with("intersection_t") || Name.ends_with("ray_t_min"))
      Result = C.FloatTy;
    else if (Name.ends_with("barycentrics"))
      Result = GetShaderResultType(C.FloatTy, 2);
    else if (Name.ends_with("ray_origin") || Name.ends_with("ray_direction"))
      Result = GetShaderResultType(C.FloatTy, 3);
    else if (Getter)
      Result = C.UnsignedIntTy;
    Call->setType(Result);
    return false;
  }
  if (Name.starts_with("ddx") || Name.starts_with("ddy") || Name == "fwidth") {
    if (!Type(0)->isRealFloatingType())
      return Error("derivatives require a floating-point scalar");
    if (!Scalar(0, Type(0).getUnqualifiedType()))
      return true;
    Call->setType(Type(0));
    return false;
  }
  Call->setType(C.VoidTy);
  return false;
}

ExprResult SemaSPIRV::BuildResourceMethodCall(CXXMethodDecl *Method,
                                              MemberExpr *Member,
                                              ArrayRef<Expr *> Args,
                                              SourceLocation RParenLoc) {
  ASTContext &C = SemaRef.Context;
  llvm::StringRef Name = Method->getName();
  bool Query =
      shaderResource(C.getCanonicalTagType(Method->getParent())).Name ==
      "ray_query";
  std::string IntrinsicName =
      ((Query ? "ray_query_" : "texture_") + Name).str();
  unsigned ID = 0;
#define SPIRV_SHADER_INTRINSIC(BuiltinName, Arity)                             \
  if (IntrinsicName == #BuiltinName)                                           \
    ID = Builtin::BI__builtin_shader_##BuiltinName;
#include "clang/Basic/SPIRVShaderIntrinsics.def"
  assert(ID && "unknown native resource method");
  Expr *Object = Member->getBase();
  if (Member->isArrow()) {
    auto Deref = SemaRef.BuildUnaryOp(nullptr, Member->getOperatorLoc(),
                                      UO_Deref, Object);
    if (Deref.isInvalid())
      return ExprError();
    Object = Deref.get();
  }
  unsigned ObjectIndex = 0;
  if (Query && Args.size() + 1 != shaderIntrinsicArity(ID)) {
    SemaRef.Diag(Member->getMemberLoc(), diag::err_spirv_shader)
        << ((Query ? "ray query method " : "texture method ") + Name +
            " requires " + llvm::Twine(shaderIntrinsicArity(ID) - 1) +
            " arguments")
               .str();
    return ExprError();
  }
  SmallVector<Expr *> NativeArgs(Args.begin(), Args.end());
  NativeArgs.insert(NativeArgs.begin() + ObjectIndex, Object);
  return BuildShaderBuiltinCall(ID, NativeArgs, RParenLoc);
}

ExprResult SemaSPIRV::BuildDescriptorSubscriptExpr(Expr *Base, Expr *Index,
                                                   SourceLocation RBracketLoc) {
  return BuildShaderBuiltinCall(
      Builtin::BI__builtin_shader_descriptor_subscript, {Base, Index},
      RBracketLoc);
}

ExprResult SemaSPIRV::BuildShaderBuiltinCall(unsigned ID, ArrayRef<Expr *> Args,
                                             SourceLocation CallLoc) {
  ASTContext &C = SemaRef.Context;
  auto *II =
      &C.Idents.get(("__builtin_shader_" + shaderIntrinsicName(ID)).str());
  auto *FD = cast<FunctionDecl>(
      SemaRef.LazilyCreateBuiltin(II, ID, SemaRef.TUScope, false, CallLoc));
  ExprResult Fn =
      SemaRef.BuildDeclRefExpr(FD, FD->getType(), VK_LValue, CallLoc);
  Fn = SemaRef.DefaultFunctionArrayLvalueConversion(Fn.get());
  if (Fn.isInvalid())
    return ExprError();
  auto *Call = CallExpr::Create(C, Fn.get(), Args, C.VoidTy, VK_PRValue,
                                CallLoc, SemaRef.CurFPFeatureOverrides());
  if (CheckShaderBuiltin(ID, Call))
    return ExprError();
  return Call;
}
