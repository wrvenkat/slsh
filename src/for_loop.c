#include "for_loop.h"
#include <stdlib.h>
#include <stdio.h>
#include <glob.h>
#include "enums.h"
#include "structure.h"

//creates a new for loop
ForLoopPtr createForLoop(char* varName, char* expr){
  ForLoopPtr newForLoop = malloc(sizeof(FORLOOP));
  newForLoop->varName = varName;
  newForLoop->expr = expr;
  newForLoop->newPipeline=0;
  newForLoop->pipeline=0;
  newForLoop->next=0;
  return newForLoop;
}

//frees the ForLoop and its fields
void freeForLoop(ForLoopPtr forLoop){
  if(!forLoop)	return;
  printf("Inside freeForLoop\n");
  freePipelineList(forLoop->pipeline);
  freePipelineList(forLoop->newPipeline);
  free(forLoop->expr);
  free(forLoop->varName);
  free(forLoop);
}

//this makes the list of filepath string that confirm to the
// expr in for_loop
glob_t* getFileList(char* expr){
  //we need to get rid of the "" surrounding the
  //expression string
  //printf("Given expression is %s with length %d\n",expr,strlen(expr));
  int len=strlen(expr)-2;
  //printf("Len is %d\n",len);
  char newExpr[len+1];
  memset(newExpr,0,len+1);
  //printf("Char at expr+1 is %c and char at expr+1+len is %c\n",*(expr+1),*(expr+1+len));
  snprintf(newExpr,len+1,"%s",expr+1);
  printf("New expression is %s\n",newExpr);
  size_t cnt, length;
  glob_t* glob_results = malloc(sizeof(glob_t));
  //get the glob results for the current directory
  int retValue = glob(newExpr, GLOB_NOCHECK, 0, glob_results);
  if(retValue!=0){
    if(retValue==GLOB_NOMATCH)
      printf("No match found!\n");
    return 0;
  }
  printf("Number of matching path here is %d\n",(int)(glob_results->gl_pathc));
  //printf("THe first one is %s\n",glob_results->gl_pathv[0]);
  if(!strcmp(glob_results->gl_pathv[0],newExpr)){
    printf("glob_results empty!\n");
    return 0;
  }  
  return glob_results;  
}

//the bootstrap function to manage references in the pipeline list
int manageReferences(ForLoopPtr currForLoop){
  glob_t* glob_results = getFileList(currForLoop->expr);
  if(!glob_results)
    return 0;  
  manageReferencesInLoop(currForLoop->varName,(int)(glob_results->gl_pathc),glob_results->gl_pathv,currForLoop);
  globfree(glob_results);
  return 1;
}

//this function manages or replaces references in
//each of the pipeline in the pipeline list
int manageReferencesInLoop(char* varName, int count, char** fileList,ForLoopPtr loopHeadPtr){
  //now that we have the list of files, we go over the
  //syntax/parse tree for each of the pipeline in the pipeline
  //list and replace any references with the full file path
  char* filePath = 0;
  int length = 0;
  ForLoopPtr currForLoop = loopHeadPtr;
  //for each of the for loop
  //for now there is only on for loop
 
  int i=0;
  PipelineListPtr newPipelineHead=0;
  PipelineListPtr newPipelineTail=0;
  char* reference = malloc(sizeof(char)*(strlen(varName)+2));
  sprintf(reference,"$%s",varName);
  printf("The reference is %s\n",reference);
  while(i<count){
    PipelineListPtr newPipeline = 0;      
    PipelineListPtr currPipeline = currForLoop->pipeline;
    length = strlen(currWD)+strlen(fileList[i]);
    filePath = malloc(sizeof(char)*(length+1+1));
    memset(filePath,0,length+1+1);
    sprintf(filePath,"%s/%s",currWD,fileList[i]);      
    //printf("The actual filepath is %s and the reference is %s\n",filePath,reference);
    //for each of the pipeline get a new pipeline for each occurence of the glob
    //and has the reference replaced and add that reference list to the for loop
    while(currPipeline){
      newPipeline= manageReferencesInPipeline(currPipeline,reference,/*filePath*/fileList[i]);
      if(newPipeline){
	if(!newPipelineHead)
	  newPipelineHead=newPipelineTail=newPipeline;
	else{
	  newPipelineTail->next=newPipeline;
	  newPipelineTail=newPipeline;
	}
      }
      currPipeline=currPipeline->next;
    }
    free(filePath);      
    i++;
  }
  currForLoop->newPipeline = newPipelineHead;
  free(reference);  
  return 1;
}

//this function goes through each pipeline tree and returns a new pipeline tree
//with their references replaced
PipelineListPtr manageReferencesInPipeline(PipelineListPtr pipeline,char* reference, char* filePath){
  //printf("Inside manageReferencesInPipeline\n");
  CommandPtr currCmdPtr = pipeline->headCommand;
  CommandPtr cmdPtrListHead=0;
  CommandPtr cmdPtrListTail=0;
  CommandPtr tempCmdPtr = 0;
  //first fix the IP redirection
  fixIPRedir(currCmdPtr);
  while(currCmdPtr){    
    tempCmdPtr = manageReferencesInCmd(currCmdPtr,reference,filePath);
    //now add tempCmdPtr to the new pipeline
    if(tempCmdPtr){
      if(!cmdPtrListHead)
	cmdPtrListHead=cmdPtrListTail=tempCmdPtr;
      else{
	cmdPtrListTail->next=tempCmdPtr;
	cmdPtrListTail=tempCmdPtr;
      }
    }
    currCmdPtr=currCmdPtr->next;
  }
  PipelineListPtr newPipeline = createPipelineList();
  newPipeline->headCommand=cmdPtrListHead;
  //print the pipeline and see
  printf("--------begin printPipeline-----------------\n");
  printPipeline(newPipeline);
  printf("\n--------end printPipeline-----------------\n");
  return newPipeline;
}

