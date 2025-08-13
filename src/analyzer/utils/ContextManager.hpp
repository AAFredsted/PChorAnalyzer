#pragma once
#include <variant>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <compare>
#include <stdexcept>
#include <format> 

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


/*
new implementation of keysystem
*/

/*
Enum Classes
The equivalence class projections are valid over a finite number of ranges over I=[l,n],
which can be identified using K=|l| and c=lower((k-l)/2)+l.
Furthermore, we differentiate between eq classes that are valid for even and odd K.
Thereby, we get the following
*/

enum class EvenCase: u_int8_t {
  Both,
  Even,
  Odd
};

/*
All ranges can be specified by this system, being:
[L,L], (L, lC), [L, C], (L, lC], [lC,lC], [C,C], (c,n), (c,n], [n,n]

*/
enum class RangeSymbol: u_int8_t {
  L = 0,
  lC = 1,
  C = 2,
  N = 3
};


struct Bound {
  RangeSymbol symbol;
  bool inclusive;
  bool lhs;

  explicit Bound() : symbol(RangeSymbol::C), inclusive(true), lhs(true) {}

  Bound(RangeSymbol symbol, bool inclusive, bool lhs) : symbol(symbol), inclusive(inclusive), lhs(lhs) {} 

  bool isMinInclusive() {
    return lhs && inclusive && symbol == RangeSymbol::L;
  }
  bool isMaxInclusive() {
    return !lhs && inclusive && symbol == RangeSymbol::N;
  }
  Bound(const Bound& other) {
    symbol = other.symbol;
    inclusive = other.inclusive;
    lhs = other.lhs;
  }
  Bound& operator=(const Bound& other) {
    if(this != &other) {
      symbol = other.symbol;
      inclusive = other.inclusive;
      lhs = other.lhs;
    }
    return *this;
  }
  Bound(Bound&& other) = delete;
  Bound& operator=(Bound&& other) = delete;
  ~Bound()= default;

  std::string toString() const {

    switch(symbol) {
      case RangeSymbol::L:
        return lhs ? (inclusive? "[l" : "(l" ) : (inclusive? "l]": "l)");
      case RangeSymbol::lC:
        return lhs ? (inclusive? "[c-1" : "(c-1" ) : (inclusive? "c-1]": "c-1)");
      case RangeSymbol::C:
        return lhs ? (inclusive? "[c" : "(c" ) : (inclusive? "c]": "c)");
      case RangeSymbol::N: 
        return lhs ? (inclusive? "[n" : "(n" ) : (inclusive? "n]": "n)");
      default:
        throw std::runtime_error("[] Unknown RangeSymbol Received");
    }
  }

  bool operator==(const Bound& other) const {
    return symbol == other.symbol && inclusive == other.inclusive;
  }

  int normalize() const noexcept {
    if(lhs) {
      return static_cast<int>(this->symbol) + (this->inclusive ? 0 : 1); 
    }
    else {
      return static_cast<int>(this->symbol) + (this->inclusive? 0 : -1);
    }
  }

  std::strong_ordering operator<=>(const Bound& other) const {
    int a = normalize();
    int b = other.normalize();
    return a <=> b;
  }


};


struct Range {
  Bound lower;
  Bound upper;

  explicit Range(const Bound& lower, const Bound& upper): lower(lower), upper(upper) {}

  Range(const Range& other) {
    lower = other.lower;
    upper = other.upper;
  }
  Range& operator=(const Range& other) {
    if(this != &other) {
      lower = other.lower;
      upper = other.upper;
    }
    return *this;
  }
  Range(Range&&) = delete;
  Range& operator=(Range&&) = delete;

  ~Range() = default;

  bool isMinInclusive() {
    return lower.isMinInclusive();
  }
  bool isMaxInclusive() {
    return upper.isMaxInclusive();
  }

  void changeLower(const Bound& nlower) {
    if(nlower.lhs) {
      this->lower = nlower;
    }
  }
  void changeUpper(const Bound& nupper) {
    if(!nupper.lhs){
      this->upper = nupper;
    }
  }
 
