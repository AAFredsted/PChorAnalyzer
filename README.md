# PChorAnalyzer

**PChorAnalyzer** is a static code analyzer seeking to validate c++ code against a formal choreography description written in a DSL.

The Choreography DSL was inspired by the following papers:

- [A Comparative Study of Graph-Based and Constraint-Based Approaches for Program Analysis (arXiv:1208.6483)](https://arxiv.org/pdf/1208.6483)
- [A Unified Static Analysis Framework Based on Universal Program Representations (DOI:10.1145/2827695)](https://dl.acm.org/doi/10.1145/2827695)

```ebnf
<choreography> ::= <declaration>* <global_type_declaration>

<declaration> ::= <index_decl>
                | <participant_decl>
                | <channel_decl>
                | <label_decl>
                | <global_type_declaration>

<index_decl> ::= "Index" <identifier> "=" "{" <range> "}"
<range> ::= <number> | <number> ".." <number> | <number> ".." "n"

<participant_decl> ::= "Participant" <identifier>
                    |"Participant" <identifier> "{" <index_expr> "}"

<channel_decl> ::= "Channel" <identifier>
                 | "Channel" <identifier> "{" <index_expr> "}"

<label_decl> ::= "Label" <identifier> "{" <label_list> "}"
<label_list> ::= <identifier> ("," <identifier>)*

<global_type_declaration> ::= <identifier> "=" <global_type>

<global_type> ::= <interaction>
                | <recursion>
                | <branching>
                | <foreach>
                | <if_else>
                | <composition>
                | <end>

<interaction> ::= <participant> "->" <participant> ":" <channel> "<" <type> ">" ("{" <global_type> "}")?

<recursion> ::= "Rec" <identifier> "(" <index_binding> ")" "{" <global_type> "}"
<index_binding> ::= <identifier> ":"( <index_expr> | <type> ) ("," <identifier> ":" ( <index_expr> | <type> ))*

<foreach> ::= "foreach" "(" <identifier> ":" <index_expr> ")" "{" <global_type> "}"

<if_else> ::= "if" <condition> "then" "{" <global_type> "}" "else" "{" <global_type> "}"

<branching> ::= <label> "::" <identifier> ":" <interaction> "." <global_type>
              | <label> "::" <identifier> ":" <interaction> "|" <global_type>
              | <label> "::" <identifier> ":" <end>

<composition> ::= <linear_composition> | <parallel_composition>
<linear_composition> ::= <global_type> "." <global_type>
<parallel_composition> ::= <global_type> "|" <global_type>

<participant> ::= <identifier>
                | <identifier> "[" <index_expr> "]"

<channel> ::= <identifier>
            | <identifier> "[" <index_expr> "]"

<type> ::= "nat" | "bool" | <identifier>

<index_expr> ::= <identifier>
               | <number>
               | <number> ".." <number>

<label> ::= <identifier>

<condition> ::= <expression>
<expression> ::= <identifier> <operator> <value>
<value> ::= <number> | <identifier>

<comment> ::= "#" <text>
```

## Usage of Plugin

PChorAnalyzer uses Clang's ASTActions to inspect your source file during compilation. 
Once the .so-file has been built, you can link it in your compilation process as follows.

```bash
clang++ -std=c++23 -Xclang -load -Xclang ../build/libPchorAnalyzerPlugin.so  -Xclang -plugin-arg-PchorAnalyzer -Xclang --cor=<path_to_cor-file> -c test.cpp -o test
```

Debug Output can be accessed with the following command setup:

```bash
clang++ -std=c++23 -Xclang -load -Xclang ../build/libPchorAnalyzerPlugin.so  -Xclang -plugin-arg-PchorAnalyzer -Xclang --cor=<path_to_cor-file> -Xclang -plugin-arg-PchorAnalyzer -Xclang --debug -c test.cpp -o test
```

---

## Prerequisites

- **Compiler**: Clang with C++23 support.
- **Build System**: CMake (v3.x or higher recommended).
- **LLVM/Clang Development**: Ensure that the relevant LLVM/Clang headers and libraries are installed.

---

## Installation and Build

Clone the Repository:

```bash
git clone <repository-url>
cd PChorAnalyzer
```

Configure and Build:

```bash
mkdir build
cd build
cmake ..
make
```

This generates the plugin shared library (e.g., `libPchorAnalyzerPlugin.so`) in the build directory.

---

## License

MIT License

---

## Acknowledgements

- **Clang Community**: Thanks to the Clang documentation and community examples that provided guidance.
- **Research References**:
  - [A Comparative Study of Graph-Based and Constraint-Based Approaches for Program Analysis](https://arxiv.org/pdf/1208.6483)
  - [A Unified Static Analysis Framework Based on Universal Program Representations](https://dl.acm.org/doi/10.1145/2827695)
- Developed as part of my master’s thesis.

---

## Contact

For questions or further discussion, please contact **[AAFredsted]** at **[Aandr@itu.dk]** or open an issue on GitHub.

