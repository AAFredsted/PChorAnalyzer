#pragma once

#include <limits>
#include <memory> // For std::shared_ptr
#include <print>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "../parser/PchorTokenizer.hpp"

// ASTParser using visitor pattern
namespace PchorAST {

class AbstractPchorASTVisitor; // forward declaration of class

struct Token;

// Forward declaration of AbstractPchorASTVisitor
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
  ForEachExpr,
  IterExpr,
  ComExpr,
  SelectionExpr,
  AggregateExpr,
  ParticipantExpr,
  ChannelExpr,
  IndexExpr
};

//Arithmetic expression base.. Not visited and evaluated using it's own eval function

enum class ArithmeticExpr : uint8_t {
  Literal,
  Identifier,
  Addition,
  Subtraction
};

struct BaseArithmeticExpr {
  ArithmeticExpr exprType;
  BaseArithmeticExpr(ArithmeticExpr exprType): exprType(exprType) {}

  virtual ~BaseArithmeticExpr() = default;
  
  virtual std::string toString() const = 0;
  virtual void print() const = 0;
  virtual size_t eval(std::unordered_map<std::string, size_t>& ctx) const = 0;
};

struct LiteralExpr : public BaseArithmeticExpr {

  size_t value;
  explicit LiteralExpr(size_t v): BaseArithmeticExpr(ArithmeticExpr::Literal), value(v) {}
  ~LiteralExpr() = default;
  std::string toString() const override {
    return std::format("{}", value);
  }
  void print() const override {
    std::println("{}", this->toString());
  }
  size_t eval([[maybe_unused]] std::unordered_map<std::string, size_t>& ctx) const override { return value; }
};

struct IdentifierExpr: public BaseArithmeticExpr {
  std::string name;
  explicit IdentifierExpr(const std::string& name): BaseArithmeticExpr(ArithmeticExpr::Identifier), name(std::move(name)) {}
  explicit IdentifierExpr(const std::string_view& name): BaseArithmeticExpr(ArithmeticExpr::Identifier), name(std::string(name)) {}
  ~IdentifierExpr() = default;
  std::string toString() const override { return name; }
  void print() const override { std::println("{}", this->toString()); }

  size_t eval(std::unordered_map<std::string, size_t> &ctx) const override {
    if(!ctx.contains(this->name)){
      throw std::runtime_error(std::format("Arithmetic Expression Context does not provide value for identifier {}", this->name));
    }
    return ctx.at(this->name);
  }

};

struct BaseBinaryOpExpr : public BaseArithmeticExpr {
  std::unique_ptr<BaseArithmeticExpr> lhs;
  std::unique_ptr<BaseArithmeticExpr> rhs;

  BaseBinaryOpExpr(ArithmeticExpr type, std::unique_ptr<BaseArithmeticExpr> lhs, std::unique_ptr<BaseArithmeticExpr> rhs): BaseArithmeticExpr(type), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
  ~BaseBinaryOpExpr() = default;
  std::string toString() const override = 0;
  void print() const override = 0;
  size_t eval(std::unordered_map<std::string, size_t>& ctx) const override = 0;
  
};

struct AdditionExpr: public BaseBinaryOpExpr {

    AdditionExpr(std::unique_ptr<BaseArithmeticExpr> lhs, std::unique_ptr<BaseArithmeticExpr> rhs): BaseBinaryOpExpr(ArithmeticExpr::Addition, std::move(lhs), std::move(rhs)) {}
    ~AdditionExpr() = default;

    std::string toString() const override {
      return std::format("{} + {}", lhs->toString(), rhs->toString());
    }
    void print() const override {
      std::println("{}", this->toString());
    }
    size_t eval([[maybe_unused]] std::unordered_map<std::string, size_t>& ctx) const override {
      size_t l = lhs->eval(ctx);
      size_t r = rhs->eval(ctx);
      //assume both are below max
      if(std::numeric_limits<size_t>::max() - l < r){
        throw std::overflow_error(std::format("AdditionExpr: Size_t overflow for expression: {}", this->toString()));
      }
      return lhs->eval(ctx) + rhs->eval(ctx);
    }
};
struct SubstractionExpr: public BaseBinaryOpExpr {
    SubstractionExpr(std::unique_ptr<BaseArithmeticExpr> lhs, std::unique_ptr<BaseArithmeticExpr> rhs): BaseBinaryOpExpr(ArithmeticExpr::Subtraction, std::move(lhs), std::move(rhs)) {}
    ~SubstractionExpr() = default;

