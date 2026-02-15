# Makefile for libdda - DDA numerical core static library
# Builds without BLAS/LAPACK and without Win32 GUI code.
# Run from src/ directory.
#
# Dependencies: libxml2 (for XML geometry/analysis file support)
# On macOS: libxml2 is typically available via Xcode Command Line Tools

CC = gcc
AR = ar
CFLAGS = -g -Wall -O2 -I../include -Iplatform/stubs
ARFLAGS = rcs

# Do not define WIN32 - enables non-GUI code paths
LIB = libdda.a
OBJDIR = .obj

# Numerical core sources (excludes blas/, lapack/, win32gui/, unittests/)
SRCS = \
	analysisdata.c \
	analysisdriver.c \
	analysisreader.c \
	analysisddaml.c \
	blockhandler.c \
	blocks.c \
	bolt.c \
	combineddf.c \
	contacts.c \
	constants.c \
	contact.c \
	dda.c \
	dda_error.c \
	ddadlist.c \
	ddafile.c \
	ddaml.c \
	ddamemory.c \
	display.c \
	dxf.c \
	friction.c \
	functions.c \
	geomddaml.c \
	geomdriver.c \
	geometrydata.c \
	geomreader.c \
	ghssolver.c \
	gravity.c \
	inpoly.c \
	joint.c \
	loadmatrix.c \
	loadpoint.c \
	loghtml.c \
	lns.c \
	material.c \
	mohrcoulomb.c \
	postprocess.c \
	rb.c \
	replay.c \
	sparsestorage.c \
	stress.c \
	timehistory.c \
	timestep.c \
	transfercontact.c \
	utils.c \
	graphics.c

# Stub implementations for statusbar/toolbar (replaces win32gui versions)
STUB_SRCS = platform/stubs/stubs.c

OBJS = $(addprefix $(OBJDIR)/,$(SRCS:.c=.o))
STUB_OBJS = $(OBJDIR)/stubs.o

all: $(LIB)

$(LIB): $(OBJS) $(STUB_OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/stubs.o: platform/stubs/stubs.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(LIB)

.PHONY: all clean
