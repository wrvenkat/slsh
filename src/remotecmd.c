#include "remotecmd.h"
#include "globals.h"
#include "structure.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>
#include "structure.h"

//this function returns an initalised REMOTECMD struct pointer
RemoteCmdPtr createRemoteCmd(){
  RemoteCmdPtr remoteCmdPtr = malloc(sizeof(REMOTECMD));
  remoteCmdPtr->host=-1;
  memset(remoteCmdPtr,0,sizeof(REMOTECMD));
  return remoteCmdPtr;
}

//this function returns an initalised REMOTECMDTEXT struct pointer
RemoteCmdTextPtr createRemoteCmdText(){
  RemoteCmdTextPtr remoteCmdTextPtr = malloc(sizeof(REMOTECMDTEXT));
  memset(remoteCmdTextPtr,0,sizeof(REMOTECMDTEXT));
  return remoteCmdTextPtr;
}

//prints the list of RemoteCmdText
void printRemoteCmdTextList(RemoteCmdTextPtr list){
  RemoteCmdTextPtr tempListPtr = list;
  while(tempListPtr){
    printf("Text: %s Type: %d\n",tempListPtr->cmdText->text,tempListPtr->type);
    tempListPtr=tempListPtr->next;
  }
}

//this function returns the altered filepath
StringPtr getFilePath(ArgPtr arg, int execHost){
  //printf("Argument here is %s\n",arg->text);
  //if the filePtr has been set, then we get the necessary details from there
  if(!(arg->filePtr)){
    //printf("FilePtr for argument is null\n");
    return 0;
  }  
  //get the home directory on the remote machine where the file is present
  int fileHostId = arg->filePtr->host;
  char* homeDir = getHomeDir(hostMap[fileHostId]->mnt_fsname);
  char* mountDir = hostMap[fileHostId]->mnt_dir;
  //look here!
  char* fileName = getFileName(arg->filePtr->actualPath);
  char* filePathStr = 0;
  StringPtr filePath = createString();
  int length=0;
  if(execHost==fileHostId){
    char* temp = getRidOfEscapeChars(arg->filePtr->origPath);
    length=strlen(homeDir)+(strlen(temp)-strlen(mountDir));
    filePathStr = malloc(sizeof(char)*(length+3));
    memset(filePathStr,0,length+3);
    sprintf(filePathStr,"\"%s%s\"",homeDir,strdup(temp)+strlen(mountDir)+1);
    filePath->text=filePathStr;
    filePath->length=length-1;
  }
  else{
    length=strlen(homeDir)+strlen(fileName)+strlen(TEMP_DIR);
    length+=3;
    filePathStr = malloc(sizeof(char)*(length));
    memset(filePathStr,0,length);
    sprintf(filePathStr,"\"%s%s%s\"",homeDir,TEMP_DIR,fileName);
    filePath->text=filePathStr;
    filePath->length=length-1;
  }
  //printf("String is %s\n",filePathStr);
  //if(DEBUG1) printf("Modified filepath at getFilePath is %s of length %d\n",filePath->text,filePath->length);
  return filePath;
}

