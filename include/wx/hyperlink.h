/////////////////////////////////////////////////////////////////////////////
// Name:        wx/hyperlink.h
// Purpose:     Hyperlink control
// Author:      David Norris <danorris@gmail.com>, Otto Wyss
// Modified by: Ryan Norton, Francesco Montorsi
// Created:     04/02/2005
// Copyright:   (c) 2005 David Norris
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_HYPERLINK_H_
#define _WX_HYPERLINK_H_

#include "wx/defs.h"

#if wxUSE_HYPERLINKCTRL

#include "wx/control.h"

#include <string>

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

constexpr unsigned int wxHL_CONTEXTMENU        = 0x0001;
constexpr unsigned int wxHL_ALIGN_LEFT         = 0x0002;
constexpr unsigned int wxHL_ALIGN_RIGHT        = 0x0004;
constexpr unsigned int wxHL_ALIGN_CENTRE       = 0x0008;
constexpr unsigned int wxHL_DEFAULT_STYLE      = wxHL_CONTEXTMENU | wxNO_BORDER | wxHL_ALIGN_CENTRE;

constexpr char wxHyperlinkCtrlNameStr[] = "hyperlink";

// ----------------------------------------------------------------------------
// wxHyperlinkCtrl
// ----------------------------------------------------------------------------

// A static text control that emulates a hyperlink. The link is displayed
// in an appropriate text style, derived from the control's normal font.
// When the mouse rolls over the link, the cursor changes to a hand and the
// link's color changes to the active color.
//
// Clicking on the link does not launch a web browser; instead, a
// HyperlinkEvent is fired. The event propagates upward until it is caught,
// just like a wxCommandEvent.
//
// Use the EVT_HYPERLINK() to catch link events.
class wxHyperlinkCtrlBase : public wxControl
{
public:

    // get/set
    virtual wxColour GetHoverColour() const = 0;
    virtual void SetHoverColour(const wxColour &colour) = 0;

    virtual wxColour GetNormalColour() const = 0;
    virtual void SetNormalColour(const wxColour &colour) = 0;

    virtual wxColour GetVisitedColour() const = 0;
    virtual void SetVisitedColour(const wxColour &colour) = 0;

    virtual const std::string& GetURL() const = 0;
    virtual void SetURL (const std::string &url) = 0;

    virtual void SetVisited(bool visited = true) = 0;
    virtual bool GetVisited() const = 0;

    // NOTE: also wxWindow::Set/GetLabel, wxWindow::Set/GetBackgroundColour,
    //       wxWindow::Get/SetFont, wxWindow::Get/SetCursor are important !

    bool HasTransparentBackground() override { return true; }

protected:
    wxBorder GetDefaultBorder() const override { return wxBORDER_NONE; }

    // checks for validity some of the ctor/Create() function parameters
    void CheckParams(const std::string& label, const std::string& url, unsigned int style);

public:
    // Send wxHyperlinkEvent and open our link in the default browser if it
    // wasn't handled.
    //
    // not part of the public API but needs to be public as used by
    // GTK+ callbacks:
    void SendEvent();
};

// ----------------------------------------------------------------------------
// wxHyperlinkEvent
// ----------------------------------------------------------------------------

class wxHyperlinkEvent;

wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_HYPERLINK, wxHyperlinkEvent );

//
// An event fired when the user clicks on the label in a hyperlink control.
// See HyperlinkControl for details.
//
class wxHyperlinkEvent : public wxCommandEvent
{
public:
    wxHyperlinkEvent() = default;
    wxHyperlinkEvent(wxObject *generator, wxWindowID id, const std::string& url)
        : wxCommandEvent(wxEVT_HYPERLINK, id),
          m_url(url)
    {
        SetEventObject(generator);
    }

	wxHyperlinkEvent& operator=(const wxHyperlinkEvent&) = delete;

    // Returns the URL associated with the hyperlink control
    // that the user clicked on.
    const std::string& GetURL() const { return m_url; }
    void SetURL(const std::string &url) { m_url = url; }

    // default copy ctor, assignment operator and dtor are ok
    wxEvent *Clone() const override { return new wxHyperlinkEvent(*this); }

private:

    // URL associated with the hyperlink control that the used clicked on.
    std::string m_url;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};


// ----------------------------------------------------------------------------
// event types and macros
// ----------------------------------------------------------------------------

typedef void (wxEvtHandler::*wxHyperlinkEventFunction)(wxHyperlinkEvent&);

#define wxHyperlinkEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxHyperlinkEventFunction, func)

#define EVT_HYPERLINK(id, fn) \
    wx__DECLARE_EVT1(wxEVT_HYPERLINK, id, wxHyperlinkEventHandler(fn))


#if defined(__WXGTK210__) && !defined(__WXUNIVERSAL__)
    #include "wx/gtk/hyperlink.h"
// Note that the native control is only available in Unicode version under MSW.
#elif defined(__WXMSW__) && !defined(__WXUNIVERSAL__)
    #include "wx/msw/hyperlink.h"
#else
    #include "wx/generic/hyperlink.h"

    class wxHyperlinkCtrl : public wxGenericHyperlinkCtrl
    {
    public:
        wxHyperlinkCtrl() = default;

        wxHyperlinkCtrl(wxWindow *parent,
                        wxWindowID id,
                        const std::string& label,
                        const std::string& url,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        unsigned int style = wxHL_DEFAULT_STYLE,
                        const std::string& name = wxHyperlinkCtrlNameStr)
            : wxGenericHyperlinkCtrl(parent, id, label, url, pos, size,
                                     style, name)
        {
        }
        
        wxHyperlinkCtrl& operator=(wxHyperlinkCtrl&&) = delete;

        wxClassInfo *wxGetClassInfo() const;
        static wxClassInfo ms_classInfo;
        static wxObject* wxCreateObject();
    };
#endif

// old wxEVT_COMMAND_* constants
#define wxEVT_COMMAND_HYPERLINK   wxEVT_HYPERLINK

#endif // wxUSE_HYPERLINKCTRL

#endif // _WX_HYPERLINK_H_
