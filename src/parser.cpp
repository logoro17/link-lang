#include "parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(const std::vector<Token>& t) : tokens(t), current(0) {}

const Token& Parser::peek() const { return tokens[current]; }
const Token& Parser::advance() { if (!isAtEnd()) current++; return tokens[current - 1]; }
bool Parser::match(TokenType type) { if (isAtEnd()) return false; if (peek().type == type) { advance(); return true; } return false; }
const Token& Parser::consume(TokenType type, const std::string& err) { if (match(type)) return tokens[current - 1]; throw std::runtime_error(err); }
bool Parser::isAtEnd() const { return peek().type == TokenType::EOF_TOKEN; }

bool Parser::isBlockStart() const {
    return peek().type == TokenType::INDENT || peek().type == TokenType::LBRACE;
}

bool Parser::consumeBlockStart() {
    if (match(TokenType::INDENT)) return true;
    if (match(TokenType::LBRACE)) return true;
    throw std::runtime_error("Expected block start (indent or {)");
}

bool Parser::consumeBlockEnd() {
    if (match(TokenType::DEDENT)) return true;
    if (match(TokenType::RBRACE)) return true;
    throw std::runtime_error("Expected block end (dedent or })");
}


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

    advance(); // skip unknown tokens
    return nullptr;
}

std::unique_ptr<AppDecl> Parser::parseApp() {
    auto name = consume(TokenType::IDENTIFIER, "Expected app name").value;

    // accept either NEWLINE or { immediately after the name
    if (!match(TokenType::NEWLINE) && !match(TokenType::LBRACE)) {
        throw std::runtime_error("Expected newline or { after app name");
    }

    // if we saw LBRACE instead of NEWLINE, put it back so consumeBlockStart handles it
    if (peek().type != TokenType::INDENT && peek().type != TokenType::LBRACE) {
        advance(); // skip the already matched LBRACE
    }

    consumeBlockStart();  // will consume INDENT or LBRACE

    auto app = std::make_unique<AppDecl>(name);

    while ((peek().type != TokenType::DEDENT) && (peek().type != TokenType::RBRACE) && !isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) app->body.push_back(std::move(stmt));
    }

    consumeBlockEnd();  // consume DEDENT or RBRACE
    return app;
}

std::unique_ptr<WindowDecl> Parser::parseWindow() {
    auto name = consume(TokenType::IDENTIFIER, "Expected window name").value;

    if (!match(TokenType::NEWLINE) && !match(TokenType::LBRACE)) {
        throw std::runtime_error("Expected newline or { after window name");
    }

    if (peek().type != TokenType::INDENT && peek().type != TokenType::LBRACE) {
        advance();
    }

    consumeBlockStart();
    auto window = std::make_unique<WindowDecl>(name);

    while ((peek().type != TokenType::DEDENT) && (peek().type != TokenType::RBRACE) && !isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) window->body.push_back(std::move(stmt));
    }

    consumeBlockEnd();
    return window;
}

std::unique_ptr<FuncDecl> Parser::parseFunc() {
    auto name = consume(TokenType::IDENTIFIER, "Expected function name").value;

    if (!match(TokenType::NEWLINE) && !match(TokenType::LBRACE)) {
        throw std::runtime_error("Expected newline or { after function name");
    }

    if (peek().type != TokenType::INDENT && peek().type != TokenType::LBRACE) {
        advance();
    }

    consumeBlockStart();
    auto func = std::make_unique<FuncDecl>(name);

    while ((peek().type != TokenType::DEDENT) && (peek().type != TokenType::RBRACE) && !isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) func->body.push_back(std::move(stmt));
    }

    consumeBlockEnd();
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

