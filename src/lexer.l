
/**********************/
/* C header files */
/**********************/

%{
#include "structure.h"
#include "for_loop.h"
#include "parser.tab.h"
char tokenString[255];
int printoutScan = 0;
int lexAlone=0;

%}

%option yylineno

/**********************/
/* start your regular definitions  here */
/**********************/
SEMI	";"
LBRACE	"{"
RBRACE	"}"
IN	"in"
/* this is for arguments that are enclosed between ' and "*/
/* this is required because white-spaces included between these make sense*/
QARG		((\'[[:graph:][:blank:]]+\')|(\"[[:graph:][:blank:]]+\"))
/* exclude any character other than -,=,/ and any white space */
SMALLOPT	(-[^-= \t\r\n\v\/]+)
/* -- followed by anything and optionally followed by = and anything*/
LONGOPT		(--[[:alnum:]]+(=[[:graph:]]+){0,1})
/* starting off with aything that doesn't have / or \ and white-spaces followed by a group with / and anything*/
/*its ugly but hey its working*/
/*besides this is inevitably badass \m/*/
PATH		((((\/{0,1})(((\\[\r\t ]+)|(\\))+[^\\ \n\t\v\/]*(\\+[\t\v ]+)*)*)+[^\/\\ \t\v\n]*)+)
GT 			\>
LT 			\<
PIPE		\|
WORD		[[:graph:]]+
WSPACE		([ \r\n\t\v]+)
NEWLINE		[\r\n]+

/* start your token specifications here */
/* Token names must come from cmparser.tab.h */

%%

{SEMI}	{ yylval.cVal = strdup(yytext); return TOK_SEMI; }
"for"	{ yylval.cVal = strdup(yytext); return TOK_FOR; }
"do"	{ yylval.cVal = strdup(yytext); return TOK_DO; }
"done" 	{ yylval.cVal = strdup(yytext); return TOK_DONE; }
"in"	{ yylval.cVal = strdup(yytext); return TOK_IN; }
{LBRACE}	{ yylval.cVal = strdup(yytext); return TOK_LBRACE; }
{RBRACE}	{ yylval.cVal = strdup(yytext); return TOK_RBRACE; }
{NEWLINE}	{ /*yylval.cVal = strdup(yytext); return TOK_NEWLINE;*/ }
{GT} { return GT; }
{LT} { return LT; }
{PIPE} { return PIPE; }
{SMALLOPT} { yylval.cVal = strdup(yytext); return SOPT; }
{LONGOPT} { yylval.cVal = strdup(yytext); return LOPT; }
{QARG} { yylval.cVal = strdup(yytext); return QARG; }
{WORD} { yylval.cVal = strdup(yytext); return WORD; }
{PATH}	{ yylval.cVal = strdup(yytext); return PATH; }
{WSPACE} {  }	/*IGNORE THEM?*/

%%
/**********************/
/* C support functions */
/**********************/

void printToken(int token, char *str){			
	switch(token){		
		case WS:
			fprintf(yyout,"%d : WS : %s\n",yylineno,str);
			break;
		case QARG:
			fprintf(yyout,"%d : QARG : %s\n",yylineno,str);
			break;
		case SOPT:
			fprintf(yyout,"%d : SMALLOPT : %s\n",yylineno,str);
			break;
		case LOPT:
			fprintf(yyout,"%d : LONGOPT : %s\n",yylineno,str);
			break;			
		case WORD:
			fprintf(yyout,"%d : WORD : %s\n",yylineno,str);
			break;
		case PATH:
			fprintf(yyout,"%d : PATH : %s\n",yylineno,str);
			break;
		case GT:
			fprintf(yyout,"%d : GT : %s\n",yylineno,str);
			break;
		case LT:
			fprintf(yyout,"%d : LT : %s\n",yylineno,str);
			break;
		case PIPE:
			fprintf(yyout,"%d : PIPE : %s\n",yylineno,str);
			break;
		case TOK_DO:
			fprintf(yyout,"%d : TOK_DO : %s\n",yylineno,str);
			break;
		case TOK_DONE:
			fprintf(yyout,"%d : TOK_DONE : %s\n",yylineno,str);
			break;
		case TOK_FOR:
			fprintf(yyout,"%d : TOK_FOR : %s\n",yylineno,str);
			break;
		case TOK_SEMI:
			fprintf(yyout,"%d : TOK_SEMI : %s\n",yylineno,str);
			break;
		case TOK_LBRACE:
			fprintf(yyout,"%d : TOK_LBRACE : %s\n",yylineno,str);
			break;
		case TOK_RBRACE:
			fprintf(yyout,"%d : TOK_RBRACE : %s\n",yylineno,str);
			break;
		case TOK_IN:
			fprintf(yyout,"%d : TOK_IN : %s\n",yylineno,str);
			break;
		case TOK_NEWLINE:
			fprintf(yyout,"%d : TOK_NEWLINE\n",yylineno);
			break;
	}
}

int gettok(void){
    int currentToken;
    
    currentToken=yylex();    
    if (currentToken == 0) { // means EOF}
            return 0;
    }
     if (printoutScan || lexAlone) {
    	strncpy(tokenString, yytext, 255);   
        printToken(currentToken,tokenString);
    }
    return currentToken;
}

void initLex(int argc, char **argv){
   if ( argc > 1 )
       yyin = fopen( argv[1], "r" );
   else
    yyin = stdin;

   if(lexAlone){
	while (gettok() !=0) ; //gettok returns 0 on EOF
		return;
   }
}