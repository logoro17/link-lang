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
    if (match(TokenType::SET)) return parseSet(); 
    if (match(TokenType::APP)) return parseApp();
    if (match(TokenType::FOR)) return parseFor(); 
    if (match(TokenType::WHILE)) return parseWhile(); 
    if (match(TokenType::IF)) return parseIf(); 
    if (match(TokenType::WINDOW)) return parseWindow();
    if (match(TokenType::FUNC)) return parseFunc();
    if (match(TokenType::CONNECT)) return parseConnect();
        
    if (match(TokenType::SH)){
        auto command = consume(TokenType::STRING, "Ouch! 'sh' command needs <str> in front of it ('command' sh).").value; 
        return std::make_unique<PropertyStmt>("sh", command); 
    }

    if (peek().type == TokenType::IDENTIFIER) {
        std::string name = advance().value; 

        // 1. Cek i++ (Update)
        if (match(TokenType::PLUS_PLUS)) {
            return std::make_unique<UpdateStmt>(name, "++"); 
        }

        // 2. Cek PRINT
        if (name == "print") {
            auto argExpr = parseExpression(); 
            return std::make_unique<CallStmt>("print", std::move(argExpr));
        }
        
        // Default CallStmt
        auto emptyExpr = std::make_unique<NumberExpr>(0);
        return std::make_unique<CallStmt>(name, std::move(emptyExpr));
    }

    advance(); 
    return nullptr; 
}

// --- EXPRESSION HIERARCHY (Fixed) ---

// Level 1: Equation (<, >, ==)
std::unique_ptr<Expr> Parser::parseExpression() {
    auto left = parseAdditive(); // Panggil Level 2

    while (match(TokenType::LT) || match(TokenType::GT) || match(TokenType::EQ_EQ)) {
        std::string op = tokens[current - 1].value;
        auto right = parseAdditive();
        // Ambil char pertama (<, >, =)
        left = std::make_unique<BinaryExpr>(op[0], std::move(left), std::move(right));
    }
    return left;
}

// Level 2: Addition (+, -)
std::unique_ptr<Expr> Parser::parseAdditive() {
    auto left = parseTerm(); // Panggil Level 3
    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        char op = tokens[current - 1].value[0];
        auto right = parseTerm();
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    return left;
}

// Level 3: Multiplication or Division (*, /)
std::unique_ptr<Expr> Parser::parseTerm() {
    auto left = parsePrimary(); // Panggil Level 4
    while (match(TokenType::STAR) || match(TokenType::SLASH)) {
        char op = tokens[current - 1].value[0];
        auto right = parsePrimary();
        left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
    }
    return left;
}

