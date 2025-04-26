#pragma once

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>
#include <string>

namespace PchorAST {
//class only providing static functions
class AnalyzerUtils {
public:
    static const clang::Decl* findDecl(clang::ASTContext& context, const std::string& name) {
        return nullptr;
    }
    static const clang::Stmt* findStmt(clang::ASTContext& context, const std::string& name) {
        return nullptr;
    }

};
} //namespace PchorAST