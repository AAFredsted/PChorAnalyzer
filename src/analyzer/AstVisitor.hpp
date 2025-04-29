#pragma once
#include <memory>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>
#include "../pchor/PchorAST.hpp"

namespace PchorAST {


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


};

} //namespace PchorAST