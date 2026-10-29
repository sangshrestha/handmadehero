#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <xinput.h>
#include <dsound.h>

LRESULT CALLBACK WindowProcedure(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);
void InitDirectSound(HWND Window, int32_t SamplesPerSec, int32_t BufferSize);
void PaintBitmap(int Xoffset, int Yoffset);
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(direct_sound_create);

struct my_rect GetRect(HWND Window);

struct my_rect {
  int Width;
  int Height;
};

struct my_bitmap {
  void *Memory;
  BITMAPINFO Info;
  int Height;
  int Width;
};

int global_running = 1;
int Xoffset = 0;
int Yoffset = 0;
struct my_bitmap global_bitmap;

// Sound stuff
#define SamplesPerSecond 48000
#define ToneHz 256
#define SquareWavePeriod (SamplesPerSecond / ToneHz)
#define HalfSquareWavePeriod (SquareWavePeriod / 2)
#define BytesPerSample (sizeof(int16_t)*2)
#define SecondaryBufferSize (SamplesPerSecond * BytesPerSample)
int16_t ToneVolume = 2000;
uint32_t RunningSampleIndex = 0;
LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;
int SoundIsPlaying = 0;

struct my_rect GetRect(HWND Window)
{
  struct my_rect Rect;
  RECT ClientRect;
  GetClientRect(Window, &ClientRect);
  Rect.Width = ClientRect.right;
  Rect.Height = ClientRect.bottom;
  return Rect;
}

// Entry point
int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int CmdShow)
{
  WNDCLASS WindowClass = {0};
  WindowClass.style = CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = WindowProcedure;
  WindowClass.hInstance = Instance;
  WindowClass.lpszClassName = "WinMainClass";

  LARGE_INTEGER PerfCountFrequencyResult;
  QueryPerformanceFrequency(&PerfCountFrequencyResult);
  uint64_t PerfCountFrequency = PerfCountFrequencyResult.QuadPart;

  if (RegisterClass(&WindowClass))
  {
    HWND Window = CreateWindowEx(0, WindowClass.lpszClassName, "WindowMain", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0);

    if (Window)
    {
      ShowWindow(Window, SW_SHOWDEFAULT);

      if (global_bitmap.Memory)
      {
        VirtualFree(global_bitmap.Memory, 0, MEM_RELEASE);
      }

      InitDirectSound(Window, SamplesPerSecond, SecondaryBufferSize);

      if (!SoundIsPlaying)
      {
        GlobalSecondaryBuffer->lpVtbl->Play(GlobalSecondaryBuffer, 0, 0, DSBPLAY_LOOPING);
        SoundIsPlaying = 1;
      }

      global_bitmap.Height = 720;
      global_bitmap.Width = 1280;

      global_bitmap.Info.bmiHeader.biSize = sizeof(global_bitmap.Info.bmiHeader);
      global_bitmap.Info.bmiHeader.biWidth = global_bitmap.Width;
      global_bitmap.Info.bmiHeader.biHeight = -global_bitmap.Height;
      global_bitmap.Info.bmiHeader.biPlanes = 1;
      global_bitmap.Info.bmiHeader.biBitCount = 32;
      global_bitmap.Info.bmiHeader.biCompression = BI_RGB;
      
      global_bitmap.Memory = VirtualAlloc(0, global_bitmap.Width * global_bitmap.Height * 4, MEM_COMMIT, PAGE_READWRITE);

      // performance counter
      LARGE_INTEGER LastCounter;
      QueryPerformanceCounter(&LastCounter);

      while (global_running)
      {

        MSG Message;

        while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
        {
          TranslateMessage(&Message);
          DispatchMessage(&Message);
        }

        for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
        {
          XINPUT_STATE Controller_State;

          if (XInputGetState(i, &Controller_State) == ERROR_SUCCESS)
          {
            int DPadLeft = Controller_State.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;

            if (DPadLeft) 
            {
              Xoffset++;
            }
          }
          else 
          {
          }
        }
        struct my_rect Rect = GetRect(Window);

        // Sound
        DWORD PlayCursor;
        DWORD WriteCursor;

        if (SUCCEEDED(GlobalSecondaryBuffer->lpVtbl->GetCurrentPosition(GlobalSecondaryBuffer, &PlayCursor, &WriteCursor)))
        {
          DWORD ByteToLock = RunningSampleIndex * BytesPerSample % SecondaryBufferSize;
          DWORD BytesToWrite;

          if (ByteToLock == PlayCursor) 
          {
            if (!SoundIsPlaying)
            {
              BytesToWrite = SecondaryBufferSize;
            }
            else
            {
              BytesToWrite = 0;
            }
          }
          else if (ByteToLock > PlayCursor)
          {
            BytesToWrite = SecondaryBufferSize - ByteToLock;
            BytesToWrite += PlayCursor;
          }
          else
          {
            BytesToWrite = PlayCursor - ByteToLock;
          }

          VOID *Region1;
          DWORD Region1Size;
          VOID *Region2;
          DWORD Region2Size;

          if (SUCCEEDED(GlobalSecondaryBuffer->lpVtbl->Lock(GlobalSecondaryBuffer, ByteToLock, BytesToWrite, &Region1, &Region1Size, &Region2, &Region2Size, 0)))
          {
            int16_t *SampleOut = (int16_t *)Region1;
            DWORD Region1SampleCount = Region1Size/BytesPerSample;
            DWORD Region2SampleCount = Region2Size/BytesPerSample;

            for(DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)
            {
              int16_t SampleValue = ((RunningSampleIndex++ / HalfSquareWavePeriod) % 2) ? ToneVolume : -ToneVolume;
              *SampleOut++ = SampleValue;
              *SampleOut++ = SampleValue;
            }

            SampleOut = (int16_t *)Region2;
            for(DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)
            {
              int16_t SampleValue = ((RunningSampleIndex++ / HalfSquareWavePeriod) % 2) ? ToneVolume : -ToneVolume;
              *SampleOut++ = SampleValue;
              *SampleOut++ = SampleValue;
            }

            GlobalSecondaryBuffer->lpVtbl->Unlock(GlobalSecondaryBuffer, Region1, Region1Size, Region2, Region2Size);
          }

          
        }

        HDC DeviceContext = GetDC(Window);
        PaintBitmap(Xoffset, Yoffset);
        StretchDIBits(DeviceContext, 0, 0, Rect.Width, Rect.Height, 0, 0, global_bitmap.Width, global_bitmap.Height, global_bitmap.Memory, &global_bitmap.Info, DIB_RGB_COLORS, SRCCOPY);
        ReleaseDC(Window, DeviceContext);


        // perf counter end
        LARGE_INTEGER EndCounter;
        QueryPerformanceCounter(&EndCounter);

        uint64_t CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
        uint32_t MSPerFrame = (1000 * CounterElapsed) / PerfCountFrequency;

        char Buffer[256];
        printf("%dms\n", MSPerFrame);

        LastCounter = EndCounter;
      }
    }
  }
  return 0;
}