  std::string toString() const {
    return std::format("{},{}", lower.toString(), upper.toString());
  }

  bool operator==(const Range& other) const {
    return lower == other.lower && upper == other.upper;
  }
  bool overlaps(const Range& other) const noexcept {
    return !((upper < other.lower) || (lower > other.upper));
  }
  Range intersection(const Range& other) const {
    if(!overlaps(other)){
      throw std::runtime_error(std::format("Ranges do not overlap: {} {}", this->toString(), other.toString()));
    }
    //i know it looks weird, but I don't want to write default constructors, and the compiler complains if write it differently
    return Range{(lower <= other.lower)? other.lower: lower, (upper < other.upper)? upper: other.upper};
  }
  // true if left difference exists, otherwise false
  bool isLeftDifference(const Range& other) const {
    if (!overlaps(other)) {
      return false;
    }
    return lower < other.lower;
  }

  // Returns the left part of this range not in other (returns default if none)
  Range leftDifference(const Range& other) const {
    if (!overlaps(other)) {
      return Range(lower, upper);
    }
    if (lower < other.lower) {
      Bound newUpper = Bound(other.lower.symbol, !other.lower.inclusive, !other.lower.lhs);
      return Range(lower, newUpper);
    }

    throw std::runtime_error(std::format("Invalid Ranges Received, no right difference for {} without {}",this->toString(), other.toString()));
  }

  // returns true if rightdifference exists, otherwise false
  bool isRightDifference(const Range& other) const {
    if (!overlaps(other)) {
      return false;
    }
    return other.upper < upper;
  }

  // Returns the right part of this range not in other (returns default if none)
  Range rightDifference(const Range& other) const {
    if (!overlaps(other)) {
      return Range(lower, upper);
    }
    if (other.upper < upper) {
      Bound newLower = Bound(other.upper.symbol, !other.upper.inclusive, !other.upper.lhs);
      return Range(newLower, upper);
    }
    throw std::runtime_error(std::format("Invalid Ranges Received, no right difference for {} without {}",this->toString(), other.toString()));
  }
};
/*
We use Variant to have a relatively simple way to deal with multiple keys in one set.
*/
struct ParticipantKey {
  std::string name;
  std::unique_ptr<size_t> index;
  std::unique_ptr<Range> range;
  std::unique_ptr<EvenCase> even;

  // Name only
  explicit ParticipantKey(const std::string& name)
    : name(name), index(nullptr), range(nullptr), even(nullptr) {}


  // Name + index
  ParticipantKey(const std::string& name, size_t idx)
    : name(name), index(std::make_unique<size_t>(idx)), range(nullptr), even(nullptr) {}

  // Name + range + even
  ParticipantKey(const std::string& name, const Range& rng, EvenCase ev)
    : name(name), index(nullptr), range(std::make_unique<Range>(rng)), even(std::make_unique<EvenCase>(ev)) {}

  // Name + index + range + even
  ParticipantKey(const std::string& name, size_t idx, const Range& rng, EvenCase ev)
    : name(name), index(std::make_unique<size_t>(idx)), range(std::make_unique<Range>(rng)), even(std::make_unique<EvenCase>(ev)) {}

      ParticipantKey(const ParticipantKey& other)
    : name(other.name),
      index(other.index ? std::make_unique<size_t>(*other.index) : nullptr),
      range(other.range ? std::make_unique<Range>(*other.range) : nullptr),
      even(other.even ? std::make_unique<EvenCase>(*other.even) : nullptr) {}

  ParticipantKey& operator=(const ParticipantKey& other) {
    if (this != &other) {
      name = other.name;
      index = other.index ? std::make_unique<size_t>(*other.index) : nullptr;
      range = other.range ? std::make_unique<Range>(*other.range) : nullptr;
      even = other.even ? std::make_unique<EvenCase>(*other.even) : nullptr;
    }
    return *this;
  }

  ParticipantKey(ParticipantKey&& other) = delete;
  ParticipantKey& operator=(ParticipantKey&& other) = delete;

  ~ParticipantKey() = default;


