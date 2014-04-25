#include <string.h>
#include "globals.h"
#include <stdlib.h>

StringPtr createString(){
  StringPtr newString = malloc(sizeof(STRING));
  memset(newString,0,sizeof(STRING));
  return newString;
}

void freeStringList(StringPtr head){
  StringPtr tempStrPtr = head;
  while(tempStrPtr){
    freeString(tempStrPtr);
    StringPtr tempTempStrPtr = tempStrPtr->next;
    free(tempStrPtr);
    tempStrPtr=tempTempStrPtr;
  }
}

void freeString(StringPtr stringPtr){
  free(stringPtr->text);
  free(stringPtr);
}