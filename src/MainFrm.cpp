#include "Sample.h"
#include "Sample_s.h"
#include <UIRibbonPropertyHelpers.h>

LRESULT CMainFrm::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    HRESULT hr = S_OK;
    // 设置图标
    SetIcon(LoadIcon(_Module.m_hInst, MAKEINTRESOURCE(IDR_MAIN)));
    // 绑定对象
    m_pExternal = new CComObject<CExternal>();
    m_pExternal->SetWnd(this);
    // 创建浏览器控件
    BOOL_CHECK(wndView.Create(m_hWnd, rcDefault, _T("Shell.Explorer.2"), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0, IDC_IE));
    HR_CHECK(wndView.SetExternalDispatch(m_pExternal));
    HR_CHECK(wndView.SetExternalUIHandler(this));
    HR_CHECK(wndView.CreateControl(IDR_INDEX));
    HR_CHECK(wndView.QueryControl(IID_PPV_ARGS(&m_pWb2)));
    // 绑定事件
    HR_CHECK(DispEventAdvise(m_pWb2));
    // 初始化 Frame
    HR_CHECK(m_pUIFrame.CoCreateInstance(CLSID_UIRibbonFramework, NULL, CLSCTX_INPROC_SERVER));
    HR_CHECK(m_pUIFrame->Initialize(m_hWnd, this));
    HR_CHECK(m_pUIFrame->LoadUI(_Module.m_hInst, TEXT("APPLICATION_RIBBON")));
    // 初始化任务栏
    HR_CHECK(m_pTaskbar.CoCreateInstance(CLSID_TaskbarList));
    HR_CHECK(m_pTaskbar->HrInit());
exit:
    return SUCCEEDED(hr) ? 0 : -1;
}

LRESULT CMainFrm::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    if (m_pUIFrame != NULL)
        m_pUIFrame->Destroy();
    m_pUIFrame.Release();
    ::PostQuitMessage(0);
    return S_OK;
}

LRESULT CMainFrm::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    UINT uHeight;
    RECT rcWnd;
    if (FAILED(m_pRibbon->GetHeight(&uHeight))) return E_FAIL;
    if (!GetClientRect(&rcWnd)) return E_FAIL;
    rcWnd.top += uHeight;
    if (IsRectEmpty(&rcWnd) || !IsWindowVisible()) return S_FALSE;
    return wndView.MoveWindow(&rcWnd);
}

HRESULT CMainFrm::LoadConfig()
{
    CComPtr<IStorage> pStorage;
    CComPtr<IStream> pStream;
    WCHAR szPath[MAX_PATH];
    WINDOWPLACEMENT wp;
    HRESULT hr = S_OK;

    BOOL_CHECK(::GetModuleFileNameW(NULL, szPath, _countof(szPath)));
    BOOL_CHECK(::PathRenameExtensionW(szPath, L".ini"));

    HR_CHECK(::StgOpenStorageEx(szPath, STGM_READ | STGM_SHARE_DENY_WRITE, STGFMT_STORAGE,
        0, NULL, NULL, IID_PPV_ARGS(&pStorage)));
    HR_CHECK(pStorage->OpenStream(L"Ribbon", NULL, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pStream));
    HR_CHECK(m_pRibbon->LoadSettingsFromStream(pStream));

    pStream.Release();
    HR_CHECK(pStorage->OpenStream(L"Window", NULL, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pStream));
    HR_CHECK(pStream->Read(&wp, sizeof(wp), NULL));
    BOOL_CHECK(SetWindowPlacement(&wp));

exit:
    return hr;
}

HRESULT CMainFrm::SaveConfig()
{
    CComPtr<IStorage> pStorage;
    CComPtr<IStream> pStream;
    WCHAR szPath[MAX_PATH];
    WINDOWPLACEMENT wp;
    HRESULT hr = S_OK;

    BOOL_CHECK(::GetModuleFileNameW(NULL, szPath, _countof(szPath)));
    BOOL_CHECK(::PathRenameExtensionW(szPath, L".ini"));

    HR_CHECK(::StgCreateStorageEx(szPath, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, STGFMT_STORAGE,
        0, NULL, NULL, IID_PPV_ARGS(&pStorage)));
    HR_CHECK(pStorage->CreateStream(L"Ribbon", STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &pStream));
    HR_CHECK(m_pRibbon->SaveSettingsToStream(pStream));

    pStream.Release();
    BOOL_CHECK(GetWindowPlacement(&wp));
    HR_CHECK(pStorage->CreateStream(L"Window", STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &pStream));
    HR_CHECK(pStream->Write(&wp, sizeof(wp), NULL));

    pStream.Release();
    HR_CHECK(pStorage->CreateStream(L"Recent", STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &pStream));

exit:
    return hr;
}

STDMETHODIMP CMainFrm::QueryInterface(REFIID iid, void** ppvObject)
{
    if (iid == __uuidof(IDispatch))
        *ppvObject = static_cast<IDispatch *>(this);
    if (iid == __uuidof(IDocHostUIHandlerDispatch))
        *ppvObject = static_cast<IDocHostUIHandlerDispatch *>(this);
    else if (iid == __uuidof(IUICommandHandler))
        *ppvObject = static_cast<IUICommandHandler *>(this);
    else if (iid == __uuidof(IUIApplication))
        *ppvObject = static_cast<IUIApplication *>(this);
    else return E_NOINTERFACE;
    return S_OK;
}

STDMETHODIMP CMainFrm::ShowContextMenu(DWORD /*dwID*/, DWORD x, DWORD y, IUnknown * /*pcmdtReserved*/, IDispatch * /*pdispReserved*/, HRESULT *dwRetVal)
{
    HRESULT hr = E_NOTIMPL;
    CComPtr<IUIContextualUI> pUI;
    HR_CHECK(m_pUIFrame->GetView(IDM_CONTEXT, IID_PPV_ARGS(&pUI)));
    HR_CHECK(pUI->ShowAtLocation(x, y));
exit:
    *dwRetVal = hr;
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
            if (SUCCEEDED(hr)) LoadConfig();
            break;
        case UI_VIEWVERB_SIZE:
        {
            BOOL bHandled = FALSE;
            hr = OnSize(WM_SIZE, 0, 0, bHandled);
            break;
        }
        // The view was destroyed.
        case UI_VIEWVERB_DESTROY:
            hr = SaveConfig();
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
        WCHAR szPath[MAX_PATH] = L"file://";
        OPENFILENAMEW ofn = { sizeof(ofn) };
        ofn.hwndOwner = m_hWnd;
        ofn.lpstrFilter = L"HTML Files\0*.htm;*.html\0";
        ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        ofn.nMaxFile = _countof(szPath);
        ofn.lpstrFile = szPath + wcslen(szPath);
        if (!::GetOpenFileNameW(&ofn)) return S_FALSE;

        CComVariant v;
        return m_pWb2->Navigate(szPath, &v, &v, &v, &v);
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
            CComVariant v;
            return m_pWb2->Navigate(mru[nCmdID].m_Item, &v, &v, &v, &v);
        }
        break;
    }
    return E_NOTIMPL;
}

STDMETHODIMP_(void) CMainFrm::OnTitleChange(BSTR title)
{
    SetWindowText(COLE2CT(title));
}

STDMETHODIMP_(void) CMainFrm::OnDocumentComplete(LPDISPATCH /*disp*/, VARIANT* url)
{
    mru.Add(CComBSTR(url->bstrVal));
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