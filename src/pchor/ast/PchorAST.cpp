#include "PchorAST.hpp"
#include "../../analyzer/visitors/AstVisitor.hpp"

namespace PchorAST {
void IndexASTNode::accept(AbstractPchorASTVisitor &visitor) const {
  visitor.visit(*this);
}

void ParticipantASTNode::accept(AbstractPchorASTVisitor &visitor) const {
  visitor.visit(*this);
}

void ChannelASTNode::accept(AbstractPchorASTVisitor &visitor) const {
  visitor.visit(*this);
}

void LabelASTNode::accept(AbstractPchorASTVisitor &visitor) const {
  visitor.visit(*this);
}
void IndexExpr::accept(AbstractPchorASTVisitor &visitor) const {
  visitor.visit(*this);
}

void ParticipantExpr::accept(AbstractPchorASTVisitor &visitor) const {
  visitor.visit(*this);
}

void ChannelExpr::accept(AbstractPchorASTVisitor &visitor) const {
  visitor.visit(*this);
}

void CommunicationExpr::accept(AbstractPchorASTVisitor &visitor) const {
  visitor.visit(*this);
}

void ExprList::accept(AbstractPchorASTVisitor &visitor) const {
  visitor.visit(*this);
}

void ConExpr::accept(AbstractPchorASTVisitor &visitor) const {
  visitor.visit(*this);
}

void RecExpr::accept(AbstractPchorASTVisitor &visitor) const {
  visitor.visit(*this);
}

void GlobalTypeASTNode::accept(AbstractPchorASTVisitor &visitor) const {
  visitor.visit(*this);
}

} // namespace PchorAST