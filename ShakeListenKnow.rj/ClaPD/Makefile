current:
	echo make pd_linux, pd_nt, pd_irix5, pd_irix6, or pd_darwin

clean: ; rm -f *.pd_linux *.dll *.o *.pd_darwin [!freeverb]*~
# ----------------------- NT -----------------------

pd_nt: reson~.dll noiseq~.dll twopz~.dll cosc.dll ppl.dll

.SUFFIXES: .obj .dll

PDNTCFLAGS = /W3 /WX /DNT /DPD /nologo
VC="C:\Program Files\Microsoft Visual Studio .NET\Vc7"
DI="E:\leevi\pd\work"

PDNTINCLUDE = /I. /I\tcl\include /I..\..\src /I$(VC)\include

PDNTLDIR = $(VC)\lib
PDNTLIB = $(PDNTLDIR)\libc.lib \
	$(PDNTLDIR)\oldnames.lib \
	$(PDNTLDIR)\kernel32.lib \
	$(DI)\pd.lib 

.c.dll:
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c
	link /dll /export:$*_setup $*.obj $(PDNTLIB)

# override explicitly for tilde objects like this:
reson~.dll: reson~.c; 
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c
	link /dll /export:reson_tilde_setup $*.obj $(PDNTLIB)
noiseq~.dll: noiseq~.c; 
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c
	link /dll /export:noiseq_tilde_setup $*.obj $(PDNTLIB)
twopz~.dll: twopz~.c; 
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c
	link /dll /export:twopz_tilde_setup $*.obj $(PDNTLIB)

# ----------------------- IRIX 5.x -----------------------

pd_irix5:  reson~.pd_irix5 noiseq~.pd_irix5 \
    twopz~.pd_irix5 cosc.pd_irix5 ppl.pd_irix5

.SUFFIXES: .pd_irix5

SGICFLAGS5 = -o32 -DPD -DUNIX -DIRIX -O2


SGIINCLUDE =  -I../../src/

.c.pd_irix5:
	cc $(SGICFLAGS5) $(SGIINCLUDE) -o $*.o -c $*.c
	ld -elf -shared -rdata_shared -o $*.pd_irix5 $*.o
	rm $*.o

# ----------------------- LINUX i386 -----------------------

pd_linux: reson~.pd_linux noiseq~.pd_linux twopz~.pd_linux  cosc.pd_linux ppl.pd_linux 


.SUFFIXES: .pd_linux

LINUXCFLAGS = -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch

LINUXINCLUDE = -I /usr/lib/pd/include

.c.pd_linux:
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c
	ld -export_dynamic  -shared -o $*.pd_linux $*.o -lc -lm
	strip --strip-unneeded $*.pd_linux
	rm $*.o

# ----------------------- Mac OSX -----------------------

pd_darwin:  reson~.pd_darwin noiseq~.pd_darwin \
     twopz~.pd_darwin  cosc.pd_darwin ppl.pd_darwin

.SUFFIXES: .pd_darwin

DARWINCFLAGS = -DPD -O2 -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch

DARWININCLUDE = -I/Users/cerkut/pd-SVN/pd/src

.c.pd_darwin:
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -o $*.o -c $*.c
	cc -bundle -undefined suppress -flat_namespace -o $*.pd_darwin $*.o 
	rm -f $*.o

