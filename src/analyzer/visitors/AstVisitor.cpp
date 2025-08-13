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

  size_t lower = baseIndex->getLower();
  size_t upper = baseIndex->getUpper();

  if(literal < lower|| literal > upper){
    throw std::runtime_error(std::format("Index expression {} evaluated to {}, which is not within the range of [{}, {}].", indexExpr->toString(), literal, baseIndex->getLower(), baseIndex->getUpper()));
  }
  ParticipantKey key{expr.getBaseParticipant()->getName(), literal};


  //this needs to be rewritten, and we can work it out
  if (!this->ctx->hasParticipantGroup(key.name)) {
    this->ctx->addParticipantGroup(key.name);
  }
  if (this->isSender) {
    this->ctx->addLiteralProjection(key,
                              std::make_unique<Psend>(this->currentChannelName,
                                                      this->currentDataType,
                                                      this->channelIndex),
                              lower,
                              upper
                                                    );
  } else {
    this->ctx->addLiteralProjection(
        key, std::make_unique<Preceive>(this->currentChannelName,
                                        this->currentDataType,
                                        this->channelIndex),
             lower,
             upper                         
                                      );
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
  size_t l = baseIndex->getLower();
  size_t n = baseIndex->getUpper();
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
        if(debug) {
          for(const auto& [name, elem]: fullIterCase) {
            elem.print(name);
          }
        }

        addEquivalenceClassesFull(fullIterCase, identifier, indexName, l, n);
        break;
      case IterType::MaxExIter :
        maxIterCase = getCasesMaxEx(body, identifier);
        if(debug) {
          for(const auto& [name, elem]: maxIterCase) {
            elem.print(name);
          }
        }        

        addEquivalenceClassesMaxEx(maxIterCase, identifier, indexName, l, n);
        break;
      case IterType::MinExIter :
        minIterCase = getCasesMinEx(body, identifier);
        for(const auto& [name, elem]: minIterCase) {
          elem.print(name);
          //missing method here
        }
        addEquivalenceClassesMinEx(minIterCase, identifier, indexName, l, n);
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
                   const std::shared_ptr<ChannelExpr>& channel,
                   const std::string& dataType,
                   const std::string& identifier
                  ) {

  /*
  Simple case setup
  if evaluated to i, insert Comtype(Isend|Ireceive) into a and a+b
  if evaluated to n-i+l, insert Comtype(Isend|Ireceive) into b and a+b
  */
  const std::string& channelName = channel->getBaseParticipant()->getName();
  const auto index = channel->getIndex();
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
                   const std::shared_ptr<ChannelExpr>& channel,
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
  const std::string& channelName = channel->getBaseParticipant()->getName();
  const auto index = channel->getIndex();
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
                   const std::shared_ptr<ChannelExpr>& channel,
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
  const std::string& channelName = channel->getBaseParticipant()->getName();
  const auto index = channel->getIndex();
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
    insertFullPattern<Isend>(BasePattern.at(senderName), sender, channel, dataType, i);

    const std::string receiverName = receiver->getBaseParticipant()->getName();
    if(!BasePattern.contains(receiverName)) {
      BasePattern.try_emplace(receiverName);
    }
    insertFullPattern<Ireceive>(BasePattern.at(receiverName), receiver, channel, dataType, i);

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
    insertMaxExPattern<Isend>(BasePattern.at(senderName), sender, channel, dataType, i);

    const std::string receiverName = receiver->getBaseParticipant()->getName();
    if(!BasePattern.contains(receiverName)) {
      BasePattern.try_emplace(receiverName);
    }
    insertMaxExPattern<Ireceive>(BasePattern.at(receiverName), receiver, channel, dataType, i);

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
    insertMinExPattern<Isend>(BasePattern.at(senderName), sender, channel, dataType, i);

    const std::string receiverName = receiver->getBaseParticipant()->getName();
    if(!BasePattern.contains(receiverName)) {
      BasePattern.try_emplace(receiverName);
    }
    insertMinExPattern<Ireceive>(BasePattern.at(receiverName), receiver, channel, dataType, i);

  }
  return BasePattern;
}
void Proj_PchorASTVisitor::addEquivalenceClassesFull(std::unordered_map<std::string, FullIter>& baseCases, [[maybe_unused]]  const std::string& identifier, [[maybe_unused]] const std::string& indexName, size_t l, size_t n) {
  /*  
    for every FullIter, we run the following assesment:
    if(backward.empty()) we only need to care about forward case
    
    case: a

    else we also need to care about b and a+b, where the following pattern appears

    for i: I = [l,n] and K=|I| and c=rounddown((K-l)/2)+l, the following patterns emerge for all n and l<n:
    a.b : [l,c) for K|2 and K!|2
    a+b : [c,c] for K!|2
    b.a : [c,n] K|2 and (c,n] for K!|2

    We turn the above into equivalence classes and add them to our key system

    In total, three distinct patterns are possible, against which must be checked for all.
  */

  for(const auto& [name, bases] : baseCases) {

      if(!this->ctx->hasParticipantGroup(name)){
        ctx->addParticipantGroup(name);
      }

      // a.b : [l,c) for K|2 and K!|2
      Range r1 = Range{Bound{RangeSymbol::L, true, true}, Bound{RangeSymbol::C, false, false}};
      ParticipantKey key1{name, r1, EvenCase::Both};
      ParticipantKey key1o{name, r1, EvenCase::Odd};

      std::shared_ptr<ProjectionList> case1 = std::make_shared<ProjectionList>();
      case1->appendCloneBack(bases.forward);
      case1->appendCloneBack(bases.backward);

      //add to other relevant types here
      //new function to handle all cases for us :) <3
      ctx->appendCloneRangedProjection(key1, case1,l, n);
      //ctx->appendCloneRangedProjection(key1o, case1,l, n);


      //a+b : [c,c] for K!|2
      Range r2 = Range{Bound{RangeSymbol::C, true, true}, Bound{RangeSymbol::C, true, false}};
      ParticipantKey key2{name, r2, EvenCase::Odd};

      std::shared_ptr<ProjectionList> apb = std::make_shared<ProjectionList>();
      apb->appendCloneBack(bases.unevenOverlapAB);

      //add to other relevant types here

      ctx->appendCloneRangedProjection(key2, apb, l, n);
      

      // b.a : [c,n] K|2 and (c,n] for K!|2
      Range r3e = Range{Bound{RangeSymbol::C, true, true}, Bound{RangeSymbol::N,true, false}};
      ParticipantKey key3e{name, r3e, EvenCase::Even};
      Range r3o = Range{Bound{RangeSymbol::C, false, true}, Bound{RangeSymbol::N, true, false}};
      ParticipantKey key3o{name, r3o, EvenCase::Odd};

      std::shared_ptr<ProjectionList> case3 = std::make_shared<ProjectionList>();
      case3->appendCloneBack(bases.backward);
      case3->appendCloneBack(bases.forward);

      //add to other relevant types here
      //return here
      ctx->appendCloneRangedProjection(key3e, case3, l, n);
      //we only remove when adding closing odd case !
      ctx->appendCloneRangedProjection(key3o, case3, l, n);

    }
  }

