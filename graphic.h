#pragma once

#include <vector>

/*
* 图形渲染
*/
class CGraphic
{
public:
    HRESULT Init(HWND hWnd, int nWidth, int nHeight);

    HRESULT OnResize(int nWidth, int nHeight);

    LRESULT OnRender(HWND hWnd);

    LRESULT OnOpen(HWND hWnd);

    void OnMouseMove(int x, int y, int key);

    void OnMouseWheel(float delta);

    inline ID3D11Device * operator ->() { return pDevice.Get(); }

protected:
    ComPtr<ID3D11Device> pDevice;
    ComPtr<ID3D11DeviceContext> pContext;
    ComPtr<IDXGISwapChain> pSwapChain;
    ComPtr<ID3D11RenderTargetView> pBackRTV;
    ComPtr<ID3D11DepthStencilView> pDepthSV;

    CShader pShader;
    CModel::Collection pModel;
};

