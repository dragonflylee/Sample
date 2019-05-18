#include "Sample.h"
#include "Sample_i.c"

CAppModule _Module;

int Run(int nCmdShow)
{
    CMainFrm wndMain;
    HWND hWnd = wndMain.CreateEx(HWND_DESKTOP);
    if (NULL == hWnd) return ::GetLastError();
    // 主消息循环:
    wndMain.ShowWindow(nCmdShow);
    return wndMain.Run();
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

    hr = _Module.Init(NULL, hInstance, &LIBID_SampleLib);
    ATLASSERT(SUCCEEDED(hr));

    int nRet = Run(nCmdShow);

    _Module.Term();
    ::CoUninitialize();
    return nRet;
}