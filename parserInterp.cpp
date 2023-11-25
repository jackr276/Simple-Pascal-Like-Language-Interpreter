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
    //The token that may be used for type checking after the optional ASSOP
    Token t;

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
        //Save the token value for type checking
        t = l.GetToken();
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
        //This is the value reference that we will pass into our expr
        Value val;
		bool status = Expr(in, line, val);

        //Throw an error if Expr fails
		if (!status) {
			ParseError(line, "Invalid expression following assignment operator.");
			return false;
		}

        //If we get here, we need to do type checking and then assign every variable the value found by expr
        switch(t){
            case STRING:
                //Types must match for this to work
                if(val.GetType() == VSTRING){
                    for(auto var : tempSet){
                        //Everything in our declstmt will have the same type, so set it all to that type
                        TempsResults[var] = val;
                    }

                } else {
                    //Otherwise we fail here
                    ParseError(line, "Illegal Assignment Operation");
                    return false;
                }

            case BOOLEAN:
                //Types also must match for booleans
                if(val.GetType() == VBOOL){
                    for(auto var : tempSet){
                        TempsResults[var] = val;
                    }

                } else {
                    ParseError(line, "Illegal Assignment Operation");
                    return false;
                }

            case REAL:
                //they match, nothing to do here
                if(val.GetType() == VREAL){
                    for(auto var : tempSet){
                        TempsResults[var] = val;
                    }

                //here is the special case, cast to the type of the LHS(t in our case)
                } else if(val.GetType() == VINT){
                    val.SetReal((float)val.GetInt());
                    val.SetType(VREAL);

                    for(auto var : tempSet){
                        TempsResults[var] = val;
                    }

                } else {
                    ParseError(line, "Illegal Assignment Operation");
                    return false;
                }

            case INTEGER:
                //they match, nothing to do here
                if(val.GetType() == VINT){
                    for(auto var : tempSet){
                        TempsResults[var] = val;
                    }

                //another special case here, cast to the LHS of INT in this case
                } else if(val.GetType() == VREAL) {
                    val.SetInt((int)val.GetReal());
                    val.SetType(VINT);

                    for(auto var : tempSet){
                        TempsResults[var] = val;
                    }
                } else {
                    ParseError(line, "Illegal Assignment Operation");
                    return false;
                }

            default:
                ParseError(line, "Illegal Assignment Operation");
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


/** 
 * Stmt is responsible for determining what kind of stmt we have and making appropriate calls
* Grammar Rules
* Stmt ::= SimpleStmt | StructuredStmtStmt
* SimpleStmt ::= AssignStmt | WriteLnStmt | WriteStmt
* StructuredStmt ::= IfStmt | CompoundStmt
*/
bool Stmt(istream& in, int& line) {
	bool status;
	// Get the next lexItem from the instream and analyze it
	LexItem l = Parser::GetNextToken(in, line);

	// If l is uncrecognizable, no use in checking anything
	if (l == ERR){
		ParseError(line, "Unrecognized input pattern.");
		cout << "(" << l.GetLexeme() << ")";
		return false;
	}

	// Check if we have a structured statement
	if (l == BEGIN || l == IF){
		//Put token back to be reprocessed
		Parser::PushBackToken(l);
		return StructuredStmt(in, line);
	}

	// Check to see if we have a simple statement
	// Assignments start with IDENT
	if (l == IDENT || l == WRITE || l == WRITELN){
		//Put token back to be reprocessed
		Parser::PushBackToken(l);
		status = SimpleStmt(in, line);

		if(!status){
			ParseError(line, "Incorrect Simple Statement.");
			return false;
		}

		return status;
	}

	//We didn't find anything so push the token back
	Parser::PushBackToken(l);
	//Stmt was not successful if we got here
	return false;
}


/**
* stmt will call StructuredStmt if appropriate according to our grammar rules
* StructuredStmt ::= IfStmt | CompoundStmt
*/
bool StructuredStmt(istream& in, int& line){
	bool status;
	LexItem strd = Parser::GetNextToken(in, line);

	switch (strd.GetToken()){
		case IF:
			status = IfStmt(in, line);
			if (!status) {
				ParseError(line, "Bad structured statement.");
				return false;
			}
			return true;
		//Compound statements begin with in
		case BEGIN:
			return CompoundStmt(in, line);

		default:
			//we won't ever get here, added to remove compile warnings
			return false;
	}
}


/**
 * Compound statements start with BEGIN and stop with END
 * CompoundStmt ::= BEGIN Stmt {; Stmt } END
*/
bool CompoundStmt(istream& in, int& line){
	LexItem l;
	LexItem lookAhead;
	//If we got here we already have consumed a BEGIN
	bool status = Stmt(in, line);

    //keep calling stmt until we have a failure
	while(status) {
		l = Parser::GetNextToken(in, line);
		if (l != SEMICOL && l != END){
			ParseError(line, "Missing Semicolon in Compound statement.");
			return false;
		}

		status = Stmt(in, line);
	}

	if (l == ERR) {
		ParseError(line, "Unrecognized Input Pattern");
		//print out the unrecognized input
		cout << "(" << l.GetLexeme() << ")" << endl;
		return false;
	}


	if (l != END) {
		line++;
		ParseError(line, "Missing END in compound statement.");
		return false;
	}

	//If we make it to this point, we had valid expressions and saw END, so return true
	return true;
}


/**
* stmt will call SimpleStmt if appropriate according to our grammar rules
* SimpleStmt ::= AssignStmt | WriteLnStmt | WriteStmt
*/
bool SimpleStmt(istream& in, int& line){
	LexItem smpl = Parser::GetNextToken(in, line);

	switch (smpl.GetToken()){
		//Assignments start with identifiers
		case IDENT:
			Parser::PushBackToken(smpl);
			return AssignStmt(in, line);

		case WRITELN:
			return WriteLnStmt(in, line);

		case WRITE: 
			return WriteStmt(in, line);
		
		//We won't ever get here, added for compile safety on Vocareum
		default:
			return false;
	}
}


/**
 * WriteLnStmt
 * WriteLnStmt ::= writeln (ExprList)
 * */ 
bool WriteLnStmt(istream& in, int& line) {
	LexItem t;

    //A queue to be used to print out all of the values from exprList
	ValQue = new queue<Value>;
	
    //Get the first token and ensure its an LPAREN
	t = Parser::GetNextToken(in, line);
	if( t != LPAREN ) {
		ParseError(line, "Missing Left Parenthesis");
		return false;
	}
	
    //Call ExprList to populate the ValQueue using our pointer
	bool ex = ExprList(in, line);
	
    //If ExprList fails we have an error
	if( !ex ) {
		ParseError(line, "Missing expression list for WriteLn statement");
		return false;
	}
	
    //finally check for the required RPAREN
	t = Parser::GetNextToken(in, line);
	if(t != RPAREN ) {
		
		ParseError(line, "Missing Right Parenthesis");
		return false;
	}
	
	//Evaluate: print out the list of expressions' values once all syntax works
	while (!(*ValQue).empty())
	{
		Value nextVal = (*ValQue).front();
		cout << nextVal;
		ValQue->pop();
	}
    //Print the endl since this is a writeln statement
	cout << endl;

	return ex;
}


/**
 * Write statements are the exact same as writelnstmt's, just without the added endl
 * WriteStmt ::= write (ExprList)
*/
bool WriteStmt(istream& in, int& line){
    //A queue to be used to print out all of the values from exprList
	ValQue = new queue<Value>;

	//Get the token after the word "write" and check if its an lparen
	LexItem t = Parser::GetNextToken(in, line);

	//No left parenthesis is an error, create error and exit
	if (t != LPAREN) {
		ParseError(line, "Missing Left Parenthesis");
		return false;
	}

	//Generate the ExprList recursively
	bool expr = ExprList(in, line);

	//If no ExprList was gotten create a different error
	if (!expr){
		ParseError(line, "Missing expression list for Write statement");
		return false;
	}

	//Check for a right parenthesis
	t = Parser::GetNextToken(in, line);
	
    //If no RPAREN, syntax error
	if (t != RPAREN) {
		ParseError(line, "Missing right Parenthesis");
		return false;
	}

    //Evaluate: print out the list of expressions' values once all syntax works
	while (!(*ValQue).empty())
	{
		Value nextVal = (*ValQue).front();
		cout << nextVal;
		ValQue->pop();
	}

	return expr;
}