#include "AstVisitor.hpp"
#include "../../pchor/ast/PchorProjection.hpp"

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
  auto indexExpr = expr.getIndex();
  auto baseIndex = expr.getBaseParticipant()->getIndex();
  size_t literal = indexExpr->getLiteral(this->indexIdentifierMap);


  if(literal < baseIndex->getLower() || literal > baseIndex->getUpper()){
    throw std::runtime_error(std::format("Index expression {} evaluated to {}, which is not within the range of [{}, {}].", indexExpr->toString(), literal, baseIndex->getLower(), baseIndex->getUpper()));
  }
  ParticipantKey key{expr.getBaseParticipant()->getName(), literal};

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
        key, std::make_unique<Preceive>(this->currentChannelName,
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

  const std::string identifier = iterExpr->getIdentifierRef();

  if(baseIndex->getUpper() == std::numeric_limits<size_t>::max()) {
    std::println("The case for indeces with no upper bound has not been implemented");
    mappingSuccess = false;


    std::unordered_map<std::string, FullIter> fullIterCase;
    std::unordered_map<std::string, MaxExcludingIter> maxIterCase;
    std::unordered_map<std::string, MinExcludingIter> minIterCase;

    const auto body = expr.getBody();

    bool minAllowed;
    switch(iterExpr->getType()){
      case IterType::FullIter :
        std::println("FullIter Not implemented Yet");
        minAllowed = iterExpr->getMin() == 0;
        fullIterCase = getCasesFull(body, identifier, minAllowed);
        for(const auto& [name, elem]: fullIterCase) {
          std::println("{}:",name);
          elem.forward->print();
          elem.backward->print();
          elem.unevenOverlapAB->print();
        }

        //we need method to construct new equivalence classes
        break;
      case IterType::MaxExIter :
        std::println("MaxIter Not implemented Yet");
        maxIterCase = getCasesMaxEx(body);
        break;
      case IterType::MinExIter :
        std::println("MinIter Not implemented Yet");
        minIterCase = getCasesMinEx(body);
        break;
    }

  }
  else {
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
//Helper Function for Insertion
template <typename ComType>
requires std::derived_from<ComType, AbstractProjection>
void insertFullPattern(FullIter& iter, 
                   const std::shared_ptr<ParticipantExpr>& participant, 
                   const std::string& channelName,
                   const std::string& dataType,
                   const std::string& identifier,
                   bool min_allowed) {
  const auto index = participant->getIndex();
  switch (index->getEquivalenceType(identifier)) {
    case EquivalenceBaseType::forward:
      iter.addForward<ComType>(channelName, dataType, index, min_allowed);
      break;
    case EquivalenceBaseType::backward:
      if(!min_allowed) {
        throw std::runtime_error(std::format("[Proj_visitor] Backwards iteration only allowed if lower bound for index is 0. Found {}", index->toString()));
      }
      iter.addBackward<ComType>(channelName, dataType, index);
      break;
    default:
      throw std::runtime_error(std::format(
        "[Proj_Visitor] The iteration pattern ({}: I) only allows expressions of type (i|n-i). Found {}",
        identifier, index->toString()));
  }
}

/*
This only runs when we have i: I in the iteration !
*/
std::unordered_map<std::string, FullIter> Proj_PchorASTVisitor::getCasesFull(const std::shared_ptr<ExprList>& expr, const std::string& i, bool minAllowed) {
  std::unordered_map<std::string, FullIter> BasePattern{};

  std::println("getCasesFull Beginning");
  for(const auto& com: *expr) {
    if(com->getExprType() != Expr::ComExpr){
       throw std::runtime_error(std::format("Only Com Allowed in unbounded foreach, found {}", com->toString()));
    }
    
    std::shared_ptr<CommunicationExpr> comExpr = std::dynamic_pointer_cast<CommunicationExpr>(com);
  
    std::println("Communication Expressoin successfully extracted");
    const auto sender = comExpr->getSender();
    const auto receiver = comExpr->getReciever();
    const auto channel = comExpr->getChannel();
    const std::string& channelName = channel->getBaseParticipant()->getName();
    const auto dataType = comExpr->getDataType();

    //sender and receiver must be indexed by some type
    if(sender->getIndex()->isExprLiteral() || receiver->getIndex()->isExprLiteral() || channel->getIndex()->isExprLiteral()){
      throw std::runtime_error(std::format("[Proj_Visitor] Equivalence Class projection requires indeces to be expressions. Found {} and {}", sender->toString(), receiver->toString(), channel->toString()));
    }

    //idea, find pattern for types 
    const std::string senderName = sender->getBaseParticipant()->getName();
    if(!BasePattern.contains(senderName)) {
      BasePattern.try_emplace(senderName);
    }
    insertFullPattern<Isend>(BasePattern.at(senderName), sender, channelName, dataType, i, minAllowed);

    const std::string receiverName = receiver->getBaseParticipant()->getName();
    if(!BasePattern.contains(receiverName)) {
      BasePattern.try_emplace(receiverName);
    }
    insertFullPattern<Ireceive>(BasePattern.at(receiverName), receiver, channelName, dataType, i, minAllowed);

  }
    return BasePattern;
}
std::unordered_map<std::string, MaxExcludingIter> Proj_PchorASTVisitor::getCasesMaxEx(const std::shared_ptr<ExprList>& expr) {
    std::println("Exprlist of length {}", expr->size());
    for(const auto& com: *expr) {
    std::println("We attempt to print what we have");
    com->print();
    if(com->getExprType() != Expr::ComExpr){
      throw std::runtime_error(std::format("Only Com Allowed in unbounded foreach, found {}", com->toString()));
    }
  }
  return std::unordered_map<std::string, MaxExcludingIter>();

}
std::unordered_map<std::string, MinExcludingIter> Proj_PchorASTVisitor::getCasesMinEx(const std::shared_ptr<ExprList>& expr) {
    for(const auto& com: *expr) {
    if(com->getExprType() != Expr::ComExpr){
       throw std::runtime_error(std::format("Only Com Allowed in unbounded foreach, found {}", com->toString()));
    }
  }

  return std::unordered_map<std::string, MinExcludingIter>();

}

} // namespace PchorAST