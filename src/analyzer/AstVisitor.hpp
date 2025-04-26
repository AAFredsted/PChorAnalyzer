#pragma once
#include <memory>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>

namespace PchorAST {

class ParticipantASTNode;
class ChannelASTNode;
class LabelASTNode;
class GlobalTypeASTNode;
class IndexASTNode;
class CommunicationExpr;
class ExprList;
class ParticipantExpr;
class ChannelExpr;
class IndexExpr;
class RecExpr;
class ConExpr;

class PchorASTVisitor {

public:
    
    struct Context {

    };
    PchorASTVisitor(clang::ASTContext &clangContext): clangContext(clangContext), ctx(std::make_shared<Context>()) {}

    ~PchorASTVisitor() = default;

    //Visiting Declarations
    void visit(const ParticipantASTNode& node);
    void visit(const ChannelASTNode& node);
    void visit(const LabelASTNode& node);
    void visit(const GlobalTypeASTNode& node);
    void visit(const IndexASTNode& node);

    //Visiting Expressions
    void visit(const CommunicationExpr& expr);
    void visit(const ExprList& expr);
    void visit(const ParticipantExpr& expr);
    void visit(const ChannelExpr& expr);
    void visit(const IndexExpr& expr);
    void visit(const RecExpr& expr);
    void visit(const ConExpr& expr);

    std::shared_ptr<Context> getContext(){
        return ctx;
    }

private:
    clang::ASTContext &clangContext;
    std::shared_ptr<Context> ctx;

    const clang::Decl* tryFindClangDecl(const std::string& val);
    const clang::Stmt* tryFindClangStmt(const std::string& val);

};

} //namespace PchorAST