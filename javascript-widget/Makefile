.SUFFIXES: .cc .bc .js
CFLAGS=-I. -I../src --std=c++11 --bind
#-s ALLOW_MEMORY_GROWTH=1
RAVELSRC=$(wildcard ../src/*.cc)
RAVELOBJS=$(RAVELSRC:%.cc=%.bc)
$(warning $(RAVELOBJS))

all: jsravel.js test.js

jsravel.js: jsravel.bc $(RAVELOBJS)
	em++ $(CFLAGS) -O1 $^ -o $@

test.js: testCS.bc cairoShimHTMLCanvas.bc $(RAVELOBJS)
	em++ $(CFLAGS) -O1 $^ -o $@

.cc.bc:
	em++ $(CFLAGS) -O1 -c $< -o $@

clean:
	rm jsravel.js *.bc *.mem
