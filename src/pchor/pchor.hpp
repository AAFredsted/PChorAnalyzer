//ASTParser using visitorpattern
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include "PchorFileWrapper.hpp"

namespace PchorAST {

class PchorASTVisitor; //parser used for mapping
class EvalCTX; //cppwrapping

class PchorASTNode {
public:
    enum class Decl :  uint8_t { Index_Decl, Participant_Decl, Channel_Decl, Label_Decl, Global_Type_Decl };
    enum class Ref : uint8_t { Index_Ref, Participant_Ref, Channel_Ref, Label_Ref, Global_Type_Ref };

    virtual ~PchorASTNode() = default;

    Decl getDeclType() const { return decl; }

    virtual void accept(PchorASTVisitor &visitor) const = 0;

protected:
    Decl decl;
    explicit PchorASTNode(Decl decl) : decl(decl) {} 
};

class DeclPchorASTNode : public PchorASTNode {
public:
    const std::string& getName() const { return name; }
    void accept(PchorASTVisitor &visitor) const override; 


protected:
    std::string name;

    explicit DeclPchorASTNode(Decl declType, std::string name);
};


class RefPchorASTNode : public PchorASTNode {
public:
    const std::string& getName() const { return name; }
    void accept(PchorASTVisitor &visitor) const override; 

protected:
    Ref ref;
    std::string name;

    explicit RefPchorASTNode(Decl declType, std::string name);
};






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
    explicit Parser(const std::string &filePath, SymbolTable &symbolTable) : file(filePath), chorSymbolTable(symbolTable) {}

    void parse();

private:
    SymbolTable& chorSymbolTable;
    PchorFileWrapper file;

    void generateTokens();

    void parseDeclaration();

    void parseParticipant();

    void parseChannel();

    void parseIndex();

    void ParseGlobalType();

    void fetchReference();
};

} //namespace PchorAST