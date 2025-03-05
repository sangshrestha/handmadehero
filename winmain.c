#include <stdint.h>
#include <windows.h>

LRESULT CALLBACK WindowProcedure(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);

// Entry point
int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int CmdShow)
{
    WNDCLASS WindowClass = {0};
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

            MSG Message;
            while (GetMessage(&Message, 0, 0, 0) > 0)
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            }
        }
    }
    return 0;
}

// Pass this to the WindowClass
LRESULT CALLBACK WindowProcedure(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    switch (Message) {
    case WM_PAINT: 
    {
        PAINTSTRUCT Paint;
        HDC DeviceContext = BeginPaint(Window, &Paint);

        int X = Paint.rcPaint.left;
        int Y = Paint.rcPaint.top;
        int Width = Paint.rcPaint.right - X;
        int Height = Paint.rcPaint.bottom - Y;

        RECT ClientRect;
        GetClientRect(Window, &ClientRect);

        BITMAPINFO BitmapInfo;
        BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
        BitmapInfo.bmiHeader.biWidth = Width;
        BitmapInfo.bmiHeader.biHeight = Height;
        BitmapInfo.bmiHeader.biPlanes = 1;
        BitmapInfo.bmiHeader.biBitCount = 32;
        BitmapInfo.bmiHeader.biCompression = BI_RGB;

        void *BitmapMemory;
        BitmapMemory = VirtualAlloc(0, Width * Height * 4, MEM_COMMIT, PAGE_READWRITE);
        uint32_t *Pixel = (uint32_t *)BitmapMemory;

        for (int i = 0; i < Width * Height; i++)
        {
            uint8_t Red = 102;
            uint8_t Green = 255;
            uint8_t Blue = 178;
            *Pixel = Blue | Green << 8 | Red << 16;
            Pixel++;
        }

        StretchDIBits(DeviceContext, X, Y, Width, Height, X, Y, Width, Height, BitmapMemory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
        EndPaint(Window, &Paint);
    }
    break;
    default:
        break;
    }
    return DefWindowProc(Window, Message, WParam, LParam);  
}
