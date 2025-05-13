#pragma once

#include <clang/AST/Decl.h>
#include "../utils/CASTAnalyzerUtils.hpp"
#include "../utils/ContextManager.hpp"
#include "../../pchor/ast/PchorProjection.hpp"


namespace PchorAST {

class CASTValidator {
public:
  static clang::FunctionDecl* validateFuncDecl(
      std::shared_ptr<CASTMapping> CASTMap,
      const std::vector<std::unique_ptr<PchorAST::AbstractProjection>>& projections,
      const ParticipantKey& participantName);

  static bool validateProjection(std::shared_ptr<CASTMapping>& CASTmap,
                                 std::shared_ptr<PchorProjection>& projectionMap);
                                 
};
} // namespace PchorAST
