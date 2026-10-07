//===-- SemaSPIRVShader.cpp - Native C++ shaders
//----------------------------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "clang/Basic/Builtins.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Sema/ParsedAttr.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/SemaSPIRV.h"
#include "llvm/ADT/StringSwitch.h"

using namespace clang;

void SemaSPIRV::InitializeShaderCXX() {
  ASTContext &C = SemaRef.Context;
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
  DeclContext *TU = C.getTranslationUnitDecl();
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

  // A real scoped enum, rather than integer macros or a source header.
  IdentifierInfo *ID = &C.Idents.get("ImageFormat");
  if (!TU->lookup(ID).empty())
    return;
  auto *E = EnumDecl::Create(C, TU, {}, {}, ID, nullptr, true, true, true);
  E->setImplicit();
  E->setIntegerType(C.UnsignedIntTy);
  E->startDefinition();
  QualType ET = C.getCanonicalTagType(E);
  static constexpr const char *Names[] = {
      "Unknown",    "Rgba32f",     "Rgba16f",   "R32f",         "Rgba8",
      "Rgba8Snorm", "Rg32f",       "Rg16f",     "R11fG11fB10f", "R16f",
      "Rgba16",     "Rgb10A2",     "Rg16",      "Rg8",          "R16",
      "R8",         "Rgba16Snorm", "Rg16Snorm", "Rg8Snorm",     "R16Snorm",
      "R8Snorm",    "Rgba32i",     "Rgba16i",   "Rgba8i",       "R32i",
      "Rg32i",      "Rg16i",       "Rg8i",      "R16i",         "R8i",
      "Rgba32ui",   "Rgba16ui",    "Rgba8ui",   "R32ui",        "Rgb10a2ui",
      "Rg32ui",     "Rg16ui",      "Rg8ui",     "R16ui",        "R8ui",
      "R64ui",      "R64i"};
  for (unsigned I = 0; I != std::size(Names); ++I) {
    auto *EC =
        EnumConstantDecl::Create(C, E, {}, &C.Idents.get(Names[I]), ET, nullptr,
                                 llvm::APSInt(llvm::APInt(32, I), true));
    EC->setImplicit();
    E->addDecl(EC);
  }
  E->completeDefinition(C.UnsignedIntTy, C.UnsignedIntTy, 32, 0);
  SemaRef.PushOnScopeChains(E, SemaRef.TUScope);
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
    if (auto *Old = D->getAttr<SPIRVBindingAttr>()) {
      if (Old->getSet() != Values[0] || Old->getBinding() != Values[1])
        Error("conflicting descriptor bindings");
      return;
    }
    D->addAttr(new (C) SPIRVBindingAttr(C, AL, Values[0], Values[1]));
    break;
  case ParsedAttr::AT_SPIRVInput:
  case ParsedAttr::AT_SPIRVOutput: {
    auto *V = cast<VarDecl>(D);
    bool Input = AL.getKind() == ParsedAttr::AT_SPIRVInput;
    if (Input) {
      V->addAttr(new (C) SPIRVInputAttr(C, AL, Values[0]));
    } else {
      V->addAttr(new (C) SPIRVOutputAttr(C, AL, Values[0]));
    }
    // Globals themselves occupy the interface space; parameter pointers have
    // interface-space pointees, while the pointer variable is ordinary C++.
    QualType T = V->getType();
    bool Pointer = T->isPointerType();
    QualType Element = Pointer ? T->getPointeeType() : T;
    if (Pointer || !isa<ParmVarDecl>(V))
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
