#pragma once

#include "../pchor/PchorProjection.hpp"

#include <clang/AST/ASTContext.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <llvm/Support/raw_ostream.h>

#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include <memory>

template <typename NodeType>
class MatchCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  explicit MatchCallback(const NodeType* &result, const std::string &bindName)
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

namespace PchorAST {
// class only providing static functions
class AnalyzerUtils {
public:
  static void analyzeDeclChildren(const clang::Decl *decl) {
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
  static auto createTypeMatcher(const std::string &typeName) {
    return clang::ast_matchers::hasType(
        clang::ast_matchers::namedDecl(clang::ast_matchers::hasName(typeName)));
  }

  static const clang::Decl *findDecl(clang::ASTContext &context,
                                     const std::string &name) {

    auto matcher =
        clang::ast_matchers::namedDecl(clang::ast_matchers::hasName(name))
            .bind("namedDecl");

    const clang::NamedDecl *result = nullptr;

    MatchCallback<clang::NamedDecl> callback(result, "namedDecl");
    clang::ast_matchers::MatchFinder finder;
    finder.addMatcher(matcher, &callback);
    finder.matchAST(context);

    return result;
  }

  static const clang::Stmt *
  findStmt([[maybe_unused]] clang::ASTContext &context,
           [[maybe_unused]] const std::string &name) {
    return nullptr;
  }

  static std::vector<const clang::FunctionDecl *>
  findDataTypeInClass(clang::ASTContext &context, const clang::Decl *decl,
                      const std::string &typeName) {

    // Define the matcher to validate individual child declarations
    auto methodMatcher =
        clang::ast_matchers::cxxMethodDecl(
            clang::ast_matchers::unless(
                clang::ast_matchers::cxxConstructorDecl()),
            clang::ast_matchers::anyOf(
                // Match methods with a parameter of the specified type
                clang::ast_matchers::hasAnyParameter(
                    clang::ast_matchers::hasType(clang::ast_matchers::qualType(
                        clang::ast_matchers::hasDeclaration(
                            clang::ast_matchers::namedDecl(
                                clang::ast_matchers::hasName(typeName)))))),
                // Match methods that use a variable of the specified type in
                // their body
                clang::ast_matchers::hasDescendant(clang::ast_matchers::varDecl(
                    clang::ast_matchers::hasType(clang::ast_matchers::qualType(
                        clang::ast_matchers::hasDeclaration(
                            clang::ast_matchers::namedDecl(
                                clang::ast_matchers::hasName(typeName))))))),
                // Match methods that use an object of the specified type in
                // their body
                clang::ast_matchers::hasDescendant(clang::ast_matchers::expr(
                    clang::ast_matchers::hasType(clang::ast_matchers::qualType(
                        clang::ast_matchers::anyOf(
                            // Match the type directly
                            clang::ast_matchers::hasDeclaration(
                                clang::ast_matchers::namedDecl(
                                    clang::ast_matchers::hasName(typeName))),
                            // Match pointer types with the specified pointee
                            // type
                            clang::ast_matchers::pointerType(
                                clang::ast_matchers::pointee(
                                    clang::ast_matchers::hasDeclaration(
                                        clang::ast_matchers::namedDecl(
                                            clang::ast_matchers::hasName(
                                                typeName)))))))))),
                // Match methods that return the specified type
                clang::ast_matchers::returns(clang::ast_matchers::qualType(
                    clang::ast_matchers::hasDeclaration(
                        clang::ast_matchers::namedDecl(
                            clang::ast_matchers::hasName(typeName)))))))
            .bind("methodDecl");

    std::vector<const clang::FunctionDecl *> results;

    auto ctx = decl->getDeclContext();

    clang::ast_matchers::MatchFinder finder;
    MatchTypeUseCallback<const clang::FunctionDecl> callback(results,
                                                             "methodDecl");
    finder.addMatcher(methodMatcher, &callback);

    for (const auto childDecl : ctx->decls()) {
      // Match the child declaration
      finder.match(*childDecl, context);
    }
    // Collect results
    return results;
  }

  static const clang::FieldDecl* findMatchingMember(clang::ASTContext &context, const clang::Decl *decl, const std::string &typeName) {

    auto memberMatcher = clang::ast_matchers::fieldDecl(
      clang::ast_matchers::hasType(
        clang::ast_matchers::qualType(
          clang::ast_matchers::anyOf(
            clang::ast_matchers::hasDeclaration(
              clang::ast_matchers::namedDecl(clang::ast_matchers::hasName(typeName))            
            ),
            clang::ast_matchers::pointerType(
              clang::ast_matchers::pointee(
                clang::ast_matchers::hasDeclaration(
                  clang::ast_matchers::namedDecl(clang::ast_matchers::hasName(typeName))            
                )
              )
            ),
            clang::ast_matchers::templateSpecializationType(
              clang::ast_matchers::hasAnyTemplateArgument(
                clang::ast_matchers::templateArgument(
                  clang::ast_matchers::refersToType(
                    clang::ast_matchers::qualType(
                      clang::ast_matchers::hasDeclaration(
                        clang::ast_matchers::namedDecl(clang::ast_matchers::hasName(typeName))            
                      )
                    )
                  )
                )
              )
            )
          )
        )
      )
    ).bind("fieldDecl");
    const clang::FieldDecl* field = nullptr;

    auto ctx = decl->getDeclContext();

    clang::ast_matchers::MatchFinder finder;
    MatchCallback<clang::FieldDecl> callback(field, "fieldDecl");
    finder.addMatcher(memberMatcher, &callback);

    for(const auto childDecl: ctx->decls()){
      finder.match(*childDecl, context);
      if(field != nullptr) {
        break;
      }
    }
    return field;
  }

private:
};

enum class ContextType { Decl, Stmt };
struct Context {
  ContextType type;
  std::variant<const clang::Decl *, const clang::Stmt *> value;

