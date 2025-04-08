# PChorAnalyzer
Implementation of a static code analyzer plugin for c++ where a parametrized choreography written in a  DSL is used to validate a c++ implementation of that choreography.

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