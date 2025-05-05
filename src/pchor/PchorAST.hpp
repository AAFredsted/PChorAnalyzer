#pragma once

#include <limits>
#include <memory> // For std::shared_ptr
#include <print>
#include <string>
#include <unordered_set>
#include <vector>

#include "PchorTokenizer.hpp"

// ASTParser using visitor pattern
namespace PchorAST {

class CAST_PchorASTVisitor; // forward declaration of class

struct Token;

// Forward declaration of CAST_PchorASTVisitor
class ExprPchorASTNode;
class ExprList;

enum class Decl : uint8_t {
  Index_Decl,
  Participant_Decl,
  Channel_Decl,
  Label_Decl,
  Global_Type_Decl
};
enum class Expr : uint8_t {
  RecExpr,
  ConExpr,
  ComExpr,
  SelectionExpr,
  AggregateExpr,
  ParticipantExpr,
  ChannelExpr,
  IndexExpr
};

// Base Class for Declaration Nodes
class DeclPchorASTNode {
public:
  virtual ~DeclPchorASTNode() = default;
  // Delete copy constructor and copy assignment operator
  DeclPchorASTNode(const DeclPchorASTNode &) = delete;
  DeclPchorASTNode &operator=(const DeclPchorASTNode &) = delete;

  // Delete move constructor and move assignment operator
  DeclPchorASTNode(DeclPchorASTNode &&) = delete;
  DeclPchorASTNode &operator=(DeclPchorASTNode &&) = delete;

  // Get the name of the declaration
  const std::string getName() const { return std::string(name); }
  Decl getDeclType() const { return decl; }
  // Accept a visitor
  virtual void accept(CAST_PchorASTVisitor &visitor) const = 0;
  virtual void print() const = 0;

protected:
  std::string name;
  Decl decl;
  // Constructor
  explicit DeclPchorASTNode(Decl declType, std::string_view name)
      : name(std::string(name)), decl(declType) {}
};

class ExprPchorASTNode {
public:
  virtual ~ExprPchorASTNode() = default;

  // Delete copy constructor and copy assignment operator
  ExprPchorASTNode(const ExprPchorASTNode &) = delete;
  ExprPchorASTNode &operator=(const ExprPchorASTNode &) = delete;

  // Delete move constructor and move assignment operator
  ExprPchorASTNode(ExprPchorASTNode &&) = delete;
  ExprPchorASTNode &operator=(ExprPchorASTNode &&) = delete;

  Expr getExprType() const { return exprType; }
  virtual void accept(CAST_PchorASTVisitor &visitor) const = 0;
  virtual void print() const = 0;

protected:
  Expr exprType;
  explicit ExprPchorASTNode(Expr exprType) : exprType(exprType) {}
};

class IndexASTNode : public DeclPchorASTNode {
public:
  explicit IndexASTNode(std::string_view name, Token &lower_token,
                        Token &upper_token)
      : DeclPchorASTNode(Decl::Index_Decl, std::move(name)),
        lower(parseLiteral(lower_token.value)),
        upper(parseLiteral(upper_token.value)) {}
  size_t getLower() const { return lower; }
  size_t getUpper() const { return upper; }

  void print() const override {
    std::println("Index {} with lower bound:{}, upper bound: {}", name, lower,
                 upper);
  }

  void accept(CAST_PchorASTVisitor &visitor) const override;

  static size_t parseLiteral(std::string_view literal) {
    if (literal == "n") {
      return std::numeric_limits<size_t>::max(); // Unbounded
    }

    size_t value = 0;
    auto [ptr, ec] =
        std::from_chars(literal.data(), literal.data() + literal.size(), value);
    if (ec == std::errc::invalid_argument) {
      throw std::runtime_error("Invalid literal: " + std::string(literal));
    } else if (ec == std::errc::result_out_of_range) {
      throw std::runtime_error("Literal out of range: " + std::string(literal));
    }
    return value;
  }

protected:
  const size_t lower;
  const size_t upper;
};

class ParticipantASTNode : public DeclPchorASTNode {
public:
  explicit ParticipantASTNode(std::string_view name,
                              std::shared_ptr<IndexASTNode> index)
      : DeclPchorASTNode(Decl::Participant_Decl, name), index(index) {}

  const std::shared_ptr<IndexASTNode> &getIndex() const { return index; }

  void accept(CAST_PchorASTVisitor &visitor) const override;

  void print() const override {
    if (index == nullptr) {
      std::println("Participant {} indexed with 1", name);
    } else {
      std::println("Participant {} indexed with {}", name, index->getName());
    }
  }

protected:
  std::shared_ptr<IndexASTNode> index;
};

class ChannelASTNode : public DeclPchorASTNode {
public:
  explicit ChannelASTNode(std::string_view name,
                          std::shared_ptr<IndexASTNode> index)
      : DeclPchorASTNode(Decl::Channel_Decl, name), index(index) {}

