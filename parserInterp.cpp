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
                if(val.IsString()){
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
                if(val.IsBool()){
                    for(auto var : tempSet){
                        TempsResults[var] = val;
                    }

                } else {
                    ParseError(line, "Illegal Assignment Operation");
                    return false;
                }

            case REAL:
                //they match, nothing to do here
                if(val.IsReal()){
                    for(auto var : tempSet){
                        TempsResults[var] = val;
                    }

                //here is the special case, cast to the type of the LHS(t in our case)
                } else if(val.IsInt()){
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
                if(val.IsInt()){
                    for(auto var : tempSet){
                        TempsResults[var] = val;
                    }

                //another special case here, cast to the LHS of INT in this case
                } else if(val.IsReal()) {
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


// Processing all IF statements, 
// IfStmt ::= IF Expr THEN Stmt [ ELSE Stmt ]
bool IfStmt(istream& in, int& line){
	LexItem l;
	//The val value to be used with Expr
	Value val;

	//Once this function is called, the IF token has been consumed already
	//We should see a valid expression at this point
	bool status = Expr(in, line, val);

	//if expression is not valid, throw an error
	if(!status){
		ParseError(line, "Invalid expression in IF statement.");
		return false;
	}

	//If val is not a boolean, that's an error
	if(val.GetType() != VBOOL){
		ParseError(line, "Expression in IF Statement must be of type Boolean");
		return false;
	}

	//if we get here, then we had a valid expression. Next token must be THEN
	l = Parser::GetNextToken(in, line);

	//If its unknown, throw error
	if (l == ERR ){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << l.GetToken() << ")" << endl;
		return false;
	}

	//If its not a THEN, we have an error
	if (l != THEN) {
		ParseError(line, "Missing THEN in IF statement.");
		return false;
	}

	//Once we're here we know we have ::= IF expr = true THEN stmt 
	
	//If val is true, execute the stmt and return
	if (val.GetBool() == true){
		//Exceute the stmt
		status = Stmt(in, line);
		
		//Potential for a bad stmt here
		if(!status){
			ParseError(line, "Bad Statement in IF Statement");
			return false;
		}

		//we also need to "skip over" the entire else statement if this is the case
		l = Parser::GetNextToken(in, line);
		
		//Skip over until l is a semicol
		while(l != SEMICOL) {
			//Catch any errors we might see while we're at it
			if (l == ERR){
				ParseError(line, "Unrecognized Input Pattern");
				cout << "(" << l.GetToken() << ")" << endl;
				return false;
			}

			l = Parser::GetNextToken(in, line);
		}
		
		//We will have consumed one extra token by here, so return it
		Parser::PushBackToken(l);

	} else {
		//If we get here the val was false, so there are 2 options based on whether or not there is an ELSE
		l = Parser::GetNextToken(in, line);

		//We want to skip over the statement after then, as it is not being executed
		while(l != SEMICOL && l != ELSE){
			//Catch any errors we might see while we're at it
			if (l == ERR){
				ParseError(line, "Unrecognized Input Pattern");
				cout << "(" << l.GetToken() << ")" << endl;
				return false;
			}

			l = Parser::GetNextToken(in, line);
		}

		//If we get here, we know l is either a SEMICOL or an ELSE
		
		//If we just have a semicol, return and be done
		if(l == SEMICOL) {
			//put the semicol back, it's not this function's job to procees it
			Parser::PushBackToken(l);
			return true;
		} 

		//otherwise if we have an else stmt, keep the token and process the stmt
		if (l == ELSE){
			//Execute the else stmt
			status = Stmt(in, line);

			//If it fails return false
			if(!status){
				ParseError(line, "Invalid stmt in IF-ELSE stmt");
				return false;
			}
		}
	}

	//If we make it here, everything worked
	return true;
}


/**
 * Assignment Statements take in a var, ASSOP and expression
 * AssignStmt ::= Var := Expr
*/
bool AssignStmt(istream& in, int& line){
	bool status = false;
	bool varStatus = false;
	LexItem l;
	//This will be used for finding types from var
	LexItem idtok;
	//This will be used for Expr
	Value val;

	//Check to see the status of the identifier that we have(was it already declared?)
	varStatus = Var(in, line, idtok);

	//If the variable wasn't correct, we essentially have no variable
	if(!varStatus){
		ParseError(line, "Missing Left-Hand Side Variable in Assignment statement");
		return false;
	}

	//If we get here, we know we have a valid Var
	//Get the next token(should be assop)
	l = Parser::GetNextToken(in, line);

	//If we have an error token, throw error and exit
	if (l == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		//print out the unrecognized input
		cout << "(" << l.GetLexeme() << ")" << endl;
		return false;
	}

	//If we don't have an assop here, no reason in continuing
	if (l != ASSOP){
		ParseError(line, "Missing Assignment Operator in AssignStmt");
		return false;
	}

	//Once we're here, we know we have Valid Var :=, now analyze the expr
	status = Expr(in, line, val);

	//If there's no expression, thats an error
	if (!status){
		ParseError(line, "Missing Expression in Assignment Statement");
		return false;
	}

	//Once we're here, we know we have Var := Expr, but we don't know if our types match, so do type checking

	//using our idtok, which has the variable name as its lexeme
	string var = idtok.GetLexeme();

	//right now, idtok is holding the type of the valid variable, and val is holding the the value gotten from some valid expr
	switch(idtok.GetToken()){
		case STRING:
			//Types must match for this to work
			if(val.IsString()){
				TempsResults[var] = val;
				return true;
			}

		case BOOLEAN:
			//Types also must match for booleans
			if(val.IsBool()){
				TempsResults[var] = val;
				return true;
			}

		case REAL:
			//they match, nothing to do here
			if(val.IsReal()){
				TempsResults[var] = val;
				return true;
			} 
			
			//here is the special case, cast to the type of the LHS(t in our case)
			if(val.IsInt()){
				val.SetReal((float)val.GetInt());
				val.SetType(VREAL);

				TempsResults[var] = val;
				return true;
			}

		case INTEGER:
			//they match, nothing to do here
			if(val.IsInt()){
				TempsResults[var] = val;
				return true;
			} 
		
			//another special case here, cast to the LHS of INT in this case
			if(val.IsReal()) {
				val.SetInt((int)val.GetReal());
				val.SetType(VINT);

				TempsResults[var] = val;
				return true;
			}

		//We should never get here in theory, added to remove compile warnings
		default:
			ParseError(line, "Illegal Assignment Operation");
			return false;
	}

	//if we get here, we know there must have been some mismatched types
	ParseError(line, "Mismatched types in assignment operation");
	return false;
}


// Check to see if the variable is valid and has previously been declared
// Var ::= IDENT
bool Var(istream& in, int& line, LexItem& idtok){
	//get the token, check to see if var was declared
	LexItem l = Parser::GetNextToken(in, line);

	//If we can find the variable, return true
	if(defVar.find(l.GetLexeme())-> second){
		//Use idtok to conveniently store the type of the variable using symTable
		idtok = LexItem(SymTable[l.GetLexeme()], l.GetLexeme(), line);
		return true;

	//If lexeme is unrecognized, then give this error
	} else if (l == ERR) {
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << l.GetToken() << ")" << endl;
		return false;

	//If we get here, we have a valid variable name that was just not declared. Show appropriate error
	} else {
		ParseError(line, "Undeclared Variable");
		return false;
	}

	//We should never get here, added to remove compile warnings
	return false;
}


//Simply calls expr recursively, so long as there are more commas. This will be used in our writeln
//ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
	bool status = false;
	Value retVal;

	//Call expr, passing in retVal so we can have a result
	status = Expr(in, line, retVal);

	//If expr is bad, no point in continuing
	if(!status){
		ParseError(line, "Missing Expression");
		return false;
	}

	//Push the value to the queue of values to be printed
	ValQue->push(retVal);

	//get and analyze next token, if it is a comma we should recursively call this function again
	LexItem tok = Parser::GetNextToken(in, line);
	
	if (tok == COMMA) {
		status = ExprList(in, line);

	} else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;

	} else {
		Parser::PushBackToken(tok);
		return true;
	}

	return status;
}


//Expr ::= LogOrExpr ::= LogAndExpr { OR LogAndExpr }
//So essentially: Expr ::= LogANDExpr { OR LogAndExpr}
bool Expr(istream& in, int& line, Value& retVal){
	bool status = false;
	LexItem l;
	
	//Once we get here, first thing to do is call LogAndExpr
	status = LogANDExpr(in, line, retVal);
	//If this is all that we have, it doesn't matter the type of retVal

	//If expression is bad, return false
	if (!status){
		return false; // FIXME potentially may want to have an error here
	}

	//Once we're here, we can either have nothing or one or more OR's followed by more LogAndExpr
	//Get the next token
	l = Parser::GetNextToken(in, line);

	//While we have an OR, keep processing LogAndExpr's
	while (l == OR){
		Value val;
		status = LogANDExpr(in, line, val);

		//If expression is bad, return error
		if (!status){
			return false; //FIXME potentially may want to throw an error here
		}

		//perform the "OR"ing for retval and val and reassign it to retVal
		retVal = retVal || val;

		//If this doesn't work, we had a bad or operation(i.e. we either val or retval isn't a boolean)
		if(retVal.IsErr()){
			ParseError(line, "Illegal use of non-boolean operand with OR");
			return false;
		}

		//refresh the value of l
		l = Parser::GetNextToken(in, line);
	}

	// if we have an ERR token, throw error
	if (l == ERR) {
		ParseError(line, "Unrecognized input pattern.");
		cout << "(" << l.GetLexeme() << ")";
		return false;
	}

	//Once we get here, l was not an OR, so put it back and we're done
	Parser::PushBackToken(l);

	//If we end up here, everything worked
	return true;
}


// LogAndExpr ::= RelExpr {AND RelExpr }
bool LogANDExpr(istream& in, int& line, Value& retVal){
	bool status = false;
	LexItem l;

	//Once we get here, the first thing we should do is check for a relational expression
	status = RelExpr(in, line, retVal);
	//If we there are no AND operations, it's fine if retVal isn't a boolean

	//If we have a bad relational expression, return error
	if (!status){
		//ParseError(line, "Syntactic error in relational expression.");
		return false;
	}

	//Good first expression, check to see if we have an AND token
	l = Parser::GetNextToken(in, line);

	//So long as we keep seeing AND, keep processing tokens
	while (l == AND){
		Value val;
		//Check the next relational expression
		status = RelExpr(in, line, val);

		//If its a bad expression, throw an error and stop
		if (!status){
			return false; //FIXME potentially want to throw error here
		}

		//Perform the "AND"ing between retVal and Val
		retVal = retVal && val;

		//If retVal happens to be an error, that means we had a non-boolean operand somewhere
		if (retVal.IsErr()){
			ParseError(line, "Illegal use of a non-boolean operand with AND");
			return false;
		}

		//Refresh the value of l
		l = Parser::GetNextToken(in, line);
	}

	//If we got here, we know l wasn't AND, check if it is ERR
	if (l == ERR) {
		ParseError(line, "Unrecognized input pattern.");
		cout << "(" << l.GetLexeme() << ")";
		return false;
	}
	
	//If we get here, l wasn't AND or an ERR, so push it back to the stream
	Parser::PushBackToken(l);

	return status;
}


// RelExpr ::= SimpleExpr [ ( = | < | > ) SimpleExpr ]
bool RelExpr(istream& in, int& line, Value& retVal){
	bool status;
	LexItem l;

	//We should first see a valid SimpleExpr
	status = SimpleExpr(in, line, retVal);

	if (!status) {
		ParseError(line, "Invalid Simple Expression in Relational Expression");
		return false;
	}

	//If we get here we can optionally see =, < or > once
	l = Parser::GetNextToken(in, line);

	//If it is one of these, check for the validity of the simpleExpr
	if (l == EQ || l == GTHAN || l == LTHAN) {
		Value val;
		//Process the simpleExpr following this
		status = SimpleExpr(in, line, val);

		//Throw error if bad
		if(!status) {
			ParseError(line, "Invalid Simple Expression in Relational Expression.");
			return false;
		}

		//Now we need to perform the operation, making use of our overloaded operators
		switch(l.GetToken()){
			case EQ:
				retVal = retVal == val;

			case GTHAN:
				retVal = retVal > val;

			case LTHAN:
				retVal = retVal < val;
			
			//We won't ever get here, added to remove compile warnings
			default:
				return false;
		}

		//If we have an error here, retVal or Val were non-boolean, and the relational operation failed
		if (retVal.IsErr()){
			ParseError(line, "Illegal use of non-boolean operand with relational operators");
			return false;
		}
	}

	//If lexeme is unknown, throw error
	if (l == ERR) {
		ParseError(line, "Unrecognized input pattern.");
		cout << "(" << l.GetLexeme() << ")";
		return false;
	}

	//If we didn't have =, < or > or ERR, push token back and return status
	Parser::PushBackToken(l);

	//If we end up here, all went well
	return true;
}


//SimpleExpr :: Term { ( + | - ) Term }
bool SimpleExpr(istream& in, int& line, Value& retVal){
	bool status;
	LexItem l;

	//We should see a valid term first
	status = Term(in, line, retVal);

	//Throw error if invalid
	if(!status){
		return false; //FIXME potentiall want an error here
	}

	//once we're here, we can see 0 or many + and -
	l = Parser::GetNextToken(in, line);

	//So as long as we have plus or minus, we keep processing
	while (l == PLUS || l == MINUS) {
		//Secondary value for the other term
		Value val;

		//Process the next term
		status = Term(in, line, val);

		//If we have a bad term, throw error
		if(!status) {
			//ParseError(line, "Invalid term in expression.");
			return false;
		}

		//Once we're here, we process the arithmetic accordingly, using our overloaded operators
		if (l == PLUS){
			retVal = retVal + val;
		}

		if (l == MINUS){
			retVal = retVal - val;
		}

		//If somehow retVal is now an error, that means something didn't work, so exit accordingly
		if (retVal.IsErr()){
			ParseError(line, "Illegal arithmetic operation");
			return false;
		}

		//Refresh l
		l = Parser::GetNextToken(in, line);
	}

	//If lexeme is unknown, throw error
	if (l == ERR) {
		ParseError(line, "Unrecognized input pattern.");
		cout << "(" << l.GetLexeme() << ")";
		return false;
	}

	//Once we're here, we know l wasn't + or -, so we're done
	//Push l back and return status
	Parser::PushBackToken(l);

	//All went well if we end up here
	return true;
}
