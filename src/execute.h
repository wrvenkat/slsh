#ifndef _EXECUTE_H
#define _EXECUTE_H

#include "remotecmd.h"
#include "globals.h"

int gIndex;

typedef struct remotecmd REMOTECMD;
typedef REMOTECMD* RemoteCmdPtr;

//the bootstrap function that travels the
//RemoteCmd list and executes each command
int executePlan(RemoteCmdPtr remoteCmdHeadPtr);

//TODO inout and output redirection is pending
int executeCommand(RemoteCmdPtr remoteCmdPtr,char* ipFile, int prevExecHost);

//this function executes the output redirection part of the pipeline
int executeOutputRedirCmd(RemoteCmdPtr remoteCmdPtr,char* ipFile, int prevExecHost);

//this function transfers the given file to the
//execHost
int transferFile(FilePtr currFile, int execHost);

//this function transfers the fileName which is at prevHost
//to currHost
int transferInputFile(char* fileName, int prevHost, int currHost);

//this function prepapres the string to be executed 
//based on the target machine
StringPtr prepareCommand(RemoteCmdPtr cmdText, char* tempFile, char* opFile, int execHost);

//make temporary directories
void makeTempDir();

//this function makes the persistent SSH connections and also exits them once
//all the commands are executed
int makePersistentSSH(RemoteCmdPtr remoteCmdHeadPtr);

#endif