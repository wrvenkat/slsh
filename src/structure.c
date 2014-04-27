#include "structure.h"
#include <stdlib.h>
#include <unistd.h>
#include "globals.h"
#include "enums.h"
#include "print.h"
#include "for_loop.h"
#include "remotecmd.h"

/* creates a new command*/
CommandPtr createCommand(char* cmdName, ArgPtr argsList){
  CommandPtr newCommand = (CommandPtr)malloc(sizeof(COMMAND));
  memset(newCommand,0,sizeof(COMMAND));
  newCommand->name = strdup(cmdName);
  newCommand->headArgs = argsList;
  return newCommand;
}

/*creates a new FILELIST*/
FilePtr createFile(char* path, int host){
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

//create an InputUnityType
InputUnitPtr createInputUnit(InputUnitType type){
  InputUnitPtr ipUnitPtr = malloc(sizeof(INPUTUNIT));
  ipUnitPtr->type = type;  
  return ipUnitPtr;
}

//create PipelineListPtr
PipelineListPtr createPipelineList(){
  PipelineListPtr pipelineListPtr = malloc(sizeof(PIPELINELIST));
  memset(pipelineListPtr,0,sizeof(PIPELINELIST));
  return pipelineListPtr;
}

//the bootstrap function that starts the processing of the whole parse tree
void startProcessing(){
  if(DBG_HOSTMAP_PRINT)
    printHostMap();
  InputUnitPtr ipUnitPtr = headPtr;
  if(!ipUnitPtr)	return;
  if(DBG_CMD_TREE_PRINT)
    printTree();
  if(ipUnitPtr->type==FORLOOPT){
    ForLoopPtr currForLoop = (ForLoopPtr)(headPtr->inputUnit);
    //fix any references in the forloop    
    manageReferences(currForLoop);    
    //now process the for loop
    processForLoop(currForLoop);
    freeForLoop(currForLoop);
  }
  else if(ipUnitPtr->type==PIPELINELST){
    processPipelineList((PipelineListPtr)(ipUnitPtr->inputUnit));
    freePipelineList((PipelineListPtr)(ipUnitPtr->inputUnit));
  }
}

//function used by startProcessing
void processForLoop(ForLoopPtr forLoop){
  PipelineListPtr currPipeline = forLoop->newPipeline;  
  processPipelineList(currPipeline);  
}

//function used by startProcessing
void processPipelineList(PipelineListPtr pipeline){
  PipelineListPtr currPipeline = pipeline;
  while(currPipeline){
    processPipeline(pipeline);
    currPipeline=currPipeline->next;
  }
}

//function used by startProcessing
void processPipeline(PipelineListPtr pipeline){
  CommandPtr cmdHeadPtr = pipeline->headCommand;
  makePlan1(cmdHeadPtr);  
  RemoteCmdPtr remoteCmdHeadPtr =  makeRemoteCmd(cmdHeadPtr);
  if(DBG_RMTE_CMD_TREE_PRINT)
    printRemoteCmdTree(remoteCmdHeadPtr);  
  executePlan(remoteCmdHeadPtr);
}

//function that frees all the pipelines in this list
void freePipelineList(PipelineListPtr currPipeline){
  if(!currPipeline)	return;
  if(DBG_FREE)
    printf("Inside freePipelineList\n");
  PipelineListPtr tempPipeline = currPipeline;  
  PipelineListPtr tempNextPipeline = 0;  
  while(tempPipeline){
    tempNextPipeline=tempPipeline->next;
    freePipeline(tempPipeline->headCommand);
    tempPipeline=tempNextPipeline;
  }
}

//frees all the commands in the current pipeline
void freePipeline(CommandPtr headCmdPtr){
  if(!headCmdPtr)	return;
  if(DBG_FREE)
    printf("Inside freePipeline\n");
  CommandPtr currCmdPtr = headCmdPtr;
  CommandPtr tempCmdPtr = 0;
  while(currCmdPtr){
    tempCmdPtr=currCmdPtr->next;
    freeCommandPtr(currCmdPtr);
    currCmdPtr=tempCmdPtr;
  }
}

//this function deallocates the CommandPtr
//and its idividual fields
void freeCommandPtr(CommandPtr cmd){
  if(!cmd)	return;
  if(DBG_FREE)
    printf("Inside freeCommandPtr\n");
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
  if(!arg)	return;
  if(DBG_FREE)
    printf("Inside freeArgPtr for arg %s\n",arg->text);
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
  if(!currFile)	return;
  if(DBG_FREE)
    printf("Inside freeFilePtr freeing %s\n",currFile->origPath);
  free(currFile->actualPath);
  free(currFile->origPath);
  free(currFile->remotePath);
  free(currFile);
}