//gets the text for the target file of an output redirection
CommandPtr getRemoteCmdTextFromOutputCmd(CommandPtr cmd){
  //if the current command was part of op redir
  //we need to take care of its file path
  //TODO THERE IS A PROBLEM WITH /dev/null or /dev/* devices
  //not returning their actual major and minor id
  if(cmd->currOutputRedir){
    if(isFile(cmd->name)){
      FilePtr currDirFile = getFileStruct(cmd->name);
      //if there is no such file
      if(!currDirFile){
	char* filePath = getRidOfEscapeChars(cmd->name);
	char* dir = getFileDir(cmd->name);
	printf("Dir here is %s\n",dir);
	FilePtr currFilePtr = getFileStruct(dir);
	//if there is no such directory
	if(!currFilePtr)	return 0;
	//if the directory exists
	else{
	  //if the directory is in a local machine
	  if(currFilePtr->host==0){	    
	    CommandPtr newCmdPtr = createCommand(strdup(cmd->name),0);	    
	    newCmdPtr->name = strdup(cmd->name);
	    newCmdPtr->execHost = currFilePtr->host;
	    printf("After making the command name now is %s\n",newCmdPtr->name);	    
	    return newCmdPtr;
	  }
	  else{	  
	    char* homeDir = getHomeDir(hostMap[currFilePtr->host]->mnt_fsname);
	    int length = strlen(homeDir)+(strlen(filePath)-strlen(hostMap[currFilePtr->host]->mnt_dir))+1;
	    char* text = malloc(sizeof(char)*(length+1));
	    memset(text,0,length+1);
	    sprintf(text,"%s%s",homeDir,filePath+1+strlen(hostMap[currFilePtr->host]->mnt_dir));
	    CommandPtr newCmdPtr = createCommand(text,0);
	    newCmdPtr->execHost = currFilePtr->host;
	    //printf("After making the command name now is %s\n",newCmdPtr->name);
	    return newCmdPtr;
	  }  
	}
      }
      //if such a file already exists
      else{
	//if such a file exists already, then it might be local or mounted
	if(currDirFile->host==0){
	  CommandPtr newCmdPtr = createCommand(strdup(cmd->name),0);	
	  newCmdPtr->execHost = currDirFile->host;
	  //printf("After making the command name now is %s\n",newCmdPtr->name);
	  return newCmdPtr;
	}
	else{
	  char* homeDir = getHomeDir(hostMap[currDirFile->host]->mnt_fsname);
	  int length = 0;
	  length = strlen(homeDir)+(strlen(cmd->name)-strlen(hostMap[currDirFile->host]->mnt_dir));
	  char* text = malloc(sizeof(char)*(length+1));
	  memset(text,0,length+1);
	  sprintf(text,"%s%s",homeDir,strdup(cmd->name)+strlen(hostMap[currDirFile->host]->mnt_dir)+1);
	  //printf("The target dir is %s\n",text);
	  CommandPtr newCmdPtr = createCommand(text,0);
	  newCmdPtr->execHost = currDirFile->host;
	  //printf("After making the command name now is %s\n",newCmdPtr->name);
	  return newCmdPtr;
	}
      }
    }
    //if it is not a file then it should be relative to the current directory
    else{
      char* text = malloc(sizeof(char)*(strlen(currWD)+strlen(cmd->name)+2));
      memset(text,0,strlen(currWD)+strlen(cmd->name)+2);
      sprintf(text,"%s/%s",currWD,cmd->name);
      CommandPtr newCmdPtr = createCommand(text,0);      
      newCmdPtr->execHost = 0;
      printf("After making the command name now is %s\n",newCmdPtr->name);
      return newCmdPtr;
    }
  }
}

