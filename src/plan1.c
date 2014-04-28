#include "structure.h"
#include "globals.h"
#include <sys/stat.h>
#include <mntent.h>
#include <stdlib.h>
#include <regex.h>
#include "util.h"
#include "plan1.h"
#include "enums.h"

//this function basically calls the processWord1
//function because the '\ ' sequence is recognised
//by the stat function
FilePtr processPath1(ArgPtr pathArg){  
  return processWord1(pathArg);
}

//processes the Word argument and returns a FILE_ struct if true
FilePtr processWord1(ArgPtr wordArg){
  if(!wordArg)	return 0;
  if(DBG_PLAN)
    printf("processWord1 for arg %s\n",wordArg->text);
  FilePtr tempFilePtr =0;
  //check if the word is a path or a local file
  if(isFilePath(wordArg->text)){
    tempFilePtr = getFileStruct(wordArg->text);    
    if(tempFilePtr){
      if(DBG_GEN)
	printf("processWord1: Host:%d ActualPath:%s Size:%d\n",tempFilePtr->host,tempFilePtr->actualPath,tempFilePtr->size);
      return tempFilePtr;
    }
  }
  /*if(strlen(wordArg->text)==1){
    char* newFilePath = malloc(sizeof(char)*1);
    sprintf(newFilePath,"%s",wordArg->text);
    
  }*/
  //this is to avoid filePaths which have '//' or '.' or '..' in them
  else
    if(wordArg->text[0]=='/' || wordArg->text[0]=='.' ||  (wordArg->text[0]=='.' && wordArg->text[1]=='.'))
      return 0;
  //if its not a file, then check if the file is present in the
  //current working directory
  char* newFilePath = (char*)malloc(sizeof(char)*(strlen(currWD)+strlen(wordArg->text)+1));
  memset(newFilePath,0,strlen(currWD)+strlen(wordArg->text)+1);
  sprintf(newFilePath,"%s/%s",currWD,wordArg->text);
  if(DBG_GEN)
    printf("processWord1: The newFilePath is %s\n",newFilePath);  
  tempFilePtr = getFileStruct(newFilePath);
  free(newFilePath);
  return tempFilePtr;
}

//this function processes an argument and returns a 
//filestructure pointer if the argument word is a valid file
FilePtr makeArgPlan1(ArgPtr currArg){
  if(!currArg)	return 0;
  if(DBG_PLAN)
    printf("makeArgPlan1 for arg %s\n",currArg->text);
  ArgPtr tempArgPtr = currArg;
  if(!tempArgPtr)
    return 0;  
  //FilePtr fileList = 0;
  //int fileListSize=0;
  ArgType argType = tempArgPtr->type;    
  switch(argType){
    case WORDT:
      return processWord1(tempArgPtr);
      break;
    case SOPTT:
      //tempFile = processSmallOpt1(tempArgPtr);
      break;
    case LOPTT:
      //TODO: implement the code to find potential filepaths
      //from the value for the Long Options
      //tempFile = processLongOpt1(tempArgPtr);
      break;
    case PATHT:
      return processPath1(tempArgPtr);
      break;
    case QARGT:
      //TODO: possibly implement an aggressive scan for potential
      //fliepaths here
      //tempFile = processQArg1(tempArgPtr);
      break;
    default:
      break;
  }
  return 0;
}

