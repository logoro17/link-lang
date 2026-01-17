#pragma once
#include <string>

enum class TokenType {
    // Keywords
    APP, WINDOW, FUNC, EXPOSE, CONNECT, PACKAGE, SH, FOR, IN, SET, 
    WHILE, IF, ELIF, ELSE, IMPORT, RETURN, TRY, CATCH, 
    TRUE, FALSE, CLEAR, CLS,
    
    //Class Token
    CLASS, INIT, NEW, THIS, 

    // Structural
    INDENT, DEDENT, NEWLINE, EOF_TOKEN, COMMA, COLON,

    // Literals & Identifiers
    IDENTIFIER, 
    STRING,     // "abc"
    CHAR,       // 'a'
    TOKEN_NUM,  // 123
    TOKEN_FLOAT,// 12.34
    
    // Symbols
    LBRACE, RBRACE,    // { }
    LPAREN, RPAREN,    // ( )
    LBRACKET, RBRACKET,// [ ]
    ARROW, DOT, PLUS, MINUS, STAR, SLASH, PLUS_PLUS,
    ASSIGN, 
    LT, GT, EQ_EQ 
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};
