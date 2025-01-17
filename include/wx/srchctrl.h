/////////////////////////////////////////////////////////////////////////////
// Name:        wx/srchctrl.h
// Purpose:     wxSearchCtrlBase class
// Author:      Vince Harron
// Created:     2006-02-18
// Copyright:   (c) Vince Harron
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SEARCHCTRL_H_BASE_
#define _WX_SEARCHCTRL_H_BASE_

#if wxUSE_SEARCHCTRL

#include "wx/textctrl.h"

import <string>;

#if (!defined(__WXMAC__) && !defined(__WXGTK20__)) || defined(__WXUNIVERSAL__)
    // no native version, use the generic one
    #define wxUSE_NATIVE_SEARCH_CONTROL 0

    #include "wx/compositewin.h"
    #include "wx/containr.h"

    class wxSearchCtrlBaseBaseClass
        : public wxCompositeWindow< wxNavigationEnabled<wxControl> >,
          public wxTextCtrlIface
    {
    };
#elif defined(__WXMAC__)
    // search control was introduced in Mac OS X 10.3 Panther
    #define wxUSE_NATIVE_SEARCH_CONTROL 1

    #define wxSearchCtrlBaseBaseClass wxTextCtrl
#elif defined(__WXGTK20__)
    // Use GtkSearchEntry if available, construct a similar one using GtkEntry
    // otherwise.
    #define wxUSE_NATIVE_SEARCH_CONTROL 1

    class wxGTKSearchCtrlBase
        : public wxControl, public wxTextEntry
    {
    };

    #define wxSearchCtrlBaseBaseClass wxGTKSearchCtrlBase
#endif

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

inline constexpr std::string_view wxSearchCtrlNameStr = "searchCtrl";

wxDECLARE_EVENT( wxEVT_SEARCH_CANCEL, wxCommandEvent);
wxDECLARE_EVENT( wxEVT_SEARCH, wxCommandEvent);

// ----------------------------------------------------------------------------
// a search ctrl is a text control with a search button and a cancel button
// it is based on the MacOSX 10.3 control HISearchFieldCreate
// ----------------------------------------------------------------------------

class wxSearchCtrlBase : public wxSearchCtrlBaseBaseClass
{
public:
    // search control
#if wxUSE_MENUS
    virtual void SetMenu(wxMenu *menu) = 0;
    virtual wxMenu *GetMenu() = 0;
#endif // wxUSE_MENUS

    // get/set options
    virtual void ShowSearchButton( bool show ) = 0;
    virtual bool IsSearchButtonVisible() const = 0;

    virtual void ShowCancelButton( bool show ) = 0;
    virtual bool IsCancelButtonVisible() const = 0;

    virtual void SetDescriptiveText(const std::string& text) = 0;
    virtual std::string GetDescriptiveText() const = 0;

#if wxUSE_NATIVE_SEARCH_CONTROL
    const wxTextEntry* WXGetTextEntry() const override { return this; }
#endif // wxUSE_NATIVE_SEARCH_CONTROL

private:
    // implement wxTextEntry pure virtual method
    wxWindow *GetEditableWindow() override { return this; }
};


// include the platform-dependent class implementation
#if wxUSE_NATIVE_SEARCH_CONTROL
    #if defined(__WXMAC__)
        #include "wx/osx/srchctrl.h"
    #elif defined(__WXGTK__)
        #include "wx/gtk/srchctrl.h"
    #endif
#else
    #include "wx/generic/srchctlg.h"
#endif

// ----------------------------------------------------------------------------
// macros for handling search events
// ----------------------------------------------------------------------------

#define EVT_SEARCH_CANCEL(id, fn) \
    wx__DECLARE_EVT1(wxEVT_SEARCH_CANCEL, id, wxCommandEventHandler(fn))

#define EVT_SEARCH(id, fn) \
    wx__DECLARE_EVT1(wxEVT_SEARCH, id, wxCommandEventHandler(fn))

#endif // wxUSE_SEARCHCTRL

#endif // _WX_SEARCHCTRL_H_BASE_