//this function sets the host on which this command 
//is to be executed based on the largest file size
//present in its arg list
Plan1StructPtr makeCmdPlan1(CommandPtr cmd, double ipCost,int prevExecHost){
  if(!cmd)	return 0;
  if(DBG_PLAN)
    printf("makeCmdPlan1 for the command %s\n",cmd->name);
  ArgPtr tempArgPtr = cmd->headArgs;
  u_int32_t totalFileCost=0;
  double transferCost=0;
  double outputCost=0;
  if(!tempArgPtr && cmd->currOutputRedir)
    tempArgPtr = createArg(strdup(cmd->name),WORDT);
  else if(!tempArgPtr)
    return 0;
  
  //this array holds the total file sizes
  //for hosts corresponding to the files
  //present in this command
  unsigned int hostFileSize[maxHost];
  int i=0;
  while(i<maxHost)
    hostFileSize[i++]=0;
  int targetHost=0;
  //find the file with the largest file size
  while(tempArgPtr){    
    FilePtr currFile = makeArgPlan1(tempArgPtr);      
    if(currFile){
      hostFileSize[currFile->host]+=currFile->size;
      //also calculate the totalFileCost
      totalFileCost+=currFile->size;
      tempArgPtr->filePtr=currFile;      
    }
    else
      tempArgPtr->filePtr=0;
    tempArgPtr=tempArgPtr->next;    
  }
  //now find the target host
  i=1;
  targetHost=0;
  
  //get the targetHost based on the max bytes to be transferred
  while(i<maxHost){
    if(hostFileSize[targetHost]<hostFileSize[i])
      targetHost=i;
    i++;
  }
  //get the trargetHost
  transferCost = hostFileSize[targetHost];
  
  //calculate the targethost
  //if the ipCost is not -ve then we need to include that in the
  //target host calculation
  if(ipCost>=0 && !(cmd->currOutputRedir)){
    if(ipCost>hostFileSize[targetHost])
      targetHost=prevExecHost;
    //else the targetHost remains the same
  }
  //else
  /*//means no preference of host to be executed
    cmd->execHost=-1;*/
  
  //set the host where this command needs to be executed
  cmd->execHost=targetHost;
   
  //get the cmdStat for this command
  CmdStatPtr cmdStat = cmdStatLookup(cmd->name);  
  //calculate the outputCost
  if(cmdStat){
    if(cmdStat->type==IPCOST){
      if(ipCost>=0)
	outputCost = ipCost*(cmdStat->costConstant);
    }
    else if(cmdStat->type==ARGCOST)
      outputCost = totalFileCost*(cmdStat->costConstant);
    else if(cmdStat->type==BOTHCOST)
      outputCost = (totalFileCost+ipCost)*(cmdStat->costConstant);    
  }
  //if we don't get the cmdStat for the command, we set it to 0
  else
    outputCost = 0;
  
  if(DBG_GEN)
    printf("makeCmdPlan1: The ipCost:%f transferCost:%f outputCost:%f\n",ipCost,transferCost,outputCost);
    
  if(DBG_PLAN){
    printf("The target execution host for %s command is %d\n",cmd->name, targetHost);
    printf("----------------------\n");
  }
  
  Plan1StructPtr plan1StructPtr = malloc(sizeof(PLAN1STRUCT));
  memset(plan1StructPtr,0,sizeof(PLAN1STRUCT));
  plan1StructPtr->opCost = outputCost;
  plan1StructPtr->execHost=targetHost;
  //we return the plan1StructPtr
  return plan1StructPtr;
}

//the bootstrap function to make the plan starting with each command
void makePlan1(CommandPtr cmdHeadPtr){
  if(DBG_PLAN){
    printf("------------begin Plan1---------------\n");
    printf("Inside makePlan1\n");
  }
  if(!cmdHeadPtr)
    return;  
  CommandPtr tempCmdPtr = cmdHeadPtr;
  Plan1StructPtr prevPlan1=0;
  int i=0;
  while(tempCmdPtr){
    //first time
    if(i==0){
      prevPlan1 = makeCmdPlan1(tempCmdPtr,-1,-1);
      i++;
    }
    //otherwise
    else{
      if(prevPlan1){
	double opCost = prevPlan1->opCost;
	int prevExecHost = prevPlan1->execHost;
	free(prevPlan1);
	prevPlan1 = makeCmdPlan1(tempCmdPtr,opCost,prevExecHost);
      }
      else
	prevPlan1 = makeCmdPlan1(tempCmdPtr,0,0);
    }
    tempCmdPtr=tempCmdPtr->next;
  }
  if(DBG_PLAN)
    printf("------------end Plan1---------------\n");
}

//this function traverses the pipeline list and fixes any < filename with cat filename
//for now all we want is the behaviour and not how fast or how slow cat is vs. <
void fixIPRedir(CommandPtr cmdHeadPtr){
  if(!cmdHeadPtr)	return;  
  CommandPtr tempCmdPtr = cmdHeadPtr;
  
  while(tempCmdPtr && !(tempCmdPtr->inputRedir))    
    tempCmdPtr=tempCmdPtr->next;
  //if no ip redir was found
  if(!tempCmdPtr)	return;  
  CommandPtr inputRedirCmd = tempCmdPtr->next;
  //adjust the pointer  
  tempCmdPtr->next = inputRedirCmd->next;
  //copy the pipeline properties of inputRedirCmd
  tempCmdPtr->inputRedir =  inputRedirCmd->inputRedir;
  tempCmdPtr->outputRedir =  inputRedirCmd->outputRedir;
  //create the new arg for the cat command
  ArgPtr catArg = createArg(strdup(inputRedirCmd->name),PATHT);
  CommandPtr catCmdPtr = createCommand("cat",catArg);
  catCmdPtr->next = cmdHeadPtr;
  cmdHeadPtr = catCmdPtr;
  //now free the useless inputredir node
  //an ugly free
  freeCommandPtr(inputRedirCmd);
}