/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/mdi.h
// Author:      Peter Most, Mariano Reingart
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_MDI_H_
#define _WX_QT_MDI_H_

class wxMDIParentFrame : public wxMDIParentFrameBase
{
public:
    wxMDIParentFrame();
    wxMDIParentFrame(wxWindow *parent,
                     wxWindowID id,
                     const wxString& title,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     unsigned int style = wxDEFAULT_FRAME_STYLE | wxVSCROLL | wxHSCROLL,
                     const wxString& name = wxASCII_STR(wxFrameNameStr));

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDEFAULT_FRAME_STYLE | wxVSCROLL | wxHSCROLL,
                const wxString& name = wxASCII_STR(wxFrameNameStr));

    // override/implement base class [pure] virtual methods
    // ----------------------------------------------------

    static bool IsTDI() { return false; }

    void ActivateNext() override;
    void ActivatePrevious() override;

protected:

private:
    wxDECLARE_DYNAMIC_CLASS(wxMDIParentFrame);
};



class wxMDIChildFrame : public wxMDIChildFrameBase
{
public:
    wxMDIChildFrame();
    wxMDIChildFrame(wxMDIParentFrame *parent,
                    wxWindowID id,
                    const wxString& title,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    unsigned int style = wxDEFAULT_FRAME_STYLE,
                    const wxString& name = wxASCII_STR(wxFrameNameStr));

    bool Create(wxMDIParentFrame *parent,
                wxWindowID id,
                const wxString& title,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDEFAULT_FRAME_STYLE,
                const wxString& name = wxASCII_STR(wxFrameNameStr));

    void Activate() override;

    wxDECLARE_DYNAMIC_CLASS(wxMDIChildFrame);
};



class wxMDIClientWindow : public wxMDIClientWindowBase
{
public:
    wxMDIClientWindow();

    bool CreateClient(wxMDIParentFrame *parent, unsigned int style = wxVSCROLL | wxHSCROLL) override;
    wxDECLARE_DYNAMIC_CLASS(wxMDIClientWindow);
};

#endif // _WX_QT_MDI_H_
