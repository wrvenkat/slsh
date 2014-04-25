#include "print.h"

void printFile(FilePtr filePtr);

void printArg(ArgPtr argPtr);

int printCommand(CommandPtr cmd){
  int retVal=1;
  printf("%s ",cmd->name);  
  ArgPtr tempArgPtr = cmd->headArgs;
  //printf("Printing Args\n");
  while(tempArgPtr){
    printArg(tempArgPtr);
    tempArgPtr=tempArgPtr->next;
  }  
  if(cmd->inputRedir){
    printf("< ");
    retVal=0;
  }
  if(cmd->outputRedir){
    printf("> ");
    retVal=0;
  }
  /*FilePtr tempFilePtr = cmd->headFiles;
  while(tempFilePtr){
    printFile(tempFilePtr);  
    tempFilePtr=tempFilePtr->next;
  }*/
  return retVal;
}

void printTree(){
  CommandPtr cmdPtr = cmdHeadPtr;
  printf("--------------------------------------------------\n");
  while(cmdPtr){
    if(printCommand(cmdPtr) && cmdPtr->next!=0)
      printf(" | ");
    cmdPtr=cmdPtr->next;
  }
  printf("\n");
  printf("--------------------------------------------------\n");
}

void printHostMap(){
  int i=0;
  for(i=0;i<maxHost;i++){
    printf("%d %s %s %d\n",i,hostMap[i]->mnt_dir,hostMap[i]->mnt_fsname,hostMap[i]->minId);
  }
}


void printArg(ArgPtr argPtr){
  printf("%s ",argPtr->text);
}