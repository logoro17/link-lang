#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "lexer.h"
#include "parser.h"
#include "runtime.cpp" 
#include "help.h"

bool isBlockStart(const std::string& line) {
    size_t start = line.find_first_not_of(" \t");
    if (start == std::string::npos) return false; 
    
    std::string s = line.substr(start);

    size_t end = s.find_last_not_of(" \t");
    if (end != std::string::npos) {
        s = s.substr(0, end + 1);
    }

    if (s.rfind("if", 0) == 0) return true;
    if (s.rfind("while", 0) == 0) return true;
    if (s.rfind("for", 0) == 0) return true;
    if (s.rfind("func", 0) == 0) return true;
    if (s.rfind("app", 0) == 0) return true;
    if (s.rfind("window", 0) == 0) return true;
    if (s.rfind("elif", 0) == 0) return true;
    if (s.rfind("else", 0) == 0) return true;
    
    if (!s.empty() && s.back() == '{') return true;
    if (!s.empty() && s.back() == '[') return true;
    
    return false;
}

void run(Runtime& runtime, const std::string& source, bool isDebug) {
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto program = parser.parse(); 

        if (isDebug) {
            std::cout << "\n--- DEBUG: AST STRUCTURE ---\n";
            program->print();
            std::cout << "----------------------------\n";
        }

        runtime.execute(std::move(program)); 

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main(int argc, char** argv) {
    Runtime runtime; 

    if (argc == 2 && std::string(argv[1]) == "--help") {
        printHelp();
        return 0;
    }

    bool debugMode = false;
    for(int i=1; i<argc; i++) {
        if(std::string(argv[i]) == "--debug") {
            debugMode = true;
            break;
        }
    }

    if (argc < 2 || (argc == 2 && debugMode)) {
        std::cout << "NebulaOS Link-Lang v0.2 (Interactive)" << std::endl;
        if (debugMode) std::cout << "[DEBUG MODE ACTIVE]" << std::endl;
        std::cout << "Type 'exit' or './link --help'" << std::endl;
        
        std::string inputBuffer;
        std::string line;

        while (true) {
            if (inputBuffer.empty()) std::cout << "link> ";
            else std::cout << "... ";

            if (!std::getline(std::cin, line)) break;
            if (line == "exit") break;

            if (inputBuffer.empty()) {
                if (isBlockStart(line)) {
                    inputBuffer += line + "\n";
                    continue; 
                } else {
                    if (!line.empty()) run(runtime, line, debugMode);
                }
            } else {
                if (line.empty()) {
                    run(runtime, inputBuffer, debugMode);
                    inputBuffer.clear();
                } else {
                    inputBuffer += line + "\n";
                }
            }
        }
        return 0;
    }

    std::string filename;
    for(int i=1; i<argc; i++) {
        std::string arg = argv[i];
        if (arg != "--debug") {
            filename = arg;
            break;
        }
    }

    if (filename.empty()) {
        std::cout << "Error: No file specified." << std::endl;
        return 1;
    }

    std::ifstream file(filename);
    if(!file){
        std::cout << "No such file or directory: " << filename << std::endl;
        return 1;
    }

    std::string source((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    run(runtime, source, debugMode);

    return 0;        
}