  bool isMinInclusive() const {
    if(!range){
      return false;
    }
    return range->isMinInclusive();
  }
  bool isMaxInclusive() const {
    if(!range){
      return false;
    }
    return range->isMaxInclusive();
  }
  bool overlaps(const Range& other) const noexcept {
    if(!range) {
      return false;
    }
    return range->overlaps(other);
  }
  Range intersection(const Range& other) const {
    if(!range) {
      throw std::runtime_error(std::format("intersection not defined for key {}", this->toString()));
    }
    return range->intersection(other);
  }

  std::string toString() const {
    constexpr size_t n = std::numeric_limits<size_t>::max();
    constexpr size_t nl = n-1; 

    std::string result = name + "{";
    if (index) {
      if(*index == n){
        result += "[n]";
      }
      else if(*index == nl) {
        result += "[n-1]";
      }
      else {
        result += std::format("[{}]", *index);
      }

    }
    if (range) {
      result += std::format("{}", range->toString());
    }
    if (even) {
      switch(*even) {
        case EvenCase::Both :
          result += "B";
          break;
        case EvenCase::Even :
          result += "E";
          break;
        case EvenCase::Odd :
          result += "O";
          break; 
      }
      
    }
    return result + "}";
  }

  bool operator==(const ParticipantKey& other) const {
    if (name != other.name) return false;
    if (static_cast<bool>(index) != static_cast<bool>(other.index)) return false;
    if (index && other.index && *index != *other.index) return false;
    if (static_cast<bool>(range) != static_cast<bool>(other.range)) return false;
    if (range && other.range && !(*range == *other.range)) return false;
    if (static_cast<bool>(even) != static_cast<bool>(other.even)) return false;
    if (even && other.even && *even != *other.even) return false;
    return true;
  }
};

