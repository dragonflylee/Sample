#include "Sample.h"
#include "Persist.h"

LPCTSTR lpRegKey = _T("Software\\Toolkit\\Sample");

int CMainFrm::OnCreate(LPCREATESTRUCT /*lpCreate*/)
{
    HRESULT hr = S_OK;
    // 初始化任务栏
    HR_CHECK(m_pTaskbar.CoCreateInstance(CLSID_TaskbarList));
    HR_CHECK(m_pTaskbar->HrInit());
    // 绑定对象
    m_pExternal = new CComObject<CExternal>();
    m_pExternal->SetWnd(this);
    // 创建浏览器控件
    m_hWndClient = m_wndView.Create(m_hWnd, rcDefault, _T("Shell.Explorer.2"),
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0, IDC_IE);
    BOOL_CHECK(m_hWndClient);
    // 创建地址栏
    // m_wndAddress.Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE |
    //    WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBS_DROPDOWN | CBS_AUTOHSCROLL);
    // BOOL_CHECK(CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE));
    // BOOL_CHECK(AddSimpleReBarBand(m_wndAddress, _T("Address"), FALSE));
    bool bRibbon = RunTimeHelper::IsRibbonUIAvailable();
    // Ribbon UI state and settings restoration
    CRibbonPersist(lpRegKey).Restore(bRibbon, m_hgRibbonSettings);
    m_mru.ReadFromRegistry(lpRegKey);
    // 读取之前窗口位置
    if (FAILED(CPlacementPersist(lpRegKey).Restore(m_hWnd))) CenterWindow();
    ShowRibbonUI(bRibbon);
    BOOL_CHECK(CreateSimpleStatusBar(IDR_MAIN));
    // 绑定事件
    HR_CHECK(m_wndView.SetExternalDispatch(m_pExternal));
    HR_CHECK(m_wndView.SetExternalUIHandler(this));
    HR_CHECK(m_wndView.CreateControl(IDR_INDEX));
    HR_CHECK(m_wndView.QueryControl(IID_PPV_ARGS(&m_pWb2)));
    // 绑定事件
    HR_CHECK(DispEventAdvise(m_pWb2));
exit:
    return SUCCEEDED(hr) ? 0 : -1;
}

void CMainFrm::OnDestroy()
{
    if (RunTimeHelper::IsRibbonUIAvailable())
    {
        bool bRibbonUI = IsRibbonUI();
        if (bRibbonUI) SaveRibbonSettings();
        CRibbonPersist(lpRegKey).Save(bRibbonUI, m_hgRibbonSettings);
    }
    // 保存窗口位置
    CPlacementPersist(lpRegKey).Save(m_hWnd);
    ::PostQuitMessage(0);
}

void CMainFrm::OnOpen(UINT /*uNotifyCode*/, int /*nID*/, HWND /*wndCtl*/)
{
    CFileDialog dlg(TRUE, _T("html"), NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY,
        TEXT("HTML Files (*.html;*htm)\0*.html;*htm\0"));

    if (dlg.DoModal(m_hWnd) == IDOK && SUCCEEDED(Open(dlg.m_szFileName)))
    {
        m_mru.AddToList(dlg.m_szFileName);
        m_mru.WriteToRegistry(lpRegKey);
    }
}

void CMainFrm::OnRecent(UINT /*uNotifyCode*/, int nID, HWND /*wndCtl*/)
{
    TCHAR szPath[MAX_PATH];
    if (m_mru.GetFromList(nID, szPath, _countof(szPath)))
    {
        SUCCEEDED(Open(szPath)) ? m_mru.MoveToTop(nID) : m_mru.RemoveFromList(nID);
        m_mru.WriteToRegistry(lpRegKey);
    }
}

void CMainFrm::OnExit(UINT /*uNotifyCode*/, int /*nID*/, HWND /*wndCtl*/)
{
    PostMessage(WM_SYSCOMMAND, SC_CLOSE);
}

void CMainFrm::OnAbout(UINT /*uNotifyCode*/, int /*nID*/, HWND /*wndCtl*/)
{
    ATL::CSimpleDialog<IDD_ABOUT> dlgAbout;
    dlgAbout.DoModal(m_hWnd);
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
    *dwRetVal = TrackRibbonMenu(IDM_CONTEXT, x, y);
    return S_OK;
}

STDMETHODIMP_(void) CMainFrm::OnTitleChange(BSTR title)
{
    SetWindowText(COLE2CT(title));
}

STDMETHODIMP_(void) CMainFrm::OnDocumentComplete(LPDISPATCH /*disp*/, VARIANT* /*url*/)
{

}

HRESULT CMainFrm::Open(LPCTSTR szURL)
{
    CComVariant v;
    return m_pWb2->Navigate(CComBSTR(szURL), &v, &v, &v, &v);
}