#include <string.h>
#include "globals.h"
#include <stdlib.h>

StringPtr createString(){
  StringPtr newString = malloc(sizeof(STRING));
  memset(newString,0,sizeof(STRING));
  return newString;
}

void freeString(StringPtr stringPtr){
  free(stringPtr->text);
  free(stringPtr);
}