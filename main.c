#pragma comment(lib, "gdi32.lib") // for StretchDIBits
#pragma comment(lib, "winmm.lib") // for timeBeginPeriod

#include <windows.h>
#include <stdint.h>
#include <stdio.h> // _sprintf_s

typedef struct {
  BITMAPINFO Info;
  void* Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
} Win32_offscreen_buffer;


static Win32_offscreen_buffer globalBackbuffer;
static int64_t GlobalPerfCountFrequency;

char headerName[] = "RENAME ME";
boolean globalRunning;
int bufferWidth = 800;
int bufferHeight = 600;

float samples[120];
int sampleIndex;
int sampleMax = 120;

static inline LARGE_INTEGER
Win32GetWallClock(void)
{
  LARGE_INTEGER Result;
  QueryPerformanceCounter(&Result);
  return (Result);
}

static inline float
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
  float Result = ((float)(End.QuadPart - Start.QuadPart) / (float)GlobalPerfCountFrequency);
  return (Result);
}


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void Win32ResizeDIBSection(Win32_offscreen_buffer* buffer, int width, int height);
void UpdateAndRender(Win32_offscreen_buffer* buffer);
void WndDisplayBufferInWindow(Win32_offscreen_buffer* buffer, HDC deviceContext);
void WndProcessPendingMessages();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		   PSTR szCmdLine, int iCmdShow)
{
  LARGE_INTEGER PerfCounterFrequencyResult; 
  QueryPerformanceFrequency(&PerfCounterFrequencyResult);
  GlobalPerfCountFrequency = PerfCounterFrequencyResult.QuadPart;

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

  // update size if needed
  Win32ResizeDIBSection(&globalBackbuffer, bufferWidth, bufferHeight);

  ShowWindow(hwnd, iCmdShow);

  if(!hwnd)
    return 0;

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

      
      UpdateAndRender(&globalBackbuffer);


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
      WndDisplayBufferInWindow(&globalBackbuffer, DeviceContext);
      ReleaseDC(hwnd, DeviceContext);

      // todo: consider cycles per frame
      // todo: lower frame logs 
      // int32_t MCyclesPerFrame = (int32_t)(CyclesElapsed /( 1000 * 1000));
      int64_t CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;


      char FPSBuffer[256];
      _snprintf_s(FPSBuffer, sizeof (FPSBuffer), _TRUNCATE ,"work:%.3f \t fitIn:%.3f \t  maxFPS:%.3f \t currentFPS:%.3f\n",
		  WorkSecondsElapsed, SecondsElapsedForFrame,
		  1/WorkSecondsElapsed, 1/SecondsElapsedForFrame);
      OutputDebugStringA(FPSBuffer);

      samples[sampleIndex] = WorkSecondsElapsed * 1000;
      sampleIndex++;
      sampleIndex = sampleIndex%sampleMax;
    }
  
  return(0);  
}

void DrawRect(Win32_offscreen_buffer* buffer, 
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

// counter for ouput check
unsigned char tick =0;
void UpdateAndRender(Win32_offscreen_buffer* buffer){

  // update
  tick = (tick+1) %256;

  int MinX = 0, MinY=0;
  int MaxX = buffer->Width;
  int MaxY = buffer->Height;

  // render
  uint32_t Color = ((tick << 16) |
		    (tick << 8) |
		    (tick << 0));

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


  int bufferW = bufferWidth;
  int bufferH = bufferHeight;
  int barCount = sampleMax;
  int pxPerBar = bufferW / barCount;
  int cursorHeight = 100;
  int cursorWidth = 1;
  
  int yMsOffset200fps = 5;
  int yMsOffset100fps = 10;
  int yMsOffset66fps = 15;
  

  // todo validate hScale
  int hScale = 4;

  // todo cleanup
  // move to separate
  uint32_t rectColor = ((255 << 16) |
		    (0 << 8) |
		    (0 << 0));

  uint32_t cursorColor = ((255 << 16) |
			(255 << 8) |
			(0 << 0));

  uint32_t hlineColor = ~Color;

  float barWidth = 1;
  float barHeight = 10;
  
  // cursor
  DrawRect( buffer,
	    sampleIndex*pxPerBar, bufferHeight - (cursorHeight*hScale),
	    sampleIndex*pxPerBar + cursorWidth, bufferHeight,
	    cursorColor);

  // samples
  for( int i =0; i< sampleMax ; i++){
    DrawRect( buffer,
	      i*pxPerBar, bufferHeight - (samples[i]*hScale),
	      i*pxPerBar + pxPerBar, bufferHeight,
	      rectColor);
  }


  // horizontal refs 200 fps
  DrawRect( buffer,
	    0,       bufferHeight - (yMsOffset200fps*hScale)-barWidth,
	    bufferW, bufferHeight - (yMsOffset200fps*hScale),
	    hlineColor);

  // horizontal refs 100 fps
  DrawRect( buffer,
	    0,       bufferHeight - (yMsOffset100fps*hScale)-barWidth,
	    bufferW, bufferHeight - (yMsOffset100fps*hScale),
	    hlineColor);

  // horizontal refs 66 fps
    DrawRect( buffer,
	    0,       bufferHeight - (yMsOffset66fps*hScale)-barWidth,
	    bufferW, bufferHeight - (yMsOffset66fps*hScale),
	    hlineColor);
  
  // todo min, max avg
}

void  WndDisplayBufferInWindow(Win32_offscreen_buffer* buffer, HDC deviceContext){
  StretchDIBits(deviceContext,
		0, 0, buffer->Width, buffer->Height,
		0, 0, buffer->Width, buffer->Height,
		buffer->Memory,
		&buffer->Info,
		DIB_RGB_COLORS, SRCCOPY);
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
     
      WndDisplayBufferInWindow(&globalBackbuffer, hdc);

      EndPaint(hwnd, &ps);
      break;

    case WM_DESTROY:
      globalRunning = FALSE;
      PostQuitMessage(0);
      return 0;
    default:
      {
	// tringA("default\n");
	Result = DefWindowProcA(hwnd, message, wParam, lParam);
      }
      break;
    }

  return (Result);
}

void
Win32ResizeDIBSection(Win32_offscreen_buffer* buffer, int width, int height) {
  if (buffer->Memory) {
    VirtualFree(buffer->Memory, 0, MEM_RELEASE);
  }

  buffer->Width = width;
  buffer->Height = height;

  int BytesPerPixel = 4;
  buffer->BytesPerPixel = BytesPerPixel;

  buffer->Info.bmiHeader.biSize = sizeof(buffer->Info.bmiHeader);
  buffer->Info.bmiHeader.biWidth = buffer->Width;
  buffer->Info.bmiHeader.biHeight = -buffer->Height;
  buffer->Info.bmiHeader.biPlanes = 1;
  buffer->Info.bmiHeader.biBitCount = 32;
  buffer->Info.bmiHeader.biCompression = BI_RGB;

  int BitmapMemorySize = (buffer->Width * buffer->Height) * BytesPerPixel;
  buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  buffer->Pitch = width * BytesPerPixel;
}
