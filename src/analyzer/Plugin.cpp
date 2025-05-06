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
  explicit ChoreographyAstConsumer(
      std::shared_ptr<PchorAST::SymbolTable> sTable, bool debug)
      : sTable(std::move(sTable)), debug(debug) {}
  void HandleTranslationUnit(ASTContext &Context) override {
    llvm::outs() << "\n\nAST has been fully created. CASTMapping and Choreopgrahy "
                    "Projection Commencing!\n";

    PchorAST::CAST_PchorASTVisitor CAST_visitor(Context);
    PchorAST::Proj_PchorASTVisitor Proj_visitor(Context);
    try {
      // Create the PchorASTVisitor
      if (sTable) {
        llvm::outs()
            << "Symbol table correctly passed to ChoreographyAstConsumer\n";
        for (auto itr = sTable->begin(); itr != sTable->end(); ++itr) {
          (*itr)->accept(CAST_visitor);
        }
        llvm::outs() << "CAST mapping created\n";
        auto globalTypePtr = sTable->back();
        if ((*globalTypePtr)->getDeclType() !=
            PchorAST::Decl::Global_Type_Decl) {
          throw std::runtime_error(
              "Final Expression is required to be a Global type expression.");
        }
        (*globalTypePtr)->accept(Proj_visitor);
        llvm::outs() << "Projection created\n";
      }

      if (debug) {
        CAST_visitor.printMappings();
        Proj_visitor.printProjections();
      }
    } catch (const std::exception &e) {
      llvm::errs() << "Error in CAST Mapping or Choreography Projection: \n"
                   << e.what() << "\n";
    }

    // Traverse the choreography AST (example)
  }

private:
  std::shared_ptr<PchorAST::SymbolTable> sTable;
  bool debug;
};

class ChoreographyValidatorFrontendAction : public PluginASTAction {
  std::string corFilePath;
  std::shared_ptr<PchorAST::SymbolTable> sTable;
  bool debug;

protected:
  std::unique_ptr<ASTConsumer>
  CreateASTConsumer([[maybe_unused]] CompilerInstance &CI,
                    llvm::StringRef) override {
    // Create and return your AST consumer that prints messages.
    return std::make_unique<ChoreographyAstConsumer>(std::move(sTable), debug);
  }

  bool ParseArgs([[maybe_unused]] const CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    // Handle plugin arguments if any.
    debug = false;
    for (const auto &arg : args) {
      if (arg.find("--cor=") != std::string::npos) {
        corFilePath = arg.substr(arg.find("--cor=") + 6);
        llvm::outs() << "Recieved .cor file path: " << corFilePath << "\n";
      }
      if (arg.find("--debug") != std::string::npos) {
        debug = true;
        llvm::outs() << "Debug flag found. Debug Output will be printed\n";
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

      if (debug) {
        parser.printTokenList();
        parser.printAST();
      }

      sTable = parser.getChorAST();

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
    X("PchorAnalyzer", "A Plugin that validates your c++ code against a "
                       "choreography written in a Domain-Specific Parametrized "
                       "Multiparty Asynchronous Session Type Language");