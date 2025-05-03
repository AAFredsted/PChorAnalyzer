#pragma once
#include <memory>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>
#include "../pchor/PchorAST.hpp"
#include "Analyzer.hpp"

namespace PchorAST {


class AbstractPchorASTVisitor {
public:
    AbstractPchorASTVisitor(clang::ASTContext& clangContext) : clangContext(clangContext) {}
    virtual ~AbstractPchorASTVisitor() = default;

    //Visiting Declarations
    virtual void visit(const ParticipantASTNode& node) = 0;
    virtual void visit(const ChannelASTNode& node) = 0;
    virtual void visit(const LabelASTNode& node) = 0;
    virtual void visit(const GlobalTypeASTNode& node) = 0;
    virtual void visit(const IndexASTNode& node) = 0;

    //Visiting Expressions
    virtual void visit(const CommunicationExpr& expr) = 0;
    virtual void visit(const ExprList& expr) = 0;
    virtual void visit(const ParticipantExpr& expr) = 0;
    virtual void visit(const ChannelExpr& expr) = 0;
    virtual void visit(const IndexExpr& expr) = 0;
    virtual void visit(const RecExpr& expr) = 0;
    virtual void visit(const ConExpr& expr) = 0;
    
protected:
    clang::ASTContext& clangContext;
};

class CAST_PchorASTVisitor : public AbstractPchorASTVisitor {

public:
    
    CAST_PchorASTVisitor(clang::ASTContext &clangContext): AbstractPchorASTVisitor(clangContext), ctx(std::make_shared<CASTMapping>()), mappingSuccess(true), currentDataType("") {}

    ~CAST_PchorASTVisitor() = default;

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

    std::shared_ptr<CASTMapping> getContext(){
        return ctx;
    }

private:
    std::shared_ptr<CASTMapping> ctx;
    std::string currentDataType;
    bool mappingSuccess;

};

} //namespace PchorAST