// Level 4: Primary (Numbers, Identifiers, Parentheses)
std::unique_ptr<Expr> Parser::parsePrimary() {
    // 1. Integer
    if (match(TokenType::TOKEN_NUM)) {
        return std::make_unique<NumberExpr>(std::stoi(tokens[current - 1].value));
    }
    // 2. Float
    if (match(TokenType::TOKEN_FLOAT)) {
        return std::make_unique<FloatExpr>(std::stod(tokens[current - 1].value));
    }
    // 3. String
    if (match(TokenType::STRING)) {
        return std::make_unique<StringExpr>(tokens[current - 1].value);
    }
    // 4. Char
    if (match(TokenType::CHAR)) {
        return std::make_unique<CharExpr>(tokens[current - 1].value[0]);
    }
    // 5. Boolean (true/false)
    if (match(TokenType::TRUE)) return std::make_unique<BoolExpr>(true);
    if (match(TokenType::FALSE)) return std::make_unique<BoolExpr>(false);

    // 6. Input & Variabel (Kode yang kita buat sebelumnya)
    if (peek().type == TokenType::IDENTIFIER) {
         if (peek().value == "input") {
            advance(); consume(TokenType::LPAREN, "Expected '('");
            std::string prompt = "";
            if (peek().type == TokenType::STRING) prompt = advance().value;
            consume(TokenType::RPAREN, "Expected ')'");
            return std::make_unique<InputExpr>(prompt);
        }
        return std::make_unique<VariableExpr>(advance().value);
    }

    if (match(TokenType::LPAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Butuh ')'");
        return expr;
    }
    throw std::runtime_error("Unknown token: " + peek().value);
}

// --- OTHER STATEMENTS ---

std::unique_ptr<Stmt> Parser::parseSet() {
    auto name = consume(TokenType::IDENTIFIER, "Butuh nama variabel setelah 'set'").value;
    consume(TokenType::ASSIGN, "Butuh '=' setelah nama variabel");
    auto value = parseExpression(); // Gunakan hierarchy baru
    return std::make_unique<SetStmt>(name, std::move(value));
}

std::unique_ptr<Stmt> Parser::parseWhile() {
    auto condition = parseExpression(); // while x < 5
    
    consume(TokenType::NEWLINE, "Expected newline after while condition");
    consume(TokenType::INDENT, "Expected indent block for while loop");

    auto stmt = std::make_unique<WhileStmt>(std::move(condition));

    while (!match(TokenType::DEDENT) && !isAtEnd()) {
        if (peek().type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        auto s = parseStatement();
        if (s) stmt->body.push_back(std::move(s));
    }
    return stmt;
}

std::unique_ptr<AppDecl> Parser::parseApp() {
    std::string name;
    if (peek().type == TokenType::IDENTIFIER || peek().type == TokenType::STRING) {
        name = advance().value;
    } else {
        throw std::runtime_error("Error: App-name must be a word or text");
    }

    consume(TokenType::NEWLINE, "Harus ada baris baru setelah nama app");
    consume(TokenType::INDENT, "Harus ada indentasi");
    
    auto app = std::make_unique<AppDecl>(name);
    while (!match(TokenType::DEDENT) && !isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) app->body.push_back(std::move(stmt));
    }
    return app;
}

std::unique_ptr<Stmt> Parser::parseIf() {
    auto condition = parseExpression(); 

    consume(TokenType::NEWLINE, "Expected newline after if/elif condition");
    consume(TokenType::INDENT, "Expected indent block for if/elif");

    auto stmt = std::make_unique<IfStmt>(std::move(condition));

    while (!match(TokenType::DEDENT) && !isAtEnd()) {
        if (peek().type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        auto s = parseStatement();
        if (s) stmt->thenBranch.push_back(std::move(s));
    }

    while (peek().type == TokenType::NEWLINE) advance();

    if (match(TokenType::ELIF)) {
        stmt->elseBranch.push_back(parseIf());
    } 
    else if (match(TokenType::ELSE)) {
        consume(TokenType::NEWLINE, "Expected newline after else");
        consume(TokenType::INDENT, "Expected indent block for else");

        while (!match(TokenType::DEDENT) && !isAtEnd()) {
            if (peek().type == TokenType::NEWLINE) {
                advance();
                continue;
            }
            auto s = parseStatement();
            if (s) stmt->elseBranch.push_back(std::move(s));
        }
    }

    return stmt;
}

std::unique_ptr<Stmt> Parser::parseFor() {
    auto iteratorName = consume(TokenType::IDENTIFIER, "Butuh nama variabel setelah 'for'").value;
    consume(TokenType::IN, "Butuh 'in' setelah nama variabel");

    std::string fullRange = "";
    if (peek().type == TokenType::IDENTIFIER) {
        fullRange = advance().value; 
        if (peek().type == TokenType::LPAREN) {
            fullRange += "(";
            advance();
            if (peek().type == TokenType::STRING || peek().type == TokenType::IDENTIFIER || peek().type == TokenType::TOKEN_NUM) {
                fullRange += advance().value;
            }
            if (peek().type == TokenType::RPAREN) {
                fullRange += ")";
                advance(); 
            }
        }
    } else if (peek().type == TokenType::STRING) {
        fullRange = advance().value; 
    } else {
        throw std::runtime_error("Butuh range atau list setelah 'in'");
    }

    while (!isAtEnd() && peek().type == TokenType::NEWLINE) advance();
    if (peek().type == TokenType::INDENT) advance();

    auto forStmt = std::make_unique<ForStmt>(iteratorName, fullRange);

    while (!isAtEnd() && peek().type != TokenType::DEDENT) {
        if (peek().type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        auto stmt = parseStatement();
        if (stmt) forStmt->body.push_back(std::move(stmt));
        else break; 
    }
    if (!isAtEnd() && peek().type == TokenType::DEDENT) advance();

    return forStmt;
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
