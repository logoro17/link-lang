LINK

Link is an experimental scripting language for Nebula OS, designed to create apps, GUIs, and handle package management.
This is a v0.x dev build. Early, minimal, and primarily for contributors and testing.

-----

Current Status:

- Lexer: tokenizes keywords, identifiers, and strings
- Parser: parses scripts and builds an AST
- AST: minimal representation for apps and functions
- Execution/runtime: X not implemented yet

-----

Getting started

1. Clone this repo:

bash
git clone https://github.com/lorogo12/link-lang.git
cd link-lang

2. Build Link (G++):

g++ src/*.cpp -I include -o link

(Other build instructions for different compilers will be added later)

3. Run an example

./link examples/hello.link

-----

Contributing

This is an experimental project. Please avoid redesigning syntax or architecture without discussion.

Good first tasks:

- Expand the lexer (number literals, expansion on comments)
- Expand the AST nodes in the parser
- Improve parser error messages

-----

License

MIT License. See the LICENSE file for details.

-----

Notes

Eventually, this will be developed alongside Nebula OS. Expect breaking changes, experimental features, and
rapid iteration. Your contributions are welcome and appreciated!
