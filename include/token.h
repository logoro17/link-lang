#pragma once
#include <string>

enum class TokenType {
    // Structure
    INDENT,
    DEDENT,
    NEWLINE,
    EOF_TOKEN,

    // Keywords
    APP,
    WINDOW,
    FUNC,
    EXPOSE,
    CONNECT,
    PACKAGE,

    // Symbols
    LBRACE,
    RBRACE,
    LPAREN,
    RPAREN,
    COMMA,
    ARROW,
    DOT,
    NUMBER,
    EQUAL,

    // Literals
    IDENTIFIER,
    STRING,

    // Misc
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;

    // Default constructor
    Token() : type(TokenType::UNKNOWN), value(""), line(0), column(0) {}

    // Constructor with line and column
    Token(TokenType t, const std::string& v, int l, int c)
        : type(t), value(v), line(l), column(c) {}
};
