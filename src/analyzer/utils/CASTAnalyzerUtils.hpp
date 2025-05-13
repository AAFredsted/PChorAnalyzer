#pragma once

#include <clang/AST/ASTContext.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <llvm/Support/raw_ostream.h>

#include <string>
#include <vector>


namespace PchorAST {
//matcher templates to match for array of NodeType of one element of NodeType
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


class AnalyzerUtils {
public:
  //print functions for debugging
  static void printDecl(const clang::Decl *decl);
  static void printRecordFields(const clang::Decl *decl);
  static void printDeclChildren(const clang::Decl *decl);

  //matcher functions for different types of AST-types
  static const clang::Decl *findDecl(clang::ASTContext &context,
                                     const std::string &name);

  static std::vector<const clang::FunctionDecl *>
  findDataTypeInClass(clang::ASTContext &context, const clang::Decl *decl,
                      const std::string &typeName);

  static const clang::FieldDecl *
  findMatchingMember(clang::ASTContext &context, const clang::Decl *decl,
                     const std::string &typeName);

};


} // namespace PchorAST
