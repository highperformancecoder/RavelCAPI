# root directory for ecolab include files and libraries
ifeq ($(shell ls $(HOME)/usr/ecolab/include/ecolab.h),$(HOME)/usr/ecolab/include/ecolab.h)
ECOLAB_HOME=$(HOME)/usr/ecolab
else
ECOLAB_HOME=/usr/local/ecolab
endif

include $(ECOLAB_HOME)/include/Makefile

ACTIONS+=xml_pack xml_unpack random_init
FLAGS+=-std=c++11 #-Wno-error=offsetof

VPATH+=src src/shims src/tcl
FLAGS+=-I. -Isrc -Isrc/tcl

# object files making up libravel
#OBJS=src/ravel.o src/dataCube.o src/ravelCairo.o src/shims/cairoShimCairo.o \
#	src/filterCairo.o src/splitMerge.o src/sortedVector.o
OBJS=ravel.o dataCube.o ravelCairo.o cairoShimCairo.o \
	filterCairo.o splitMerge.o sortedVector.o
LIBS+=libravel.a
LIBS:=-L$(HOME)/usr/lib64 $(LIBS)
MODELS=ravelTest

DLLS=libgcc_s_dw2-1.dll libstdc++-6.dll libcairo-2.dll libgobject-2.0-0.dll libgsl-0.dll libpango-1.0-0.dll libpangocairo-1.0-0.dll tcl85.dll tk85.dll libz-1.dll libpixman-1-0.dll libpng15-15.dll libglib-2.0-0.dll libintl-8.dll libiconv-2.dll libffi-6.dll libgslcblas-0.dll libgmodule-2.0-0.dll libpangowin32-1.0-0.dll Tktable211.dll

ifdef AEGIS
aegis-all: all
	cd test; $(MAKE)
endif

#chmod command is to counteract AEGIS removing execute privelege from scripts
all: $(MODELS) Ravel/Installer/ravelDoc.wxi src/ravelVersion.h
	-$(CHMOD) a+x *.tcl

ifeq ($(OS),Darwin)
all: $(MODELS:=.app)
endif

# we want to build this target always when under AEGIS, otherwise only
# when non-existing
ifdef AEGIS
.PHONY: src/ravelVersion.h
endif

src/ravelVersion.h:
	rm -f $@
ifdef AEGIS
	echo '#define RAVEL_VERSION "'$(version)'"' >$@
else
	echo '#define RAVEL_VERSION "unknown"' >$@
endif

# This rule uses a header file of object descriptors
$(MODELS:=.o): %.o: %.cc 
#	$(CPLUSPLUS) -c $(FLAGS)  $<

# how to build a model executable
$(MODELS): %: %.o libravel.a
	$(LINK) $(FLAGS) $*.o $(MODLINK) $(LIBS) -o $@

#make MacOS application bundles
$(MODELS:=.app): %.app: %
	$(ECOLAB_HOME)/utils/mkmacapp $<

libravel.a: $(OBJS)
	ar r $@ $^

ifneq ($(MAKECMDGOALS),clean)
include $(MODELS:=.d) 
include $(OBJS:.o=.d)
endif

clean:
	$(BASIC_CLEAN)
	cd src; $(BASIC_CLEAN)
	cd src/shims; $(BASIC_CLEAN)
	cd src/tcl; $(BASIC_CLEAN)
	cd test; $(BASIC_CLEAN)
	rm -f $(MODELS) *.cd
	rm -rf *.app

win-dist: ravelTest
	cp $(DLLS:%=/bin/%) .
	mkdir -p library tcl8.5.11/library
	-cp -rf /usr/local/lib/tcl8.5/* /usr/local/lib/tk8.5/* tcl8.5.11/library
	cp $(HOME)/usr/ecolab/include/*.tcl library
	cp -rf $(HOME)/usr/ecolab/include/Xecolab library

sure: all
	cd test; $(MAKE)
	sh runTests.sh

Ravel/Installer/ravelDoc.wxi: doc doc/ravelDoc.tex 
	cd doc && sh makeDoc.sh && sh createRavelDocWXI.sh
