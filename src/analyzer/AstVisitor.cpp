#include "AstVisitor.hpp"
#include <memory>
#include <print>

namespace PchorAST {

// Visit Declaration Nodes

void CAST_PchorASTVisitor::visit(const ParticipantASTNode &node) {
  llvm::outs() << "Visiting ParticipantASTNode: " << node.getName() << "\n";

  // Simulate finding a declaration in the C++ AST
  auto *decl = AnalyzerUtils::findDecl(clangContext, node.getName());
  if (decl == nullptr) {
    llvm::outs() << "Declaration for " << node.getName() << " not found.\n";
    mappingSuccess = false;
  } else {
    llvm::outs() << "Found declaration for " << node.getName() << ": "
                 << decl->getDeclKindName() << "\n";
  }
  AnalyzerUtils::analyzeDeclChildren(decl);
  ctx->addMapping(node.getName(), decl);
}

void CAST_PchorASTVisitor::visit([[maybe_unused]] const ChannelASTNode &node) {
  std::println("No Visit Required");
}
void CAST_PchorASTVisitor::visit([[maybe_unused]] const LabelASTNode &node) {
  std::println("Not Implemented Yet");
}
void CAST_PchorASTVisitor::visit(const GlobalTypeASTNode &node) {
  llvm::outs() << "Visiting GlobalASTVisitor" << node.getName() << "\n";
  node.getExprList()->accept(*this);
}
void CAST_PchorASTVisitor::visit([[maybe_unused]] const IndexASTNode &node) {
  std::println("Not Implemented Yet");
}
// Visit Expression Nodes
void CAST_PchorASTVisitor::visit(const CommunicationExpr &expr) {

  auto *dataTypeDecl =
      AnalyzerUtils::findDecl(clangContext, expr.getDataType());

  if (dataTypeDecl == nullptr) {
    llvm::outs() << "Declaration for " << expr.getDataType() << " not found.\n";
    mappingSuccess = false;
  } else {
    llvm::outs() << "Found declaration for " << expr.getDataType() << ": "
                 << dataTypeDecl->getDeclKindName() << "\n";
  }
  ctx->addMapping(expr.getDataType(), dataTypeDecl);
  this->currentDataType = expr.getDataType();

  // visit participants and channel {figure out who owns the collective state}

  // sender
  expr.getSender()->accept(*this);
  // reciever
  expr.getReciever()->accept(*this);

  // channel
  expr.getChannel()->accept(*this);
}
void CAST_PchorASTVisitor::visit(const ExprList &expr) {
  for (auto it = expr.begin(); it != expr.end(); ++it) {
    (*it)->accept(*this); // Read-only access
  }
}
void CAST_PchorASTVisitor::visit(const ParticipantExpr &expr) {
  if (expr.getIndex() == nullptr) {
    std::println("We implement this !");
    auto dataTypeUse = AnalyzerUtils::findDataTypeInClass(
        clangContext,
        ctx->getMapping<const clang::Decl *>(
            expr.getBaseParticipant()->getName()),
        this->currentDataType);
    ctx->addMapping(expr.getBaseParticipant()->getName() +
                        this->currentDataType,
                    dataTypeUse.back());
  } else {
    std::println("indexed expressions not supported yet :(");
  }
}
void CAST_PchorASTVisitor::visit(const ChannelExpr &expr) {
  if (expr.getIndex() == nullptr) {
    // we have singular type without index
    std::println("We implement this !");
  } else {
    std::println("Not Implemented yet");
  }
}
void CAST_PchorASTVisitor::visit([[maybe_unused]] const IndexExpr &expr) {
  std::println("Not Implemented yet");
}
void CAST_PchorASTVisitor::visit([[maybe_unused]] const RecExpr &expr) {
  std::println("Not Implemented yet");
}
void CAST_PchorASTVisitor::visit([[maybe_unused]] const ConExpr &expr) {
  std::println("Not Implemented yet");
}

} // namespace PchorAST