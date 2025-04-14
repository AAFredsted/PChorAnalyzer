#pragma once

#include <string>
#include <memory> // For std::shared_ptr

// ASTParser using visitor pattern
namespace PchorAST {

// Forward declaration of PchorASTVisitor
class PchorASTVisitor;

// Base class for all AST nodes
class PchorASTNode {
public:
    // Enum for declaration types
    enum class Decl : uint8_t { Index_Decl, Participant_Decl, Channel_Decl, Label_Decl, Global_Type_Decl };
    // Enum for reference types
    enum class Ref : uint8_t { Index_Ref, Participant_Ref, Channel_Ref, Label_Ref, Global_Type_Ref };

    virtual ~PchorASTNode() = default;

    // Get the declaration type
    Decl getDeclType() const { return decl; }

    // Accept a visitor (pure virtual function)
    virtual void accept(PchorASTVisitor &visitor) const = 0;

protected:
    Decl decl;

    // Protected constructor to ensure this class is abstract
    explicit PchorASTNode(Decl decl) : decl(decl) {}
};

// Class for declaration AST nodes
class DeclPchorASTNode : public PchorASTNode {
public:
    // Get the name of the declaration
    const std::string& getName() const { return name; }

    // Accept a visitor
    void accept(PchorASTVisitor &visitor) const override;

protected:
    std::string name;

    // Constructor
    explicit DeclPchorASTNode(Decl declType, std::string name);
};

// Class for reference AST nodes
class RefPchorASTNode : public PchorASTNode {
public:
    // Get the name of the reference
    const std::string& getName() const { return name; }

    // Accept a visitor
    void accept(PchorASTVisitor &visitor) const override;

protected:
    Ref ref;
    std::string name;

    // Constructor
    explicit RefPchorASTNode(Decl declType, std::string name);
};

} // namespace PchorAST