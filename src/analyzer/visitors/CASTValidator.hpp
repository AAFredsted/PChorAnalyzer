#pragma once

#include "../../pchor/ast/PchorProjection.hpp"
#include "../utils/CASTAnalyzerUtils.hpp"
#include "../utils/ContextManager.hpp"
#include <clang/AST/Decl.h>

namespace PchorAST {

class CASTValidator {
public:

    explicit CASTValidator(): successfullValidations(), failedValidations() {}

    void printValidations();

    clang::FunctionDecl *validateFuncDecl(
        std::shared_ptr<CASTMapping> CASTMap,
        const std::vector<std::unique_ptr<PchorAST::AbstractProjection>>
            &projections,
        const ParticipantKey &participantName);

    bool
    validateProjection(clang::ASTContext &Context,
                        std::shared_ptr<CASTMapping> &CASTmap,
                        std::shared_ptr<PchorProjection> &projectionMap);
private:
    std::unordered_map<std::string, std::vector<std::string>> successfullValidations;
    std::unordered_map<std::string, std::vector<std::string>> failedValidations;

};
} // namespace PchorAST
