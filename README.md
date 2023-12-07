# Interpreter for a Simple Pascal-like Programming Language
Author : [Jack Robbins](https://github.com/jackr276)

## Executive Summary
This project is an interpreter for an interpreted Pascal-like programming language. Interpreted languages, such as the language in this project and more well known ones like **Python** and **R**, are executed line-by-line, as opposed to compiled languages where programs are translated into machine code all at once before execution.  The interpreter for this language is made up of two main copmonents, a [tokenizer/lexical analyzer](https://github.com/jackr276/Simple-Pascal-Like-Language-Interpreter/blob/main/lex.cpp), and a [recursive-descent parser/interpreter](https://github.com/jackr276/Simple-Pascal-Like-Language-Interpreter/blob/main/parserInterp.cpp). To help show how this programming language is supposed to look and function, there are numerous example programs provided in the [tests folder](https://github.com/jackr276/Simple-Pascal-Like-Language-Interpreter/tree/main/tests). If you are interested, I would greatly encourage you to read the formal write-up below, download the code, and write some
programs for yourself in this programming langauge.

>[!Note]
>There is adequate documentation within the source code itself, but for a full theoretical understanding of this project, I strongly recommend reading the full write-up that follows this executive summary.

## Extended Backus-Naur Form(EBNF) Meta-syntax
EBNF is a notation/ruleset for formally describing context-free grammars. Context-free grammars are the theoretical foundation for all programming languages. The formal language generated by a context-free grammar is the set of terminal strings that can be produced by the grammar rules.

### Mathematical Foundations

Mathematically, any context-free grammar _G_ is defined as the 4-tuple $G = (V, \Sigma, R, S)$, such that:
 1. $V$ is the finite set of all non-terminals $v \in V$ where each element _v_ is a non-terminal character. Every non-terminal character is a character that cannot be changed the rules of the language.
 2. $\Sigma$ is the finite set of all terminals such that $\Sigma \cap V = \varnothing$. The set of terminals are defined by the language.
 3. $R$ is a finite subset of the binary relation defined as $V \times (V \cup \Sigma)^\*$ where $\beta \in (V \cup \Sigma)^\*$ is a string of terminals and non-terminals.
 5. $S$ is the start/entry symbol for the program. It is necessary that $S \in V$.

The mathematical rule for the grammar above allows for the creation of all valid "sentences", or in our case, programmatic commands, for our language.

### EBNF Notation Rules

We have now explored the mathematical foundation for context-free grammars. As stated above, EBNF meta-syntax is used to describe context-free grammars, which are the foundation for all programming languages, including the langauge inmplemented in this project. The table below is a handy "cheat sheet" that contains everything that you will need to know to understand the EBNF rules for this language.

>[!Important]
> Listed below is the notation that will be used in the formal EBNF definition of this programming language in the next section. It is important that this notation is understood before reading the rules for this language.


| Usage | Notation | Description |
| ----- | -------- | -------------- |
| Language Terminals | Any UPPERCASE text| Terminals are the lexical foundation of the language. They are symbols that cannot be changed using the rules of the grammar. The final output after applying all grammar rules will be a string of terminals. The terminals that are in this language are further defined below|
| Language Nonterminals | Any **bolded** text | Nonterminals, or _syntactic variables_, are symbols that can be replaced. |
|Optional Repetition|{. . . . .}|Indicates that the sequence of terminals and nonterminals inside may be repeated _0 or many times_|
|Optional Inclusion|[. . . . .]|Indicates that the sequence of terminals and nonterminals inside may be used _not at all or 1 time_|
|Grouping|(. . . . .)|Indicates that the sequence of terminals and nonterminals inside are grouped together. Usually seen used with the \| operator|
|Concatenation|    ,    |Inidicates concatenation of items |
|Alternation|    \|    |Indicates that items on the left or right are used "either-or"|
|Definition| ::= | The non-terminal on the left of the definition operator is defined as the sequence of terminals and nonterminals on the right of the operator|


## Definining the Programming Langauge in This Project

With this brief introduction in mind, let's take a look at the EBNF ruleset for the language implemented in this project.

### EBNF Description of Grammar Rules

 1. **Prog** ::= PROGRAM IDENT ; **DeclPart** **CompoundStmt**
 2. **DeclPart** ::= VAR **DeclStmt** { ; **DeclStmt** }
 3. **DeclStmt** ::= IDENT {, IDENT } : **Type** [:= **Expr**]
 4. **Type** ::= INTEGER | REAL | BOOLEAN | STRING
 5. **Stmt** ::= **SimpleStmt** | **StructuredStmt**
 6. **SimpleStmt** ::= **AssignStmt** | **WriteLnStmt** | **WriteStmt**
 7. **StructuredStmt** ::= **IfStmt** | **CompoundStmt**
 8. **CompoundStmt** ::= BEGIN **Stmt** {; **Stmt** } END
 9. **WriteLnStmt** ::= WRITELN (**ExprList**)
 10. **WriteStmt** ::= WRITE (**ExprList**)
 11. **IfStmt** ::= IF **Expr** THEN **Stmt** [ ELSE **Stmt** ]
 12. **AssignStmt** ::= **Var** := **Expr**
 13. **Var** ::= IDENT
 14. **ExprList** ::= **Expr** { , **Expr** }
 15. **Expr** ::= **LogOrExpr** ::= **LogAndExpr** { OR **LogAndExpr** }
 16. **LogAndExpr** ::= **RelExpr** {AND **RelExpr** }
 17. **RelExpr** ::= **SimpleExpr** [ ( = | < | > ) **SimpleExpr** ]
 18. **SimpleExpr** :: **Term** { ( + | - ) **Term** }
 19. **Term** ::= **SFactor** { ( * | / | DIV | MOD ) **SFactor** }
 20. **SFactor** ::= [( - | + | NOT )] **Factor**
 21. **Factor** ::= IDENT | ICONST | RCONST | SCONST | BCONST | (**Expr**)

With this description of our langauge in mind, let's explore the project's structure and function.

## Project Structure/Design Philosophy

This project is made up of 2 main components that work in tandem to achieve the goal of running a file that the user gives to us in our coding language. Additionally, there are several, less important components. Those will be covered towards throughout the explaination, but have not been given their own individual sections

The 2 main components are: the tokenizer/lexical analyzer, in [lex.cpp](https://github.com/jackr276/Simple-Pascal-Like-Language-Interpreter/blob/main/lex.cpp) and
the Parser/Interpreter, in [parserInterp.cpp](https://github.com/jackr276/Simple-Pascal-Like-Language-Interpreter/blob/main/parserInterp.cpp). Let's explore each program individually.

## Tokenizing/Lexical Analysis in [lex.cpp](https://github.com/jackr276/Simple-Pascal-Like-Language-Interpreter/blob/main/lex.cpp)

Tokenizing/Lexical analysis refers to effectively translating a program file that a user has written into a a stream of tokens that we can use to run the program. **lex.cpp** is responsible for just this task. 

This is accomplished through the main function of **lex.cpp**:

```cpp 
Lexitem getNextToken(istream& in, int& linenum)
```
This function takes a reference to an istream object and a reference to an integer as the line number, and acts as a state machine to go through the characters that its currently at. For example, if the program observes the next character to be a letter of some kind, it will automatically enter the INID(inside of identifier) state, and process the following characters as part of an identifier. It does this for integers, real number, strings, booleans and constants. If at any point the lexical analyzer runs into a lexeme(word) that is not a recognized part of the language, it will return the ERR token. It is important to note that this program only tokenizes and analyzes the lexemes of the language, it does not check for syntax, or logical correctness. That is all handled by the other program.

There is a special function that is used when dealing with keywords/reserved words in our language. This function is:
```cpp
Lexitem id_or_kw(String& lexeme, int linenum)
```
This function is called by getNextToken because there is a need to differentiate between identifiers(variable names) and reserved words. For example, if the program observes the lexeme "writeln", it has to have a way of determining if writeln is a valid variable name, or if it is a reserved word in our language. It is for this reason that, when in the state INID, the getNextToken function calls id_or_kw(), which then compares the lexeme with a map of all of our reserved words, to determine whether it is an identifier or keyword.

## Tokens for our programming language

Once a token is returned by **lex.cpp**, that token is processed by **parserInterp.cpp**. We will discuss **parserInterp.cpp** soon, but before that, provided below is a comprehensive list of all of the tokens that can be generated by the tokenizer.

### Identifiers and constants

Identifiers and constants follow certain rules, and will only be tokenized if those rules are obeyed. These rules and some examples are shown in the table below.

| Token | Description | Regular Expression Definition | Valid Examples | Invalid Examples |
|----|---------|------|-------|----|
|IDENT| An identifier is a user-defined program variable| IDENT := Letter {( Letter \| Digit \| _ \| $ )} <br /> Letter := [ a-z A-Z ] <br /> Digit := [0-9]|hello$, myVar, first_name|$hello, 1st_name, _name|
|SCONST|A string constant is defined as a sequence of characters enclosed in single quotes|SCONST := 'Any Character string'|'Hello $5 #., s9 my name is', 'hello'|"This is an invalid string due to the double quotes"|
|ICONST|An integer constant is any valid integer|ICONST := [0-9]+|2, 200, 3444|-68, 9.56|
|RCONST|A real constant is any valid real number|RCONST := ([0-9]+)\.([0-9]*)|9.01, 0.2, 2.|.2, 2.3.4|
|BCONST|A boolean constant is either true or false| BCONST := (true \| false)|true, false|tRuE, FALSE|

### Reserved Words

Furthermore, our language has certain reserved words. A reserved word is a word that may not be used as a variable name by the programmer, as the word has some syntactic function in the language. For the sake of brevity, the tokens that these reserved words are tokenized into are also included here. The tokens will make more sense as you read on.

|Reserved Word| Token |
|------------| -----|
|and|AND|
|begin|BEGIN|
|boolean|BOOLEAN|
|div|DIV|
|end|END|
|else|ELSE|
|false|FALSE|
|if|IF|
|integer|INTEGER|
|mod|MOD|
|not|NOT|
|or|OR|
|program|PROGRAM|
|real|REAL|
|string|STRING|
|write|WRITE|
|writeln|WRITELN|
|var|VAR|
|true|TRUE|
|false|FALSE|

### Operator Symbols/Keywords

Our language also has various arithmetic/logical operators that can be used. The table below shows the operands and their functions. Just like with the reserved words, the tokens for each operator/keyword are included in this table

|Operator Symbol/Keyword|Token|Description|
|-------------------|----------|-----|
|+|PLUS|Arithmetic addition or concatenation|
|-|MINUS|Arithmetic subtraction|
|*|MULT|Multiplication|
|/|DIV|Division|
|:=|ASSOP|Assignment operator|
|=|EQ|Equality|
|<|LTHAN|Less than operator|
|>|GTHAN|Greater than operator|
|and|AND|Logical conjunction|
|or|OR|Logical disjunction|
|not|NOT|Logical complement|
|div|IDIV|Integer division|
|mod|MOD|Modulos operator|


### Delimiters

Our language also has various delimiters that have certain syntactic meanings. The table below shows the delimiter characters, their tokens and descriptions




## Compiling/Running this program
