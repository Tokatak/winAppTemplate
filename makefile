CFLAGS = -Wall -Wextra -Wpedantic -g -O0 -mwindows
LDFLAGS = -lgdi32 -lwinmm

win.exe: main.c
	gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)
