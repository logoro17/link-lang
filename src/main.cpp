#include <fstream>
#include <iostream>
#include "lexer.h"
#include "parser.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: link <file.link>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    std::string source((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    auto program = parser.parse(); // PARSE YOU PARSER.
                            
    program->print();
    return 0;

    //Just for debugging, not needed.
    //for (auto& t : tokens) {
    //    std::cout << (int)t.type << " [" << t.value << "] "
    //              << "line " << t.line << "\n";
    //}
}

