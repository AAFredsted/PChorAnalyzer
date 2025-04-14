#include "PchorFileWrapper.hpp"
#include <string>
#include <vector>
#include <memory> // For std::unique_ptr

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
    std::string value;
    size_t line;

    std::string toString();
};

class PchorLexer {
public:
    // Constructor: Takes ownership of the PchorFileWrapper
    explicit PchorLexer(std::unique_ptr<PchorFileWrapper> file)
        : file(std::move(file)), line(1) {}

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
    Token nextToken();

private:
    std::unique_ptr<PchorFileWrapper> file; // Unique ownership of the file wrapper
    size_t line;

    void skipToNextToken();

    bool isSymbol(char c) const;

    Token parseSymbol(char c);

    Token parseIdentifierOrKeyword();

    Token parseLiteral(char firstChar);
};

} // namespace PchorAST