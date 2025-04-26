#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

#include "../pchor/PchorParser.hpp"
#include "AstVisitor.hpp"

#include <memory>
#include <string>
#include <vector>

using namespace clang;

namespace {
class ChoreographyAstConsumer : public ASTConsumer {
public:
  void HandleTranslationUnit(ASTContext &Context) override {
    llvm::outs() << "AST has been fully created. Hello from HandleTranslationUnit!\n";

    // Create the PchorASTVisitor
    PchorAST::PchorASTVisitor visitor(Context);

    // Traverse the choreography AST (example)
  }
private: 
  std::unique_ptr<PchorAST::SymbolTable> sTable;
  
};

class ChoreographyValidatorFrontendAction : public PluginASTAction {
  std::string corFilePath;

protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 llvm::StringRef) override {
    // Create and return your AST consumer that prints messages.
    return std::make_unique<ChoreographyAstConsumer>();
  }

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    // Handle plugin arguments if any.

    for (const auto &arg : args) {
      if (arg.rfind("--cor=", 0) == 0) {
        corFilePath = arg.substr(6);
        llvm::outs() << "Recieved .cor file path: " << corFilePath;
      }
    }

    if (corFilePath.empty()) {
      llvm::errs()
          << "Error: No .cor file specified. Use --cor=<path_to_file>\n";
      return false;
    }

    try {
      PchorAST::PchorParser parser{corFilePath};
      parser.parse();
      
    } catch (const std::exception &e) {
      llvm::errs() << "Error processing .cor-file:" << e.what() << "\n";
      return false;
    }

    return true;
  }

  // Override to let clang run the backend code generation after our plugin.
  PluginASTAction::ActionType getActionType() override {
    return AddAfterMainAction;
  }
};
} // namespace
// Register the plugin with Clang
static FrontendPluginRegistry::Add<ChoreographyValidatorFrontendAction>
    X("PchorAnalyzer",
      "A simple plugin that prints a message after the AST is created");