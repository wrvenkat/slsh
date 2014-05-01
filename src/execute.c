#include <stdlib.h>
#include "remotecmd.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include "globals.h"
#include "execute.h"
#include <unistd.h>
#include "structure.h"
#include "util.h"

//this function makes the persistent SSH connections
//and returns the String linked list which has all the connection
//string
int makePersistentSSH(){
  if(DBG_FUNC_ENTRY)
    printf("Inside makePersistentSSH\n");
  char* persistPath = 0;
  int i=1;
  //int retValue=0;
  int length1 = strlen("ssh ")+strlen(PERSIST_SSH)+1+strlen(P_PATH_STR)+10;;
  for(i=1;i<maxHost;i++){
    //if the host is involved and there is not active connections
    if(hostInvolved[i] && !(hostMap[i]->active)){
      //printf("Host Involved value for %d is %d\n",i,hostInvolved[i]);
      StringPtr sshString = getSSHString(i);
      length1+=sshString->length;
      //get the length
      srand(time(0));    
      persistPath = malloc(sizeof(char)*(strlen(P_PATH_STR)+10));
      memset(persistPath,0,strlen(P_PATH_STR)+10+1);
      double f= rand() / (double)RAND_MAX;
      sprintf(persistPath,"%s%f",P_PATH_STR,f);
      //printf("Persist path is %s\n",persistPath);
      char cmd[length1+1+strlen("'exit'")];
      sprintf(cmd,"ssh %s %s %s%f 'exit'",sshString->text,PERSIST_SSH,P_PATH_STR,f);
      if(DBG_GEN)	
	printf("The ssh persist string for host %d is %s\n",i,cmd);
      if(FLG_EXEC_DO){
	/*pid_t status =*/ system(cmd);
	//retValue = WEXITSTATUS(status);
      }
      /*if(retValue>0)*/{
	hostMap[i]->active=1;
	hostMap[i]->persistPath=persistPath;
      }
    }
  }
  return 1;
}  
  
//exits the persistent connection with the persist path in
//the 2d array persistpath
int exitPersistentSSH(){  
  if(DBG_FUNC_ENTRY)
    printf("Inside exitPersistentSSH\n");
  int i=0;
  int retValue=0;
  int length1=0;
  char* persistPath=0;  
  //here the condition means that if the host is still involved don't disconnect
  //the persistent connections
  for(i=1;i<maxHost /*&& hostMap[i]->active*/;i++){    
    if(!hostInvolved[i] && hostMap[i]->active){      
      persistPath=hostMap[i]->persistPath;
      if(!persistPath)	continue;
      StringPtr sshString = getSSHString(i);    
      length1 = strlen(PERSIST_EXIT)+1+strlen(persistPath)+sshString->length+strlen(REDIR_NULL)+1;
      char cmd[length1+1];
      memset(cmd,0,length1+1);
      sprintf(cmd,"%s %s %s %s",PERSIST_EXIT,persistPath,sshString->text,REDIR_NULL);
      if(DBG_GEN)
	printf("SSH exit string is %s\n",cmd);
      if(FLG_EXEC_DO){
	pid_t status = system(cmd);
	retValue = WEXITSTATUS(status);
	hostMap[i]->active=0;
      }      
    }
  }  
  return retValue;
}

//the bootstrap function that starts the execution of the prepared commands
int executePlan(RemoteCmdPtr remoteCmdHeadPtr){
  if(!remoteCmdHeadPtr)	return 1;
  if(DBG_FUNC_ENTRY)
    printf("Inside executePlan\n");
  int retValue = 0;  
  retValue = executeCommand(remoteCmdHeadPtr,0,-2);  
  return retValue;
}

