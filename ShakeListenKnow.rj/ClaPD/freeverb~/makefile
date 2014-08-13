NAME=freeverb~
CSYM=freeverb_tilde

current: 
	echo make pd_linux, pd_nt, pd_darwin
# ----------------------- NT -----------------------

pd_nt: $(NAME).dll

.SUFFIXES: .dll

PDNTCFLAGS = /W3 /WX /O2 /G6 /DNT /DPD /nologo /D_LANGUAGE_C_PLUS_PLUS
VC="C:\Program Files\Microsoft Visual Studio .NET\Vc7"

PDNTINCLUDE = /I. /Ic:\pd\tcl\include /Ic:\pd\src /I$(VC)\include /Iinclude

PDNTLDIR = $(VC)\Lib
PDNTLIB = $(PDNTLDIR)\libc.lib \
	$(PDNTLDIR)\oldnames.lib \
	$(PDNTLDIR)\kernel32.lib \
	$(PDNTLDIR)\user32.lib \
	$(PDNTLDIR)\uuid.lib \
	e:\leevi\pd\bin\pd.lib \

.cpp.dll:
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.cpp
	link /dll /export:$(CSYM)_setup $*.obj $(PDNTLIB)

# ----------------------- LINUX i386 -----------------------

pd_linux: $(NAME).pd_linux

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DPD -DUNIX -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch

LINUXINCLUDE =  -I /usr/include

.cpp.pd_linux:
	g++ $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.cpp
	g++ -export_dynamic  -shared -o $*.pd_linux $*.o -lc -lm -L /usr/lib
	strip --strip-unneeded $*.pd_linux
	rm -f $*.o ../$*.pd_linux
	ln -s $*/$*.pd_linux ..

# ----------------------- MAX OS X (Pd) -------------------

pd_darwin: $(NAME).pd_darwin

.SUFFIXES: .pd_darwin

DARWINCFLAGS = -DPD -DUNIX -DMACOSX -O2 \
    -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch

DARWININCLUDE =  -I../../src 

.cpp.pd_darwin:
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -o $*.o -c $*.cpp
	cc -bundle -undefined suppress -flat_namespace -o $*.pd_darwin $*.o
	rm -f $*.o ../$*.pd_darwin
	ln -s $*/$*.pd_darwin ..

# ----------------------------------------------------------

install:
	cp help-*.pd ../../doc/5.reference

clean:
	rm -f *.o *.pd_* so_locations
