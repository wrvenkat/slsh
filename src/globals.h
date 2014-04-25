#ifndef GLOBALS_H
#define GLOBALS_H

#define MAX_HOST 255
#define MAX_CHAR 255
#define MAX_DIR_LENGTH 255
#define TEMP_FILE_LENGTH 30

#define DEBUG1 1
#define DEBUG2 1
#define DEBUG3 1

#define TEMP_DIR ".slsh/"
#define P_PATH_STR "~/.ssh/%r@%h:%p"
#define PERSIST_SSH "-o ControlPersist=yes -M -S"
#define PERSIST_EXIT "ssh -O exit -S"
#define REDIR_NULL "> /dev/null 2> /dev/null"

#define ESCAPE_SEQ1 "\ "
#define ESCAPE_SEQ2 "\\t"

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
}STRING;

typedef STRING* StringPtr;

StringPtr createString();

void freeString(StringPtr stringPtr);

void freeStringList(StringPtr head);

#endif