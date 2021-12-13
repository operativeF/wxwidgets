/////////////////////////////////////////////////////////////////////////////
// Name:        wx/collpane.h
// Purpose:     wxCollapsiblePane
// Author:      Francesco Montorsi
// Modified by:
// Created:     8/10/2006
// Copyright:   (c) Francesco Montorsi
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_COLLAPSABLE_PANE_H_BASE_
#define _WX_COLLAPSABLE_PANE_H_BASE_

#if wxUSE_COLLPANE

#include "wx/control.h"

import <string>;

// class name
inline constexpr std::string_view wxCollapsiblePaneNameStr = "collapsiblePane";

// ----------------------------------------------------------------------------
// wxCollapsiblePaneBase: interface for wxCollapsiblePane
// ----------------------------------------------------------------------------

inline constexpr unsigned int wxCP_DEFAULT_STYLE          = wxTAB_TRAVERSAL | wxNO_BORDER;
inline constexpr unsigned int wxCP_NO_TLW_RESIZE          = 0x0002;

class wxCollapsiblePaneBase : public wxControl
{
public:
    virtual void Collapse(bool collapse = true) = 0;
    void Expand() { Collapse(false); }

    virtual bool IsCollapsed() const = 0;
    bool IsExpanded() const { return !IsCollapsed(); }

    virtual wxWindow *GetPane() const = 0;

    std::string GetLabel() const override = 0;
    void SetLabel(std::string_view label) override = 0;

    bool
    InformFirstDirection(int direction,
                         int size,
                         int availableOtherDir) override
    {
        wxWindow* const p = GetPane();
        if ( !p )
            return false;

        if ( !p->InformFirstDirection(direction, size, availableOtherDir) )
            return false;

        InvalidateBestSize();

        return true;
    }
};


// ----------------------------------------------------------------------------
// event types and macros
// ----------------------------------------------------------------------------

class wxCollapsiblePaneEvent;

wxDECLARE_EVENT( wxEVT_COLLAPSIBLEPANE_CHANGED, wxCollapsiblePaneEvent );

class wxCollapsiblePaneEvent : public wxCommandEvent
{
public:
    wxCollapsiblePaneEvent() = default;
    wxCollapsiblePaneEvent(wxObject *generator, int id, bool collapsed)
        : wxCommandEvent(wxEVT_COLLAPSIBLEPANE_CHANGED, id),
        m_bCollapsed(collapsed)
    {
        SetEventObject(generator);
    }

    bool GetCollapsed() const { return m_bCollapsed; }
    void SetCollapsed(bool c) { m_bCollapsed = c; }


    // default copy ctor, assignment operator and dtor are ok
    std::unique_ptr<wxEvent> Clone() const override { return std::make_unique<wxCollapsiblePaneEvent>(*this); }

private:
    bool m_bCollapsed;
};

// ----------------------------------------------------------------------------
// event types and macros
// ----------------------------------------------------------------------------

typedef void (wxEvtHandler::*wxCollapsiblePaneEventFunction)(wxCollapsiblePaneEvent&);

#define wxCollapsiblePaneEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxCollapsiblePaneEventFunction, func)

#define EVT_COLLAPSIBLEPANE_CHANGED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_COLLAPSIBLEPANE_CHANGED, id, wxCollapsiblePaneEventHandler(fn))


#if defined(__WXGTK20__) && !defined(__WXUNIVERSAL__)
    #include "wx/gtk/collpane.h"
#else
    #include "wx/generic/collpaneg.h"

    // use #define and not a typedef to allow forward declaring the class
    using wxCollapsiblePane = wxGenericCollapsiblePane;
#endif

#endif // wxUSE_COLLPANE

#endif // _WX_COLLAPSABLE_PANE_H_BASE_
