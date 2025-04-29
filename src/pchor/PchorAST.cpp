#include "PchorAST.hpp"
#include "../analyzer/AstVisitor.hpp"

namespace PchorAST {
    void IndexASTNode::accept(PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }

    void ParticipantASTNode::accept(PchorASTVisitor &visitor) const {
        visitor.visit(*this);
      }
    
    void ChannelASTNode::accept(PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }

    void LabelASTNode::accept(PchorASTVisitor &visitor) const {
        visitor.visit(*this);
      }
    void IndexExpr::accept(PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }

    void ParticipantExpr::accept(PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }

    void ChannelExpr::accept(PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }

    void CommunicationExpr::accept(PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }

    void ExprList::accept(PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }

    void ConExpr::accept(PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }
      
    void RecExpr::accept(PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }

    void GlobalTypeASTNode::accept(PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }
     
}