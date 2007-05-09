all: test64.dll


test64.dll: cpuid.c plugin.c
	i386-mingw32msvc-gcc -Wl,--file-alignment,512 -Os cpuid.c plugin.c -shared -o test64.dll
	i386-mingw32msvc-strip test64.dll
