#pragma once
#include "AstVisitor.hpp"
#include <memory>
#include <print>

namespace PchorAST {

    //Visit Declaration Nodes

    void PchorASTVisitor::visit(const ParticipantASTNode& node) {
        llvm::outs() << "Visiting ParticipantASTNode: " << node.getName() << "\n";
    
        // Simulate finding a declaration in the C++ AST
        auto* decl = AnalyzerUtils::findDecl(clangContext, node.getName());
        if (decl == nullptr) {
            llvm::outs() << "Declaration for " << node.getName() << " not found.\n";
        } else {
            llvm::outs() << "Found declaration for " << node.getName() << ": "
                         << decl->getDeclKindName() << "\n";
        }
    
        // Additional test logic
        llvm::outs() << "Test visit function executed successfully.\n";
    }

    void PchorASTVisitor::visit(const ChannelASTNode& node) {
        std::println("Not implemented yet");
    }
    void PchorASTVisitor::visit(const LabelASTNode& node) {
        std::println("Not implemented yet");
    }
    void PchorASTVisitor::visit(const GlobalTypeASTNode& node) {
        std::println("Not Implemented yet");
    }
    void PchorASTVisitor::visit(const IndexASTNode& node)  {
        std::println("Not Implemented yet");
    }

    //Visit Expression Nodes
    void PchorASTVisitor::visit(const CommunicationExpr& expr) {
        std::println("Not Implemented yet");
    }
    void PchorASTVisitor::visit(const ExprList& expr)  {
        std::println("Not Implemented yet");
    }
    void PchorASTVisitor::visit(const ParticipantExpr& expr) {
        std::println("Not Implemented yet");
    }
    void PchorASTVisitor::visit(const ChannelExpr& expr)  {
        std::println("Not Implemented yet");
    }
    void PchorASTVisitor::visit(const IndexExpr& expr)  {
        std::println("Not Implemented yet");
    }
    void PchorASTVisitor::visit(const RecExpr& expr)  {
        std::println("Not Implemented yet");
    }
    void PchorASTVisitor::visit(const ConExpr& expr)  {
        std::println("Not Implemented yet");
    }

} //namespace PchorAST