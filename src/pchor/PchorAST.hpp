#pragma once

#include <string>
#include <memory> // For std::shared_ptr
#include <limits>

#include "PchorTokenizer.hpp"

// ASTParser using visitor pattern
namespace PchorAST {

struct Token;

// Forward declaration of PchorASTVisitor
class PchorASTVisitor;

// Base class for all AST nodes
class PchorASTNode {
public:
    // Enum for declaration types
    enum class Decl : uint8_t { Index_Decl, Participant_Decl, Channel_Decl, Label_Decl, Global_Type_Decl };

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
    const std::string getName() const { return std::string(name); }


    // Accept a visitor
    void accept(PchorASTVisitor &visitor) const override;

protected:
    std::string_view name;

    // Constructor
    explicit DeclPchorASTNode(Decl declType, std::string_view name): PchorASTNode(declType), name(name) {}
};

class IndexASTNode : public DeclPchorASTNode {
public:
    explicit IndexASTNode(std::string_view name, Token& lower_token, Token& upper_token) : DeclPchorASTNode(Decl::Index_Decl, std::move(name)), lower(parseLiteral(lower_token.value)), upper(parseLiteral(upper_token.value)) {}
protected:
    const size_t lower;
    const size_t upper;
private:
    static size_t parseLiteral(std::string_view literal) {
        if (literal == "n") {
            return std::numeric_limits<size_t>::max(); // Unbounded
        }

        size_t value = 0;
        auto [ptr, ec] = std::from_chars(literal.data(), literal.data() + literal.size(), value);
        if (ec == std::errc::invalid_argument) {
            throw std::runtime_error("Invalid literal: " + std::string(literal));
        } else if (ec == std::errc::result_out_of_range) {
            throw std::runtime_error("Literal out of range: " + std::string(literal));
        }
        return value;
    }

};

class ParticipantASTNode : public DeclPchorASTNode {
public:
    explicit ParticipantASTNode(std::string_view name, std::shared_ptr<IndexASTNode> index)
        : DeclPchorASTNode(Decl::Participant_Decl, name), index(index) {}

    const std::shared_ptr<IndexASTNode>& getIndex() const { return index; }

protected:
    std::shared_ptr<IndexASTNode> index;

};

class ChannelASTNode : public DeclPchorASTNode {
    public:
        explicit ChannelASTNode(std::string_view name, std::shared_ptr<IndexASTNode> index)
            : DeclPchorASTNode(Decl::Participant_Decl, name), index(index) {}
    
        const std::shared_ptr<IndexASTNode>& getIndex() const { return index; }
    
    protected:
        std::shared_ptr<IndexASTNode> index;
    
    };
} // namespace PchorAST