#include <stdint.h> // _t typese
#include <windows.h> //BITMAPINFO

/*********/
// #define RENDERER_EXTERNAL
// to exclude dummy system side placeholder
/*********/

#ifndef OFFSCREEN_BUFFER_H
#define OFFSCREEN_BUFFER_H
typedef struct {
  void* Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
} OffscreenBuffer;
#endif

#ifndef COLOR_H
#define COLOR_H
#define COLOR_RGB(r,g,b) ((uint32_t)((r)<<16 | (g)<<8 | (b)))
#define COLOR_RED   COLOR_RGB(255,0,0)
#define COLOR_YELLOW COLOR_RGB(255,255,0)
#define COLOR_WHITE COLOR_RGB(255,255,255)
#endif


#ifndef RENDERER_H
#define RENDERER_H
void Renderer_UpdateAndRender(OffscreenBuffer* buffer);
void Renderer_DrawRect(OffscreenBuffer* buffer,
		       int MinX, int MinY, int MaxX, int MaxY,
		       uint32_t Color);
#endif

#ifndef STATS_H
#define STATS_H
void Stats_Sample(float workMsElapsed);
void Stats_Render(OffscreenBuffer* buffer);
const char* Stats_Message();
#endif

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void Win32ResizeDIBSection(OffscreenBuffer* buffer, BITMAPINFO* info, int width, int height);
void WndDisplayBufferInWindow(OffscreenBuffer* buffer, BITMAPINFO* info, HDC deviceContext);
void WndProcessPendingMessages();

