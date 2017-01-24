.SUFFIXES: .cc .bc .js .d
CFLAGS=-I. -I../src --std=c++11 --bind
#-s ALLOW_MEMORY_GROWTH=1
RAVELSRC=$(wildcard ../src/*.cc)
RAVELOBJS=$(RAVELSRC:%.cc=%.bc)
JSRAVELOBJS=cairoShimHTMLCanvas.bc
JSMODULES=jsravel.js
$(warning $(RAVELOBJS))

all: $(JSMODULES)

$(JSMODULES): %.js: %.bc $(JSRAVELOBJS) $(RAVELOBJS)
	em++ $(CFLAGS) -O1 $^ -o $@

.cc.bc:
	em++ $(CFLAGS) -O1 -c $< -o $@

.cc.d: 
	em++ $(CFLAGS) -MM -MG $< >$@
	sed -i -e 's/\.o:/.bc:/' $@

ifneq ($(MAKECMDGOALS),clean)
-include $(RAVELOBJS:.bc=.d) $(JSRAVELOBJS:.bc=.d) $(JSMODULES:.js=.d) 
endif


clean:
	-rm -f $(JSMODULES) *.bc *.mem *.d
