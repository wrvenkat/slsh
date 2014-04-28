#ifndef GLOBALS_H
#define GLOBALS_H

#define MAX_HOST 255
#define MAX_CHAR 255
#define MAX_DIR_LENGTH 255
#define TEMP_FILE_LENGTH 30
#define MAX_CMDS 200

//debug flags to enable/disable debug messages of various stages
//and various functions
//general debug messages
#define DBG_GEN 0
//debug messages at the entry of functions
#define DBG_FUNC_ENTRY 0
//turns on or off all of the debug messages in makePlan1()
//NOTE if turning on DBG_PLAN it would help tp turn on DBG_FILE
//because then you can see what the details of each file are
#define DBG_PLAN 1
//enable debug messages for getFileStruct function
#define DBG_FILE 0
//turns on or off all of the debug messages in printCmdTree()
#define DBG_CMD_TREE_PRINT 0
//turns on or off all of the debug messages in makeRemoteCmd()
#define DBG_MAKE_RMTE_CMD 0
//turns on or off all of the debug messages in printRemoteCmdTree()
#define DBG_RMTE_CMD_TREE_PRINT 1
//turns on or off all of the debug messages in free*() functions
#define DBG_FREE 0
//turns on or off all of the debug messages in lexer
#define DBG_LEX 0
//turns on or off all of the debug messages in parser
#define DBG_PARSE 0
//toggle on or off to print hostMap
#define DBG_HOSTMAP_PRINT 0
//toggle on or off to display final execution command string
#define DBG_EXEC 1

#define FLG_EXEC_DO 1

#define TEMP_DIR ".slsh/"
#define P_PATH_STR "~/.ssh/%r@%h:%p"
#define PERSIST_SSH "-o ControlPersist=yes -M -S"
#define PERSIST_EXIT "ssh -O exit -S"
#define REDIR_NULL "> /dev/null 2> /dev/null"

#define ESCAPE_SEQ1 "\ "
#define ESCAPE_SEQ2 "\\t"

//TODO For future use, does nothing as of now
//flag to decide if we have to aggressively
//look for args inside QARGS as well
int aggFlag;
//flag to decide if we have to try to look for
//files inside the QARG by splitting the argument based on
//chars , or | or space
int sepFlag;
//flag to decide if we have to consider multiple appearance
//of same file again as separate or same
int sameFileFlag1;
//use persistent connection flag
int usePersistent;

typedef struct stringstruct{
  char* text;
  int length;
  struct stringstruct* next;
}STRINGSTRUCT;

typedef STRINGSTRUCT* StringPtr;

//creates a string
StringPtr createString();

//frees a string
void freeString(StringPtr stringPtr);

#endif