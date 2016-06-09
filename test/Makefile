DEBUG=1

# root directory for ecolab include files and libraries
ifeq ($(shell ls $(HOME)/usr/ecolab/include/ecolab.h),$(HOME)/usr/ecolab/include/ecolab.h)
ECOLAB_HOME=$(HOME)/usr/ecolab
else
ECOLAB_HOME=/usr/local/ecolab
endif

include $(ECOLAB_HOME)/include/Makefile

ACTIONS+=xml_pack xml_unpack random_init
UNITTESTOBJS=main.o ravelCairo.o filterCairo.o sortedVectorTest.o splitMergeTest.o
FLAGS+=-std=c++11 -I../src -I.. -g -UNDEBUG
LIBS+=-L.. -lravel -lUnitTest++ -lboost_regex -lcairo
VPATH+=src

EXES=unittests testSerialisation 

all: $(EXES)

unittests: $(UNITTESTOBJS) ../src/*.o
	$(CPLUSPLUS) $(FLAGS)  -o $@ $^ $(LIBS)

testSerialisation: %: %.o
	$(LINK) $(FLAGS) $*.o $(LIBS) -o $@

include $(UNITTESTOBJS:.o=.d)

clean:
	$(BASIC_CLEAN) $(EXES)
	cd 00; $(BASIC_CLEAN)
