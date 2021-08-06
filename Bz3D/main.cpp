#include <Windows.h>
#include "window.h"


int WinMain(HINSTANCE hInstance, 
    HINSTANCE hPrevInstance, 
    LPSTR lpCmdLine, 
    int nShowCmd)
{
    window app{ hInstance, nShowCmd };
    return app.Run();
}
