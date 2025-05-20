#pragma once

#include <clang/AST/ASTContext.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <llvm/Support/raw_ostream.h>

#include <string>
#include <vector>

namespace PchorAST {
// matcher templates to match for array of NodeType of one element of NodeType
template <typename NodeType>
class MatchCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  explicit MatchCallback(const NodeType *&result, const std::string &bindName)
      : result(result), bindName(bindName) {}

  void run(const clang::ast_matchers::MatchFinder::MatchResult &matchResult)
      override {
    if (const auto *node = matchResult.Nodes.getNodeAs<NodeType>(bindName)) {
      result = node;
    }
  }

private:
  const NodeType *&result; // Single result reference
  std::string bindName;
};

template <typename NodeType>
class MatchTypeUseCallback
    : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  explicit MatchTypeUseCallback(std::vector<const NodeType *> &results,
                                const std::string &bindName)
      : results(results), bindName(bindName) {}

  void run(const clang::ast_matchers::MatchFinder::MatchResult &matchResult)
      override {
    if (const auto *node = matchResult.Nodes.getNodeAs<NodeType>(bindName)) {
      results.push_back(node); // Add the matched node to the results vector
    }
  }

private:
  std::vector<const NodeType *> &results; // Reference to the results vector
  std::string bindName;
};

template <typename NodeType>
class DebugStoreMatchCallback
    : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  DebugStoreMatchCallback(const std::string &bindName, const NodeType *&result)
      : bindName(bindName), result(result) {}

  void run(const clang::ast_matchers::MatchFinder::MatchResult &matchResult)
      override {
    llvm::outs() << "[DebugStoreMatchCallback] Running for: " << bindName
                 << "\n";
    if (const auto *node = matchResult.Nodes.getNodeAs<NodeType>(bindName)) {
      result = node;
      llvm::outs() << "Matched Node (" << bindName << "):\n";
      // node->dump();
    } else {
      llvm::outs() << "No match for bind name: " << bindName << "\n";
    }
  }

private:
  std::string bindName;
  const NodeType *&result;
};

template <typename BindType, typename ResultType>
class CrossTypeMatchCallback
    : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  CrossTypeMatchCallback(const std::string &bindName, const ResultType *&result)
      : bindName(bindName), result(result) {}

  void run(const clang::ast_matchers::MatchFinder::MatchResult &matchResult)
      override {
    if (const auto *node = matchResult.Nodes.getNodeAs<BindType>(bindName)) {
      result = extract(node);
    }
  }

private:
  std::string bindName;
  const ResultType *&result;

  static const ResultType *extract(const BindType *node);
};

template <>
inline const clang::CXXMethodDecl *
CrossTypeMatchCallback<clang::CXXMemberCallExpr, clang::CXXMethodDecl>::extract(
    const clang::CXXMemberCallExpr *node) {
  return node->getMethodDecl();
}

class AnalyzerUtils {
public:
  // print functions for debugging
  static void printDecl(const clang::Decl *decl);
  static void printRecordFields(const clang::Decl *decl);
  static void printDeclChildren(const clang::Decl *decl);

  // matcher functions for different types of AST-types
  static const clang::Decl *findDecl(clang::ASTContext &context,
                                     const std::string &name);

  static const clang::FunctionDecl *
  getFullDecl(const clang::FunctionDecl *funcDecl);

  static std::vector<const clang::FunctionDecl *>
  findDataTypeInClass(clang::ASTContext &context, const clang::Decl *decl,
                      const std::string &typeName);

  static const clang::FieldDecl *
  findMatchingMember(clang::ASTContext &context, const clang::Decl *decl,
                     const std::string &typeName);

  // Validation Functions
  static bool validateSendExpression(const clang::Stmt *opCallExpr,
                                     const clang::Decl *channelDecl,
                                     const clang::Decl *typeDecl,
                                     clang::ASTContext &context);
  // Validation Functions
  static bool validateRecieveExpression(
      const clang::Stmt *whileStmt, const clang::Decl *channelDecl,
      [[maybe_unused]] const clang::Decl *typeDecl, clang::ASTContext &context);

  static const clang::FunctionDecl *
  findFunctionDefinition(const clang::Stmt *possibleFunctionCall,
                         clang::ASTContext &context);
};

} // namespace PchorAST
