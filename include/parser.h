#pragma once
#include <vector>
#include <memory>
#include "token.h"
#include "ast.h"

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
    
    std::unique_ptr<Stmt> parseClass();
    
    std::unique_ptr<Stmt> parseFor(); 
    std::unique_ptr<Stmt> parseTry();
    std::unique_ptr<Stmt> parseWhile(); 
    std::unique_ptr<Stmt> parseIf(); 
    std::unique_ptr<Stmt> parseSet();
    std::unique_ptr<Stmt> parseReturn();
	
    // Expression Hierarchy
    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parseAdditive();
    std::unique_ptr<Expr> parseTerm();
    std::unique_ptr<Expr> parseUnary(); 
    std::unique_ptr<Expr> parsePostfix(); 
    std::unique_ptr<Expr> parsePrimary();
    
    std::vector<std::unique_ptr<Expr>> parseArguments();
    std::unique_ptr<CallStmt> parseCall();
    std::unique_ptr<ConnectStmt> parseConnect();

    bool isAtEnd() const;
};
