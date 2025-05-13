#include "PchorProjection.hpp"
#include "../../analyzer/utils/ContextManager.hpp"
namespace PchorAST {

void Psend::validateFunctionDecl(std::shared_ptr<PchorAST::CASTMapping>& CASTmap, clang::Stmt::const_child_iterator& itr, clang::Stmt::const_child_iterator& end) {
    //we set a param to a value
    auto cpy = itr;
    bool matchingdone = false;

    while(!matchingdone){
        if(cpy == end){
        throw std::runtime_error(std::format("Reached end of function before matching projection of type {} at statement: {}", this->getChannelString(), itr->getStmtClassName()));
        }
        std::string type = cpy->getStmtClassName();
        if(type == "CXXOperatorCallExpr") {
            //expand on this here:
            auto decl = CASTmap->getMapping<const clang::Decl*>(this->channelName);

            matchingdone = true;
        }
        cpy++;
    }
    itr = cpy;
    //we set that declaration equal to another
};

} //namespace PchorAST