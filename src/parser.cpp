#include "parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(const std::vector<Token>& t) : tokens(t), current(0) {}

const Token& Parser::peek() const { return tokens[current]; }
const Token& Parser::advance() { if (!isAtEnd()) current++; return tokens[current - 1]; }
bool Parser::match(TokenType type) { if (isAtEnd()) return false; if (peek().type == type) { advance(); return true; } return false; }
const Token& Parser::consume(TokenType type, const std::string& err) { if (match(type)) return tokens[current - 1]; throw std::runtime_error(err); }
bool Parser::isAtEnd() const { return peek().type == TokenType::EOF_TOKEN; }

std::unique_ptr<Program> Parser::parse() {
    auto program = std::make_unique<Program>();
    while (!isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) program->statements.push_back(std::move(stmt));
    }
    return program;
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (match(TokenType::APP)) return parseApp();
    if (match(TokenType::WINDOW)) return parseWindow();
    if (match(TokenType::FUNC)) return parseFunc();
    if (match(TokenType::CONNECT)) return parseConnect();

    if (peek().type == TokenType::IDENTIFIER) {
        // Is this a property or call?
        std::string name = advance().value;
        if (match(TokenType::STRING)) return std::make_unique<PropertyStmt>(name, tokens[current - 1].value);
        else if (!isAtEnd()) return std::make_unique<CallStmt>(name, ""); // simplified
    }

    advance(); // skip unknown token, we dont like em
    return nullptr;
}

std::unique_ptr<AppDecl> Parser::parseApp() {
    auto name = consume(TokenType::IDENTIFIER, "Expected app name").value;
    consume(TokenType::NEWLINE, "Expected newline after app name");
    consume(TokenType::INDENT, "Expected indent after app");
    auto app = std::make_unique<AppDecl>(name);

    while (!match(TokenType::DEDENT) && !isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) app->body.push_back(std::move(stmt));
    }
    return app;
}

std::unique_ptr<WindowDecl> Parser::parseWindow() {
    auto name = consume(TokenType::IDENTIFIER, "Expected window name").value;
    consume(TokenType::NEWLINE, "Expected newline after window name");
    consume(TokenType::INDENT, "Expected indent after window");
    auto window = std::make_unique<WindowDecl>(name);

    while (!match(TokenType::DEDENT) && !isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) window->body.push_back(std::move(stmt));
    }
    return window;
}

std::unique_ptr<FuncDecl> Parser::parseFunc() {
    auto name = consume(TokenType::IDENTIFIER, "Expected function name").value;
    consume(TokenType::NEWLINE, "Expected newline after function name");
    consume(TokenType::INDENT, "Expected indent after function");
    auto func = std::make_unique<FuncDecl>(name);

    while (!match(TokenType::DEDENT) && !isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) func->body.push_back(std::move(stmt));
    }
    return func;
}

std::unique_ptr<ConnectStmt> Parser::parseConnect() {
    auto source = consume(TokenType::IDENTIFIER, "Expected source in connect").value;
    consume(TokenType::DOT, "Expected dot in connect");
    auto event = consume(TokenType::IDENTIFIER, "Expected event in connect").value;
    consume(TokenType::ARROW, "Expected arrow in connect");
    auto target = consume(TokenType::IDENTIFIER, "Expected target in connect").value;
    return std::make_unique<ConnectStmt>(source, event, target);
}

