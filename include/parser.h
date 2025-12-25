#pragma once
#include <vector>
#include <memory>
#include "token.h"
#include "ast.h"

// the MEAT

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::unique_ptr<Program> parse();

private:
    const std::vector<Token>& tokens;
    size_t current;

    const Token& peek() const;
    const Token& advance();
    bool match(TokenType type);
    const Token& consume(TokenType type, const std::string& err);

    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<AppDecl> parseApp();
    std::unique_ptr<WindowDecl> parseWindow();
    std::unique_ptr<FuncDecl> parseFunc();
    std::unique_ptr<PropertyStmt> parseProperty();
    std::unique_ptr<CallStmt> parseCall();
    std::unique_ptr<ConnectStmt> parseConnect();

    bool isAtEnd() const;
};

