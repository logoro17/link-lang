#include <iostream>
#include <variant> 
#include <chrono> 
#include <string> 
#include <cstdlib>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cmath> 
#include "parser.h" 

// Universal Object Type
using Obj = std::variant<std::monostate, int, double, std::string, char, bool>;

class Runtime {
public:
    void execute(Program* program) {
        if (!program) return;
        auto start = std::chrono::high_resolution_clock::now();
        for (auto& stmt : program->statements) runStatement(stmt.get());
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        // std::cout << "\nExecution Time: " << diff.count() << "s\n"; // For debugging
    }

private:
    std::unordered_map<std::string, Obj> variables;

    // Helper: Print Object
    void printObj(const Obj& val) {
        if (std::holds_alternative<int>(val)) std::cout << std::get<int>(val);
        else if (std::holds_alternative<double>(val)) std::cout << std::get<double>(val);
        else if (std::holds_alternative<std::string>(val)) std::cout << std::get<std::string>(val);
        else if (std::holds_alternative<char>(val)) std::cout << std::get<char>(val);
        else if (std::holds_alternative<bool>(val)) std::cout << (std::get<bool>(val) ? "true" : "false");
        else std::cout << "nil";
    }

    // Helper: Cek Kebenaran (untuk IF/WHILE)
    bool isTruthy(const Obj& val) {
        if (std::holds_alternative<bool>(val)) return std::get<bool>(val);
        if (std::holds_alternative<int>(val)) return std::get<int>(val) != 0;
        if (std::holds_alternative<double>(val)) return std::get<double>(val) != 0.0;
        return false; // string/char/nil dianggap false di logic sederhana ini
    }

    // --- Evaluate Expression ---
    Obj evaluateExpr(Expr* expr) {
        if (!expr) return std::monostate{};

        if (auto num = dynamic_cast<NumberExpr*>(expr)) return num->value;
        if (auto flt = dynamic_cast<FloatExpr*>(expr)) return flt->value;
        if (auto str = dynamic_cast<StringExpr*>(expr)) return str->value;
        if (auto chr = dynamic_cast<CharExpr*>(expr)) return chr->value;
        if (auto bl = dynamic_cast<BoolExpr*>(expr)) return bl->value;

        if (auto var = dynamic_cast<VariableExpr*>(expr)) {
            if (variables.count(var->name)) return variables[var->name];
            return 0; // Default undefined var = 0
        }

        if (auto inp = dynamic_cast<InputExpr*>(expr)) {
            if (!inp->prompt.empty()) std::cout << inp->prompt; 
            std::string line;
            if (!std::getline(std::cin, line)) return 0;
            // Coba parsing pintar
            try {
                if (line.find('.') != std::string::npos) return std::stod(line); // Float
                return std::stoi(line); // Int
            } catch(...) { return line; } // Gagal angka? Ya udah jadi String
        }

        if (auto bin = dynamic_cast<BinaryExpr*>(expr)) {
            Obj left = evaluateExpr(bin->lhs.get());
            Obj right = evaluateExpr(bin->rhs.get());
            
            // MATH & LOGIC
            // Casus 1: Integer vs Integer
            if (std::holds_alternative<int>(left) && std::holds_alternative<int>(right)) {
                int l = std::get<int>(left);
                int r = std::get<int>(right);
                switch (bin->op) {
                    case '+': return l + r;
                    case '-': return l - r;
                    case '*': return l * r;
                    case '/': return (r != 0) ? l / r : 0;
                    case '<': return l < r;
                    case '>': return l > r;
                    case '=': return l == r;
                }
            }
            // Casus 2: Float vs Float / Int vs Float / Float vs Int
            else if ((std::holds_alternative<double>(left) || std::holds_alternative<int>(left)) && 
                     (std::holds_alternative<double>(right) || std::holds_alternative<int>(right))) {
                double l = std::holds_alternative<int>(left) ? std::get<int>(left) : std::get<double>(left);
                double r = std::holds_alternative<int>(right) ? std::get<int>(right) : std::get<double>(right);
                switch (bin->op) {
                    case '+': return l + r;
                    case '-': return l - r;
                    case '*': return l * r;
                    case '/': return (r != 0.0) ? l / r : 0.0;
                    case '<': return l < r;
                    case '>': return l > r;
                    case '=': return l == r;
                }
            }
            // Casus 3: String vs String (Concatenation)
            else if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
                if (bin->op == '+') return std::get<std::string>(left) + std::get<std::string>(right);
                if (bin->op == '=') return std::get<std::string>(left) == std::get<std::string>(right);
            }
        }
        return 0;
    }

    void runStatement(Stmt* stmt) {
        if (!stmt) return;
        
        if (auto set = dynamic_cast<SetStmt*>(stmt)) {
            variables[set->name] = evaluateExpr(set->expression.get());
            return;
        }

        if (auto call = dynamic_cast<CallStmt*>(stmt)) {
            if (call->func == "print") {
                Obj res = evaluateExpr(call->argExpr.get());
                std::cout << "[LINK-OUT]: "; 
                printObj(res); 
                std::cout << "\n";
            }
            return;
        }

        if (auto ifStmt = dynamic_cast<IfStmt*>(stmt)) {
            if (isTruthy(evaluateExpr(ifStmt->condition.get()))) {
                for (auto& s : ifStmt->thenBranch) runStatement(s.get());
            } else {
                for (auto& s : ifStmt->elseBranch) runStatement(s.get());
            }
            return;
        }

        if (auto whileLoop = dynamic_cast<WhileStmt*>(stmt)) {
            while (isTruthy(evaluateExpr(whileLoop->condition.get()))) {
                for (auto& s : whileLoop->body) runStatement(s.get());
            }
            return;
        }
        
        // Loop For ( Simple Int loop)
        if (auto loop = dynamic_cast<ForStmt*>(stmt)) {
             if (loop->rangeValue.find("range(") == 0) {
                 std::string numStr = loop->rangeValue.substr(6);
                 if (!numStr.empty() && numStr.back() == ')') numStr.pop_back();
                 int limit = std::stoi(numStr);
                 for (int i = 0; i < limit; i++) {
                     variables[loop->iteratorName] = i;
                     for (auto& s : loop->body) runStatement(s.get());
                 }
             }
             return;
        }
        
        if (auto up = dynamic_cast<UpdateStmt*>(stmt)) {
            if (variables.count(up->name) && std::holds_alternative<int>(variables[up->name])) {
                std::get<int>(variables[up->name])++;
            }
            return;
        }
        
        if (auto prop = dynamic_cast<PropertyStmt*>(stmt)) {
            if (prop->name == "sh") {
                int status = system(prop->value.c_str());
                (void)status; 
            }
            return;
        }
   }
};
