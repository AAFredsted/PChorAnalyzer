#pragma once
#include <memory>

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
    PchorASTVisitor(): ctx(std::make_shared<Context>()) {}

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

private:
    std::shared_ptr<Context> ctx;
};

} //namespace PchorAST