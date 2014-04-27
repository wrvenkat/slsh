#include <string.h>
#include "globals.h"
#include <stdlib.h>
#include <stdio.h>

StringPtr createString(){
  StringPtr newString = (StringPtr)malloc(sizeof(STRINGSTRUCT));
  newString->text=0;
  newString->next=0;
  newString->length=0;
  return newString;
}

void freeString(StringPtr stringPtr){  
  if(!stringPtr)	return;  
  free(stringPtr->text);  
  stringPtr->length=0;  
  free(stringPtr);  
}