void Proj_PchorASTVisitor::addEquivalenceClassesMaxEx(std::unordered_map<std::string, MaxExcludingIter>& baseCases, [[maybe_unused]]  const std::string& identifier, [[maybe_unused]]  const std::string& indexName, size_t l, size_t n ) {
  /*
    for every MaxExcludingIter, we run the following assesment:
    if(backward.empty() and backwardMinus.empty()) we only need to care about forward and forwardPlus case
    This logic has not been added, but will be at some future date

    a for i = l
    c.a for 1<i<n
    c for i = n
    

    else we also need to care about  a, b, c, d, and all overlaps (a+b, c+d | a+d, b+c)

    for i: I = [l,n] and K=|I| and k=rounddown((K-l)/2)-l, the following patterns emerge for all n and l<n:

    a.d : [l,l] for K|2 and K!|2
    c.a.d.b : (l, c-1)  for K|2 and (l, c-1] and K!|2
    c+d.a+b : [c, c] for K!|2
    c.a+d.b : [c-1, c-1] for K|2
    d.c+b.a : [c, c] for K|2
    d.b.c.a : (c, n) for K|2 and K!|2
    b.c : [n,n] for K|2 and K!|2
    We turn the above into equivalence classes and add them to our key system

    In total, seven distinct patterns are possible, against which must be checked for all.
  */
  for(const auto& [name, bases]: baseCases) {

    if(!this->ctx->hasParticipantGroup(name)){
      this->ctx->addParticipantGroup(name);
    }
    
    //case 1: a.d : [l,l] for K|2 and K!|2
    ParticipantKey key1{name, Range{Bound{RangeSymbol::L, true, true}, Bound{RangeSymbol::L, true, false}}, EvenCase::Both};

    std::shared_ptr<ProjectionList> case1 = std::make_shared<ProjectionList>();
    case1->appendCloneBack(bases.forward);
    case1->appendCloneBack(bases.backwardMinus);

    //add to other relevant types here
    ctx->appendCloneRangedProjection(key1, case1, l, n);

    //case 2: c.a.d.b : (l, c-1)  for K|2 and (l, c-1] and K!|2 issue here
    ParticipantKey key2e{name, Range{Bound{RangeSymbol::L, false, true}, Bound{RangeSymbol::lC, false, false}}, EvenCase::Even};
    ParticipantKey key2o{name, Range{Bound{RangeSymbol::L, false, true}, Bound{RangeSymbol::lC, true, false}}, EvenCase::Odd};


    std::shared_ptr<ProjectionList> case2 = std::make_shared<ProjectionList>();
    case2->appendCloneBack(bases.forwardPlus);
    case2->appendCloneBack(bases.forward);
    case2->appendCloneBack(bases.backwardMinus);
    case2->appendCloneBack(bases.backward);

    //add to other relevant types here

    ctx->appendCloneRangedProjection(key2e, case2, l, n);
    ctx->appendCloneRangedProjection(key2o, case2, l, n);

    //case 3:  c+d.a+b : [c, c] for K!|2

    ParticipantKey key3{name, Range{Bound{RangeSymbol::C, true, true}, Bound{RangeSymbol::C, true, false}}, EvenCase::Odd};

    std::shared_ptr<ProjectionList> case3 = std::make_shared<ProjectionList>();
    case3->appendCloneBack(bases.unevenOverlapCD);
    case3->appendCloneBack(bases.unevenOverlapAB);

    //add to other relevant types here

    ctx->appendCloneRangedProjection(key3, case3, l, n);
    

    //case 4: c.a+d.b : [c-1, c-1] for K|2

    ParticipantKey key4{name, Range{Bound{RangeSymbol::lC, true, true}, Bound{RangeSymbol::lC, true, false}}, EvenCase::Even};


    std::shared_ptr<ProjectionList> case4 = std::make_shared<ProjectionList>();
    case4->appendCloneBack(bases.forwardPlus);
    case4->appendCloneBack(bases.evenOverlapAD);
    case4->appendCloneBack(bases.backward);

    //add to other relevant types here
    ctx->appendCloneRangedProjection(key4, case4, l, n);

    //case 5: d.c+b.a : [c, c] for K|2
    ParticipantKey key5{name, Range{Bound{RangeSymbol::C, true, true}, Bound{RangeSymbol::C, true, false}}, EvenCase::Even};

    std::shared_ptr<ProjectionList> case5 = std::make_shared<ProjectionList>();
    case5->appendCloneBack(bases.backwardMinus);
    case5->appendCloneBack(bases.evenOverlapBC);
    case5->appendCloneBack(bases.forward);
    //add to other relevant types here
    ctx->appendCloneRangedProjection(key5, case5, l, n);

    //case 6: d.b.c.a : (c, n) for K|2 and K!|2

    ParticipantKey key6{name, Range{Bound{RangeSymbol::C, false, true}, Bound{RangeSymbol::N, false, false}}, EvenCase::Both};

    std::shared_ptr<ProjectionList> case6 = std::make_shared<ProjectionList>();
    case6->appendCloneBack(bases.backwardMinus);
    case6->appendCloneBack(bases.backward);
    case6->appendCloneBack(bases.forwardPlus);
    case6->appendCloneBack(bases.forward);
    //add to other relevant types here
    ctx->appendCloneRangedProjection(key6, case6, l, n);

    //case7: [n,n] for K|2 and K!|2
    ParticipantKey key7{name, Range{Bound{RangeSymbol::N, true, true}, Bound{RangeSymbol::N, true, false}}, EvenCase::Both};

    std::shared_ptr<ProjectionList> case7 = std::make_shared<ProjectionList>();
    case7->appendCloneBack(bases.backward);
    case7->appendCloneBack(bases.forwardPlus);

    //add to other relevant types here

    ctx->appendCloneRangedProjection(key7, case7, l, n);
  }
}

