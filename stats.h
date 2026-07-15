#ifndef STATS_H
#define STATS_H

#include <string.h> //memcpy
#include <stdio.h> //_snprintf_s
#include "win32OffscreenBuffer.h"
#include "color.h"
#include "renderer.h"


void Stats_Sample(float workMsElapsed);
void Stats_Render(Win32_offscreen_buffer* buffer);
const char* Stats_Message();

#endif