//this function goes through each of the individual fields of a command struct
//looking for references of varName and replace it with filePath
CommandPtr manageReferencesInCmd(CommandPtr cmd,char* reference, char* filePath){
  //printf("Inside manageReferencesInCmd for command %s\n",cmd->name);
  //create a new CommandPtr
  CommandPtr newCmd = createCommand(cmd->name,0);  
  newCmd->currOutputRedir=cmd->currOutputRedir;
  newCmd->inputRedir=cmd->inputRedir;
  newCmd->outputRedir=cmd->outputRedir;
  newCmd->execHost=cmd->execHost;
  ArgPtr argListHead=0;
  ArgPtr argListTail=0;
  ArgPtr currArg=0;  
  //replace the cmd->name if it matches the 
  //reference
  char* replacement = replaceOccurence(strdup(newCmd->name),strlen(newCmd->name),reference,filePath);
  //printf("The new command name is %s\n",replacement);
  if(replacement){
    free(newCmd->name);
    newCmd->name = replacement;
    printf("The new command name is %s\n",replacement);
  }
  ArgPtr currArgPtr = cmd->headArgs;
  //now the arguments
  while(currArgPtr){
    currArg= manageReferencesInArg(currArgPtr,reference,filePath);
    //add it to the list of arguments for this command
    if(currArg){
      if(!argListHead)
	argListHead=argListTail=currArg;
      else{
	argListTail->next=currArg;
	argListTail=currArg;
      }
    }
    currArgPtr=currArgPtr->next;
  }
  //set the arguments list head
  newCmd->headArgs=argListHead;  
  return newCmd;
}

//this function manages the reference in the
//current argument
ArgPtr manageReferencesInArg(ArgPtr currArg,char* reference,char* filePath){
  //printf("Inside manageReferencesInArg for argument %s\n",currArg->text);
  ArgPtr newArg = createArg(strdup(currArg->text),currArg->type);
  //look for the reference anywhere in the text of the argument
  //besides we don't have to look at other fields because they musn't have
  //been used or populated yet
  printf("The original text is:%s\n",newArg->text);
  char* replacedText = replaceOccurence(newArg->text,strlen(newArg->text), reference,filePath);
  if(replacedText){
    free(newArg->text);
    newArg->text = replacedText;
    printf("Post replacement text is:%s\n",replacedText);
  }  
  return newArg;
}

//replaces the occurence of reference with filePath in text
//does so recursively until all of it has been analysed
//remLength is the remaining length of the text to be analysed
char* replaceOccurence(char* text,int remLength, char* reference, char* filePath){
  //printf("Inside replaceOccurence for text %s and length %d\n",text,remLength);
  if(remLength<=0 || !text)	return 0;
  //get the position of the first occurence of the
  //reference
  char* posPtr = strstr(text,reference);
  if(posPtr){
    char* replacedText = replaceOccurence(posPtr+strlen(reference),remLength-strlen(reference)-(posPtr-text),reference,filePath);
    //if replacedText is not NULL, then it means we genuinely have a text that was replaced
    //so we have to use that in our new replacement
    if(replacedText){
      char* replacedTextNew = 0;
      char* charPtr=0;
      int length = 0;
      length+=posPtr-text	//the lenght of string from text to the start of occurence of reference
	     +strlen(filePath)	//the length of the replacement filePath
	     +strlen(replacedText);	//the length of the replaced string from the recursion
      replacedTextNew = malloc(sizeof(char)*(length+1));
      memset(replacedTextNew,0,length+1);
      charPtr = replacedTextNew;
      snprintf(charPtr,posPtr-text+1,"%s",text);
      charPtr+=posPtr-text;      
      sprintf(charPtr,"%s%s",filePath, replacedText);
      free(replacedText);
      printf("1 Original Text here is:%s\n",text);
      printf("1 Replaced Text here is:%s\n",replacedTextNew);
      return replacedTextNew;
    }
    else{
      char* charPtr = 0;
      char* replacedText = 0;
      int length = 0;
      length+=posPtr-text 				//this length is length from the start of the text to 
							//to the point where the reference begins
	     +strlen(filePath)				//the length of the replacement string
	     +strlen(posPtr+strlen(reference));		//this is the length of the string that remains after
							//the end of the reference till the end of the string
      replacedText = malloc(sizeof(char)*(length+1));
      memset(replacedText,0,length+1);
      charPtr = replacedText;
      snprintf(charPtr,posPtr-text+1,"%s",text);
      charPtr+=posPtr-text;
      sprintf(charPtr,"%s%s",filePath,posPtr+strlen(reference));
      //printf("2 Original Text here is:%s\n",text);
      //printf("2 Replaced Text here is:%s\n",replacedText);
      return replacedText;
    }
  }
  else
    return 0;
}