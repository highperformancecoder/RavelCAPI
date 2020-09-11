.SUFFIXES: .cd .d $(SUFFIXES)
CC=gcc
CXX=g++

ifdef DEBUG
FLAGS+=-g
else
FLAGS+=-O3 -DNDEBUG
endif

OBJS=ravelState.o dynamicRavelCAPI.o

all: testCAPI.o libravelCAPI.a

libravelCAPI.a: $(OBJS)
	ar r $@ $^

.c.d: 
	$(CC) $(FLAGS) -MM -MG  $< >$@

.cc.d: 
	$(CXX) $(FLAGS)  $(CXXFLAGS) -MM -MG $< >$@

ifneq ($(MAKECMDGOALS),clean)
include $(OBJS:.o=.d)
include testCAPI.d
endif

clean:
	-rm *.o *.d *.cd
