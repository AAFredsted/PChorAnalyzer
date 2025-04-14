#pragma once
#include "PchorTokenizer.hpp"
#include "PchorFileWrapper.hpp"
#include "PchorAST.hpp"

#include <unordered_map>


namespace PchorAST {

class SymbolTable {
public: 
    void addDeclaration(const std::string& name, std::shared_ptr<PchorASTNode> node) {
        table.insert(std::make_pair(name, node));
    }

    std::shared_ptr<PchorASTNode> resolve(const std::string& name) const {
        auto it = table.find(name);
        if(it != table.end()){
            return it-> second;
        }
        return nullptr;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<PchorASTNode>> table;
};

class Parser {
public:
    explicit Parser(PchorLexer& lexer, SymbolTable &symbolTable) : lexer(lexer), symbolTable(symbolTable) {}

    void parse();

private:
    PchorLexer& lexer;
    SymbolTable& symbolTable;
    std::vector<TokenType> Tokens;
    
    void parseDeclaration();

    void parseParticipant();

    void parseChannel();

    void parseIndex();

    void ParseGlobalType();

    void fetchReference();
};
    
} //namespace PchorAST