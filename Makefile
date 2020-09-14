.SUFFIXES: .cd .d $(SUFFIXES)
CC=gcc
CXX=g++
FLAGS=-fPIC

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
	-rm *.o *.d *.cd
