CFLAGS = -Wall -Wextra -Wpedantic -g -O0 -mwindows
LDFLAGS = -lgdi32 -lwinmm

main.exe: main.c 
	gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)

msvc:
	cl /Zi /DEBUG /EHsc main.c user32.lib gdi32.lib
