#include "Sample.h"
#include "Sample_s.h"
#include <commdlg.h>
#include <UIRibbonPropertyHelpers.h>

LRESULT CMainFrm::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    HRESULT hr = S_OK;
    DXGI_SWAP_CHAIN_DESC sd;
    UINT uHeight;
    RECT rc;

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
    };

    // 初始化 Frame
    HR_CHECK(m_pUIFrame.CoCreateInstance(CLSID_UIRibbonFramework, NULL, CLSCTX_INPROC_SERVER));
    HR_CHECK(m_pUIFrame->Initialize(m_hWnd, this));
    HR_CHECK(m_pUIFrame->LoadUI(_Module.m_hInst, TEXT("APPLICATION_RIBBON")));

    BOOL_CHECK(::GetClientRect(m_hWnd, &rc));
    HR_CHECK(m_pRibbon->GetHeight(&uHeight));
    rc.top += uHeight;
    wndView.m_hWnd = wndView.Create(m_hWnd, &rc);

    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = rc.right - rc.left;
    sd.BufferDesc.Height = rc.bottom - rc.top;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = wndView.m_hWnd;
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

    HR_CHECK(OnResize(&rc));

    HR_CHECK(pShader.Init(d3dDevice, d3dContext));

exit:
    // 返回 -1 表示窗口创建失败
    return SUCCEEDED(hr) ? 0 : -1;
}

LRESULT CMainFrm::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    ::PostQuitMessage(0);
    return S_OK;
}

LRESULT CMainFrm::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    bHandled = FALSE;

    UINT uHeight;
    RECT rcWnd;
    if (FAILED(m_pRibbon->GetHeight(&uHeight))) return E_FAIL;
    if (!GetClientRect(&rcWnd)) return E_FAIL;
    rcWnd.top += uHeight;
    if (IsRectEmpty(&rcWnd) || !IsWindowVisible()) return S_FALSE;
    wndView.MoveWindow(&rcWnd);
    return OnResize(&rcWnd);
}

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>

LRESULT CMainFrm::OnOpen()
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

HRESULT CMainFrm::OnResize(LPRECT rc)
{
    CComPtr<ID3D11Texture2D> tex;
    DXGI_SWAP_CHAIN_DESC sd;
    HRESULT hr = S_OK;

    HR_CHECK(dxSwapChain->GetDesc(&sd));

    pBackRTV.Release();
    pDepthSV.Release();

    int width = rc->right - rc->left;
    int height = rc->bottom - rc->top;

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
    vp.TopLeftX = static_cast<float>(rc->left);
    vp.TopLeftY = static_cast<float>(rc->top);
    d3dContext->RSSetViewports(1, &vp);

    pShader.Resize(vp);

exit:
    return hr;
}

LRESULT CMainFrm::OnRender()
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


STDMETHODIMP CMainFrm::QueryInterface(REFIID iid, void** ppvObject)
{
    if (iid == __uuidof(IUICommandHandler))
        *ppvObject = static_cast<IUICommandHandler *>(this);
    else if (iid == __uuidof(IUIApplication))
        *ppvObject = static_cast<IUIApplication *>(this);
    else return E_NOINTERFACE;
    return S_OK;
}

STDMETHODIMP CMainFrm::OnCreateUICommand(UINT /*nCmdID*/, UI_COMMANDTYPE /*typeID*/,
    IUICommandHandler **ppCmd)
{
    return QueryInterface(IID_PPV_ARGS(ppCmd));
}

STDMETHODIMP CMainFrm::OnViewChanged(UINT /*viewId*/, UI_VIEWTYPE typeId,
    IUnknown *pView, UI_VIEWVERB verb, INT /*uReasonCode*/)
{
    HRESULT hr = E_NOTIMPL;
    // Checks to see if the view that was changed was a Ribbon view.
    if (UI_VIEWTYPE_RIBBON == typeId)
    {
        switch (verb)
        {
        case UI_VIEWVERB_CREATE:
            // Call to the framework to determine the desired height of the Ribbon.
            hr = pView->QueryInterface(IID_PPV_ARGS(&m_pRibbon));
            break;
        case UI_VIEWVERB_SIZE:
        {
            BOOL bHandled = FALSE;
            hr = OnSize(WM_SIZE, 0, 0, bHandled);
            break;
        }
        // The view was destroyed.
        case UI_VIEWVERB_DESTROY:
            m_pRibbon.Release();
            break;
        }
    }
    return hr;
}

STDMETHODIMP CMainFrm::OnDestroyUICommand(UINT32 /*nCmdID*/, UI_COMMANDTYPE /*typeID*/,
    IUICommandHandler * /*commandHandler*/)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMainFrm::UpdateProperty(UINT /*nCmdID*/, REFPROPERTYKEY key,
    const PROPVARIANT * /*ppropvarCurrentValue*/, PROPVARIANT *pNewVal)
{
    HRESULT hr = E_NOTIMPL;
    if (UI_PKEY_RecentItems == key)
    {
        if (SAFEARRAY* psa = SafeArrayCreateVector(VT_UNKNOWN, 0, mru.GetSize()))
        {
            const int iLast = mru.GetSize() - 1;
            for (LONG i = 0; i <= iLast; i++)
                SafeArrayPutElement(psa, &i, &mru[iLast - i]); // reverse order
            hr = UIInitPropertyFromIUnknownArray(key, psa, pNewVal);
            SafeArrayDestroy(psa);
        }
    }
    else if (UI_PKEY_BooleanValue == key)
    {
        hr = UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, FALSE, pNewVal);
    }
    return hr;
}

STDMETHODIMP CMainFrm::Execute(UINT nCmdID, UI_EXECUTIONVERB /*verb*/, const PROPERTYKEY * key,
    const PROPVARIANT * ppVal, IUISimplePropertySet * /*pCommandExecutionProperties*/)
{
    switch (nCmdID)
    {
    case IDM_OPENFILE:
    {
        return OnOpen();
    }
    case IDM_ABOUT:
    {
        ATL::CSimpleDialog<IDD_ABOUT> dlgAbout;
        return dlgAbout.DoModal(m_hWnd);
    }
    case IDM_EXIT:
        return PostMessage(WM_SYSCOMMAND, SC_CLOSE);
    case IDC_RECENT_FILES:
        if (SUCCEEDED(UIPropertyToUInt32(*key, *ppVal, &nCmdID)))
        {
            return S_OK;
        }
        break;
    }
    return E_NOTIMPL;
}


STDMETHODIMP CMainFrm::CRecentItems::QueryInterface(REFIID iid, void** ppv)
{
    if (iid == __uuidof(IUISimplePropertySet))
    {
        *ppv = static_cast<IUISimplePropertySet *>(this);
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }
}

// IUISimplePropertySet method.
STDMETHODIMP CMainFrm::CRecentItems::GetValue(REFPROPERTYKEY key, PROPVARIANT *value)
{
    if (UI_PKEY_Label == key) return ::UIInitPropertyFromString(key, ::PathFindFileNameW(m_Item), value);
    if (UI_PKEY_LabelDescription == key) return ::UIInitPropertyFromString(key, m_Item, value);
    return E_NOTIMPL;
}