  // Delete default constructor
  Context() = delete;

  // Delete copy constructor and copy assignment operator
  Context(const Context &other) = delete;
  Context &operator=(const Context &other) = delete;

  // Move constructor
  Context(Context &&other) noexcept
      : type(other.type), value(std::move(other.value)) {}

  // Move assignment operator
  Context &operator=(Context &&other) noexcept {
    if (this != &other) {
      type = other.type;
      value = std::move(other.value);
    }
    return *this;
  }

  // Constructor for Decl
  explicit Context(const clang::Decl *decl)
      : type(ContextType::Decl), value(decl) {}

  // Constructor for Stmt
  explicit Context(const clang::Stmt *stmt)
      : type(ContextType::Stmt), value(stmt) {}

  // Get the type of the context
  ContextType getType() const { return type; }

  // Get the Decl
  const clang::Decl *getDecl() const {
    return std::get<const clang::Decl *>(value);
  }

  // Get the Stmt
  const clang::Stmt *getStmt() const {
    return std::get<const clang::Stmt *>(value);
  }
};

class CASTMapping {
public:
  CASTMapping() : map() {}

  void addMapping(const std::string &name, const clang::Decl *decl) {
    map.emplace(name, Context(decl));
  }

  void addMapping(const std::string &name, const clang::Stmt *stmt) {
    map.emplace(name, Context(stmt));
  }

  template <typename NodeType> NodeType getMapping(const std::string &name) {
    auto it = map.find(name);
    if (it != map.end()) {
      // Try to get the requested type from the variant
      if (auto *result = std::get_if<NodeType>(&it->second.value)) {
        return *result;
      }
    }
    return nullptr;
  }
  void printMappings() const {
    for(const auto& [key, value]: map) {
      std::println("Mapping for {}", key);
      AnalyzerUtils::analyzeDeclChildren(value.getDecl());
    }
  }

private:
  std::unordered_map<std::string, Context> map;
};

class PchorProjection {
public:
  PchorProjection(): projectionMap() {}

  PchorProjection(const PchorProjection& other) = delete;
  PchorProjection& operator=(const PchorProjection& other) = delete;

  PchorProjection(PchorProjection&& other) noexcept : projectionMap(std::move(other.projectionMap)) {
    other.projectionMap.clear();
  }
  PchorProjection& operator=(PchorProjection&& other) noexcept {
    if (this != &other) {
      projectionMap = std::move(other.projectionMap);
      other.projectionMap.clear();
    }
    return *this;
  }
  void addParticipant(const std::string& participantName) {
    projectionMap.emplace(participantName, std::vector<std::unique_ptr<PchorAST::AbstractProjection>>());
  }

  void addProjection(const std::string& participantName, std::unique_ptr<PchorAST::AbstractProjection> proj) {
    projectionMap[participantName].emplace_back(std::move(proj));
  }
  bool hasProjection(const std::string& participantName) const {
    return projectionMap.contains(participantName);
  }
  void printProjections() const {
    for( const auto& [elem, value] : projectionMap){
      std::print("Projection for participant {}", elem);
      for(const auto& projection: value){
        projection->print();
      }
      std::println(" ");
    }
  }
private:
  std::unordered_map<std::string, std::vector<std::unique_ptr<PchorAST::AbstractProjection>>> projectionMap;
};

} // namespace PchorAST
