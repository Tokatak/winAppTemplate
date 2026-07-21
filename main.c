#pragma comment(lib, "gdi32.lib") // for StretchDIBits
#pragma comment(lib, "winmm.lib") // for timeBeginPeriod

#include <stdio.h> // _sprintf_s

#include "main.h"

static OffscreenBuffer globalBackbuffer;
static BITMAPINFO globalBufferInfo;
static int64_t globalPerfCountFrequency;

char headerName[] = "RENAME ME";
boolean globalRunning;

static inline float
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
  float Result = ((float)(End.QuadPart - Start.QuadPart) / (float)globalPerfCountFrequency);
  return (Result);
}

static inline LARGE_INTEGER
Win32GetWallClock(void)
{
  LARGE_INTEGER Result;
  QueryPerformanceCounter(&Result);
  return (Result);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		   PSTR szCmdLine, int iCmdShow)
{
  LARGE_INTEGER PerfCounterFrequencyResult; 
  QueryPerformanceFrequency(&PerfCounterFrequencyResult);
 globalPerfCountFrequency = PerfCounterFrequencyResult.QuadPart;

  HWND hwnd;
  MSG msg;
  WNDCLASS wndclass;
  wndclass.style = CS_HREDRAW | CS_VREDRAW;
  wndclass.lpfnWndProc = WndProc;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = 0;
  wndclass.hInstance = hInstance;
  wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  wndclass.lpszMenuName = NULL;
  wndclass.lpszClassName = headerName;

  if (!RegisterClass(&wndclass))
    {
      MessageBox(NULL, TEXT("This program requires Windows NT!"),
		 headerName, MB_ICONERROR);
      return 0;
    }

  hwnd = CreateWindow(headerName,                 // window class name
		      TEXT(headerName),           // window caption
		      WS_OVERLAPPEDWINDOW,        // window style
		      CW_USEDEFAULT,              // initial x position
		      CW_USEDEFAULT,              // initial y position
		      CW_USEDEFAULT,              // initial x size
		      CW_USEDEFAULT,              // initial y size
		      NULL,                       // parent window handle
		      NULL,                       // window menu handle
		      hInstance,                  // program instance handle
		      NULL);                      // creation parameters

  UINT DesiredSchedularMS = 1;
  int32_t SleepIsGranular =( timeBeginPeriod(DesiredSchedularMS) == TIMERR_NOERROR );
  LARGE_INTEGER LastCounter = Win32GetWallClock();
  uint64_t LastCycleCount = __rdtsc();

  // update size if needed
  Win32ResizeDIBSection(&globalBackbuffer, &globalBufferInfo, 640, 480);

  ShowWindow(hwnd, iCmdShow);

  if(!hwnd)
    return -1;

  // timings
  int MonitorRefreshHz = 60;
  HDC RefreshDC = GetDC(hwnd);
  int Win32RefreshRate = GetDeviceCaps(RefreshDC,VREFRESH);
  ReleaseDC(hwnd,RefreshDC);
  if(Win32RefreshRate >1){
    MonitorRefreshHz = Win32RefreshRate;
  }
  float GameUpdateHz = ( MonitorRefreshHz/2.0f );
  float TargetSecondsPerFrame = 1.0f / (float)GameUpdateHz;

  globalRunning = TRUE;

  while (globalRunning)
    {
      WndProcessPendingMessages();

      Renderer_UpdateAndRender(&globalBackbuffer);

      LARGE_INTEGER WorkCounter = Win32GetWallClock();
      float WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);

      float SecondsElapsedForFrame = WorkSecondsElapsed;
      if (SecondsElapsedForFrame < TargetSecondsPerFrame)
	{	

	  if (SleepIsGranular)
	    {
	      DWORD SleepMS = (DWORD)(1000.f * (TargetSecondsPerFrame - SecondsElapsedForFrame));
	      if (SleepMS > 0)
		{
		  Sleep(SleepMS);
		}
	    }

	  float TestSecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
	  if (TestSecondsElapsedForFrame < TargetSecondsPerFrame)
	    {
	      /// log here
	    }

	  while (SecondsElapsedForFrame < TargetSecondsPerFrame)
	    {
	      SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
	    }
	}
      else
	{
	  //  skip frame
	}

      LARGE_INTEGER EndCounter = Win32GetWallClock();
      float MSPerFrame = 1000.0f * Win32GetSecondsElapsed(LastCounter, EndCounter);
      LastCounter = EndCounter;
	
      HDC DeviceContext = GetDC(hwnd);

      Stats_Render(&globalBackbuffer);
      
      WndDisplayBufferInWindow(&globalBackbuffer, &globalBufferInfo, DeviceContext);
      ReleaseDC(hwnd, DeviceContext);

      // todo: lower frame logs 

      uint64_t EndCycleCount = __rdtsc();

      uint64_t CyclesElapsed = EndCycleCount - LastCycleCount;
      LastCycleCount = EndCycleCount;
      int32_t MCyclesPerFrame = (int32_t)(CyclesElapsed /(1000 * 1000));

      double FPS = 0.0;
      double MCPF = ((double)CyclesElapsed / (1000.0f *1000.0f));

      char FPSBuffer[256];
      _snprintf_s(FPSBuffer, sizeof (FPSBuffer), _TRUNCATE ,
		  "work:%.3f \t fitIn:%.3f \t  maxFPS:%.3f \t currentFPS:%.3f\t MCPF:%.3f\n",
		  WorkSecondsElapsed, SecondsElapsedForFrame,
		  1/WorkSecondsElapsed, 1/SecondsElapsedForFrame,
		  MCPF);
      OutputDebugStringA(FPSBuffer);

      Stats_Sample( WorkSecondsElapsed * 1000);
    }
  
  return(0);  
}


