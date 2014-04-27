#ifndef PRINT_H
#define PRINT_H

#include "structure.h"

typedef struct command COMMAND;
typedef COMMAND* CommandPtr;

typedef struct arg ARG;
typedef ARG* ArgPtr;

typedef struct for_loop FORLOOP;
typedef FORLOOP* ForLoopPtr;

typedef struct pipeline_list PIPELINELIST;
typedef PIPELINELIST* PipelineListPtr;

//print the entire prse tree
void printTree();

//print the stuff of for loop
void printForLoop(ForLoopPtr currForLoop);

//print the pipeline
void printPipeline(PipelineListPtr pipeline);

//print the command
void printCommand(CommandPtr cmd);

//print the argument
void printArg(ArgPtr argPtr);

//print the file
void printFile(FilePtr filePtr);

//print the hostMap[] array
void printHostMap();

#endif