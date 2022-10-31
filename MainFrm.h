#pragma once

#include "Model.h"
#include "Shader.h"

class CMainFrm : public CWindowImpl<CMainFrm, CWindow, CFrameWinTraits>,
    public IUIApplication, public IUICommandHandler
{
public:
    DECLARE_WND_CLASS(TEXT("Sample"));

    BEGIN_MSG_MAP(CMainFrm)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
    END_MSG_MAP()

    /**
    * IUnkown 实现
    */
    STDMETHOD_(ULONG, AddRef)() { return 1; }
    STDMETHOD_(ULONG, Release)() { return 1; }
    STDMETHOD(QueryInterface)(REFIID /*iid*/, void** /*ppvObject*/);
    /**
    * 窗口事件相关
    */
    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnOpen();
    /**
    * DirectX 3D 相关
    */
    HRESULT OnResize(LPRECT rc);
    LRESULT OnRender();

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

    class CDXView : public CWindowImpl<CDXView>
    {
    public:
        DECLARE_WND_CLASS(TEXT("SampleDX"));

        BEGIN_MSG_MAP(CGLView)
        END_MSG_MAP()
    };

    CComPtr<IUIFramework> m_pUIFrame;
    CComPtr<IUIRibbon> m_pRibbon;
    CSimpleArray<CRecentItems> mru;

    CDXView wndView;
    CComPtr<ID3D11Device> d3dDevice;
    CComPtr<ID3D11DeviceContext> d3dContext;
    CComPtr<IDXGISwapChain> dxSwapChain;
    CComPtr<ID3D11RenderTargetView> pBackRTV;
    CComPtr<ID3D11DepthStencilView> pDepthSV;

    CAutoPtrArray<CModel> pModel;
    CShader pShader;
};
