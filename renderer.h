#ifndef RENDERER_H
#define RENDERER_H

#include "win32OffscreenBuffer.h"
#include "color.h"

void Renderer_UpdateAndRender(Win32_offscreen_buffer* buffer);
void Renderer_DrawRect(Win32_offscreen_buffer* buffer, 
              int MinX, int MinY, int MaxX, int MaxY, 
		       uint32_t Color);
#endif