  const std::shared_ptr<IndexASTNode> &getIndex() const { return index; }

  void accept(CAST_PchorASTVisitor &visitor) const override;

  void print() const override {
    std::println("Channel {} indexed with {}", name,
                 index ? index->getName() : "1");
  }

protected:
  std::shared_ptr<IndexASTNode> index;
};

class LabelASTNode : public DeclPchorASTNode {
public:
  explicit LabelASTNode(std::string_view name,
                        std::unordered_set<std::string> labels)
      : DeclPchorASTNode(Decl::Label_Decl, name), labels(labels) {}

  bool isLabel(const std::string &label) const {
    return labels.contains(label);
  }

  void accept(CAST_PchorASTVisitor &visitor) const override;

  void print() const override {
    std::print("Label {} with labels: ", name);
    for (const std::string &i : labels) {
      std::print("{} ", i);
    }
    std::println("");
  }

protected:
  std::unordered_set<std::string> labels;
};

class IndexExpr : public ExprPchorASTNode {
public:
  explicit IndexExpr(std::shared_ptr<IndexASTNode> baseIndex, size_t literal)
      : ExprPchorASTNode(Expr::IndexExpr), baseIndex(baseIndex),
        literal(literal), variableName(""), isLiteral(true) {}
  explicit IndexExpr(std::shared_ptr<IndexASTNode> baseIndex,
                     const std::string &variableName)
      : ExprPchorASTNode(Expr::IndexExpr), baseIndex(baseIndex), literal(),
        variableName(variableName), isLiteral(false) {}
  explicit IndexExpr(std::shared_ptr<IndexASTNode> baseIndex,
                     std::string_view variableName)
      : ExprPchorASTNode(Expr::IndexExpr), baseIndex(baseIndex), literal(),
        variableName(std::string(variableName)), isLiteral(false) {}

  void accept(CAST_PchorASTVisitor &visitor) const override;

  void print() const override {
    if (isLiteral) {
      std::println("Index Expr with base {} and value: {}",
                   baseIndex->getName(), literal);
    } else {
      std::println("Index Expr with base {} and value: {}",
                   baseIndex->getName(), variableName);
    }
  }
  std::string getName() const { return baseIndex->getName(); }
  bool isExprLiteral() const { return isLiteral; }

protected:
  std::shared_ptr<IndexASTNode> baseIndex;
  size_t literal;
  std::string variableName;
  bool isLiteral;
};

class ParticipantExpr : public ExprPchorASTNode {
public:
  ParticipantExpr(std::shared_ptr<ParticipantASTNode> baseParticipant,
                  std::shared_ptr<IndexExpr> index = nullptr)
      : ExprPchorASTNode(Expr::ParticipantExpr),
        baseParticipant(baseParticipant), index(index) {}
  const std::shared_ptr<ParticipantASTNode> &getBaseParticipant() const {
    return baseParticipant;
  }
  const std::shared_ptr<IndexExpr> &getIndex() const { return index; }

  void accept(CAST_PchorASTVisitor &visitor) const override;

  void print() const override {
    std::print("Participant {} indexed with: ", baseParticipant->getName());
    if (index) {
      index->print();
    } else {
      std::println("1");
    }
  }

protected:
  std::shared_ptr<ParticipantASTNode> baseParticipant;
  std::shared_ptr<IndexExpr> index;
};

class ChannelExpr : public ExprPchorASTNode {
public:
  ChannelExpr(std::shared_ptr<ChannelASTNode> Participant,
              std::shared_ptr<IndexExpr> index = nullptr)
      : ExprPchorASTNode(Expr::ChannelExpr), baseParticipant(Participant),
        index(index) {}
  const std::shared_ptr<ChannelASTNode> &getBaseParticipant() const {
    return baseParticipant;
  }
  const std::shared_ptr<IndexExpr> &getIndex() const { return index; }

  void accept(CAST_PchorASTVisitor &visitor) const override;

  void print() const override {
    std::print("Channel {} indexed with: ", baseParticipant->getName());
    if (index) {
      index->print();
    } else {
      std::println("1");
    }
  }

protected:
  std::shared_ptr<ChannelASTNode> baseParticipant;
  std::shared_ptr<IndexExpr> index;
};

class CommunicationExpr : public ExprPchorASTNode {
public:
  explicit CommunicationExpr(
      std::shared_ptr<ParticipantExpr> sender,
      std::shared_ptr<ParticipantExpr> reciever,
      std::shared_ptr<ChannelExpr> channel, std::string dataType,
      std::shared_ptr<ExprPchorASTNode> expression = nullptr)
      : ExprPchorASTNode(Expr::ComExpr), sender(std::move(sender)),
        reciever(std::move(reciever)), channel(std::move(channel)),
        dataType(dataType), dependantExpr(expression) {}

