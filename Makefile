.SUFFIXES: .cd .d $(SUFFIXES)
OS=$(shell uname)

ifdef MXE
MXE_32bit=$(shell if which i686-w64-mingw32.static-g++>&/dev/null; then echo 1; fi)
MXE_64bit=$(shell if which x86_64-w64-mingw32.static-g++>&/dev/null; then echo 1; fi)

ifeq ($(MXE_32bit),1)
MXE_PREFIX=i686-w64-mingw32.static
else
ifeq ($(MXE_64bit),1)
MXE_PREFIX=x86_64-w64-mingw32.static
else
$(error "MXE compiler not found")
endif
endif
CC=$(MXE_PREFIX)-gcc
CXX=$(MXE_PREFIX)-g++
FLAGS=-DWIN32
else
CC=gcc
CXX=g++
FLAGS=-fPIC -isystem /usr/local/include -isystem /opt/local/include 
endif
CXXFLAGS=-std=c++11

CXXFLAGS=-std=c++11


ifeq ($(OS),Darwin)
FLAGS+=-isystem /usr/local/include -isystem /opt/local/include
endif

ifdef DEBUG
OPT=-g
else
OPT=-O3 -DNDEBUG
endif

OBJS=ravelState.o dynamicRavelCAPI.o

all: testCAPI.o libravelCAPI.a

libravelCAPI.a: $(OBJS)
	ar r $@ $^

.cc.o: 
	$(CXX) -c $(FLAGS) $(CXXFLAGS) $(OPT) -o $@ $<

.c.o: 
	$(CC) -c $(FLAGS) $(OPT) -o $@ $<

.c.d: 
	$(CC) $(FLAGS) -MM -MG  $< >$@

.cc.d: 
	$(CXX) $(FLAGS)  $(CXXFLAGS) -MM -MG $< >$@

ifneq ($(MAKECMDGOALS),clean)
include $(OBJS:.o=.d)
include testCAPI.d
endif

clean:
	-rm *.o *.d *.cd *.a *~ \#*
