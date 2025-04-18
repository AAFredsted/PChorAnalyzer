#pragma once
#include "PchorFileWrapper.hpp"
#include "PchorAST.hpp"

#include <unordered_map>
#include <memory> // For std::unique_ptr
#include <print>

namespace PchorAST {

class SymbolTable {
public: 
    void addDeclaration(const std::string& name, std::shared_ptr<PchorASTNode> node);
    std::shared_ptr<PchorASTNode> resolve(const std::string& name) const;
    std::shared_ptr<PchorASTNode> resolve(const std::string_view name) const;
private:
    std::unordered_map<std::string, std::shared_ptr<PchorASTNode>> table;
};

class PchorParser {
public:
    // Constructor now takes ownership of lexer and symbol table
    explicit PchorParser(const std::string& filePath) : lexer(std::make_unique<PchorLexer>(filePath)), symbolTable(std::make_unique<SymbolTable>()), tokens(){}

    void parse();

private:
    std::unique_ptr<PchorLexer> lexer; // Unique ownership of lexer
    std::unique_ptr<SymbolTable> symbolTable; // Unique ownership of symbol table
    std::vector<Token> tokens;
    
    void parseParticipantDecl(std::vector<Token>::iterator& itr, const std::vector<Token>::iterator& end);
    void parseChannelDecl(std::vector<Token>::iterator& itr, const std::vector<Token>::iterator& end);
    void parseIndexDecl(std::vector<Token>::iterator& itr, const std::vector<Token>::iterator& end);
    void parseGlobalTypeDecl(std::vector<Token>::iterator& itr, const std::vector<Token>::iterator& end);
};

} // namespace PchorAST