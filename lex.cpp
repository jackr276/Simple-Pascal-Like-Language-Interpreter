/*
 * lex.cpp
 * Lexical Analyzer for Simple Pascal-Like Language
 */

#include <cctype>
#include <map>

using std::map;
using namespace std;

#include "lex.h"
//Keywords or reserved words mapping
LexItem id_or_kw(const string& lexeme , int linenum)
{
	map<string,Token> kwmap = {
		{ "writeln", WRITELN},
		{ "write", WRITE},
		{ "if", IF },
		{ "else", ELSE },
		{ "then", THEN },
		{ "div", IDIV },
		{ "mod", MOD },
		{ "and", AND },
		{ "or", OR },
		{ "not", NOT },
		{ "true", TRUE },
		{ "false", FALSE },
		{ "integer", INTEGER },
		{ "real", REAL },
		{ "string", STRING },
		{ "boolean", BOOLEAN },
		{ "begin", BEGIN },
		{ "end", END },
		{ "var", VAR },
		{ "program", PROGRAM }
	};
	Token tt ;
	//cout << "inside id_or_kw " << lexeme << endl;
	if(lexeme == "true" || lexeme == "false")
	{
		tt= BCONST;
	}	
	else
	{
		tt = IDENT;
		auto kIt = kwmap.find(lexeme);
		if( kIt != kwmap.end() )
		{
			tt = kIt->second;
		}
	}	
	return LexItem(tt, lexeme, linenum);
}

map<Token,string> tokenPrint = {
		{PROGRAM, "PROGRAM" },
		{VAR, "VAR" },
		{WRITELN, "WRITELN"},
		{WRITE, "WRITE"},
		{ IF, "IF" },
		{ ELSE, "ELSE" },	
		{ IDENT, "IDENT" },
		{ BEGIN, "BEGIN" },
		{ END, "END" },
		{ INTEGER, "INTEGER" },
		{ REAL, "REAL" },
		{ STRING, "STRING" },
		{ BOOLEAN, "BOOLEAN" },
		{ ICONST, "ICONST" },
		{ RCONST, "RCONST" },
		{ SCONST, "SCONST" },
		{ BCONST, "BCONST"},
		{ TRUE, "TRUE" },
		{ FALSE, "FALSE" },
		{ THEN, "THEN" },
		
			
		{ PLUS, "PLUS" },
		{ MINUS, "MINUS" },
		{ MULT, "MULT" },
		{ DIV, "DIV" },
		{ IDIV, "IDIV" },
		{ MOD, "MOD" },
		{ ASSOP, "ASSOP" },
		{ EQ, "EQ" },
		{ GTHAN, "GTHAN" },
		{ LTHAN, "LTHAN" },
		{ AND, "AND" },
		{ OR, "OR"},
		{ NOT, "NOT" },
		
		            
		{ COMMA, "COMMA" },
		{ LPAREN, "LPAREN" },
		{ RPAREN, "RPAREN" },
		{ SEMICOL, "SEMICOL" },
		{ DOT, "DOT" },
		{ COLON, "COLON" },
		{ ERR, "ERR" },

		{ DONE, "DONE" },
};

ostream& operator<<(ostream& out, const LexItem& tok) {
	
	Token tt = tok.GetToken();
	out << tokenPrint[ tt ];
	if( tt == IDENT || tt == ICONST || tt == SCONST || tt == RCONST || tt == BCONST || tt == ERR ) {
		out << ": \"" << tok.GetLexeme() << "\"";
	}
	return out;
}

LexItem getNextToken(istream& in, int& linenum)
{
	enum TokState { START, INID, INSTRING, ININT, INREAL, INCOMMENT } 
	lexstate = START;
	string lexeme;
	char ch, nextchar;
	Token tt;
	bool decimal = false;
	       
	
    while(in.get(ch)) {
    	
		switch( lexstate ) {
		case START:
			if( ch == '\n' ){
				linenum++;
				
			}	
                
			if( isspace(ch) )
				continue;

			lexeme = ch;

			if( isalpha(ch) ) {
				lexeme = ch;
				lexstate = INID;
				//cout << "in ID " << endl;
			}
			else if( ch == '\'' ) {
				lexstate = INSTRING;
				
			}
			
			else if( isdigit(ch) ) {
				lexstate = ININT;
			}
			else if( ch == '{' ) {
				//lexeme += ch;
				lexstate = INCOMMENT;
				in.get(ch);
			}				
			else {
				tt = ERR;
				switch( ch ) {
				case '+':
					tt = PLUS;
                    break;  
					
				case '-':
					tt = MINUS;
                    break; 
					
				case '*':
								
					tt = MULT;
					break;

				case '/':
					tt = DIV;
					break;
									
				case ':':
					tt = COLON;
					nextchar = in.peek();
					if(nextchar == '='){
						in.get(ch);
						lexeme += ch;
						tt = ASSOP;
						break;
					}
					//error
					break;
				
				case '=':
					tt = EQ;
					break;
				case '(':
					tt = LPAREN;
					break;			
				case ')':
					tt = RPAREN;
					break;
				
				case ';':
					tt = SEMICOL;
					break;
					
				case ',':
					tt = COMMA;
					break;
					
				case '>':
					tt = GTHAN;
					break;
				
				case '<':
					tt = LTHAN;
					break;
					
				case '.':
					tt = DOT;
					break;
				
				}
				return LexItem(tt, lexeme, linenum);
			}
			break;	

		case INID:
			if( isalpha(ch) || isdigit(ch) || ch == '_' || ch == '$') {
		
				lexeme += ch;
			}
			else {
				in.putback(ch);

				return id_or_kw(lexeme, linenum);
				
			}
			break;
					
		case INSTRING:
                          
			if( ch == '\n' ) {
				return LexItem(ERR, lexeme, linenum);
			}
			lexeme += ch;
			if( ch == '\'' ) {
				lexeme = lexeme.substr(1, lexeme.length()-2);
				return LexItem(SCONST, lexeme, linenum);
			}
			break;

		case ININT:
			if( isdigit(ch) ) {
				lexeme += ch;
			}
			else if(ch == '.') {
				lexstate = INREAL;
				in.putback(ch);
			}
			else {
				in.putback(ch);
				return LexItem(ICONST, lexeme, linenum);
			}
			break;
		
		case INREAL:
				
			if( ch == '.' && isdigit(in.peek()) && !decimal) {
				lexeme += ch; decimal = true;
				
			}
			else if(ch == '.' && !isdigit(in.peek()) && !decimal){
				lexeme += ch;
				
				return LexItem(RCONST, lexeme, linenum);
			}
			else if(isdigit(ch) && decimal){
				lexeme += ch;
			}
			
			else if(ch == '.' && decimal){
				lexeme += ch;
				return LexItem(ERR, lexeme, linenum);
			}
			else {
				in.putback(ch);
				return LexItem(RCONST, lexeme, linenum);
			}
			
			break;
		
					
		case INCOMMENT:
			if(ch == '\n') 
				linenum++;
				
			else if( ch == '}' ) {
               	//in.get(ch);
				lexstate = START;
			}
					
			break;
			
		
		}
	}//end of while loop
	
	if( in.eof() )
		return LexItem(DONE, "", linenum);
		
	return LexItem(ERR, "some strange I/O error", linenum);
}