struct ParticipantKeyHash {
  size_t operator()(const ParticipantKey& key) const {
    size_t h = std::hash<std::string>{}(key.name);
    if (key.index) {
      h ^= std::hash<size_t>{}(*key.index) << 1;
    }
    if (key.range) {
      h ^= std::hash<int>{}(static_cast<int>(key.range->lower.symbol)) << 2;
      h ^= std::hash<bool>{}(key.range->lower.inclusive) << 3;
      h ^= std::hash<bool>{}(key.range->lower.lhs) << 4;
      h ^= std::hash<int>{}(static_cast<int>(key.range->upper.symbol)) << 5;
      h ^= std::hash<bool>{}(key.range->upper.inclusive) << 6;
      h ^= std::hash<bool>{}(key.range->upper.lhs) << 7;
    }
    if (key.even) {
      h ^= std::hash<int>{}(static_cast<int>(*key.even)) << 8;
    }
    return h;
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

  void addParticipantGroup(const std::string& name) {
    if(!projectionMap.contains(name)) {
      projectionMap.emplace(
        name,
        std::unordered_map<ParticipantKey, 
                          std::shared_ptr<ProjectionList>,
                          ParticipantKeyHash>{});
    }
  }

  bool hasParticipantGroup(const std::string& name) {
    return projectionMap.contains(name);
  }

  bool hasParticipant(const ParticipantKey &key) const {
    return projectionMap.contains(key.name) && projectionMap.at(key.name).contains(key);
  }

  void addParticipant(const ParticipantKey &participantName) {
    if(!projectionMap.contains(participantName.name)) {
      projectionMap.emplace(
        participantName.name,
        std::unordered_map<ParticipantKey, 
                          std::shared_ptr<ProjectionList>,
                          ParticipantKeyHash>{});
    }
    projectionMap[participantName.name].emplace(
        participantName,
        std::make_shared<ProjectionList>());
  }

  void addProjection(const ParticipantKey &key,
                     std::unique_ptr<PchorAST::AbstractProjection> proj) {
    projectionMap[key.name][key]->appendBack(std::move(proj));
    
  }

  void appendCloneProjection(const ParticipantKey &key, std::shared_ptr<ProjectionList> proj) {
    projectionMap.at(key.name).at(key)->appendCloneBack(proj);
  }


  bool hasMatchingRangeWithLiteral(const ParticipantKey &key) const {
    //check if literal range exists
    for(const auto& [iKey, iProj]: this->projectionMap.at(key.name)) {
      //we check only if perfect range exists
      if(iKey.index && iKey.range) {
        if(*(iKey.index) == *(key.index)) {
          return true;
        }
      } 
    }
    return false;


  }
  bool hasMathingRangeToLiteral(const ParticipantKey &key, size_t l, size_t n) const {
      size_t i = *(key.index);
      
      for(const auto& [iKey, iProj]: projectionMap.at(key.name)) {
        if(iKey.range) {
          if(i == l && iKey.isMinInclusive()) {
            return true;
          }
          else if(i == n && iKey.isMaxInclusive()) {
            return true;
          }
          else if(i != l && i != n) {
            // all inbetween ranges match
            return true;
          }

        }

      }
      return false;
  }
  void addLiteralProjection(const ParticipantKey &key,
                     std::unique_ptr<PchorAST::AbstractProjection> proj, size_t l, size_t n) {
    
    //we only need it to work for max and min, but cases


    //1: we do not exist, and no other version like us exists, so we add ourselves anew
    //2: find ranged version of us without literal, so we generate version of us
    //3: we already exist, so we just add
    //4: find ranged version with literal, just add

      //first, check if any matches with ranges exist

      //then, check if any only ranges exist

      //add directly if exists
      if(this->hasParticipant(key)){
        addProjection(key, std::move(proj));
      }
      //matching range and literal
      else if(hasMatchingRangeWithLiteral(key)) {
        for(const auto& [iKey, iProj]: projectionMap.at(key.name)) {
          if(iKey.index && *(iKey.index) == *(key.index)){
            addProjection(iKey, proj->clone());
          }
        }
      }
      else if(hasMathingRangeToLiteral(key, l, n)) {
        //case where we need to gen I for entire range 0: ohh nooo
        std::vector<ParticipantKey> keysToRemove;
        std::vector<std::pair<ParticipantKey, std::shared_ptr<ProjectionList>>> keysToInsert;
        size_t i = *(key.index);

        //(l,n)
        Range lnRange{Bound(RangeSymbol::L, false, true), Bound(RangeSymbol::N, false, false)};
        for(const auto& [iKey, iProj]: projectionMap.at(key.name)) {
            //basically, we map entire range, except for max and min, or we only map max or min,
            //then, we remove old one
            //finish this tomorrow and make hella sick example, then do presentation
            if(*(key.index) == l && !iKey.index && iKey.isMinInclusive()) {
              //okay.. so we have P{[l]} and P{[l,Bound, both}
              //generate 2 keys
              //P{[l], [l,l], both} and P{(l,Bound, both}
              

              //insert the two and remove the old one 

              Range lr{Bound{RangeSymbol::L, true, true}, Bound{RangeSymbol::L, true, false}};
             

              ParticipantKey lKey{key.name, i, lr, EvenCase::Both};
              std::shared_ptr<ProjectionList> lProj = std::make_shared<ProjectionList>();
              lProj->appendCloneBack(iProj);
              lProj->appendBack(proj->clone());
              keysToInsert.emplace_back(lKey, lProj);


              if(iKey.range->isRightDifference(lr)){
                Range cr = iKey.range->rightDifference(lr);
                ParticipantKey lcKey{iKey.name, cr, *(iKey.even)};
                std::shared_ptr<ProjectionList> lcProj = std::make_shared<ProjectionList>();
                lcProj->appendCloneBack(iProj);
                keysToInsert.emplace_back(lcKey, lcProj);
              }
            
              keysToRemove.push_back(iKey);              
            }
            else if(*(key.index) == n && !iKey.index && iKey.isMaxInclusive()) {

              Range nr{Bound{RangeSymbol::N, true, true}, Bound{RangeSymbol::N, true, false}};
              ParticipantKey nKey{key.name, i, nr, EvenCase::Both};
              std::shared_ptr<ProjectionList> nProj = std::make_shared<ProjectionList>();

              nProj->appendCloneBack(iProj);
              nProj->appendBack(proj->clone());
              keysToInsert.emplace_back(nKey, nProj);

              if(iKey.range->isLeftDifference(nr)){
                Range cr = iKey.range->leftDifference(cr);
                ParticipantKey cnKey{iKey.name, cr, *(iKey.even)};
                std::shared_ptr<ProjectionList> cnProj = std::make_shared<ProjectionList>();

                cnProj->appendCloneBack(iProj);
                keysToInsert.emplace_back(cnKey, cnProj);

              }          
          
              keysToRemove.push_back(iKey);     

            }
            else if(!iKey.index){
              //okay.. so we have to copy it over directly, and get differences on left and right
              Range r = *(iKey.range);
              ParticipantKey rKey{key.name, i, r, *(iKey.even)};
              std::shared_ptr<ProjectionList> rProj = std::make_shared<ProjectionList>();
              rProj->appendCloneBack(iProj);
              rProj->appendBack(proj->clone());

              keysToInsert.emplace_back(rKey, rProj);
              
              if(lnRange.isLeftDifference(r)) {
                Range lcr = lnRange.leftDifference(r);
                ParticipantKey lcKey{key.name, i, lcr, *(iKey.even)};
                std::shared_ptr<ProjectionList> lcProj = std::make_shared<ProjectionList>();
                lcProj->appendCloneBack(iProj);
                lcProj->appendBack(proj->clone());

                keysToInsert.emplace_back(lcKey, lcProj);
              }
              if(lnRange.isRightDifference(r)) {                
                Range cnr = lnRange.rightDifference(r);
                ParticipantKey cnKey{key.name, i, cnr, *(iKey.even)};
                std::shared_ptr<ProjectionList> cnProj = std::make_shared<ProjectionList>();
                cnProj->appendCloneBack(iProj);
                cnProj->appendBack(proj->clone());

                keysToInsert.emplace_back(cnKey, cnProj);

              }

              keysToRemove.push_back(iKey);

            }
        }
        // 3. Apply removals
        for (const auto& rKey : keysToRemove) {
            projectionMap[rKey.name].erase(rKey);
        }

        // 4. Apply insertions/updates
        for (const auto& [newKey, newProj] : keysToInsert) {
            projectionMap[newKey.name][newKey] = newProj;
        }
      }
      else{
        //we have no ranged
        this->addParticipant(key);
        addProjection(key, std::move(proj));
      }


  }


  //missing handling of literal case. Once this is done, we are completely done and can debug !
  //only to be used in ranged loops with no upper bound (i.e we only call this function from those cases)
  /*
  Bug: currently,this function does not handle the cases of even|odd|both for key and rKey, it simply uses the even attribute from rKey.
  */
  void appendCloneRangedProjection(const ParticipantKey &rKey, std::shared_ptr<ProjectionList> rProj, size_t l, size_t n) {
    // 1. Collect changes to apply after iteration
      std::vector<ParticipantKey> keysToRemove;
      std::vector<std::pair<ParticipantKey, std::shared_ptr<ProjectionList>>> keysToInsert;

      //used for inserting ranges into other ranged types
      bool rangeConsumed = false;
      //used for inserting ranges into non-ranged indexes
      Range limitedRange = Range{Bound{RangeSymbol::L, false, true}, Bound{RangeSymbol::N, false, false}};

      // 2. Iterate over the map
      for (const auto& [key, proj] : projectionMap[rKey.name]) {
       
          if (key.index && !key.range) {
            std::println("literal key found {}", key.toString());
            size_t index = *(key.index);
            if(index == l && rKey.isMinInclusive()) {
              std::println("we enter minInclusive Case");
              //we replace key with key range 
              Range ll = Range{Bound{RangeSymbol::L, true, true}, Bound{RangeSymbol::L, true, false}};
              ParticipantKey lKey{key.name, *(key.index), ll, EvenCase::Both};

              std::shared_ptr<ProjectionList> lProj = std::make_shared<ProjectionList>();
              lProj->appendCloneBack(proj);
              lProj->appendCloneBack(rProj);

              //i.e we remove P[1] and replace with P[1][l,l]
              keysToRemove.push_back(key);
              keysToInsert.emplace_back(lKey, lProj);
            }
            else if(index == n && rKey.isMaxInclusive()) {
              std::println("we enter maxinclusive case");
              //we replace key with key range + new range
             
              Range nn = Range{Bound{RangeSymbol::N, true, true}, Bound{RangeSymbol::N, true, false}};
              ParticipantKey nKey{key.name, *(key.index), nn, EvenCase::Both};

              std::shared_ptr<ProjectionList> nProj = std::make_shared<ProjectionList>();
              nProj->appendCloneBack(proj);
              nProj->appendCloneBack(rProj);

              keysToRemove.push_back(key);
              keysToInsert.emplace_back(nKey, nProj);
            }
            else {
              std::println("we enter in between case");
              //make intersect
              Range pRange = *(rKey.range);
              EvenCase pEven = *(rKey.even);
              ParticipantKey pKey{key.name, *(key.index), pRange, pEven};
              std::shared_ptr<ProjectionList> pProj = std::make_shared<ProjectionList>();
              std::println("pProj created");
              pProj->appendCloneBack(proj);
              std::println("first clone done");
              pProj->appendCloneBack(rProj);
              std::println("second clone done");

              keysToInsert.emplace_back(pKey, pProj);

              //make left
              if(limitedRange.isLeftDifference(pRange)) {
                std::println("isleftDif");
                ParticipantKey LKey{key.name, *(key.index), limitedRange.leftDifference(pRange), pEven};
                keysToInsert.emplace_back(LKey, proj);
              }

              //make right
              if(limitedRange.isRightDifference(pRange)){
                std::println("is rightDif");
                ParticipantKey RKey{key.name, *(key.index), limitedRange.rightDifference(pRange), pEven};
                keysToInsert.emplace_back(RKey, proj);
              }
              //append to to remove
              //we keep literal untill entire range has been added
              keysToRemove.push_back(key);
            }
          }
          else if(key.index && key.range && (*(key.even) == EvenCase::Both  || *(rKey.even) == EvenCase::Both || *(key.even) == *(rKey.even)) ) {
            //final case to solve
            std::println("we enter both case");
            size_t i = *(key.index);
            if(i == l && rKey.range->isMinInclusive()){
              //handling of specific edgecase
              //okay, we append if and only if range is minInclusive
              proj->appendCloneBack(rProj);
            }
            else if(i == n && rKey.range->isMaxInclusive()) {
              //handling of specific edgecase
              proj->appendCloneBack(rProj);
            }
            else {
              //we solve this similar to before
              const Range& L = *key.range;
              const Range& R = *rKey.range;

              if(L == R) {
                proj->appendCloneBack(rProj);
              }
              else if(L.overlaps(R)) {

                //Part Consumed by L resulting in proj.Rproj
                Range inter = L.intersection(R);
                //techically wrong, but it is going to be sufficiently correct 
                EvenCase iC = *(rKey.even);
                ParticipantKey intersectKey{key.name, *(key.index), inter, iC};
                std::shared_ptr<ProjectionList> iProj = std::make_shared<ProjectionList>();
                iProj->appendCloneBack(proj);
                iProj->appendCloneBack(rProj);
                keysToInsert.emplace_back(intersectKey, iProj);

                if (L.isLeftDifference(R)) {
                    Range LL = L.leftDifference(R);
                    EvenCase LC = *(rKey.even);
                    ParticipantKey LLKey{key.name, *(key.index) ,LL, LC};
                    LLKey.range = std::make_unique<Range>(LL);
                    keysToInsert.emplace_back(LLKey, proj);
                }

                // right over from intersect
                if (L.isRightDifference(R)) {
                    Range LR = L.rightDifference(R);
                    EvenCase RC = *(rKey.even);
                    ParticipantKey LRKey{key.name, *(key.index) ,LR, RC};
                    LRKey.range = std::make_unique<Range>(LR);
                    keysToInsert.emplace_back(LRKey, proj);
                }

                // Remove the old key
                keysToRemove.push_back(key);

              }
            }
          }
          else if (key.range && (*(key.even) == EvenCase::Both  || *(rKey.even) == EvenCase::Both || *(key.even) == *(rKey.even))) {
              const Range& L = *key.range;
              const Range& R = *rKey.range;

              if(L == R){
                //we directly append no change to bucket structure
                rangeConsumed = true;
                proj->appendCloneBack(rProj);
              }
              else if(L.overlaps(R)) {
                //RandL
                rangeConsumed = true;

                //Part Consumed by L resulting in proj.Rproj
                Range inter = L.intersection(R);
                //techically wrong, but it is going to be sufficiently correct 
                EvenCase iC = *(rKey.even);
                ParticipantKey intersectKey{key.name, inter, iC};
                std::shared_ptr<ProjectionList> iProj = std::make_shared<ProjectionList>();
                iProj->appendCloneBack(proj);
                iProj->appendCloneBack(rProj);
                keysToInsert.emplace_back(intersectKey, iProj);

                //Part of L not consumed by R resulting in proj only
                //can both be left and right diff:

                // left over from intersect
                if (L.isLeftDifference(R)) {
                    Range LL = L.leftDifference(R);
                    EvenCase LC = *(rKey.even);
                    ParticipantKey LLKey{key.name, LL, LC};
                    LLKey.range = std::make_unique<Range>(LL);
                    keysToInsert.emplace_back(LLKey, proj);
                }

                // right over from intersect
                if (L.isRightDifference(R)) {
                    Range LR = L.rightDifference(R);
                    EvenCase RC = *(rKey.even);
                    ParticipantKey LRKey{key.name, LR, RC};
                    LRKey.range = std::make_unique<Range>(LR);
                    keysToInsert.emplace_back(LRKey, proj);
                }

                // Remove the old key
                keysToRemove.push_back(key);

                     
              }
          }
      }

      if(!rangeConsumed){
        keysToInsert.emplace_back(rKey, rProj);
      }
      // 3. Apply removals
      for (const auto& key : keysToRemove) {
          projectionMap[key.name].erase(key);
      }

      // 4. Apply insertions/updates
      for (const auto& [newKey, newProj] : keysToInsert) {
          projectionMap[newKey.name][newKey] = newProj;
      }
  }


  void printProjections() const {
    std::println("\n\nPchorAST Participant "
                 "projections:\n-------------------------------");
    for (const auto &[elem, value] : projectionMap) {
      for(const auto& [key, projection]: value) {
        std::print("Projection for participant {}: ", key.toString());
        for (const auto &expr : *projection) {
          expr.print();
        }
        std::println(" ");
      }

    }
  }
  //this function goes into memory territory, so watch out
  void printEfficient() const {
    std::println("\n\nPchorAST Participant "
       "projections:\n-------------------------------");

    for(const auto& [elem, value]: projectionMap) {
      std::unordered_map<const void*, std::vector<const ParticipantKey*>> reverseMap;
      for(const auto& [key, projection]: value) {
        size_t c = projection.use_count();
        const void* ptr = projection.get();
        if(!reverseMap.contains(ptr)){
          reverseMap[ptr] = std::vector<const ParticipantKey*>();
        }
        reverseMap[ptr].push_back(&key);
        if(reverseMap[ptr].size() == c) {
          std::print("Projection for participants: ");
          for(const auto& key : reverseMap[ptr]){
            std::print("{}", key->toString());
          }
          for (const auto &expr : *projection) {
            expr.print();
          }
          std::println(" ");
        }
      }
    }
  }

  
  auto begin() { return projectionMap.begin(); }
  auto end() { return projectionMap.end(); }
  auto begin() const { return projectionMap.begin(); }
  auto end() const { return projectionMap.end(); }

private:
  std::unordered_map<std::string, 
    std::unordered_map<ParticipantKey, 
                      std::shared_ptr<ProjectionList>,
                      ParticipantKeyHash>
                      >
        projectionMap;
};

} // namespace PchorAST