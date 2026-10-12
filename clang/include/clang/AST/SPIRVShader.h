//===-- SPIRVShader.h - Native shader types ------------------------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef LLVM_CLANG_AST_SPIRVSHADER_H
#define LLVM_CLANG_AST_SPIRVSHADER_H
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Basic/Builtins.h"
#include "llvm/ADT/StringSwitch.h"

namespace clang {
inline bool shaderCanCopyFields(ASTContext &Context, QualType Type) {
  if (const auto *Array = Context.getAsConstantArrayType(Type))
    return shaderCanCopyFields(Context, Array->getElementType());
  if (const auto *Record = Type->getAsCXXRecordDecl()) {
    if (!Record->isStandardLayout() || Record->isUnion() ||
        Record->getNumBases())
      return false;
    for (const auto *Field : Record->fields())
      if (Field->isBitField() ||
          !shaderCanCopyFields(Context, Field->getType()))
        return false;
    return true;
  }
  return Type->isScalarType();
}

inline bool shaderTrivialCopyFromAddressSpace(ASTContext &Context,
                                              const CXXConstructorDecl *Ctor,
                                              QualType SourceType) {
  unsigned AS = Context.getTargetAddressSpace(SourceType.getAddressSpace());
  return (AS == 12 || AS == 13 || AS == 14) && Ctor->isCopyConstructor() &&
         Ctor->isMemcpyEquivalentSpecialMember(Context) &&
         Context.hasSameUnqualifiedType(
             SourceType.getUnqualifiedType(),
             Context.getCanonicalTagType(Ctor->getParent())) &&
         shaderCanCopyFields(Context, SourceType);
}


inline llvm::StringRef shaderIntrinsicName(unsigned ID) {
  switch (ID) {
#define SPIRV_SHADER_INTRINSIC(Name, Arity)                                    \
  case Builtin::BI__builtin_shader_##Name:                                     \
    return #Name;
#include "clang/Basic/SPIRVShaderIntrinsics.def"
  default:
    return {};
  }
}
inline unsigned shaderIntrinsicArity(unsigned ID) {
  switch (ID) {
#define SPIRV_SHADER_INTRINSIC(Name, Arity)                                    \
  case Builtin::BI__builtin_shader_##Name:                                     \
    return Arity;
#include "clang/Basic/SPIRVShaderIntrinsics.def"
  default:
    llvm_unreachable("not a shader builtin");
  }
}
inline bool shaderRayQueryGetter(llvm::StringRef Name) {
  Name.consume_front("ray_query_");
  return Name != "initialize" && Name != "proceed" && Name != "terminate" &&
         Name != "confirm_intersection" && Name != "generate_intersection";
}
inline bool shaderComponentMath(llvm::StringRef Name) {
  return llvm::StringSwitch<bool>(Name)
      .Cases({"add", "sub", "mul", "div", "min", "max"}, true)
      .Cases({"clamp", "lerp", "sqrt", "abs", "floor", "ceil"}, true)
      .Cases({"sin", "cos", "acos", "exp", "log", "pow"}, true)
      .Cases({"asin", "atan", "atan2", "tan", "sinh", "cosh", "tanh"}, true)
      .Cases({"asinh", "acosh", "atanh", "radians", "degrees"}, true)
      .Cases({"round", "round_even", "trunc", "sign", "fract", "exp2", "log2"},
             true)
      .Cases({"inverse_sqrt", "step", "smoothstep", "fma", "mod"}, true)
      .Default(false);
}
inline bool shaderIntegerMath(llvm::StringRef Name) {
  return llvm::StringSwitch<bool>(Name)
      .Cases({"add", "sub", "mul", "div", "min", "max", "clamp", "abs", "sign"},
             true)
      .Default(false);
}
struct ShaderShape {
  QualType Element;
  unsigned Count = 0, Rows = 0, Columns = 0;
  explicit operator bool() const { return Count != 0; }
};
// C arrays and standard-layout homogeneous records retain their C++ layout.
// Only builtin marshalling introduces an LLVM vector, never a public vector
// type.
inline ShaderShape shaderShape(ASTContext &C, QualType T) {
  if (T.isNull())
    return {};
  if (T->isRealFloatingType() || T->isIntegerType())
    return {T.getUnqualifiedType(), 1, 0, 0};
  if (auto *A = C.getAsConstantArrayType(T)) {
    auto S = shaderShape(C, A->getElementType());
    unsigned N = A->getSize().getLimitedValue(17);
    if (!S || !N || N > 16 || S.Count * N > 16)
      return {};
    if (C.getAsConstantArrayType(A->getElementType())) {
      if (S.Rows || S.Columns)
        return {};
      S.Rows = N;
      S.Columns = S.Count;
    }
    S.Count *= N;
    return S;
  }
  if (auto *R = T->getAsCXXRecordDecl()) {
    if (R->isImplicit() && R->getName().starts_with("__shader_result_") &&
        R->hasDefinition())
      return shaderShape(C, (*R->field_begin())->getType());
    if (!R->hasDefinition() || !R->isStandardLayout() || R->isUnion() ||
        R->getNumBases())
      return {};
    ShaderShape S;
    for (auto *F : R->fields()) {
      if (F->isBitField())
        return {};
      auto Member = shaderShape(C, F->getType());
      if (!Member || Member.Rows || Member.Columns ||
          (S && !C.hasSameType(S.Element, Member.Element)))
        return {};
      S.Element = Member.Element;
      S.Count += Member.Count;
    }
    if (S.Count > 16)
      return {};
    return S;
  }
  return {};
}
inline bool shaderShapeIsWritable(ASTContext &C, QualType T) {
  if (T.isConstQualified())
    return false;
  if (auto *A = C.getAsArrayType(T))
    return shaderShapeIsWritable(C, A->getElementType());
  if (auto *R = T->getAsCXXRecordDecl())
    for (auto *F : R->fields())
      if (!shaderShapeIsWritable(C, F->getType()))
        return false;
  return true;
}
struct ShaderResource {
  llvm::StringRef Name;
  QualType Element, Descriptor;
  unsigned Dim = 0, Arrayed = 0, MS = 0, Sampled = 1, Format = 0;
  uint64_t Count = 0;
  explicit operator bool() const { return !Name.empty(); }
};
inline ShaderResource shaderResource(QualType T) {
  if (T.isNull())
    return {};
  if (T->isReferenceType())
    T = T->getPointeeType();
  // Native descriptor ranges use real C++ array types. A decayed resource
  // pointer retains the borrowed range semantics when passed to a helper.
  QualType ArrayElement;
  uint64_t Count = 0;
  if (const auto *A = dyn_cast<ArrayType>(T.getCanonicalType())) {
    ArrayElement = A->getElementType();
    if (const auto *Fixed = dyn_cast<ConstantArrayType>(A))
      Count = Fixed->getSize().getZExtValue();
  } else if (T->isPointerType()) {
    QualType Pointee = T->getPointeeType();
    // Sampler/acceleration handles are themselves address-space-2 pointers.
    // A pointer to one of these handles, or to a texture class, is a range.
    if (Pointee.getAddressSpace() != getLangASFromTargetAS(2))
      ArrayElement = Pointee;
  }
  if (!ArrayElement.isNull()) {
    auto Element = shaderResource(ArrayElement);
    if (!Element)
      return {};
    ShaderResource S;
    S.Name = "descriptor_array";
    S.Descriptor = ArrayElement;
    S.Count = Count;
    return S;
  }
  if (T->isPointerType())
    T = T->getPointeeType();
  auto *R = T->getAsCXXRecordDecl();
  const CXXRecordDecl *Definition = R;
  if (auto *S = dyn_cast_or_null<ClassTemplateSpecializationDecl>(R))
    Definition = S->getSpecializedTemplate()->getTemplatedDecl();
  if (!R || !Definition->isImplicit() ||
      !R->getName().starts_with("__shader_") ||
      R->getName().starts_with("__shader_result_"))
    return {};
  ShaderResource S;
  S.Name = R->getName().drop_front(9);
  if (auto *Spec = dyn_cast<ClassTemplateSpecializationDecl>(R)) {
    auto &Args = Spec->getTemplateArgs();
    if (Args.size() && Args[0].getKind() == TemplateArgument::Type)
      S.Element = Args[0].getAsType();
    if (Args.size() > 1)
      S.Format = Args[1].getAsIntegral().getZExtValue();
  }
  S.Sampled =
      S.Name.starts_with("storage_") || S.Name == "input_attachment" ||
              S.Name == "tile_image" ? 2 : 1;
  S.Arrayed = S.Name.ends_with("_layers");
  S.MS = S.Name.contains("_ms");
  S.Dim = S.Name == "tile_image"       ? 4173
          : S.Name == "input_attachment" ? 6
          : S.Name.contains("cube")    ? 3
          : S.Name.contains("3d")      ? 2
          : S.Name.contains("2d")      ? 1
          : S.Name.contains("buffer")  ? 5
                                       : 0;
  return S;
}
} // namespace clang
#endif
