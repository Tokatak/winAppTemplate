#ifndef WIN_32_OFFSCREEN_BUFFER_H
#define WIN_32_OFFSCREEN_BUFFER_H

#include <windows.h> //BITMAPINFO
#include <stdint.h> // _t types

typedef struct {
  BITMAPINFO Info;
  void* Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
} Win32_offscreen_buffer;


#endif

