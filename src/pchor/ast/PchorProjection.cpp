#include "PchorProjection.hpp"
#include "../../analyzer/utils/CASTAnalyzerUtils.hpp"
#include "../../analyzer/utils/ContextManager.hpp"
namespace PchorAST {

static std::unordered_set<std::string> sendSet{
    "CXXOperatorCallExpr", "CallExpr", "BinaryOperator",
    "ExprWithCleanups"
};
static std::unordered_set<std::string> recieveSet{};

bool Psend::validateFunctionDecl(
    clang::ASTContext &context, std::shared_ptr<PchorAST::CASTMapping> &CASTmap,
    clang::Stmt::const_child_iterator &itr,
    clang::Stmt::const_child_iterator &end) {
  // we set a param to a value
  auto cpy = itr;
  bool matchingdone = false;

  while (!matchingdone) {
    if (cpy == end) {
        llvm::errs() << std::format("Reached end of function before matching projection of send "
                      "type {} at statement: {}\n",
                      this->getChannelString(), itr->getStmtClassName());
        break;
    }
    std::string type = cpy->getStmtClassName();
    if (sendSet.contains(type)) {

      const auto *opExpr = *cpy;
      const auto *channelDecl =
          CASTmap->getMapping<const clang::Decl *>(this->channelName);
      const auto *typeDecl =
          CASTmap->getMapping<const clang::Decl *>(this->typeName);

      if (!opExpr || !channelDecl || !typeDecl) {
        throw std::runtime_error(
            std::format("Failed to Retrieve data from CASTmap"));
      }
      if (AnalyzerUtils::validateSendExpression(opExpr, channelDecl, typeDecl, context)) {
        matchingdone = true;
      }
    }
    cpy++;
  }
  itr = cpy;
  return matchingdone;
}
bool Precieve::validateFunctionDecl(
    clang::ASTContext &context, std::shared_ptr<PchorAST::CASTMapping> &CASTmap,
    clang::Stmt::const_child_iterator &itr,
    clang::Stmt::const_child_iterator &end) {
    
    auto cpy = itr;
    bool waitMatchingDone = false;

    const auto *channelDecl =
        CASTmap->getMapping<const clang::Decl *>(this->channelName);
    const auto *typeDecl =
        CASTmap->getMapping<const clang::Decl *>(this->typeName);

    if (!channelDecl || !typeDecl) {
        throw std::runtime_error(
            std::format("Failed to Retrieve data from CASTmap"));
    }

    while(!waitMatchingDone){
        if(cpy == end){
            llvm::outs() << std::format("Reached end of function before matching projection of "
                            "receive type {} at statement: {}\n",
                            this->getChannelString(), itr->getStmtClassName());
            break;
        }
        std::string type = cpy->getStmtClassName();
        if(type == "WhileStmt"){ 

            const auto* whileStmt = *cpy;
            if(AnalyzerUtils::validateRecieveExpression(whileStmt, channelDecl, typeDecl, context)){
                waitMatchingDone=true;
            }
            
        }
        cpy++;
    }
    itr = cpy;
    return waitMatchingDone;
};

} // namespace PchorAST