//the main function which travels the remotCommandTree recursively and executes them
int executeCommand(RemoteCmdPtr remoteCmdPtr,char* ipFile, int prevExecHost){  
  if(!remoteCmdPtr)	return 0;
  if(DBG_FUNC_ENTRY)
    printf("Execution at host:%d for command %s\n",remoteCmdPtr->host,remoteCmdPtr->cmdText->text);  
  //if the prevExecHost is not -2 and if ipFileis not empty, then we need
  //to transfer the ipFile from prevExecHost to current host
  if(prevExecHost!=-2 && ipFile){
    if(transferInputFile(ipFile,prevExecHost,remoteCmdPtr->host)<0){
      fprintf(stderr,"Error transferring temporary file from host: %d to host: %d\n",prevExecHost,remoteCmdPtr->host);
      return -1;
    }
  }  
  char* opFile=0;
  //this is the health flag that signals if
  //everythings fine
  int health = 1;
  int execHost = remoteCmdPtr->host;
  //for now if the execHost is -1, we set it to be 0, the host machine
  if(execHost<0) execHost=0;
  //StringPtr cmdText = remoteCmdPtr->cmdText;
  FilePtr transferList = remoteCmdPtr->transferFileList;
    
  //first we always start witht he set of files to be transferred
  //to the target machine where the command is to be executed
  FilePtr currFile = transferList;
  while(currFile && health){
    if(transferFile(currFile, execHost)<0){
      health=0;
      break;
    }
    else	health=1;
    currFile=currFile->next;
  }
  //opFile is the file to which the o/p is saved to
  //we prepare the opFile name for this command
  if(remoteCmdPtr->next){
    opFile = malloc(sizeof(char)*(TEMP_FILE_LENGTH+1));
    memset(opFile,0,TEMP_FILE_LENGTH+1);
    srand(time(0));
    sprintf(opFile,"%s%d%f","command",gIndex++,rand() / (double)RAND_MAX);
  }
  
  //once all the files have been transferred
  //we execute the command
  StringPtr sshConnString = prepareCommand(remoteCmdPtr,ipFile,opFile,execHost);
  if(DBG_EXEC)
    printf("executeCommand: Execution command: %s\n",sshConnString->text);
  int retValue = 0;
  if(sshConnString){
    if(FLG_EXEC_DO){
      pid_t status = system(sshConnString->text);
      retValue = WEXITSTATUS(status);
    }
  }
  if(retValue!=0){      
    fprintf(stderr,"Command %s terminated with return value %d\n",remoteCmdPtr->cmdText->text,retValue);   
    return retValue;
  }
  if(DBG_GEN)
    printf("--------------------------------------------------------------------\n");
    
  RemoteCmdPtr nextRemoteCmdPtr = remoteCmdPtr->next;
  
  int val = 0;
  //we need to follow different strategies for different kinds of
  //upcoming commands  
  if(nextRemoteCmdPtr && nextRemoteCmdPtr->currOutputRedir)
    val = executeOutputRedirCmd(nextRemoteCmdPtr,opFile,execHost);  
  //the most ususal one is the pipe
  else
    val =  executeCommand(nextRemoteCmdPtr,opFile,execHost);
  freeString(sshConnString);
  free(opFile);
  return val;
}

//this function executes the output redirection part of the pipeline
int executeOutputRedirCmd(RemoteCmdPtr remoteCmdPtr,char* ipFile, int prevExecHost){
  //TODO implement the logic for file transfer absed on the file present and the execution host
  if(!remoteCmdPtr)	return 0;
  if(DBG_FUNC_ENTRY)
    printf("Outputredir execution at host:%d\n",remoteCmdPtr->host);
  //if the prevExecHost is not -2 and if ipFileis not empty, then we need
  //to transfer the ipFile from prevExecHost to current host
  if(prevExecHost!=-2 && ipFile){
    if(transferInputFile(ipFile,prevExecHost,remoteCmdPtr->host)<0){
      fprintf(stderr,"ERROR:File required to transfer input from\n");
      return -1;
    }
  }
  int retValue=0;
  char* command =0;
  //the ipFile is already transferred all we need to do is
  //to cat it to the target file
  //now to do that we need to ssh to it or not ssh
  
  //if the target is not local host
  if(remoteCmdPtr->host!=0){
    StringPtr sshString = getSSHString(remoteCmdPtr->host);
    char* currentHomeDir = getHomeDir(hostMap[remoteCmdPtr->host]->mnt_fsname);
    int length1 = strlen("cat ")+2+strlen(currentHomeDir)+strlen(TEMP_DIR)+strlen(ipFile)
	      + strlen(" > ")+remoteCmdPtr->cmdText->length;
    int length2 = strlen("ssh ''")+2+length1+sshString->length;
    command = malloc(sizeof(char)*(length1+length2+1));
    memset(command,0,length1+length2+1);
    sprintf(command,"ssh %s 'cat \"%s%s%s\" > \"%s\"'",sshString->text,currentHomeDir,TEMP_DIR,ipFile,remoteCmdPtr->cmdText->text);
    freeString(sshString);    
  }
  //if its local
  else{
    char* currentHomeDir = getHomeDir(hostMap[0]->mnt_fsname);
    int length = strlen("cat ")+2+strlen(currentHomeDir)+strlen(TEMP_DIR)+strlen(ipFile)
	      + strlen(" > ")+strlen(currWD)+1+remoteCmdPtr->cmdText->length+2;
    command = malloc(sizeof(char)*(length+1));
    memset(command,0,length+1);
    sprintf(command,"cat \"%s%s%s\" > \"%s\"",currentHomeDir,TEMP_DIR,ipFile,remoteCmdPtr->cmdText->text);    
  }  
  if(DBG_EXEC)
    printf("executeOutputRedirCmd: Execution command is %s\n",command);
  if(FLG_EXEC_DO){
    pid_t status = system(command);
    retValue = WEXITSTATUS(status);
  }
  free(command);
  return retValue;  
}

