#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>

#include "../../pchor/ast/PchorProjection.hpp"
#include "CASTAnalyzerUtils.hpp"

namespace PchorAST {
enum class ContextType { Decl, Stmt };

struct Context {
  ContextType type;
  std::variant<const clang::Decl *, const clang::Stmt *> value;

  // Delete default constructor
  Context() = delete;

  // Delete copy constructor and copy assignment operator
  Context(const Context &other) : type(other.type), value(other.value) {}
  Context &operator=(const Context &other) {
    if (this != &other) {
      type = other.type;
      value = other.value;
    }
    return *this;
  }

  // Move constructor
  Context(Context &&other) noexcept
      : type(other.type), value(std::move(other.value)) {}

  // Move assignment operator
  Context &operator=(Context &&other) noexcept {
    if (this != &other) {
      type = other.type;
      value = std::move(other.value);
    }
    return *this;
  }

  // Constructor for Decl
  explicit Context(const clang::Decl *decl)
      : type(ContextType::Decl), value(decl) {}

  // Constructor for Stmt
  explicit Context(const clang::Stmt *stmt)
      : type(ContextType::Stmt), value(stmt) {}

  ~Context() = default;

  // Get the type of the context
  ContextType getType() const { return type; }

  // Get the Decl
  const clang::Decl *getDecl() const {
    return std::get<const clang::Decl *>(value);
  }

  // Get the Stmt
  const clang::Stmt *getStmt() const {
    return std::get<const clang::Stmt *>(value);
  }
};

class CASTMapping {
public:
  CASTMapping() : map() {}

  CASTMapping(const CASTMapping &other) { this->map = other.map; }
  CASTMapping &operator=(const CASTMapping &other) {
    if (this != &other) {
      this->map = other.map;
    }
    return *this;
  }
  CASTMapping(CASTMapping &&other) noexcept {
    this->map = std::move(other.map);
    other.map.clear();
  }
  CASTMapping &operator=(CASTMapping &&other) noexcept {
    if (this != &other) {
      this->map = std::move(other.map);
      other.map.clear();
    }
    return *this;
  }

  void addMapping(const std::string &name, const clang::Decl *decl) {
    map.emplace(name, Context(decl));
  }

  void addMapping(const std::string &name, const clang::Stmt *stmt) {
    map.emplace(name, Context(stmt));
  }

  template <typename NodeType> NodeType getMapping(const std::string &name) {
    auto it = map.find(name);
    if (it != map.end()) {
      // Try to get the requested type from the variant
      if (auto *result = std::get_if<NodeType>(&it->second.value)) {
        return *result;
      }
    }
    return nullptr;
  }
  void printMappings() const {
    std::println("\n\nPChorAST CAST Mappings\n------------------");
    for (const auto &[key, value] : map) {
      std::println("Mapping for {}", key);
      AnalyzerUtils::printDecl(value.getDecl());
    }
  }

private:
  std::unordered_map<std::string, Context> map;
};

struct ParticipantKey {
  std::string name;
  size_t index;

  ParticipantKey(const std::string &name, size_t index)
      : name(name), index(index) {}

  ParticipantKey(const ParticipantKey &other) {
    this->name = other.name;
    this->index = other.index;
  }
  ParticipantKey &operator=(const ParticipantKey &other) {
    if (this != &other) {
      this->name = other.name;
      this->index = other.index;
    }
    return *this;
  }

  std::string toString() const { return std::format("{}[{}]", name, index); }
  ParticipantKey(ParticipantKey &&other) = delete;
  ParticipantKey &operator=(ParticipantKey &&other) = delete;

  ~ParticipantKey() = default;

  bool operator==(const ParticipantKey &other) const {
    return name == other.name && index == other.index;
  }
};

struct ParticipantKeyHash {
  size_t operator()(const ParticipantKey &key) const {
    size_t h1 = std::hash<std::string>{}(key.name);
    size_t h2 = std::hash<size_t>{}(key.index);
    return h1 ^ (h2 << 1);
  }
};

class PchorProjection {
public:
  PchorProjection() : projectionMap() {}

  PchorProjection(const PchorProjection &other) = delete;
  PchorProjection &operator=(const PchorProjection &other) = delete;

  PchorProjection(PchorProjection &&other) noexcept
      : projectionMap(std::move(other.projectionMap)) {
    other.projectionMap.clear();
  }
  PchorProjection &operator=(PchorProjection &&other) noexcept {
    if (this != &other) {
      projectionMap = std::move(other.projectionMap);
      other.projectionMap.clear();
    }
    return *this;
  }
  ~PchorProjection() = default;
  void addParticipant(const ParticipantKey &participantName) {
    projectionMap.emplace(
        participantName,
        ProjectionList{});
  }

  void addProjection(const ParticipantKey &key,
                     std::unique_ptr<PchorAST::AbstractProjection> proj) {
    projectionMap[key].appendBack(std::move(proj));
    
  }


  bool hasProjection(const ParticipantKey &key) const {
    return projectionMap.contains(key);
  }
  void printProjections() const {
    std::println("\n\nPchorAST Participant "
                 "projections:\n-------------------------------");
    for (const auto &[elem, value] : projectionMap) {
      std::print("Projection for participant {}: ", elem.toString());
      for (const auto &projection : value) {
        projection.print();
      }
      std::println(" ");
    }
  }

  auto begin() { return projectionMap.begin(); }
  auto end() { return projectionMap.end(); }
  auto begin() const { return projectionMap.begin(); }
  auto end() const { return projectionMap.end(); }

private:
  std::unordered_map<ParticipantKey,
                      ProjectionList,
                     ParticipantKeyHash>
      projectionMap;
};

} // namespace PchorAST