///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/ole/activex.h
// Purpose:     wxActiveXContainer class
// Author:      Ryan Norton <wxprojects@comcast.net>
// Modified by:
// Created:     8/18/05
// Copyright:   (c) Ryan Norton
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// ============================================================================
// Definitions
// ============================================================================

#ifndef _WX_MSW_OLE_ACTIVEXCONTAINER_H_
#define _WX_MSW_OLE_ACTIVEXCONTAINER_H_

#if wxUSE_ACTIVEX

//---------------------------------------------------------------------------
// wx includes
//---------------------------------------------------------------------------

#include "wx/msw/ole/oleutils.h" // wxBasicString &c
#include "wx/msw/ole/uuid.h"
#include "wx/window.h"
#include "wx/variant.h"

import WX.WinDef;

class FrameSite;

//---------------------------------------------------------------------------
// MSW COM includes
//---------------------------------------------------------------------------

#include <exdisp.h>

#include <docobj.h>

#ifndef STDMETHOD
    #define STDMETHOD(funcname)  virtual HRESULT wxSTDCALL funcname
#endif

//
//  These defines are from another ole header - but its not in the
//  latest sdk.  Also the ifndef DISPID_READYSTATE is here because at
//  least on my machine with the latest sdk olectl.h defines these 3
//
#ifndef DISPID_READYSTATE
    #define DISPID_READYSTATE                               (-525)
    #define DISPID_READYSTATECHANGE                         (-609)
    #define DISPID_AMBIENT_TRANSFERPRIORITY                 (-728)
#endif

#define DISPID_AMBIENT_OFFLINEIFNOTCONNECTED            (-5501)
#define DISPID_AMBIENT_SILENT                           (-5502)

#ifndef DISPID_AMBIENT_CODEPAGE
    #define DISPID_AMBIENT_CODEPAGE                         (-725)
    #define DISPID_AMBIENT_CHARSET                          (-727)
#endif


//---------------------------------------------------------------------------
//
//  wxActiveXContainer
//
//---------------------------------------------------------------------------

template<typename I>
class wxAutoOleInterface
{
public:
    using Interface = I;

    explicit wxAutoOleInterface(I *pInterface = nullptr) : m_interface(pInterface)
        {}
    wxAutoOleInterface(REFIID riid, IUnknown *pUnk) : m_interface(nullptr)
        { QueryInterface(riid, pUnk); }
    wxAutoOleInterface(REFIID riid, IDispatch *pDispatch) : m_interface(nullptr)
        { QueryInterface(riid, pDispatch); }
    wxAutoOleInterface(REFCLSID clsid, REFIID riid) : m_interface(NULL)
        { CreateInstance(clsid, riid); }
    wxAutoOleInterface(const wxAutoOleInterface& ti) : m_interface(NULL)
        { operator=(ti); }

    wxAutoOleInterface& operator=(const wxAutoOleInterface& ti)
    {
        if ( ti.m_interface )
            ti.m_interface->AddRef();
        Free();
        m_interface = ti.m_interface;
        return *this;
    }

    wxAutoOleInterface& operator=(I*& ti)
    {
        Free();
        m_interface = ti;
        return *this;
    }

    ~wxAutoOleInterface() { Free(); }

    void Free()
    {
        if ( m_interface )
            m_interface->Release();
        m_interface = nullptr;
    }

    HRESULT QueryInterface(REFIID riid, IUnknown *pUnk)
    {
        Free();
        wxASSERT(pUnk != nullptr);
        return pUnk->QueryInterface(riid, (void **)&m_interface);
    }

    HRESULT CreateInstance(REFCLSID clsid, REFIID riid)
    {
        Free();
        return CoCreateInstance
               (
                   clsid,
                   NULL,
                   CLSCTX_ALL,
                   riid,
                   (void **)&m_interface
               );
    }

    operator I*() const {return m_interface; }
    I* operator->() {return m_interface; }
    I** GetRef() {return &m_interface; }
    bool IsOk() const { return m_interface != nullptr; }

protected:
    I *m_interface;
};

using wxAutoIDispatch = wxAutoOleInterface<IDispatch>;
using wxAutoIOleClientSite = wxAutoOleInterface<IOleClientSite>;
using wxAutoIUnknown = wxAutoOleInterface<IUnknown>;
using wxAutoIOleObject = wxAutoOleInterface<IOleObject>;
using wxAutoIOleInPlaceObject = wxAutoOleInterface<IOleInPlaceObject>;
using wxAutoIOleInPlaceActiveObject = wxAutoOleInterface<IOleInPlaceActiveObject>;
using wxAutoIOleDocumentView = wxAutoOleInterface<IOleDocumentView>;
using wxAutoIViewObject = wxAutoOleInterface<IViewObject>;

