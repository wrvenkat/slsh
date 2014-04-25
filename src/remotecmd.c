#include "remotecmd.h"
#include "globals.h"
#include "structure.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>
#include "structure.h"

#include "enums.h"

//this function returns an initalised REMOTECMD struct pointer
RemoteCmdPtr createRemoteCmd(){
  RemoteCmdPtr remoteCmdPtr = malloc(sizeof(REMOTECMD));
  remoteCmdPtr->host=-1;
  memset(remoteCmdPtr,0,sizeof(REMOTECMD));
  remoteCmdPtr->cmdText = createString();
  return remoteCmdPtr;
}

//this function returns an initalised REMOTECMDTEXT struct pointer
RemoteCmdTextPtr createRemoteCmdText(){
  RemoteCmdTextPtr remoteCmdTextPtr = malloc(sizeof(REMOTECMDTEXT));
  memset(remoteCmdTextPtr,0,sizeof(REMOTECMDTEXT));
  remoteCmdTextPtr->cmdText = createString();
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
  //printf("Argument at getFilePath is %s\n",arg->text);
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
  //printf("FileName in getFilePath is %s\n",fileName);
  char* filePathStr = 0;
  StringPtr filePath = createString();
  int length=0;
  if(execHost==fileHostId){
    char* temp = getRidOfEscapeChars(arg->filePtr->origPath);
    //printf("temp here is %s\n",temp);
    //printf("actualPath is %s\n",arg->filePtr->actualPath);
    //printf("mountDir is %s\n",mountDir);
    //if so we need to only write the filename with the mountdir
    if(strlen(getFileDir(temp))!=strlen(mountDir)){    
      length=strlen(homeDir)+(strlen(temp)-strlen(mountDir));
      filePathStr = malloc(sizeof(char)*(length+3));
      memset(filePathStr,0,length+3);
      sprintf(filePathStr,"\"%s%s\"",homeDir,temp+strlen(mountDir)+1);
    }
    else{      
      length=strlen(mountDir)+strlen(fileName);
      filePathStr = malloc(sizeof(char)*(length+3));
      memset(filePathStr,0,length+3);
      sprintf(filePathStr,"\"%s%s\"",mountDir,fileName);
    }
    filePath->text=filePathStr;
    filePath->length=length;
    //free(temp);
  }
  else{
    length=strlen(homeDir)+strlen(fileName)+strlen(TEMP_DIR);
    length+=3;
    filePathStr = malloc(sizeof(char)*(length));
    memset(filePathStr,0,length);
    sprintf(filePathStr,"\"%s%s%s\"",homeDir,TEMP_DIR,fileName);
    filePath->text=filePathStr;
    filePath->length=length-1;
    free(homeDir);    
  }
  //printf("String is %s\n",filePathStr);
  //if(DEBUG1) 
  //printf("Modified filepath at getFilePath is %s of length %d\n",filePath->text,filePath->length);
  return filePath;
}

