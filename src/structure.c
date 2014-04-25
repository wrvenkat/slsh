#include "structure.h"
#include <stdlib.h>
#include <unistd.h>
#include "globals.h"

/* creates a new command*/
CommandPtr createCommand(char* cmdName, ArgPtr argsList){
  CommandPtr newCommand = (CommandPtr)malloc(sizeof(COMMAND));
  memset(newCommand,0,sizeof(COMMAND));
  newCommand->name = strdup(cmdName);
  newCommand->headArgs = argsList;
  return newCommand;
}

/*creates a new FILELIST*/
FilePtr createFileList(char* path, int host){
  FilePtr filePtr = (FilePtr)malloc(sizeof(FILE_));
  memset(filePtr,0,sizeof(FILE_));
  filePtr->origPath=path;
  filePtr->remotePath=0;
  filePtr->host=host;
  filePtr->next=0;
  return filePtr;
}

/*create a new arg*/
ArgPtr createArg(char* text, ArgType type){
  ArgPtr newArg = (ArgPtr)malloc(sizeof(ARG));
  newArg->type=type;
  newArg->text=text;
  newArg->filePtr=0;
  newArg->next=0;
  return newArg;
}

//this function deallocates the CommandPtr
//and its idividual fields
void freeCommandPtr(CommandPtr cmd){
  free(cmd->name);
  ArgPtr currArg = cmd->headArgs;
  while(currArg){
    ArgPtr tempArg = currArg->next;
    freeArgPtr(currArg);
    currArg = tempArg;
  }
  free(cmd);
}

//this function deallocates the given ArgPtr
// and all of its fields
void freeArgPtr(ArgPtr arg){
  free(arg->text);
  FilePtr currFile = arg->filePtr;
  while(currFile){
    FilePtr tempFile = currFile->next;
    freeFilePtr(currFile);
    currFile= tempFile;
  }
  free(arg);
}

//this function deallocates the given FilePtr
// and all of its fields
void freeFilePtr(FilePtr currFile){
  free(currFile->actualPath);
  free(currFile->origPath);
  free(currFile->remotePath);
  free(currFile);
}
