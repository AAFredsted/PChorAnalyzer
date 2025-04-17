#include "PchorAST.hpp"

namespace PchorAST {

// Definition of the accept method for DeclPchorASTNode
void DeclPchorASTNode::accept(PchorASTVisitor &visitor) const {
    //donothing
}

} // namespace PchorAST