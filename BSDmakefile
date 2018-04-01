CC      = g++
AR      = ar
CFLAGS      = -std=c++11 -ggdb -Ofast -Os -W -Wall -Wno-unused-parameter -Wno-unused-variable -fPIC
TCFLAGS     = -Wno-unused-but-set-variable -Wno-unknown-warning-option
EXTFLAGS    =
SRCDIR      = src
BUILDDIR    = build
TARGETDIR   = bin
TESTDIR     = test
LIBPATH     = lib
SRCEXT      = cpp
SOURCES     = $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
TESTSRCS    = $(shell find $(TESTDIR) -type f -name *.$(SRCEXT))
OBJECTS     = $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
TESTOBJS    = $(patsubst $(TESTDIR)/%,$(TESTDIR)/%,$(TESTSRCS:.$(SRCEXT)=.o))
HEADERS     = -Iinclude -I$(LIBPATH) -I.
ANAME       = $(TARGETDIR)/libnogdb.a
SONAME      = $(TARGETDIR)/libnogdb.so
DEPOBJECTS  = $(LIBPATH)/lmdb/*.o
DEPENDENCIES    = $(LIBPATH)/lmdb/liblmdb.a
LDFLAGS     =
TESTNAME    = runtest
TESTFLAGS   = -DTEST_CONTEXT_OPERATIONS -DTEST_SCHEMA_OPERATIONS -DTEST_RECORD_OPERATIONS -DTEST_MISC_OPERATIONS -DTEST_GRAPH_OPERATIONS -DTEST_FIND_OPERATIONS -DTEST_INHERITANCE_OPERATIONS -DTEST_INDEX_OPERATIONS -DTEST_SCHEMA_TXN_OPERATIONS -DTEST_TXN_OPERATIONS

all: lmdb $(ANAME) $(SONAME)

lmdb:
    @echo "Compiling dependencies..."
    @cd lib/lmdb; make
    @echo "All dependencies have been compiled... [Done]"
    
$(ANAME): $(OBJECTS)
    @echo "Building nogdb static library... "
    $(AR) rcs $@ $(OBJECTS) $(DEPOBJECTS)
    @echo "nogdb static library has been created... [Done]"

$(SONAME): $(OBJECTS)
    @echo "Building nogdb shared library... "
    $(CC) -L/usr/local/lib -shared -o $@ $(OBJECTS) $(DEPENDENCIES) $(LDFLAGS)
    @echo "nogdb shared library has been created... [Done]"

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
    @mkdir -p $(BUILDDIR)
    $(CC) $(CFLAGS) $(HEADERS) -c $< -o $@

test: lmdb $(ANAME) $(SONAME) $(TESTOBJS)
	$(CC) $(CFLAGS) $(TCFLAGS) $(HEADERS) $(TESTOBJS) $(ANAME) -o $(TESTNAME) $(LDFLAGS)
	$(CC) $(CFLAGS) $(TCFLAGS) $(HEADERS) $(TESTOBJS) $(ANAME) -o $(TESTNAME)_debug_address $(LDFLAGS) $(EXTFLAGS)
	$(CC) $(CFLAGS) $(TCFLAGS) $(HEADERS) $(TESTOBJS) -o $(TESTNAME) $(SONAME) $(LDFLAGS)
	$(CC) $(CFLAGS) $(TCFLAGS) $(HEADERS) $(TESTOBJS) -o $(TESTNAME)_debug_address $(SONAME) $(LDFLAGS) $(EXTFLAGS)
	@./runtest; rm -rf runtest.db
	@./runtest_debug_address; rm -rf runtest.db

$(TESTDIR)/%.o: $(TESTDIR)/%.$(SRCEXT)
	$(CC) $(CFLAGS) $(TCFLAGS) $(TESTFLAGS) $(HEADERS) -c $< -o $@


install:
    @printf "Installing libnogdb..."
    @mkdir -p /usr/local/lib
    @cp $(TARGETDIR)/* /usr/local/lib
    @mkdir -p /usr/local/include/nogdb
    @cp include/*.h /usr/local/include/nogdb
    @cp -R lib/* /usr/local/include/nogdb
    @echo " [Done]"

uninstall:
    @printf "Uninstalling libnogdb..."
    @rm -f /usr/local/lib/libnogdb.*
    @rm -rf /usr/local/include/nogdb
    @echo " [Done]"
    
clean:
    @printf "Cleaning all executable objects and libs..."
    @rm -rf runtest*
    @find * -name "*.o" -type f -exec rm -f {} \;
    @find * -name "*.a" -type f -exec rm -f {} \;
    @find * -name "*.so" -type f -exec rm -f {} \;
    @echo " [Done]"

.PHONY: all lmdb test install uninstall clean