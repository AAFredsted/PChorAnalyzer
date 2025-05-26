#include "PchorParser.hpp"
#include <string>

namespace PchorAST {

// HelperFunctions

std::vector<Token>::iterator
PchorParser::findEndofScope(std::vector<Token>::iterator &itr,
                            const std::vector<Token>::iterator &end) {
  auto endofScope = itr;
  endofScope++;
  while (endofScope != end && endofScope->value != "}") {
    if (endofScope->value == "{") {
      endofScope = findEndofScope(endofScope, end);
    }
    endofScope++;
  }
  if (endofScope == end) {
    throw std::runtime_error(
        "End of Scope not found. Scope Initiater found at: " + itr->toString());
  }
  return endofScope;
}


std::vector<Token>::iterator
PchorParser::findEndofIterScope(std::vector<Token>::iterator &itr,
                            const std::vector<Token>::iterator &end) {
  auto endofScope = itr;
  endofScope++;
  while (endofScope != end && endofScope->value != ")") {
    if (endofScope->value == "(") {
      endofScope = findEndofIterScope(endofScope, end);
    }
    endofScope++;
  }
  if (endofScope == end) {
    throw std::runtime_error(
        "End of Scope not found. Scope Initiater found at: " + itr->toString());
  }
  return endofScope;
}
// Parsing Tree
void SymbolTable::addDeclaration(const std::string &name,
                                 std::shared_ptr<DeclPchorASTNode> node) {
  keys.push_back(name);
  table.insert(std::make_pair(name, node));
}

std::shared_ptr<DeclPchorASTNode>
SymbolTable::resolve(const std::string &name) const {
  auto it = table.find(name);
  if (it != table.end()) {
    return it->second;
  }
  return nullptr;
}
std::shared_ptr<DeclPchorASTNode>
SymbolTable::resolve(const std::string_view name) const {
  std::string n{name};
  auto it = table.find(n);
  if (it != table.end()) {
    return it->second;
  }
  return nullptr;
}

void PchorParser::printAST() const { symbolTable->print(); }
void PchorParser::printTokenList() const {
  std::println("\n\nToken List provided by "
               "PchorLexer\n--------------------------------");
  for (const Token &t : tokens) {
    std::println("{}", t.toString());
  }
}

void PchorParser::genTokens() { tokens = lexer->genTokens(); }
void PchorParser::parse() {

  // create default index for literal 1
  symbolTable->addDeclaration("PchorUnaryIndex", std::make_shared<IndexASTNode>(
                                                     "PchorUnaryIndex", 1, 1));

  auto itr = tokens.begin();
  const auto end = tokens.end();
  /*
      Parsing of outer expressions, where expressions are limited to
     declarations of Indeces, Participants, Channels and Global Types
  */

  while (itr->type != TokenType::EndOfFile && itr != end) {
    switch (itr->type) {
    case TokenType::Keyword:
      if (itr->value == "Index") {
        parseIndexDecl(itr, end);
      } else if (itr->value == "Participant") {
        parseParticipantDecl(itr, end);
      } else if (itr->value == "Channel") {
        parseChannelDecl(itr, end);
      } else if (itr->value == "Label") {
        parseLabelDecl(itr, end);
      } else {
        throw std::runtime_error("Token: " + itr->toString() +
                                 "cannot be an Outer Expression");
      }
      break;
    case TokenType::Identifier:
      parseGlobalTypeDecl(itr, end);
      break;
    default:
      throw std::runtime_error(
          "Token: " + itr->toString() +
          "cannot be an Outer Expression Keyword or Identifier");
    }
    ++itr;
  }
  std::println("Succesfully parsed file");
}

void PchorParser::parseIndexDecl(std::vector<Token>::iterator &itr,
                                 const std::vector<Token>::iterator &end) {
  /*
  Declaration Semantics
  Index <Identifier>{<literal>..<literal>}
  which is 8 tokens
  */
  if (std::distance(itr, end) < 8) {
    throw std::runtime_error(
        "Incomplete Index declaration: not enough tokens remaining.");
  }
  if (itr->value != "Index") {
    throw std::runtime_error("Expected 'Index' keyword, but got: " +
                             itr->toString());
  }

  ++itr;

  if (itr == end || itr->type != TokenType::Identifier) {
    throw std::runtime_error("Expected Identifier after 'Index', but got: " +
                             itr->toString());
  }
  std::string_view indexName = itr->value;
  ++itr;

  if (itr == end || itr->type != TokenType::Symbol || itr->value != "{") {
    throw std::runtime_error("Expected '{' after Identifier, but got: " +
                             itr->toString());
  }
  ++itr; // Move to the next token

  if (itr == end || (itr->type != TokenType::Literal && itr->value != "n")) {
    throw std::runtime_error("Expected lower bound literal or 'n', but got: " +
                             itr->toString());
  }

  Token lowerToken = *itr;
  ++itr;

  if (itr == end || itr->type != TokenType::Symbol || itr->value != ".") {
    throw std::runtime_error("Expected '.' after lower bound, but got: " +
                             itr->toString());
  }
  ++itr;

  if (itr == end || itr->type != TokenType::Symbol || itr->value != ".") {
    throw std::runtime_error("Expected '..' after lower bound, but got: " +
                             itr->toString());
  }
  ++itr;

  if (itr == end || (itr->type != TokenType::Literal && itr->value != "n")) {
    throw std::runtime_error("Expected upper bound literal or 'n', but got: " +
                             itr->toString());
  }
  Token upperToken = *itr;
  ++itr;

  // verify }
  if (itr == end || itr->type != TokenType::Symbol || itr->value != "}") {
    throw std::runtime_error("Expected '}' after upper bound, but got: " +
                             itr->toString());
  }

  // Create the IndexASTNode and add it to the symbol table
  auto indexNode =
      std::make_shared<IndexASTNode>(indexName, lowerToken, upperToken);
  symbolTable->addDeclaration(std::string(indexName), indexNode);
}

void PchorParser::parseParticipantDecl(
    std::vector<Token>::iterator &itr,
    const std::vector<Token>::iterator &end) {
  /*
  Declaration Semantics
  Participant <Identifier>{Index} || {1}
  which is 5 tokens
  */
  if (std::distance(itr, end) < 5) {
    throw std::runtime_error(
        "Incomplete Index declaration: not enough tokens remaining.");
  }

  if (itr->value != "Participant") {
    throw std::runtime_error("Expected 'Participant' keyword, but got: " +
                             itr->toString());
  }

  itr++;

  if (itr == end || itr->type != TokenType::Identifier) {
    throw std::runtime_error(
        "Expected Identifier after 'Participant', but got: " + itr->toString());
  }
  std::string participantName = std::string(itr->value);
  ++itr;

  if (itr == end || itr->type != TokenType::Symbol || itr->value != "{") {
    throw std::runtime_error("Expected '{' after Identifier, but got: " +
                             itr->toString());
  }
  ++itr;

  std::shared_ptr<DeclPchorASTNode> ASTNode;
  std::shared_ptr<IndexASTNode> IdxNode;

  switch (itr->type) {
  case TokenType::Identifier:
    ASTNode = symbolTable->resolve(itr->value);
    if (ASTNode == nullptr) {
      throw std::runtime_error("Declaration for Identifier " + itr->toString() +
                               "not found");
    }
    if (!(ASTNode->getDeclType() == Decl::Index_Decl)) {
      throw std::runtime_error("Namespace " + itr->toString() +
                               "is not an Index type");
    }
    IdxNode = std::dynamic_pointer_cast<IndexASTNode>(ASTNode);
    break;
  case TokenType::Literal:
    if (itr->value.at(0) != '1') {
      throw std::runtime_error(std::format(
          "Only  literal allowed in participant declaration is '1'. Found {}",
          itr->toString()));
    }
    ASTNode = symbolTable->resolve(std::string("PchorUnaryIndex"));
    IdxNode = std::dynamic_pointer_cast<IndexASTNode>(ASTNode);
    break;
  default:
    throw std::runtime_error(
        "Expected Literal or Identifier of Index, but found: " +
        itr->toString());
  }
  ++itr;

  if (itr == end || itr->type != TokenType::Symbol || itr->value != "}") {
    throw std::runtime_error("Expected '}' after Identifier, but got: " +
                             itr->toString());
  }
  auto Participant =
      std::make_shared<ParticipantASTNode>(participantName, IdxNode);
  symbolTable->addDeclaration(participantName, Participant);
}

void PchorParser::parseChannelDecl(std::vector<Token>::iterator &itr,
                                   const std::vector<Token>::iterator &end) {
  /*
  Declaration Semantics
  Channel <Identifier>{Index} || {1}
  which is 5 tokens
  */
  if (std::distance(itr, end) < 5) {
    throw std::runtime_error(
        "Incomplete Index declaration: not enough tokens remaining.");
  }

  if (itr->value != "Channel") {
    throw std::runtime_error("Expected 'Channel' keyword, but got: " +
                             itr->toString());
  }
  ++itr;
  if (itr == end || itr->type != TokenType::Identifier) {
    throw std::runtime_error("Expected Identifier after 'Channel', but got: " +
                             itr->toString());
  }
  std::string channelName = std::string(itr->value);
  ++itr;

  if (itr == end || itr->type != TokenType::Symbol || itr->value != "{") {
    throw std::runtime_error("Expected '{' after Identifier, but got: " +
                             itr->toString());
  }

  std::shared_ptr<DeclPchorASTNode> ASTNode;
  std::shared_ptr<IndexASTNode> IdxNode;
  ++itr;

  switch (itr->type) {
  case TokenType::Identifier:
    ASTNode = symbolTable->resolve(itr->value);
    if (ASTNode == nullptr) {
      throw std::runtime_error("Declaration for Identifier " + itr->toString() +
                               "not found");
    }
    if (!(ASTNode->getDeclType() == Decl::Index_Decl)) {
      throw std::runtime_error("Namespace " + itr->toString() +
                               "is not an Index type");
    }
    IdxNode = std::dynamic_pointer_cast<IndexASTNode>(ASTNode);
    break;
  case TokenType::Literal:
    if (itr->value.at(0) != '1') {
      throw std::runtime_error(
          "Only unary Channels can be declared with literal Type");
    }
    ASTNode = symbolTable->resolve(std::string("PchorUnaryIndex"));
    IdxNode = std::dynamic_pointer_cast<IndexASTNode>(ASTNode);
    break;
  default:
    throw std::runtime_error(
        "Expected Literal or Identifier of Index, but found: " +
        itr->toString());
  }
  itr++;

  if (itr == end || itr->type != TokenType::Symbol || itr->value != "}") {
    throw std::runtime_error("Expected '}' after Identifier, but got: " +
                             itr->toString());
  }

  auto Channel = std::make_shared<ChannelASTNode>(channelName, IdxNode);
  symbolTable->addDeclaration(channelName, Channel);
}

void PchorParser::parseLabelDecl(std::vector<Token>::iterator &itr,
                                 const std::vector<Token>::iterator &end) {
  /*
  Declaration Semantics
  Label <Identifier>{<Identifier List>}
  which is n tokens
  */
  // validate setup

  if (std::distance(itr, end) < 5) {
    throw std::runtime_error(
        "Incomplete Index declaration: not enough tokens remaining.");
  }

  if (itr->value != "Label") {
    throw std::runtime_error("Expected 'Channel' keyword, but got: " +
                             itr->toString());
  }
  ++itr;
  if (itr == end || itr->type != TokenType::Identifier) {
    throw std::runtime_error("Expected Identifier after 'Channel', but got: " +
                             itr->toString());
  }
  std::string labelName = std::string(itr->value);
  ++itr;

  if (itr == end || itr->type != TokenType::Symbol || itr->value != "{") {
    throw std::runtime_error("Expected Identifier after 'Channel', but got: " +
                             itr->toString());
  }

  auto endofScope = itr;

  while (endofScope != end && endofScope->type != TokenType::Symbol &&
         endofScope->value != "}") {
    endofScope++;
  }

  if (endofScope == end) {
    throw std::runtime_error(
        "Scope not closed for identifier list of Label Declaration" +
        itr->toString());
  }
  itr++;

  // find endofscope such that we can parse all identifiers inbetween

  std::unordered_set<std::string> identifierSet;

  while (itr != endofScope) {
    if (itr->type == TokenType::Identifier) {
      identifierSet.insert(std::string(itr->value));
      itr++;
    } else {
      throw std::runtime_error("Identifier List may only consist of "
                               "identifiers. Instead, parser recieved: " +
                               itr->toString());
    }
  }

  auto Label = std::make_shared<LabelASTNode>(labelName, identifierSet);
  symbolTable->addDeclaration(labelName, Label);
}
void PchorParser::parseGlobalTypeDecl(std::vector<Token>::iterator &itr,
                                      const std::vector<Token>::iterator &end) {
  /*
  Declaration Semantics
  This is the most complex setup, so it will be split into three parts
  1. direct parsing <identifier> -> <identifier> : channel<type>{
  2. other global types aggregatet with .
  3. global types wrapped in aggregator Rec X() or foreach(i: I ) or (i::ls: I)
  }//possible selection based on label !
  <Identifier> = <aggregate>
  */
  if (itr->type != TokenType::Identifier) {
    throw std::runtime_error(
        "Statement inferred to be a global Type declaration as no explicit "
        "keyword has been used.\n Expected an identifier but got: " +
        itr->toString());
  }
  std::string globalTypeName = std::string(itr->value);

  itr++;
  if (itr->type != TokenType::Symbol && itr->value != "=") {
    throw std::runtime_error(
        "Expected '=' followed by Global Type Declaration. Got: " +
        itr->toString());
  }

  itr++;

  auto endofScope = itr;

  while (endofScope != end && endofScope->value != "end") {
    if (endofScope->type == TokenType::Symbol && endofScope->value == "{") {
        endofScope = findEndofScope(endofScope, end);
    }
    endofScope++;
  }

  if (endofScope == end) {
    throw std::runtime_error("Scope not closed for Global Type Declaration");
  }

  std::shared_ptr<ExprList> expr = parseExpressionList(itr, endofScope);

  auto globaltype = std::make_shared<GlobalTypeASTNode>(globalTypeName, expr);
  symbolTable->addDeclaration(globalTypeName, globaltype);
}

std::shared_ptr<ExprList>
PchorParser::parseExpressionList(std::vector<Token>::iterator &itr,
                                 const std::vector<Token>::iterator &end) {

  // first, identify the type of expression 0:
  //  aggregator around non-aggregate or aggregate com expression
  //  aggregate com expression
  //  non-aggregate com expr

  std::shared_ptr<ExprList> expr = std::make_shared<ExprList>();
  std::println("for expression list begin is {} and end is {}", itr->toString(), end->toString());
  while (itr != end) {
     std::println("current itr is {}", itr->toString());
     std::println("itr == end is {}", itr == end);
     std::println("distance from itr to end: {}", std::distance(itr, end));
    switch (itr->type) {
    // has to be a communication expression
    case TokenType::Identifier: {

      /*
      Declaration Semantics
      <Identifier>?[literal/<index identifier>] -> <Identifier>?[literal/<index
      Identifier>]: <channelIdentifier><Type>?{<GlobalType>} ?.<GlobalType>
      which is n tokens
      */
      // can be identifier of Participant or identifier for other global type
      auto identified = symbolTable->resolve(itr->value);
      auto endofExpr = itr;

      switch (identified->getDeclType()) {
      case Decl::Participant_Decl:
        std::println("we enter setup for communication expression");
        std::println("itr is {}, end is {}", itr->toString(), end->toString());
        while (endofExpr != end && endofExpr->value != ".") {
          endofExpr++;
        }
        std::println("itr is {}, endofExpr is {}", itr->toString(), endofExpr->toString());
        expr->addExpr(parseCommunicationExpr(itr, endofExpr));
        break;
      case Decl::Global_Type_Decl:
        expr->addExpr(std::dynamic_pointer_cast<GlobalTypeASTNode>(identified)->getExprList());
        itr++;
        break;
      default: {
        throw std::runtime_error("Expected Global_Type expression, found: " +
                                 itr->toString());
        break;
      }
      }
      break;
    }
    case TokenType::Keyword: {
      if (itr->value == "end") {
        std::println("do we enter this section?");
        std::println("distance from itr to end before ++: {}", std::distance(itr, end));
        itr++;
        std::println("distance from itr to end after ++: {}", std::distance(itr, end));
        break;
      } else if (itr->value == "foreach") {
        itr++;
        expr->addExpr(parseForEachExpr(itr, end));
        std::println("does something go wrong here?");
        break;
      } else {
        throw std::runtime_error("expected valid keyword for body of GlobalTypeDecl. Found: " +
                                 itr->toString());
      }
      break;
    }
    case TokenType::Symbol: {
      if (itr->value == ".") {
        itr++;
      } else {
        throw std::runtime_error(
            "Expected continuation of expression list '.'. Found: " +
            itr->toString());
      }

      break;
    }
    default: {
      throw std::runtime_error("Expected expression type but got: " +
                               itr->toString());
      break;
    }
    }
  }
  return expr;
}


std::shared_ptr<IterExpr>
PchorParser::parseIterExpr(std::vector<Token>::iterator &itr, const std::vector<Token>::iterator &end) {
  /*
  IterExpr takes one of three shapes
  (<identifier> : <IndexIdentifier> ) #forEach for each (all behavior is equivalent)
  (<identifier> < max(<IndexIdentifier>)) #forEach excluding max (behavior can be split into two equivalence classes)
  (<identifier> > min(<IndexIdentifier>)) #forEach excluding min (behavior can be split into two equivalence classes)
  */  
  
  //1. assume we have non-existant identifier

  if(itr->type != TokenType::Identifier) {
    throw std::runtime_error(
      std::format("Expected Index Identifier. Instead, found: {}", itr->toString())
    );
  }

  if(std::shared_ptr<DeclPchorASTNode> decl = symbolTable->resolve(itr->value)) {
    throw std::runtime_error(
      std::format("Invalid identifier for IterIndex. Identifier {} has previously been declared as {}.",itr->value, decl->toString())
    );
  }

  std::string identifier{itr->value};
  itr++;
  //2. check for which of the three cases we have (i.e, which symbol is used)

  if(itr->type != TokenType::Symbol){
    throw std::runtime_error(
      std::format("Expected one of the symbols ('<', '>', ':'), but found: {}", itr->toString())
    );
  }
  //plan, we allow for three patterns. 
  /*
  A pattern can use n if the projected protocol for each participant does not change with the size of the iteration.
  Hence, we can break it down into equivalence classes (proof for this in paper)
  */

  size_t min;
  size_t max;
  std::shared_ptr<IndexASTNode> IndexASTDecl = nullptr;
  if(itr->value == ":"){
    //we expect the name of an index, where we copy the min and max straight to our setup
    itr++;
    if(itr->type != TokenType::Identifier){
      throw std::runtime_error(std::format("Expected an Identifier for a Index Declaration, recieved {}", itr->toString()));
    }
    auto elem = symbolTable->resolve(itr->value);
    if(!elem || elem->getDeclType() != Decl::Index_Decl){
        throw std::runtime_error(std::format("Identifier {} did not map to an index declaration", itr->value));
    }
    //we now have our base index.. now we get the base modifier
    IndexASTDecl = std::dynamic_pointer_cast<IndexASTNode>(elem);

    min = IndexASTDecl->getLower();
    max = IndexASTDecl->getUpper();
    itr++;
  }
  else if(itr->value == "<"){
    itr++;
    //we assume max here 
    if(itr->type != TokenType::Keyword || itr->value != "max"){
      throw std::runtime_error(std::format("Following the symbol, '<' in a IterExpr, a max operator must occur. Instead, found: {}", itr->toString()));
    }

    itr++;
    max = parseMaxExpr(itr, end, IndexASTDecl)-1;
    min = IndexASTDecl->getLower();
  }
  else if(itr->value == ">"){
    itr++;
    //we assume max here 
    if(itr->type != TokenType::Keyword || itr->value != "min"){
      throw std::runtime_error(std::format("Following the symbol, '<' in a IterExpr, a max operator must occur. Instead, found: {}", itr->toString()));
    }

    itr++;
    min = parseMinExpr(itr, end, IndexASTDecl)-1;
    max = IndexASTDecl->getUpper();
  }
  else {
    throw std::runtime_error(
      std::format("Expected one of the symbols ('<', '>', ':'), but found: {}", itr->toString())
    );
  }
  if(itr->type != TokenType::Symbol || itr->value != ")"){
    throw std::runtime_error(std::format("Expected Iteration Expression to be closed by ')'. Instead, found: {}", itr->value));
  }
  itr++;
  return std::make_shared<IterExpr>(IndexASTDecl, min, max, identifier);

}

std::shared_ptr<ForEachExpr>
PchorParser::parseForEachExpr(std::vector<Token>::iterator &itr, const std::vector<Token>::iterator &end) {
  /*
    forEach has been consumed and we have the expr of type
    forEach(<IterExpr>){<ExprList}.
  */
  if(itr->type !=  TokenType::Symbol || itr->value != "("){
    throw std::runtime_error(
      std::format("Expected '(' after forEach Expr, found {}", itr->toString())
    );
  }
  std::vector<Token>::iterator endOfIterExpr = findEndofIterScope(itr, end);
  itr++;
  std::shared_ptr<IterExpr> iterExpr = parseIterExpr(itr, endOfIterExpr);

  if(itr->type != TokenType::Symbol || itr->value != "{"){
      throw std::runtime_error(
        std::format("Expected '{{' after forEach Expr, found {}", itr->toString())
      );
  }
  std::vector<Token>::iterator endOfExprListScope = findEndofScope(itr, end);
  if(endOfExprListScope->type != TokenType::Symbol || endOfExprListScope->value != "}"){
    throw std::runtime_error(std::format("Body of foreach expression must end in '}}'. Instead, parser found: {}", endOfExprListScope->toString()));
  }
  itr++; //enter scope
  std::println("we enter parseexpressionlist from foreach");
  std::shared_ptr<ExprList> exprList = parseExpressionList(itr, endOfExprListScope);
  if(itr->type != TokenType::Symbol || itr->value != "}"){
    throw std::runtime_error(std::format("Parser failed to parse body of forEach Statement. Stopped at {}", itr->toString()));
  }
  itr++;
  std::println("do we make it past here?");
  return std::make_shared<ForEachExpr>(iterExpr, exprList);
}

std::shared_ptr<CommunicationExpr>
PchorParser::parseCommunicationExpr(std::vector<Token>::iterator &itr,
                                    const std::vector<Token>::iterator &end) {

  // parseSender
  
  auto sender = symbolTable->resolve(itr->value);
  std::println("We have sucessfully identified sender: {}", sender->getName());

  if (!sender || sender->getDeclType() != Decl::Participant_Decl) {
    throw std::runtime_error("Expected Participant Identifier, but got: " +
                             itr->toString());
  }

  auto senderAST = std::dynamic_pointer_cast<ParticipantASTNode>(sender);

  std::shared_ptr<IndexExpr> senderIndex = nullptr;

  itr++;
  std::println("we successfully converted sender and begin to enter indexsection");
  // either beginning of index expr or com operator
  if (itr->type == TokenType::Symbol && itr->value == "[") {
    auto endofIndex = itr;

    while (endofIndex != end && endofIndex->value != "]") {
      endofIndex++;
    }
    if (endofIndex == end || endofIndex->value != "]") {
      throw std::runtime_error("Expected ']', but found: " +
                               endofIndex->toString());
    }

    std::println("we found index and the end of the expr. Begin {}. End {}.", itr->toString(), endofIndex->toString());
    senderIndex = parseIndexExpr(senderAST->getIndex(), itr, endofIndex);
    std::println("we successfully leave indexmanagement");
  } else {
    senderIndex = std::make_shared<IndexExpr>(
        std::dynamic_pointer_cast<IndexASTNode>(
            symbolTable->resolve(std::string("PchorUnaryIndex"))));
  }

  std::shared_ptr<ParticipantExpr> senderexpr =
      std::make_shared<ParticipantExpr>(senderAST, senderIndex);

  if (itr == end || itr->type != TokenType::Symbol || itr->value != "->") {
    throw std::runtime_error("Expected communication operator '->', but got: " +
                             itr->toString());
  }

  itr++;
  // parseReciever

  auto reciever = symbolTable->resolve(itr->value);
  if (!sender || sender->getDeclType() != Decl::Participant_Decl) {
    throw std::runtime_error("Expected Participant Identifier, but got: " +
                             itr->toString());
  }

  auto recieverAST = std::dynamic_pointer_cast<ParticipantASTNode>(reciever);

  std::shared_ptr<IndexExpr> recieverIndex = nullptr;
  itr++;

  // either index or : operator
  if (itr->type == TokenType::Symbol && itr->value == "[") {
    auto endofIndex = itr;

    while (endofIndex != end && endofIndex->value != "]") {
      endofIndex++;
    }
    if (endofIndex == end || endofIndex->value != "]") {
      throw std::runtime_error("Expected ']', but found: " +
                               endofIndex->toString());
    }
    recieverIndex = parseIndexExpr(recieverAST->getIndex(), itr, endofIndex);
  } else {
    recieverIndex = std::make_shared<IndexExpr>(
        std::dynamic_pointer_cast<IndexASTNode>(
            symbolTable->resolve(std::string("PchorUnaryIndex"))));
  }

  std::shared_ptr<ParticipantExpr> recieverexpr =
      std::make_shared<ParticipantExpr>(recieverAST, recieverIndex);

  if (itr->type != TokenType::Symbol || itr->value != ":") {
    throw std::runtime_error("Communication statement requires specifier ':'. "
                             "Instead, parser recieved: " +
                             itr->toString());
  }

  itr++;
  auto channel = symbolTable->resolve(itr->value);

  if (!channel || channel->getDeclType() != Decl::Channel_Decl) {
    throw std::runtime_error("Communication statement requires reference to "
                             "declared channel. Instead, parser recieved: " +
                             itr->toString());
  }

  auto channelAST = std::dynamic_pointer_cast<ChannelASTNode>(channel);
  std::shared_ptr<IndexExpr> channelIndex = nullptr;

  itr++;

  if (itr->type == TokenType::Symbol && itr->value == "[") {
    auto endofIndex = itr;

    while (endofIndex != end && endofIndex->value != "]") {
      endofIndex++;
    }
    if (endofIndex == end || endofIndex->value != "]") {
      throw std::runtime_error("Expected ']', but found: " +
                               endofIndex->toString());
    }
    channelIndex = parseIndexExpr(channelAST->getIndex(), itr, endofIndex);
  } else {
    channelIndex = std::make_shared<IndexExpr>(
        std::dynamic_pointer_cast<IndexASTNode>(
            symbolTable->resolve(std::string("PchorUnaryIndex"))));
  }

  std::shared_ptr<ChannelExpr> channelexpr =
      std::make_shared<ChannelExpr>(channelAST, channelIndex);

  if (itr->type != TokenType::Symbol || itr->value != "<") {
    throw std::runtime_error("Expected '<' but recieved: " + itr->toString());
  }

  itr++;

  if (itr->type != TokenType::Identifier) {
    throw std::runtime_error("Expected DataType Namespace, but recieved: " +
                             itr->toString());
  }
  std::string dataType = std::string(itr->value);
  itr++;

  if (itr->type != TokenType::Symbol || itr->value != ">") {
    throw std::runtime_error("Expected '>' but recieved: " + itr->toString());
  }

  // go to next expression
  itr++;
  return std::make_shared<CommunicationExpr>(senderexpr, recieverexpr,
                                             channelexpr, dataType, nullptr);
}

std::shared_ptr<IndexExpr>
PchorParser::parseIndexExpr(std::shared_ptr<IndexASTNode> indexType,
                            std::vector<Token>::iterator &itr,
                            const std::vector<Token>::iterator &end) {

    

  itr++;
  bool isLiteral = true;
  std::println("we have entered parseindexexpr and try run parseArithmeticExpr");
  std::unique_ptr<BaseArithmeticExpr> aritExpr = parseArithmeticExpr(indexType, itr, end, isLiteral);
  std::println("we successfully left parseArithmeticExpr");

  std::shared_ptr<IndexExpr> expr = std::make_shared<IndexExpr>(indexType, std::move(aritExpr), isLiteral);

  if (itr != end) {
    throw std::runtime_error("Expected end of index expression ']'. Found: " +
                             itr->toString());
  }
  if (expr == nullptr) {
    std::println("Not Implemented");
  }
  itr++;
  return expr;
}
// Todo: Implement recursive expression parser
std::shared_ptr<RecExpr> PchorParser::parseRecursiveExpr(
    [[maybe_unused]] std::vector<Token>::iterator &itr,
    [[maybe_unused]] const std::vector<Token>::iterator &end) {
  std::println("Not Implemented");
  return nullptr;
}

size_t PchorParser::parseMaxExpr(std::vector<Token>::iterator &itr, const std::vector<Token>::iterator &end, std::shared_ptr<IndexASTNode>& nodePtr) {
  
  if(itr->type != TokenType::Symbol || itr->value != "(") {
    throw std::runtime_error(std::format("Expected symbol, '(', following min-operator. Instead, found: {}", itr->toString()));
  }
  itr++;

  if(itr->type != TokenType::Identifier) {
    throw std::runtime_error(std::format("Expected Identifier as argument for min-operator. Instead, found: {}", itr->toString()));
  }
  auto elem = symbolTable->resolve(itr->value);

  if(!elem || elem->getDeclType() != Decl::Index_Decl) {
    throw std::runtime_error(std::format("Identifier {} did not map to an index declaration", itr->value));
  }

  nodePtr = std::dynamic_pointer_cast<IndexASTNode>(elem);

  itr++;

  if(itr->type != TokenType::Symbol || itr->value != ")") {
    throw std::runtime_error(std::format("Expected symbol, '(', following min-operator. Instead, found: {}", itr->toString()));
  }
  itr++;

  return nodePtr->getUpper();
}

size_t PchorParser::parseMinExpr(std::vector<Token>::iterator &itr, const std::vector<Token>::iterator &end, std::shared_ptr<IndexASTNode>& nodePtr) {

    if(itr->type != TokenType::Symbol || itr->value != "(") {
      throw std::runtime_error(std::format("Expected symbol, '(', following min-operator. Instead, found: {}", itr->toString()));
    }
    itr++;

    if(itr->type != TokenType::Identifier) {
      throw std::runtime_error(std::format("Expected Identifier as argument for min-operator. Instead, found: {}", itr->toString()));
    }
    auto elem = symbolTable->resolve(itr->value);

    if(!elem || elem->getDeclType() != Decl::Index_Decl) {
      throw std::runtime_error(std::format("Identifier {} did not map to an index declaration", itr->value));
    }

    nodePtr = std::dynamic_pointer_cast<IndexASTNode>(elem);

    itr++;

    if(itr->type != TokenType::Symbol || itr->value != ")") {
      throw std::runtime_error(std::format("Expected symbol, '(', following min-operator. Instead, found: {}", itr->toString()));
    }
    itr++;
    return nodePtr->getLower();

}
//recursive descent parsing based !

std::unique_ptr<BaseArithmeticExpr> PchorParser::parseArithmeticExpr(std::shared_ptr<IndexASTNode> indexType, std::vector<Token>::iterator &itr, const std::vector<Token>::iterator &end, bool& isLiteral) {
  std::println("we now enter parseArithmeticExpr");
  std::println("itr is {}, end is {}", itr->toString(), end->toString());
  auto left = parsePrimaryArithmeticExpr(indexType, itr, end, isLiteral);
  std::println("we successfully found left {}", left->toString());
  //we only deal with symbols from here !
  std::println("itr should be at end 0:: itr is:  {}, end is {}", itr->toString(), end->toString());
  std::println("ptr dif is: {}", std::distance(itr, end));
  while(itr != end && itr->type == TokenType::Symbol) {
    std::println("itr is:  {}, end is {}", itr->toString(), end->toString());
    std::println("ptr dif is: {}", std::distance(itr, end));
    ArithmeticExpr type;
    if(itr->value == "+"){
      type = ArithmeticExpr::Addition;
    }
    else if(itr->value == "-"){
      type = ArithmeticExpr::Subtraction;
    }
    else {
      throw std::runtime_error(std::format("Arithmetic Expressions can only be connected with symbols '+' or '-'. Instead, found {}", itr->toString()));
    }
    itr++;
    auto right = parsePrimaryArithmeticExpr(indexType, itr, end, isLiteral);

    switch(type) {
      case ArithmeticExpr::Addition:
        left = std::make_unique<AdditionExpr>(std::move(left), std::move(right));
        break;
      case ArithmeticExpr::Subtraction:
        left = std::make_unique<SubstractionExpr>(std::move(left), std::move(right));
        break;
      default :
        throw std::runtime_error(std::format("Arithmetic Expressions can only be connected with symbols '+' or '-'. This error should not be possible"));
    }
  }
  return left;
}

std::unique_ptr<BaseArithmeticExpr> PchorParser::parsePrimaryArithmeticExpr(std::shared_ptr<IndexASTNode> indexType, std::vector<Token>::iterator &itr, const std::vector<Token>::iterator &end, bool& isLiteral) {
  std::unique_ptr<BaseArithmeticExpr> left;

  if(itr == end) {
    throw std::runtime_error("Unexpected End of Input Expression");
  }
  
  std::unique_ptr<BaseArithmeticExpr> node;
  //for case where we have min or max
  std::shared_ptr<IndexASTNode> exprIndexDecl = nullptr;
  std::vector<Token>::iterator endofScope;
  switch(itr->type) {
    case TokenType::Literal:
      node = std::make_unique<LiteralExpr>(std::stoull(std::string(itr->value)));
      itr++;
      break;
    case TokenType::Identifier:
      isLiteral = false;
      node = std::make_unique<IdentifierExpr>(itr->value);
      itr++;
      break;
    case TokenType::Symbol:
      if(itr->value != "("){
        throw std::runtime_error(std::format("Only symbols '(' or ')' allowed at expression level. Instead, found: {}", itr->toString()));
      }
      endofScope = findEndofIterScope(itr, end);
      itr++;
      node = parseArithmeticExpr(indexType, itr, endofScope, isLiteral);
      itr++;
      break;
    case TokenType::Keyword:
      std::println("we successfully identified min or max");
      if(itr->value != "min" && itr->value != "max"){
        throw std::runtime_error(std::format("Only keywords'min' or 'max' allowed at expression level. Instead, found: {}", itr->toString()));
      }
      size_t literal;
      if(itr->value == "min") {
        itr++;
        literal = parseMinExpr(itr, end, exprIndexDecl);
      }
      else if(itr->value == "max") {
        itr++;
        literal = parseMaxExpr(itr, end, exprIndexDecl);
      }
      else {
        throw std::runtime_error(std::format("neither parsemin nor parsemax was run"));
      }
      std::println("we try to validate the names of the identified decls");
      std::println("do we have indexType? {}", indexType->getName());
      std::println("do we have exprIndexDecl? {}", exprIndexDecl->getName());
      if(indexType->getName() != exprIndexDecl->getName()){
        throw std::runtime_error(std::format("Arithmetic Expression Contains reference to Index Declaration that is unrelated to the indexed type. Base requires {}. Instead, found {}", indexType->getName(), exprIndexDecl->getName()));
      }
      std::println("we successfully validated them");
      node = std::make_unique<LiteralExpr>(literal);

      break;

    default:
      throw std::runtime_error("Unexpected token in arithmetic expression: " + itr->toString());
  }
  return node;

}


} // namespace PchorAST