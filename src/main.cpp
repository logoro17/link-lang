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

    if (s.rfind("if", 0) == 0) return true;
    if (s.rfind("while", 0) == 0) return true;
    if (s.rfind("for", 0) == 0) return true;
    if (s.rfind("func", 0) == 0) return true;
    if (s.rfind("app", 0) == 0) return true;
    if (s.rfind("window", 0) == 0) return true;
    if (s.rfind("elif", 0) == 0) return true;
    if (s.rfind("else", 0) == 0) return true;
    
    return false;
}

void run(Runtime& runtime, const std::string& source) {
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto program = parser.parse();
        runtime.execute(program.get());
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main(int argc, char** argv) {
    Runtime runtime; 

    // Panggil fungsi dari help.cpp
    if (argc == 2 && std::string(argv[1]) == "--help") {
        printHelp();
        return 0;
    }

    // INTERACTIVE MODE
    if (argc < 2) {
        std::cout << "NebulaOS Link-Lang v0.2 (Interactive)" << std::endl;
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
                    if (!line.empty()) run(runtime, line);
                }
            } else {
                if (line.empty()) {
                    run(runtime, inputBuffer);
                    inputBuffer.clear();
                } else {
                    inputBuffer += line + "\n";
                }
            }
        }
        return 0;
    }

    // FILE MODE
    std::ifstream file(argv[1]);
    if(!file){
        std::cout << "No such file or directory: " << argv[1] << std::endl;
        return 1;
    }

    std::string source((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    run(runtime, source);

    return 0;        
}