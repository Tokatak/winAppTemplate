#include <windows.h>

typedef struct {
  BITMAPINFO Info;
  void* Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
} Win32_offscreen_buffer;


Win32_offscreen_buffer globalBackbuffer;

char headerName[] = "RENAME ME";

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void Win32ResizeDIBSection(Win32_offscreen_buffer* buffer, int width, int height);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		   PSTR szCmdLine, int iCmdShow)
{
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


  // update size if needed
  Win32ResizeDIBSection(&globalBackbuffer, 800, 600);

  ShowWindow(hwnd, iCmdShow);
  UpdateWindow(hwnd);
  while (GetMessage(&msg, NULL, 0, 0))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  return msg.wParam;

}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  HDC hdc;
  switch (message)
    {
    case WM_PAINT:
      PAINTSTRUCT ps;
      hdc = BeginPaint(hwnd, &ps);
      
      FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

      // Importnat: avoid billiniear filtration
      SetStretchBltMode(hdc, COLORONCOLOR);

      StretchDIBits(hdc,
		      0, 0, globalBackbuffer.Width, globalBackbuffer.Height,
		      0, 0, globalBackbuffer.Width, globalBackbuffer.Height,
		      globalBackbuffer.Memory,
		      &globalBackbuffer.Info,
		      DIB_RGB_COLORS, SRCCOPY);
      
      EndPaint(hwnd, &ps);
      break;

    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    }

  return DefWindowProc(hwnd, message, wParam, lParam);
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
    // todo: probably clear to black?
}