void Proj_PchorASTVisitor::addEquivalenceClassesMinEx(std::unordered_map<std::string, MinExcludingIter>& baseCases, [[maybe_unused]] const std::string& identifier, [[maybe_unused]] const std::string& indexName, size_t l, size_t n) {
  /*
    for every MinExcludingIter, we run the following assesment:
    if(backward.empty() and backwardPlus.empty()) we only need to care about forward and forwardPlus case
    This logic has not been added, but will be at some future date

    c for i = l
    a.c for 1<i<n
    a for i = n
    

    else we also need to care about  a, b, c, d, and all overlaps (a+b, c+d | a+d, b+c)

    for i: I = [l,n] and K=|I| and k=rounddown((K-l)/2)+l, the following patterns emerge for all n and l<n:

    c.b : [l,l] for K|2 and K!|2
    a.c.b.d: (l, c-1) for K|2 and (l, c-1] K!|2
    a+b.c+d :[c,c] for K!|2
    a.b+c.d : [c-1,c-1] for K|2
    b.a+d.c : [c,c] for K|2
    b.d.a.c : (c,n) for K|2 and K!|2
    d.a : [n,n for] K|2 and K!|2
    We turn the above into equivalence classes and add them to our key system

    In total, seven distinct patterns are possible, against which must be checked for all.
  */
  for(const auto& [name, bases]: baseCases) {
    
    if(!this->ctx->hasParticipantGroup(name)){
      this->ctx->addParticipantGroup(name);
    }
    
    
    //case 1: c.b : [l,l] for K|2 and K!|2
    ParticipantKey key1{name, Range{Bound{RangeSymbol::L, true, true}, Bound{RangeSymbol::L, true, false}}, EvenCase::Both};

    std::shared_ptr<ProjectionList> case1 = std::make_shared<ProjectionList>();
    case1->appendCloneBack(bases.forwardMinus);
    case1->appendCloneBack(bases.backward);

    //add to other relevant types here

    ctx->appendCloneRangedProjection(key1, case1, l, n);

    //case 2: a.c.b.d: (l, c-1) for K|2 and (l, c-1] K!|2
    ParticipantKey key2e{name, Range{Bound{RangeSymbol::L, false, true}, Bound{RangeSymbol::lC, false, false}}, EvenCase::Even};
    ParticipantKey key2o{name, Range{Bound{RangeSymbol::L, false, true}, Bound{RangeSymbol::lC, true, false}}, EvenCase::Odd};


    std::shared_ptr<ProjectionList> case2 = std::make_shared<ProjectionList>();
    case2->appendCloneBack(bases.forward);
    case2->appendCloneBack(bases.forwardMinus);
    case2->appendCloneBack(bases.backward);
    case2->appendCloneBack(bases.backwardPlus);

    //add to other relevant types here

    ctx->appendCloneRangedProjection(key2e, case2, l, n);
    ctx->appendCloneRangedProjection(key2o, case2, l, n);

    //case 3: a+b.c+d :[c,c] for K!|2
    ParticipantKey key3{name, Range{Bound{RangeSymbol::C, true, true}, Bound{RangeSymbol::C, true, false}}, EvenCase::Odd};


    std::shared_ptr<ProjectionList> case3 = std::make_shared<ProjectionList>();
    case3->appendCloneBack(bases.unevenOverlapAB);
    case3->appendCloneBack(bases.unevenOverlapCD);

    //add to other relevant types here

    ctx->appendCloneRangedProjection(key3, case3, l, n);
    

    //case 4: a.b+c.d : [c-1,c-1] for K|2
    ParticipantKey key4{name, Range{Bound{RangeSymbol::lC, true, true}, Bound{RangeSymbol::lC, true, false}}, EvenCase::Even};

    std::shared_ptr<ProjectionList> case4 = std::make_shared<ProjectionList>();
    case4->appendCloneBack(bases.forward);
    case4->appendCloneBack(bases.evenOverlapBC);
    case4->appendCloneBack(bases.backwardPlus);

    //add to other relevant types here
    ctx->appendCloneRangedProjection(key4, case4, l, n);

    //case 5: b.a+d.c : [c,c] for K|2
    ParticipantKey key5{name, Range{Bound{RangeSymbol::C, true, true}, Bound{RangeSymbol::C, true, false}}, EvenCase::Even};

    std::shared_ptr<ProjectionList> case5 = std::make_shared<ProjectionList>();
    case5->appendCloneBack(bases.backward);
    case5->appendCloneBack(bases.evenOverlapAD);
    case5->appendCloneBack(bases.forwardMinus);
    //add to other relevant types here
    ctx->appendCloneRangedProjection(key5, case5, l, n);

    //case 6: b.d.a.c : (c,n) for K|2 and K!|2

    ParticipantKey key6{name, Range{Bound{RangeSymbol::C, false, true}, Bound{RangeSymbol::N, false, false}}, EvenCase::Both};

    std::shared_ptr<ProjectionList> case6 = std::make_shared<ProjectionList>();
    case6->appendCloneBack(bases.backward);
    case6->appendCloneBack(bases.backwardPlus);
    case6->appendCloneBack(bases.forward);
    case6->appendCloneBack(bases.forwardMinus);
    //add to other relevant types here
    ctx->appendCloneRangedProjection(key6, case6, l, n);

    //case7: d.a : [n,n for] K|2 and K!|2

    ParticipantKey key7{name, Range{Bound{RangeSymbol::N, true, true}, Bound{RangeSymbol::N, true, false}}, EvenCase::Both};

    std::shared_ptr<ProjectionList> case7 = std::make_shared<ProjectionList>();
    case7->appendCloneBack(bases.backwardPlus);
    case7->appendCloneBack(bases.forward);

    //add to other relevant types here

    ctx->appendCloneRangedProjection(key7, case7, l, n);
  }
}

} // namespace PchorAST