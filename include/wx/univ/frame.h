///////////////////////////////////////////////////////////////////////////////
// Name:        wx/univ/frame.h
// Purpose:     wxFrame class for wxUniversal
// Author:      Vadim Zeitlin
// Modified by:
// Created:     19.05.01
// Copyright:   (c) 2001 SciTech Software, Inc. (www.scitechsoft.com)
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_UNIV_FRAME_H_
#define _WX_UNIV_FRAME_H_

// ----------------------------------------------------------------------------
// wxFrame
// ----------------------------------------------------------------------------

class wxFrame : public wxFrameBase
{
public:
    wxFrame() {}
    wxFrame(wxWindow *parent,
            wxWindowID id,
            const wxString& title,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            unsigned int style = wxDEFAULT_FRAME_STYLE,
            const wxString& name = wxASCII_STR(wxFrameNameStr))
    {
        Create(parent, id, title, pos, size, style, name);
    }

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDEFAULT_FRAME_STYLE,
                const wxString& name = wxASCII_STR(wxFrameNameStr));

    wxPoint GetClientAreaOrigin() const override;
    bool Enable(bool enable = true) override;

#if wxUSE_STATUSBAR
    virtual wxStatusBar* CreateStatusBar(int number = 1,
                                         unsigned int style = wxSTB_DEFAULT_STYLE,
                                         wxWindowID id = 0,
                                         const wxString& name = wxASCII_STR(wxStatusLineNameStr)) override;
#endif // wxUSE_STATUSBAR

#if wxUSE_TOOLBAR
    // create main toolbar bycalling OnCreateToolBar()
    virtual wxToolBar* CreateToolBar(unsigned int style = -1,
                                     wxWindowID id = wxID_ANY,
                                     const wxString& name = wxASCII_STR(wxToolBarNameStr)) override;
#endif // wxUSE_TOOLBAR

    wxSize GetMinSize() const override;

protected:
    void OnSize(wxSizeEvent& event);
    void OnSysColourChanged(wxSysColourChangedEvent& event);

    wxSize DoGetClientSize() const override;
    void DoSetClientSize(int width, int height) override;

#if wxUSE_MENUS
    // override to update menu bar position when the frame size changes
    void PositionMenuBar() override;
    void DetachMenuBar() override;
    void AttachMenuBar(wxMenuBar *menubar) override;
#endif // wxUSE_MENUS

#if wxUSE_STATUSBAR
    // override to update statusbar position when the frame size changes
    void PositionStatusBar() override;
#endif // wxUSE_MENUS

protected:
#if wxUSE_TOOLBAR
    void PositionToolBar() override;
#endif // wxUSE_TOOLBAR

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_DYNAMIC_CLASS(wxFrame);
};

#endif // _WX_UNIV_FRAME_H_