//this function transfers the fileName which is at prevHost
//to currHost
int transferInputFile(char* fileName, int prevHost, int currHost){
  if(!fileName) return 0;
  if(currHost<0)	currHost=0;
  if(prevHost==currHost)	return 1;  
  int retValue = 0;
  char* prevHomeDir = getHomeDir(hostMap[prevHost]->mnt_fsname);
  char* currHomeDir = getHomeDir(hostMap[currHost]->mnt_fsname);
  int length1 = 0;
  int length2 = 0;
  int length3 = strlen(TEMP_DIR)+strlen(fileName);  
  StringPtr prevSSHString = 0;
  StringPtr currSSHString = 0;
  if(DBG_GEN)
    printf("TransferInputFile from prevHost: %d to currHost: %d and fileName:%s\n",prevHost,currHost,fileName);
  if(prevHost!=0){
    prevSSHString =getSSHString(prevHost);
    length1 = prevSSHString->length+1+strlen(prevHomeDir)+length3+4;
  }
  else
    length1 = strlen(prevHomeDir)+length3+2;
  if(currHost!=0){
    currSSHString = getSSHString(currHost);
    length2 = currSSHString->length+1+strlen(currHomeDir)+length3+4;
  }
  else
    length2 = strlen(currHomeDir)+length3+2;
  //int nullRLen = strlen(REDIR_NULL)+1;
  char cmd1[length1+1];
  char cmd2[length2+1];
  char command[length1+length2+strlen("scp ")+1/*+nullRLen*/+1];  
  memset(command,0,length1+length2+strlen("scp ")+1/*+nullRLen*/+1);
  memset(cmd1,0,length1+1);
  memset(cmd2,0,length2/*+nullRLen*/+1);
  char* cmdPtr = command;
  if(prevHost!=0){
    sprintf(cmdPtr,"scp '%s:\"%s%s%s\"'",prevSSHString->text,prevHomeDir,TEMP_DIR,fileName);    
    cmdPtr+=strlen("scp ")+length1;
  }
  else{
    sprintf(cmdPtr,"scp \"%s%s%s\"",prevHomeDir,TEMP_DIR,fileName);
    cmdPtr+=strlen("scp ")+length1;
  }
  if(currHost!=0){
    sprintf(cmdPtr," '%s:\"%s%s%s\"'",currSSHString->text,currHomeDir,TEMP_DIR,fileName);
    cmdPtr+=strlen(" ")+length2;
  }
  else{
    //sprintf(cmdPtr," \"%s%s%s\" %s",currHomeDir,TEMP_DIR,fileName,REDIR_NULL);
    sprintf(cmdPtr," \"%s%s%s\"",currHomeDir,TEMP_DIR,fileName);
    cmdPtr+=strlen(" ")+length2;
  }
  if(DBG_EXEC)
    printf("transferInputFile: Input Transfer Command: %s\n",command);
  retValue=0;
  if(FLG_EXEC_DO){
    pid_t status = system(command);
    retValue = WEXITSTATUS(status);
  }
  return WEXITSTATUS(retValue);
}

