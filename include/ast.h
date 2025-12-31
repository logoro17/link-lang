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

struct PropertyCall {
    std::string name;
    std::vector<std::string> args;
};

// Window declaration
struct WindowDecl : public Stmt {
    std::string name;
    std::string title;
    std::vector<PropertyCall> properties;       // <-- title, size, etc
    std::vector<std::unique_ptr<Stmt>> body;

    WindowDecl(const std::string& n) : name(n) {}
    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "Window: " << name << "\n";

        // print properties
        for (const auto& prop : properties) {
            std::cout << std::string(indent + 2, ' ') << "Property: " << prop.name;
            if (!prop.args.empty()) {
                std::cout << " = ";
                for (size_t i = 0; i < prop.args.size(); i++) {
                    std::cout << prop.args[i];
                    if (i + 1 < prop.args.size()) std::cout << ", ";
                }
            }
            std::cout << "\n";
        }

        // print body statements
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

// We the Functions of Link, in Order to...
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
