#ifndef STATS_H
#define STATS_H

#include "win32OffscreenBuffer.h"
#include "color.h"
#include "renderer.h"


void STATS_SAMPLE(float workMsElapsed);
void STATS_RENDER(Win32_offscreen_buffer* buffer);
char* STATS_MESSAGE();

#endif