//this finction does the same as getRemoteCmdTextFromCmd but replaces the
//local mounted filepaths with the remote file paths
RemoteCmdTextPtr getRemoteCmdTextFromCmd(CommandPtr cmd){
  if(!cmd)
    return 0;
  //printf("IN HERE %s and execHost %d\n",cmd->name,cmd->execHost);
  RemoteCmdTextPtr remoteCmdTextPtr = createRemoteCmdText();
  int totalLengthCombined =0;  
  {
    totalLengthCombined+=strlen(cmd->name);
    ArgPtr argPtr = cmd->headArgs;
    StringPtr argListHeadPtr = 0;
    StringPtr argListTailPtr = 0;
    totalLengthCombined=0;
    //start with the command name
    argListHeadPtr=argListTailPtr=createString();
    argListHeadPtr->text=strdup(cmd->name);
    argListHeadPtr->length=strlen(cmd->name);
    totalLengthCombined+=strlen(cmd->name);
    StringPtr tempStrPtr=0;
    //now get the argsList and their length
    while(argPtr){
      if(argPtr->type==PATHT || argPtr->type==WORDT){
	tempStrPtr = getFilePath(argPtr,cmd->execHost);
	if(tempStrPtr){
	  //printf("The filePath at getRemoteCmdTextFromCmd is %s\n",tempStrPtr->text);
	  totalLengthCombined+=tempStrPtr->length+strlen(" ");
	}
	else{
	  tempStrPtr=createString();
	  tempStrPtr->text=strdup(argPtr->text);
	  tempStrPtr->length = strlen(argPtr->text);
	  totalLengthCombined+=tempStrPtr->length+strlen(" ");
	}
      }
      else{
	tempStrPtr=createString();
	tempStrPtr->text=strdup(argPtr->text);
	tempStrPtr->length = strlen(argPtr->text);
	totalLengthCombined+=tempStrPtr->length+strlen(" ");
      }
      //now add them to the argList
      if(tempStrPtr){
	if(!argListHeadPtr)	argListHeadPtr=argListTailPtr=tempStrPtr;
	else{
	  argListTailPtr->next=tempStrPtr;
	  argListTailPtr=tempStrPtr;
	}
      }
      argPtr=argPtr->next;
    }
    totalLengthCombined+=1;
    //now copy the arguments
    StringPtr remoteCmdText = createString();
    char* cmdText = malloc(sizeof(char)*totalLengthCombined);
    memset(cmdText,0,totalLengthCombined);
    char* cmdTextPtr = cmdText;
    tempStrPtr=0;
    tempStrPtr=argListHeadPtr;
    int i=0;
    while(tempStrPtr){
      if(i==0){
	sprintf(cmdTextPtr,"%s",tempStrPtr->text);
	//printf("cmdTextPtr here is %s\n",cmdTextPtr);
	cmdTextPtr+=tempStrPtr->length;
	i++;
      }
      else{
	sprintf(cmdTextPtr," %s",tempStrPtr->text);
	//printf("cmdTextPtr here is %s\n",cmdTextPtr);
	cmdTextPtr+=strlen(tempStrPtr->text)+strlen(" ");
      }
      
      tempStrPtr=tempStrPtr->next;
    }
    remoteCmdText->text=cmdText;
    remoteCmdText->length=totalLengthCombined;
    //freeStringList(argListHeadPtr);
    //if(DEBUG1)	printf("The command with modified filePath is %s\n",remoteCmdText->text);
    remoteCmdTextPtr->cmdText=remoteCmdText;
    return remoteCmdTextPtr;
  }
  return 0;
}

//this function returns a string to which all the cmdText 
//from remoteCmdTextList is copied from
//along with the proper | or > or  <
char* createCmdStringFromList(CommandPtr startCmd, CommandPtr endCmd, RemoteCmdTextPtr remoteCmdTextList,int length){
  CommandPtr tempCmdPtr=startCmd;
  RemoteCmdTextPtr tempRemoteCmdTextPtr = remoteCmdTextList;
  if(!tempCmdPtr || !tempRemoteCmdTextPtr)	return 0;
  char* combinedCmdString = malloc(sizeof(char)*(length+1));
  memset(combinedCmdString,0,length+1);
  char* tempStrPtr = combinedCmdString;  
  fflush(stdout);
  //we go over all the commands and the tempRemoteCmdTextPtr
  while(tempRemoteCmdTextPtr){    
    //now if the next command is an outputRedir, then we need to process
    //the command name to get the actual file path
    //printf("createCmdStringFromList processing command %s\n",tempRemoteCmdTextPtr->cmdText->text);
    if(tempCmdPtr->next && !(tempCmdPtr->next->currOutputRedir)){	
      sprintf(tempStrPtr,"%s | ",tempRemoteCmdTextPtr->cmdText->text);
      tempStrPtr+=strlen(tempRemoteCmdTextPtr->cmdText->text)+strlen(" | ");
    }
    else if(tempCmdPtr->currOutputRedir){
      sprintf(tempStrPtr," > %s",tempRemoteCmdTextPtr->cmdText->text);
      tempStrPtr+=strlen(tempRemoteCmdTextPtr->cmdText->text)+strlen(" > ");
    }
    else{
      sprintf(tempStrPtr,"%s",tempRemoteCmdTextPtr->cmdText->text);      
      tempStrPtr+=strlen(tempRemoteCmdTextPtr->cmdText->text);
    }    
    tempRemoteCmdTextPtr=tempRemoteCmdTextPtr->next;
    tempCmdPtr=tempCmdPtr->next;
  }
  if(DEBUG1) printf("The combined cmd string is %s\n",combinedCmdString);
  return combinedCmdString;
}

