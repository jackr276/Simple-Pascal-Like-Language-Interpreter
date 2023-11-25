/**
 * Jack Robbins
 * Recursive-Descent Parser and Interpreter for a Pascal-Like language
*/

#include "parserInterp.h"
#include <iostream>
#include <set>

// defVar keeps track of all variables that have been defined in the program thus far
map<string, bool> defVar;
// SymTable keeps track of the type for all of our variables
map<string, Token> SymTable;
//Container of temporary locations of Value objects for results of expressions, variables values and constants 
//Key is a variable name, value is its Value. Holds all variables defined by assign statements
map<string, Value> TempsResults;
//declare a pointer variable to a queue of Value objects
queue <Value> * ValQue;


//The parser namespace that interacts with lex for us
namespace Parser {
	bool pushed_back = false;
	LexItem	pushed_token;

	static LexItem GetNextToken(istream& in, int& line) {
		if( pushed_back ) {
			pushed_back = false;
			return pushed_token;
		}
		return getNextToken(in, line);
	}

	static void PushBackToken(LexItem & t) {
		if( pushed_back ) {
			abort();
		}
		pushed_back = true;
		pushed_token = t;	
	}
}

//Initialize error count to be 0
static int error_count = 0;


// A simple wrapper that allows access to the number of syntax errors
int ErrCount(){
    return error_count;
}


//A simple error wrapper that incrememnts error count, and prints out the error
void ParseError(int line, string msg)
{
	++error_count;
	cout << line << ": " << msg << endl;
}


/**
 * Prog is the entry point to our entire interpreter, the "root" of our parse tree
 * To start, the program must use the keyword Program and give an identifier name.
 * It must then go into the Declaritive part followed by a compound statement
 * Prog ::= PROGRAM IDENT ; DeclPart CompoundStmt
*/
bool Prog(istream& in, int& line){
	bool status = false;

	//This should be the keyword "program"
	LexItem l = Parser::GetNextToken(in, line);

	//We're missing the required program keyword, throw an error and exit
	if (l != PROGRAM){
		ParseError(line, "Missing PROGRAM keyword.");
		return false;
	} else {
		//We have the program keyword, move on to more processing
		l = Parser::GetNextToken(in, line);

		//This token should be an IDENT if all is correct, if not we have an error
		if (l != IDENT){
			ParseError(line, "Missing Program name.");
			return false;
		}

		//If we're at this point we have PROGRAM IDENT, need a semicol
		l = Parser::GetNextToken(in, line);

		//If there's no semicolon, syntax error
		if (l != SEMICOL) {
			ParseError(line, "Syntax Error.");
			return false;
		}

		//By this point, we have checked up to PROGRAM IDENT ;
		//Check the DeclPart
		status = DeclPart(in, line);
		
		//If the declaration was bad, no point in continuing
		if (!status){
			ParseError(line, "Incorrect Declaration Section.");
			return false;
		}

		//Up to here we have gotten PROGRAM IDENT ; DeclPart
		//Check for the compound statement, make sure that there actually is a BEGIN
		l = Parser::GetNextToken(in, line);
		if (l != BEGIN){
			ParseError(line, "Syntactic Error in Declaration Block.");
			ParseError(line, "Incorrect Declaration Section");
			return false;
		}

		//If we  have BEGIN, consume it and call CompoundStmt
		status = CompoundStmt(in, line);

		//If the compound statement was bad, return false
		if (!status){
			ParseError(line, "Incorrect Program Body.");
			return false;
		}

	}

	//There could also be some unrecognizable token here
	if (l == ERR) {
		ParseError(line, "Unrecognized input pattern.");
		cout << "(" << l.GetLexeme() << ")" << endl;
		return false; 
	}

	//If we reach here, status will be true and parsing will have been successful
	return status;
}


