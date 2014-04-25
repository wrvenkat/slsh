# Makefile for cminus

CC=gcc
LEX=flex
YACC=bison
MV=/bin/mv
RM=/bin/rm
CP=/bin/cp

LIBRARIES=  -ll

CFLAGS=-g 
YFLAGS=-d

PROGRAM=parser

OBJECTS=parser.o lexer.o structure.o print.o util.o plan1.o remotecmd.o globals.o execute.o

SOURCES=lexer.l parser.y

CTEMPS=lexer.c parser.c parser.tab.h parser.tab.c

$(PROGRAM): $(OBJECTS) globals.h
	$(CC) $(CFLAGS) -o $(PROGRAM) $(OBJECTS) $(LIBRARIES)

lexer: lexer.o
	$(CC) $(CFLAGS) -o lexer lexer.o -ll

structure.o: structure.c
	$(CC) $(CFLAGS) -c structure.c

util.o: util.c
	$(CC) $(CFLAGS) -c util.c

plan1.o:  plan1.c
	$(CC) $(CFLAGS) -c plan1.c

remotecmd.o:  remotecmd.c
	$(CC) $(CFLAGS) -c remotecmd.c

globals.o:  globals.c
	$(CC) $(CFLAGS) -c globals.c

execute.o:  execute.c
	$(CC) $(CFLAGS) -c execute.c

#ast.o:  ast.c
#	$(CC) $(CFLAGS) -c ast.c

print.o: print.c
	$(CC) $(CFLAGS) -c print.c

parser.o: parser.c
	$(CC) $(CFLAGS) -c parser.c

parser.c: parser.y
	$(YACC)  parser.y -o parser.c

lexer.o: lexer.c parser.tab.h
	$(CC) $(CFLAGS) -c lexer.c

parser.tab.h: parser.y
	$(YACC) -d -v --report=state parser.y 

lexer.c: lexer.l
	$(LEX) -olexer.c lexer.l 

clean:
	$(RM) $(PROGRAM) 

ex:
	$(RM) $(OBJECTS) $(CTEMPS)