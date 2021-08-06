#pragma once
#include <windows.h>

class window
{
public:
    window(HINSTANCE hInstance, int nCmdShow);
    ~window();
    int Run();
private:
    HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
};

