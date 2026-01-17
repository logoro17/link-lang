#include <iostream>
#include <variant> 
#include <string> 
#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <thread>
#include <chrono>

// INCLUDE HEADER
#include "types.h"  
#include "env.h"    
#include "lexer.h" 
#include "parser.h" 
#include "os.h" 
#include "link_str.h"
#include "link_math.h"

class Runtime {
public:
    Runtime() {
        globalEnv = std::make_shared<Environment>();
        currentEnv = globalEnv;
    }
    
    void execute(std::unique_ptr<Program> program) {
        if (!program) return;
        
        loadedPrograms.push_back(std::move(program));
        
        for (auto& stmt : loadedPrograms.back()->statements) { 
            runStatement(stmt.get()); 
        }
    }

private:
    std::shared_ptr<Environment> globalEnv;
    std::shared_ptr<Environment> currentEnv;
    std::unordered_map<std::string, FuncDecl*> functionRegistry;
    std::vector<std::unique_ptr<Program>> loadedPrograms;

    bool isTruthy(const Obj& val) {
        if (std::holds_alternative<bool>(val.as)) return std::get<bool>(val.as);
        if (std::holds_alternative<int>(val.as)) return std::get<int>(val.as) != 0;
        if (std::holds_alternative<double>(val.as)) return std::get<double>(val.as) != 0.0;
        if (std::holds_alternative<std::shared_ptr<List>>(val.as)) return !std::get<std::shared_ptr<List>>(val.as)->empty();
        return false; 
    }

    FuncDecl* findMethod(LinkClass* klass, const std::string& name) {
        if (klass->methods.count(name)) {
            return dynamic_cast<FuncDecl*>(klass->methods[name]);
        }
        return nullptr;
    }

