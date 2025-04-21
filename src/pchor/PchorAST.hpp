#pragma once

#include <string>
#include <memory> // For std::shared_ptr
#include <limits>
#include <unordered_set>

#include "PchorTokenizer.hpp"

// ASTParser using visitor pattern
namespace PchorAST {

struct Token;

// Forward declaration of PchorASTVisitor
class PchorASTVisitor;


enum class Decl : uint8_t { Index_Decl, Participant_Decl, Channel_Decl, Label_Decl, Global_Type_Decl };
enum class Expr : uint8_t { ForeachExpr, RecExpr, ComExpr, SelectionExpr, AggregateExpr, ParticipantExpr, ChannelExpr };

// Base Class for Declaration Nodes
class DeclPchorASTNode {
public:
    virtual ~DeclPchorASTNode() = default;
    // Get the name of the declaration
    const std::string getName() const { return std::string(name); }
    Decl getDeclType() const { return decl; }
    // Accept a visitor
    virtual void accept(PchorASTVisitor &visitor) const = 0;

protected:
    std::string_view name;
    Decl decl;
    // Constructor
    explicit DeclPchorASTNode(Decl declType, std::string_view name): name(name), decl(declType) {}
};


class ExprPchorASTNode {
public:
    virtual ~ExprPchorASTNode() = default;
    virtual void accept(PchorASTVisitor &visitor) const = 0;
    Expr getExprType() const {return exprType; }

protected:
    Expr exprType;
    explicit ExprPchorASTNode(Expr exprType) : exprType(exprType) {}
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

class LabelASTNode: public DeclPchorASTNode {
public:
    explicit LabelASTNode(std::string_view name, std::unordered_set<std::string> labels)
        : DeclPchorASTNode(Decl::Label_Decl, name), labels(labels) {}

    const bool isLabel(const std::string& label){
        return labels.contains(label);
    }
protected:
    std::unordered_set<std::string> labels; 

};

class GlobalTypeASTNode: public DeclPchorASTNode {
public:
    explicit GlobalTypeASTNode(std::string_view name)
        : DeclPchorASTNode(Decl::Global_Type_Decl, name) {}

    void addExpr(std::shared_ptr<ExprPchorASTNode> expr) {
        expr_ptr = std::move(expr); // Assign the shared pointer directly
    }

protected:
    std::shared_ptr<ExprPchorASTNode>  expr_ptr;
};  


class PchorIndexExpr: public ExprPchorASTNode {
public:
    enum class Type { Literal, Variable, Derived }; //mapping to 1 or i

protected:
    Type type;
    size_t literal;
    std::string derivedValue; 
    std::string variableName;
    std::shared_ptr<IndexASTNode> baseIndex;

};


class IndexExpr: public ExprPchorASTNode {

    protected:
        size_t literal;
        std::string derivedValue;
        std::string variableName;
        std::shared_ptr<IndexASTNode> baseIndex;
};

class ParticipantExpr: public ExprPchorASTNode { 
public:
    ParticipantExpr(std::shared_ptr<ParticipantASTNode> baseParticipant, std::shared_ptr<IndexExpr> index = nullptr): ExprPchorASTNode(Expr::ParticipantExpr), baseParticipant(baseParticipant), index(index) {}
    const std::shared_ptr<ParticipantASTNode>& getBaseParticipant() const { return baseParticipant; }
    const std::shared_ptr<IndexExpr>& getIndex() const { return index; }

protected: 
    std::shared_ptr<ParticipantASTNode> baseParticipant;
    std::shared_ptr<IndexExpr> index;
};


class ChannelExpr:  public ExprPchorASTNode{
public:
    ChannelExpr(std::shared_ptr<ChannelASTNode> Participant, std::shared_ptr<IndexExpr> index = nullptr): ExprPchorASTNode(Expr::ChannelExpr), baseParticipant(baseParticipant), index(index) {}
    const std::shared_ptr<ChannelASTNode>& getBaseParticipant() const { return baseParticipant; }
    const std::shared_ptr<IndexExpr>& getIndex() const { return index; }
    
protected: 
    std::shared_ptr<ChannelASTNode> baseParticipant;
    std::shared_ptr<IndexExpr> index;
};


class CommunicationExpr: public ExprPchorASTNode {
public:


protected:
    //consists of sender, reciever, channel and type (and dependant expression if it exists)
    std::shared_ptr<ParticipantExpr> sender;
    std::shared_ptr<ParticipantExpr> reciever;
    std::shared_ptr<ChannelExpr> channel;
    std::string dataType;
    std::shared_ptr<ExprPchorASTNode> dependantExpr;

    explicit CommunicationExpr(std::shared_ptr<ParticipantExpr> sender, std::shared_ptr<ParticipantExpr> reciever, std::shared_ptr<ChannelExpr> channel, std::string dataType,  std::shared_ptr<ExprPchorASTNode> expression = nullptr ):
        ExprPchorASTNode(Expr::ComExpr), sender(std::move(sender)), reciever(std::move(reciever)), channel(std::move(channel)), dataType(dataType), dependantExpr(expression) {}
};

} // namespace PchorAST