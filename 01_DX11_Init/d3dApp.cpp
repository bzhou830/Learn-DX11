#include "d3dApp.h"
#include "DXTrace.h"
#include <sstream>

namespace
{
    // This is just used to forward Windows messages from a global window
    // procedure to our member function window procedure because we cannot
    // assign a member function to WNDCLASS::lpfnWndProc.
    D3DApp* g_pd3dApp = nullptr;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Forward hwnd on because we can get messages (e.g., WM_CREATE)
    // before CreateWindow returns, and thus before m_hMainWnd is valid.
    return g_pd3dApp->MsgProc(hwnd, msg, wParam, lParam);
}

D3DApp::D3DApp(HINSTANCE hInstance)
    : m_hAppInst(hInstance),
    m_MainWndCaption(L"DirectX11"),
    m_ClientWidth(800),
    m_ClientHeight(600),
    m_hMainWnd(nullptr),
    m_AppPaused(false),
    m_Minimized(false),
    m_Maximized(false),
    m_Resizing(false),
    m_Enable4xMsaa(true),
    m_4xMsaaQuality(0),
    m_pDevice(nullptr),
    m_pImmediateContext(nullptr),
    m_pSwapChain(nullptr),
    m_pRenderTargetView(nullptr)
{
    ZeroMemory(&m_ScreenViewport, sizeof(D3D11_VIEWPORT));


    // 让一个全局指针获取这个类，这样我们就可以在Windows消息处理的回调函数
    // 让这个类调用内部的回调函数了
    g_pd3dApp = this;
}

D3DApp::~D3DApp()
{
    // 恢复所有默认设定
    if (m_pImmediateContext)
        m_pImmediateContext->ClearState();
}

HINSTANCE D3DApp::AppInst()const
{
    return m_hAppInst;
}

HWND D3DApp::MainWnd()const
{
    return m_hMainWnd;
}

float D3DApp::AspectRatio()const
{
    return static_cast<float>(m_ClientWidth) / m_ClientHeight;
}

int D3DApp::Run()
{
    MSG msg = { 0 };

    m_Timer.Reset();

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            m_Timer.Tick();

            if (!m_AppPaused)
            {
                CalculateFrameStats();
                UpdateScene(m_Timer.DeltaTime());
                DrawScene();
            }
            else
            {
                Sleep(100);
            }
        }
    }

    return (int)msg.wParam;
}

bool D3DApp::Init()
{
    if (!InitMainWindow())
        return false;

    if (!InitDirect3D())
        return false;

    return true;
}

void D3DApp::OnResize()
{
    return;
}

LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        // WM_ACTIVATE is sent when the window is activated or deactivated.  
        // We pause the game when the window is deactivated and unpause it 
        // when it becomes active.  
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            m_AppPaused = true;
            m_Timer.Stop();
        }
        else
        {
            m_AppPaused = false;
            m_Timer.Start();
        }
        return 0;

        // WM_SIZE is sent when the user resizes the window.  
    case WM_SIZE:
        // Save the new client area dimensions.
        m_ClientWidth = LOWORD(lParam);
        m_ClientHeight = HIWORD(lParam);
        if (m_pDevice)
        {
            if (wParam == SIZE_MINIMIZED)
            {
                m_AppPaused = true;
                m_Minimized = true;
                m_Maximized = false;
            }
            else if (wParam == SIZE_MAXIMIZED)
            {
                m_AppPaused = false;
                m_Minimized = false;
                m_Maximized = true;
                OnResize();
            }
            else if (wParam == SIZE_RESTORED)
            {

                // Restoring from minimized state?
                if (m_Minimized)
                {
                    m_AppPaused = false;
                    m_Minimized = false;
                    OnResize();
                }

                // Restoring from maximized state?
                else if (m_Maximized)
                {
                    m_AppPaused = false;
                    m_Maximized = false;
                    OnResize();
                }
                else if (m_Resizing)
                {
                    // If user is dragging the resize bars, we do not resize 
                    // the buffers here because as the user continuously 
                    // drags the resize bars, a stream of WM_SIZE messages are
                    // sent to the window, and it would be pointless (and slow)
                    // to resize for each WM_SIZE message received from dragging
                    // the resize bars.  So instead, we reset after the user is 
                    // done resizing the window and releases the resize bars, which 
                    // sends a WM_EXITSIZEMOVE message.
                }
                else // API call such as SetWindowPos or m_pSwapChain->SetFullscreenState.
                {
                    OnResize();
                }
            }
        }
        return 0;

        // WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
    case WM_ENTERSIZEMOVE:
        m_AppPaused = true;
        m_Resizing = true;
        m_Timer.Stop();
        return 0;

        // WM_EXITSIZEMOVE is sent when the user releases the resize bars.
        // Here we reset everything based on the new window dimensions.
    case WM_EXITSIZEMOVE:
        m_AppPaused = false;
        m_Resizing = false;
        m_Timer.Start();
        OnResize();
        return 0;

        // WM_DESTROY is sent when the window is being destroyed.
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

        // The WM_MENUCHAR message is sent when a menu is active and the user presses 
        // a key that does not correspond to any mnemonic or accelerator key. 
    case WM_MENUCHAR:
        // Don't beep when we alt-enter.
        return MAKELRESULT(0, MNC_CLOSE);

        // Catch this message so to prevent the window from becoming too small.
    case WM_GETMINMAXINFO:
        ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
        ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
        return 0;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        return 0;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        return 0;
    case WM_MOUSEMOVE:
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}


