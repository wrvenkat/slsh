#ifndef STRUCTURE_H
#define STRUCTURE_H

#include <stdio.h>
#include <string.h>
#include "globals.h"

//types of command-line arguments
typedef enum {WORDT, SOPTT, LOPTT, PATHT, QARGT}ArgType;

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

//head and tail pointers for the Command list
CommandPtr cmdHeadPtr;

/* creates a new command*/
CommandPtr createCommand(char* cmdName, ArgPtr argsList);

/* inserts a Command into the command list*/
void insertCommand(CommandPtr commandPtr);

/*creates a new FILELIST*/
FilePtr createFileList(char* path, int host);

/*insert at the end of the args list of the command*/
void insertFile(FilePtr filePtr,CommandPtr command);

/*create a new arg*/
ArgPtr createArg(char* text, ArgType type);

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

#endif