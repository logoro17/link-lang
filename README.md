```text
██╗     ██╗███╗   ██╗██╗  ██╗           ██╗      █████╗ ███╗   ██╗ ██████╗ 
██║     ██║████╗  ██║██║ ██╔╝           ██║     ██╔══██╗████╗  ██║██╔════╝ 
██║     ██║██╔██╗ ██║█████╔╝  █████╗    ██║     ███████║██╔██╗ ██║██║  ███╗
██║     ██║██║╚██╗██║██╔═██╗  ╚════╝    ██║     ██╔══██║██║╚██╗██║██║   ██║
███████╗██║██║ ╚████║██║  ██╗           ███████╗██║  ██║██║ ╚████║╚██████╔╝
╚══════╝╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝           ╚══════╝╚═╝  ╚═╝╚═╝  ╚═══╝ ╚═════╝ 

---------------------------------------------------------------------------
Creator   : Pilot0253
Developer : Lowis Roy Figo
PROJECT   : LinkLang (Interpreter)
LANGUAGE  : C++ / Python
OS        : Kali Linux / Android (Custom Kernel)
STATUS    : 🚀 Active Development
```
**Link-Lang** is a dynamic, interpreted programming language built for **NebulaOS**. It features a clean, Python-like syntax based on indentation, designed for simplicity and readability. Built from scratch using C++ (Lexer, Parser, AST, Runtime).

## ✨ Features

Link-Lang has evolved from a simple math parser into a fully functional scripting language.

### 🧠 Core Capabilities
*   **Dynamic Typing:** Supports Integer, Float, String, Char, and Boolean.
*   **Math Engine:** Full support for `+`, `-`, `*`, `/` with operator precedence (PEMDAS) and parentheses `()`.
*   **Logic & Comparison:** Support for `>`, `<`, `==` operators.
*   **Control Flow:**
    *   `if`, `elif`, `else` conditionals (recursive parsing).
    *   `while` loops.
    *   `for` loops (basic numeric range).
*   **I/O Operations:** Built-in `print()` and `input()`.
*   **Indentation-Based:** Blocks are defined by whitespace (no `{}` or `;` required).
*   **Comments:** Use `#` for single-line comments.

### 🛠️ Runtime Modes
1.  **File Mode:** Execute `.link` script files.
2.  **Interactive REPL:** A smart shell that supports multi-line blocks (Shift+Enter logic).

---

## 📦 Installation & Build

Ensure you have a C++ compiler installed (G++ recommended).

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/Pilot0253/link-lang.git
    cd link-lang
    ```

2.  **Compile the source code:**
    You can use the provided build script:
    ```bash
    ./compile-link.sh
    ```
    *Or manually:*
    ```bash
    g++ -std=c++17 -o link src/*.cpp -I include
    ```

---

## 🚀 Usage

### Running a Script
To run a file ending in `.link`:
```bash
./link examples/test.link