    Obj evaluateExpr(Expr* expr) {
        if (!expr) return Obj();
        
        if (auto num = dynamic_cast<NumberExpr*>(expr)) return Obj(num->value);
        if (auto flt = dynamic_cast<FloatExpr*>(expr)) return Obj(flt->value);
        if (auto str = dynamic_cast<StringExpr*>(expr)) return Obj(str->value);
        if (auto chr = dynamic_cast<CharExpr*>(expr)) return Obj(chr->value);
        if (auto bl = dynamic_cast<BoolExpr*>(expr)) return Obj(bl->value);
        if (auto var = dynamic_cast<VariableExpr*>(expr)) return currentEnv->get(var->name);

        if (auto arr = dynamic_cast<ArrayExpr*>(expr)) {
            auto list = std::make_shared<List>();
            for (auto& el : arr->elements) list->push_back(evaluateExpr(el.get()));
            return Obj(list);
        }
        if (auto dictNode = dynamic_cast<DictExpr*>(expr)) {
            auto dict = std::make_shared<Dict>();
            for (auto& p : dictNode->pairs) {
                Obj key = evaluateExpr(p.first.get());
                Obj val = evaluateExpr(p.second.get());
                if (std::holds_alternative<std::string>(key.as)) (*dict)[std::get<std::string>(key.as)] = val;
                else std::cout << "Runtime Error: Dict key must be string.\n";
            }
            return Obj(dict);
        }

        if (auto idx = dynamic_cast<IndexExpr*>(expr)) {
            Obj object = evaluateExpr(idx->object.get());
            Obj index = evaluateExpr(idx->index.get());
            if (std::holds_alternative<std::shared_ptr<List>>(object.as) && std::holds_alternative<int>(index.as)) {
                auto list = std::get<std::shared_ptr<List>>(object.as);
                int i = std::get<int>(index.as);
                if (i < 0) i += list->size(); 
                if (i >= 0 && i < (int)list->size()) return (*list)[i];
                std::cout << "Runtime Error: Index out of bounds\n";
            } else if (std::holds_alternative<std::shared_ptr<Dict>>(object.as) && std::holds_alternative<std::string>(index.as)) {
                auto dict = std::get<std::shared_ptr<Dict>>(object.as);
                std::string key = std::get<std::string>(index.as);
                if (dict->count(key)) return (*dict)[key];
                return Obj();
            }
            return Obj();
        }

        if (auto newExpr = dynamic_cast<NewExpr*>(expr)) {
            Obj classObj = currentEnv->get(newExpr->className);
            if (!std::holds_alternative<std::shared_ptr<LinkClass>>(classObj.as)) {
                std::cout << "Runtime Error: '" << newExpr->className << "' is not a class.\n";
                return Obj();
            }

            auto klass = std::get<std::shared_ptr<LinkClass>>(classObj.as);
            auto instance = std::make_shared<LinkInstance>();
            instance->klass = klass;

            FuncDecl* init = findMethod(klass.get(), "init");
            if (init) {
                std::vector<Obj> args;
                for (auto& arg : newExpr->args) args.push_back(evaluateExpr(arg.get()));

                auto prevEnv = currentEnv;
                currentEnv = std::make_shared<Environment>(globalEnv.get());
                
                currentEnv->define("this", Obj(instance));
                for (size_t i = 0; i < init->params.size(); ++i) {
                     if (i < args.size()) currentEnv->define(init->params[i], args[i]);
                }

                try {
                    for (auto& s : init->body) runStatement(s.get());
                } catch (const ReturnException&) {} // Init tidak mengembalikan nilai
                
                currentEnv = prevEnv;
            }
            return Obj(instance);
        }

        if (dynamic_cast<ThisExpr*>(expr)) {
            return currentEnv->get("this");
        }
        if (auto get = dynamic_cast<GetExpr*>(expr)) {
            Obj obj = evaluateExpr(get->object.get());
            if (std::holds_alternative<std::shared_ptr<LinkInstance>>(obj.as)) {
                auto instance = std::get<std::shared_ptr<LinkInstance>>(obj.as);
                if (instance->fields.count(get->name)) {
                    return instance->fields[get->name];
                }
                return Obj(); 
            }
            return Obj();
        }

        if (auto set = dynamic_cast<SetExpr*>(expr)) {
            Obj obj = evaluateExpr(set->object.get());
            if (std::holds_alternative<std::shared_ptr<LinkInstance>>(obj.as)) {
                auto instance = std::get<std::shared_ptr<LinkInstance>>(obj.as);
                Obj val = evaluateExpr(set->value.get());
                instance->fields[set->name] = val; 
                return val;
            }
            std::cout << "Runtime Error: Only instances have fields.\n";
            return Obj();
        }

        if (auto methodCall = dynamic_cast<MethodCallExpr*>(expr)) {
            Obj obj = evaluateExpr(methodCall->object.get());
            if (!std::holds_alternative<std::shared_ptr<LinkInstance>>(obj.as)) {
                std::cout << "Runtime Error: Method call on non-instance.\n";
                return Obj();
            }

            auto instance = std::get<std::shared_ptr<LinkInstance>>(obj.as);
            FuncDecl* method = findMethod(instance->klass.get(), methodCall->method);
            
            if (!method) {
                std::cout << "Runtime Error: Method '" << methodCall->method << "' not found.\n";
                return Obj();
            }

            std::vector<Obj> args;
            for (auto& arg : methodCall->args) args.push_back(evaluateExpr(arg.get()));

            auto prevEnv = currentEnv;
            currentEnv = std::make_shared<Environment>(globalEnv.get());

            currentEnv->define("this", Obj(instance));
            for (size_t i = 0; i < method->params.size(); ++i) {
                if (i < args.size()) currentEnv->define(method->params[i], args[i]);
            }

            try {
                for (auto& s : method->body) runStatement(s.get());
            } catch (const ReturnException& e) {
                currentEnv = prevEnv;
                return e.value;
            }

            currentEnv = prevEnv;
            return Obj();
        }

        if (auto call = dynamic_cast<CallExpr*>(expr)) {
			
			if (call->func == "time.sleep") {
                if (call->args.empty()) return Obj();

                Obj val = evaluateExpr(call->args[0].get());
                int ms = 0;
                
                if (std::holds_alternative<int>(val.as)) ms = std::get<int>(val.as);
                else if (std::holds_alternative<double>(val.as)) ms = (int)std::get<double>(val.as);

                std::this_thread::sleep_for(std::chrono::milliseconds(ms));
                return Obj();
            }

            if (call->func == "range") {
                if (call->args.empty()) return Obj();
                Obj limitObj = evaluateExpr(call->args[0].get());
                int limit = 0;
                if (std::holds_alternative<int>(limitObj.as)) limit = std::get<int>(limitObj.as);
                auto list = std::make_shared<List>();
                for (int i = 0; i < limit; i++) list->push_back(Obj(i));
                return Obj(list);
            }
            if (call->func == "len" || call->func == "str.len") {
                if (call->args.empty()) return Obj(0);
                Obj target = evaluateExpr(call->args[0].get());
                if (std::holds_alternative<std::string>(target.as)) return Obj((int)std::get<std::string>(target.as).length());
                if (std::holds_alternative<std::shared_ptr<List>>(target.as)) return Obj((int)std::get<std::shared_ptr<List>>(target.as)->size());
                return Obj(0);
            }
            if (call->func == "io.read") {
                if (call->args.empty()) return Obj("");
                Obj pathObj = evaluateExpr(call->args[0].get()); 
                if (std::holds_alternative<std::string>(pathObj.as)) {
                    std::string path = std::get<std::string>(pathObj.as);
                    if (!Sys::fileExists(path)) throw RuntimeException("File not found: " + path);
                    return Obj(Sys::readFile(path));
                }
                return Obj("");
            }
            if (call->func == "io.exists") {
                if (call->args.empty()) return Obj(false);
                Obj path = evaluateExpr(call->args[0].get());
                if (std::holds_alternative<std::string>(path.as)) return Obj(Sys::fileExists(std::get<std::string>(path.as)));
                return Obj(false);
            }
            if (call->func == "os.exec") {
                if (call->args.empty()) return Obj("");
                Obj cmd = evaluateExpr(call->args[0].get());
                if (std::holds_alternative<std::string>(cmd.as)) return Obj(Sys::exec(std::get<std::string>(cmd.as).c_str()));
                return Obj("");
            }
            if (call->func == "os.getenv") {
                if (call->args.empty()) return Obj(""); 
                Obj key = evaluateExpr(call->args[0].get());
                if (std::holds_alternative<std::string>(key.as)) return Obj(Sys::getEnv(std::get<std::string>(key.as))); 
                return Obj(""); 
            }

            if (call->func == "str.trim") {
                if (call->args.empty()) return Obj("");
                Obj s = evaluateExpr(call->args[0].get());
                if (std::holds_alternative<std::string>(s.as)) return Obj(SysString::trim(std::get<std::string>(s.as)));
                return s;
            }
            if (call->func == "str.replace") {
                if (call->args.size() < 3) return Obj("");
                Obj s = evaluateExpr(call->args[0].get());
                Obj t = evaluateExpr(call->args[1].get());
                Obj r = evaluateExpr(call->args[2].get());
                if (std::holds_alternative<std::string>(s.as) && std::holds_alternative<std::string>(t.as) && std::holds_alternative<std::string>(r.as))
                    return Obj(SysString::replace(std::get<std::string>(s.as), std::get<std::string>(t.as), std::get<std::string>(r.as)));
                return s;
            }
            if (call->func == "str.split") {
                 if (call->args.size() < 2) return Obj(std::make_shared<List>());
                 Obj s = evaluateExpr(call->args[0].get());
                 Obj d = evaluateExpr(call->args[1].get());
                 auto list = std::make_shared<List>();
                 if (std::holds_alternative<std::string>(s.as) && std::holds_alternative<std::string>(d.as)) {
                     auto vec = SysString::split(std::get<std::string>(s.as), std::get<std::string>(d.as));
                     for(const auto& v : vec) list->push_back(Obj(v));
                 }
                 return Obj(list);
            }
            if (call->func == "str.merge") {
                if (call->args.size() < 2) return Obj("");
                Obj l = evaluateExpr(call->args[0].get());
                Obj d = evaluateExpr(call->args[1].get());
                if (std::holds_alternative<std::shared_ptr<List>>(l.as) && std::holds_alternative<std::string>(d.as)) {
                    std::vector<std::string> vs;
                    auto lst = std::get<std::shared_ptr<List>>(l.as);
                    for(auto& i : *lst) {
                        if(std::holds_alternative<std::string>(i.as)) vs.push_back(std::get<std::string>(i.as));
                        else if(std::holds_alternative<int>(i.as)) vs.push_back(std::to_string(std::get<int>(i.as)));
                        else if(std::holds_alternative<double>(i.as)) vs.push_back(std::to_string(std::get<double>(i.as)));
                    }
                    return Obj(SysString::merge(vs, std::get<std::string>(d.as)));
                }
                return Obj("");
            }
            if (call->func == "str.contains") {
                if (call->args.size() < 2) return Obj(false);
                Obj h = evaluateExpr(call->args[0].get());
                Obj n = evaluateExpr(call->args[1].get());
                if (std::holds_alternative<std::string>(h.as) && std::holds_alternative<std::string>(n.as)) 
                    return Obj(Sys::contains(std::get<std::string>(h.as), std::get<std::string>(n.as)));
                return Obj(false);
            }

            auto getDouble = [&](Expr* e) -> double {
                Obj o = evaluateExpr(e);
                if (std::holds_alternative<double>(o.as)) return std::get<double>(o.as);
                if (std::holds_alternative<int>(o.as)) return (double)std::get<int>(o.as);
                return 0.0;
            };

            if (call->func == "math.pi") return Obj(SysMath::pi());
            if (call->func == "math.sin") return Obj(SysMath::sin(getDouble(call->args[0].get())));
            if (call->func == "math.cos") return Obj(SysMath::cos(getDouble(call->args[0].get())));
            if (call->func == "math.tan") return Obj(SysMath::tan(getDouble(call->args[0].get())));
            if (call->func == "math.sqrt") return Obj(SysMath::sqrt(getDouble(call->args[0].get())));
            if (call->func == "math.abs") return Obj(SysMath::abs(getDouble(call->args[0].get())));
            if (call->func == "math.pow") {
                 if (call->args.size() >= 2) return Obj(SysMath::pow(getDouble(call->args[0].get()), getDouble(call->args[1].get())));
                 return Obj(0.0);
            }

            if (functionRegistry.count(call->func)) {
                FuncDecl* fn = functionRegistry[call->func];
                if (call->args.size() != fn->params.size()) {
                    std::cout << "Runtime Error: Function " << fn->name << " mismatch args.\n";
                    return Obj();
                }

                std::vector<Obj> argValues;
                for (auto& arg : call->args) argValues.push_back(evaluateExpr(arg.get()));

                auto previousEnv = currentEnv;
                currentEnv = std::make_shared<Environment>(globalEnv.get());

                for (size_t i = 0; i < fn->params.size(); ++i) {
                    currentEnv->define(fn->params[i], argValues[i]);
                }

                try {
                    for (auto& s : fn->body) runStatement(s.get());
                } catch (const ReturnException& e) {
                    currentEnv = previousEnv; 
                    return e.value;
                }

                currentEnv = previousEnv;
                return Obj();
            }

            return Obj(); 
        }

        if (auto inp = dynamic_cast<InputExpr*>(expr)) {
            if (!inp->prompt.empty()) std::cout << inp->prompt; 
            std::string line;
            if (!std::getline(std::cin, line)) return Obj();
            try {
                if (line.find('.') != std::string::npos) return Obj(std::stod(line)); 
                return Obj(std::stoi(line)); 
            } catch(...) { return Obj(line); } 
        }

        if (auto bin = dynamic_cast<BinaryExpr*>(expr)) {
            Obj left = evaluateExpr(bin->lhs.get());   
            Obj right = evaluateExpr(bin->rhs.get());  
            
            if (std::holds_alternative<std::string>(left.as)) {
                std::string sLeft = std::get<std::string>(left.as);
                std::string sRight = "";
                if (std::holds_alternative<std::string>(right.as)) sRight = std::get<std::string>(right.as);
                else if (std::holds_alternative<int>(right.as)) sRight = std::to_string(std::get<int>(right.as));
                else if (std::holds_alternative<double>(right.as)) {
                    std::ostringstream oss;
                    oss << std::get<double>(right.as);
                    sRight = oss.str();
                }
                if (bin->op == '+') return Obj(sLeft + sRight);
            }

            if (std::holds_alternative<int>(left.as) && std::holds_alternative<int>(right.as)) {
                int l = std::get<int>(left.as), r = std::get<int>(right.as);
                switch (bin->op) {
                    case '+': return Obj(l + r); case '-': return Obj(l - r);
                    case '*': return Obj(l * r); case '/': return Obj((r != 0) ? l / r : 0);
                    case '<': return Obj(l < r); case '>': return Obj(l > r); case '=': return Obj(l == r);
                }
            } else if ((std::holds_alternative<double>(left.as)||std::holds_alternative<int>(left.as)) && (std::holds_alternative<double>(right.as)||std::holds_alternative<int>(right.as))) {
                double l = std::holds_alternative<int>(left.as)?std::get<int>(left.as):std::get<double>(left.as);
                double r = std::holds_alternative<int>(right.as)?std::get<int>(right.as):std::get<double>(right.as);
                switch (bin->op) {
                    case '+': return Obj(l + r); case '-': return Obj(l - r);
                    case '*': return Obj(l * r); case '/': return Obj((r != 0.0) ? l / r : 0.0);
                    case '<': return Obj(l < r); case '>': return Obj(l > r); case '=': return Obj(l == r);
                }
            } else if (std::holds_alternative<std::string>(left.as) && std::holds_alternative<std::string>(right.as)) {
                if (bin->op == '=') return Obj(std::get<std::string>(left.as) == std::get<std::string>(right.as));
            }
        }
        return Obj();
    }

