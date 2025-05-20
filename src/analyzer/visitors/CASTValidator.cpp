#include "CASTValidator.hpp"

namespace PchorAST {

void CASTValidator::printValidations() {
  std::println("\n\nSuccessfull Validations:\n-------------------");
  for (const auto &[key, values] : successfullValidations) {
    std::print("{}: ", key);
    for (const auto &str : values) {
      std::print("{} ", str);
    }
    std::println("");
  }
  std::println("\n\nFailed Validations:\n-------------------");
  for (const auto &[key, values] : failedValidations) {
    std::print("{}: ", key);
    for (const auto &str : values) {
      std::print("{} ", str);
    }
    std::println("");
  }
}
clang::FunctionDecl *CASTValidator::validateFuncDecl(
    std::shared_ptr<CASTMapping> CASTMap,
    const ProjectionList &projections,
    const ParticipantKey &participantName) {
  clang::FunctionDecl *funcDecl =
      nullptr;

  for (const auto &projection : projections) {

    const clang::Decl *currentDecl = CASTMap->getMapping<const clang::Decl *>(
        std::format("{}{}", participantName.name, projection.getTypeName()));

    const auto *currentFuncDecl =
        llvm::dyn_cast<clang::FunctionDecl>(currentDecl);
    if (!currentFuncDecl) {
      throw std::runtime_error(std::format(
          "Mapping for participant '{}' does not point to a valid FunctionDecl",
          participantName.toString()));
    }
    if (funcDecl == nullptr) {
      funcDecl = const_cast<clang::FunctionDecl *>(
          currentFuncDecl);
    } else if (funcDecl != currentFuncDecl) {
      throw std::runtime_error(std::format(
          "Projections for participant '{}' must map to the same function. "
          "Expected '{}', but found '{}'.",
          participantName.toString(), funcDecl->getNameAsString(),
          currentFuncDecl->getNameAsString()));
    }
  }
  return funcDecl;
}

bool CASTValidator::validateProjection(
    clang::ASTContext &Context, std::shared_ptr<CASTMapping> &CASTmap,
    std::shared_ptr<PchorProjection> &projectionMap) {
  for (const auto &[participantName, projections] : *projectionMap) {

    clang::FunctionDecl *funcDecl =
        validateFuncDecl(CASTmap, projections, participantName);
    std::string funcName = funcDecl->getNameAsString();

    if (!funcDecl->hasBody()) {
      throw std::runtime_error(
          std::format("Function {} has no body\n", funcName));
    }

    if (!failedValidations.contains(funcName) &&
        !successfullValidations.contains(funcName)) {
      failedValidations[funcName] = std::vector<std::string>{};
      successfullValidations[funcName] = std::vector<std::string>{};
    }

    const clang::Stmt *body = funcDecl->getBody();

    std::println("{}", body->getStmtClassName());

    auto elm = body->children();
    auto itr = elm.begin();
    auto end = elm.end();

    std::println("validating {} for {}", funcName, participantName.toString());

    auto projectionNode = projections.begin();
    AbstractProjection* nestedFunctionNext = nullptr;
    
    bool successFullMapping = projectionNode->validateFunctionDecl(Context, CASTmap, itr, end, nestedFunctionNext);

    /*
    for (auto &projection : projections) {
      if (!projection.validateFunctionDecl(Context, CASTmap, itr, end)) {
        successFullMapping = false;
        break;
      }
    }
    */

    if (successFullMapping) {
      successfullValidations[funcName].push_back(participantName.toString());
    } else {
      failedValidations[funcName].push_back(participantName.toString());
    }

    if (itr != end) {
      llvm::errs() << std::format("Warning: Not all statements in {} consumed "
                                  "by projections. Stopped at: {}",
                                  funcName, itr->getStmtClassName());
    }
  }

  return true;
}
} // namespace PchorAST