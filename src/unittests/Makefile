# Makefile for DDA unit tests


# The makefile should reflect the structure of the 
# DDA code.  Thus all of the solver stuff could go 
# under one main target, which could handle all of the 
# dependencies very nicely.


CFLAGS = -g -Wall -DSTANDALONE

OBJECTS =  bolttest   \
           matmulttest \
	   loadpointtest

all: $(OBJECTS)




bolttest: bolttest.c 
	gcc $(CFLAGS)  -o bolttest bolttest.c ../bolt.c  ../ddadlist.c ../ddamemory.c  -I ../../include -lm

stress_test: stress_test.c ../stress.c
	gcc $(CFLAGS)  -o stress_test stress_test.c ../stress.c  -I ../../include -lm


loadpointtest: loadpointtest.c 
	gcc $(CFLAGS)  -o loadpointtest loadpointtest.c ../loadpoint.c  ../ddadlist.c ../ddamemory.c  -I ../../include -lm


matmulttest:
	gcc -o matmulttest matmulttest.c ../ghssolver.c ../ddamemory.c  -I ../../include -lm

# target is broken, won't link
contacttest:
	gcc -o contacttest contacttest.c ../contacts.c ../ddamemory.c ../utils.c -I ../../include -lm

clean:
	rm -f *.exe *~  *.stackdump $(OBJECTS)
