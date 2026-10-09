//===----- SemaSPIRV.h ----- Semantic Analysis for SPIRV constructs--------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file declares semantic analysis for SPIRV constructs.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_SEMA_SEMASPIRV_H
#define LLVM_CLANG_SEMA_SEMASPIRV_H

#include "clang/AST/ASTFwd.h"
#include "clang/Sema/SemaBase.h"

namespace clang {
class ParsedAttr;
class SemaSPIRV : public SemaBase {
public:
  SemaSPIRV(Sema &S);

  void InitializeShaderCXX();
  bool CheckShaderBuiltin(unsigned BuiltinID, CallExpr *Call);
  ExprResult BuildResourceMethodCall(CXXMethodDecl *Method, MemberExpr *Member,
                                     ArrayRef<Expr *> Args,
                                     SourceLocation RParenLoc);
  QualType GetShaderResultType(QualType Element, unsigned Count,
                               unsigned Rows = 0, unsigned Columns = 0,
                               bool ForceAggregate = false);
  ExprResult BuildShaderResultSubscript(Expr *Base, Expr *Index,
                                        SourceLocation LBracketLoc,
                                        SourceLocation RBracketLoc);
  ExprResult BuildDescriptorSubscriptExpr(Expr *Base, Expr *Index,
                                          SourceLocation RBracketLoc);
  ExprResult BuildShaderBuiltinCall(unsigned ID, ArrayRef<Expr *> Args,
                                    SourceLocation CallLoc);
  void HandleShaderAttribute(Decl *D, const ParsedAttr &AL);

  bool CheckSPIRVBuiltinFunctionCall(const TargetInfo &TI, unsigned BuiltinID,
                                     CallExpr *TheCall);
};
} // namespace clang

#endif // LLVM_CLANG_SEMA_SEMASPIRV_H
