#include "lexer.h"
#include <cctype>
#include <stdexcept>
#include <unordered_map>

static const std::unordered_map<std::string, TokenType> keywords = {
    {"app", TokenType::APP},            {"for", TokenType::FOR},            {"while", TokenType::WHILE},
    {"if", TokenType::IF},              {"elif", TokenType::ELIF},          {"else", TokenType::ELSE},
    {"window", TokenType::WINDOW},      {"func", TokenType::FUNC},          {"return", TokenType::RETURN},  
    {"expose", TokenType::EXPOSE},      {"connect", TokenType::CONNECT},    {"import", TokenType::IMPORT}, 
   
    {"try", TokenType::TRY}, {"catch", TokenType::CATCH},
    {"class", TokenType::CLASS}, {"init", TokenType::INIT}, {"new", TokenType::NEW}, {"this", TokenType::THIS}, 

    {"package", TokenType::PACKAGE},    {"sh", TokenType::SH},
    {"in", TokenType::IN},              {"set", TokenType::SET},
    {"true", TokenType::TRUE},          {"false", TokenType::FALSE}, 
    {"clear", TokenType::CLEAR},        {"cls", TokenType::CLS} 
};

Lexer::Lexer(const std::string& source) : src(source), pos(0), line(1), column(1) {
    indentStack.push_back(0);
}

char Lexer::peek() const {
    if (pos >= src.size()) return '\0';
    return src[pos];
}

char Lexer::advance() {
    char c = peek();
    pos++; column++;
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
    while (peek() == ' ' || peek() == '\t' || peek() == '\r') {
        advance();
    }
}

Token Lexer::identifier() {
    int startCol = column;
    std::string value;
    while (std::isalnum(peek()) || peek() == '_') value += advance();
    auto it = keywords.find(value);
    if (it != keywords.end()) return Token{it->second, value, line, startCol};
    return Token{TokenType::IDENTIFIER, value, line, startCol};
}

Token Lexer::stringLiteral() {
    int startCol = column;
    std::string value;
    advance(); 
    while (peek() != '"' && peek() != '\0') {
        if (peek() == '\n') throw std::runtime_error("Unterminated string");
        value += advance();
    }
    if (!match('"')) throw std::runtime_error("Unterminated string");
    return Token{TokenType::STRING, value, line, startCol};
}

void Lexer::handleIndentation(std::vector<Token>& tokens) {
    int count = 0;
    while (peek() == ' ') { advance(); count++; }
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
        if (count != currentIndent) throw std::runtime_error("Invalid indentation");
    }
}

void Lexer::skipComment() {
    while (peek() != '\n' && peek() != '\0') advance();
}

Token Lexer::number() {
    int startCol = column;
    std::string value;
    bool isFloat = false;
    while (std::isdigit(peek())) value += advance();
    if (peek() == '.') {
        isFloat = true; value += advance();
        while (std::isdigit(peek())) value += advance();
    }
    return Token{isFloat ? TokenType::TOKEN_FLOAT : TokenType::TOKEN_NUM, value, line, startCol}; 
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (pos < src.size()) {
        char c = peek();
        if (c == '#') { skipComment(); continue; }
        if (c == '\n') { advance(); tokens.push_back(makeToken(TokenType::NEWLINE)); line++; column = 1; handleIndentation(tokens); continue; }
        if (c == ' ' || c == '\t') { skipWhitespace(); continue; }
        if (std::isalpha(c) || c == '_') { tokens.push_back(identifier()); continue; }
        if (std::isdigit(c)) { tokens.push_back(number()); continue; }
        if (c == '"') { tokens.push_back(stringLiteral()); continue; }
        if (c == '\'') {
            int startCol = column;
            std::string value;
            advance(); 
            while (peek() != '\'' && peek() != '\0') value += advance();
            if (!match('\'')) throw std::runtime_error("Unterminated quote");
            if (value.length() == 1) tokens.push_back(Token{TokenType::CHAR, value, line, startCol});
            else tokens.push_back(Token{TokenType::STRING, value, line, startCol});
            continue;
        }
        
        if (c == '(') { advance(); tokens.push_back(makeToken(TokenType::LPAREN, "(")); continue; }
        if (c == ')') { advance(); tokens.push_back(makeToken(TokenType::RPAREN, ")")); continue; }
        if (c == '[') { advance(); tokens.push_back(makeToken(TokenType::LBRACKET, "[")); continue; }
        if (c == ']') { advance(); tokens.push_back(makeToken(TokenType::RBRACKET, "]")); continue; }
        if (c == '{') { advance(); tokens.push_back(makeToken(TokenType::LBRACE, "{")); continue; }
        if (c == '}') { advance(); tokens.push_back(makeToken(TokenType::RBRACE, "}")); continue; }
        if (c == '.') { advance(); tokens.push_back(makeToken(TokenType::DOT, ".")); continue; }
        if (c == ',') { advance(); tokens.push_back(makeToken(TokenType::COMMA, ",")); continue; }
        if (c == ':') { advance(); tokens.push_back(makeToken(TokenType::COLON, ":")); continue; }

        if (c == '*') { advance(); tokens.push_back(makeToken(TokenType::STAR, "*")); continue; }
        if (c == '/') { advance(); tokens.push_back(makeToken(TokenType::SLASH, "/")); continue; }
        if (c == '<') { advance(); tokens.push_back(makeToken(TokenType::LT, "<")); continue; }
        if (c == '>') { advance(); tokens.push_back(makeToken(TokenType::GT, ">")); continue; }
        
        if (c == '=') { 
            advance(); 
            if (match('=')) tokens.push_back(makeToken(TokenType::EQ_EQ, "=="));
            else tokens.push_back(makeToken(TokenType::ASSIGN, "=")); 
            continue; 
        }
        if (c == '+') {
            advance();
            if (match('+')) tokens.push_back(makeToken(TokenType::PLUS_PLUS, "++"));
            else tokens.push_back(makeToken(TokenType::PLUS, "+")); 
            continue;
        }
        if (c == '-') {
            advance();
            if (match('>')) tokens.push_back(makeToken(TokenType::ARROW, "->"));
            else tokens.push_back(makeToken(TokenType::MINUS, "-")); 
            continue;
        }
        throw std::runtime_error("Unknown char: " + std::string(1, c));
    }
    tokens.push_back(makeToken(TokenType::EOF_TOKEN));
    return tokens;
}
