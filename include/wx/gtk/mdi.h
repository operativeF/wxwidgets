/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/mdi.h
// Purpose:     TDI-based MDI implementation for wxGTK
// Author:      Robert Roebling
// Modified by: 2008-10-31 Vadim Zeitlin: derive from the base classes
// Copyright:   (c) 1998 Robert Roebling
//              (c) 2008 Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTK_MDI_H_
#define _WX_GTK_MDI_H_

#include "wx/frame.h"

class WXDLLIMPEXP_FWD_CORE wxMDIChildFrame;
class WXDLLIMPEXP_FWD_CORE wxMDIClientWindow;

typedef struct _GtkNotebook GtkNotebook;

//-----------------------------------------------------------------------------
// wxMDIParentFrame
//-----------------------------------------------------------------------------

class wxMDIParentFrame : public wxMDIParentFrameBase
{
public:
    wxMDIParentFrame() { Init(); }
    wxMDIParentFrame(wxWindow *parent,
                     wxWindowID id,
                     const wxString& title,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = wxDEFAULT_FRAME_STYLE | wxVSCROLL | wxHSCROLL,
                     const wxString& name = wxASCII_STR(wxFrameNameStr))
    {
        Init();

        (void)Create(parent, id, title, pos, size, style, name);
    }

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxDEFAULT_FRAME_STYLE | wxVSCROLL | wxHSCROLL,
                const wxString& name = wxASCII_STR(wxFrameNameStr));

    // we don't store the active child in m_currentChild unlike the base class
    // version so override this method to find it dynamically
    wxMDIChildFrame *GetActiveChild() const override;

    
    // ----------------------------------

    void ActivateNext() override;
    void ActivatePrevious() override;

    static bool IsTDI() { return true; }

    // implementation

    bool                m_justInserted;

    void OnInternalIdle() override;

protected:
    wxSize DoGetClientSize() const override;

private:
    friend class wxMDIChildFrame;
    void Init();

    wxDECLARE_DYNAMIC_CLASS(wxMDIParentFrame);
};

//-----------------------------------------------------------------------------
// wxMDIChildFrame
//-----------------------------------------------------------------------------

class wxMDIChildFrame : public wxTDIChildFrame
{
public:
    wxMDIChildFrame() { Init(); }
    wxMDIChildFrame(wxMDIParentFrame *parent,
                    wxWindowID id,
                    const wxString& title,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    long style = wxDEFAULT_FRAME_STYLE,
                    const wxString& name = wxASCII_STR(wxFrameNameStr))
    {
        Init();

        Create(parent, id, title, pos, size, style, name);
    }

    bool Create(wxMDIParentFrame *parent,
                wxWindowID id,
                const wxString& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxDEFAULT_FRAME_STYLE,
                const wxString& name = wxASCII_STR(wxFrameNameStr));

    virtual ~wxMDIChildFrame();

    void SetMenuBar( wxMenuBar *menu_bar ) override;
    wxMenuBar *GetMenuBar() const override;

    void Activate() override;

    void SetTitle(const wxString& title) override;

    // implementation

    void OnActivate( wxActivateEvent& event );
    void OnMenuHighlight( wxMenuEvent& event );
    void GTKHandleRealized() override;

    wxMenuBar         *m_menuBar;
    bool               m_justInserted;

protected:
    wxPoint DoGetPosition() const override;

private:
    void Init();

    GtkNotebook *GTKGetNotebook() const;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_DYNAMIC_CLASS(wxMDIChildFrame);
};

//-----------------------------------------------------------------------------
// wxMDIClientWindow
//-----------------------------------------------------------------------------

class wxMDIClientWindow : public wxMDIClientWindowBase
{
public:
    wxMDIClientWindow() { }
    ~wxMDIClientWindow();

    virtual bool CreateClient(wxMDIParentFrame *parent,
                              long style = wxVSCROLL | wxHSCROLL) override;

private:
    void AddChildGTK(wxWindowGTK* child) override;

    wxDECLARE_DYNAMIC_CLASS(wxMDIClientWindow);
};

#endif // _WX_GTK_MDI_H_
