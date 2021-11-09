/////////////////////////////////////////////////////////////////////////////
// Name:        wx/panel.h
// Purpose:     Base header for wxPanel
// Author:      Julian Smart
// Modified by:
// Created:
// Copyright:   (c) Julian Smart
//              (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PANEL_H_BASE_
#define _WX_PANEL_H_BASE_

// ----------------------------------------------------------------------------
// headers and forward declarations
// ----------------------------------------------------------------------------

#include "wx/window.h"
#include "wx/containr.h"

import <string>;

class wxControlContainer;

// ----------------------------------------------------------------------------
// wxPanel contains other controls and implements TAB traversal between them
// ----------------------------------------------------------------------------

class wxPanelBase : public wxNavigationEnabled<wxWindow>
{
public:
    wxPanelBase& operator=(wxPanelBase&&) = delete;

    // Derived classes should also provide this constructor:
    /*
    wxPanelBase(wxWindow *parent,
                wxWindowID winid = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxTAB_TRAVERSAL | wxNO_BORDER,
                const std::string& name = wxPanelNameStr);
    */

    // Pseudo ctor
    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID winid = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxTAB_TRAVERSAL | wxNO_BORDER,
                const std::string& name = wxPanelNameStr);


    // implementation from now on
    // --------------------------

    void InitDialog() override;
};

#if defined(__WXUNIVERSAL__)
    #include "wx/univ/panel.h"
#elif defined(__WXMSW__)
    #include "wx/msw/panel.h"
#else
    #define wxHAS_GENERIC_PANEL
    #include "wx/generic/panelg.h"
#endif

#endif // _WX_PANELH_BASE_
