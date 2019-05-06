#include "Sample.h"
#include "Shader.h"

using namespace DirectX;

const D3D11_INPUT_ELEMENT_DESC CShader::inputLayout[3] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

HRESULT CShader::Init(ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dContext)
{
    HREFTYPE hr = S_OK;

    HR_CHECK(Compile(d3dDevice, IDR_SHADER));

    // 设置常量缓冲区描述
    D3D11_BUFFER_DESC cbd;
    ZeroMemory(&cbd, sizeof(cbd));
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.ByteWidth = sizeof(ConstantBuffer);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = 0;
    // 新建常量缓冲区，不使用初始数据
    HR_CHECK(d3dDevice->CreateBuffer(&cbd, NULL, &pConstantBuffer));

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    HR_CHECK(d3dDevice->CreateSamplerState(&sampDesc, &pSamplerLinear));

    // 设置图元类型，设定输入布局
    d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3dContext->IASetInputLayout(pVertexLayout);
    // 将着色器绑定到渲染管线
    d3dContext->VSSetShader(pVertexShader, NULL, 0);
    // 将更新好的常量缓冲区绑定到顶点着色器
    d3dContext->VSSetConstantBuffers(0, 1, &pConstantBuffer.p);
    d3dContext->PSSetShader(pPixelShader, NULL, 0);
    d3dContext->PSSetSamplers(0, 1, &pSamplerLinear.p);

exit:
    return hr;
}

void CShader::Resize(const D3D11_VIEWPORT& vp)
{
    // 初始化常量缓冲区的值
    XMMATRIX view = XMMatrixLookAtLH(         // 视图变换矩阵
        XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f),          // 摄影机坐标
        XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),            // 摄影机焦点坐标
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));           // 摄影机上朝向坐标

    XMMATRIX projection = XMMatrixPerspectiveFovLH(     // 投影矩阵
        XM_PIDIV2,                // 中心垂直弧度
        vp.Width / vp.Height,     // 宽高比
        1.0f,                     // 近平面距离
        200.0f);                  // 远平面距离

    // 将屏幕坐标点从视口变换回NDC坐标系
    static const XMVECTORF32 D = { { { -1.0f, 1.0f, 0.0f, 0.0f } } };
    XMVECTOR scale = XMVectorReciprocal(XMVectorSet(vp.Width * 0.5f, -vp.Height * 0.5f, vp.MaxDepth - vp.MinDepth, 1.0f));
    XMVECTOR offset = XMVectorMultiplyAdd(scale, XMVectorSet(-vp.TopLeftX, -vp.TopLeftY, -vp.MinDepth, 0.0f), D.v);
    XMMATRIX device = XMMatrixMultiply(XMMatrixScalingFromVector(scale), XMMatrixTranslationFromVector(offset));
    // 从NDC坐标系变换回世界坐标系
    XMMATRIX trans = XMMatrixInverse(NULL, XMMatrixMultiply(view, projection));

    XMVECTOR V = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    XMFLOAT3 ndc;
    V = XMVector3TransformCoord(V, XMMatrixMultiply(device, trans));
    XMStoreFloat3(&ndc, V);


    XMStoreFloat4x4(&mView, XMMatrixTranspose(view));
    XMStoreFloat4x4(&mProjection, XMMatrixTranspose(projection));
    XMStoreFloat4x4(&deviceToView, XMMatrixMultiply(device, trans));
}

HRESULT CShader::Compile(ID3D11Device *d3dDevice, UINT nID, LPCTSTR szType)
{
    CComPtr<ID3DBlob> blob;
    HGLOBAL hRes = NULL;
    HREFTYPE hr = S_OK;

    HMODULE hModule = ::GetModuleHandle(NULL);
    HRSRC hSrc = ::FindResource(hModule, MAKEINTRESOURCE(nID), szType);
    BOOL_CHECK(hSrc);

    hRes = ::LoadResource(hModule, hSrc);
    BOOL_CHECK(hRes);

    DWORD cbSize = ::SizeofResource(hModule, hSrc);
    LPVOID pData = ::LockResource(hRes);

    // 创建顶点着色器
    HR_CHECK(::D3DCompile(pData, cbSize, NULL, NULL, NULL, "VS", "vs_4_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &blob, NULL));
    HR_CHECK(d3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &pVertexShader));
    // 创建顶点布局
    HR_CHECK(d3dDevice->CreateInputLayout(inputLayout, _countof(inputLayout), blob->GetBufferPointer(), blob->GetBufferSize(), &pVertexLayout));
    // 创建像素着色器
    blob.Release();
    HR_CHECK(::D3DCompile(pData, cbSize, NULL, NULL, NULL, "PS", "ps_4_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &blob, NULL));
    HR_CHECK(d3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &pPixelShader));

exit:
    return hr;
}

void XM_CALLCONV CShader::Update(ID3D11DeviceContext *d3dContext, DirectX::CXMMATRIX world)
{
    // 初始化常量缓冲区的值
    ConstantBuffer mCBuffer;
    mCBuffer.World = XMMatrixTranspose(world);
    mCBuffer.View = XMLoadFloat4x4(&mView);
    mCBuffer.Projection = XMLoadFloat4x4(&mProjection);
    // 更新常量缓冲区，让立方体转起来
    d3dContext->UpdateSubresource(pConstantBuffer, 0, NULL, &mCBuffer, 0, 0);
}