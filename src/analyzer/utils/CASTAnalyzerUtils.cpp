#include "CASTAnalyzerUtils.hpp"

namespace PchorAST {
void AnalyzerUtils::printDecl(const clang::Decl *decl) {
  if (!decl) {
    llvm::outs() << "Decl not found\n";
    return;
  }

  // Check if the declaration is a class
  llvm::outs() << "Decl of type " << decl->getDeclKindName() << "\n";

  if (const auto *classDecl = llvm::dyn_cast<clang::CXXRecordDecl>(decl)) {
    llvm::outs() << "Class Decl: " << classDecl->getNameAsString() << "\n";
  }
  // Check if the declaration is a field
  else if (const auto *fieldDecl = llvm::dyn_cast<clang::FieldDecl>(decl)) {
    llvm::outs() << "Field Decl: " << fieldDecl->getNameAsString() << "\n";
  }
  // Check if the declaration is a function
  else if (const auto *funcDecl = llvm::dyn_cast<clang::FunctionDecl>(decl)) {
    // Skip constructors and destructors
    if (llvm::isa<clang::CXXConstructorDecl>(funcDecl)) {
      llvm::outs() << "Constructor Decl: " << funcDecl->getNameAsString()
                   << "\n";
    } else if (llvm::isa<clang::CXXDestructorDecl>(funcDecl)) {
      llvm::outs() << "Destructor Decl: " << funcDecl->getNameAsString()
                   << "\n";
    } else {
      llvm::outs() << "Function Decl: " << funcDecl->getNameAsString() << "\n";
    }
  }
  // Handle other types of declarations
  else {
    llvm::outs() << "Other Decl Type: " << decl->getDeclKindName() << "\n";
  }
}

void AnalyzerUtils::printRecordFields(const clang::Decl *decl) {
  if (decl) {
    llvm::outs() << "Mapped Node is: " << decl->getDeclKindName() << "\n";
    if (auto ctx = decl->getDeclContext()) {
      for (const auto childDecl : ctx->decls()) {
        if (childDecl->getDeclKindName() == std::string("Field")) {
          printDecl(childDecl);
          printDeclChildren(childDecl);
        }
      }
    } else {
      llvm::outs() << "Decl has no children\n";
    }
  } else {
    llvm::outs() << "Decl not found\n";
  }
}

void AnalyzerUtils::printDeclChildren(const clang::Decl *decl) {
  if (decl) {
    llvm::outs() << "Mapped Node is: " << decl->getDeclKindName() << "\n";
    if (auto ctx = decl->getDeclContext()) {
      for (const auto childDecl : ctx->decls()) {

        if (childDecl) {
          llvm::outs() << "Child Decl: " << childDecl->getDeclKindName()
                       << "\n";
        }
      }
    } else {
      llvm::outs() << "Decl has no children\n";
    }
  } else {
    llvm::outs() << "Decl not found\n";
  }
}

const clang::FunctionDecl *
AnalyzerUtils::getFullDecl(clang::ASTContext &context,
                           const clang::FunctionDecl *funcDecl) {
  const clang::FunctionDecl *fullDecl = nullptr;
  if (funcDecl->isThisDeclarationADefinition()) {
    fullDecl = funcDecl;
  } else {
    fullDecl = funcDecl->getDefinition();
  }
  if (!fullDecl) {
    throw std::runtime_error(
        std::format("Definition for declared function {} not found.\n",
                    funcDecl->getNameAsString()));
  }
  return fullDecl;
}

const clang::FunctionDecl *
AnalyzerUtils::findFunctionDefinition(const clang::Stmt *possibleFunctionCall,
                                      clang::ASTContext &context) {
  if (!possibleFunctionCall) {
    llvm::errs() << "Invalid input to findFunctionDefinition.\n";
    return nullptr;
  }
  llvm::outs() << "We dump what we have\n";
  possibleFunctionCall->dump();
  auto memberCallMatcher = clang::ast_matchers::stmt(
    clang::ast_matchers::anyOf(
      clang::ast_matchers::cxxMemberCallExpr().bind("memberCall"),
      clang::ast_matchers::hasDescendant(
          clang::ast_matchers::cxxMemberCallExpr().bind("memberCall")))
    );

  const clang::CXXMethodDecl *method = nullptr;
  clang::ast_matchers::MatchFinder finder;
  CrossTypeMatchCallback<clang::CXXMemberCallExpr, clang::CXXMethodDecl>
      callback("memberCall", method);
  finder.addMatcher(clang::ast_matchers::stmt(memberCallMatcher), &callback);
  finder.match(*possibleFunctionCall, context);

  if (method) {
    if (const auto *def = method->getDefinition()) {
      return AnalyzerUtils::getFullDecl(context, def);
    }
    return nullptr;
  }

  llvm::errs()
      << "Could not find member function definition from call expression.\n";
  return nullptr;
}

const clang::Decl *AnalyzerUtils::findDecl(clang::ASTContext &context,
                                           const std::string &name) {

  auto matcher =
      clang::ast_matchers::cxxRecordDecl(
          clang::ast_matchers::hasName(name),
          clang::ast_matchers::has(clang::ast_matchers::cxxConstructorDecl()))
          .bind("classDecl");

  const clang::CXXRecordDecl *result = nullptr;

  MatchCallback<clang::CXXRecordDecl> callback(result, "classDecl");
  clang::ast_matchers::MatchFinder finder;
  finder.addMatcher(matcher, &callback);
  finder.matchAST(context);

  return result;
}

std::vector<const clang::FunctionDecl *>
AnalyzerUtils::findDataTypeInClass(clang::ASTContext &context,
                                   const clang::Decl *decl,
                                   const std::string &typeName) {

  // Define a reusable matcher for the type
  auto declMatcher = clang::ast_matchers::qualType(clang::ast_matchers::anyOf(
      // Match the type directly
      clang::ast_matchers::hasDeclaration(clang::ast_matchers::namedDecl(
          clang::ast_matchers::hasName(typeName))),
      // Match pointer types with the specified pointee type
      clang::ast_matchers::pointerType(clang::ast_matchers::pointee(
          clang::ast_matchers::hasDeclaration(clang::ast_matchers::namedDecl(
              clang::ast_matchers::hasName(typeName))))),
      // Match reference types with the specified pointee type
      clang::ast_matchers::referenceType(clang::ast_matchers::pointee(
          clang::ast_matchers::hasDeclaration(clang::ast_matchers::namedDecl(
              clang::ast_matchers::hasName(typeName)))))));

  // Define the matcher to validate individual child declarations
  auto methodMatcher =
      clang::ast_matchers::cxxMethodDecl(
          clang::ast_matchers::unless(
              clang::ast_matchers::cxxConstructorDecl()),
          clang::ast_matchers::anyOf(
              // Match methods with a parameter of the specified type
              clang::ast_matchers::hasAnyParameter(
                  clang::ast_matchers::hasType(declMatcher)),
              // Match methods that use a variable of the specified type in
              // their body
              clang::ast_matchers::hasDescendant(clang::ast_matchers::varDecl(
                  clang::ast_matchers::hasType(declMatcher))),
              // Match methods that use an object of the specified type in their
              // body
              clang::ast_matchers::hasDescendant(clang::ast_matchers::expr(
                  clang::ast_matchers::hasType(declMatcher))),
              // Match methods that return the specified type
              clang::ast_matchers::returns(declMatcher)))
          .bind("methodDecl");

  std::vector<const clang::FunctionDecl *> results;

  clang::ast_matchers::MatchFinder finder;
  MatchTypeUseCallback<const clang::FunctionDecl> callback(results,
                                                           "methodDecl");
  finder.addMatcher(methodMatcher, &callback);

  const clang::CXXRecordDecl *record =
      llvm::dyn_cast<clang::CXXRecordDecl>(decl);

  if (!record) {
    throw std::runtime_error(
        std::format("Recieved Declaration is not a class. Recieved {}",
                    decl->getDeclKindName()));
  }

  for (const auto *method : record->methods()) {
    const clang::FunctionDecl *fullDecl = getFullDecl(context, method);
    finder.match(*fullDecl, context);
  }
  // Collect results
  return results;
}

const clang::FieldDecl *
AnalyzerUtils::findMatchingMember(clang::ASTContext &context,
                                  const clang::Decl *decl,
                                  const std::string &typeName) {

  auto directDeclaration = clang::ast_matchers::hasDeclaration(
      clang::ast_matchers::namedDecl(clang::ast_matchers::hasName(typeName)));
  auto pointerDeclaration = clang::ast_matchers::pointerType(
      clang::ast_matchers::pointee(directDeclaration));
  auto templateDeclaration = clang::ast_matchers::hasDeclaration(
      clang::ast_matchers::classTemplateSpecializationDecl(
          clang::ast_matchers::hasTemplateArgument(
              0, clang::ast_matchers::templateArgument(
                     clang::ast_matchers::refersToType(
                         clang::ast_matchers::qualType(directDeclaration))))));
  auto templatePointerDeclaration = clang::ast_matchers::pointerType(
      clang::ast_matchers::pointee(templateDeclaration));
  auto fieldMatcher =
      clang::ast_matchers::fieldDecl(
          clang::ast_matchers::hasType(clang::ast_matchers::qualType(
              clang::ast_matchers::anyOf(directDeclaration, pointerDeclaration,
                                         templateDeclaration,
                                         templatePointerDeclaration))))
          .bind("fieldDecl");
  const clang::FieldDecl *field = nullptr;

  const clang::CXXRecordDecl *record =
      llvm::dyn_cast<clang::CXXRecordDecl>(decl);
  if (!record) {
    throw std::runtime_error(
        std::format("Recieved Declaration is not a class. Recieved {}",
                    decl->getDeclKindName()));
  }

  clang::ast_matchers::MatchFinder finder;
  MatchCallback<clang::FieldDecl> callback(field, "fieldDecl");
  finder.addMatcher(fieldMatcher, &callback);

  for (const auto *matchField : record->fields()) {
    finder.match(*matchField, context);
    if (field != nullptr) {
      break;
    }
  }
  return field;
}

bool AnalyzerUtils::validateSendExpression(const clang::Stmt *opCallExpr,
                                           const clang::Decl *channelDecl,
                                           const clang::Decl *typeDecl,
                                           clang::ASTContext &context) {
  if (!opCallExpr || !channelDecl || !typeDecl) {
    llvm::errs() << "Invalid input to validateSendExpression.\n";
    return false;
  }
  llvm::outs() << "we try to run validateSendExpr\n";

  std::string typeName = "";
  if (const auto *named = llvm::dyn_cast<clang::NamedDecl>(typeDecl)) {
    typeName = named->getNameAsString();
  } else {
    llvm::errs() << "Failed to get name of TypeDecl. This should not happen\n";
  }

  auto rhsTypeMatcher = clang::ast_matchers::anyOf(
      clang::ast_matchers::cxxConstructExpr(
          clang::ast_matchers::hasType(clang::ast_matchers::recordDecl(
              clang::ast_matchers::hasName(typeName))))
          .bind("sendRHS"),
      clang::ast_matchers::cxxTemporaryObjectExpr(
          clang::ast_matchers::hasType(clang::ast_matchers::recordDecl(
              clang::ast_matchers::hasName(typeName))))
          .bind("sendRHS"),
      clang::ast_matchers::expr(
          clang::ast_matchers::hasType(clang::ast_matchers::recordDecl(
              clang::ast_matchers::hasName(typeName))))
          .bind("sendRHS"));

  auto directMemberExpr =
      clang::ast_matchers::memberExpr(
          clang::ast_matchers::member(clang::ast_matchers::fieldDecl(
              clang::ast_matchers::equalsNode(channelDecl))))
          .bind("sendLHS");

  auto nestedMemberExpr = clang::ast_matchers::hasDescendant(directMemberExpr);

  auto lhsTypeMatcher =
      clang::ast_matchers::anyOf(directMemberExpr, nestedMemberExpr);
  auto operatorCallExprMatcher =
      clang::ast_matchers::cxxOperatorCallExpr(
          clang::ast_matchers::hasOverloadedOperatorName("="),
          clang::ast_matchers::hasArgument(0, lhsTypeMatcher),
          clang::ast_matchers::hasArgument(
              1,
              clang::ast_matchers::ignoringImplicit(
                  clang::ast_matchers::ignoringParenImpCasts(rhsTypeMatcher))));

  auto binaryOperatorCallMatcher = 
    clang::ast_matchers::binaryOperator(
      clang::ast_matchers::hasOperatorName("=")//,
      //clang::ast_matchers::hasLHS(lhsTypeMatcher),
      //clang::ast_matchers::hasRHS(rhsTypeMatcher)
    );

  auto expressionMatcher = clang::ast_matchers::expr(
  clang::ast_matchers::anyOf(
      operatorCallExprMatcher,
      binaryOperatorCallMatcher,
      clang::ast_matchers::hasDescendant(operatorCallExprMatcher),
      clang::ast_matchers::hasDescendant(binaryOperatorCallMatcher)
      )
    ).bind("sendAssignment");

  // Use MatchFinder to debug the matcher
  const clang::Expr *matched = nullptr;
  clang::ast_matchers::MatchFinder finder;
  finder.addMatcher(expressionMatcher,
                    new DebugStoreMatchCallback<clang::Expr>(
                        "sendAssignment", matched));

  // Run the matcher on the AST node
  finder.match(*opCallExpr, context);
  if (matched) {
    llvm::outs() << "We matched for this: " << opCallExpr->getStmtClassName()
                 << "\n";
  } else {
    llvm::outs() << "We failed to match for this: "
                 << opCallExpr->getStmtClassName() << "\n";
  }

  return matched != nullptr;
}

bool AnalyzerUtils::validateRecieveExpression(
    const clang::Stmt *whileStmt, const clang::Decl *channelDecl,
    [[maybe_unused]] const clang::Decl *typeDecl, clang::ASTContext &context) {

  if (!whileStmt || !channelDecl) {
    llvm::errs() << "Invalid input to validateSendExpression.\n";
    return false;
  }

  auto whileMatcher =
      clang::ast_matchers::whileStmt(
          clang::ast_matchers::hasCondition(
              clang::ast_matchers::forEachDescendant(
                  clang::ast_matchers::memberExpr(
                      clang::ast_matchers::member(
                          clang::ast_matchers::fieldDecl(
                              clang::ast_matchers::equalsNode(channelDecl))))
                      .bind("recvChannel"))))
          .bind("recvWhile");

  const clang::WhileStmt *matched = nullptr;

  clang::ast_matchers::MatchFinder finder;
  finder.addMatcher(whileMatcher, new DebugStoreMatchCallback<clang::WhileStmt>(
                                      "recvWhile", matched));
  finder.match(*whileStmt, context);
  if (matched) {
    llvm::outs() << matched->getStmtClassName() << "\n";
  }

  return matched != nullptr;
}

} // namespace PchorAST