//this function gets the list of files to be transferred for command(s)
FilePtr getTransferFileList(CommandPtr currCmdPtr, CommandPtr prevCmdPtr){
  if(!currCmdPtr) return 0;
  //if there is only one command in the list
  if(!prevCmdPtr){    
    FilePtr currFilePtr = 0;
    FilePtr tempFilePtr = 0;
    FilePtr fileListHeadPtr = 0;
    FilePtr fileListTailPtr = 0;
    ArgPtr currArgPtr = 0;
    currArgPtr = currCmdPtr->headArgs;
    while(currArgPtr){
      tempFilePtr = currArgPtr->filePtr;
      //if we do have a file argument here
      //then we clone it
      if(tempFilePtr && tempFilePtr->host!=currCmdPtr->execHost){	
	StringPtr remoteFilePath = getFilePath(currArgPtr,currCmdPtr->execHost);
	if(remoteFilePath){
	  currFilePtr = createFileList(strdup(tempFilePtr->origPath),tempFilePtr->host);
	  currFilePtr->actualPath = tempFilePtr->actualPath;
	  currFilePtr->remotePath = remoteFilePath->text;
	  currFilePtr->minId = tempFilePtr->minId;
	  currFilePtr->size = tempFilePtr->size;
	  //printf("Inside getTransferFileList 0\n");
	  //update our list
	  if(!fileListHeadPtr)
	    fileListHeadPtr = fileListTailPtr = currFilePtr;
	  else{
	    fileListTailPtr->next = currFilePtr;
	    fileListTailPtr = currFilePtr;
	  }
	}
      }
      currArgPtr=currArgPtr->next;
    }
    //printf("Before returning from getTransferFileList 0\n");
    return fileListHeadPtr;
  }
  else{
    //printf("Inside getTransferFileList 1\n");
    FilePtr currFilePtr = 0;
    FilePtr fileListHeadPtr = 0;
    FilePtr fileListTailPtr = 0;
    CommandPtr tempCmdPtr = prevCmdPtr;
    while(currCmdPtr->next!=tempCmdPtr){
      currFilePtr = getTransferFileList(tempCmdPtr,0);
      printTransferList(currFilePtr);
      FilePtr tempFilePtr = currFilePtr;
      if(tempFilePtr){
	//printf("Inside getTransferFileList 1\n");
	//if the list head pointer is null
	if(!fileListHeadPtr){
	  fileListHeadPtr = fileListTailPtr = tempFilePtr;
	}
	else{	
	  //update the file list tail pointer
	  fileListTailPtr->next=tempFilePtr;
	  FilePtr interFilePtr = tempFilePtr;
	  //get to the end of this list
	  while(tempFilePtr){
	    interFilePtr = tempFilePtr;
	    tempFilePtr=tempFilePtr->next;
	  }
	  if(interFilePtr)	fileListTailPtr = interFilePtr;	
	}
      }
      tempCmdPtr=tempCmdPtr->next;
    }
    //printf("Before returning from getTransferFileList 1\n");
    //return the head pointer    
    return fileListHeadPtr;
  }
}

