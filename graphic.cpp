#include "Demo.h"
#include <commdlg.h>

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>

HRESULT CGraphic::Init(HWND hWnd, int nWidth, int nHeight)
{
    DXGI_SWAP_CHAIN_DESC sd;
    HRESULT hr = S_OK;

    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = nWidth;
    sd.BufferDesc.Height = nHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
    };

    HR_CHECK(::D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        0, featureLevels, _countof(featureLevels), D3D11_SDK_VERSION, &sd,
        &pSwapChain, &pDevice, nullptr, &pContext));

    HR_CHECK(OnResize(nWidth, nHeight));

    HR_CHECK(pShader.Init(pDevice.Get(), pContext.Get()));

exit:
    return hr;
}

HRESULT CGraphic::OnResize(int nWidth, int nHeight)
{
    ComPtr<ID3D11Texture2D> tex;
    DXGI_SWAP_CHAIN_DESC sd;
    HRESULT hr = S_OK;

    HR_CHECK(pSwapChain->GetDesc(&sd));

    pBackRTV.Reset();
    pDepthSV.Reset();

    // 重设交换链并且重新创建渲染目标视图
    HR_CHECK(pSwapChain->ResizeBuffers(1, nWidth, nHeight, sd.BufferDesc.Format, sd.Flags));
    // Create a render target view
    HR_CHECK(pSwapChain->GetBuffer(0, IID_PPV_ARGS(&tex)));

    HR_CHECK(pDevice->CreateRenderTargetView(tex.Get(), nullptr, &pBackRTV));

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC depth;
    ZeroMemory(&depth, sizeof(depth));
    depth.Width = nWidth;
    depth.Height = nHeight;
    depth.MipLevels = 1;
    depth.ArraySize = 1;
    depth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth.SampleDesc.Count = 1;
    depth.SampleDesc.Quality = 0;
    depth.Usage = D3D11_USAGE_DEFAULT;
    depth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depth.CPUAccessFlags = 0;
    depth.MiscFlags = 0;

    // 创建深度缓冲区以及深度模板视图
    HR_CHECK(pDevice->CreateTexture2D(&depth, nullptr, &tex));
    HR_CHECK(pDevice->CreateDepthStencilView(tex.Get(), nullptr, &pDepthSV));
    // 将渲染目标视图和深度/模板缓冲区结合到管线
    pContext->OMSetRenderTargets(1, pBackRTV.GetAddressOf(), pDepthSV.Get());

    // Setup the viewport
    D3D11_VIEWPORT vp;
    ZeroMemory(&vp, sizeof(vp));
    vp.Width = static_cast<float>(nWidth);
    vp.Height = static_cast<float>(nHeight);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    pContext->RSSetViewports(1, &vp);

    pShader.Resize(vp);

exit:
    return hr;
}

LRESULT CGraphic::OnRender(HWND /*hWnd*/)
{
    /*
    ComPtr<IDXGISurface1> pSurface;
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };
    HDC hDC = nullptr;
    SIZE sz = { 0, 0 };
    POINT pt = { 0, 0 };*/
    HRESULT hr = S_OK;
    float color[] = { 0, 0, 1.0f, 1.0f };

    pContext->ClearRenderTargetView(pBackRTV.Get(), color);
    // 深度バッファのクリア
    pContext->ClearDepthStencilView(pDepthSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    for (size_t i = 0; i < pModel.size(); i++) pModel[i]->Draw(pContext.Get(), &pShader);

    HR_CHECK(pSwapChain->Present(0, 0));

    /*
    HR_CHECK(pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pSurface)));

    DXGI_SURFACE_DESC desc;
    HR_CHECK(pSurface->GetDesc(&desc));

    sz.cx = desc.Width;
    sz.cy = desc.Height;

    HR_CHECK(pSurface->GetDC(FALSE, &hDC));

    BOOL_CHECK(::UpdateLayeredWindow(hWnd, nullptr, nullptr, &sz, hDC, &pt, RGB(0, 0, 0), &blend, ULW_ALPHA));

    HR_CHECK(pSurface->ReleaseDC(nullptr));

    pContext->OMSetRenderTargets(1, pBackRTV.GetAddressOf(), pDepthSV.Get());
    */
exit:
    return hr;
}

LRESULT CGraphic::OnOpen(HWND hWnd)
{
    WCHAR path[MAX_PATH] = { 0 };
    WCHAR filter[MAX_PATH * 2] = L"model file";
    Assimp::Importer import;

    // 获取扩展名列表
    aiString str;
    size_t len = wcslen(filter) + 1;
    import.GetExtensionList(str);
    mbstowcs_s(&len, filter + len, _countof(filter) - len, str.C_Str(), _TRUNCATE);

    OPENFILENAMEW ofn = { sizeof(OPENFILENAMEW) };
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = filter;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.nMaxFile = _countof(path);
    ofn.lpstrFile = path;
    if (!::GetOpenFileNameW(&ofn)) return S_FALSE;

    wcstombs_s(&len, str.data, sizeof(str.data) - 1, path, _TRUNCATE);
    const aiScene* scene = import.ReadFile(str.C_Str(),
        aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
    if (nullptr == scene) return E_FAIL;

    CModel *m = new CModel();
    m->Load(pDevice.Get(), scene, path);
    pModel.push_back(std::auto_ptr<CModel>(m));

    return S_OK;
}

void CGraphic::OnMouseMove(int x, int y, int key)
{
    //DirectX::XMMATRIX w = pShader.Transform(static_cast<float>(x), static_cast<float>(y));
    //for (size_t i = 0; i < pModel.size(); i++)
    //     pModel[i]->SetWorld(w);
}

void CGraphic::OnMouseWheel(float delta)
{
    for (size_t i = 0; i < pModel.size(); i++)
        pModel[i]->Scale(delta, delta, delta);
}