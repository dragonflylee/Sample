#include "Demo.h"
#include <windowsx.h>

CMainWnd::CMainWnd() : hWnd(nullptr), uTimer(1)
{
}

HWND CMainWnd::Create(HINSTANCE hInst, HWND hParent)
{
    static WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
    wc.lpszClassName = TEXT("DemoWnd");

    if (!::GetClassInfoEx(hInst, wc.lpszClassName, &wc))
    {
        wc.hInstance = hInst;
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = CMainWnd::WndProc;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
        if (!::RegisterClassEx(&wc)) return nullptr;
    }

    return ::CreateWindowEx(0, wc.lpszClassName, TEXT("Demo"), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 800, 600, hParent, nullptr, hInst, this);
}

LRESULT CMainWnd::OnCreate(LPCREATESTRUCT pParam)
{
    HRESULT hr = S_OK;
    
    HR_CHECK(pGraphic.Init(hWnd, pParam->cx, pParam->cy));

    uTimer = ::SetTimer(hWnd, uTimer, 100, nullptr);
exit:
    // 返回 -1 表示窗口创建失败
    return SUCCEEDED(hr) ? 0 : -1;
}

LRESULT CMainWnd::OnTimer(WPARAM wParam)
{
    HRESULT hr = pGraphic->GetDeviceRemovedReason();
    switch (hr)
    {
    case S_OK:
        break;
    case DXGI_ERROR_DEVICE_HUNG:
    case DXGI_ERROR_DEVICE_RESET:
        ::KillTimer(hWnd, wParam);
        return pGraphic.Init(hWnd, 0, 0);
    }
    return pGraphic.OnRender(hWnd);
}

LRESULT CALLBACK CMainWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (WM_CREATE == uMsg)
    {
        LPCREATESTRUCT pParam = reinterpret_cast<LPCREATESTRUCT>(lParam);
        CMainWnd *pT = reinterpret_cast<CMainWnd *>(pParam->lpCreateParams);
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pT));
        pT->hWnd = hWnd;
        return pT->OnCreate(pParam);
    }

    CMainWnd *pT = reinterpret_cast<CMainWnd *>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    switch (uMsg)
    {
    case WM_SIZE:
        return pT->pGraphic.OnResize(LOWORD(lParam), HIWORD(lParam));
    case WM_TIMER:
        return pT->OnTimer(wParam);
    case WM_PAINT:
        pT->pGraphic.OnRender(hWnd);
        break;
    case WM_LBUTTONDOWN:
    case WM_MOUSEMOVE:
        if (wParam > 0)
        {
            pT->pGraphic.OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
        }
        break;
    case WM_MOUSEWHEEL:
        pT->pGraphic.OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam) < 0 ? 0.9f : 1.1f);
        break;
    case WM_RBUTTONDOWN:
        pT->pGraphic.OnOpen(hWnd);
        break;
    case WM_DESTROY:
        ::KillTimer(hWnd, pT->uTimer);
        PostQuitMessage(0);
        return S_OK;
    }
    return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}