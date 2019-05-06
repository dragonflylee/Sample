#pragma once

class CShader
{
public:
    struct ConstantBuffer
    {
        DirectX::XMMATRIX World;
        DirectX::XMMATRIX View;
        DirectX::XMMATRIX Projection;
    };

    static const D3D11_INPUT_ELEMENT_DESC inputLayout[3];

    HRESULT Init(ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dContext);

    void Resize(const D3D11_VIEWPORT& vp);

    void XM_CALLCONV Update(ID3D11DeviceContext *d3dContext, DirectX::CXMMATRIX world);

    HRESULT Compile(ID3D11Device *d3dDevice, UINT nID, LPCTSTR szType = RT_RCDATA);

private:
    CComPtr<ID3D11InputLayout> pVertexLayout;      // 顶点输入布局
    CComPtr<ID3D11Buffer> pConstantBuffer;         // 常量缓冲区

    CComPtr<ID3D11VertexShader> pVertexShader;     // 顶点着色器
    CComPtr<ID3D11PixelShader> pPixelShader;       // 像素着色器
    CComPtr<ID3D11SamplerState> pSamplerLinear;
    // 观察矩阵和透视投影矩阵
    DirectX::XMFLOAT4X4 mView;
    DirectX::XMFLOAT4X4 mProjection;
    DirectX::XMFLOAT4X4 deviceToView;
};

