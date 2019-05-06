#include "Sample.h"
#include "MainFrm.h"
#include <windowsx.h>
#include <commdlg.h>

CMaimFrm::CMaimFrm()
{
}

LRESULT CMaimFrm::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    HRESULT hr = S_OK;
    DXGI_SWAP_CHAIN_DESC sd;
    RECT rc;

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
    };

    BOOL_CHECK(::GetClientRect(m_hWnd, &rc));

    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = rc.right - rc.left;
    sd.BufferDesc.Height = rc.bottom - rc.top;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;

    UINT flag = 0;
#ifdef _DEBUG
    flag |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    HR_CHECK(::D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
        flag, featureLevels, _countof(featureLevels), D3D11_SDK_VERSION, &sd,
        &dxSwapChain, &d3dDevice, NULL, &d3dContext));

    HR_CHECK(OnResize(sd.BufferDesc.Width, sd.BufferDesc.Height));

    HR_CHECK(pShader.Init(d3dDevice, d3dContext));

exit:
    // 返回 -1 表示窗口创建失败
    return SUCCEEDED(hr) ? 0 : -1;
}

LRESULT CMaimFrm::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    ::PostQuitMessage(0);
    return S_OK;
}

LRESULT CMaimFrm::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE;
    return OnResize(LOWORD(lParam), HIWORD(lParam));
}

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>

LRESULT CMaimFrm::OnOpen(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
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
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFilter = filter;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.nMaxFile = _countof(path);
    ofn.lpstrFile = path;
    if (!::GetOpenFileNameW(&ofn)) return S_FALSE;

    wcstombs_s(&len, str.data, sizeof(str.data) - 1, path, _TRUNCATE);
    const aiScene* scene = import.ReadFile(str.C_Str(),
        aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
    if (NULL == scene) return E_FAIL;

    CAutoPtr<CModel> m(new CModel());
    m->Load(d3dDevice, scene, path);
    return pModel.Add(m);
}

HRESULT CMaimFrm::OnResize(int width, int height)
{
    CComPtr<ID3D11Texture2D> tex;
    DXGI_SWAP_CHAIN_DESC sd;
    HRESULT hr = S_OK;

    HR_CHECK(dxSwapChain->GetDesc(&sd));

    pBackRTV.Release();
    pDepthSV.Release();

    // 重设交换链并且重新创建渲染目标视图
    HR_CHECK(dxSwapChain->ResizeBuffers(1, width, height, sd.BufferDesc.Format, sd.Flags));
    // Create a render target view
    HR_CHECK(dxSwapChain->GetBuffer(0, IID_PPV_ARGS(&tex)));

    HR_CHECK(d3dDevice->CreateRenderTargetView(tex, NULL, &pBackRTV));

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC depth;
    ZeroMemory(&depth, sizeof(depth));
    depth.Width = width;
    depth.Height = height;
    depth.MipLevels = 1;
    depth.ArraySize = 1;
    depth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth.SampleDesc.Count = 1;
    depth.SampleDesc.Quality = 0;
    depth.Usage = D3D11_USAGE_DEFAULT;
    depth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depth.CPUAccessFlags = 0;
    depth.MiscFlags = 0;

    tex.Release();
    // 创建深度缓冲区以及深度模板视图
    HR_CHECK(d3dDevice->CreateTexture2D(&depth, NULL, &tex));
    HR_CHECK(d3dDevice->CreateDepthStencilView(tex, NULL, &pDepthSV));
    // 将渲染目标视图和深度/模板缓冲区结合到管线
    d3dContext->OMSetRenderTargets(1, &pBackRTV.p, pDepthSV);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    ZeroMemory(&vp, sizeof(vp));
    vp.Width = static_cast<float>(width);
    vp.Height = static_cast<float>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    d3dContext->RSSetViewports(1, &vp);

    pShader.Resize(vp);

exit:
    return hr;
}

LRESULT CMaimFrm::OnRender()
{
    HRESULT hr = S_OK;
    float color[] = { 0, 0, 1.0f, 1.0f };

    d3dContext->ClearRenderTargetView(pBackRTV, color);
    // 深度バッファのクリア
    d3dContext->ClearDepthStencilView(pDepthSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

    for (size_t i = 0; i < pModel.GetCount(); i++)
    {
        pShader.Update(d3dContext, pModel[i]->GetWorld());
        pModel[i]->Draw(d3dContext);
    }

    HR_CHECK(dxSwapChain->Present(0, 0));
exit:
    return hr;
}