//this function transfers the given file to the
//execHost
int transferFile(FilePtr currFile, int execHost){
  if(DBG_FUNC_ENTRY)
    printf("Inside transferFile with actual path %s\n",currFile->actualPath);
  if(!currFile || execHost==-1) return -1;  
  char* toHomeDir = 0;
  char* fromHomeDir = 0;
  char* fromHomeRelPath =0;
  StringPtr fromSSHString = 0;
  StringPtr toSSHString = 0;
  char* fileName = 0;
  int length1=0;
  int length2=0;
  int length = 0;
  fileName = getFileName(currFile->origPath);  
  //if the file to be transferred is on a
  //remote host
  if(currFile->host!=0){
    char* filePath = strdup(currFile->actualPath);
    fromHomeRelPath = strdup(filePath+strlen(hostMap[currFile->host]->mnt_dir)+1);
    free(filePath);
    fromSSHString = getSSHString(currFile->host);
    fromHomeDir = getHomeDir(hostMap[currFile->host]->mnt_fsname);
    length1 = fromSSHString->length+strlen(fromHomeDir)+strlen(fromHomeRelPath)+1+4;
  }
  //if the file is to be transferred to a
  //remote host
  if(execHost!=0){
    toSSHString = getSSHString(execHost);
    toHomeDir = getHomeDir(hostMap[execHost]->mnt_fsname);
    length2 = toSSHString->length+strlen(toHomeDir)+strlen(TEMP_DIR)+strlen(fileName)+1+4;
  }
  else if(execHost==0){
    toHomeDir = getHomeDir(hostMap[execHost]->mnt_fsname);
    length2 = strlen(toHomeDir)+strlen(TEMP_DIR)+strlen(fileName)+1+4;
  }
  char fromRemoteFileString[length1+1];  
  char toRemoteFileString[length2+1];
  memset(fromRemoteFileString,0,length1+1);
  memset(toRemoteFileString,0,length2+1);
  if(currFile->host!=0)
    sprintf(fromRemoteFileString,"'%s:\"%s%s\"'",fromSSHString->text,fromHomeDir,fromHomeRelPath);
  else
    sprintf(fromRemoteFileString,"\"%s\"",currFile->actualPath);
  if(execHost!=0)
    sprintf(toRemoteFileString,"'%s:\"%s%s%s\"'",toSSHString->text,toHomeDir,TEMP_DIR,fileName);
  //here for now the TEMP_DIR is created under the CWD
  else
    sprintf(toRemoteFileString,"\"%s%s%s\"",toHomeDir,TEMP_DIR,fileName);
  
  length = strlen("scp ")+length1+length2+1;
  char* command = malloc(sizeof(char)*(length+1));
  memset(command,0,length+1);
  sprintf(command,"scp %s %s",fromRemoteFileString,toRemoteFileString);
  if(DBG_EXEC)
    printf("transferFile: %s\n",command);
  int retValue =0;
  if(FLG_EXEC_DO){
    int status = system(command);
    retValue = WEXITSTATUS(status);
  }
  free(command);
  return retValue;
}

