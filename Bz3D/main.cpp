#include <Windows.h>
#include "WindowsMessageMap.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    //static WindowsMessageMap mm;
    //OutputDebugString(mm(msg, lParam, wParam).c_str());
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return S_OK;
}

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // 注册窗口类
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;  //指定消息处理函数
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "Bz3DWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_APPLICATION);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // 创建窗口
    RECT rc = { 0, 0, 640, 480 };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hWND = CreateWindow("Bz3DWindowClass", 
        "Bz3D",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 
        CW_USEDEFAULT, 
        rc.right - rc.left, 
        rc.bottom - rc.top, 
        NULL, 
        NULL, 
        hInstance,
        NULL);

    // 显示窗口
    ShowWindow(hWND, nCmdShow);
    return S_OK;
}

int WinMain(HINSTANCE hInstance, 
    HINSTANCE hPrevInstance, 
    LPSTR lpCmdLine, 
    int nShowCmd)
{
    InitWindow(hInstance, nShowCmd);
    
    //消息循环
    MSG msg{ 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return static_cast<int>(msg.wParam);
}
