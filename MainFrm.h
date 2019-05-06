#pragma once

#include "Model.h"
#include "Shader.h"

class CMaimFrm : public CWindowImpl<CMaimFrm, CWindow, CFrameWinTraits>
{
public:
    DECLARE_WND_CLASS_EX(TEXT("SampleWnd"), 0, COLOR_WINDOWFRAME);

    BEGIN_MSG_MAP(CMainFrm)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnOpen)
    END_MSG_MAP()

    CMaimFrm();

    /**
    * 窗口事件相关
    */
    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnOpen(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    /**
    * DirectX 3D 相关
    */
    HRESULT OnResize(int width, int height);
    LRESULT OnRender();

protected:
    CComPtr<ID3D11Device> d3dDevice;
    CComPtr<ID3D11DeviceContext> d3dContext;
    CComPtr<IDXGISwapChain> dxSwapChain;
    CComPtr<ID3D11RenderTargetView> pBackRTV;
    CComPtr<ID3D11DepthStencilView> pDepthSV;

    CModel::Collection pModel;
    CShader pShader;
};
