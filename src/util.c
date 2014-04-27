#include "util.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mntent.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "globals.h"
#include "enums.h"
#include "structure.h"

//intialize global variables and flags
int initGlobals(){
  memset(hostMap,0,MAX_HOST);
  aggFlag=0;
  sepFlag=0;
  sameFileFlag1=0;
  maxHost=0;
  currWD = (char*)malloc(sizeof(char)*MAX_DIR_LENGTH);
  getcwd(currWD,MAX_DIR_LENGTH);
  createHostMap();
  if(maxHost<=1)
    return -1;
  compRegex();
  return 0;
}

// this function gets rid of the sequence(s) of "\ "
// in the filePath and returns a string which doesn't contain those
char* getRidOfEscapeChars(char* filePath){  
  char* finalStr = malloc(sizeof(char)*(strlen(filePath)+1));
  memset(finalStr,0,(strlen(filePath)+1));
  sprintf(finalStr,"%s",filePath);
  char* strPtr = filePath;
  char* strCopyPtr = finalStr;
  //char* startPtr = filePath;
  int i=0;
  for(i=0;i<strlen(filePath);){    
    if(strPtr[i]=='\\'){      
      if(strPtr[i+1]=='\\' || strPtr[i+1]==' ' || strPtr[i+1]=='\t'){	
	char currChar = strPtr[i+1];
	switch (currChar){
	  case '\\':
	    *strCopyPtr='\\';
	    break;
	  case ' ':
	    *strCopyPtr=' ';
	    break;
	  case '\t':
	    *strCopyPtr='\t';
	    break;
	  default:
	    break;
	}
	strCopyPtr++;	
	i=i+2;
      }
      else{
	*strCopyPtr=strPtr[i];
	strCopyPtr++;
	i++;
      }
    }
    else{
      *strCopyPtr=strPtr[i];
      strCopyPtr++;
      i++;
    }
  }
  *strCopyPtr=0;
  return finalStr;
}

//this function returns a FILE_ structure for a given valid fileStr
FilePtr getFileStruct(char* fileStr){
  //printf("FileStr in getFileStruct is %s\n",fileStr);
  struct stat* newStat = malloc(sizeof(struct stat));
  char* actualPath = getRidOfEscapeChars(fileStr);
  //printf("After getRidOfEscapeChars is %s\n",actualPath);
  if(!stat(actualPath, newStat)){
    FilePtr newFile = createFile(0,-1);
    dev_t m1=newStat->st_dev;
    int minId=minor(m1);
    int majId=major(m1);
    //printf("Major Id is %d\n",majId);
    //TODO there is a problem with getting the major and
    //minor device no of /dev/null
    newFile->minId=minId;
    int host = -1;
    //if major ID !=0 meands it is a
    //local drive
    if(majId!=0)
      host = 0;
    //majId is 0 means it is a network drive
    else
      host= getHostId(minId);
    newFile->host=host;
    newFile->size=(int)newStat->st_size;
    newFile->actualPath=strdup(actualPath);
    newFile->origPath=strdup(fileStr);
    //if(DEBUG2)
      printf("Size: %d bytes Host: %d ActPath: %s origPath: %s MinId: %d and MajId: %d\n",newFile->size,
	     newFile->host,newFile->actualPath,newFile->origPath,newFile->minId,majId);
    free(actualPath);
    return newFile;
  }
  else{    
    //printf("Returning 0 from getFileStruct with errorno %d\n",errno);
    return 0;
  }
}

//return the host index associated with the minId
int getHostId(int minId){
  int i=0;  
  while(hostMap[i] && hostMap[i]->minId!=minId)
    i++;
  if(i<=maxHost)
    return i;  
  //signal error
  return -1;
}

