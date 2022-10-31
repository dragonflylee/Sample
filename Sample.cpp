#include "Sample.h"

CComModule _Module;

int Run(int nCmdShow)
{
    CMainFrm wndMain;
    HWND hWnd = wndMain.Create(HWND_DESKTOP);
    if (NULL == hWnd) return ::GetLastError();

    ::ShowWindow(hWnd, nCmdShow);
    ::UpdateWindow(hWnd);

    // 主消息循环:
    MSG msg;
    for (;;)
    {
        if (!::GetMessage(&msg, NULL, 0, 0))
        {
            return (int)msg.wParam;   // WM_QUIT
        }
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);

        wndMain.OnRender();
    }
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine,
    int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    HRESULT hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    ATLASSERT(SUCCEEDED(hr));

    hr = _Module.Init(NULL, hInstance);
    ATLASSERT(SUCCEEDED(hr));

    int nRet = Run(nCmdShow);

    _Module.Term();
    ::CoUninitialize();
    return nRet;
}