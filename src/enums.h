#ifndef ENUMS_H
#define ENUMS_H

//types of command-line arguments
typedef enum {WORDT, SOPTT, LOPTT, PATHT, QARGT}ArgType;
//types of high level structures
typedef enum {FORLOOPT, PIPELINELST}InputUnitType;
//types for Command stats to which the cost constant applies
typedef enum {ARGCOST,IPCOST,BOTHCOST}CostType;

#endif