class wxActiveXContainer : public wxWindow
{
public:
    wxActiveXContainer(wxWindow * parent, REFIID iid, IUnknown* pUnk);
    ~wxActiveXContainer();

    void OnSize(wxSizeEvent&);
    void OnPaint(wxPaintEvent&);
    void OnSetFocus(wxFocusEvent&);
    void OnKillFocus(wxFocusEvent&);
    bool MSWTranslateMessage(WXMSG* pMsg) override;
    virtual bool QueryClientSiteInterface(REFIID iid, void **_interface, const char *&desc);

protected:
    friend class FrameSite;
    friend class wxActiveXEvents;

    FrameSite *m_frameSite;
    wxAutoIDispatch            m_Dispatch;
    wxAutoIOleClientSite      m_clientSite;
    wxAutoIUnknown         m_ActiveX;
    wxAutoIOleObject            m_oleObject;
    wxAutoIOleInPlaceObject    m_oleInPlaceObject;
    wxAutoIOleInPlaceActiveObject m_oleInPlaceActiveObject;
    wxAutoIOleDocumentView    m_docView;
    wxAutoIViewObject            m_viewObject;
    WXHWND m_oleObjectHWND;
    bool m_bAmbientUserMode;
    WXDWORD m_docAdviseCookie;
    wxWindow* m_realparent;

    void CreateActiveX(REFIID, IUnknown*);
};

///\brief Store native event parameters.
///\detail Store OLE 'Invoke' parameters for event handlers that need to access them.
/// These are the exact values for the event as they are passed to the wxActiveXContainer.
struct wxActiveXEventNativeMSW
{
    DISPID  dispIdMember;
    REFIID  riid;
    LCID    lcid;
    WXWORD    wFlags;
    DISPPARAMS  *pDispParams;
    VARIANT     *pVarResult;
    EXCEPINFO   *pExcepInfo;
    unsigned int *puArgErr;

    wxActiveXEventNativeMSW
        (DISPID a_dispIdMember, REFIID a_riid, LCID a_lcid, WXWORD a_wFlags, DISPPARAMS  *a_pDispParams,
        VARIANT *a_pVarResult, EXCEPINFO *a_pExcepInfo, unsigned int *a_puArgErr)
        :dispIdMember(a_dispIdMember), riid(a_riid), lcid(a_lcid), wFlags(a_wFlags), pDispParams(a_pDispParams),
        pVarResult(a_pVarResult), pExcepInfo(a_pExcepInfo), puArgErr(a_puArgErr)
    { }
};

// Events
class wxActiveXEvent : public wxCommandEvent
{
private:
    friend class wxActiveXEvents;
    wxVariant m_params;
    DISPID m_dispid;

public:
    std::unique_ptr<wxEvent> Clone() const override
    { return std::make_unique<wxActiveXEvent>(*this); }

    size_t ParamCount() const;

    wxString ParamType(size_t idx) const
    {
        wxASSERT(idx < ParamCount());
        return m_params[idx].GetType();
    }

    wxString ParamName(size_t idx) const
    {
        wxASSERT(idx < ParamCount());
        return m_params[idx].GetName();
    }

    wxVariant& operator[] (size_t idx);

    DISPID GetDispatchId() const
    {   return m_dispid;    }

    wxActiveXEventNativeMSW *GetNativeParameters() const
    {   return (wxActiveXEventNativeMSW*)GetClientData(); }
};

// #define wxACTIVEX_ID    14001
wxDECLARE_EVENT( wxEVT_ACTIVEX, wxActiveXEvent );

typedef void (wxEvtHandler::*wxActiveXEventFunction)(wxActiveXEvent&);

#define wxActiveXEventHandler(func) \
    wxEVENT_HANDLER_CAST( wxActiveXEventFunction, func )

#define EVT_ACTIVEX(id, fn)     wxDECLARE_EVENT_TABLE_ENTRY(wxEVT_ACTIVEX, id, -1, wxActiveXEventHandler( fn ), NULL ),

#endif // wxUSE_ACTIVEX

#endif // _WX_MSW_OLE_ACTIVEXCONTAINER_H_
