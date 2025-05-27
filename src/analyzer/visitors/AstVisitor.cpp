#include "AstVisitor.hpp"

namespace PchorAST {

// Visit Declaration Nodes

void CAST_PchorASTVisitor::visit(const ParticipantASTNode &node) {
  // Simulate finding a declaration in the C++ AST
  auto *decl = AnalyzerUtils::findDecl(clangContext, node.getName());
  if (decl == nullptr) {
    mappingSuccess = false;
    throw std::runtime_error(
        std::format("Declaration for {} not found\n", node.getName()));
  }
  ctx->addMapping(node.getName(), decl);
}

void CAST_PchorASTVisitor::visit([[maybe_unused]] const ChannelASTNode &node) {}
void CAST_PchorASTVisitor::visit([[maybe_unused]] const LabelASTNode &node) {
  std::println("Not Implemented Yet");
}
void CAST_PchorASTVisitor::visit(const GlobalTypeASTNode &node) {
  node.getExprList()->accept(*this);
}
void CAST_PchorASTVisitor::visit([[maybe_unused]] const IndexASTNode &node) {}
// Visit Expression Nodes
void CAST_PchorASTVisitor::visit(const CommunicationExpr &expr) {

  auto *dataTypeDecl =
      AnalyzerUtils::findDecl(clangContext, expr.getDataType());

  if (dataTypeDecl == nullptr) {
    mappingSuccess = false;
    throw std::runtime_error(
        std::format("Declaration for {} not found\n", expr.getDataType()));
  }
  ctx->addMapping(expr.getDataType(), dataTypeDecl);
  this->currentDataType = expr.getDataType();

  // visit participants and channel {figure out who owns the collective state}
  auto sender = expr.getSender();
  auto reciever = expr.getReciever();

  this->senderIdentifier = sender->getBaseParticipant()->getName();
  this->recieverIdentifier = reciever->getBaseParticipant()->getName();

  //unused
  // sender
  //expr.getSender()->accept(*this);
  // reciever
  //expr.getReciever()->accept(*this);

  // channel
  expr.getChannel()->accept(*this);
}
void CAST_PchorASTVisitor::visit(const ExprList &expr) {
  for (auto it = expr.begin(); it != expr.end(); ++it) {
    (*it)->accept(*this); // Read-only access
  }
}
void CAST_PchorASTVisitor::visit([[maybe_unused]] const ParticipantExpr &expr) {}

void CAST_PchorASTVisitor::visit(const ChannelExpr &expr) {
  if (expr.getIndex()->isExprLiteral()) {
    auto sender = ctx->getMapping<const clang::Decl *>(this->senderIdentifier);
    auto reciever =
        ctx->getMapping<const clang::Decl *>(this->recieverIdentifier);

    // Try Find Reciever Member
    // Try Find Sender Member

    auto recieverUse = AnalyzerUtils::findMatchingMember(clangContext, reciever,
                                                         this->currentDataType);
    if (recieverUse) {
      ctx->addMapping(expr.getBaseParticipant()->getName(), recieverUse);
    } else {
      auto senderUse = AnalyzerUtils::findMatchingMember(clangContext, sender,
                                                         this->currentDataType);
      if (senderUse) {
        ctx->addMapping(expr.getBaseParticipant()->getName(), senderUse);
      } else {
        mappingSuccess = false;
        throw std::runtime_error(
            std::format("No matching member found for data type '{}' in sender "
                        "'{}' or receiver '{}'",
                        this->currentDataType, this->senderIdentifier,
                        this->recieverIdentifier));
      }
    }
  } else {
    std::println("non-literal Indexes not implemented yet");
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

void CAST_PchorASTVisitor::visit([[maybe_unused]] const IterExpr &expr) {
  std::println("Not Implemented yet");
}

void CAST_PchorASTVisitor::visit(const ForEachExpr &expr) {
  /*
    CAST_Mapping is independant of indeces, so we just skip it and go for the expression list contained within
  */
  expr.getBody()->accept(*this);

}


// visitor functions for ProjectionVisitor
//  Visiting Declarations
void Proj_PchorASTVisitor::visit(
    [[maybe_unused]] const ParticipantASTNode &node) {
  std::println("No visit Required: projection only performed on final global "
               "type declaration");
}
void Proj_PchorASTVisitor::visit([[maybe_unused]] const ChannelASTNode &node) {
  std::println("No visit Required: projection only performed on final global "
               "type declaration");
}
void Proj_PchorASTVisitor::visit([[maybe_unused]] const LabelASTNode &node) {
  std::println("No visit Required: projection only performed on final global "
               "type declaration");
}
void Proj_PchorASTVisitor::visit(const GlobalTypeASTNode &node) {
  node.getExprList()->accept(*this);
}

void Proj_PchorASTVisitor::visit([[maybe_unused]] const IndexASTNode &node) {
  std::println("No visit Required: projection only performed on final global "
               "type declaration");
}

// Visiting Expressions
void Proj_PchorASTVisitor::visit(const CommunicationExpr &expr) {

  // store datatype for projection generation
  this->currentDataType = expr.getDataType();

  // set currentChannelName through this !
  expr.getChannel()->accept(*this);

  this->isSender = true;
  // project sender
  expr.getSender()->accept(*this);
  // check if sender exists
  // check if index is defined, otherwise, throw error

  this->isSender = false;
  // project reciever
  // check if reciever exists
  expr.getReciever()->accept(*this);
  // check if index is defined, otherwise throw error

  // done
}
void Proj_PchorASTVisitor::visit(const ExprList &expr) {
  // visit each com expression
  for (auto it = expr.begin(); it != expr.end(); ++it) {
    (*it)->accept(*this); // Read-only access
  }
}
void Proj_PchorASTVisitor::visit(const ParticipantExpr &expr) {
  ParticipantKey key{expr.getBaseParticipant()->getName(),
                     expr.getIndex()->getLiteral(this->indexIdentifierMap)};

  if (!this->ctx->hasProjection(key)) {
    this->ctx->addParticipant(key);
  }
  if (this->isSender) {
    this->ctx->addProjection(key,
                              std::make_unique<Psend>(this->currentChannelName,
                                                      this->currentDataType,
                                                      this->channelIndex));
  } else {
    this->ctx->addProjection(
        key, std::make_unique<Precieve>(this->currentChannelName,
                                        this->currentDataType,
                                        this->channelIndex));
  }
}
void Proj_PchorASTVisitor::visit(const ChannelExpr &expr) {
  this->currentChannelName = expr.getBaseParticipant()->getName();
  expr.getIndex()->accept(*this);
}

void Proj_PchorASTVisitor::visit(const IndexExpr &expr) {
  this->channelIndex = expr.getLiteral(this->indexIdentifierMap);
}
void Proj_PchorASTVisitor::visit([[maybe_unused]] const RecExpr &expr) {
  mappingSuccess = false;
  throw std::runtime_error("Recursive Expressions not implemented");
}
void Proj_PchorASTVisitor::visit([[maybe_unused]] const ConExpr &expr) {
  mappingSuccess = false;
  throw std::runtime_error("Continuation Expressions not implemented");
}

void Proj_PchorASTVisitor::visit([[maybe_unused]] const IterExpr &expr) {
  mappingSuccess = false;
  throw std::runtime_error("Continuation Expressions not implemented");
}
void Proj_PchorASTVisitor::visit(const ForEachExpr &expr) {

  /*
    We provide code for two approaches:
    1. iterate over indeces from min to max defined in IterExpr and set that index in visitor, then call visit on child
    2. if end index is abstract, break expression down into equivalence classes if feasable (requires inductive proof for this property!)
  */

  const auto iterExpr = expr.getIter();

  const auto baseIndex = iterExpr->getBaseIndex();


  if(baseIndex->getUpper() == std::numeric_limits<size_t>::max()) {
    std::println("The case for indeces with no upper bound has not been implemented");
    mappingSuccess = false;
  }
  else {
    const std::string identifier = iterExpr->getIdentifierRef();
    size_t el = iterExpr->getMin();
    size_t max = iterExpr->getMax();
    //due to previous check, we know that max is not max, so we can check for one above !
    //to stay on the safe side however, we project in the following way
    while(true) {

      this->indexIdentifierMap.insert_or_assign(identifier, el);
      expr.getBody()->accept(*this);
      
      if(el == max){
        break;
      }
      el++;
    }
    this->indexIdentifierMap.erase(identifier);

  }
  //set index context for this iteration, then run it
}
} // namespace PchorAST