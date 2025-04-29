#pragma once

#include <clang/AST/ASTContext.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <llvm/Support/raw_ostream.h>
#include <string>

#include <clang/ASTMatchers/ASTMatchFinder.h>

template <typename NodeType>
class MatchCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
    explicit MatchCallback(const NodeType*& result, const std::string& bindName)
        : result(result), bindName(bindName) {}

    void run(const clang::ast_matchers::MatchFinder::MatchResult& matchResult) override {
        if (const auto* node = matchResult.Nodes.getNodeAs<NodeType>(bindName)) {
            llvm::outs() << "Found node: " << bindName << "\n";
            result = node;
        }
    }

private:
    const NodeType*& result;
    std::string bindName;
};

namespace PchorAST {
//class only providing static functions
class AnalyzerUtils {
public:
    static const clang::Decl* findDecl(clang::ASTContext& context, const std::string& name) {

        auto matcher = clang::ast_matchers::namedDecl(clang::ast_matchers::hasName(name)).bind("namedDecl");

        const clang::NamedDecl* result = nullptr;

        MatchCallback<clang::NamedDecl> callback(result, "namedDecl");
        clang::ast_matchers::MatchFinder finder;
        finder.addMatcher(matcher, &callback);
        finder.matchAST(context);

        return result;

    }
    static const clang::Stmt* findStmt(clang::ASTContext& context, const std::string& name) {
        return nullptr;
    }
private: 
};
} //namespace PchorAST

