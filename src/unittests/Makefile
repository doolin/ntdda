# Makefile for DDA unit tests


# The makefile should reflect the structure of the 
# DDA code.  Thus all of the solver stuff could go 
# under one main target, which could handle all of the 
# dependencies very nicely.


CFLAGS = -g -Wall -DSTANDALONE


ADATA_SOURCE =  ../constants.c      \
                ../timehistory.c    \
                ../analysisdata.c   \
                ../ddamemory.c      \
                ../loadpoint.c      \
                ../gravity.c        \
                ../utils.c          \
                ../timestep.c

OBJECTS =  bolttest          \
           matmulttest       \
	   loadpointtest     \
           stress_test       \
	   material_test     \
           analysisdata_test 

all: $(OBJECTS)


analysisdata_test: analysisdata_test.c 
	gcc $(CFLAGS)  -o analysisdata_test analysisdata_test.c  $(ADATA_SOURCE)  -I ../../include -lm


material_test: material_test.c 
	gcc $(CFLAGS)  -o material_test material_test.c ../material.c  ../ddadlist.c ../ddamemory.c  -I ../../include -lm

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