//this function prepapres the string to be executed 
//based on the target machine
StringPtr prepareCommand(RemoteCmdPtr cmd, char* ipFile, char* opFile, int execHost){  
  if(!cmd) return 0;
  if(DBG_FUNC_ENTRY)
    printf("Inside prepareCommand with execHost %d\n",execHost);
  StringPtr cmdText = cmd->cmdText;
  StringPtr sshString = 0;
  char* command=0;
  int length=0;
  char* execHomeDir = 0;
  int opPathLength = 0;  
  if(execHost!=0){
    sshString = getSSHString(execHost);
    if(!sshString)	return 0;
    execHomeDir = getHomeDir(hostMap[execHost]->mnt_fsname);    
  }
  else{
    sshString= createString();
    execHomeDir = strdup(hostMap[execHost]->mnt_fsname);
  }
  //if we need to save it to an intermediate file
  //if not, it usually means that we are the end of the
  //pipeline and the output from the last command outputs
  //to the host machine where it is executed
  if(opFile)
    opPathLength = strlen(execHomeDir)+strlen(TEMP_DIR)+strlen(opFile);
  else
    opPathLength = strlen(execHomeDir)+strlen(TEMP_DIR);
  if(!ipFile){
    if(execHost!=0)
      //the 3 here is for the two '' and one for the single space
      //the two is here for the two \"
      length=strlen("ssh ")+sshString->length+cmdText->length+strlen(" > ")+opPathLength+3+2;
    else
      length = cmdText->length+strlen(" > ")+opPathLength+1+2;    
    command = malloc(sizeof(char)*(length+1));
    memset(command,0,length+1);
    if(execHost!=0){
      if(opFile)
	sprintf(command,"ssh %s '%s > \"%s%s%s\"'",sshString->text,cmdText->text,execHomeDir,TEMP_DIR,opFile);
      else
	sprintf(command,"ssh %s '%s'",sshString->text,cmdText->text);
    }
    else{
      if(opFile)
	sprintf(command,"%s > \"%s%s%s\"",cmdText->text,execHomeDir,TEMP_DIR,opFile);
      else
	sprintf(command,"%s",cmdText->text);
    }
  }
  else{
    char* remoteHomeDir = 0;
    int length=0;
    if(execHost!=0){
      remoteHomeDir = getHomeDir(hostMap[execHost]->mnt_fsname);    
      length=strlen("ssh ")+sshString->length+2+strlen("cat ")+strlen(remoteHomeDir)
	      +strlen(TEMP_DIR)+strlen(ipFile)+strlen(" | ")+cmdText->length+strlen(" > ")+opPathLength+1+2+2;
    }
    else{
      length = strlen("cat ")+strlen(execHomeDir)+strlen(TEMP_DIR)+strlen(ipFile)+strlen(" | ")
	      +cmdText->length+strlen(" > ")+opPathLength+2+2;
    }
    command = malloc(sizeof(char)*(length+1));
    memset(command,0,length+1);
    if(execHost!=0){
      if(opFile)
	sprintf(command,"ssh %s 'cat \"%s%s%s\" | %s > \"%s%s%s\"'"
	      ,sshString->text,remoteHomeDir,TEMP_DIR,ipFile,cmdText->text,execHomeDir,TEMP_DIR,opFile);
      else
	sprintf(command,"ssh %s 'cat \"%s%s%s\" | %s'"
	      ,sshString->text,remoteHomeDir,TEMP_DIR,ipFile,cmdText->text);
    }
    else{
      if(opFile)
	sprintf(command,"cat \"%s%s%s\" | %s > \"%s%s%s\""
	      ,execHomeDir,TEMP_DIR,ipFile,cmdText->text,execHomeDir,TEMP_DIR,opFile);
      else
	sprintf(command,"cat \"%s%s%s\" | %s"
	      ,execHomeDir,TEMP_DIR,ipFile,cmdText->text);
    }
  }
  //if(DBG_GEN)
    printf("Prepared command is %s\n",command);
  sshString->text = command;
  sshString->length = length;
  return sshString;
}

//TODO make it use the persistent connections
//make temporary directories
void makeTempDir(){
  int length = 0;
  int i=0;
  StringPtr sshString = 0;
  //make the temporary directories for the active connections whose temp directories haven't been made yet
  for(i=0;i<maxHost;i++){
    if(hostMap[i]->active && !hostMap[i]->madeTempDir){
      char* homeDir = getHomeDir(hostMap[i]->mnt_fsname);
      if(i==0){
	length = strlen("mkdir ")+1+strlen(homeDir)+strlen(TEMP_DIR)+1+1+strlen(REDIR_NULL);
	char mkDirCmd[length+1];
	memset(mkDirCmd,0,length+1);
	sprintf(mkDirCmd,"mkdir \"%s%s\" %s",homeDir,TEMP_DIR,REDIR_NULL);
	if(DBG_GEN)
	  printf("1 The mkDirCmd is %s\n",mkDirCmd);
	//printf("1 Length of mkDirCmd is %d and %d\n",length,(int)strlen(mkDirCmd));
	if(FLG_EXEC_DO)
	  system(mkDirCmd);
	hostMap[i]->madeTempDir=1;
      }
      else{
	sshString = getSSHString(i);     
	length = strlen("ssh ")+sshString->length+strlen(" 'mkdir ")+1+strlen(homeDir)+strlen(TEMP_DIR)+1+1+strlen(REDIR_NULL)+1;      
	char mkDirCmd[length+1];
	memset(mkDirCmd,0,length+1);
	sprintf(mkDirCmd,"ssh %s 'mkdir \"%s%s\" %s'",sshString->text,homeDir,TEMP_DIR, REDIR_NULL);
	if(DBG_GEN)
	  printf("2 The mkDirCmd is %s\n",mkDirCmd);
	//printf("Length of mkDirCmd is %d and %d\n",length,(int)strlen(mkDirCmd));
	if(FLG_EXEC_DO)
	  system(mkDirCmd);
	hostMap[i]->madeTempDir=1;
      }    
      freeString(sshString);
    }
  }  
}