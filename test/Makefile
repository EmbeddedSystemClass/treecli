#CDEBUGFLAGS=-fprofile-arcs -ftest-coverage -pg -g
CFLAGS=-I . -I .. -O0 --std=c99  $(CDEBUGFLAGS)
LDFLAGS=-O0  $(CDEBUGFLAGS)
CC=gcc
LD=gcc
GCOV=gcov


all: test1

treecli:
	$(CC) $(CFLAGS) -c ../treecli.c
	$(CC) $(CFLAGS) -c ../lineedit.c
	$(CC) $(CFLAGS) -c ../treecli_tests.c

test1: treecli
	$(CC) $(CFLAGS) -c test1.c
	$(LD) $(LDFLAGS) test1.o lineedit.o treecli.o treecli_tests.o -o test1


