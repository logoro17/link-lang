#include "help.h"
#include <iostream>

void printHelp() {
    std::cout << R"(

========================================================
           NEBULA OS LINK-LANG v0.2.2a (Alpha) HELP
========================================================
History:
Created by   : Pilot0253
Developed by : logoro17 // LoRoGo17 

USAGE:
  ./link                  : Enter Interactive Mode (REPL).
  ./link <file.link>      : Execute a Link-Lang script file.
  ./compile-link.sh       : Recompile source code.

DATA TYPES:
  Integer  : 10, 25, -5
  Float    : 3.14, 2.5
  String   : "Hello", 'World'
  Char     : 'A', 'x'
  Boolean  : true, false

BASIC COMMANDS:
  set x = 10              : Variable declaration.
  print(x)                : Output to screen.
  input("Message: ")      : Get user input.

MATHEMATICAL OPERATORS:
  +  : Addition (or String concatenation).
  -  : Subtraction.
  * : Multiplication.
  /  : Division.

LOGIC & COMPARISON:
  >  : Greater than.
  <  : Less than.
  == : Equal to.

CONTROL FLOW:
  Execute code blocks using indentation (spaces/tabs).

  1. CONDITIONAL (IF-ELIF-ELSE)
     if x > 10
         print("Large")
     elif x > 5
         print("Medium")
     else
         print("Small")

  2. LOOPING (WHILE)
     while x < 5
         print(x)
         set x = x + 1

  3. LOOPING (FOR)
     for i in range(5)
         print(i)

SYSTEM COMMANDS:
  sh "clear"              : Execute Linux terminal commands.
  sh "ls -la"             : Another example.

INTERACTIVE MODE:
  - Press Enter once for standard command lines.
  - When writing blocks (if/while), press Enter twice (empty line)
    to execute the block.
========================================================

)" << std::endl;
}   