# 
# Makefile for project 2 (COMP30017)
# William Pan 694945
#


## CC  = Compiler.
## CFLAGS = Compiler flags.
CC	= gcc
CFLAGS 	= -Wall  

## OBJ = Object files.
## SRC = Source files.
## EXE = Executable name.

## Server arguments
SRCSERVER = server.c log.c game.c
OBJSERVER =	server.o log.o game.o
EXESERVER = server
## Client arguments
SRCCLIENT =	client.c
OBJCLIENT =	client.o
EXECLIENT = client

all:
	@echo "Please use make server or make client"

## Build for server command
server:	$(OBJSERVER)
		$(CC) $(CFLAGS) -o $(EXESERVER) $(OBJSERVER) -lm -lpthread

## Build for client command
## Build for client command
client:	$(OBJCLIENT)
		$(CC) $(CFLAGS) -o $(EXECLIENT) $(OBJCLIENT) -lm


## Clean: Remove object files and core dump files.
clean:
		/bin/rm $(OBJSERVER)
		/bin/rm $(OBJCLIENT) 

## Clobber: Performs Clean and removes executable file.
clobber: clean
		/bin/rm $(EXESERVER) 
		/bin/rm $(EXECLIENT) 

## Dependencies
client.o:	client.c
server.o:	server.c log.h game.h
