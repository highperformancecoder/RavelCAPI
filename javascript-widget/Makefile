.SUFFIXES: .cc .bc .js .d
CFLAGS=-I. -I../src --std=c++11 --bind -I$(HOME)/usr/emscripten/include
LIBS=-L$(HOME)/usr/emscripten/lib

# different emscripten options here
#CFLAGS+=-s ALLOW_MEMORY_GROWTH=1
CFLAGS+=-s TOTAL_MEMORY=67108864
# 0 here means emit catch blocks, 1 don't emit catch blocks, 2 selectively emit catch blocks
CFLAGS+=-s DISABLE_EXCEPTION_CATCHING=0

# the separated memory init file seems to cause a problem with missing
# constructors
OPT=-O3  --memory-init-file 0 -DNDEBUG

RAVELSRC=$(wildcard ../src/*.cc)
RAVELOBJS=$(RAVELSRC:%.cc=%.bc)
JSRAVELOBJS=cairoShimHTMLCanvas.bc
JSMODULES=jsravel.js
$(warning $(RAVELOBJS))

WEBINSTALLROOT=public_html/ravelation
WEBINSTALL=$(WEBINSTALLROOT)/examples

all: $(JSMODULES)

$(JSMODULES): %.js: %.bc $(JSRAVELOBJS) $(RAVELOBJS)
	em++ $(CFLAGS) $(OPT) $^ -o $@

.cc.bc:
	em++ $(CFLAGS) $(OPT) -c $< -o $@

.cc.d: 
	em++ $(CFLAGS) -MM -MG $< >$@
	sed -i -e 's/\.o:/.bc:/' $@

ifneq ($(MAKECMDGOALS),clean)
-include $(RAVELOBJS:.bc=.d) $(JSRAVELOBJS:.bc=.d) $(JSMODULES:.js=.d) 
endif


clean:
	-rm -f $(JSMODULES) *.bc *.mem *.d

install-slickgrid:
	cd SlickGrid; ncftpput -F -m -S .tmp -f ../hpcoders.conf $(WEBINSTALL)/SlickGrid slick.core.* slick.grid.*
	cd SlickGrid/lib; ncftpput -F -m -S .tmp -f ../../hpcoders.conf $(WEBINSTALL)/SlickGrid/lib jquery-1.11.2.min.js jquery.event.drag-2.3.0.js
	cd SlickGrid; ncftpput -F -m -S .tmp -f ../hpcoders.conf -R $(WEBINSTALL)/SlickGrid css

electron: all
	rm -rf electron-ravel electron-ravel.zip
	mkdir -p electron-ravel
	cp -r ravel1D.html package.json main.js desktop.js ravel.js jsravel.js ravel-1dplotly.js plotly.min.js electron-ravel
	zip -r electron-ravel.zip electron-ravel
