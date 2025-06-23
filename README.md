# PChorAnalyzer

**PChorAnalyzer** is a static code analyzer that validates C++ code against formal choreography descriptions written in a domain-specific language (DSL) called Pchor.

---

## Table of Contents

- [PChorAnalyzer](#pchoranalyzer)
  - [Table of Contents](#table-of-contents)
  - [Overview](#overview)
  - [Architecture](#architecture)
  - [Choreography DSL Grammar](#choreography-dsl-grammar)
  - [Usage](#usage)
  - [Prerequisites](#prerequisites)
  - [Installation and Build](#installation-and-build)
  - [Test Environment](#test-environment)
  - [License](#license)
  - [Acknowledgements](#acknowledgements)
  - [Contact](#contact)

---

## Overview

PChorAnalyzer leverages Clang’s plugin infrastructure to perform static analysis of C++ source files, ensuring conformance to a choreography specified in a `.cor` file. The Pchor DSL was inspired by:

- [A Comparative Study of Graph-Based and Constraint-Based Approaches for Program Analysis (arXiv:1208.6483)](https://arxiv.org/pdf/1208.6483)
- [A Unified Static Analysis Framework Based on Universal Program Representations (DOI:10.1145/2827695)](https://dl.acm.org/doi/10.1145/2827695)

---

## Architecture
Pchor consists of a PchorCore and PchorAnalyzerPlugin component.

* PchorCore: Responsible for Frontend compilation framework and provides abstract classes to provide domain-specific static analysis on top. 
* PchorAnalyzerPlugin: Implements static analysis using output of PchorCore

The frontend compilation framework for Pchor draws heavy inspiration from the Clang-compiler, while
the PchorAnalyzerPlugin uses similar design patterns to clang-tidy to implement static code analysis.

The following diagram illustrates the high-level architecture and workflow of PChorAnalyzer:

![PChorAnalyzer Architecture](pchor_arc.png)

---

## Choreography DSL Grammar

Pchor supports `.cor` files written using the following grammar. Example choreographies can be found in the `/test` folder.

```ebnf
(* Overall program structure: a sequence of declarations 
   followed by a global type declaration*)
<Program> ::= <Decls> <GlobalDecl> ;


(* Declarations are ordered in a sequence of any length *)
<Decls> ::= <Decl>* ;

(* Each Declaration fits into one of four constructs *)
<Decl> ::= <IndexDecl>
         | <ParticipantDecl>
         | <ChannelDecl>
         | <GlobalDecl> ;

(* Index declaration with provided range *)
<IndexDecl> ::= "Index" <Namespace> "{" <Range> "}" ;

(* Participant declaration with optional index identifier, 
   if none provided "PchorUnaryIndex" is assumed *)
<ParticipantDecl> ::= "Participant" <Namespace> [ "{" <IndexIdentifier> "}" ] ;

(* Channel declaration with optional index identifier
   if none provided "PchorUnaryIndex" is assumed *)
<ChannelDecl> ::= "Channel" <Namespace> [ "{" <IndexIdentifier> "}" ] ;

(* Global type definition containing an expressionslist construct *)
<GlobalDecl> ::= <Namespace> "=" <ExpressionList> ;


(* Sequence of expressions terminated by "end". 
   An empty expression list is allowed via '. end'. *)
<ExpressionList> ::= <Expression>* { "." <Expression> } "." "end" ;


(* Three types of expressions can be chained together in an expression list *)
<Expression> ::= <CommunicationExpression>
               | <ForEachExpression>
               | <GlobalTypeName> ;


(* Iteration construct using index based semantics *)
<ForEachExpression> ::= "foreach" "(" <IterationExpression> ")" <Body> ;

(* Iteration body contains expressionlist *)
<Body> ::= "{" <ExpressionList> "}" ;


(* Iteration expression variants:
   - full:   i : I
   - min:    i < min(I)
   - max:    i > max(I) *)
<IterationExpression> ::= <MinExcludingIteration>
                        | <MaxExcludingIteration>
                        | <FullIteration> ;

<MinExcludingIteration> ::= Namespace "<" <MinExpr> ;
<MaxExcludingIteration> ::= Namespace ">" <MaxExpr> ;
<FullIteration> ::= Namespace ":" <IndexIdentifier> ;


(* Minimum/maximum operations over index ranges *)
<MinExpr> ::= "min" "(" <IndexIdentifier> ")" ;
<MaxExpr> ::= "max" "(" <IndexIdentifier> ")" ;


(* Communication between two participants over a typed channel *)
<CommunicationExpression> ::= <ParticipantExpression> "->" <ParticipantExpression> ":"
                              <ChannelExpression> <TypeExpression> ;


(* Participant/Channel usage with optional evaluation index.
   Literal "1" is assumed if none is provided *)
<ParticipantExpression> ::= <ParticipantIdentifier> [ "[" <EvaluationExpression> "]" ] ;
<ChannelExpression> ::= <ChannelIdentifier> [ "[" <EvaluationExpression> "]" ] ;


(* Arithmetic expressions to determine Participants for enpoint projection  *)
<EvaluationExpression> ::= <EvaluationTerm> { <BinaryArithmeticOperator> <EvaluationTerm> } ;

(* A term can be a literal, 
   an operation that is derived to be a literal 
   (i.e min and max), 
   an identifier for an iteration index inside a foreach expression 
   or another evaluation expression *)
<EvaluationTerm> ::= <Literal> 
                   | <IterationIdentifier>
                   | <MinExpr>
                   | <MaxExpr>
                   | "(" <EvaluationExpression> ")" ;

(* Arithmetic operations supported in Evaluation Expression *)
<BinaryArithmeticOperator> ::= "+" | "-" ;


(* Representation of payload in communication expression *)
<TypeExpression> ::= "<" <TypeName> ">" ;


(* Numeric range used in index declarations *)
<Range> ::= <Literal> ".." <Literal> ;


(* Identifier categories used in the language *)

(* Namespaces used in declarations; must be unique and valid identifiers *)
<Namespace> ::= Identifier ;

(* Refers to a previously declared Participant namespace *)
<ParticipantIdentifier> ::= Identifier ;

(* Refers to a previously declared Channel namespace *)
<ChannelIdentifier> ::= Identifier ;

(* Refers to a previously declared Index namespace *)
<IndexIdentifier> ::= Identifier ;

(* Refers to a previously declared Global type *)
<GlobalTypeName> ::= Identifier ;

(* Iteration variables used inside foreach constructs *)
<IterationIdentifier> ::= Identifier ;

(* Type labels: e.g., Data, Ping, Quote, etc. *)
<TypeName> ::= Identifier ;


(* Disallowed names that may not be used as new Namespace declarations.
   These are enforced during semantic parsing, not by the grammar itself. *)
<ProtectedIdentifiers> ::= "PchorUnaryIndex" ;


(* Terminal tokens provided by the lexer *)

(* User-defined indentifiers *)
<Identifier> ::= <Char> { <Char> | <Digit> }* ;

(* Keywords reserved by the grammar *)
<KeyWords> ::= "Channel" | "Participant" | "Index" | "foreach" | "min" | "max" ;

(* Characters allowed in identifiers *)
<Char> ::= ? any Unicode character excluding grammar symbols ? ;

(* Symbols used in grammar syntax *)
<Symbol> ::= "<" | ">" | "(" | ")" | "{" | "}" | "[" | "]"
             | "->" | "-" | "+" | ":" | "=" ;

(* Numeric literals *)
<Literal> ::= <Digit>+ ;

(* Digits *)
<Digit> ::= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
```

---

## Usage

PChorAnalyzer uses Clang's ASTActions to inspect your source file during compilation. Once the `.so` file has been built, you can link it in your compilation process as follows:

```bash
clang++ -std=c++23 -Xclang -load -Xclang ../build/libPchorAnalyzerPlugin.so  -Xclang -plugin-arg-PchorAnalyzer -Xclang --cor=<path_to_cor-file> <path_to_cpp_file> -o test
```

**Plugin Arguments:**

- `--debug`: Prints output from PchorTokenizer, PchorParser, CAST_Visitor, and Proj_Visitor for debugging.
- `--projection`: Tests the projection algorithm only; skips CAST_Visitor and CAST_Validator.

Example:

```bash
-Xclang -plugin-arg-PchorAnalyzer -Xclang --debug
-Xclang -plugin-arg-PchorAnalyzer -Xclang --projection
```

---

## Prerequisites

- **Compiler**: Clang with C++23 support (preferably 18.1.1)
- **Build System**: Ninja (v3.x or higher recommended)
- **LLVM/Clang Development**: Ensure relevant LLVM/Clang headers and libraries are installed

---

## Installation and Build

Clone the repository:

```bash
git clone <repository-url>
cd PChorAnalyzer
```

Configure and build:

```bash
mkdir build
cd build
cmake ..
make
```

This generates the plugin shared library (e.g., `libPchorAnalyzerPlugin.so`) in the build directory.

---

## Test Environment

A Dockerized environment is provided via `Dockerfile.testrunner` for convenience.

To build and enter the container:

```bash
docker build -t pchor-analyzer -f Dockerfile.testrunner .
docker run -it pchor-analyzer:latest 
```

This opens a bash terminal with a built version of Pchor. The test suite can be run using `runTest.sh`, or you can run individual scripts as described above.

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