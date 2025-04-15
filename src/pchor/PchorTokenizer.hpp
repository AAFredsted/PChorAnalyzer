#include "PchorFileWrapper.hpp"
#include <string>
#include <vector>
#include <memory> // For std::unique_ptr
#include <unordered_set>

namespace PchorAST {

enum class TokenType {
    Keyword,       // e.g., Index, Participant, Channel, foreach, Rec, end
    Identifier,    // e.g., I, W, w
    Symbol,        // e.g., {, }, :, <, >, =, ->, ., |
    Literal,       // e.g., 1, 2, ..., n for arbitrary literal
    EndOfFile,     // End of the file
    Unknown        // Unknown token
};

struct Token {
    TokenType type;
    std::string_view value;
    size_t line;

    std::string toString();
};

class PchorLexer {
public:
    // Constructor: Takes ownership of the PchorFileWrapper
    explicit PchorLexer(const std::string& filePath): file(std::make_unique<PchorFileWrapper>(filePath)), line(1) {}
    // Delete copy constructor and copy assignment operator
    PchorLexer(const PchorLexer &other) = delete;
    PchorLexer &operator=(const PchorLexer &other) = delete;

    // Move constructor
    PchorLexer(PchorLexer &&other) noexcept
        : file(std::move(other.file)), line(other.line) {}

    // Move assignment operator
    PchorLexer &operator=(PchorLexer &&other) noexcept {
        if (this != &other) {
            file = std::move(other.file);
            line = other.line;
        }
        return *this;
    }

    // Generate all tokens
    std::vector<Token> genTokens();

    // Get the next token
    Token nextToken(std::string_view::iterator& itr, const std::string_view::iterator& end);

private:
    static const std::string symbols;
    static const std::unordered_set<std::string_view> keywords;
    std::unique_ptr<PchorFileWrapper> file; // Unique ownership of the file wrapper
    size_t line;


    void skipToNextToken(std::string_view::iterator& itr, const std::string_view::iterator& end);

    std::string_view::iterator isSymbol(std::string_view::iterator& itr, const std::string_view::iterator& end) const;
    std::string_view::iterator isIdentifier(std::string_view::iterator& itr, const std::string_view::iterator& end) const; 
    std::string_view::iterator isKeyword(std::string_view::iterator& itr, const std::string_view::iterator& end) const; 
    std::string_view::iterator isLiteral(std::string_view::iterator& itr, const std::string_view::iterator& end) const;

    Token parseSymbol(char c);

    Token parseIdentifier();

    Token parseKeyWord();

    Token parseLiteral(char firstChar);
};

} // namespace PchorAST