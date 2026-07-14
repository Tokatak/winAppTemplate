#include "renderer.h"

// counter for ouput check
unsigned char tick =0;

void  Renderer_UpdateAndRender(Win32_offscreen_buffer* buffer){

  // update
  tick = (tick+1) %256;

  int MinX = 0, MinY=0;
  // todo cleanup
  int MaxX = buffer->Width;
  int bufferWidth = buffer->Width;
  
  int MaxY = buffer->Height;
  int bufferHeight = buffer->Height;

  // render
  uint32_t Color = COLOR_RGB(tick, tick, tick);

  uint8_t *Row = ((uint8_t *)buffer->Memory +
                  MinX*buffer->BytesPerPixel +
                  MinY*buffer->Pitch);
  for(int Y = MinY;
      Y < MaxY;
      ++Y)
    {
      uint32_t *Pixel = (uint32_t *)Row;
      for(int X = MinX;
	  X < MaxX;
	  ++X)
        {            
	  *Pixel++ = Color;
        }
        
      Row += buffer->Pitch;
    }
}

void Renderer_DrawRect(Win32_offscreen_buffer* buffer, 
              int MinX, int MinY, int MaxX, int MaxY, 
              uint32_t Color)
{
    // Clamp to buffer bounds
    if (MinX < 0) MinX = 0;
    if (MinY < 0) MinY = 0;
    if (MaxX > buffer->Width) MaxX = buffer->Width;
    if (MaxY > buffer->Height) MaxY = buffer->Height;
    
    // If rectangle is invalid or outside buffer, return
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
