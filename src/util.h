#ifndef UTIL_H
#define UTIL_H

#include "structure.h"
#include "globals.h"
#include <regex.h>

#define FILE_REGEX "([^\n\r\v\/]*(\/[^\/]+)+)"

regex_t fileReg;

typedef struct file FILE_;
typedef FILE_* FilePtr;

typedef struct cmdstat CMDSTAT;
typedef CMDSTAT* CmdStatPtr;

//intialize global variables and flags
int initGlobals();

//this function returns a FILE_ structure for a given valid fileStr
FilePtr getFileStruct(char* fileStr);

//returns true if a string is a valid file path or not
int isFilePath(char* fakeFile);

//compiles the filepath regex
void compRegex();

//this function populates the global
//hostMap array
void createHostMap();

//this function returns the home directory from the the remote
//fs_name
char* getHomeDir(char* fsName);

//returns the filename given the filePath
char* getFileName(char* filePath);

//returns the directory part excluding the filename
char* getFileDir(char* filePath);

// this function gets rid of the sequence(s) of "\ "
// in the filePath and returns a string which doesn't contain those
char* getRidOfEscapeChars(char* filePath);

//strips leading and trailing spaces
char *strstrip(char *s);

//function that returns the hashValue of a given identifier
int computeHashValue(char * identifier);

//insert cmdStat in the cmdStats
CmdStatPtr cmdStatInsert(char *name, double cost, int type);

//looks up cmdStat
CmdStatPtr cmdStatLookup(char *name);

//this function returns the user@hostname for the ssh
//conneciton
StringPtr getSSHString(int host);

//loads the command stats from cmdStatsFileName onto cmdStats array
void loadCmdStats(char* cmdStatsFileName);

//initializes the host involved array to zero
void initHostInvolved();

#endif