    void runStatement(Stmt* stmt) {
        if (!stmt) return;
        
        if (dynamic_cast<ClearStmt*>(stmt)) {
            #ifdef _WIN32 
            system("cls"); 
            #else 
            system("clear"); 
            #endif
            return; 
        }

        if (auto cls = dynamic_cast<ClassDecl*>(stmt)) {
            auto klass = std::make_shared<LinkClass>();
            klass->name = cls->name;
            for (auto& method : cls->methods) {
                klass->methods[method->name] = method.get();
            }
            currentEnv->define(cls->name, Obj(klass));
            return;
        }

        if (auto exprStmt = dynamic_cast<ExprStmt*>(stmt)) {
            evaluateExpr(exprStmt->expression.get());
            return;
        }

        if (auto tryStmt = dynamic_cast<TryStmt*>(stmt)) {
            try {
                for (auto& s : tryStmt->tryBody) runStatement(s.get());
            } catch (const RuntimeException& e) {
                auto prevEnv = currentEnv;
                currentEnv = std::make_shared<Environment>(prevEnv.get());
                currentEnv->define(tryStmt->errorVar, Obj(e.message));
                for (auto& s : tryStmt->catchBody) runStatement(s.get());
                currentEnv = prevEnv;
            }
            return;
        }
        
        if (auto ret = dynamic_cast<ReturnStmt*>(stmt)) {
			Obj result; 
			if (ret->value) result = evaluateExpr(ret->value.get()); 
			throw ReturnException(result); 
		}
        
        if (auto set = dynamic_cast<SetStmt*>(stmt)) {
            currentEnv->define(set->name, evaluateExpr(set->expression.get())); 
            return;
        }

        if (auto func = dynamic_cast<FuncDecl*>(stmt)) {
            functionRegistry[func->name] = func;
            return;
        }
        
        
        if (auto call = dynamic_cast<CallStmt*>(stmt)) {
			
			if (call->func == "time.sleep") {
                if (!call->args.empty()) {
                    Obj val = evaluateExpr(call->args[0].get());
                    int ms = 0;
                    if (std::holds_alternative<int>(val.as)) ms = std::get<int>(val.as);
                    else if (std::holds_alternative<double>(val.as)) ms = (int)std::get<double>(val.as);
                    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
                }
                return; 
            }
			
            if (call->func == "io.write" || call->func == "io.append") { 
                if (call->args.size() < 2) return;
                Obj pathObj = evaluateExpr(call->args[0].get());
                Obj contentObj = evaluateExpr(call->args[1].get());
                if (std::holds_alternative<std::string>(pathObj.as)) {
                    std::string path = std::get<std::string>(pathObj.as);
                    std::string content = "";
                    if (std::holds_alternative<std::string>(contentObj.as)) content = std::get<std::string>(contentObj.as);
                    else if (std::holds_alternative<int>(contentObj.as)) content = std::to_string(std::get<int>(contentObj.as));
                    else if (std::holds_alternative<double>(contentObj.as)) content = std::to_string(std::get<double>(contentObj.as));
                    Sys::writeFile(path, content, (call->func == "io.append"));
                }
                return; 
            }
            if (call->func == "os.setenv") { 
                if (call->args.size() < 2) return;
                Obj key = evaluateExpr(call->args[0].get());
                Obj val = evaluateExpr(call->args[1].get()); 
                if (std::holds_alternative<std::string>(key.as) && std::holds_alternative<std::string>(val.as)) {
                    Sys::setEnv(std::get<std::string>(key.as), std::get<std::string>(val.as)); 
                }
                return;
            }
            if (call->func == "io.remove") { 
                if (call->args.empty()) return;
                Obj pathObj = evaluateExpr(call->args[0].get());
                if (std::holds_alternative<std::string>(pathObj.as)) Sys::removeFile(std::get<std::string>(pathObj.as));
                return;
            }
            if (call->func == "list.add") { 
                if (call->args.size() < 2) return;
                Obj target = evaluateExpr(call->args[0].get());
                Obj item = evaluateExpr(call->args[1].get());
                if (std::holds_alternative<std::shared_ptr<List>>(target.as)) {
                    std::get<std::shared_ptr<List>>(target.as)->push_back(item);
                }
                return;
            }
            if (call->func == "print") { 
                if (!call->args.empty()) printObj(evaluateExpr(call->args[0].get())); 
                std::cout << "\n";
                return;
            }
            if (call->func == "os.exec") { 
                if (!call->args.empty()) {
                    Obj cmd = evaluateExpr(call->args[0].get());
                    if (std::holds_alternative<std::string>(cmd.as)) system(std::get<std::string>(cmd.as).c_str());
                }
                return;
            }
            
            if (functionRegistry.count(call->func)) { 
                FuncDecl* fn = functionRegistry[call->func];
                if (call->args.size() != fn->params.size()) {
                    std::cout << "Runtime Error: Function " << fn->name << " mismatch args.\n";
                    return; 
                } 

                std::vector<Obj> argValues;
                for (auto& arg : call->args) argValues.push_back(evaluateExpr(arg.get()));
       
                auto previousEnv = currentEnv;
                currentEnv = std::make_shared<Environment>(globalEnv.get());
                
                for (size_t i = 0; i < fn->params.size(); ++i) {
                    currentEnv->define(fn->params[i], argValues[i]);
                }
                
                try {
					for (auto& s : fn->body) runStatement(s.get()); 
				} catch (const ReturnException&) {
					currentEnv = previousEnv; 
					return; 
				}
                currentEnv = previousEnv; 
                return;
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
        if (auto loop = dynamic_cast<ForStmt*>(stmt)) {
             Obj collection = evaluateExpr(loop->collection.get());
             if (std::holds_alternative<std::shared_ptr<List>>(collection.as)) {
                 auto list = std::get<std::shared_ptr<List>>(collection.as);
                 currentEnv->define(loop->iteratorName, Obj(0)); 
                 for (auto& item : *list) {
                     currentEnv->assign(loop->iteratorName, item); 
                     for (auto& s : loop->body) runStatement(s.get());
                 }
             } else {
                 std::cout << "Runtime Error: 'For' loop expects a list/range.\n";
             }
             return;
        }
        if (auto up = dynamic_cast<UpdateStmt*>(stmt)) {
            Obj val = currentEnv->get(up->name);
            if (std::holds_alternative<int>(val.as)) {
                currentEnv->assign(up->name, Obj(std::get<int>(val.as) + 1));
            }
            return;
        }
        if (auto prop = dynamic_cast<PropertyStmt*>(stmt)) {
            if (prop->name == "sh") {
                int status = system(prop->value.c_str()); (void)status; 
            }
            return;
        }
        if (auto app = dynamic_cast<AppDecl*>(stmt)) {
            for (auto& s : app->body) runStatement(s.get());
            return;
        }
        
        // Import Logic (SAFE POINTER VERSION)
        if (auto imp = dynamic_cast<ImportStmt*>(stmt)) {
             std::string path = imp->path;
             if (!Sys::fileExists(path)) {
                 std::cout << "Runtime Error: Cannot import '" << path << "'. File not found.\n";
                 return;
             }
             std::string source = Sys::readFile(path);
             Lexer lexer(source);
             auto tokens = lexer.tokenize();
             Parser parser(tokens);
             auto importedProgram = parser.parse();
             
             if (importedProgram) {
                 loadedPrograms.push_back(std::move(importedProgram));
                 Program* storedProgram = loadedPrograms.back().get();
                 for (auto& s : storedProgram->statements) {
                     runStatement(s.get());
                 }
             }
             return;
        }
	}
};
