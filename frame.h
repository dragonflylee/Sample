#pragma once

class CMainWnd
{
public:
    CMainWnd();

    /**
    * 创建窗体
    */
    HWND Create(HINSTANCE hInst, HWND hParent = HWND_DESKTOP);

protected:
    /**
    * 窗体事件
    */
    LRESULT OnCreate(LPCREATESTRUCT /*pParam*/);

    LRESULT OnTimer(WPARAM wParam);

    /**
    * 窗体消息循环
    */
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    HWND hWnd;
    UINT_PTR uTimer;
    CGraphic pGraphic;
};
