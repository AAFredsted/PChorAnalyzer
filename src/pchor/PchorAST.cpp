#include "PchorAST.hpp"
#include "../analyzer/AstVisitor.hpp"

namespace PchorAST {
    void IndexASTNode::accept(CAST_PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }

    void ParticipantASTNode::accept(CAST_PchorASTVisitor &visitor) const {
        visitor.visit(*this);
      }
    
    void ChannelASTNode::accept(CAST_PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }

    void LabelASTNode::accept(CAST_PchorASTVisitor &visitor) const {
        visitor.visit(*this);
      }
    void IndexExpr::accept(CAST_PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }

    void ParticipantExpr::accept(CAST_PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }

    void ChannelExpr::accept(CAST_PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }

    void CommunicationExpr::accept(CAST_PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }

    void ExprList::accept(CAST_PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }

    void ConExpr::accept(CAST_PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }
      
    void RecExpr::accept(CAST_PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }

    void GlobalTypeASTNode::accept(CAST_PchorASTVisitor &visitor) const {
        visitor.visit(*this);
    }
     
}