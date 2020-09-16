.SUFFIXES: .cc .bc .js .d .jcd
CFLAGS=-I. -I../src -I../RavelCAPI -I$(HOME)/usr/include --std=c++11 --bind -I$(HOME)/usr/emscripten/include
LIBS=-L$(HOME)/usr/emscripten/lib -ljson_spirit

# different emscripten options here
#CFLAGS+=-s ALLOW_MEMORY_GROWTH=1
CFLAGS+=-s TOTAL_MEMORY=67108864
# 0 here means emit catch blocks, 1 don't emit catch blocks, 2 selectively emit catch blocks
CFLAGS+=-s DISABLE_EXCEPTION_CATCHING=0

WASM=1
# en/disable wasm
CFLAGS+=-s WASM=$(WASM)

# JSRavel's API
#CFLAGS+=-s EXPORTED_FUNCTIONS="['Module.RavelCanvasDataCube']"

# the separated memory init file seems to cause a problem with missing
# constructors
ifdef DEBUG
OPT=-g -s ASSERTIONS=1
else
OPT=-O3  --memory-init-file 0 -DNDEBUG
endif

RAVELSRC=$(wildcard ../src/*.cc)
RAVELOBJS=$(RAVELSRC:%.cc=%.bc)
JSRAVELOBJS=cairoShimHTMLCanvas.bc
JSMODULES=jsravel.js
$(warning $(RAVELOBJS))

WEBINSTALLROOT=public_html/ravelation
WEBINSTALL=$(WEBINSTALLROOT)/examples

VPATH=../src ../RavelCAPI

all: $(JSMODULES)

$(JSMODULES): %.js: %.bc $(JSRAVELOBJS) $(RAVELOBJS)
	em++ $(CFLAGS) $(OPT) $(LIBS) $^ -o $@

.cc.bc:
	em++ $(CFLAGS) $(OPT) -c $< -o $@

.cc.d: 
	em++ $(CFLAGS) -MM -MG $< >$@
	sed -i -e 's/\.o:/.bc:/' $@

.h.jcd:
	classdesc -nodef -onbase -typeName -respect_private json_pack json_unpack <$< >$@

ifneq ($(MAKECMDGOALS),clean)
-include $(RAVELOBJS:.bc=.d) $(JSRAVELOBJS:.bc=.d) $(JSMODULES:.js=.d) 
endif


clean:
	-rm -f $(JSMODULES) *.bc *.mem *.d $(RAVELOBJS)

install-slickgrid:
	cd SlickGrid; ncftpput -F -m -S .tmp -f ../hpcoders.conf $(WEBINSTALL)/SlickGrid slick.core.* slick.grid.*
	cd SlickGrid/lib; ncftpput -F -m -S .tmp -f ../../hpcoders.conf $(WEBINSTALL)/SlickGrid/lib jquery-1.11.2.min.js jquery.event.drag-2.3.0.js
	cd SlickGrid; ncftpput -F -m -S .tmp -f ../hpcoders.conf -R $(WEBINSTALL)/SlickGrid css

electron: all
	rm -rf electron-ravel electron-ravel.zip
	mkdir -p electron-ravel
	cp -r ravel1D.html package.json main.js desktop.js ravel.js jsravel.js ravel-1dplotly.js plotly.min.js electron-ravel
	zip -r electron-ravel.zip electron-ravel
