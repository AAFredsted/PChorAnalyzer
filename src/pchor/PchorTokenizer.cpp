#include "PchorTokenizer.hpp"
#include <print>
#include <string>
#include <vector>
namespace PchorAST {
std::string Token::toString() const {
  std::string s;
  switch (type) {
  case TokenType::Keyword:
    s = "Keyword";
    break;
  case TokenType::Identifier:
    s = "Identifier";
    break;
  case TokenType::Symbol:
    s = "Symbol";
    break;
  case TokenType::Literal:
    s = "Literal";
    break;
  case TokenType::EndOfFile:
    s = "EndOfFile";
    break;
  case TokenType::Unknown:
    s = "Unknown";
    break;
  }
  s += " " + std::string(value) + " " + std::to_string(line) + "\n";
  return s;
}

const std::string PchorLexer::symbols = "{}<>[]().=+-:";
const std::unordered_set<std::string_view> PchorLexer::keywords{
    "Index", "Participant", "Channel", "Label", "foreach", "end", "min", "max"};

std::vector<Token> PchorLexer::genTokens() {
  std::string_view input = file->getBuffer();
  std::vector<Token> tokens{};

  auto itr = input.begin();
  const auto end = input.end();

  size_t i = 1;
  tokens.emplace_back(nextToken(itr, end));
  while (tokens.back().type != TokenType::EndOfFile) {
    i++;
    tokens.emplace_back(nextToken(itr, end));
    if (i == 200) {
      break;
    }
  }

  return tokens;
}

Token PchorLexer::nextToken(std::string_view::iterator &itr,
                            const std::string_view::iterator &end) {
  skipToNextToken(itr, end);

  if (itr == end) {
    return {TokenType::EndOfFile, std::string_view{itr, end}, line};
  }

  auto possibleEnd = isSymbol(itr, end);
  if (possibleEnd != end) {
    std::string_view value(itr, std::distance(itr, possibleEnd));
    itr = possibleEnd;
    return {TokenType::Symbol, value, line};
  }

  possibleEnd = isLiteral(itr, end);
  if (possibleEnd != end) {
    std::string_view value(itr, std::distance(itr, possibleEnd));
    itr = possibleEnd;
    return {TokenType::Literal, value, line};
  }

  possibleEnd = isKeyword(itr, end);
  if (possibleEnd != end) {
    std::string_view value(itr, std::distance(itr, possibleEnd));
    itr = possibleEnd;
    return {TokenType::Keyword, value, line};
  }

  possibleEnd = isIdentifier(itr, end);
  if (possibleEnd != end) {
    std::string_view value(itr, std::distance(itr, possibleEnd));
    itr = possibleEnd;
    return {TokenType::Identifier, value, line};
  }

  std::string_view value(itr, std::distance(itr, itr + 1));
  itr++;
  return {TokenType::Unknown, value, line};
}

void PchorLexer::skipToNextToken(std::string_view::iterator &itr,
                                 const std::string_view::iterator &end) {
  while (itr != end && std::isspace(*itr)) {
    if (*itr == '\n') {
      line++;
    }
    ++itr;
  }
}

std::string_view::iterator
PchorLexer::isSymbol(std::string_view::iterator &itr,
                     const std::string_view::iterator &end) const {
  if (itr == end) {
    return end;
  }
  if (*itr == '-' && (itr + 1) != end && *(itr + 1) == '>') {
    return itr + 2; // Return iterator past "->"
  }

  if (symbols.find(*itr) != std::string::npos) {
    return itr + 1;
  }

  return end;
}

std::string_view::iterator
PchorLexer::isKeyword(std::string_view::iterator &itr,
                      const std::string_view::iterator &end) const {
  auto spaceItr = std::find_if(itr, end, [](char c) {
    return std::isspace(c) || symbols.find(c) != std::string_view::npos;
  });
  std::string_view possibleKeyword(&(*itr), std::distance(itr, spaceItr));

  return keywords.find(possibleKeyword) != keywords.end() ? spaceItr : end;
}

std::string_view::iterator
PchorLexer::isLiteral(std::string_view::iterator &itr,
                      const std::string_view::iterator &end) const {
  if (itr == end) {
    return end;
  }

  if (*itr == 'n' && (itr + 1) != end &&
      (symbols.find(*(itr + 1)) != std::string_view::npos ||
       std::isspace(*(itr + 1)))) {
    return itr + 1;
  }

  if (std::isdigit(*itr)) {
    auto incr_itr = itr;
    while (incr_itr != end && std::isdigit(*incr_itr)) {
      ++incr_itr;
    }
    return incr_itr;
  }

  return end;
}

std::string_view::iterator
PchorLexer::isIdentifier(std::string_view::iterator &itr,
                         const std::string_view::iterator &end) const {
  return std::find_if(itr, end, [](char c) {
    return std::isspace(c) || symbols.find(c) != std::string_view::npos;
  });
}

} // namespace PchorAST