void  WndDisplayBufferInWindow(OffscreenBuffer* buffer, BITMAPINFO* bufferInfo,  HDC deviceContext){
  StretchDIBits(deviceContext,
		0, 0, buffer->Width, buffer->Height,
		0, 0, buffer->Width, buffer->Height,
		buffer->Memory,
		bufferInfo,
		DIB_RGB_COLORS, SRCCOPY);

  const char* text = Stats_Message();
  RECT rect = {10, 10, 500, 100};
  DrawTextA(deviceContext, text, -1, &rect, DT_LEFT | DT_TOP | DT_SINGLELINE);
}

void WndProcessPendingMessages(){
  MSG Message;

  while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
    switch(Message.message){

    case WM_QUIT:{
      globalRunning = FALSE;
      return;
    }

    case WM_KEYUP: {
      uint32_t VKCode = (uint32_t)Message.wParam;
      int32_t AltKeyWasDown = ((Message.lParam & (1 << 29)) != 0);
      if ((VKCode == VK_F4) && AltKeyWasDown) {
	globalRunning = FALSE;
      }
    }
      break;

    default:{
      TranslateMessage(&Message);
      DispatchMessage( &Message);
    }
      break;
    }
  }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  HDC hdc;
  LRESULT Result = 0;
  switch (message)
    {
    case WM_CLOSE:
      globalRunning = FALSE;
      break;
      
    case WM_PAINT:
      PAINTSTRUCT ps;
      hdc = BeginPaint(hwnd, &ps);
      
      FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

      // Importnat: avoid billiniear filtration
      SetStretchBltMode(hdc, COLORONCOLOR);
     
      WndDisplayBufferInWindow(&globalBackbuffer,&globalBufferInfo, hdc);

      EndPaint(hwnd, &ps);
      break;

    case WM_DESTROY:
      globalRunning = FALSE;
      PostQuitMessage(0);
      return 0;
    default:
      {
	Result = DefWindowProcA(hwnd, message, wParam, lParam);
      }
      break;
    }

  return (Result);
}

void
Win32ResizeDIBSection(OffscreenBuffer* buffer, BITMAPINFO* info, int width, int height) {
  if (buffer->Memory) {
    VirtualFree(buffer->Memory, 0, MEM_RELEASE);
  }

  buffer->Width = width;
  buffer->Height = height;

  int BytesPerPixel = 4;
  buffer->BytesPerPixel = BytesPerPixel;

  info->bmiHeader.biSize = sizeof(info->bmiHeader);
  info->bmiHeader.biWidth = buffer->Width;
  info->bmiHeader.biHeight = -buffer->Height;
  info->bmiHeader.biPlanes = 1;
  info->bmiHeader.biBitCount = 32;
  info->bmiHeader.biCompression = BI_RGB;

  int BitmapMemorySize = (buffer->Width * buffer->Height) * BytesPerPixel;
  buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  buffer->Pitch = width * BytesPerPixel;
}



// todo: renderer move
#ifndef RENDERER_EXTERNAL
#define RENDERER_EXTERNAL

// counter for ouput check
unsigned char tick =0;

void  Renderer_UpdateAndRender(OffscreenBuffer* buffer){

  // update
  tick = (tick+1) %256;

  // render
  const uint32_t Color = COLOR_RGB(tick, tick, tick);
  Renderer_DrawRect(buffer, 0, 0, buffer->Width, buffer->Height, Color);
}

#endif

void Renderer_DrawRect(OffscreenBuffer* buffer,
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


// todo: move stats

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

void Stats_Render(OffscreenBuffer* buffer){
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

