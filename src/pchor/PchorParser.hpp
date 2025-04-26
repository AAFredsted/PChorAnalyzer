#pragma once
#include "PchorAST.hpp"
#include "PchorFileWrapper.hpp"

#include <memory> // For std::unique_ptr
#include <print>
#include <unordered_map>

namespace PchorAST {

class SymbolTable {
public:
  void addDeclaration(const std::string &name,
                      std::shared_ptr<DeclPchorASTNode> node);
  std::shared_ptr<DeclPchorASTNode> resolve(const std::string &name) const;
  std::shared_ptr<DeclPchorASTNode> resolve(const std::string_view name) const;
  
  void print() {
    size_t i = 0;
    for (const std::string& key: keys) {
      i++;
      std::println("We print ASTElement {}", i);
      table[key]->print();
    }
  }

private:
  std::unordered_map<std::string, std::shared_ptr<DeclPchorASTNode>> table;
  std::vector<std::string> keys;
};

class PchorParser {
public:
  // Constructor now takes ownership of lexer and symbol table
  explicit PchorParser(const std::string &filePath)
      : lexer(std::make_unique<PchorLexer>(filePath)),
        symbolTable(std::make_shared<SymbolTable>()), tokens() {}

  void parse();

  std::shared_ptr<SymbolTable> getChorAST() {
    return symbolTable;
  }

private:
  std::unique_ptr<PchorLexer> lexer;        // Unique ownership of lexer
  std::shared_ptr<SymbolTable> symbolTable; // Unique ownership of symbol table
  std::vector<Token> tokens;

  void parseParticipantDecl(std::vector<Token>::iterator &itr,
                            const std::vector<Token>::iterator &end);
  void parseChannelDecl(std::vector<Token>::iterator &itr,
                        const std::vector<Token>::iterator &end);
  void parseIndexDecl(std::vector<Token>::iterator &itr,
                      const std::vector<Token>::iterator &end);
  void parseLabelDecl(std::vector<Token>::iterator &itr,
                      const std::vector<Token>::iterator &end);
  void parseGlobalTypeDecl(std::vector<Token>::iterator &itr,
                           const std::vector<Token>::iterator &end);
  std::shared_ptr<ExprList>
  parseExpressionList(std::vector<Token>::iterator &itr,
                      const std::vector<Token>::iterator &end);
  std::shared_ptr<ParticipantExpr>
  parseParticipantExpr(std::vector<Token>::iterator &itr,
                       const std::vector<Token>::iterator &end);
  std::shared_ptr<ChannelExpr>
  parseChannelExpr(std::vector<Token>::iterator &itr,
                   const std::vector<Token>::iterator &end);
  std::shared_ptr<IndexExpr>
  parseIndexExpr(std::shared_ptr<IndexASTNode> index,
                 std::vector<Token>::iterator &itr,
                 const std::vector<Token>::iterator &end);
  std::shared_ptr<CommunicationExpr>
  parseCommunicationExpr(std::vector<Token>::iterator &itr,
                         const std::vector<Token>::iterator &end);
  std::shared_ptr<RecExpr>                       
  parseRecursiveExpr(std::vector<Token>::iterator &itr,
                      const std::vector<Token>::iterator &end);

  std::vector<Token>::iterator
  findEndofScope(std::vector<Token>::iterator &itr,
                const std::vector<Token>::iterator &end);
  
};

} // namespace PchorAST