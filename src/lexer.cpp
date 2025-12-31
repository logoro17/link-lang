#include "lexer.h"
#include <cctype>
#include <stdexcept>
#include <unordered_map>

static const std::unordered_map<std::string, TokenType> keywords = {
    {"app", TokenType::APP},
    {"window", TokenType::WINDOW},
    {"func", TokenType::FUNC},
    {"expose", TokenType::EXPOSE},
    {"connect", TokenType::CONNECT},
    {"package", TokenType::PACKAGE}
};

bool isAlphaNumeric(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

Lexer::Lexer(const std::string& source)
    : src(source), pos(0), line(1), column(1) {
    indentStack.push_back(0);
}

char Lexer::peek() const {
    if (pos >= src.size()) return '\0';
    return src[pos];
}

char Lexer::advance() {
    char c = peek();
    pos++;
    column++;
    return c;
}

bool Lexer::match(char expected) {
    if (peek() != expected) return false;
    advance();
    return true;
}

Token Lexer::makeToken(TokenType type, const std::string& value) {
    return Token{type, value, line, column};
}

void Lexer::skipWhitespace() {
    while (peek() == ' ' || peek() == '\t') {
        if (peek() == '\t') {
            throw std::runtime_error("Tabs are illegal in Link (line " + std::to_string(line) + ")");
        }
        advance();
    }
}

Token Lexer::identifier() {
    int startCol = column;
    std::string value;
    while (isAlphaNumeric(peek())) {
        value += advance(); // consumes the character
    }

    // Check if keyword
    auto it = keywords.find(value);
    if (it != keywords.end()) {
        return Token{it->second, value, line, startCol};
    }

    return Token{TokenType::IDENTIFIER, value, line, startCol};
}

Token Lexer::stringLiteral() {
    int startCol = column;
    std::string value;

    advance(); // consume opening quote

    while (peek() != '"' && peek() != '\0') {
        if (peek() == '\n') {
            throw std::runtime_error("Unterminated string literal at line " + std::to_string(line));
        }
        value += advance();
    }

    if (!match('"')) {
        throw std::runtime_error("Unterminated string literal at line " + std::to_string(line));
    }

    return Token{TokenType::STRING, value, line, startCol};
}

Token Lexer::number() {
    int startCol = column;
    std::string value;

    while (std::isdigit(peek())) {
        value += advance();
    }

    return Token(TokenType::NUMBER, value, line, startCol);
}

void Lexer::handleIndentation(std::vector<Token>& tokens) {
    int count = 0;
    while (peek() == ' ') {
        advance();
        count++;
    }

    if (peek() == '\n' || peek() == '\0') return;

    int currentIndent = indentStack.back();

    if (count > currentIndent) {
        indentStack.push_back(count);
        tokens.push_back(makeToken(TokenType::INDENT));
    } else {
        while (count < currentIndent) {
            indentStack.pop_back();
            currentIndent = indentStack.back();
            tokens.push_back(makeToken(TokenType::DEDENT));
        }

        if (count != currentIndent) {
            throw std::runtime_error(
                "Invalid indentation at line " + std::to_string(line)
            );
        }
    }
}

void Lexer::skipComment() {
    while (peek() != '\n' && peek() != '\0') {
        advance();
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (pos < src.size()) {
        char c = peek();

        if (c == '#') {
            skipComment();
            continue;
        }

        if (c == '\n') {
            advance();
            tokens.push_back(makeToken(TokenType::NEWLINE));
            line++;
            column = 1;
            handleIndentation(tokens);
            continue;
        }

        if (c == ' ' || c == '\t') {
            skipWhitespace();
            continue;
        }

        if (std::isalpha(c) || c == '_') {
            tokens.push_back(identifier());
            continue;
        }

        if (c == '"') {
            tokens.push_back(stringLiteral());
            continue;
        }

        if (c == '{') {
            advance();
            tokens.push_back(makeToken(TokenType::LBRACE));
            continue;
        }

        if (c == '}') {
            advance();
            tokens.push_back(makeToken(TokenType::RBRACE));
            continue;
        }

        if (c == '(') {
            advance();
            tokens.push_back(Token(TokenType::LPAREN, "(", line, column));
            continue;
        }

        if (c == ')') {
            advance();
            tokens.push_back(Token(TokenType::RPAREN, ")", line, column));
            continue;
        }

        if (c == '-' && src[pos + 1] == '>') {
            advance();
            advance();
            tokens.push_back(makeToken(TokenType::ARROW));
            continue;
        }

        if (c == '.') {
            advance();
            tokens.push_back(makeToken(TokenType::DOT));
            continue;
        }

        if (std::isdigit(c)) {
            tokens.push_back(number());
            continue;
        }

        if (c == '=') {
            tokens.push_back(makeToken(TokenType::EQUAL));
            advance();
            continue;
        }



        throw std::runtime_error(
            "Unexpected character '" + std::string(1, c) +
            "' at line " + std::to_string(line)
        );
    }

    while (indentStack.size() > 1) {
        indentStack.pop_back();
        tokens.push_back(makeToken(TokenType::DEDENT));
    }

    tokens.push_back(makeToken(TokenType::EOF_TOKEN));
    return tokens;
}

