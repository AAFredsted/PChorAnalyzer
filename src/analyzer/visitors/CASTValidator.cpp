#include "CASTValidator.hpp"

namespace PchorAST {
clang::FunctionDecl *CASTValidator::validateFuncDecl(
    std::shared_ptr<CASTMapping> CASTMap,
    const std::vector<std::unique_ptr<PchorAST::AbstractProjection>>
        &projections,
    const ParticipantKey &participantName) {
  clang::FunctionDecl *funcDecl =
      nullptr; // To store the consistent function declaration

  for (const auto &projection : projections) {
    // Retrieve the Decl for the current projection
    const clang::Decl *currentDecl = CASTMap->getMapping<const clang::Decl *>(
        std::format("{}{}", participantName.name, projection->getTypeName()));

    // Check if the Decl is a FunctionDecl
    const auto *currentFuncDecl =
        llvm::dyn_cast<clang::FunctionDecl>(currentDecl);
    if (!currentFuncDecl) {
      throw std::runtime_error(std::format(
          "Mapping for participant '{}' does not point to a valid FunctionDecl",
          participantName.toString()));
    }

    // Validate consistency of the FunctionDecl within this projection array
    if (funcDecl == nullptr) {
      funcDecl = const_cast<clang::FunctionDecl *>(
          currentFuncDecl); // Set the first FunctionDecl
    } else if (funcDecl != currentFuncDecl) {
      throw std::runtime_error(std::format(
          "Projections for participant '{}' must map to the same function. "
          "Expected '{}', but found '{}'.",
          participantName.toString(), funcDecl->getNameAsString(),
          currentFuncDecl->getNameAsString()));
    }
  }
  // Return the consistent FunctionDecl or nullptr if no projections exist
  return funcDecl;
}

bool CASTValidator::validateProjection(
    clang::ASTContext &Context, std::shared_ptr<CASTMapping> &CASTmap,
    std::shared_ptr<PchorProjection> &projectionMap) {
  for (const auto &[participantName, projections] : *projectionMap) {

    clang::FunctionDecl *funcDecl =
        validateFuncDecl(CASTmap, projections, participantName);
    if (!funcDecl->hasBody()) {
      throw std::runtime_error(std::format("Function {} has no body\n",
                                           funcDecl->getNameAsString()));
    }

    const clang::Stmt *body = funcDecl->getBody();

    std::println("{}", body->getStmtClassName());

    auto elm = body->children();
    auto itr = elm.begin();
    auto end = elm.end();
    for (auto &projection : projections) {
      projection->validateFunctionDecl(Context, CASTmap, itr, end);
    }
    if (itr != end) {
      throw std::runtime_error(std::format(
          "Not all statements in {} consumed by projections. Stopped at: {}",
          funcDecl->getNameAsString(), itr->getStmtClassName()));
    }
  }

  return true;
}
} // namespace PchorAST