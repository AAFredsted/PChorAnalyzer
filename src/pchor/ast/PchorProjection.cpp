#include "PchorProjection.hpp"
#include "../../analyzer/utils/CASTAnalyzerUtils.hpp"
#include "../../analyzer/utils/ContextManager.hpp"
namespace PchorAST {

static std::unordered_set<std::string> sendSet{
    "CXXOperatorCallExpr", "CallExpr", "BinaryOperator",
    "ExprWithCleanups"
};
static std::unordered_set<std::string> recieveSet{};

void Psend::validateFunctionDecl(
    clang::ASTContext &context, std::shared_ptr<PchorAST::CASTMapping> &CASTmap,
    clang::Stmt::const_child_iterator &itr,
    clang::Stmt::const_child_iterator &end) {
  // we set a param to a value
  auto cpy = itr;
  bool matchingdone = false;

  while (!matchingdone) {
    if (cpy == end) {
      throw std::runtime_error(
          std::format("Reached end of function before matching projection of send "
                      "type {} at statement: {}",
                      this->getChannelString(), itr->getStmtClassName()));
    }
    std::string type = cpy->getStmtClassName();
    std::println("We try to match expression of type {}", type);
    if (sendSet.contains(type)) {
      std::println("we enter here");
      // expand on this here:
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
      else {
        std::println("we did not match..");
      }
    }
    cpy++;
  }
  itr = cpy;
  std::println("sendMatching successfull");
  // we set that declaration equal to another
}
void Precieve::validateFunctionDecl(
    clang::ASTContext &context, std::shared_ptr<PchorAST::CASTMapping> &CASTmap,
    clang::Stmt::const_child_iterator &itr,
    clang::Stmt::const_child_iterator &end) {
    
    auto cpy = itr;
    bool waitMatchingDone = false;
    bool accessMatchingDone = false;

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
            throw std::runtime_error(
                std::format("Reached end of function before matching projection of "
                            "receive type {} at statement: {}",
                            this->getChannelString(), itr->getStmtClassName()));
        }
        std::string type = cpy->getStmtClassName();
        if(type == "WhileStmt"){
            
            std::println("We try to map whilestatement of");
            cpy->dump();

            const auto* whileStmt = *cpy;
            
            if(AnalyzerUtils::validateRecieveExpression(whileStmt, channelDecl, typeDecl, context)){
                waitMatchingDone=true;
            }
            else {
                std::println("Failed to map recievestatement");
            }
            
            waitMatchingDone=true;
        }
        cpy++;
    }
    while(!accessMatchingDone){
        if(cpy == end){
            throw std::runtime_error(
                std::format("Reached end of function before matching projection of "
                            "send type {} at statement: {}",
                            this->getChannelString(), itr->getStmtClassName()));
        }
       std::string type = cpy->getStmtClassName();
        if(type == "BinaryOperator"){
            accessMatchingDone=true;
        }   
        cpy++;
    }
    itr = cpy;
    std::println("recieveMatching Successfull");
};

} // namespace PchorAST