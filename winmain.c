#include <stdint.h>
#include <windows.h>

LRESULT CALLBACK WindowProcedure(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);
void PaintBitmap(int Xoffset, int Yoffset);
struct my_rect GetRect(HWND Window);

struct my_rect {
    int Width;
    int Height;
};

void *BitmapMemory;
BITMAPINFO BitmapInfo;
int BitmapHeight;
int BitmapWidth;

int Global_Running = 1;

// Entry point
int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int CmdShow)
{
    WNDCLASS WindowClass = {0};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = WindowProcedure;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "WinMainClass";

    if (RegisterClass(&WindowClass))
    {
        HWND Window = CreateWindowEx(0, WindowClass.lpszClassName, "WindowMain", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                                     CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0);

        if (Window)
        {
            ShowWindow(Window, SW_SHOWDEFAULT);

            int Xoffset = 0;

            while (Global_Running)
            {
                MSG Message;
                while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                struct my_rect rect = GetRect(Window);

                HDC DeviceContext = GetDC(Window);
                PaintBitmap(Xoffset, 0);
                StretchDIBits(DeviceContext, 0, 0, rect.Width, rect.Height, 0, 0, BitmapWidth, BitmapHeight, BitmapMemory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
                ReleaseDC(Window, DeviceContext);
                Xoffset++;
            }
        }
    }
    return 0;
}

// Pass this to the WindowClass
LRESULT CALLBACK WindowProcedure(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    switch (Message) {
    case WM_SIZE:
    {
        if (BitmapMemory)
        {
            VirtualFree(BitmapMemory, 0, MEM_RELEASE);
        }

        struct my_rect rect = GetRect(Window);

        BitmapHeight = 720;
        BitmapWidth = 1280;

        BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
        BitmapInfo.bmiHeader.biWidth = BitmapWidth;
        BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
        BitmapInfo.bmiHeader.biPlanes = 1;
        BitmapInfo.bmiHeader.biBitCount = 32;
        BitmapInfo.bmiHeader.biCompression = BI_RGB;
        
        BitmapMemory = VirtualAlloc(0, BitmapWidth * BitmapHeight * 4, MEM_COMMIT, PAGE_READWRITE);
    }
    break;

    case WM_PAINT: 
    {
        PAINTSTRUCT Paint;
        HDC DeviceContext = BeginPaint(Window, &Paint);

        struct my_rect rect = GetRect(Window);

        StretchDIBits(DeviceContext, 0, 0, rect.Width, rect.Height, 0, 0, BitmapWidth, BitmapHeight, BitmapMemory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
        EndPaint(Window, &Paint);
    }
    break;
    default:
        break;
    }

    return DefWindowProc(Window, Message, WParam, LParam);  
}

void PaintBitmap(int Xoffset, int Yoffset)
{
    uint32_t *Pixel = (uint32_t *)BitmapMemory;

    for (int Y = 0; Y < BitmapHeight; Y++)
    {
        for (int X = 0; X < BitmapWidth; X++)
        {
            uint8_t Red = (uint8_t) X + Xoffset;
            uint8_t Green = 0;
            uint8_t Blue = (uint8_t) Y + Yoffset;

            *Pixel = Blue | Green << 8 | Red << 16;
            Pixel++;
        }
    }
}

struct my_rect GetRect(HWND Window)
{
    struct my_rect Rect;
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Rect.Width = ClientRect.right;
    Rect.Height = ClientRect.bottom;
    return Rect;
}


