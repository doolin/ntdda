# Makefile for DDA unit tests


# The makefile should reflect the structure of the
# DDA code.  Thus all of the solver stuff could go
# under one main target, which could handle all of the
# dependencies very nicely.


CFLAGS = -g -Wall -DSTANDALONE -I../../include -I../../src/platform/stubs
BIN = ./bin

SRC = ../../src
ALL_TESTS_SOURCE =  $(SRC)/functions.c     \
		    constants_test.c   \
		    $(SRC)/constants.c        \
		    unittest.c         \
		    $(SRC)/bolt.c          \
		    $(SRC)/ddadlist.c      \
		    $(SRC)/ddamemory.c     \
		    bolttest.c

ADATA_SOURCE =  $(SRC)/constants.c      \
                $(SRC)/timehistory.c    \
                $(SRC)/analysisdata.c   \
                $(SRC)/ddamemory.c      \
                $(SRC)/loadpoint.c      \
                $(SRC)/dda.c            \
                $(SRC)/analysisreader.c \
                $(SRC)/ddafile.c


GDATA_SOURCE =  $(SRC)/inpoly.c         \
                $(SRC)/geometrydata.c   \
                $(SRC)/ddafile.c        \
                $(SRC)/ddamemory.c      \
                $(SRC)/geomreader.c     \
                $(SRC)/dda.c            \
                $(SRC)/platform/stubs/stubs.c


OBJECTS =                    \
           matmulttest       \
           loadpointtest     \
           stress_test       \
           material_test     \
           bolttest          \
           inpoly_test       \
           ddadlist_test     \
           constants_test    \
           geometrydata_test \
           analysisdata_test \
           all_tests


all: $(BIN) $(OBJECTS)

$(BIN):
	mkdir -p $(BIN)



all_tests: all_tests.c
	gcc $(CFLAGS) -DALL_TESTS -o $(BIN)/all_tests all_tests.c $(ALL_TESTS_SOURCE) -lm

ddadlist_test: ddadlist_test.c $(SRC)/ddadlist.c
	gcc $(CFLAGS) -o $(BIN)/ddadlist_test ddadlist_test.c $(SRC)/ddadlist.c

geometrydata_test: geometrydata_test.c
	gcc $(CFLAGS) -o $(BIN)/geometrydata_test geometrydata_test.c $(GDATA_SOURCE) -lm

analysisdata_test: analysisdata_test.c
	gcc $(CFLAGS) -o $(BIN)/analysisdata_test analysisdata_test.c $(ADATA_SOURCE) $(SRC)/platform/stubs/stubs.c -lm

material_test: material_test.c
	gcc $(CFLAGS) -o $(BIN)/material_test material_test.c $(SRC)/material.c $(SRC)/ddadlist.c $(SRC)/ddamemory.c -lm

bolttest: bolttest.c
	gcc $(CFLAGS) -o $(BIN)/bolttest bolttest.c $(SRC)/bolt.c $(SRC)/ddadlist.c $(SRC)/ddamemory.c -lm

stress_test: stress_test.c $(SRC)/stress.c
	gcc $(CFLAGS) -o $(BIN)/stress_test stress_test.c $(SRC)/stress.c $(SRC)/mohrcoulomb.c -lm

inpoly_test: inpoly_test.c $(SRC)/inpoly.c
	gcc $(CFLAGS) -o $(BIN)/inpoly_test inpoly_test.c $(SRC)/inpoly.c -lm

constants_test: constants_test.c $(SRC)/constants.c
	gcc $(CFLAGS) -o $(BIN)/constants_test constants_test.c $(SRC)/constants.c unittest.c -lm

loadpointtest: loadpointtest.c
	gcc $(CFLAGS) -o $(BIN)/loadpointtest loadpointtest.c $(SRC)/loadpoint.c $(SRC)/ddadlist.c $(SRC)/ddamemory.c -lm

matmulttest: matmulttest.c
	gcc $(CFLAGS) -o $(BIN)/matmulttest matmulttest.c $(SRC)/ghssolver.c $(SRC)/ddamemory.c -lm

# target is broken, won't link
contacttest:
	gcc -o  $(BIN)/contacttest contacttest.c $(SRC)/contacts.c $(SRC)/ddamemory.c $(SRC)/utils.c -I ../../include -lm

test: all
	@echo "Running unit tests..."
	@for t in $(OBJECTS); do \
	  echo "=== bin/$$t ===" && ./$(BIN)/$$t 2>&1 || true; \
	done

clean:
	rm -rf $(BIN) *.exe *~ *.stackdump
