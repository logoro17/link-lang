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
    ARROW,
    DOT,

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
};

