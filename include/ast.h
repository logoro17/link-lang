#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "token.h"

// Base class for all of the statements
struct Stmt {
    virtual ~Stmt() = default;
    virtual void print(int indent = 0) const = 0;
};

// Root Program node
struct Program {
    std::vector<std::unique_ptr<Stmt>> statements;
    void print() const {
        for (auto& stmt : statements) stmt->print(0);
    }
};

// App declaration
struct AppDecl : public Stmt {
    std::string name;
    std::vector<std::unique_ptr<Stmt>> body;

    AppDecl(const std::string& n) : name(n) {}
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "App: " << name << "\n";
        for (auto& stmt : body) stmt->print(indent + 2);
    }
};

// Window declaration
struct WindowDecl : public Stmt {
    std::string name;
    std::vector<std::unique_ptr<Stmt>> body;

    WindowDecl(const std::string& n) : name(n) {}
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "Window: " << name << "\n";
        for (auto& stmt : body) stmt->print(indent + 2);
    }
};

// Property statement (like "Hello")
struct PropertyStmt : public Stmt {
    std::string name;
    std::string value;

    PropertyStmt(const std::string& n, const std::string& v) : name(n), value(v) {}
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "Property: " << name << " = \"" << value << "\"\n";
    }
};

// The function's declaration of independence
struct FuncDecl : public Stmt {
    std::string name;
    std::vector<std::unique_ptr<Stmt>> body;

    FuncDecl(const std::string& n) : name(n) {}
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "Func: " << name << "\n";
        for (auto& stmt : body) stmt->print(indent + 2);
    }
};

// Call (like print "Hello")
struct CallStmt : public Stmt {
    std::string func;
    std::string arg;

    CallStmt(const std::string& f, const std::string& a) : func(f), arg(a) {}
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "Call: " << func << "(\"" << arg << "\")\n";
    }
};

// Connect (connect Main.onLoad -> greet)
struct ConnectStmt : public Stmt {
    std::string source;
    std::string event;
    std::string target;

    ConnectStmt(const std::string& s, const std::string& e, const std::string& t)
        : source(s), event(e), target(t) {}
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "Connect: " << source << "." << event << " -> " << target << "\n";
    }
};

