#include "GameApp.h"
#include "DXTrace.h"

GameApp::GameApp(HINSTANCE hInstance)
    : D3DApp(hInstance)
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
    if (!D3DApp::Init())
        return false;

    return true;
}

void GameApp::OnResize()
{
    D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt)
{

}

void GameApp::DrawScene()
{
    assert(m_pImmediateContext);
    assert(m_pSwapChain);
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //red,green,blue,alpha
    m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), ClearColor);
    m_pSwapChain->Present(0, 0);

    HR(m_pSwapChain->Present(0, 0));
}
