#pragma once

#include <cstdint>
#include <format>
#include <print>
#include <string>
#include <unordered_set>
#include <memory>
#include <iterator>

#include "PchorAST.hpp"


#include <clang/AST/ASTContext.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/StmtIterator.h>

namespace PchorAST {

class CASTMapping;
class ProjectionList;

enum class ProjectionType : uint8_t { Send, Recieve, ISend, IReceive };

class AbstractProjection {
public:
  AbstractProjection(ProjectionType type) : type(type) {}

  AbstractProjection(const AbstractProjection& other) 
    : type(other.type), next(other.next ? other.next->clone() : nullptr) {}

  AbstractProjection& operator=(const AbstractProjection& other) {
    if (this != &other) {
      type = other.type;
      next = other.next ? other.next->clone() : nullptr;
    }
    return *this;
  }

  AbstractProjection(AbstractProjection&& other) noexcept
    : type(std::move(other.type)), next(std::move(other.next)) {}

  AbstractProjection& operator=(AbstractProjection&& other) noexcept {
    if (this != &other) {
      type = std::move(other.type);
      next = std::move(other.next);
    }
    return *this;
  }

  virtual ~AbstractProjection() = default;
  virtual void print() const = 0;
  virtual std::string toString() const = 0;

  //clone method to avoid copy-move semantics, as unique_ptr is move only
  virtual std::unique_ptr<AbstractProjection> clone() const = 0;

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
                        const std::string &typeName)
      : AbstractProjection(type), channelName(channelName), typeName(typeName) {}

  AbstractComProjection(const AbstractComProjection& other)
      : AbstractProjection(other),  // copy base
        channelName(other.channelName),
        typeName(other.typeName) {}

  AbstractComProjection& operator=(const AbstractComProjection& other) {
    if (this != &other) {
      AbstractProjection::operator=(other);  // copy base
      channelName = other.channelName;
      typeName = other.typeName;
    }
    return *this;
  }

  AbstractComProjection(AbstractComProjection&& other) noexcept
      : AbstractProjection(std::move(other)),
        channelName(std::move(other.channelName)),
        typeName(std::move(other.typeName)) {}

  AbstractComProjection& operator=(AbstractComProjection&& other) noexcept {
    if (this != &other) {
      AbstractProjection::operator=(std::move(other));
      channelName = std::move(other.channelName);
      typeName = std::move(other.typeName);
    }
    return *this;
  }

  virtual ~AbstractComProjection() = default;
  

  //implemented in derivative class
  virtual void print() const override= 0;
  virtual std::string toString() const override = 0;
  virtual std::unique_ptr<AbstractProjection> clone() const override = 0;


  //implemented in this abstract class
  bool isComProjection() const override { return true; }
  std::string getTypeName() const override { return typeName; }
  std::string getChannelName() const override { return channelName; }
  size_t getChannelIndex() const override = 0;

  //added virtual functions
  virtual std::string getChannelString() const = 0;

  virtual bool
  validateFunctionDecl(clang::ASTContext &context,
                       std::shared_ptr<PchorAST::CASTMapping> &CASTmap,
                       clang::Stmt::const_child_iterator &itr,
                       clang::Stmt::const_child_iterator &end,
                       AbstractProjection*& parentScopeProjectionPtr) override = 0;


protected:
  std::string channelName;
  std::string typeName;
};
class Psend : public AbstractComProjection {
public:
  Psend(const std::string &channelName, const std::string &typeName,
        std::size_t channelIndex)
      : AbstractComProjection(ProjectionType::Send, channelName, typeName), channelIndex(channelIndex) {}
  Psend(const Psend&) = default;
  Psend(Psend&&) noexcept = default;
  Psend& operator=(const Psend&) = default;
  Psend& operator=(Psend&&) noexcept = default;
  ~Psend() = default;
  
  //cannot be overwritten virtually anymore
  std::unique_ptr<AbstractProjection> clone() const override {
    return std::make_unique<Psend>(*this); 
  }
  
  std::string toString() const override {
    return std::format("!{}[{}]<{}>.", this->channelName, this->channelIndex,
                       this->typeName);
  }

  virtual void print() const override {
    std::print("!{}[{}]<{}>.", this->channelName, this->channelIndex,
               this->typeName);
  }

  size_t getChannelIndex() const override {
    return channelIndex;
  }

  std::string getChannelString() const override {
    return std::format("{}[{}]", this->channelName, this->channelIndex);
  }


  bool validateFunctionDecl(clang::ASTContext &context,
                            std::shared_ptr<PchorAST::CASTMapping> &CASTmap,
                            clang::Stmt::const_child_iterator &itr,
                            clang::Stmt::const_child_iterator &end,
                            AbstractProjection*& parentScopeProjectionPtr) override;

private:
    size_t channelIndex;
};

class Preceive : public AbstractComProjection {
public:
  Preceive(const std::string &channelName, const std::string &typeName,
           std::size_t channelIndex)
      : AbstractComProjection(ProjectionType::Recieve, channelName, typeName), channelIndex(channelIndex) {}

  Preceive(const Preceive&) = default;
  Preceive(Preceive&&) noexcept = default;
  Preceive& operator=(const Preceive&) = default;
  Preceive& operator=(Preceive&&) noexcept = default;
  ~Preceive() = default;
  

  //cannot be overwritten virtually anymore
  std::unique_ptr<AbstractProjection> clone() const override {
    return std::make_unique<Preceive>(*this); 
  }

