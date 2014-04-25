%{

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "structure.h"
#include "print.h"
#include "plan1.h"
#include "util.h"
#include "execute.h"

/* external function prototypes */
extern int yylex();
extern void initLex(int, char**);
 
/* external global variables */

extern int    yydebug;
extern int    yylineno;
extern int    lexAlone;

/* function prototypes */ 
void  yyerror(const char *);

/* global variables */
char str[MAX_CHAR];

%}

/* YYSTYPE */
%union{    
    char* cVal;
    ArgPtr argPtr;
    CommandPtr cmdPtr;
    FilePtr filePtr;
    void* voidPtr;
}

/* terminals */
%token  WS
%token  <cVal>  GT
%token  <cVal>  LT
%token  <cVal>  LOPT
%token  <cVal>  SOPT
%token  <cVal>  PATH
%token  <cVal>  WORD
%token  PIPE
%token  <cVal>  QARG

/*non-terminals*/
%type <cmdPtr>  Pipeline  InputRedir  OutputRedir
%type <argPtr>  OptionList

/* associativity and precedence */
%right GT

%%

InputUnit : Pipeline{
              cmdHeadPtr = $1;
            }            
  ; 
Pipeline  :   Pipeline InputRedir{
                if(DEBUG3)  printf("Pipeline->Pipeline InputRedir: %s < %s\n",$1->name, $2->name);
                CommandPtr tempPtr = $1;
                while(tempPtr && tempPtr->next!=0)
                  tempPtr=tempPtr->next;
                tempPtr->inputRedir=1;
                tempPtr->next = $2;
              }
          |   Pipeline OutputRedir{                
                CommandPtr tempPtr = $1;
                while(tempPtr && tempPtr->next!=0)
                  tempPtr=tempPtr->next;
                if(DEBUG3)  printf("Pipeline->Pipeline OutputRedir: %s > %s\n",tempPtr->name,$2->name);
                tempPtr->outputRedir=1;
                tempPtr->next = $2;
              }
          |   OptionList PIPE Pipeline{                
                if(DEBUG3)  printf("Pipeline -> OptionList PIPE Pipeline: %s | %s\n",$1->text,$3->name);
                CommandPtr newCmd = createCommand($1->text,0);
                newCmd->headArgs = $1->next;
                newCmd->next = $3;
                //parse the args and get the files
                free($1);
                $$=newCmd;
              }
          |   OptionList{
                if(DEBUG3)  printf("Pipeline -> OptionList: %s\n",$1->text);
                CommandPtr newCmd = createCommand($1->text,0);
                newCmd->headArgs = $1->next;
                //parse the args and get the files
                free($1);
                $$=newCmd;
              }
  ;
OptionList  : SOPT OptionList{
                if(DEBUG3)  printf("SOPT: %s\n",$1);
                ArgPtr newArg = createArg($1, SOPTT);
                newArg->next = $2;
                $$=newArg;
              }
            | LOPT OptionList{
                if(DEBUG3)  printf("LOPT: %s\n",$1);
                fflush(stdout);
                ArgPtr newArg = createArg($1, LOPTT);
                newArg->next = $2;
                $$=newArg;
            }
            | QARG OptionList{
                if(DEBUG3)  printf("QARG: %s\n",$1);
                ArgPtr newArg = createArg($1, QARGT);
                newArg->next = $2;
                $$=newArg;
            }
            | WORD OptionList{
                //char* str = getRidOfEscapeChars($1);
                char* str = strdup($1);
                if(DEBUG3)  printf("WORD: %s\n",str);
                fflush(stdout);
                ArgPtr newArg = createArg(str, WORDT);
                newArg->next = $2;
                $$=newArg;
            }
            | PATH OptionList{
                //char* str = getRidOfEscapeChars($1);
                char* str = strdup($1);
                if(DEBUG3)  printf("PATH: %s\n",str);
                fflush(stdout);
                ArgPtr newArg = createArg(str, PATHT);
                newArg->next = $2;
                $$=newArg;
            }
            | {
              $$=0;
            }
  ;
InputRedir  : LT WORD{
                if(DEBUG3)  printf("LT WORD: < %s\n",$2);
                CommandPtr newCmd= createCommand($2, 0);
                $$=newCmd;
              }            
  ;
OutputRedir : GT WORD{
                if(DEBUG3)  printf("GT WORD: %s\n",$2);                
                CommandPtr newCmd= createCommand($2, 0);
                newCmd->currOutputRedir=1;
                $$=newCmd;
              }
            | GT PATH {
                if(DEBUG3)  printf("GT PATH: %s\n",strdup($2));
                CommandPtr newCmd= createCommand($2, 0);
                $$=newCmd;
              }
  ;

%%
void yyerror (char const *s) {
       fprintf (stderr, "Line %d: %s\n", yylineno, s);
}

int main(int argc, char **argv){
  initLex(argc, argv);
  if(initGlobals()<0){
    printf("No mounted SSHFS drives found!\n");
    return 1;
  }
#ifdef YYLLEXER
  while (gettok() !=0) ; //gettok returns 0 on EOF
  return 0;
#else
  if(!lexAlone){
    yyparse();
    fixIPRedir();
    /*if(DEBUG1)
      printTree();*/
    if(initGlobals()<0){
      printf("No mounted SSHFS drives found!\n");
      return 1;
    }
    //if(DEBUG1)
    printHostMap();
    makePlan1();
    makeRemoteCmd();
    //if(DEBUG1)
    printRemoteCmdTree();
    //executePlan();
  }
  return 0;
#endif
}