.SUFFIXES: .cc .c .o .h .d
FLAGS+=-std=c++11 #-Wno-error=offsetof
FLAGS+=-I. -I../src -g

EXES=processCredit processPP

# object files making up libravel
OBJS=
VPATH=../src
LIBS=
LIBS:=-L$(HOME)/usr/lib64 $(LIBS)
CPLUSPLUS=g++

#chmod command is to counteract AEGIS removing execute privelege from scripts
all: $(EXES)

# This rule uses a header file of object descriptors
.cc.o:
	$(CPLUSPLUS) -c $(FLAGS)  $<

.cc.d: 
	$(CPLUSPLUS) $(FLAGS) -MM -MG $< >$@

# how to build a model executable
$(EXES): %: %.o $(OBJS)
	$(CPLUSPLUS) $(FLAGS) $^ $(LIBS) -o $@

ifneq ($(MAKECMDGOALS),clean)
include $(EXES:=.d) 
include $(OBJS:.o=.d)
endif

clean:
	rm *.o $(EXES)

