#pragma once

#include <cstdint>
#include <memory>
#include <print>

#include "../../pchor/ast/PchorAST.hpp"
#include "../../pchor/ast/PchorProjection.hpp"
#include "../utils/CASTAnalyzerUtils.hpp"
#include "../utils/ContextManager.hpp"

namespace PchorAST {

class AbstractPchorASTVisitor {
public:
  AbstractPchorASTVisitor(clang::ASTContext &clangContext)
      : clangContext(clangContext) {}
  virtual ~AbstractPchorASTVisitor() = default;

  // Visiting Declarations
  virtual void visit(const ParticipantASTNode &node) = 0;
  virtual void visit(const ChannelASTNode &node) = 0;
  virtual void visit(const LabelASTNode &node) = 0;
  virtual void visit(const GlobalTypeASTNode &node) = 0;
  virtual void visit(const IndexASTNode &node) = 0;

  // Visiting Expressions
  virtual void visit(const CommunicationExpr &expr) = 0;
  virtual void visit(const ExprList &expr) = 0;
  virtual void visit(const ParticipantExpr &expr) = 0;
  virtual void visit(const ChannelExpr &expr) = 0;
  virtual void visit(const IndexExpr &expr) = 0;
  virtual void visit(const RecExpr &expr) = 0;
  virtual void visit(const ConExpr &expr) = 0;

protected:
  clang::ASTContext &clangContext;
};

class CAST_PchorASTVisitor : public AbstractPchorASTVisitor {

public:
  CAST_PchorASTVisitor(clang::ASTContext &clangContext)
      : AbstractPchorASTVisitor(clangContext),
        ctx(std::make_shared<PchorAST::CASTMapping>()), currentDataType(""),
        senderIdentifier(""), recieverIdentifier(""), mappingSuccess(true) {}

  ~CAST_PchorASTVisitor() = default;

  // Visiting Declarations
  void visit(const ParticipantASTNode &node) override;
  void visit(const ChannelASTNode &node) override;
  void visit(const LabelASTNode &node) override;
  void visit(const GlobalTypeASTNode &node) override;
  void visit(const IndexASTNode &node) override;

  // Visiting Expressions
  void visit(const CommunicationExpr &expr) override;
  void visit(const ExprList &expr) override;
  void visit(const ParticipantExpr &expr) override;
  void visit(const ChannelExpr &expr) override;
  void visit(const IndexExpr &expr) override;
  void visit(const RecExpr &expr) override;
  void visit(const ConExpr &expr) override;

  std::shared_ptr<PchorAST::CASTMapping> getContext() { return ctx; }

  void printMappings() { ctx->printMappings(); }

private:
  std::shared_ptr<PchorAST::CASTMapping> ctx;
  std::string currentDataType;
  std::string senderIdentifier;
  std::string recieverIdentifier;
  bool mappingSuccess;
};

class Proj_PchorASTVisitor : public AbstractPchorASTVisitor {
public:
  Proj_PchorASTVisitor(clang::ASTContext &clangContext)
      : AbstractPchorASTVisitor(clangContext),
        ctx(std::make_shared<PchorProjection>()), currentDataType(""),
        currentChannelName(""), channelIndex(), isSender(true),
        mappingSuccess(true) {}

  ~Proj_PchorASTVisitor() = default;

  // Visiting Declarations
  void visit(const ParticipantASTNode &node) override;
  void visit(const ChannelASTNode &node) override;
  void visit(const LabelASTNode &node) override;
  void visit(const GlobalTypeASTNode &node) override;
  void visit(const IndexASTNode &node) override;

  // Visiting Expressions
  void visit(const CommunicationExpr &expr) override;
  void visit(const ExprList &expr) override;
  void visit(const ParticipantExpr &expr) override;
  void visit(const ChannelExpr &expr) override;
  void visit(const IndexExpr &expr) override;
  void visit(const RecExpr &expr) override;
  void visit(const ConExpr &expr) override;

  std::shared_ptr<PchorProjection> getContext() { return ctx; }

  void printProjections() const { ctx->printProjections(); }

private:
  std::shared_ptr<PchorProjection> ctx;
  std::string currentDataType;
  std::string currentChannelName;
  size_t channelIndex;
  bool isSender;
  bool mappingSuccess;
};

} // namespace PchorAST