//gets the text for the target file of an output redirection
CommandPtr getRemoteCmdTextFromOutputCmd(CommandPtr cmd){
  //if the current command was part of op redir
  //we need to take care of its file path
  //TODO THERE IS A PROBLEM WITH /dev/null or /dev/* devices
  //not returning their actual major and minor id
  if(cmd->currOutputRedir){
    if(isFilePath(cmd->name)){
      FilePtr currDirFile = getFileStruct(cmd->name);
      //if there is no such file
      if(!currDirFile){
	char* filePath = getRidOfEscapeChars(cmd->name);
	char* dir = getFileDir(cmd->name);
	//printf("Dir here is %s\n",dir);
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
	    //printf("After making the command name now is %s\n",newCmdPtr->name);	    
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

//this finction replaces the local mounted filepaths
//with the remote file paths
RemoteCmdTextPtr getRemoteCmdTextFromCmd(CommandPtr cmd){
  if(!cmd)
    return 0;
  //printf("In getRemoteCmdTextFromCmd\n");
  RemoteCmdTextPtr remoteCmdTextPtr = createRemoteCmdText();
  int totalLengthCombined =0;  
  {    
    ArgPtr argPtr = cmd->headArgs;
    StringPtr argListHeadPtr = 0;
    StringPtr argListTailPtr = 0;    
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
    //now copy the arguments
    StringPtr remoteCmdText = createString();
    char* cmdText = malloc(sizeof(char)*(totalLengthCombined+1));
    memset(cmdText,0,totalLengthCombined+1);
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
    //if(DEBUG1)	
    //printf("The command with modified filePath is %s\n",remoteCmdText->text);
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
  if(!tempCmdPtr || !endCmd || !tempRemoteCmdTextPtr)	return 0;
  char* combinedCmdString = malloc(sizeof(char)*(length+1));
  memset(combinedCmdString,0,length+1);
  char* tempStrPtr = combinedCmdString;  
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
	  currFilePtr->actualPath = strdup(tempFilePtr->actualPath);
	  currFilePtr->remotePath = strdup(remoteFilePath->text);
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
    //for each command get the fileTransfer list and
    //combine them into one
    while(currCmdPtr->next!=tempCmdPtr){
      currFilePtr = getTransferFileList(tempCmdPtr,0);
      //printf("Before printing the transferFileList\n");
      //printTransferList(currFilePtr);
      FilePtr tempFilePtr = currFilePtr;
      if(tempFilePtr){
	//printf("Inside getTransferFileList 1\n");
	//if the list head pointer is null
	if(!fileListHeadPtr)
	  fileListHeadPtr = fileListTailPtr = tempFilePtr;	
	else{	
	  //update the file list tail pointer
	  fileListTailPtr->next=tempFilePtr;
	  FilePtr interFilePtr = tempFilePtr;
	  //get to the end of the list returned
	  //by getTransferFileList(tempCmdPtr,0)
	  while(interFilePtr->next!=0)	    
	    interFilePtr=interFilePtr->next;
	  //if(interFilePtr)
	  fileListTailPtr = interFilePtr;	
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
  //RemoteCmdPtr remoteCmdHeadPtr;
  //RemoteCmdPtr remoteCmdTailPtr;
  //if we don't have more than one command to combine
  if(!prevCmdPtr){
    //printf("Inside _makeRemoteCmd 1\n");
    printf("_makeRemoteCmd1 for command %s\n",currCmdPtr->name);

    //if theres no need to combine commands then we need to generate the text part just for
    //this command and add it to the remoteCmdTailPtr or head Pointer  
    RemoteCmdPtr currRemoteCmdPtr = createRemoteCmd();
    currRemoteCmdPtr->host=currCmdPtr->execHost;    
    currRemoteCmdPtr->currOutputRedir = currCmdPtr->currOutputRedir;
    //DONE get the execution host right for output redirection
    if(currCmdPtr->currOutputRedir){
      //printf("The command is output redir so go for output\n");
      CommandPtr newCmdPtr = getRemoteCmdTextFromOutputCmd(currCmdPtr);
      //printf("Post return from getRemoteCmdTextFromOutputCmd\n");
      if(!newCmdPtr){
	freeRemoteCmdPtr(currRemoteCmdPtr);
	currRemoteCmdPtr = 0;
	fprintf(stderr,"%s is not a diretory or file!\n",currCmdPtr->name);
	return currRemoteCmdPtr;
      }
      else{
	currRemoteCmdPtr->host = newCmdPtr->execHost;
	currRemoteCmdPtr->cmdText->text = strdup(newCmdPtr->name);
	currRemoteCmdPtr->cmdText->length = strlen(newCmdPtr->name);
	//printf("getRemoteCmdTextFromOutputCmd: After processing the command name is %s\n",currRemoteCmdPtr->cmdText->text);
	//we are only interested in the text part
	//so once copied we free it
	freeCommandPtr(newCmdPtr);
      }
    }
    else
      currRemoteCmdPtr->cmdText = getRemoteCmdTextFromCmd(currCmdPtr)->cmdText;
        
    if(currRemoteCmdPtr){
      //now get the list of files to be transferred for this group of command
      currRemoteCmdPtr->transferFileList = getTransferFileList(currCmdPtr,0);
      //printf("Cmd text at !prevCmdPtr is %s of length %d\n",currRemoteCmdPtr->cmdText->text,currRemoteCmdPtr->cmdText->length);
    }
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
    RemoteCmdTextPtr tempRemoteCmdTextPtr = 0;
    
    RemoteCmdTextPtr remoteCmdTextListHead=0;
    RemoteCmdTextPtr remoteCmdTextListTail=0;
    //until we reach the current command
    //we combine all the commands into a single command string
    //to be executed on a particular host
    int combinedStringLength=0;    
    //first we get the combined string length
    while(tempCmdPtr &&  currCmdPtr->next!=tempCmdPtr){
      //we get the RemoteCmdText for each command and then later combine them into one      
      interRemoteCmdPtr = _makeRemoteCmd(tempCmdPtr,0,1);
      //if the _makeRemoteCmd did not return 0
      if(interRemoteCmdPtr){
	//1. this gimmick is for the weird error that you get with unwanted characters like ! and q
	char duplicate[(interRemoteCmdPtr->cmdText->length)+1];
	memset(duplicate,0,(interRemoteCmdPtr->cmdText->length)+1);
	sprintf(duplicate,"%s",interRemoteCmdPtr->cmdText->text);	
	tempRemoteCmdTextPtr = createRemoteCmdText();
	//2. this gimmick is for the weird error that you get with unwanted characters like ! and q
	tempRemoteCmdTextPtr->cmdText->text = strdup(duplicate);
	tempRemoteCmdTextPtr->cmdText->length = strlen(duplicate);
	//we only need the text of the RemoteCmdPtr
	//so we once we've copied the text we free it
	freeRemoteCmdPtr(interRemoteCmdPtr);
	interRemoteCmdPtr=0;
	//add the new tempRemoteCmdTextPtr to our list
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
    }    
    //we need a currRemoteCmdPtr    
    currRemoteCmdPtr = createRemoteCmd();
    //currRemoteCmdPtr->cmdText=createString();
    currRemoteCmdPtr->host = currCmdPtr->execHost;    
    currRemoteCmdPtr->currOutputRedir=0;
    
    //before that print and see
    //printRemoteCmdTextList(remoteCmdTextListHead);
    //now set the list in currRemoteCmdPtr to this new
    currRemoteCmdPtr->cmdText->text =createCmdStringFromList(prevCmdPtr,currCmdPtr, remoteCmdTextListHead,combinedStringLength);
    currRemoteCmdPtr->cmdText->length = strlen(currRemoteCmdPtr->cmdText->text);
    
    //now get the list of files to be transferred for this group of command
    currRemoteCmdPtr->transferFileList = getTransferFileList(currCmdPtr,prevCmdPtr);       
       
    return currRemoteCmdPtr;
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
void printRemoteCmdTree(RemoteCmdPtr remoteCmdHeadPtr){
  RemoteCmdPtr tempRemoteCmdPtr = remoteCmdHeadPtr;
  //printf("--------------------------------------------------\n");
  while(tempRemoteCmdPtr){
    if(tempRemoteCmdPtr->currOutputRedir)
      printf(" > ");
    printf("%s",tempRemoteCmdPtr->cmdText->text);
    printf("\n");
    printTransferList(tempRemoteCmdPtr->transferFileList);
    if(tempRemoteCmdPtr->next && !(tempRemoteCmdPtr->next->currOutputRedir))
      printf(" | ");
    /*if(tempRemoteCmdPtr->next && (!(tempRemoteCmdPtr->inputRedir) && !(tempRemoteCmdPtr->outputRedir)))
      printf(" | ");    
    else if(tempRemoteCmdPtr->outputRedir && tempRemoteCmdPtr->next)
      printf(" > ");*/
    tempRemoteCmdPtr=tempRemoteCmdPtr->next;    
  }
  //printf("\n");
  //printf("--------------------------------------------------\n");
}

//the bootstrap function that calls _makeRemoteCmd
RemoteCmdPtr makeRemoteCmd(CommandPtr cmdHeadPtr){
  //printf("In makeRemoteCmd\n");
  //fflush(stdout);
  RemoteCmdPtr remoteCmdHeadPtr;
  RemoteCmdPtr remoteCmdTailPtr;
  RemoteCmdPtr currRemoteCmdPtr;
  remoteCmdHeadPtr=remoteCmdTailPtr=currRemoteCmdPtr=0;
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
    if(temp1CmdPtr==temp2CmdPtr){
      currRemoteCmdPtr=_makeRemoteCmd(temp1CmdPtr,0,0);
      //printf("Post Return from _makeRemoteCmd(temp1CmdPtr,0,0)\n");
    }
    //if we have more than one successive commands to be executed 
    else{
      //this means we have reahced the end of the entire pipeline
      //so we use the temp2PrevPtr to get to the last command
      if(!temp2CmdPtr){
	currRemoteCmdPtr=_makeRemoteCmd(temp2PrevPtr,temp1CmdPtr,0);
	temp1CmdPtr=temp2PrevPtr;
	//printf("Post return from _makeRemoteCmd(temp2PrevPtr,temp1CmdPtr,0)\n");
      }
      //this is the average/normal case
      else{
	currRemoteCmdPtr=_makeRemoteCmd(temp2CmdPtr,temp1CmdPtr,0);
	temp1CmdPtr=temp2CmdPtr;
	//printf("Post return from _makeRemoteCmd(temp2CmdPtr,temp1CmdPtr,0)\n");
      }
    }
    //add it to the list
    if(currRemoteCmdPtr){
      if(!remoteCmdHeadPtr)
	remoteCmdHeadPtr=remoteCmdTailPtr=currRemoteCmdPtr;
      else{
	//we've to get to the end of this RemoteCmdPtr list
	RemoteCmdPtr tempRemoteCmdPtr = currRemoteCmdPtr;
	//while(tempRemoteCmdPtr->next!=0)
	  //tempRemoteCmdPtr=tempRemoteCmdPtr->next;
	remoteCmdTailPtr->next=tempRemoteCmdPtr;
	remoteCmdTailPtr=tempRemoteCmdPtr;
      }
      currRemoteCmdPtr=0;
    }        
    temp1CmdPtr=temp1CmdPtr->next;    
  }
  //printf("Returning from makeRemoteCmd\n");
  return remoteCmdHeadPtr;
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