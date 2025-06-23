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

    const auto* record = CASTmap->getMapping<const clang::Decl*>(participantName.name);
    const auto* castRecord = llvm::dyn_cast<clang::CXXRecordDecl>(record);

    if(!castRecord) {
        throw std::runtime_error(
          std::format("Participant {} did not map to a record in the CASTmapping. Instead, mapped to:  {}\n",  participantName.name, record->getDeclKindName()));
    }

    auto methods = std::vector<clang::CXXMethodDecl*>{};

    for(auto* method: castRecord->methods() ){
      if(!method->isUserProvided() || llvm::isa<clang::CXXConstructorDecl>(method) || llvm::isa<clang::CXXDestructorDecl>(method) || method->isOverloadedOperator()){
        continue;
      }
      methods.push_back(method);
    }

    //reverse ordering to minimize runtime
    for(auto ritr = methods.rbegin(); ritr != methods.rend(); ++ritr){
      const auto* fullDecl = AnalyzerUtils::getFullDecl(*ritr);
      std::string funcName{(*ritr)->getNameAsString()};

      if(!fullDecl || !fullDecl->hasBody()){
        throw std::runtime_error(
            std::format("Function {} has no body\n", funcName));
      }
      if (!failedValidations.contains(funcName) &&
          !successfullValidations.contains(funcName)) {
        failedValidations[funcName] = std::vector<std::string>{};
        successfullValidations[funcName] = std::vector<std::string>{};
      }

      const clang::Stmt *body = fullDecl->getBody();

      auto elm = body->children();
      auto itr = elm.begin();
      auto end = elm.end();

      auto projectionNode = projections.begin();
      AbstractProjection* nestedFunctionNext = nullptr;
      
      bool successFullMapping = projectionNode->validateFunctionDecl(Context, CASTmap, itr, end, nestedFunctionNext);

      if (itr != end) {
        llvm::errs() << std::format("Warning: Not all statements in {} consumed "
                                    "by projections. Stopped at: {}\n",
                                    funcName, itr->getStmtClassName());
      }

      if (successFullMapping) {
        //to begin with, we only need one sucessfull mapping for each
        successfullValidations[funcName].push_back(participantName.toString());
        break;
      } else {
        failedValidations[funcName].push_back(participantName.toString());
      }

    }
  }

  return true;
}
} // namespace PchorAST