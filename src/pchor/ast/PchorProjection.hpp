#pragma once

#include <cstdint>
#include <format>
#include <print>
#include <string>
#include <unordered_set>

#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/StmtIterator.h>



namespace PchorAST {

class CASTMapping;

enum class ProjectionType : uint8_t { Send, Recieve };

class AbstractProjection {
public:
  AbstractProjection(ProjectionType type) : type(type) {}
  virtual ~AbstractProjection() = default;
  virtual void print() const = 0;
  virtual std::string toString() const = 0;

  virtual bool isComProjection() const = 0;
  virtual std::string getTypeName() const = 0;
  virtual std::string getChannelName() const = 0;
  virtual size_t getChannelIndex() const = 0;

  ProjectionType getType() { return type; }

  virtual bool
  validateFunctionDecl(clang::ASTContext &context,
                       std::shared_ptr<PchorAST::CASTMapping> &CASTmap,
                       clang::Stmt::const_child_iterator &itr,
                       clang::Stmt::const_child_iterator &end) = 0;

protected:
  ProjectionType type;
};

class AbstractComProjection : public AbstractProjection {
public:
  AbstractComProjection(ProjectionType type, const std::string &channelName,
                        const std::string &typeName, std::size_t channelIndex)
      : AbstractProjection(type), channelName(channelName), typeName(typeName),
        channelIndex(channelIndex) {}

  bool isComProjection() const override { return true; }
  const std::string getChannelString() const {
    return std::format("{}[{}]", channelName, channelIndex);
  }
  std::string getTypeName() const override { return typeName; }
  std::string getChannelName() const override { return channelName; }
  size_t getChannelIndex() const override { return channelIndex; }
  virtual bool
  validateFunctionDecl(clang::ASTContext &context,
                       std::shared_ptr<PchorAST::CASTMapping> &CASTmap,
                       clang::Stmt::const_child_iterator &itr,
                       clang::Stmt::const_child_iterator &end) override = 0;

protected:
  std::string channelName;
  std::string typeName;
  std::size_t channelIndex;
};
class Psend : public AbstractComProjection {
public:
  Psend(const std::string &channelName, const std::string &typeName,
        std::size_t channelIndex)
      : AbstractComProjection(ProjectionType::Send, channelName, typeName,
                              channelIndex) {}
  ~Psend() = default;
  virtual std::string toString() const override  {
    return std::format("!{}[{}]<{}>.", this->channelName, this->channelIndex,
               this->typeName);
  }

  virtual void print() const override {
    std::print("!{}[{}]<{}>.", this->channelName, this->channelIndex,
               this->typeName);
  }

  bool validateFunctionDecl(clang::ASTContext &context,
                            std::shared_ptr<PchorAST::CASTMapping> &CASTmap,
                            clang::Stmt::const_child_iterator &itr,
                            clang::Stmt::const_child_iterator &end) override;

private:
};

class Precieve : public AbstractComProjection {
public:
  Precieve(const std::string &channelName, const std::string &typeName,
           std::size_t channelIndex)
      : AbstractComProjection(ProjectionType::Recieve, channelName, typeName,
                              channelIndex) {}
  ~Precieve() = default;

  virtual std::string toString() const override  {
    return std::format("?{}[{}]<{}>.", this->channelName, this->channelIndex,
               this->typeName);
  }

  virtual void print() const override {
    std::print("?{}[{}]<{}>.", this->channelName, this->channelIndex,
               this->typeName);
  }


  bool validateFunctionDecl(clang::ASTContext &context,
                            std::shared_ptr<PchorAST::CASTMapping> &CASTmap,
                            clang::Stmt::const_child_iterator &itr,
                            clang::Stmt::const_child_iterator &end) override;

private:
};

// expand with further constructs down the line
} // namespace PchorAST