    std::string toString() const override {
      return std::format("{} - {}", lhs->toString(), rhs->toString());
    }
    void print() const override {
      std::println("{}", this->toString());
    }
    size_t eval([[maybe_unused]] std::unordered_map<std::string, size_t>& ctx) const override {
      size_t l = lhs->eval(ctx);
      size_t r = rhs->eval(ctx);
      //assume both are below max
      if(l < r){
        throw std::overflow_error(std::format("AdditionExpr: Size_t underflow for expression: {}", this->toString()));
      }
      return lhs->eval(ctx) - rhs->eval(ctx);
    }
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
  virtual void accept(AbstractPchorASTVisitor &visitor) const = 0;
  virtual void print() const = 0;
  virtual std::string toString() const = 0;

protected:
  std::string name;
  Decl decl;
  // Constructor
  explicit DeclPchorASTNode(Decl declType, std::string_view name)
      : name(std::string(name)), decl(declType) {}
};
//Base Class for ExpressionPchorASTNode
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
  virtual void accept(AbstractPchorASTVisitor &visitor) const = 0;
  virtual void print() const = 0;
  virtual std::string toString() const = 0;

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

  explicit IndexASTNode(const std::string &name, size_t lower, size_t upper)
      : DeclPchorASTNode(Decl::Index_Decl, std::move(name)), lower(lower),
        upper(upper) {}

  size_t getLower() const { return lower; }
  size_t getUpper() const { return upper; }

  void print() const override {
    std::println("{}", this->toString());
  }
  virtual std::string toString() const override  {
    return std::format("Index {} with lower bound:{}, upper bound: {}", name, lower,
                 upper);
  }

  void accept(AbstractPchorASTVisitor &visitor) const override;

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

  void accept(AbstractPchorASTVisitor &visitor) const override;

  void print() const override {
    std::println("{}", this->toString());
  }
  virtual std::string toString() const override  {
    return std::format("Participant {} indexed with {}", name, index->getName());
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

  void accept(AbstractPchorASTVisitor &visitor) const override;

  void print() const override {
    std::println("{}", this->toString());
  }
  virtual std::string toString() const override {
    return std::format("Channel {} indexed with {}", name, index->getName());
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

  void accept(AbstractPchorASTVisitor &visitor) const override;

  void print() const override {
    std::print("Label {} with labels: ", name);
    for (const std::string &i : labels) {
      std::print("{} ", i);
    }
    std::println("");
  }

  virtual std::string toString() const override  {
    return std::format("Label {}", name);
  }

protected:
  std::unordered_set<std::string> labels;
};

class IndexExpr : public ExprPchorASTNode {
public:
  explicit IndexExpr(std::shared_ptr<IndexASTNode> baseIndex, std::unique_ptr<BaseArithmeticExpr> literal, bool isLiteral)
      : ExprPchorASTNode(Expr::IndexExpr), baseIndex(baseIndex),
        literal(std::move(literal)), isLiteral(isLiteral){}

  explicit IndexExpr(std::shared_ptr<IndexASTNode> unaryIndex) : ExprPchorASTNode(Expr::IndexExpr){
    if(unaryIndex->getName() != "PchorUnaryIndex"){
      throw std::runtime_error(
        std::format("Only unary indexed types can be used with unary indeces. Instead got {}", unaryIndex->getName())
      );
    }
    baseIndex = std::move(unaryIndex);
    literal = std::make_unique<LiteralExpr>(1);
    isLiteral = true;
  }
  void accept(AbstractPchorASTVisitor &visitor) const override;

  void print() const override {
    std::println("{}", this->toString());
  }

  virtual std::string toString() const override  {
    return std::format("Index Expr with base {} and value: {}",
                  baseIndex->getName(), literal->toString());

  }
  std::string getName() const { return baseIndex->getName(); }
  bool isExprLiteral() const { return isLiteral; }
  size_t getLiteral(std::unordered_map<std::string, size_t> &ctx) const { return literal->eval(ctx); }

protected:
  std::shared_ptr<IndexASTNode> baseIndex;
  std::unique_ptr<BaseArithmeticExpr> literal;
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

  void accept(AbstractPchorASTVisitor &visitor) const override;

  void print() const override {
    std::print("Participant {} indexed with: ", baseParticipant->getName());
    index->print();
  }
  virtual std::string toString() const override  {
    return std::format("Participant {} indexed with: {}", baseParticipant->getName(), index->toString());
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

  void accept(AbstractPchorASTVisitor &visitor) const override;

  void print() const override {
    std::print("Channel {} indexed with: ", baseParticipant->getName());
    index->print();
  }
  virtual std::string toString() const override {
    return std::format("Channel {} indexed with: {}", baseParticipant->getName(), index->toString());
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

  void accept(AbstractPchorASTVisitor &visitor) const override;

  void print() const override {
    std::println("{}", this->toString());
  }

  virtual std::string toString() const override {
    return std::format("Communication Expression:\nSender: {}\nReceiver: {}\nChannel: {}\nDatatype: {}\n", sender->toString(), reciever->toString(), channel->toString(), dataType);
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

  virtual std::string toString() const override  {
    std::string str = "Expression List of: \n";
    for (const std::shared_ptr<ExprPchorASTNode> &expr : exprlist) {
      str.append(expr->toString());
    }
    return str;
  }

  void accept(AbstractPchorASTVisitor &visitor) const override;

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
//these exist but are not implemented
class ConExpr : public ExprPchorASTNode {
public:
  explicit ConExpr(const std::string &recVar,
                   std::shared_ptr<std::vector<IndexExpr>> indexContDomain)
      : ExprPchorASTNode(Expr::ConExpr), recVar(recVar),
        indexContDomain(indexContDomain) {}

  void accept(AbstractPchorASTVisitor &visitor) const override;

  void print() const override { std::println("We print recursive expr"); }
  virtual std::string toString() const override  {
    return "";
  }
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

  void accept(AbstractPchorASTVisitor &visitor) const override;

  void print() const override { std::println("We print recursive expr"); }
  virtual std::string toString() const  override {
    return "";
  }
protected:
  std::string recVar; // Ie X..
  std::shared_ptr<std::vector<IndexExpr>> indexDomain;
  std::shared_ptr<ExprList> body; // final element of list must be a
                                  // continuation
};

class IterExpr: public ExprPchorASTNode {
public:
    explicit IterExpr(std::shared_ptr<IndexASTNode> baseIndex, size_t min, size_t max, const std::string& identifier):
    ExprPchorASTNode(Expr::IterExpr), baseIndex(baseIndex), min(min), max(max), identifier(identifier) {}

    void accept(AbstractPchorASTVisitor& visitor) const override;

    void print() const override {
      std::println("{}", this->toString());
    }

    virtual std::string toString() const override {
      return std::format("Iteration Index with identifier {}, min {} and max {}\n", identifier, min, max);
    }

    std::shared_ptr<IndexASTNode> getBaseIndex() const {
      return baseIndex;
    }
    size_t getMin() const {
      return min;
    }
    size_t getMax() const {
      return max;
    }
    const std::string& getIdentifierRef() const {
      return identifier;
    }
private:
  std::shared_ptr<IndexASTNode> baseIndex;
  size_t min;
  size_t max;
  std::string identifier;

  
};
//recursive structure implemented through foreach
class ForEachExpr: public ExprPchorASTNode {
public:
  explicit ForEachExpr(std::shared_ptr<IterExpr> idxExpr, std::shared_ptr<ExprList> body):
  ExprPchorASTNode(Expr::ForEachExpr), idxExpr(std::move(idxExpr)), body(std::move(body)) {}

  void accept(AbstractPchorASTVisitor& visitor) const override;
  void print() const override {
    std::println("{}", this->toString());
  }

  virtual std::string toString() const override  {
    return std::format("forEach expression\n{}{}", idxExpr->toString(), body->toString());
  }

  std::shared_ptr<IterExpr> getIter() const {
    return idxExpr;
  }
  std::shared_ptr<ExprList> getBody() const {
    return body;
  }
protected:
//we need some min and max for the value as well as the identifier to replace with in the subexpressions
  std::shared_ptr<IterExpr> idxExpr;
  std::shared_ptr<ExprList> body;
};

class GlobalTypeASTNode : public DeclPchorASTNode {
public:
  explicit GlobalTypeASTNode(std::string_view name,
                             std::shared_ptr<ExprList> expr)
      : DeclPchorASTNode(Decl::Global_Type_Decl, name),
        expr_ptr(std::move(expr)) {}

  explicit GlobalTypeASTNode(std::string_view name)
      : DeclPchorASTNode(Decl::Global_Type_Decl, name) {}

  void accept(AbstractPchorASTVisitor &visitor) const override;

  void print() const override {
    std::println("{}", this->toString());
  }

  virtual std::string toString() const  override {
    return std::format("Global Type {} with expressions:\n{}", name, expr_ptr->toString());
  }

  std::shared_ptr<ExprList> getExprList() const { return expr_ptr; }

protected:
  std::shared_ptr<ExprList> expr_ptr;
};

} // namespace PchorAST