//this function populates the global
//hostMap array
void createHostMap(){
  memset(&hostMap,0,MAX_HOST);
  struct mntent* mntEnt=0;  
  FILE *fp = setmntent("/etc/mtab", "r");
  if(!fp)
    fprintf(stdout,"ERROR: Unable to open file for reading\n");
  else{
    //special values for host 0 - local machine
    hostMap[0]=malloc(sizeof(HOSTINFO));
    struct stat tempStat;
    stat(currWD, &tempStat);	
    dev_t m11=tempStat.st_dev;        
    hostMap[0]->minId=minor(m11);
    char *tempCurrWD = malloc(sizeof(char)*(strlen(currWD)+2));
    memset(tempCurrWD,0,strlen(currWD)+2);
    sprintf(tempCurrWD,"%s/",currWD);
    hostMap[0]->mnt_dir=tempCurrWD;
    hostMap[0]->mnt_fsname=strdup(tempCurrWD);
    int i=1;
    while((mntEnt = getmntent(fp))){
      if(!strcmp(mntEnt->mnt_type,"fuse.sshfs")){
	struct stat statStruct;
	if(!stat(mntEnt->mnt_dir, &statStruct)){
	  dev_t m1=statStruct.st_dev;
	  int minId=minor(m1);
	  hostMap[i]=malloc(sizeof(HOSTINFO));
	  memset(hostMap[i],0,sizeof(HOSTINFO));
	  hostMap[i]->minId=minId;	  
	  hostMap[i]->mnt_dir=strdup(mntEnt->mnt_dir);
	  hostMap[i]->mnt_fsname=strdup(mntEnt->mnt_fsname);	  
	  i++;
	}	
      }
    }
    maxHost=i;
    //printf("MaxHost value is %d\n",maxHost);
  }
}

//returns true if a string is a valid file path or not
int isFilePath(char* fakeFile){  
  regmatch_t matchedArray[6];
  if(regexec(&fileReg,fakeFile,(size_t)0,matchedArray,0)==REG_NOMATCH)    
    return 0;  
  return 1;
}

//compiles the filepath regex
void compRegex(){
  //this is the regular expression for a path that appears as a word or inside a QARG
  //this is different from the other lexer.l's in that the other regex has to parse the 
  //sequence "\[\t\r\v]" where as here "\ " is actually "\\ " in the other regex
  if(regcomp(&fileReg,FILE_REGEX,0 | REG_ICASE | REG_EXTENDED | REG_NOSUB)!=0){
    if(DEBUG1)
      fprintf(stdout,"FilePath regex compilation failed");
  }
}

//this function returns the home directory from the the remote
//fs_name
char* getHomeDir(char* fsName){
  if(!fsName) return 0;  
  char* homeDirStart = 0;  
  homeDirStart = strchr(fsName,'/');
  if(homeDirStart)
     return homeDirStart;  
  return 0;
}

//returns the filename given the filePath
char* getFileName(char* filePath){
  //printf("Given path is %s\n",filePath);
  if(strrchr(filePath, '/'))    
    return (strrchr(filePath, '/')+1);
  else
    return filePath;
}

//returns the directory part excluding the filename
char* getFileDir(char* filePath){
  char* fileName = getFileName(filePath);
  if(!fileName)	return 0;
  int len = fileName-filePath+1;
  char* fileDir = malloc(sizeof(char)*(len+1));
  memset(fileDir,0,len+1);
  snprintf(fileDir,len,"%s",filePath);
  //printf("The dir is %s\n",fileDir);
  return fileDir;
}

//this function returns the user@hostname for the ssh
//conneciton
StringPtr getSSHString(int host){
  if(host==0) return 0;
  HostInfoPtr hostInfo = hostMap[host];
  if(!hostInfo)	return 0;  
  char* tempString = strdup(hostInfo->mnt_fsname);
  int i=0;
  while(i<strlen(tempString) && tempString[i]!=':')
    ++i;  
  //printf("The value of i is %d and char is %c\n",i,tempString[i]);
  char* connString = malloc(sizeof(char)*(i+1));
  memset(connString,0,i+1);
  snprintf(connString,i+1,"%s",tempString);
  //printf("connsString is %s and its length is %d with i-1 as %d\n",connString,strlen(connString),i-1);
  free(tempString);
  StringPtr sshString = createString();  
  sshString->text = connString;  
  sshString->length = i;  
  return sshString;
}