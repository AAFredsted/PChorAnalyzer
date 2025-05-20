#pragma once

#include <cstdint>
#include <format>
#include <print>
#include <string>
#include <unordered_set>
#include <memory>
#include <iterator>

#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/StmtIterator.h>

namespace PchorAST {

class CASTMapping;
class ProjectionList;

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
                       clang::Stmt::const_child_iterator &end,
                       AbstractProjection*& parentScopeProjectionPtr) = 0;

protected:
  ProjectionType type;
  std::unique_ptr<AbstractProjection> next;

  friend class ProjectionList;
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
                       clang::Stmt::const_child_iterator &end,
                       AbstractProjection*& parentScopeProjectionPtr) override = 0;

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
  virtual std::string toString() const override {
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
                            clang::Stmt::const_child_iterator &end,
                            AbstractProjection*& parentScopeProjectionPtr) override;

private:
};

class Precieve : public AbstractComProjection {
public:
  Precieve(const std::string &channelName, const std::string &typeName,
           std::size_t channelIndex)
      : AbstractComProjection(ProjectionType::Recieve, channelName, typeName,
                              channelIndex) {}
  ~Precieve() = default;

  virtual std::string toString() const override {
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
                            clang::Stmt::const_child_iterator &end,
                            AbstractProjection*& parentScopeProjectionPtr) override;

private:
};


class ProjectionList {
public:
  ProjectionList(): head(nullptr), tail(nullptr) {}
  ~ProjectionList() = default;
  
  ProjectionList(const ProjectionList& other) = delete;
  ProjectionList& operator=(const ProjectionList& other) = delete;
  
  ProjectionList(ProjectionList&& other) noexcept
      : head(std::move(other.head)), tail(other.tail) {
      other.tail = nullptr;
  }

  ProjectionList& operator=(ProjectionList&& other) noexcept {
      if (this != &other) {
          head = std::move(other.head);
          tail = other.tail;
          other.tail = nullptr;
      }
      return *this;
  }

  class Iterator {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = AbstractProjection;

    Iterator() : current(nullptr) {}
    Iterator(AbstractProjection* ptr) : current(ptr) {}

    //implement copy constructiona/assignment

    Iterator(const Iterator& other) : current(other.current) {}

    Iterator& operator=(const Iterator& other) {
      if(this != &other){
        current = other.current;
      }
      return *this;
    }
    Iterator(Iterator&& other) noexcept {
      current = std::move(other.current);
      other.current = nullptr;
    }

    Iterator& operator=(Iterator&& other) noexcept {
      if(this != &other){
        current = std::move(other.current);
        other.current = nullptr;
      }
      return *this;
    }

    Iterator& operator++() {
      if(current) {
        current = current->next.get();
      }
      return *this;
    }
    Iterator operator++(int)
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }
    AbstractProjection& operator*() const {
      return *current;
    }
    AbstractProjection* operator->() const {
      return current;
    }

    bool operator!=(const Iterator& other) const {
      return current != other.current;
    }
    bool operator==(const Iterator& other) const {
      return current == other.current;
    }
  
  private:
    AbstractProjection* current;
  };
  

  void appendBack(std::unique_ptr<AbstractProjection> proj){
      if(!head) {
        head = std::move(proj);
        tail = head.get();
      }
      else {
        tail->next = std::move(proj);
        tail = tail->next.get();
      }
  }

  bool empty() const { return head == nullptr; }

  Iterator begin() { return Iterator(head.get()); }
  Iterator end() { return Iterator(nullptr); }
  Iterator begin() const { return Iterator(head.get()); }
  Iterator end() const { return Iterator(nullptr); }
  AbstractProjection* front() {return head.get(); }
  AbstractProjection* back() { return tail; }

private:
  std::unique_ptr<AbstractProjection> head;
  AbstractProjection* tail;
};

static_assert(std::forward_iterator<ProjectionList::Iterator>);
// expand with further constructs down the line
} // namespace PchorAST
