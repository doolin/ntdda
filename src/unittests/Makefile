# Makefile for DDA unit tests


# The makefile should reflect the structure of the 
# DDA code.  Thus all of the solver stuff could go 
# under one main target, which could handle all of the 
# dependencies very nicely.

CFLAGS = -g -Wall -DSTANDALONE

all: matmulttest contacttest

bolttest: bolttest.c
	gcc $(CFLAGS)  -o bolttest bolttest.c ../bolt.c  ../ddadlist.c ../ddamemory.c  -I ../../include -lm


matmulttest:
	gcc -o matmulttest matmulttest.c ../ghssolver.c ../ddamemory.c ../errorhandler.c -I ../../include -lm

contacttest:
	gcc -o contacttest contacttest.c ../contacts.c ../ddamemory.c ../errorhandler.c -I ../../include -lm

clean:
	rm -f *.exe *~  *.stackdump
