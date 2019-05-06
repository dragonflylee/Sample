#pragma once

#include "External.h"
#include <ExDispid.h>

class CMainFrm : public CWindowImpl<CMainFrm, CWindow, CFrameWinTraits>,
    public IUIApplication, public IUICommandHandler,
    public IDispEventImpl<IDC_IE, CMainFrm, &DIID_DWebBrowserEvents2, &LIBID_SHDocVw, 1, 1>,
    public IDispatchImpl<IDocHostUIHandlerDispatch>
{
public:
    DECLARE_WND_CLASS_EX(TEXT("SampleWnd"), 0, COLOR_WINDOWFRAME);

    BEGIN_MSG_MAP(CMainFrm)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
    END_MSG_MAP()

    BEGIN_SINK_MAP(CMainFrm)
        SINK_ENTRY_EX(IDC_IE, DIID_DWebBrowserEvents2, DISPID_TITLECHANGE, OnTitleChange)
        SINK_ENTRY_EX(IDC_IE, DIID_DWebBrowserEvents2, DISPID_DOCUMENTCOMPLETE, OnDocumentComplete)
    END_SINK_MAP()

    /**
    * 窗口事件相关
    */
    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    /**
    * RibbonUI 相关
    */
    HRESULT LoadConfig();
    HRESULT SaveConfig();

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
    * IUIApplication 实现
    */
    STDMETHOD(OnCreateUICommand)(UINT /*nCmdID*/, UI_COMMANDTYPE /*typeID*/, IUICommandHandler** /*ppCommandHandler*/);
    STDMETHOD(OnViewChanged)(UINT /*viewId*/, UI_VIEWTYPE /*typeId*/, IUnknown* /*pView*/, UI_VIEWVERB /*verb*/, INT /*uReasonCode*/);
    STDMETHOD(OnDestroyUICommand)(UINT32 /*nCmdID*/, UI_COMMANDTYPE /*typeID*/, IUICommandHandler* /*commandHandler*/);
    /**
    * IUICommandHandler 实现
    */
    STDMETHOD(UpdateProperty)(UINT /*nCmdID*/, REFPROPERTYKEY /*key*/, const PROPVARIANT* /*ppropvarCurrentValue*/, PROPVARIANT* /*ppropvarNewValue*/);
    STDMETHOD(Execute)(UINT /*nCmdID*/, UI_EXECUTIONVERB /*verb*/, const PROPERTYKEY* /*key*/, const PROPVARIANT* /*ppropvarValue*/, IUISimplePropertySet* /*pCommandExecutionProperties*/);
    /**
    * DWebBrowserEvents2 事件
    */
    STDMETHOD_(void, OnTitleChange)(BSTR title);
    STDMETHOD_(void, OnDocumentComplete)(LPDISPATCH /*disp*/, VARIANT* /*url*/);

protected:
    class CRecentItems : public IUISimplePropertySet
    {
    public:
        CRecentItems(const CComBSTR& i) : m_Item(i) { }
        CComBSTR m_Item;
        // IUnknown methods.
        STDMETHODIMP_(ULONG) AddRef() { return 1; }
        STDMETHODIMP_(ULONG) Release() { return 1; }
        STDMETHOD(QueryInterface)(REFIID iid, void** ppv);
        // IUISimplePropertySet method.
        STDMETHOD(GetValue)(REFPROPERTYKEY key, PROPVARIANT *value);
    };

protected:
    CAxWindow wndView;
    CComPtr<IWebBrowser2> m_pWb2;
    CComPtr<IUIFramework> m_pUIFrame;
    CComPtr<IUIRibbon> m_pRibbon;
    CComPtr<ITaskbarList3> m_pTaskbar;
    CComPtr<CExternal> m_pExternal;
    CSimpleArray<CRecentItems> mru;
};
