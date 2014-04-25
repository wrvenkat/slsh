#ifndef FORLOOP_H
#define FORLOOP_H

#include <glob.h>
#include "structure.h"
#include "enums.h"

//forward declns.
typedef struct pipeline_list PIPELINELIST;
typedef PIPELINELIST* PipelineListPtr;

typedef struct command COMMAND;
typedef COMMAND* CommandPtr;

typedef struct arg ARG;
typedef ARG* ArgPtr;

extern char* currWD;

typedef struct for_loop{
  char* varName;
  char* expr;
  PipelineListPtr pipeline;
  PipelineListPtr newPipeline;
  struct for_loop* next;
}FORLOOP;

typedef FORLOOP* ForLoopPtr;

//creates a new for loop
ForLoopPtr createForLoop(char* varName, char* expr);

//frees the ForLoop and its fields
void freeForLoop(ForLoopPtr forLoop);

//this makes the list of filepath string that confirm to the
// expr in for_loop
glob_t* getFileList(char* expr);

//the bootstrap function to manage references in the pipeline list
int manageReferences(ForLoopPtr currForLoop);

//this function manages or replaces references in
//each of the pipeline in the pipeline list
int manageReferencesInLoop(char* varName, int count, char** fileList,ForLoopPtr loopHeadPtr);

//this function goes through each pipeline tree and returns a new pipeline tree
//with their references replaced
PipelineListPtr manageReferencesInPipeline(PipelineListPtr pipeline,char* reference, char* filePath);

//this function goes through each of the individual fields of a command struct
//looking for references of varName and replace it with filePath
CommandPtr manageReferencesInCmd(CommandPtr cmd,char* reference, char* filePath);

//this function manages the reference in the
//current argument
ArgPtr manageReferencesInArg(ArgPtr currArg,char* reference,char* filePath);

//replaces the occurence of reference with filePath in text
//does so recursively until all of it has been analysed
//remLength is the remaining length of the text to be analysed
char* replaceOccurence(char* text,int remLength, char* reference, char* filePath);

#endif