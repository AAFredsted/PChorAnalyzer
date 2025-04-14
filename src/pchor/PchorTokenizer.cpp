#include "PchorTokenizer.hpp"
#include <string>
#include <vector>

namespace  PchorAST{
std::string Token::toString(){
    std::string s;
    switch(type){
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
    s += " " + value + " " + std::to_string(line) + "\n";
    return s;
}

std::vector<Token> PchorLexer::genTokens() {
    std::vector<Token> tokens{};
    tokens.emplace_back(nextToken());
    while (tokens.back().type != TokenType::EndOfFile && tokens.back().type != TokenType::Unknown) {
        tokens.emplace_back(nextToken());
    }
    return tokens;
}


// Get the next token
Token PchorLexer::nextToken() {
    skipToNextToken();

    if (!file->isValid()) {
        return {TokenType::EndOfFile, "", line};
    }

    char c = file->peekChar();

    // Determine the type of token based on the first character
    if (std::isalpha(c)) {
        return parseIdentifierOrKeyword();
    } else if (std::isdigit(c) || c == 'n') {
        return parseLiteral(c);
    } else if (isSymbol(c)) {
        return parseSymbol(file->readChar());
    }

    // Unknown token
    return {TokenType::Unknown, std::string(1, file->readChar()), line};
}

void PchorLexer::skipToNextToken() {
    bool tokenCharReached = false;
    while (!tokenCharReached && file->isValid()) {
        char c = file->readChar();
        if (c == '\n') {
            line++;
        } else if (std::isspace(c)) {
            continue;
        } else if (c == '/' && file->isValid() && file->peekChar() == '/') {
            // Skip single-line comments
            file->readChar(); // Consume the second '/'
            while (file->isValid() && file->readChar() != '\n') {}
            line++;
        } else {
            file->seekBack();
            tokenCharReached = true;
        }
    }
}


bool PchorLexer::isSymbol(char c) const {
    return std::string("{}:<>.=|->").find(c) != std::string::npos;
}


Token PchorLexer::parseSymbol(char c) {
    if (c == '-' && file->isValid()) {
        file->readChar(); // Consume '-'
        if (file->readChar() != '>') {
            throw std::runtime_error("Invalid Symbol: > missing from arrow operator");
        } // Consume '>'
        return {TokenType::Symbol, "->", line};
    }
    return {TokenType::Symbol, std::string(1, c), line};
}

Token PchorLexer::parseIdentifierOrKeyword() {
    std::string decl = file->readDecl();
    if (decl == "Index" || decl == "Participant" || decl == "Channel" ||
        decl == "foreach" || decl == "Rec" || decl == "end") {
        return {TokenType::Keyword, decl, line};
    }
    return {TokenType::Identifier, decl, line};
}

Token PchorLexer::parseLiteral(char firstChar) {
    if (firstChar == 'n') {
        file->readChar(); // Consume 'n'
        return {TokenType::Literal, "n", line};
    }
    std::string value = file->readLiteral(); // Consume literal
    return {TokenType::Literal, value, line};
}

}//namespace PchorAST