bool D3DApp::InitMainWindow()
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hAppInst;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = L"D3DWndClassName";

    if (!RegisterClass(&wc))
    {
        MessageBox(0, L"RegisterClass Failed.", 0, 0);
        return false;
    }

    // Compute window rectangle dimensions based on requested client area dimensions.
    RECT R = { 0, 0, m_ClientWidth, m_ClientHeight };
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
    int width = R.right - R.left;
    int height = R.bottom - R.top;

    m_hMainWnd = CreateWindow(L"D3DWndClassName", m_MainWndCaption.c_str(),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_hAppInst, 0);
    if (!m_hMainWnd)
    {
        MessageBox(0, L"CreateWindow Failed.", 0, 0);
        return false;
    }

    ShowWindow(m_hMainWnd, SW_SHOW);
    UpdateWindow(m_hMainWnd);

    return true;
}

bool D3DApp::InitDirect3D()
{
    HRESULT hr = S_OK;
    // 创建D3D设备 和 D3D设备上下文
    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = m_ClientWidth;
    sd.BufferDesc.Height = m_ClientHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hMainWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    D3D_FEATURE_LEVEL featureLevel;
    D3D_DRIVER_TYPE d3dDriverType;
    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        d3dDriverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(NULL, d3dDriverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
            D3D11_SDK_VERSION, &sd, &m_pSwapChain, m_pDevice.GetAddressOf(), &featureLevel, m_pImmediateContext.GetAddressOf());
        if (SUCCEEDED(hr))
            break;
    }
    if (FAILED(hr))
    {
        MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
        return false;
    }
    // Create a render target view
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr))
        return hr;

    hr = m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, m_pRenderTargetView.GetAddressOf());
    pBackBuffer->Release();
    if (FAILED(hr))
        return hr;

    m_pImmediateContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), NULL);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)m_ClientWidth;
    vp.Height = (FLOAT)m_ClientHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_pImmediateContext->RSSetViewports(1, &vp);
    return true;
}

void D3DApp::CalculateFrameStats()
{
    // 该代码计算每秒帧速，并计算每一帧渲染需要的时间，显示在窗口标题
    static int frameCnt = 0;
    static float timeElapsed = 0.0f;

    frameCnt++;

    if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
    {
        float fps = (float)frameCnt; // fps = frameCnt / 1
        float mspf = 1000.0f / fps;

        std::wostringstream outs;
        outs.precision(6);
        outs << m_MainWndCaption << L"    "
            << L"FPS: " << fps << L"    "
            << L"Frame Time: " << mspf << L" (ms)";
        SetWindowText(m_hMainWnd, outs.str().c_str());

        // Reset for next average.
        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}


