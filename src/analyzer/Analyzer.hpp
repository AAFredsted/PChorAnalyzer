#pragma once

#include <clang/AST/ASTContext.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <llvm/Support/raw_ostream.h>
#include <string>
#include <variant>
#include <cstdint>

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


enum class ContextType {
    Decl,
    Stmt
};
struct Context {
    ContextType type;
    std::variant<const clang::Decl*, const clang::Stmt*> value;
    Context(const clang::Decl* decl): type(ContextType::Decl), value(decl) {}
    Context(const clang::Stmt* stmt): type(ContextType::Stmt), value(stmt) {}

    ContextType getType(){
        return type;
    }
    
    const clang::Decl* getDecl() const {
        return std::get<const clang::Decl*>(value);
    }

    const clang::Stmt* getStmt() const {
        return std::get<const clang::Stmt*>(value);
    }
};

class CASTMapping {
public:
    CASTMapping(): map() {}

    void addMapping(const std::string& name, const clang::Decl* decl) {
        map[name] = Context(decl);
    }

    void addMapping(const std::string& name, const clang::Stmt* stmt) {
        map[name] = Context(stmt);
    }

private:
    std::unordered_map<std::string, Context> map;};
} //namespace PchorAST

