#include "PchorProjection.hpp"
#include "../../analyzer/utils/CASTAnalyzerUtils.hpp"
#include "../../analyzer/utils/ContextManager.hpp"
namespace PchorAST {

static std::unordered_set<std::string> sendSet{
    "CXXOperatorCallExpr", "CallExpr", "BinaryOperator", "ExprWithCleanups",
    "CXXMemberCallExpr"};
static std::unordered_set<std::string> recieveSet{
    "WhileStmt", "ExprWithCleanups", "CXXMemberCallExpr"};

bool Psend::validateFunctionDecl(
    clang::ASTContext &context, std::shared_ptr<PchorAST::CASTMapping> &CASTmap,
    clang::Stmt::const_child_iterator &itr,
    clang::Stmt::const_child_iterator &end,
    AbstractProjection *&parentScopeProjectionPtr) {
  // we set a param to a value
  auto cpy = itr;
  bool matchingdone = false;

  const auto *channelDecl =
      CASTmap->getMapping<const clang::Decl *>(this->channelName);
  const auto *typeDecl =
      CASTmap->getMapping<const clang::Decl *>(this->typeName);

  if (!channelDecl || !typeDecl) {
    throw std::runtime_error(
        std::format("Failed to Retrieve data from CASTmap"));
  }

  AbstractProjection *childScopeProjectionPtr = nullptr;

  while (!matchingdone) {
    if (cpy == end) {
      llvm::errs() << std::format("[PchorValidator] Warning: Reached end of "
                                  "function before matching projection of send "
                                  "type {} at statement: {}\n",
                                  this->getChannelString(),
                                  itr->getStmtClassName());
      break;
    }
    const auto *opExpr = *cpy;
    std::string type = cpy->getStmtClassName();

    if (sendSet.contains(type)) {
      if (AnalyzerUtils::validateSendExpression(opExpr, channelDecl, typeDecl,
                                                context)) {
        matchingdone = true;
      } else if (const clang::FunctionDecl *funcDecl =
                     AnalyzerUtils::findFunctionDefinition(opExpr, context)) {
        const clang::Stmt *body = funcDecl->getBody();

        auto childElm = body->children();
        auto childItr = childElm.begin();
        auto childEnd = childElm.end();

        matchingdone = this->validateFunctionDecl(
            context, CASTmap, childItr, childEnd, childScopeProjectionPtr);
      }
    }
    cpy++;
  }
  itr = cpy;

  if (itr != end) {
    if (childScopeProjectionPtr) {
      return childScopeProjectionPtr->validateFunctionDecl(
          context, CASTmap, itr, end, parentScopeProjectionPtr);
    } else if (this->next) {
      return this->next->validateFunctionDecl(context, CASTmap, itr, end,
                                              parentScopeProjectionPtr);
    } else {
      return matchingdone;
    }
  } else {
    if (matchingdone) {
      parentScopeProjectionPtr = this->next.get();
    } else {
      parentScopeProjectionPtr = this;
    }
    return matchingdone;
  }
}

bool Isend::validateFunctionDecl(
    clang::ASTContext &context, std::shared_ptr<PchorAST::CASTMapping> &CASTmap,
    clang::Stmt::const_child_iterator &itr,
    clang::Stmt::const_child_iterator &end,
    AbstractProjection *&parentScopeProjectionPtr) {
  // we set a param to a value
  auto cpy = itr;
  bool matchingdone = false;

  const auto *channelDecl =
      CASTmap->getMapping<const clang::Decl *>(this->channelName);
  const auto *typeDecl =
      CASTmap->getMapping<const clang::Decl *>(this->typeName);

  if (!channelDecl || !typeDecl) {
    throw std::runtime_error(
        std::format("Failed to Retrieve data from CASTmap"));
  }

  AbstractProjection *childScopeProjectionPtr = nullptr;

  while (!matchingdone) {
    if (cpy == end) {
      llvm::errs() << std::format("[PchorValidator] Warning: Reached end of "
                                  "function before matching projection of send "
                                  "type {} at statement: {}\n",
                                  this->getChannelString(),
                                  itr->getStmtClassName());
      break;
    }
    const auto *opExpr = *cpy;
    std::string type = cpy->getStmtClassName();

    if (sendSet.contains(type)) {
      if (AnalyzerUtils::validateSendExpression(opExpr, channelDecl, typeDecl,
                                                context)) {
        matchingdone = true;
      } else if (const clang::FunctionDecl *funcDecl =
                     AnalyzerUtils::findFunctionDefinition(opExpr, context)) {
        const clang::Stmt *body = funcDecl->getBody();

        auto childElm = body->children();
        auto childItr = childElm.begin();
        auto childEnd = childElm.end();

        matchingdone = this->validateFunctionDecl(
            context, CASTmap, childItr, childEnd, childScopeProjectionPtr);
      }
    }
    cpy++;
  }
  itr = cpy;

  if (itr != end) {
    if (childScopeProjectionPtr) {
      return childScopeProjectionPtr->validateFunctionDecl(
          context, CASTmap, itr, end, parentScopeProjectionPtr);
    } else if (this->next) {
      return this->next->validateFunctionDecl(context, CASTmap, itr, end,
                                              parentScopeProjectionPtr);
    } else {
      return matchingdone;
    }
  } else {
    if (matchingdone) {
      parentScopeProjectionPtr = this->next.get();
    } else {
      parentScopeProjectionPtr = this;
    }
    return matchingdone;
  }
}

bool Preceive::validateFunctionDecl(
    clang::ASTContext &context, std::shared_ptr<PchorAST::CASTMapping> &CASTmap,
    clang::Stmt::const_child_iterator &itr,
    clang::Stmt::const_child_iterator &end,
    AbstractProjection *&parentScopeProjectionPtr) {

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

  AbstractProjection *childScopeProjectionPtr = nullptr;

  while (!waitMatchingDone) {
    if (cpy == end) {
      std::println("[PchorValidator] Warning: Reached end of function before "
                   "matching projection of "
                   "receive type {} at statement: {}\n",
                   this->getChannelString(), itr->getStmtClassName());
      break;
    }
    std::string type = cpy->getStmtClassName();
    if (recieveSet.contains(type)) {

      const auto *whileStmt = *cpy;
      if (AnalyzerUtils::validateRecieveExpression(whileStmt, channelDecl,
                                                   typeDecl, context)) {
        waitMatchingDone = true;
      } else if (const clang::FunctionDecl *funcDecl =
                     AnalyzerUtils::findFunctionDefinition(whileStmt,
                                                           context)) {
        const clang::Stmt *body = funcDecl->getBody();

        auto childElm = body->children();
        auto childItr = childElm.begin();
        auto childEnd = childElm.end();
        waitMatchingDone = this->validateFunctionDecl(
            context, CASTmap, childItr, childEnd, childScopeProjectionPtr);
      }
    }
    cpy++;
  }
  itr = cpy;

  if (itr != end) {
    if (childScopeProjectionPtr) {
      return childScopeProjectionPtr->validateFunctionDecl(
          context, CASTmap, itr, end, parentScopeProjectionPtr);
    } else if (this->next) {
      return this->next->validateFunctionDecl(context, CASTmap, itr, end,
                                              parentScopeProjectionPtr);
    } else {
      return waitMatchingDone;
    }
  } else {
    if (waitMatchingDone) {
      parentScopeProjectionPtr = this->next.get();
    } else {
      parentScopeProjectionPtr = this;
    }
    return waitMatchingDone;
  }
};

bool Ireceive::validateFunctionDecl(
    clang::ASTContext &context, std::shared_ptr<PchorAST::CASTMapping> &CASTmap,
    clang::Stmt::const_child_iterator &itr,
    clang::Stmt::const_child_iterator &end,
    AbstractProjection *&parentScopeProjectionPtr) {

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

  AbstractProjection *childScopeProjectionPtr = nullptr;

  while (!waitMatchingDone) {
    if (cpy == end) {
      std::println("[PchorValidator] Warning: Reached end of function before "
                   "matching projection of "
                   "receive type {} at statement: {}\n",
                   this->getChannelString(), itr->getStmtClassName());
      break;
    }
    std::string type = cpy->getStmtClassName();
    if (recieveSet.contains(type)) {

      const auto *whileStmt = *cpy;
      if (AnalyzerUtils::validateRecieveExpression(whileStmt, channelDecl,
                                                   typeDecl, context)) {
        waitMatchingDone = true;
      } else if (const clang::FunctionDecl *funcDecl =
                     AnalyzerUtils::findFunctionDefinition(whileStmt,
                                                           context)) {
        const clang::Stmt *body = funcDecl->getBody();

        auto childElm = body->children();
        auto childItr = childElm.begin();
        auto childEnd = childElm.end();
        waitMatchingDone = this->validateFunctionDecl(
            context, CASTmap, childItr, childEnd, childScopeProjectionPtr);
      }
    }
    cpy++;
  }
  itr = cpy;

  if (itr != end) {
    if (childScopeProjectionPtr) {
      return childScopeProjectionPtr->validateFunctionDecl(
          context, CASTmap, itr, end, parentScopeProjectionPtr);
    } else if (this->next) {
      return this->next->validateFunctionDecl(context, CASTmap, itr, end,
                                              parentScopeProjectionPtr);
    } else {
      return waitMatchingDone;
    }
  } else {
    if (waitMatchingDone) {
      parentScopeProjectionPtr = this->next.get();
    } else {
      parentScopeProjectionPtr = this;
    }
    return waitMatchingDone;
  }
};

} // namespace PchorAST