//this function recursively travels the pipeline tree
//and prepares another tree that contains the shell/ssh
//commands to be executed on different machines
//it does so by recursively going into the tree structure and recursing with the 
//current command ptr if the next command has the same execution host
//this is done only if it is successive
RemoteCmdPtr _makeRemoteCmd(CommandPtr currCmdPtr,CommandPtr prevCmdPtr,int recursive){  
  if(!currCmdPtr)
    return 0;
  //if we don't have more than one command to combine
  if(!prevCmdPtr){
    //printf("Inside _makeRemoteCmd 1\n");
    printf("Processing for command %s\n",currCmdPtr->name);
    //fflush(stdout);
    //if theres no need to combine commands then we need to generate the text part just for
    //this command and add it to the remoteCmdTailPtr or head Pointer  
    RemoteCmdPtr currRemoteCmdPtr = createRemoteCmd();
    currRemoteCmdPtr->host=currCmdPtr->execHost;
    //currRemoteCmdPtr->inputRedir=currCmdPtr->inputRedir;
    //currRemoteCmdPtr->outputRedir=currCmdPtr->outputRedir;
    currRemoteCmdPtr->currOutputRedir = currCmdPtr->currOutputRedir;
    //DONE get the execution host right for output redirection
    if(currCmdPtr->currOutputRedir){
      printf("The command is output redir so go for output\n");
      CommandPtr newCmdPtr = getRemoteCmdTextFromOutputCmd(currCmdPtr);
      if(!newCmdPtr)	currRemoteCmdPtr = 0;
      else{
	currRemoteCmdPtr->host = newCmdPtr->execHost;
	currRemoteCmdPtr->cmdText = createString();
	currRemoteCmdPtr->cmdText->text = strdup(newCmdPtr->name);
	currRemoteCmdPtr->cmdText->length = strlen(newCmdPtr->name);
	//printf("After processing the command name is %s\n",currRemoteCmdPtr->cmdText->text);
	freeCommandPtr(newCmdPtr);
      }
    }
    else      currRemoteCmdPtr->cmdText = getRemoteCmdTextFromCmd(currCmdPtr)->cmdText;
    
    //now get the list of files to be transferred for this group of command
    currRemoteCmdPtr->transferFileList = getTransferFileList(currCmdPtr,0/*prevCmdPtr*/);
    //printf("Cmd text at !prevCmdPtr is %s of length %d\n",currRemoteCmdPtr->cmdText->text,currRemoteCmdPtr->cmdText->length);
    //if this is not a recursive call then we insert the new command 
    //into the list
    if(!recursive){
      //printf("Not recursive\n");
      if(!remoteCmdHeadPtr)	remoteCmdTailPtr= remoteCmdHeadPtr = currRemoteCmdPtr;
      else{
	remoteCmdTailPtr->next = currRemoteCmdPtr;
	remoteCmdTailPtr=currRemoteCmdPtr;
      }
    }
    //printf("Before returning\n");
    return currRemoteCmdPtr;
  }
  //if we need to do a recursive call
  //if the prevCmdPtr is not null, then we need to iterate from there 
  //till we reach the currCmdPtr and create remote command that all run
  //on the same host
  //combine the commands
  else if(prevCmdPtr && currCmdPtr->execHost==prevCmdPtr->execHost){
    //printf("Inside _makeRemoteCmd 2\n");
    CommandPtr tempCmdPtr = prevCmdPtr;
    RemoteCmdPtr currRemoteCmdPtr = 0;
    RemoteCmdPtr interRemoteCmdPtr = 0;
    
    RemoteCmdTextPtr remoteCmdTextListHead=0;
    RemoteCmdTextPtr remoteCmdTextListTail=0;
    //until we reach the current command
    //we combine all the commands into a single command string
    //to be executed on a particular host
    int combinedStringLength=0;    
    //first we get the combined string length
    while(tempCmdPtr!=currCmdPtr){
      //we get the RemoteCmdText for each command and then later combine them into one      
      RemoteCmdTextPtr tempRemoteCmdTextPtr = 0;
      interRemoteCmdPtr = _makeRemoteCmd(tempCmdPtr,0,1);      
      //if the _makeRemoteCmd returned 0
      if(interRemoteCmdPtr){
	//printf("1 The command text obtained from _makeRemoteCmd is %s of length %d\n",interRemoteCmdPtr->cmdText->text,
	      //interRemoteCmdPtr->cmdText->length);
	fflush(stdout);
	tempRemoteCmdTextPtr = createRemoteCmdText();
	tempRemoteCmdTextPtr->cmdText = createString();
	tempRemoteCmdTextPtr->cmdText->text = strdup(interRemoteCmdPtr->cmdText->text);
	tempRemoteCmdTextPtr->cmdText->length = interRemoteCmdPtr->cmdText->length;	
	if(tempRemoteCmdTextPtr){
	  if(!remoteCmdTextListHead)
	    remoteCmdTextListHead=remoteCmdTextListTail=tempRemoteCmdTextPtr;
	  else{
	    remoteCmdTextListTail->next=tempRemoteCmdTextPtr;
	    remoteCmdTextListTail=tempRemoteCmdTextPtr;
	  }
	  combinedStringLength+=tempRemoteCmdTextPtr->cmdText->length+strlen(" | ");
	}
      }      
      tempCmdPtr=tempCmdPtr->next;
      //printf("On to the next command\n");
      //freeRemoteCmdPtr(interRemoteCmdPtr);
    }
    //now get the RemoteCmdText for the last command in this list, which is the
    //current command
    RemoteCmdTextPtr lastRemoteCmdTextPtr = 0;
    interRemoteCmdPtr = 0;
    interRemoteCmdPtr = _makeRemoteCmd(tempCmdPtr,0,1);    
    if(interRemoteCmdPtr){
      //printf("2 The command text obtained from _makeRemoteCmd is %s of length %d\n",interRemoteCmdPtr->cmdText->text,
	      //interRemoteCmdPtr->cmdText->length);
      lastRemoteCmdTextPtr = createRemoteCmdText();      
      lastRemoteCmdTextPtr->cmdText = createString();
      lastRemoteCmdTextPtr->cmdText->text = strdup(interRemoteCmdPtr->cmdText->text);
      lastRemoteCmdTextPtr->cmdText->length = interRemoteCmdPtr->cmdText->length;
      //freeRemoteCmdPtr(interRemoteCmdPtr);
    }
    else lastRemoteCmdTextPtr = 0;
        
    if(lastRemoteCmdTextPtr){
      currRemoteCmdPtr = createRemoteCmd();
      currRemoteCmdPtr->host = currCmdPtr->execHost;
      //currRemoteCmdPtr->inputRedir=tempCmdPtr->inputRedir;
      //currRemoteCmdPtr->outputRedir=tempCmdPtr->outputRedir;
      currRemoteCmdPtr->currOutputRedir=0;
      combinedStringLength+=lastRemoteCmdTextPtr->cmdText->length+strlen(" | ")+1;
    }    
    
    //update our RemoteCmdText list
    if(!remoteCmdTextListHead)
      remoteCmdTextListHead=remoteCmdTextListTail=lastRemoteCmdTextPtr;
    else{
      remoteCmdTextListTail->next=lastRemoteCmdTextPtr;    
      remoteCmdTextListTail=lastRemoteCmdTextPtr;
    }
    //before that print and see
    //printRemoteCmdTextList(remoteCmdTextListHead);
    currRemoteCmdPtr->cmdText=createString();
    //now set the list in currRemoteCmdPtr to this new
    currRemoteCmdPtr->cmdText->text =createCmdStringFromList(prevCmdPtr,currCmdPtr, remoteCmdTextListHead,combinedStringLength);
    currRemoteCmdPtr->cmdText->length = strlen(currRemoteCmdPtr->cmdText->text);
    
    //now get the list of files to be transferred for this group of command
    currRemoteCmdPtr->transferFileList = getTransferFileList(currCmdPtr,prevCmdPtr);       
    
    //if the head pointer of the remotecmd list is null,
    //we set the current remotecmd ptr as the head
    //and hence so goes the same for the tail
    if(!remoteCmdHeadPtr)	remoteCmdHeadPtr=remoteCmdTailPtr=currRemoteCmdPtr;
    //if not so, we update the tail pointer
    else{
      remoteCmdTailPtr->next=currRemoteCmdPtr;
      remoteCmdTailPtr=currRemoteCmdPtr;
    }
    //printf("Exiting here!\n");
    //fflush(stdout);
    return 0;
  }
}