/**
 * The declarative part must start with the var keyword, followed by one or more colon separated declStmt's
 * There will be no actual value processing, that is handled further down the parse tree
 * DeclPart ::= VAR DeclStmt; { DeclStmt ; }
*/
bool DeclPart(istream& in, int& line){
	bool status = false;

	LexItem l = Parser::GetNextToken(in, line);
	//This first token should be VAR, if not throw an error
	if (l != VAR){
		ParseError(line, "Non-recognizable Declaration Part.");
		return false;
	}

	//Once we're here, we should be seeing DeclStmt's followed by SEMICOLs
	//There can be as many as we like, so use iteration

	//We will use this lexitem to look ahead
	LexItem lookAhead = Parser::GetNextToken(in, line);
	while(lookAhead == IDENT){
		//Once we know its an ident, put it back for processing by DeclStmt
		Parser::PushBackToken(lookAhead);
		
		//DeclStmt processing
		status = DeclStmt(in, line);
		
		//If its a bad DeclStmt, throw error
		if (!status) {
			ParseError(line, "Syntactic error in Declaration Block.");
			return false;
		}

		//We had a good DeclStmt, it has to be followed by a semicol
		l = Parser::GetNextToken(in, line);

		//If no semicolon, throw syntax error
		if (l != SEMICOL){
			//error right here
			ParseError(line, "Syntactic error in Declaration Block.");
			return false;
		}

		//Update lookahead, this will tell us if we have more declstmts
		lookAhead = Parser::GetNextToken(in, line);
	}

	//If we get here, lookAhead must not have been an IDENT. We need to put it back for processing by the CompoundStmt block
	Parser::PushBackToken(lookAhead);

	//If we get here, our DeclPart will have been successful
	return status;
}


/**
 * A delcaration statement can have one or more comma separated identifiers, followed by a valid type and an optional assignment
 * DeclStmt ::= IDENT {, IDENT } : Type [:= Expr]
*/
bool DeclStmt(istream& in, int& line){
	//All of the variables in a declstmt are going to have the same type, store in a set for type assignment
	set<string> tempSet;

	//Dummy lexItem to make the first iteration of the while loop run
	LexItem lookAhead = LexItem(COMMA, ",", 0);
	LexItem l;

	//We should see an IDENT first
	while (lookAhead == COMMA) {
		l = Parser::GetNextToken(in, line);
		//l must be an IDENT
		if (l != IDENT) {
			ParseError(line, "Non-indentifier declaration.");
			return false;
		}

		//If this variable is already in defVars, we have a redeclaration, throw error
		if (defVar.find(l.GetLexeme()) -> second){
			ParseError(line, "Variable Redefinition");
			ParseError(line, "Incorrect identifiers list in Declaration Statement.");
			return false;
		}

		//If we get here, it wasn't in defVars, so we should add it
		defVar.insert(pair<string, bool> (l.GetLexeme(), true));
        //Put the variable name into the tempSet for type processing
		tempSet.insert(l.GetLexeme());

		lookAhead = Parser::GetNextToken(in, line);
	}
	
	//If we're out of the loop, we know it wasn't a comma
	//If there's an ident after this, then we know the user forgot to put a comma in between
	if (lookAhead == IDENT){
		ParseError(line, "Missing comma in declaration statement");
		//Having this would also make it a bad identifier list, so return this error as well
		ParseError(line, "Incorrect identifiers list in Declaration Statement.");
		return false;
	}

	//If its not a colon at this point, there's some syntax error here
	//Let caller handle
	if (lookAhead != COLON){
		return false;
	}


	//following this, we need to have a type for our variables
	//The next token should be a valid type
	l = Parser::GetNextToken(in, line);
	//allowed to be integer, boolean, real, string
	if (l == STRING || l == INTEGER || l == REAL || l == BOOLEAN){
		for(auto i : tempSet){
            //symtable keeps track of the type for all variables
			SymTable[i] = l.GetToken();
		}
	} else {
		//Unrecognized type
		ParseError(line, "Incorrect Declaration Type.");
		return false;
	}

	//Once we get here, we have found and put them all in the symtable
	//DeclStmt ::= IDENT {, IDENT } : Type
	//After type, there is an optional ASSOP, so get the next token to check
	l = Parser::GetNextToken(in,line);

	//If we find the optional ASSOP, process it
	if (l == ASSOP){
		bool status = Expr(in, line); // FIXME -- should pass in a reference to a value object
		if (!status) {
			ParseError(line, "Invalid expression following assignment operator.");
			return false;
		}

	//If its unrecognized throw and error
	} else if (l == ERR){
		ParseError(line, "Unrecognized input pattern.");
		cout << "(" << l.GetLexeme() << ")";
		return false;

	//If we get here, l was not the optional ASSOP or ERR, push token back and return
	} else {
		Parser::PushBackToken(l);
	}

	return true;
}

