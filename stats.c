#include "stats.h"

static int sampleCount = 120;
static float samples[120];
static int sampleIndex;
static char sampleFormatBuffer[256];

static int FPS_200_MS = 5;
static int FPS_100_MS = 10;
static int FPS_66_MS = 15;

void SortSamples(float* samples, int count) {
  // Simple bubble sort for small arrays
  for (int i = 0; i < count - 1; i++) {
    for (int j = 0; j < count - i - 1; j++) {
      if (samples[j] > samples[j + 1]) {
	float temp = samples[j];
	samples[j] = samples[j + 1];
	samples[j + 1] = temp;
      }
    }
  }
}

void CalculatePercentiles(float* samples, int count, char* outBuffer, int outBufferSize) {
  if (count == 0) return;
    
  // todo:? note 120!
  float sorted[120];
  memcpy(sorted, samples, count * sizeof(float));
  SortSamples(sorted, count);
    
  float p50 = sorted[count * 50 / 100];  
  float p95 = sorted[count * 95 / 100];
  float p99 = sorted[count * 99 / 100];
  float max = sorted[count - 1];
    
  _snprintf_s(outBuffer, outBufferSize, _TRUNCATE,
	      "Median: %.2fms | P95: %.2fms | P99: %.2fms | Max: %.2fms",
	      p50, p95, p99, max);
}


void Stats_Sample(float workMsElapsed){
  samples[sampleIndex] = workMsElapsed;
  sampleIndex++;
  sampleIndex = sampleIndex%sampleCount;
}

const char* Stats_Message(){
  return sampleFormatBuffer;
}

void Stats_Render(Win32_offscreen_buffer* buffer){
  int bufferWidth = buffer->Width;
  int bufferHeight = buffer->Height;
  uint32_t Color = COLOR_RGB(120,120,120);
  

  
  int barCount = sampleCount;
  int pxPerBar = bufferWidth / barCount;
  int cursorHeight = FPS_66_MS;
  int cursorWidth = 1;
  int barHeight = 1;
 
  int hScale = bufferWidth / sampleCount;
  if(hScale < 0) hScale = 1;

  int offsetX =0 ;
  if( bufferWidth > sampleCount * pxPerBar){
    offsetX =  (bufferWidth - (sampleCount * pxPerBar) )/2;
  }
  
  
  uint32_t hlineColor = ~Color;

  // samples
  for( int i =0; i< sampleCount ; i++){
    Renderer_DrawRect( buffer,
		       offsetX + i*pxPerBar, bufferHeight - (samples[i]*hScale),
		       offsetX + i*pxPerBar + pxPerBar, bufferHeight,
		       COLOR_RED);
  }
  
  // horizontal refs 200 fps
  Renderer_DrawRect( buffer,
		     offsetX + 0,       bufferHeight - (FPS_200_MS * hScale)-barHeight,
		     -offsetX + bufferWidth, bufferHeight - (FPS_200_MS * hScale),
		     hlineColor);

  // horizontal refs 100 fps
  Renderer_DrawRect( buffer,
		     offsetX + 0,       bufferHeight - (FPS_100_MS * hScale)-barHeight,
		     -offsetX + bufferWidth, bufferHeight - (FPS_100_MS * hScale),
		     hlineColor);

  // horizontal refs 66 fps
  Renderer_DrawRect( buffer,
		     offsetX + 0,       bufferHeight - (FPS_66_MS * hScale)-barHeight,
		     -offsetX + bufferWidth, bufferHeight - (FPS_66_MS * hScale),
		     hlineColor);
  

  // cursor
  Renderer_DrawRect( buffer,
		     offsetX + sampleIndex*pxPerBar, bufferHeight - (cursorHeight*hScale),
		     offsetX + sampleIndex*pxPerBar + cursorWidth, bufferHeight,
		     COLOR_YELLOW);


  CalculatePercentiles(samples, sampleCount, sampleFormatBuffer, sizeof(sampleFormatBuffer)); 
}
