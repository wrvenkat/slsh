#ifndef REMOTECMD_H
#define REMOTECMD_H

#include "globals.h"
#include "structure.h"
#include "util.h"

//specifies a string linked list
typedef struct remotecmdtext{
  StringPtr cmdText;
  ArgType type;
  struct remotecmdtext* next;
}REMOTECMDTEXT;

typedef REMOTECMDTEXT* RemoteCmdTextPtr;

//specifies the list of commands in list to be executed on host 'host'
typedef struct remotecmd{
  StringPtr cmdText;
  //TransferFilePtr filesHead;
  FilePtr transferFileList;
  int host;
  //int inputRedir;
  //int outputRedir;
  int currOutputRedir;
  struct remotecmd* next;
}REMOTECMD;

typedef REMOTECMD* RemoteCmdPtr;

//this function frees the RemoteCmdPtr and all of 
//its fields
void freeRemoteCmdPtr(RemoteCmdPtr remoteCmdPtr);

//this structure is used in transferring files
// typedef struct transferfile{
//   //the one provided on the command line
//   char* orig;
//   //the one with the resolved name
//   char* actual;
//   int hostd;
//   int minId;
//   struct transferfile* next;
// }TRANSFERFILE;
// 
// typedef TRANSFERFILE* TransferFilePtr;

//the head and tail pointers for the RemoteCmd list
RemoteCmdPtr remoteCmdHeadPtr;
RemoteCmdPtr remoteCmdTailPtr;

//this function returns an initalised REMOTECMD struct pointer
RemoteCmdPtr createRemoteCmd();

//this function returns an initalised REMOTECMDTEXT struct pointer
RemoteCmdTextPtr createRemoteCmdText();

//this function returns the filepath for the 
//modified filepath
//int getFilePathLength(ArgPtr arg, int execHost);

//this function returns the altered filepath
StringPtr getFilePath(ArgPtr arg, int execHost);

//this function returns a remotecmdtext struct which is created from cmd
//here the function ierates through the arguments and replaces any
//filepath args with the appropriate remote references
RemoteCmdTextPtr getRemoteCmdTextFromCmd(CommandPtr cmd);

//this function recursively travels the pipeline tree
//and prepares another tree that contains the shell/ssh
//commands to be executed on different machines
//it does so by recursively going into the tree structure and recursing with the 
//current command ptr if the next command has the same execution host
//this is done only if it is successive
RemoteCmdPtr _makeRemoteCmd(CommandPtr currCmdPtr,CommandPtr prevCmdPtr, int recursive);

//the bootstrap function that calls _makeRemoteCmd
void makeRemoteCmd();

//print the command tree
void printRemoteCmdTree();

void printTransferList(FilePtr head);

#endif