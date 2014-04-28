#ifndef _PLAN_H
#define _PLAN_H

#include "structure.h"
#include "util.h"
#include <stdlib.h>

typedef struct file FILE_;
typedef FILE_* FilePtr;

typedef struct command COMMAND;
typedef COMMAND* CommandPtr;

typedef struct arg ARG;
typedef ARG* ArgPtr;

typedef struct plan1struct{
  double opCost;
  int execHost;
}PLAN1STRUCT;

typedef PLAN1STRUCT* Plan1StructPtr;

//this function populates the global
//hostMap array
void createHostMap();

//this function basically calls the processWord1
//function because the '\ ' sequence is recognised
//by the stat function
FilePtr processPath1(ArgPtr pathArg);

//processes the Word argument and returns a FILE_ struct if true
FilePtr processWord1(ArgPtr wordArg);

//this function processes an argument and returns a 
//filestructure pointer if the argument word is a valid file
FilePtr makeArgPlan1(ArgPtr currArg);

//this function sets the host on which this command 
//is to be executed based on the largest file size
//present in its arg list
Plan1StructPtr makeCmdPlan1(CommandPtr cmd,double ipCost,int prevExecHost);

//the bootstrap function to make the plan starting with each command
void makePlan1(CommandPtr cmdHeadPtr);

//loads the command stats from cmdStatsFileName onto cmdStats array
void loadCmdStats(char* cmdStatsFileName);

//this function traverses the pipeline list and fixes any < filename with cat filename
//for now all we want is the behaviour and not how fast or how slow cat is vs. <
void fixIPRedir(CommandPtr cmdHeadPtr);

#endif