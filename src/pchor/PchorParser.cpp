#include "PchorParser.hpp"

namespace PchorAST {


void SymbolTable::addDeclaration(const std::string& name, std::shared_ptr<PchorASTNode> node) {
    table.insert(std::make_pair(name, node));
}

std::shared_ptr<PchorASTNode> SymbolTable::resolve(const std::string& name) const {
    auto it = table.find(name);
    if(it != table.end()){
        return it-> second;
    }
    return nullptr;
}

void PchorParser::parse() {
    tokens = lexer->genTokens();
    auto itr = tokens.begin();
    const auto end = tokens.end();
    
    for(const Token& t: tokens){
        std::println("{}", t.toString());
    }

    /*
        Parsing of outer expressions, where expressions are limited to declarations of Indeces, Participants, Channels and Global Types
    */
    /*
    while(itr != end){
        switch(itr->type){
            case TokenType::Keyword :
                if(itr->value == "Index") {
                    parseIndexDecl(itr, end);
                }
                else if(itr->value == "Participant") {
                    parseParticipantDecl(itr, end);

                }
                else if(itr->value == "Channel") {
                    parseChannelDecl(itr, end);
                }
                else {
                    throw std::runtime_error("Token: " + itr->toString() + "cannot be an Outer Expression");
                }
                break;
            case TokenType::Identifier :
                parseGlobalTypeDecl(itr, end);
                break;
            default :
                throw std::runtime_error("Token: " +  itr->toString() + "cannot be an Outer Expression Keyword or Identifier");
        }
    }
    */
}

void PchorParser::parseIndexDecl(std::vector<Token>::iterator& itr, const std::vector<Token>::iterator& end) {
    /*
    Declaration Semantics
    Index <Identifier>{<literal>..<literal>}
    which is 8 tokens
    */
    if (std::distance(itr, end) < 8) {
        throw std::runtime_error("Incomplete Index declaration: not enough tokens remaining.");
    }
    if (itr->value != "Index") {
        throw std::runtime_error("Expected 'Index' keyword, but got: " + itr->toString());
    }
        
    // Ensure the next token is an Identifier
    if (itr == end || itr->type != TokenType::Identifier) {
        throw std::runtime_error("Expected Identifier after 'Index', but got: " + itr->toString());
    }
    std::string_view indexName = itr->value; // Store the identifier name
    ++itr; // Move to the next token

    // Ensure the next token is '{'
    if (itr == end || itr->type != TokenType::Symbol || itr->value != "{") {
        throw std::runtime_error("Expected '{' after Identifier, but got: " + itr->toString());
    }
    ++itr; // Move to the next token

    // Parse the lower bound
    if (itr == end || (itr->type != TokenType::Literal && itr->value != "n")) {
        throw std::runtime_error("Expected lower bound literal or 'n', but got: " + itr->toString());
    }

    Token lowerToken = *itr; // Store the lower bound token
    ++itr; // Move to the next token

    // Ensure the next token is '..'
    if (itr == end || itr->type != TokenType::Symbol || itr->value != "..") {
        throw std::runtime_error("Expected '..' after lower bound, but got: " + itr->toString());
    }
    ++itr; // Move to the next token

    // Parse the upper bound
    if (itr == end || (itr->type != TokenType::Literal && itr->value != "n")) {
        throw std::runtime_error("Expected upper bound literal or 'n', but got: " + itr->toString());
    }
    Token upperToken = *itr; // Store the upper bound token
    ++itr; // Move to the next token

    // Ensure the next token is '}'
    if (itr == end || itr->type != TokenType::Symbol || itr->value != "}") {
        throw std::runtime_error("Expected '}' after upper bound, but got: " + itr->toString());
    }
    ++itr; // Move to the next token

    // Create the IndexASTNode and add it to the symbol table
    auto indexNode = std::make_shared<IndexASTNode>(indexName, lowerToken, upperToken);
    symbolTable -> addDeclaration(std::string(indexName), indexNode);
}

}