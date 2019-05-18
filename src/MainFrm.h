#pragma once

#include "External.h"
#include "Sample_s.h"
#include <ExDispid.h>

class CMainFrm :
    public CRibbonFrameWindowImpl<CMainFrm>, public CMessageLoop,
    public IDispEventImpl<IDC_IE, CMainFrm, &DIID_DWebBrowserEvents2, &LIBID_SHDocVw, 1, 1>,
    public IDispatchImpl<IDocHostUIHandlerDispatch>
{

    typedef CRibbonFrameWindowImpl<CMainFrm> baseClass;

public:
    DECLARE_FRAME_WND_CLASS_EX(_T("SampleWnd"), IDR_MAIN, 0, COLOR_WINDOWFRAME)

    BEGIN_MSG_MAP_EX(CMainFrm)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        COMMAND_ID_HANDLER_EX(IDM_OPEN, OnOpen)
        COMMAND_ID_HANDLER_EX(IDM_EXIT, OnExit)
        COMMAND_ID_HANDLER_EX(IDM_ABOUT, OnAbout)
        COMMAND_RANGE_HANDLER_EX(ID_FILE_MRU_FIRST, ID_FILE_MRU_LAST, OnRecent)
        CHAIN_MSG_MAP(baseClass)
    END_MSG_MAP()

    BEGIN_SINK_MAP(CMainFrm)
        SINK_ENTRY_EX(IDC_IE, DIID_DWebBrowserEvents2, DISPID_TITLECHANGE, OnTitleChange)
        SINK_ENTRY_EX(IDC_IE, DIID_DWebBrowserEvents2, DISPID_DOCUMENTCOMPLETE, OnDocumentComplete)
    END_SINK_MAP()

    BEGIN_RIBBON_CONTROL_MAP(CMainFrm)
        RIBBON_CONTROL(m_mru)
    END_RIBBON_CONTROL_MAP();

    BOOL PreTranslateMessage(MSG* pMsg)
    {
        return baseClass::PreTranslateMessage(pMsg);
    }
    /**
    * 窗口事件相关
    */
    int OnCreate(LPCREATESTRUCT /*lpCreate*/);
    void OnDestroy();
    /**
    * 菜单命令
    */
    void OnOpen(UINT /*uNotifyCode*/, int /*nID*/, HWND /*wndCtl*/);
    void OnRecent(UINT /*uNotifyCode*/, int /*nID*/, HWND /*wndCtl*/);
    void OnExit(UINT /*uNotifyCode*/, int /*nID*/, HWND /*wndCtl*/);
    void OnAbout(UINT /*uNotifyCode*/, int /*nID*/, HWND /*wndCtl*/);
    /**
    * IUnkown 实现
    */
    STDMETHOD_(ULONG, AddRef)() { return 1; }
    STDMETHOD_(ULONG, Release)() { return 1; }
    STDMETHOD(QueryInterface)(REFIID /*iid*/, void** /*ppvObject*/);
    /**
    * IDocHostUIHandlerDispatch 实现
    */
    STDMETHOD(ShowContextMenu)(DWORD /*dwID*/, DWORD x, DWORD y, IUnknown * /*pcmdtReserved*/, IDispatch * /*pdispReserved*/, HRESULT *dwRetVal);
    STDMETHOD(GetExternal)(IDispatch ** ppDispatch) { return m_pExternal->QueryInterface(IID_PPV_ARGS(ppDispatch)); }
    STDMETHOD(GetHostInfo)(DWORD * /*pdwFlags*/, DWORD * /*pdwDoubleClick*/) { return E_NOTIMPL; }
    STDMETHOD(ShowUI)(DWORD /*dwID*/, IUnknown * /*pActiveObject*/, IUnknown * /*pCommandTarget*/, IUnknown * /*pFrame*/, IUnknown * /*pDoc*/, HRESULT * /*dwRetVal*/) { return E_NOTIMPL; }
    STDMETHOD(HideUI)() { return E_NOTIMPL; }
    STDMETHOD(UpdateUI)() { return E_NOTIMPL; }
    STDMETHOD(EnableModeless)(VARIANT_BOOL /*fEnable*/) { return E_NOTIMPL; }
    STDMETHOD(OnDocWindowActivate)(VARIANT_BOOL /*fActivate*/) { return E_NOTIMPL; }
    STDMETHOD(OnFrameWindowActivate)(VARIANT_BOOL /*fActivate*/) { return E_NOTIMPL; }
    STDMETHOD(ResizeBorder)(long /*left*/, long /*top*/, long /*right*/, long /*bottom*/, IUnknown * /*pUIWindow*/, VARIANT_BOOL /*fFrameWindow*/) { return E_NOTIMPL; }
    STDMETHOD(TranslateAccelerator)(DWORD_PTR /*hWnd*/, DWORD /*nMessage*/, DWORD_PTR /*wParam*/, DWORD_PTR /*lParam*/, BSTR /*bstrGuidCmdGroup*/, DWORD /*nCmdID*/, HRESULT * /*dwRetVal*/) { return E_NOTIMPL; }
    STDMETHOD(GetOptionKeyPath)(BSTR * /*pbstrKey*/, DWORD /*dw*/) { return E_NOTIMPL; }
    STDMETHOD(GetDropTarget)(IUnknown * /*pDropTarget*/, IUnknown ** /*ppDropTarget*/) { return E_NOTIMPL; }
    STDMETHOD(TranslateUrl)(DWORD /*dwTranslate*/, BSTR /*bstrURLIn*/, BSTR * /*pbstrURLOut*/) { return E_NOTIMPL; }
    STDMETHOD(FilterDataObject)(IUnknown * /*pDO*/, IUnknown ** /*ppDORet*/) { return E_NOTIMPL; }
    /**
    * DWebBrowserEvents2 事件
    */
    STDMETHOD_(void, OnTitleChange)(BSTR title);
    STDMETHOD_(void, OnDocumentComplete)(LPDISPATCH /*disp*/, VARIANT* /*url*/);

private:
    HRESULT Open(LPCTSTR /*szURL*/);

protected:
    CAxWindow m_wndView;
    CComboBoxEx m_wndAddress;
    CComPtr<IWebBrowser2> m_pWb2;
    CComPtr<ITaskbarList3> m_pTaskbar;
    CComPtr<CExternal> m_pExternal;
    CRibbonRecentItemsCtrl<IDC_RECENT_FILES> m_mru;
};
