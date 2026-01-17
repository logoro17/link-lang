#!/bin/bash
clear

echo "--- Compiling in Progress ---"

rm -f error.txt 
rm -f link 

g++ -O3 \
    src/main.cpp \
    src/lexer.cpp \
    src/parser.cpp \
    src/help.cpp \
    src/os.cpp \
    src/link_str.cpp \
    src/link_math.cpp \
    -I include \
    -o link \
    2> error.txt 

if [ -s error.txt ]; then
    echo " !!! There is an Error Dude! Showing error.txt:"
    echo "-----------------------------------"
    cat error.txt
else
    echo "Compiled Success!!!"
    echo "Try running: <file.link>"
fi
