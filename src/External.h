#pragma once

#include "Sample_h.h"

class CExternal :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IExternal, &IID_IExternal, &LIBID_SampleLib, -1, -1>
{
public:
    BEGIN_COM_MAP(CExternal)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IExternal)
    END_COM_MAP()

    CWindow * m_pWnd;
    void SetWnd(CWindow *pWnd) { m_pWnd = pWnd; }
    /**
    * IExternal 实现
    */
    STDMETHOD(get_Username)(BSTR *retval)
    {
        if (!retval) return E_POINTER;
        CComBSTR szName(MAX_PATH);
        DWORD cbLen = szName.Length();
        if (!::GetUserNameW(szName, &cbLen)) return E_FAIL;
        *retval = szName;
        return S_OK;
    }

    STDMETHOD(put_View)(IDispatch * /*disp*/)
    {
        return S_OK;
    }

    STDMETHOD(Close)()
    {
        return m_pWnd->PostMessage(WM_SYSCOMMAND, SC_CLOSE);
    }
};