# Makefile for DDA unit tests


# The makefile should reflect the structure of the
# DDA code.  Thus all of the solver stuff could go
# under one main target, which could handle all of the
# dependencies very nicely.


CFLAGS = -g -Wall -DSTANDALONE

BIN = ./bin

ALL_TESTS_SOURCE =  ../functions.c     \
		    constants_test.c   \
		    ../constants.c        \
		    unittest.c         \
		    ../bolt.c          \
		    ../ddadlist.c      \
		    ../ddamemory.c     \
		    bolttest.c

ADATA_SOURCE =  ../constants.c      \
                ../timehistory.c    \
                ../analysisdata.c   \
                ../ddamemory.c      \
                ../loadpoint.c      \
                ../dda.c            \
                ../analysisreader.c \
                ../ddafile.c


GDATA_SOURCE =  ../inpoly.c         \
                ../geometrydata.c   \
                ../ddafile.c        \
                ../ddamemory.c      \
                ../geomreader.c


OBJECTS =                    \
           matmulttest       \
	   loadpointtest     \
           stress_test       \
	   material_test     \
           bolttest          \
           inpoly_test       \
           ddadlist_test     \
           constants_test    \
	   all_tests


all: $(OBJECTS)



all_tests: all_tests.c
	gcc -Wall -g -o $(BIN)/all_tests all_tests.c $(ALL_TESTS_SOURCE) -I../../include -lm

ddadlist_test: ddadlist_test.c ../ddadlist.c
	gcc $(FLAGS) -o  $(BIN)/ddadlist_test ddadlist_test.c ../ddadlist.c  -I../../include

geometrydata_test: geometrydata_test.c
	gcc $(CFLAGS)  -o  $(BIN)/geometrydata_test geometrydata_test.c  $(GDATA_SOURCE)  -I ../../include -lm

analysisdata_test: analysisdata_test.c
	gcc $(CFLAGS)  -o  $(BIN)/analysisdata_test analysisdata_test.c  $(ADATA_SOURCE)  -I ../../include -lm


material_test: material_test.c
	gcc $(CFLAGS)  -o  $(BIN)/material_test material_test.c ../material.c  ../ddadlist.c ../ddamemory.c  -I ../../include -lm

bolttest: bolttest.c
	gcc $(CFLAGS)  -o  $(BIN)/bolttest bolttest.c ../bolt.c  ../ddadlist.c ../ddamemory.c  -I ../../include -lm

stress_test: stress_test.c ../stress.c
	gcc $(CFLAGS)  -o  $(BIN)/stress_test stress_test.c ../stress.c  -I ../../include -lm

inpoly_test: inpoly_test.c ../inpoly.c
	gcc $(CFLAGS)  -o  $(BIN)/inpoly_test inpoly_test.c ../inpoly.c  -I ../../include -lm

constants_test: constants_test.c ../constants.c
	gcc $(CFLAGS)  -o  $(BIN)/constants_test constants_test.c ../constants.c  unittest.c -I ../../include -lm

loadpointtest: loadpointtest.c
	gcc $(CFLAGS)  -o  $(BIN)/loadpointtest loadpointtest.c ../loadpoint.c  ../ddadlist.c ../ddamemory.c  -I ../../include -lm


matmulttest:
	gcc -o  $(BIN)/matmulttest matmulttest.c ../ghssolver.c ../ddamemory.c  -I ../../include -lm

# target is broken, won't link
contacttest:
	gcc -o  $(BIN)/contacttest contacttest.c ../contacts.c ../ddamemory.c ../utils.c -I ../../include -lm

clean:
	rm -f *.exe *~  *.stackdump $(OBJECTS)
