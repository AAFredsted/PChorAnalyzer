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

  const std::string& identifier = iterExpr->getIdentifierRef();
  const std::string& indexName = baseIndex->getName();

  if(baseIndex->getUpper() == std::numeric_limits<size_t>::max()) {
    std::unordered_map<std::string, FullIter> fullIterCase;
    std::unordered_map<std::string, MaxExcludingIter> maxIterCase;
    std::unordered_map<std::string, MinExcludingIter> minIterCase;

    const auto body = expr.getBody();
    
    switch(iterExpr->getType()){
      case IterType::FullIter :
        fullIterCase = getCasesFull(body, identifier);
        for(const auto& [name, elem]: fullIterCase) {
          std::println("{}:",name);
          elem.forward->print();
          elem.backward->print();
          elem.unevenOverlapAB->print();
        }
        addEquivalenceClassesFull(fullIterCase, identifier, indexName);
        break;
      case IterType::MaxExIter :
        maxIterCase = getCasesMaxEx(body, identifier);        
        for(const auto& [name, elem]: maxIterCase) {
          std::println("{}:",name);
          std::print("forward: ");
          elem.forward->print();
          std::print("forwardPlus: ");
          elem.forwardPlus->print();
          std::print("backwardMinus: ");
          elem.backwardMinus->print();
          std::print("backward: ");
          elem.backward->print();          
          std::print("EvenAD: ");
          elem.evenOverlapAD->print();
          std::print("EvenBC: ");
          elem.evenOverlapBC->print();
          std::print("UnevenAB: ");
          elem.unevenOverlapAB->print();
          std::print("EvenCD: ");
          elem.unevenOverlapCD->print();
        }
        addEquivalenceClassesMaxEx(maxIterCase, identifier, indexName);
        break;
      case IterType::MinExIter :
        minIterCase = getCasesMinEx(body, identifier);
        for(const auto& [name, elem]: minIterCase) {
          std::println("{}:",name);
          std::print("forwardMinus: ");
          elem.forwardMinus->print();
          std::print("forward: ");
          elem.forward->print();      
          std::print("backward: ");
          elem.backward->print();
          std::print("backwardPlus: ");
          elem.backwardPlus->print();    
          std::print("EvenAD: ");
          elem.evenOverlapAD->print();
          std::print("EvenBC: ");
          elem.evenOverlapBC->print();
          std::print("UnevenAB: ");
          elem.unevenOverlapAB->print();
          std::print("EvenCD: ");
          elem.unevenOverlapCD->print();
          //missing method here
          addEquivalenceClassesMinEx(minIterCase, identifier, indexName);
        }
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
//Helper Functions for Insertion
template <typename ComType>
requires std::derived_from<ComType, AbstractProjection>
void insertFullPattern(FullIter& iter, 
                   const std::shared_ptr<ParticipantExpr>& participant, 
                   const std::string& channelName,
                   const std::string& dataType,
                   const std::string& identifier
                  ) {

  /*
  Simple case setup
  if evaluated to i, insert Comtype(Isend|Ireceive) into a and a+b
  if evaluated to n-i+l, insert Comtype(Isend|Ireceive) into b and a+b
  */
  const auto index = participant->getIndex();
  switch (index->getEquivalenceBaseType(identifier)) {
    case EquivalenceBaseType::forward:
      iter.addForward<ComType>(channelName, dataType, index);
      break;
    case EquivalenceBaseType::backward:
      iter.addBackward<ComType>(channelName, dataType, index);
      break;
    default:
      throw std::runtime_error(std::format(
        "[Proj_Visitor] The iteration pattern ({}: I) only allows expressions of type (i|n-i). Found {}",
        identifier, index->toString()));
  }
}

template <typename ComType>
requires std::derived_from<ComType, AbstractProjection>
void insertMaxExPattern(MaxExcludingIter& iter, 
                   const std::shared_ptr<ParticipantExpr>& participant, 
                   const std::string& channelName,
                   const std::string& dataType,
                   const std::string& identifier
                  ) {
  /*
    Inserting a communication expression (Isend or Ireceive) into MaxExcludingIter
    is defined for the following cases:
      case 1: i maps to forward, which is a, evenAD and unevenAB
      case 2: n-i+l maps to backward, which is b, evenBC and unevenAB
      case 3: i+1 maps to forwardPlus, which is c, evenBC and unevenCD
      case 4: n-i+l-1 maps to backwardMinus, which is d, evenAD and unevenCD

      for all other cases, this function is undefined and throws an error

  */
  const auto index = participant->getIndex();
  switch(index->getEquivalenceBaseType(identifier)) {
    case EquivalenceBaseType::forward :
      iter.addForward<ComType>(channelName, dataType, index);
      break;
    case EquivalenceBaseType::backward :
      iter.addBackward<ComType>(channelName, dataType, index);
      break;
    case EquivalenceBaseType::forwardPlus :
      iter.addForwardPlus<ComType>(channelName, dataType, index);
      break;
    case EquivalenceBaseType::backwardMinus :
      iter.addBackwardMinus<ComType>(channelName, dataType, index);
      break;
    default:
      throw std::runtime_error(std::format(
        "[Proj_Visitor] The iteration pattern ({} < max(I)) only allows expressions of type (i|n-i+l|i+1|n-i+l-1). Found {}",
        identifier, index->toString()));
  }
}


template <typename ComType>
requires std::derived_from<ComType, AbstractProjection>
void insertMinExPattern(MinExcludingIter& iter, 
                   const std::shared_ptr<ParticipantExpr>& participant, 
                   const std::string& channelName,
                   const std::string& dataType,
                   const std::string& identifier
                  ) {
  /*
    Inserting a communication expression (Isend or Ireceive) into MinExcludingIter
    is defined for the following cases:
      case 1: i maps to forward, which is a, evenAD and unevenAB
      case 2: n-i+l maps to backward, which is b, evenBC and unevenAB
      case 3: i-1 maps to forwardPlus, which is c, evenBC and unevenCD
      case 4: n-i+l+1 maps to backwardPlus, which is d, evenAD and unevenCD

      for all other cases, this function is undefined and throws an error

  */
  
  const auto index = participant->getIndex();
  switch(index->getEquivalenceBaseType(identifier)) {
    case EquivalenceBaseType::forward :
      iter.addForward<ComType>(channelName, dataType, index);
      break;
    case EquivalenceBaseType::backward :
      iter.addBackward<ComType>(channelName, dataType, index);
      break;
    case EquivalenceBaseType::forwardMinus :
      iter.addForwardMinus<ComType>(channelName, dataType, index);
      break;
    case EquivalenceBaseType::backwardPlus :
      iter.addBackwardPlus<ComType>(channelName, dataType, index);
      break;
    default:
      throw std::runtime_error(std::format(
        "[Proj_Visitor] The iteration pattern ({} < max(I)) only allows expressions of type (i|n-i+l|i+1|n-i+l-1). Found {}",
        identifier, index->toString()));
  }
}
/*
This only runs when we have i: I in the iteration !
*/
std::unordered_map<std::string, FullIter> Proj_PchorASTVisitor::getCasesFull(const std::shared_ptr<ExprList>& expr, const std::string& i) const {
  /*
    For i:I = [l..n] and K = |I| and k=roundup(K/2)
    FullIter provides 3 local types representing the cases
    a: i
    b: n-i+l
    a+b: i = n-i+l which happens when K !| 2 and i = k 

    We use a symbol table to store one such pattern for each unique participant group
  */
  std::unordered_map<std::string, FullIter> BasePattern{};

  //forloop to go over each expression in ExprList
  for(const auto& com: *expr) {
    //for now, we only allow communication expressions
    if(com->getExprType() != Expr::ComExpr){
       throw std::runtime_error(std::format("Only Com Allowed in unbounded foreach, found {}", com->toString()));
    }
    //convert pointer to descendent in class hierachy ExprPchorASTNode->CommunicationExpr
    std::shared_ptr<CommunicationExpr> comExpr = std::dynamic_pointer_cast<CommunicationExpr>(com);
  
    //We add Isend and Ireceive for sender and receiver
    const auto sender = comExpr->getSender();
    const auto receiver = comExpr->getReciever();
    //To do thiw, we need the exprList, the sender/reciever, the channelname, the datatype and the identifier in the loop (to ensure coherence with loop)
    const auto channel = comExpr->getChannel();
    const std::string& channelName = channel->getBaseParticipant()->getName();
    const auto dataType = comExpr->getDataType();

    //We do not allow for index expressions only containing literals in sender, receiver or channel
    if(sender->getIndex()->isExprLiteral() || receiver->getIndex()->isExprLiteral() || channel->getIndex()->isExprLiteral()){
      throw std::runtime_error(std::format("[Proj_Visitor] Equivalence Class projection requires indeces to be expressions. Found {} and {}", sender->toString(), receiver->toString(), channel->toString()));
    }

    //we get sendername for identifying pattern, adding it if it does not exist
    const std::string senderName = sender->getBaseParticipant()->getName();
    if(!BasePattern.contains(senderName)) {
      BasePattern.try_emplace(senderName);
    }
    insertFullPattern<Isend>(BasePattern.at(senderName), sender, channelName, dataType, i);

    const std::string receiverName = receiver->getBaseParticipant()->getName();
    if(!BasePattern.contains(receiverName)) {
      BasePattern.try_emplace(receiverName);
    }
    insertFullPattern<Ireceive>(BasePattern.at(receiverName), receiver, channelName, dataType, i);

  }
    return BasePattern;
}


std::unordered_map<std::string, MaxExcludingIter> Proj_PchorASTVisitor::getCasesMaxEx(const std::shared_ptr<ExprList>& expr, const std::string& i) const {
  /*
    For i < max(I) = [l..n] and K = |I| and k=rounddown(K/2)
    maxEx provides 7 local types representing the cases
    a: i (forward)
    b: n-i+l (backward)
    c: i+1 (forwardPlus)
    d: n-i+l-1 (backwardMinus)
    a+d: i = n-i+l-1 when K|2 and i = k+l (evenOverlapAD)
    b+c: n-i+l = i+1 when K|2 i = k+l (evenOverlapBC)
    c+d: i+1=n-i+l-1 when K!|2 and i = k+l (unevenOverlapCD)
    a+b: i = n-i+l-1 when K!|2 and i=k+l (unevenOverlapAB)

    We use a symbol table to store one such pattern for each unique participant group
  */

  std::unordered_map<std::string, MaxExcludingIter> BasePattern{};

  for(const auto& com: *expr) {
    //only allow ComExpr
    if(com->getExprType() != Expr::ComExpr){
      throw std::runtime_error(std::format("Only Com Allowed in unbounded foreach, found {}", com->toString()));
    }

  
    std::shared_ptr<CommunicationExpr> comExpr = std::dynamic_pointer_cast<CommunicationExpr>(com);

    const auto sender = comExpr->getSender();
    const auto receiver = comExpr->getReciever();

    const auto channel = comExpr->getChannel();
    const std::string& channelName = channel->getBaseParticipant()->getName();
    const auto dataType = comExpr->getDataType();



    //only allow index
    if(sender->getIndex()->isExprLiteral() || receiver->getIndex()->isExprLiteral() || channel->getIndex()->isExprLiteral()){
      throw std::runtime_error(std::format("[Proj_Visitor] Equivalence Class projection requires indeces to be expressions. Found {} and {}", sender->toString(), receiver->toString(), channel->toString()));
    }

    //idea, find pattern for types 
    const std::string senderName = sender->getBaseParticipant()->getName();
    if(!BasePattern.contains(senderName)) {
      BasePattern.try_emplace(senderName);
    }
    //identifies what basetype it belongs to based on senders index :)
    insertMaxExPattern<Isend>(BasePattern.at(senderName), sender, channelName, dataType, i);

    const std::string receiverName = receiver->getBaseParticipant()->getName();
    if(!BasePattern.contains(receiverName)) {
      BasePattern.try_emplace(receiverName);
    }
    insertMaxExPattern<Ireceive>(BasePattern.at(receiverName), receiver, channelName, dataType, i);

  }
  return BasePattern;

}
std::unordered_map<std::string, MinExcludingIter> Proj_PchorASTVisitor::getCasesMinEx(const std::shared_ptr<ExprList>& expr, const std::string& i) const {
  /*
    For i > min(I) = [l..n] and K = |I| and k=roundup(K/2)
    inEx provides 7 local types representing the cases
    a: i (forward)
    b: n-i+l (backward)
    c: i-1 (forwardMinus)
    d: n-i+l+1 (backwardPlus)
    a+d: i = n-i+l-1 when K|2 and i = k+l+1 (evenOverlapAD)
    b+c: n-i+l = i+1 when K|2 i = k+l+1 (evenOverlapBC)
    c+d: i+1=n-i+l-1 when K!|2 and i = k+l+1 (unevenOverlapCD)
    a+b: i = n-i+l-1 when K!|2 and i=k+l+1 (unevenOverlapAB)

    We use a symbol table to store one such pattern for each unique participant group
  */

  std::unordered_map<std::string, MinExcludingIter> BasePattern{};

  for(const auto& com: *expr) {
    if(com->getExprType() != Expr::ComExpr){
      throw std::runtime_error(std::format("Only Com Allowed in unbounded foreach, found {}", com->toString()));
    }

    std::shared_ptr<CommunicationExpr> comExpr = std::dynamic_pointer_cast<CommunicationExpr>(com);

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
    insertMinExPattern<Isend>(BasePattern.at(senderName), sender, channelName, dataType, i);

    const std::string receiverName = receiver->getBaseParticipant()->getName();
    if(!BasePattern.contains(receiverName)) {
      BasePattern.try_emplace(receiverName);
    }
    insertMinExPattern<Ireceive>(BasePattern.at(receiverName), receiver, channelName, dataType, i);

  }
  return BasePattern;
}
void Proj_PchorASTVisitor::addEquivalenceClassesFull(std::unordered_map<std::string, FullIter>& baseCases, const std::string& identifier, const std::string& indexName) {
  /*
    for every FullIter, we run the following assesment:
    if(backward.empty()) we only need to care about forward case
    
    case: a

    else we also need to care about b and a+b, where the following pattern appears

    for i: I = [l,n] and K=|I| and k=rounddown(K/2), the following patterns emerge for all n and l<n:
    a.b : if i<=k for K|2 and K!|2
    a+b : if i=k+1 for K!|2
    b.a : if i>k for K|2 and if i>k+1 for K!|2

    We turn the above into equivalence classes and add them to our key system

    In total, three distinct patterns are possible, against which must be checked for all.
  */

  for(const auto& [name, bases] : baseCases) {



      //a.b : if i<=k for K|2 and K!|2
      ParticipantKey key1{name, std::format("{} < n/2+1 : {}", identifier, indexName)};
      if(!this->ctx->hasProjection(key1)) {
          ctx->addParticipant(key1);
      }
      std::shared_ptr<ProjectionList> ab = std::make_shared<ProjectionList>();
      ab->appendCloneBack(bases.forward);
      ab->appendCloneBack(bases.backward);

      //add to other relevant types here

      ctx->appendCloneProjection(key1, ab);

      //a+b : if i=k+1 for K!|2

      ParticipantKey key2{name, std::format("{} > n/2+1 : {}", identifier, indexName)};
      if(!this->ctx->hasProjection(key2)) {
          ctx->addParticipant(key2);
      }

      std::shared_ptr<ProjectionList> apb = std::make_shared<ProjectionList>();
      apb->appendCloneBack(bases.unevenOverlapAB);

      //add to other relevant types here

      ctx->appendCloneProjection(key2, apb);

      // b.a : if i>k for K|2 and if i>k+1 for K!|2

      ParticipantKey key3{name, std::format("{} = n/2+1 : {}", identifier, indexName)};
      if(!this->ctx->hasProjection(key3)) {
          ctx->addParticipant(key3);
      }
      std::shared_ptr<ProjectionList> ba = std::make_shared<ProjectionList>();
      ba->appendCloneBack(bases.backward);
      ba->appendCloneBack(bases.forward);

      //add to other relevant types here

      ctx->appendCloneProjection(key3, ba);

    }
  }

void Proj_PchorASTVisitor::addEquivalenceClassesMaxEx(std::unordered_map<std::string, MaxExcludingIter>& baseCases, const std::string& identifier, const std::string& indexName) {
  /*
    for every MaxExcludingIter, we run the following assesment:
    if(backward.empty() and backwardMinus.empty()) we only need to care about forward and forwardPlus case
    This logic has not been added, but will be at some future date

    a for i = l
    c.a for 1<i<n
    c for i = n
    

    else we also need to care about  a, b, c, d, and all overlaps (a+b, c+d | a+d, b+c)

    for i: I = [l,n] and K=|I| and k=rounddown(K/2), the following patterns emerge for all n and l<n:

    a.d : if i = l for K|2 and K!|2
    c.a.d.b : if l < i < k for K|2 and K!|2
    c+d.a+b : if i = k for K!|2
    c.a+d.b : if i=k for K|2
    d.c+b.a : if i=k+1 for K|2
    d.b.c.a : if n > i > k+1 for K|2 and i>k for K!|2
    b.c : if i = n for K|2 and K!|2
    We turn the above into equivalence classes and add them to our key system

    In total, seven distinct patterns are possible, against which must be checked for all.
  */
  for(const auto& [name, bases]: baseCases) {
    
    
    //case 1: a.d : if i = l for K|2 and K!|2
    ParticipantKey key1{name, std::format("{1} = min({0}): {0}", indexName, identifier)};
    if(!this->ctx->hasProjection(key1)) {
        ctx->addParticipant(key1);
    }

    std::shared_ptr<ProjectionList> ad = std::make_shared<ProjectionList>();
    ad->appendCloneBack(bases.forward);
    ad->appendCloneBack(bases.backwardMinus);

    //add to other relevant types here

    ctx->appendCloneProjection(key1, ad);

    //case 2: c.a.d.b : if l < i < k for K|2 and K!|2
    ParticipantKey key2{name, std::format("min({0}) < {1} < max({0})/2: {0}", indexName, identifier)};
    if(!this->ctx->hasProjection(key2)) {
        ctx->addParticipant(key2);
    }


    std::shared_ptr<ProjectionList> cadb = std::make_shared<ProjectionList>();
    cadb->appendCloneBack(bases.forwardPlus);
    cadb->appendCloneBack(bases.forward);
    cadb->appendCloneBack(bases.backwardMinus);
    cadb->appendCloneBack(bases.backward);

    //add to other relevant types here

    ctx->appendCloneProjection(key2, cadb);

    //case 3: c+d.a+b : if i = k for K!|2

    ParticipantKey key3{name, std::format("{1} = max({0})/2: {0} even", indexName, identifier)};
    if(!this->ctx->hasProjection(key3)) {
        ctx->addParticipant(key3);
    }

    std::shared_ptr<ProjectionList> cpdapb = std::make_shared<ProjectionList>();
    cpdapb->appendCloneBack(bases.unevenOverlapCD);
    cpdapb->appendCloneBack(bases.unevenOverlapAB);

    //add to other relevant types here

    ctx->appendCloneProjection(key3, cpdapb);
    

    //case 4:c.a+d.b : if i=k for K|2

    ParticipantKey key4{name, std::format("{1} = max({0})/2+1: {0} odd", indexName, identifier)};
    if(!this->ctx->hasProjection(key4)) {
        ctx->addParticipant(key4);
    }


    std::shared_ptr<ProjectionList> capdb = std::make_shared<ProjectionList>();
    capdb->appendCloneBack(bases.forwardPlus);
    capdb->appendCloneBack(bases.evenOverlapAD);
    capdb->appendCloneBack(bases.backward);

    //add to other relevant types here
    ctx->appendCloneProjection(key4, capdb);

    //case 5: d.b+c.a : if i=k+1 for K|2
    ParticipantKey key5{name, std::format("{1} = max({0})/2+1: {0} even", indexName, identifier)};
    if(!this->ctx->hasProjection(key5)) {
        ctx->addParticipant(key5);
    }

    std::shared_ptr<ProjectionList> dbpca = std::make_shared<ProjectionList>();
    dbpca->appendCloneBack(bases.backwardMinus);
    dbpca->appendCloneBack(bases.evenOverlapBC);
    dbpca->appendCloneBack(bases.forward);
    //add to other relevant types here
    ctx->appendCloneProjection(key5, dbpca);

    //case 6: d.b.c.a : if i > k+1 for K|2 and i>k for K!|2

    ParticipantKey key6{name, std::format(" max({0}) > {1} > max({0})/2+1: {0}", indexName, identifier)};
    if(!this->ctx->hasProjection(key6)) {
          ctx->addParticipant(key6);
    }


    std::shared_ptr<ProjectionList> dbca = std::make_shared<ProjectionList>();
    dbca->appendCloneBack(bases.backwardMinus);
    dbca->appendCloneBack(bases.backward);
    dbca->appendCloneBack(bases.forwardPlus);
    dbca->appendCloneBack(bases.forward);
    //add to other relevant types here
    ctx->appendCloneProjection(key6, dbca);

    //case7: b.c : if i = n for K|2 and K!|2

    ParticipantKey key7{name, std::format("{1} = max({0}): {0}", indexName, identifier)};
    if(!this->ctx->hasProjection(key7)) {
          ctx->addParticipant(key7);
    }


    std::shared_ptr<ProjectionList> bc = std::make_shared<ProjectionList>();
    bc->appendCloneBack(bases.backward);
    bc->appendCloneBack(bases.forwardPlus);

    //add to other relevant types here

    ctx->appendCloneProjection(key7, bc);
  }
}

void Proj_PchorASTVisitor::addEquivalenceClassesMinEx(std::unordered_map<std::string, MinExcludingIter>& baseCases, const std::string& identifier, const std::string& indexName) {
  /*
    for every MinExcludingIter, we run the following assesment:
    if(backward.empty() and backwardPlus.empty()) we only need to care about forward and forwardPlus case
    This logic has not been added, but will be at some future date

    c for i = l
    a.c for 1<i<n
    a for i = n
    

    else we also need to care about  a, b, c, d, and all overlaps (a+b, c+d | a+d, b+c)

    for i: I = [l,n] and K=|I| and k=rounddown(K/2), the following patterns emerge for all n and l<n:

    c.b : if i = l for K|2 and K!|2
    a.c.b.d: if l < i <= k for K|2 and K!|2
    a+b.c+d : if i = k+1 for K!|2
    a.b+c.d : if i=k+1 for K|2
    b.a+d.c : if i=k+2 for K|2
    b.d.a.c : if n > i > k+2 for K|2 and i>k+1 for K!|2
    d.a : if i = n for K|2 and K!|2
    We turn the above into equivalence classes and add them to our key system

    In total, seven distinct patterns are possible, against which must be checked for all.
  */
  for(const auto& [name, bases]: baseCases) {
    
    
    //case 1: c.b : if i = l for K|2 and K!|2
    ParticipantKey key1{name, std::format("{1} = min({0}): {0}", indexName, identifier)};
    if(!this->ctx->hasProjection(key1)) {
        ctx->addParticipant(key1);
    }

    std::shared_ptr<ProjectionList> cb = std::make_shared<ProjectionList>();
    cb->appendCloneBack(bases.forwardMinus);
    cb->appendCloneBack(bases.backward);

    //add to other relevant types here

    ctx->appendCloneProjection(key1, cb);

    //case 2: a.c.b.d: if l < i <= k for K|2 and K!|2
    ParticipantKey key2{name, std::format("min({0}) < {1} < max({0})/2: {0}", indexName, identifier)};
    if(!this->ctx->hasProjection(key2)) {
        ctx->addParticipant(key2);
    }


    std::shared_ptr<ProjectionList> acbd = std::make_shared<ProjectionList>();
    acbd->appendCloneBack(bases.forward);
    acbd->appendCloneBack(bases.forwardMinus);
    acbd->appendCloneBack(bases.backward);
    acbd->appendCloneBack(bases.backwardPlus);

    //add to other relevant types here

    ctx->appendCloneProjection(key2, acbd);

    //case 3: a+b.c+d : if i = k+1 for K!|2

    ParticipantKey key3{name, std::format("{1} = max({0})/2: {0} even", indexName, identifier)};
    if(!this->ctx->hasProjection(key3)) {
        ctx->addParticipant(key3);
    }

    std::shared_ptr<ProjectionList> apbcpd = std::make_shared<ProjectionList>();
    apbcpd->appendCloneBack(bases.unevenOverlapAB);
    apbcpd->appendCloneBack(bases.unevenOverlapCD);

    //add to other relevant types here

    ctx->appendCloneProjection(key3, apbcpd);
    

    //case 4: a.b+c.d : if i=k+1 for K|2
    ParticipantKey key4{name, std::format("{1} = max({0})/2+1: {0} odd", indexName, identifier)};
    if(!this->ctx->hasProjection(key4)) {
        ctx->addParticipant(key4);
    }


    std::shared_ptr<ProjectionList> abpcd = std::make_shared<ProjectionList>();
    abpcd->appendCloneBack(bases.forward);
    abpcd->appendCloneBack(bases.evenOverlapBC);
    abpcd->appendCloneBack(bases.backwardPlus);

    //add to other relevant types here
    ctx->appendCloneProjection(key4, abpcd);

    //case 5: b.a+d.c : if i=k+2 for K|2
    ParticipantKey key5{name, std::format("{1} = max({0})/2+1: {0} even", indexName, identifier)};
    if(!this->ctx->hasProjection(key5)) {
        ctx->addParticipant(key5);
    }

    std::shared_ptr<ProjectionList> bapdc = std::make_shared<ProjectionList>();
    bapdc->appendCloneBack(bases.backward);
    bapdc->appendCloneBack(bases.evenOverlapAD);
    bapdc->appendCloneBack(bases.forwardMinus);
    //add to other relevant types here
    ctx->appendCloneProjection(key5, bapdc);

    //case 6: b.d.a.c : if n > i > k+2 for K|2 and i>k+1 for K!|2

    ParticipantKey key6{name, std::format(" max({0}) > {1} > max({0})/2+1: {0}", indexName, identifier)};
    if(!this->ctx->hasProjection(key6)) {
          ctx->addParticipant(key6);
    }


    std::shared_ptr<ProjectionList> bdac = std::make_shared<ProjectionList>();
    bdac->appendCloneBack(bases.backward);
    bdac->appendCloneBack(bases.backwardPlus);
    bdac->appendCloneBack(bases.forward);
    bdac->appendCloneBack(bases.forwardMinus);
    //add to other relevant types here
    ctx->appendCloneProjection(key6, bdac);

    //case7: d.a : if i = n for K|2 and K!|2

    ParticipantKey key7{name, std::format("{1} = max({0}): {0}", indexName, identifier)};
    if(!this->ctx->hasProjection(key7)) {
          ctx->addParticipant(key7);
    }


    std::shared_ptr<ProjectionList> da = std::make_shared<ProjectionList>();
    da->appendCloneBack(bases.backwardPlus);
    da->appendCloneBack(bases.forward);

    //add to other relevant types here

    ctx->appendCloneProjection(key7, da);
  }
}

} // namespace PchorAST