void printTransferList(FilePtr head){
  FilePtr filePtr = head;
  while(filePtr){
    printf("\tRemote Destn.: %s ActualPath:%s Host:%d\n",filePtr->remotePath,filePtr->actualPath,filePtr->host);
    filePtr=filePtr->next;
  }
}

//print the command tree
void printRemoteCmdTree(){
  RemoteCmdPtr tempRemoteCmdPtr = remoteCmdHeadPtr;
  printf("--------------------------------------------------\n");
  while(tempRemoteCmdPtr){
    if(tempRemoteCmdPtr->currOutputRedir)
      printf(" > ");
    printf("%s",tempRemoteCmdPtr->cmdText->text);
    printTransferList(tempRemoteCmdPtr->transferFileList);
    if(tempRemoteCmdPtr->next && !(tempRemoteCmdPtr->next->currOutputRedir))
      printf(" | ");
    /*if(tempRemoteCmdPtr->next && (!(tempRemoteCmdPtr->inputRedir) && !(tempRemoteCmdPtr->outputRedir)))
      printf(" | ");    
    else if(tempRemoteCmdPtr->outputRedir && tempRemoteCmdPtr->next)
      printf(" > ");*/
    tempRemoteCmdPtr=tempRemoteCmdPtr->next;    
  }
  printf("\n");
  printf("--------------------------------------------------\n");
}

