#ifndef STRUCTURE_H
#define STRUCTURE_H

#include <stdio.h>
#include <string.h>
#include "globals.h"
#include "for_loop.h"
#include "plan1.h"
#include "remotecmd.h"
#include "execute.h"
#include "print.h"
#include "enums.h"

typedef struct for_loop FORLOOP;
typedef FORLOOP* ForLoopPtr;

typedef struct file{
  char* origPath;
  char* actualPath;
  char* remotePath;
  int host;
  int minId;
  unsigned int size;
  struct file* next;
}FILE_;

typedef FILE_* FilePtr;

typedef struct arg{
  ArgType type;
  char* text;
  struct file* filePtr;
  struct arg* next;
}ARG;

typedef ARG* ArgPtr;

typedef struct command{
  char* name;
  //means that the current command is after
  //a > operator.
  //TODO this is ugly and temporary
  //use the already present flags to
  //achieve what you want
  int currOutputRedir;
  int inputRedir;
  int outputRedir;
  ArgPtr headArgs;
  int execHost;
  struct command* next;
}COMMAND;

typedef COMMAND* CommandPtr;

typedef struct pipeline_list{
  CommandPtr headCommand;
  struct pipeline_list* next;
}PIPELINELIST;

typedef PIPELINELIST* PipelineListPtr;

typedef struct input_unit{
  InputUnitType type;
  void* inputUnit;
}INPUTUNIT;

typedef INPUTUNIT* InputUnitPtr;

typedef struct host_info{
  char* mnt_fsname;
  char* mnt_dir;
  int minId;
}HOSTINFO;

typedef HOSTINFO* HostInfoPtr;

//the host's IP address is indexed by their
//array position
HostInfoPtr hostMap[MAX_HOST];
int maxHost;
char* currWD;

//the all contained head pointer
InputUnitPtr headPtr;

//head and tail pointers for the Command list
//CommandPtr cmdHeadPtr;

/* creates a new command*/
CommandPtr createCommand(char* cmdName, ArgPtr argsList);

/* inserts a Command into the command list*/
void insertCommand(CommandPtr commandPtr);

/*creates a new FILELIST*/
FilePtr createFile(char* path, int host);

/*insert at the end of the args list of the command*/
void insertFile(FilePtr filePtr,CommandPtr command);

/*create a new arg*/
ArgPtr createArg(char* text, ArgType type);

//create an InputUnityType
InputUnitPtr createInputUnit(InputUnitType type);

//create PipelineListPtr
PipelineListPtr createPipelineList();

/*insert at the end of the args list of command*/
void insertArg(ArgPtr arg, CommandPtr command);

/*get the host number associated with this minId*/
int getHostId(int minId);

//this function deallocates the CommandPtr
//and its idividual fields
void freeCommandPtr(CommandPtr cmd);

//this function deallocates the given ArgPtr
// and all of its fields
void freeArgPtr(ArgPtr arg);

//this function deallocates the given FilePtr
// and all of its fields
void freeFilePtr(FilePtr currFile);

//function that frees all the pipelines in this list
void freePipelineList(PipelineListPtr currPipeline);

//frees all the commands in the current pipeline
void freePipeline(CommandPtr headCmdPtr);

//the bootstrap function that starts the processing of the whole parse tree
void startProcessing();

//function used by startProcessing
void processForLoop(ForLoopPtr forLoop);

//function used by startProcessing
void processPipelineList(PipelineListPtr pipeline);

//function used by startProcessing
void processPipeline(PipelineListPtr pipeline);

#endif