  std::string toString() const override {
    return std::format("?{}[{}]<{}>.", this->channelName, this->channelIndex,
                       this->typeName);
  }

  void print() const override {
    std::print("?{}[{}]<{}>.", this->channelName, this->channelIndex,
               this->typeName);
  }

  bool validateFunctionDecl(clang::ASTContext &context,
                            std::shared_ptr<PchorAST::CASTMapping> &CASTmap,
                            clang::Stmt::const_child_iterator &itr,
                            clang::Stmt::const_child_iterator &end,
                            AbstractProjection*& parentScopeProjectionPtr) override;
  size_t getChannelIndex() const override {
    return channelIndex;
  }

  std::string getChannelString() const override {
        return std::format("{}[{}]", this->channelName, this->channelIndex);
  }


private:
    size_t channelIndex;
};

class Ireceive: public AbstractComProjection {
public:
  Ireceive(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode) 
  : AbstractComProjection(ProjectionType::IReceive, channelName, typeName), indexNode(indexNode) {}

  Ireceive(const Ireceive&) = default;
  Ireceive(Ireceive&&) noexcept = default;
  Ireceive& operator=(const Ireceive&) = default;
  Ireceive& operator=(Ireceive&&) noexcept = default;
  ~Ireceive() = default;

  //cannot be overwritten virtually anymore
  std::unique_ptr<AbstractProjection> clone() const override {
    return std::make_unique<Ireceive>(*this); 
  }

  virtual std::string toString() const override {
    return std::format("?{}[{}]<{}>.", this->channelName, this->indexNode->getArithmeticExprString(),
                       this->typeName);
  }

  void print() const override {
    std::print("?{}[{}]<{}>.", this->channelName, this->indexNode->getArithmeticExprString(),
               this->typeName);
  }

  bool validateFunctionDecl(clang::ASTContext &context,
                            std::shared_ptr<PchorAST::CASTMapping> &CASTmap,
                            clang::Stmt::const_child_iterator &itr,
                            clang::Stmt::const_child_iterator &end,
                            AbstractProjection*& parentScopeProjectionPtr) override;
  size_t getChannelIndex() const override {
    if(indexNode->isExprLiteral()){
      auto temp =  std::unordered_map<std::string, size_t>{};
      return indexNode->getLiteral(temp);
    }
    else {
      std::println("[PchorProjection] warning: called getChannelIndex from Ireceive with non-literal index");
      return 1;
    }
  }

  std::string getChannelString() const override {
        return std::format("{}[{}]", this->channelName, this->indexNode->toString());
  }


private:
  std::shared_ptr<IndexExpr> indexNode;
};

class Isend: public AbstractComProjection {
public:
  Isend(const std::string& channelName, const std::string& typeName, std::shared_ptr<IndexExpr> indexNode) 
  : AbstractComProjection(ProjectionType::ISend, channelName, typeName), indexNode(indexNode) {}


  Isend(const Isend&) = default;
  Isend(Isend&&) noexcept = default;
  Isend& operator=(const Isend&) = default;
  Isend& operator=(Isend&&) noexcept = default;
  ~Isend() = default;


  //cannot be overwritten virtually anymore
  std::unique_ptr<AbstractProjection> clone() const override {
    return std::make_unique<Isend>(*this); 
  }

  std::string toString() const override {
    return std::format("!{}[{}]<{}>.", this->channelName, this->indexNode->getArithmeticExprString(),
                       this->typeName);
  }

  void print() const override {
    std::print("!{}[{}]<{}>.", this->channelName, this->indexNode->getArithmeticExprString(),
               this->typeName);
  }

  bool validateFunctionDecl(clang::ASTContext &context,
                            std::shared_ptr<PchorAST::CASTMapping> &CASTmap,
                            clang::Stmt::const_child_iterator &itr,
                            clang::Stmt::const_child_iterator &end,
                            AbstractProjection*& parentScopeProjectionPtr) override;
  size_t getChannelIndex() const override {
    if(indexNode->isExprLiteral()){
      auto temp =  std::unordered_map<std::string, size_t>{};
      return indexNode->getLiteral(temp);
    }
    else {
      std::println("[PchorProjection] warning: called getChannelIndex from Ireceive with non-literal index");
      return 1;
    }
  }

  std::string getChannelString() const override {
        return std::format("{}[{}]", this->channelName, this->indexNode->toString());
  }

private:
  std::shared_ptr<IndexExpr> indexNode;

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

  void appendCloneBack(std::shared_ptr<ProjectionList> projection) {
      if(projection->empty()){
        return;
      }
      std::unique_ptr<ProjectionList> projClone = projection->clone();

      if(!head) {
        head = std::move(projClone->head);
        tail = projClone->tail;
      }
      else {
        tail->next = std::move(projClone->head);
        tail = projClone->tail;
      }

  }

std::unique_ptr<ProjectionList> clone() const {
    auto clonedList = std::make_unique<ProjectionList>();

    if (head) {
        // Step 1: Deep copy via head's clone (recursively copies rest of list too)
        clonedList->head = head->clone();

        // Step 2: Walk to the end to set tail
        auto current = clonedList->head.get();
        while (current->next) {
            current = current->next.get();
        }
        clonedList->tail = current;
    }

    return clonedList;
  }


  void print() {
    std::print("ProjList of: ");
    for(const auto& elem: *this) {
      std::print("{}", elem.toString());
    }
    std::println("");
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
