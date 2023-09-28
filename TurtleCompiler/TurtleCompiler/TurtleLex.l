
%{

/* Turtle Compiler lex file
   by Pete Myers
   OIT Portland Fall 2008
   Flex C++ version Jan 2017

   Assignment 4 Handout
*/

#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
#include "TurtleYacc.h"
#include "symtable.h"

void yyerror(const char * msg);

typedef enum
{
	SCOPE_STATE_GLOBAL = 0,
	SCOPE_STATE_TO = 1,
	SCOPE_STATE_PARAM = 2,
	SCOPE_STATE_LOCAL = 3
} SCOPE_STATE;
SCOPE_STATE scope_state = SCOPE_STATE_GLOBAL;
int bracket_count = 0;
STORAGE_TYPE storageForNextDecl = ST_GLOBAL;

%}

%option noyywrap

%%

\-?[0-9]+		{ yylval.value = atoi(yytext); return NUMBER; }

"+"			{ return yytext[0]; }
"-"			{ return yytext[0]; }
"="			{ return yytext[0]; }
"<"			{ return yytext[0]; }
">"			{ return yytext[0]; }
"*"			{ return yytext[0]; }
"/"			{ return yytext[0]; }
"("			{ 
				if (scope_state == SCOPE_STATE_TO) 
				{
					scope_state = SCOPE_STATE_PARAM;
					storageForNextDecl = ST_PARAM;
					SymbolTable::GetInstance()->NewLocalScope();
				}
				return yytext[0]; 
			}
")"			{ 
				if (scope_state == SCOPE_STATE_PARAM) 
				{
					scope_state = SCOPE_STATE_LOCAL;
					storageForNextDecl = ST_LOCAL;
				}
				return yytext[0];
			}
"["			{ 
				if (scope_state == SCOPE_STATE_LOCAL) 
				{
					bracket_count++;
				}
				return yytext[0];
			}
"]"			{ 
				if (scope_state == SCOPE_STATE_LOCAL) 
				{
					bracket_count--;
					if (bracket_count == 0)
					{
						scope_state = SCOPE_STATE_GLOBAL;
						storageForNextDecl = ST_GLOBAL;
						SymbolTable::GetInstance()->ExitLocalScope();
					}
				}
				return yytext[0];
			}

home		{ return HOME; }
fd			{ return FD; }
bk			{ return BK; }
lt			{ return LT; }
rt			{ return RT; }
repeat		{ return REPEAT; }
setc		{ return SETC; }
setxy		{ return SETXY; }
setx		{ return SETX; }
sety		{ return SETY; }
seth		{ return SETH; }
pd			{ return PD; }
pu			{ return PU; }
ht			{ return HT; }
st			{ return ST; }

color 		{ return COLOR; }
xcor 		{ return XCOR; }
ycor 		{ return YCOR; }
heading 	{ return HEADING; }
random 		{ return RANDOM; }

if			{ return IF; }
ifelse		{ return IFELSE; }
while		{ return WHILE; }

black 		{ yylval.colorType = BLACK; return COLOR_NAME; }
white 		{ yylval.colorType = WHITE; return COLOR_NAME; }
orange 		{ yylval.colorType = ORANGE; return COLOR_NAME; }
yellow 		{ yylval.colorType = YELLOW; return COLOR_NAME; }
lime 		{ yylval.colorType = LIME; return COLOR_NAME; }
cyan 		{ yylval.colorType = CYAN; return COLOR_NAME; }
blue 		{ yylval.colorType = BLUE; return COLOR_NAME; }
magenta 	{ yylval.colorType = MAGENTA; return COLOR_NAME; }
red 		{ yylval.colorType = RED; return COLOR_NAME; }
brown 		{ yylval.colorType = BROWN; return COLOR_NAME; }
green 		{ yylval.colorType = GREEN; return COLOR_NAME; }
turquoise 	{ yylval.colorType = TURQUOISE; return COLOR_NAME; }
sky 		{ yylval.colorType = SKY; return COLOR_NAME; }
violet 		{ yylval.colorType = VIOLET; return COLOR_NAME; }
pink 		{ yylval.colorType = PINK; return COLOR_NAME; }




int 		{ return INT; }

to 			{ 
				if (scope_state == SCOPE_STATE_GLOBAL)
					scope_state = SCOPE_STATE_TO;
				return TO;
			}

return 		{ return RETURN; }

[a-zA-Z][a-zA-Z0-9]*	{ 
							yylval.symentry = SymbolTable::GetInstance()->Insert(yytext, ID);
							return ID;
						}

"//".* 								{ /*Ignore single line comments*/ }
\/\*([^*]|(\*+[^*/]))*\*+\/ 		{
										/* Count \n in multi line comments */
										for (int i=0; yytext[i] != '\0'; i++) 
											if (yytext[i] == '\n') 
												yylineno++;
									}

(\t|" ")+	{ /* ignore whitespace */ }

\n			{ yylineno++; }

[^ \=\<\>\+\-\*\/\(\)\[\]\t\n]*		{
				/* illegal tokens */
				yyerror("illegal token");
				exit(-1);
			}

%%

void yyerror(const char * msg)
{
	fprintf(stderr, "ERROR! Line #%d: %s, near '%s'\n", yylineno, msg, yytext);
}

