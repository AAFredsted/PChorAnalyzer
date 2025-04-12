#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/Support/raw_ostream.h"

#include <memory>
#include <vector>
#include <string>

using namespace clang;

namespace {
class HelloWorldAstConsumer : public ASTConsumer {
public:
    void HandleTranslationUnit(ASTContext &Context) override {
        // This is called after the AST for the entire translation unit is created
        llvm::outs() << "AST has been fully created. Hello from HandleTranslationUnit!\n";
        llvm::outs().flush();
    }
};

class HelloWorldFrontendAction : public PluginASTAction {
protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) override {
        // Create and return your AST consumer that prints messages.
        return std::make_unique<HelloWorldAstConsumer>();
    }

    bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &args) override {
        // Handle plugin arguments if any.
        return true;
    }

    // Override to let clang run the backend code generation after our plugin.
    PluginASTAction::ActionType getActionType() override {
        return AddAfterMainAction;
    }
};
} //namespace
// Register the plugin with Clang
static FrontendPluginRegistry::Add<HelloWorldFrontendAction>
    X("hello", "A simple plugin that prints a message after the AST is created");