  void accept(CAST_PchorASTVisitor &visitor) const override;

  void print() const override {
    std::println("Communication Expression:");
    std::print("Sender: ");
    sender->print();
    std::print("Reciever: ");
    reciever->print();
    std::print("Channel: ");
    channel->print();
    std::println("Datatype: {}", dataType);
  }

  std::string getDataType() const { return dataType; }
  std::shared_ptr<ParticipantExpr> getSender() const { return sender; }
  std::shared_ptr<ParticipantExpr> getReciever() const { return reciever; }
  std::shared_ptr<ChannelExpr> getChannel() const { return channel; }

protected:
  // consists of sender, reciever, channel and type (and dependant expression if
  // it exists)
  std::shared_ptr<ParticipantExpr> sender;
  std::shared_ptr<ParticipantExpr> reciever;
  std::shared_ptr<ChannelExpr> channel;
  std::string dataType;
  std::shared_ptr<ExprPchorASTNode> dependantExpr;
};

class ExprList : public ExprPchorASTNode {
public:
  explicit ExprList() : ExprPchorASTNode(Expr::AggregateExpr), exprlist() {}

  void addExpr(std::shared_ptr<ExprPchorASTNode> expr) {
    exprlist.emplace_back(expr);
  }

  void print() const override {
    std::println("Expression List of: ");
    for (const std::shared_ptr<ExprPchorASTNode> &expr : exprlist) {
      expr->print();
    }
  }

  void accept(CAST_PchorASTVisitor &visitor) const override;

  std::vector<std::shared_ptr<ExprPchorASTNode>>::iterator begin() {
    return exprlist.begin();
  }
  std::vector<std::shared_ptr<ExprPchorASTNode>>::iterator end() {
    return exprlist.end();
  }
  std::vector<std::shared_ptr<ExprPchorASTNode>>::const_iterator begin() const {
    return exprlist.cbegin();
  }
  std::vector<std::shared_ptr<ExprPchorASTNode>>::const_iterator end() const {
    return exprlist.cend();
  }

protected:
  std::vector<std::shared_ptr<ExprPchorASTNode>> exprlist;
};
/*
Recexpr is defined as a initial state X(index list)
and substitution for X, which is a Expr-list, which must end with a
Continuation-Expr
*/

class ConExpr : public ExprPchorASTNode {
public:
  explicit ConExpr(const std::string &recVar,
                   std::shared_ptr<std::vector<IndexExpr>> indexContDomain)
      : ExprPchorASTNode(Expr::ConExpr), recVar(recVar),
        indexContDomain(indexContDomain) {}

  void accept(CAST_PchorASTVisitor &visitor) const override;

  void print() const override { std::println("We print recursive expr"); }

protected:
  std::string recVar;
  std::shared_ptr<std::vector<IndexExpr>> indexContDomain;
};

class RecExpr : public ExprPchorASTNode {
public:
  explicit RecExpr(const std::string &recVar,
                   std::shared_ptr<std::vector<IndexExpr>> indexDomain,
                   std::shared_ptr<ExprList> body)
      : ExprPchorASTNode(Expr::RecExpr), recVar(recVar),
        indexDomain(std::move(indexDomain)), body(std::move(body)) {}

  void accept(CAST_PchorASTVisitor &visitor) const override;

  void print() const override { std::println("We print recursive expr"); }

protected:
  std::string recVar; // Ie X..
  std::shared_ptr<std::vector<IndexExpr>> indexDomain;
  std::shared_ptr<ExprList> body; // final element of list must be a
                                  // continuation
};

class GlobalTypeASTNode : public DeclPchorASTNode {
public:
  explicit GlobalTypeASTNode(std::string_view name,
                             std::shared_ptr<ExprList> expr)
      : DeclPchorASTNode(Decl::Global_Type_Decl, name),
        expr_ptr(std::move(expr)) {}

  explicit GlobalTypeASTNode(std::string_view name)
      : DeclPchorASTNode(Decl::Global_Type_Decl, name) {}

  void accept(CAST_PchorASTVisitor &visitor) const override;

  void print() const override {
    std::println("Global Type {} with expressions:", name);
    expr_ptr->print();
  }
  std::shared_ptr<ExprList> getExprList() const { return expr_ptr; }

protected:
  std::shared_ptr<ExprList> expr_ptr;
};

} // namespace PchorAST