// Pass this to the WindowClass
LRESULT CALLBACK WindowProcedure(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
  switch (Message) {
  case WM_CLOSE:
  {
    global_running = 0;
    PostQuitMessage(0);
  } 
  break;
  case WM_DESTROY:
  {
    global_running = 0;
    PostQuitMessage(0);
  } 
  break;
  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
  case WM_KEYDOWN:
  case WM_KEYUP:
  {
    if (WParam == 'W')
    {
      Yoffset+=2;
    }
    else if (WParam == 'S')
    {
      Yoffset-=2;
    }
    else if (WParam == 'A')
    {
      Xoffset-=2;
    }
    else if (WParam == 'D')
    {
      Xoffset+=2;
    }
  }
  break;
  case WM_PAINT: 
  {
    PAINTSTRUCT Paint;
    HDC DeviceContext = BeginPaint(Window, &Paint);

    struct my_rect Rect = GetRect(Window);

    StretchDIBits(DeviceContext, 0, 0, Rect.Width, Rect.Height, 0, 0, global_bitmap.Width, global_bitmap.Height, global_bitmap.Memory, &global_bitmap.Info, DIB_RGB_COLORS, SRCCOPY);
    EndPaint(Window, &Paint);
  }
  break;
  default:
    return DefWindowProc(Window, Message, WParam, LParam);  
    break;
  }

}

void PaintBitmap(int Xoffset, int Yoffset)
{
  uint32_t *Pixel = (uint32_t *)global_bitmap.Memory;

  for (int Y = 0; Y < global_bitmap.Height; Y++)
  {
    for (int X = 0; X < global_bitmap.Width; X++)
    {
      uint8_t Red = (uint8_t) X + Xoffset;
      uint8_t Blue = 0;
      uint8_t Green = (uint8_t) Y + Yoffset;

      *Pixel = Blue | Green << 8 | Red << 16;
      Pixel++;
    }
  }
}

void InitDirectSound(HWND Window, int32_t SamplesPerSec, int32_t BufferSize) 
{
  HMODULE DirectSoundLibrary = LoadLibraryA("dsound.dll");

  if (DirectSoundLibrary)
  {
    direct_sound_create *DirectSoundCreate = (direct_sound_create *) GetProcAddress(DirectSoundLibrary, "DirectSoundCreate");

    LPDIRECTSOUND DirectSound;
    if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))) {
      WAVEFORMATEX WaveFormat;
      WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
      WaveFormat.nChannels = 2;
      WaveFormat.wBitsPerSample = 16;
      WaveFormat.nSamplesPerSec = SamplesPerSecond;
      WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample)/8;
      WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
      WaveFormat.cbSize = 0;

      if (SUCCEEDED(DirectSound->lpVtbl->SetCooperativeLevel(DirectSound, Window, DSSCL_PRIORITY))) 
      {
        DSBUFFERDESC BufferDescription = {0};
        BufferDescription.dwSize = sizeof(BufferDescription);
        BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

        LPDIRECTSOUNDBUFFER PrimaryBuffer;

        if (SUCCEEDED(DirectSound->lpVtbl->CreateSoundBuffer(DirectSound, &BufferDescription, &PrimaryBuffer, 0)))
        {
          if (SUCCEEDED(PrimaryBuffer->lpVtbl->SetFormat(PrimaryBuffer, &WaveFormat)))
          {
            printf("primary buffer format set\n");
          }
        }
      }

      DSBUFFERDESC BufferDescription = {0};
      BufferDescription.dwSize = sizeof(BufferDescription);
      BufferDescription.dwBufferBytes = BufferSize;
      BufferDescription.lpwfxFormat = &WaveFormat;


      if (SUCCEEDED(DirectSound->lpVtbl->CreateSoundBuffer(DirectSound, &BufferDescription, &GlobalSecondaryBuffer, 0)))
      {
         printf("secondary buffer set\n");
      }
    }
  }
}


