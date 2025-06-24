#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

#include "../pchor/parser/PchorParser.hpp"
#include "./visitors/AstVisitor.hpp"
#include "./visitors/CASTValidator.hpp"
#include "./utils/ContextManager.hpp"

#include <memory>
#include <string>
#include <vector>
#include <iterator>

using namespace clang;

namespace {
class ChoreographyAstConsumer : public ASTConsumer {
public:
  explicit ChoreographyAstConsumer(
      std::shared_ptr<PchorAST::SymbolTable> sTable, bool debug, bool onlyproj)
      : sTable(std::move(sTable)), debug(debug), onlyproj(onlyproj) {}
void HandleTranslationUnit(ASTContext &Context) override {
    llvm::outs() << "\n\nAST has been fully created. CASTMapping and Choreography Projection Commencing!\n";
    try {
      if (!sTable) {
        llvm::outs() << "Error: HandleTranslationUnit received no SymbolTable. Continuing to compilation\n";
        return;
      }

      llvm::outs() << "Symbol table correctly passed to ChoreographyAstConsumer\n";
      auto globalTypePtr = sTable->back();
      if ((*globalTypePtr)->getDeclType() != PchorAST::Decl::Global_Type_Decl) {
        throw std::runtime_error("Final Expression is required to be a Global type expression.");
      }

      if (onlyproj) {
        // Only projection logic
        PchorAST::Proj_PchorASTVisitor Proj_visitor(Context);
        (*globalTypePtr)->accept(Proj_visitor);
        Proj_visitor.printProjections();
        return;
      }

      // Full pipeline
      PchorAST::CAST_PchorASTVisitor CAST_visitor(Context);
      PchorAST::Proj_PchorASTVisitor Proj_visitor(Context);

      for (auto itr = sTable->begin(); itr != sTable->end(); ++itr) {
        if ((*itr)->getDeclType() != PchorAST::Decl::Global_Type_Decl ||
            (std::distance(itr, sTable->end()) == 1 && (*itr)->getDeclType() == PchorAST::Decl::Global_Type_Decl)) {
          (*itr)->accept(CAST_visitor);
        }
      }
      llvm::outs() << "CAST mapping created\n";
      (*globalTypePtr)->accept(Proj_visitor);
      llvm::outs() << "Projection created\n";

      auto CASTMapping = CAST_visitor.getContext();
      auto Projections = Proj_visitor.getContext();

      PchorAST::CASTValidator validator{};
      validator.validateProjection(Context, CASTMapping, Projections);

      if (debug) {
        CAST_visitor.printMappings();
        Proj_visitor.printProjections();
      }

      validator.printValidations();

    } catch (const std::exception &e) {
      llvm::errs() << "Error in CAST Mapping or Choreography Projection: \n"
                   << e.what() << "\n";
                   
    }
}

private:
  std::shared_ptr<PchorAST::SymbolTable> sTable;
  bool debug;
  bool onlyproj;
};

class ChoreographyValidatorFrontendAction : public PluginASTAction {
  std::string corFilePath;
  std::shared_ptr<PchorAST::SymbolTable> sTable;
  bool debug;
  bool onlyproj;

protected:
  std::unique_ptr<ASTConsumer>
  CreateASTConsumer([[maybe_unused]] CompilerInstance &CI,
                    llvm::StringRef) override {
    // Create and return your AST consumer that prints messages.
    return std::make_unique<ChoreographyAstConsumer>(std::move(sTable), debug, onlyproj);
  }

  bool ParseArgs([[maybe_unused]] const CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    // Handle plugin arguments if any.
    debug = false;
    onlyproj = false;
    for (const auto &arg : args) {
      if (arg.find("--cor=") != std::string::npos) {
        corFilePath = arg.substr(arg.find("--cor=") + 6);
        llvm::outs() << "Recieved .cor file path: " << corFilePath << "\n";
      }
      if (arg.find("--debug") != std::string::npos) {
        debug = true;
        llvm::outs() << "Debug flag found. Debug Output will be printed\n";
      }
      if(arg.find("--projection") != std::string::npos) {
        onlyproj = true;
        llvm::outs() << "Projection flag found. Program will only generate local type projections\n";
      }
    }

    if (corFilePath.empty()) {
      llvm::errs()
          << "Error: No .cor file specified. Use --cor=<path_to_file>\n";
      return false;
    }

    try {
      PchorAST::PchorParser parser{corFilePath};
      parser.genTokens();
      if(debug) {
        parser.printTokenList();
      }
      parser.parse();

      if (debug) {
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