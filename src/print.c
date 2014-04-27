#include "print.h"
#include "for_loop.h"
#include "structure.h"

//print the entire prse tree
void printTree(){
  //InputUnitPtr inputUnit = headPtr;
  printf("-----------Begin printTree----------------------------------\n");
  if(headPtr->type==FORLOOPT)
    printForLoop((ForLoopPtr)(headPtr->inputUnit));
  else if(headPtr->type==PIPELINELST)
    printPipeline((PipelineListPtr)(headPtr->inputUnit));
  printf("\n");
  printf("-----------End printTree------------------------------------\n");
}

//print the stuff of for loop
void printForLoop(ForLoopPtr currForLoop){
  if(!currForLoop)	return;
  printf("for %s in %s do\n",currForLoop->varName,currForLoop->expr);
  PipelineListPtr currPipelineList = currForLoop->pipeline;
  //print each of the pipeline list in this for loop
  while(currPipelineList){
    printf("\t");
    printPipeline(currPipelineList);
    printf(";\n");
    currPipelineList=currPipelineList->next;
  }
  printf("done\n");
}

//print the pipeline
void printPipeline(PipelineListPtr pipeline){
  printf("--------begin printPipeline-----------------\n");
  CommandPtr cmdPtr = pipeline->headCommand;  
  while(cmdPtr){
    printCommand(cmdPtr);
    cmdPtr=cmdPtr->next;
  }
  printf("\n--------end printPipeline-----------------\n");
}

//print the command
void printCommand(CommandPtr cmd){  
  printf("%s ",cmd->name);
  ArgPtr tempArgPtr = cmd->headArgs;  
  while(tempArgPtr){
    printArg(tempArgPtr);
    tempArgPtr=tempArgPtr->next;
  }
  if(cmd->next &&  !(cmd->next->currOutputRedir))
    printf("| ");
  else if(cmd->next &&  cmd->next->currOutputRedir)
    printf("> ");  
}

//print the argument
void printArg(ArgPtr argPtr){
  printf("%s ",argPtr->text);
}

//print the hostMap[] array
void printHostMap(){
  int i=0;
  for(i=0;i<maxHost;i++){
    printf("%d MntDir:%s MntFsName:%s MinId:%d\n",i,hostMap[i]->mnt_dir,hostMap[i]->mnt_fsname,hostMap[i]->minId);
  }
}