//the bootstrap function that calls _makeRemoteCmd
void makeRemoteCmd(){
  //printf("In makeRemoteCmd\n");
  //fflush(stdout);
  remoteCmdHeadPtr=remoteCmdTailPtr=0;
  CommandPtr temp1CmdPtr = cmdHeadPtr;
  while(temp1CmdPtr){
    CommandPtr temp2CmdPtr=temp1CmdPtr;
    CommandPtr temp2PrevPtr=0;
    //in this while loop, we combine all the successive commands in the pipeline
    //that are to be executed on the same host
    while(temp2CmdPtr){
      if(temp2CmdPtr->next){
	if(temp2CmdPtr->execHost == temp2CmdPtr->next->execHost /*&& temp2CmdPtr->execHost!=-1*/){
	  temp2PrevPtr=temp2CmdPtr;
	  temp2CmdPtr=temp2CmdPtr->next;
	}
	else
	  break;
      }
      else	break;
    }
    //if the next command is not to be executed in the same host as the current
    if(temp1CmdPtr==temp2CmdPtr)
      _makeRemoteCmd(temp1CmdPtr,0,0);
    //if we have more than one successive commands to be executed 
    else{
      //this means we have reahced the end of the entire pipeline
      //so we use the temp2PrevPtr to get to the last command
      if(!temp2CmdPtr){
	_makeRemoteCmd(temp2PrevPtr,temp1CmdPtr,0);
	temp1CmdPtr=temp2PrevPtr;
      }
      //this is the average/normal case
      else{
	_makeRemoteCmd(temp2CmdPtr,temp1CmdPtr,0);
	temp1CmdPtr=temp2CmdPtr;
      }
    }
    temp1CmdPtr=temp1CmdPtr->next;
  }
}

//this function frees the RemoteCmdPtr and all of 
//its fields
void freeRemoteCmdPtr(RemoteCmdPtr remoteCmdPtr){
  freeString(remoteCmdPtr->cmdText);
  FilePtr currFilePtr = remoteCmdPtr->transferFileList;
  while(currFilePtr){
    freeFilePtr(currFilePtr);
    currFilePtr = currFilePtr->next;
  }
  free(remoteCmdPtr);
}