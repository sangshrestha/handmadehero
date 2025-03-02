#include <windows.h>

LRESULT CALLBACK WindowProcedure(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int CmdShow)
{
    WNDCLASS WindowClass = {0};
    WindowClass.lpfnWndProc = WindowProcedure;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "WinMainClass";

    if (RegisterClass(&WindowClass))
    {

        HWND WindowHandle =
            CreateWindowEx(0, WindowClass.lpszClassName, "WindowMain", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                           CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0);

        if (WindowHandle)
        {
            ShowWindow(WindowHandle, SW_SHOWDEFAULT);

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

LRESULT CALLBACK WindowProcedure(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    switch (Message)
    {
    case WM_PAINT: {
        PAINTSTRUCT Paint;
        HDC DeviceContext = BeginPaint(Window, &Paint);

        int X = Paint.rcPaint.left;
        int Y = Paint.rcPaint.top;
        int Width = Paint.rcPaint.right - X;
        int Height = Paint.rcPaint.bottom - Y;

        PatBlt(DeviceContext, X, Y, Width, Height, BLACKNESS);
        EndPaint(Window, &Paint);
    }
    break;
    default:
        break;
    }
    return DefWindowProc(Window, Message, WParam, LParam);
}
