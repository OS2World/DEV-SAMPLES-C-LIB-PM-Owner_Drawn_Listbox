# Tools used:
#   Compile: GNU C
#   Tools: emximp (Import Library Generator)
#   Make: nmake or GNU make

all : owndraw.dll owndraw.lib

# Rule to link the DLL
owndraw.dll : owndraw.obj owndraw.def
	gcc -Zomf -Zdll owndraw.obj owndraw.def -o owndraw.dll

# Rule to compile the C source
owndraw.obj : owndraw.c owndraw.h
	gcc -Wall -Zomf -c -O2 owndraw.c -o owndraw.obj

# Rule to automatically generate the .lib import library
owndraw.lib : owndraw.def
	emximp -o owndraw.lib owndraw.def

clean :
	rm -f *.dll *.lib *.obj *.res
