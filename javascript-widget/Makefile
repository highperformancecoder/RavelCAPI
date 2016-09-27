.SUFFIXES: .cc .bc .js
CFLAGS=-I. -I../src --std=c++11 --bind
RAVELSRC=$(wildcard ../src/*.cc)
RAVELOBJS=$(RAVELSRC:%.cc=%.bc)
$(warning $(RAVELOBJS))

jsravel.js: jsravel.bc $(RAVELOBJS)
	em++ $(CFLAGS) $^ -o $@

.cc.bc:
	em++ $(CFLAGS) -c $< -o $@

clean:
	rm jsravel.js *.bc
