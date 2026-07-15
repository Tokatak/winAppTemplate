#include "renderer.h"

// counter for ouput check
unsigned char tick =0;

void  Renderer_UpdateAndRender(Win32_offscreen_buffer* buffer){

  // update
  tick = (tick+1) %256;

  // render
  const uint32_t Color = COLOR_RGB(tick, tick, tick);
  Renderer_DrawRect(buffer, 0, 0, buffer->Width, buffer->Height, Color);
}

void Renderer_DrawRect(Win32_offscreen_buffer* buffer, 
              int MinX, int MinY, int MaxX, int MaxY, 
              uint32_t Color)
{
    if (MinX < 0) MinX = 0;
    if (MinY < 0) MinY = 0;
    if (MaxX > buffer->Width) MaxX = buffer->Width;
    if (MaxY > buffer->Height) MaxY = buffer->Height;
    
    if (MinX >= MaxX || MinY >= MaxY) return;
    
    uint8_t *Row = ((uint8_t *)buffer->Memory +
                    MinX * buffer->BytesPerPixel +
                    MinY * buffer->Pitch);
    
    for (int Y = MinY; Y < MaxY; ++Y)
    {
        uint32_t *Pixel = (uint32_t *)Row;
        for (int X = MinX; X < MaxX; ++X)
        {
            *Pixel++ = Color;
        }
        Row += buffer->Pitch;
    }
}
