#pragma once
#include "PchorTokenizer.hpp"
#include "PchorFileWrapper.hpp"
#include "PchorAST.hpp"

#include <unordered_map>
#include <memory> // For std::unique_ptr

namespace PchorAST {

class SymbolTable {
public: 
    void addDeclaration(const std::string& name, std::shared_ptr<PchorASTNode> node);
    std::shared_ptr<PchorASTNode> resolve(const std::string& name) const;
    
private:
    std::unordered_map<std::string, std::shared_ptr<PchorASTNode>> table;
};

class Parser {
public:
    // Constructor now takes ownership of lexer and symbol table
    explicit Parser(std::unique_ptr<PchorLexer> lexer, std::unique_ptr<SymbolTable> symbolTable)
        : lexer(std::move(lexer)), symbolTable(std::move(symbolTable)) {}

    void parse();

private:
    std::unique_ptr<PchorLexer> lexer; // Unique ownership of lexer
    std::unique_ptr<SymbolTable> symbolTable; // Unique ownership of symbol table
    std::vector<TokenType> Tokens;
    
    void parseDeclaration();
    void parseParticipant();
    void parseChannel();
    void parseIndex();
    void ParseGlobalType();
    void fetchReference();
};

} // namespace PchorAST