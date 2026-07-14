#ifndef COLOR_H
#define COLOR_H

#define COLOR_RGB(r,g,b) ((uint32_t)((r)<<16 | (g)<<8 | (b)))
#define COLOR_RED   COLOR_RGB(255,0,0)
#define COLOR_YELLOW COLOR_RGB(255,255,0)
#define COLOR_WHITE COLOR_RGB(255,255,255)

#endif
