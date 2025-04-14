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
}