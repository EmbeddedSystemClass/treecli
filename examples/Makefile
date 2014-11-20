#CDEBUGFLAGS=-fprofile-arcs -ftest-coverage -pg -g
CFLAGS=-I . -I .. -I ../lineedit -O0 --std=c99  $(CDEBUGFLAGS)
LDFLAGS=-O0  $(CDEBUGFLAGS)
CC=gcc
LD=gcc
GCOV=gcov


all: example1

treecli:
	$(CC) $(CFLAGS) -c ../treecli_parser.c
	$(CC) $(CFLAGS) -c ../treecli_shell.c
	$(CC) $(CFLAGS) -c ../lineedit/lineedit.c

example1: treecli
	$(CC) $(CFLAGS) -c example1.c
	$(LD) $(LDFLAGS) example1.o lineedit.o treecli_shell.o treecli_parser.o -o example1


