///////////////////////////////////////////////////////////////////////////////
// Name:        wx/notebook.h
// Purpose:     wxNotebook interface
// Author:      Vadim Zeitlin
// Modified by:
// Created:     01.02.01
// Copyright:   (c) 1996-2000 Vadim Zeitlin
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_NOTEBOOK_H_BASE_
#define _WX_NOTEBOOK_H_BASE_

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#if wxUSE_NOTEBOOK

#include "wx/bookctrl.h"

import <string>;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// wxNotebook hit results, use wxBK_HITTEST so other book controls can share them
// if wxUSE_NOTEBOOK is disabled
enum
{
    wxNB_HITTEST_NOWHERE = wxBK_HITTEST_NOWHERE,
    wxNB_HITTEST_ONICON  = wxBK_HITTEST_ONICON,
    wxNB_HITTEST_ONLABEL = wxBK_HITTEST_ONLABEL,
    wxNB_HITTEST_ONITEM  = wxBK_HITTEST_ONITEM,
    wxNB_HITTEST_ONPAGE  = wxBK_HITTEST_ONPAGE
};

// wxNotebook flags

// use common book wxBK_* flags for describing alignment
inline constexpr unsigned int wxNB_DEFAULT = wxBK_DEFAULT;
inline constexpr unsigned int wxNB_TOP     = wxBK_TOP;
inline constexpr unsigned int wxNB_BOTTOM  = wxBK_BOTTOM;
inline constexpr unsigned int wxNB_LEFT    = wxBK_LEFT;
inline constexpr unsigned int wxNB_RIGHT   = wxBK_RIGHT;

inline constexpr unsigned int wxNB_FIXEDWIDTH  = 0x0100;
inline constexpr unsigned int wxNB_MULTILINE   = 0x0200;
inline constexpr unsigned int wxNB_NOPAGETHEME = 0x0400;


using wxNotebookPage = wxWindow;  // so far, any window can be a page

inline constexpr std::string_view wxNotebookNameStr = "notebook";

// ----------------------------------------------------------------------------
// wxNotebookBase: define wxNotebook interface
// ----------------------------------------------------------------------------

class wxNotebookBase : public wxBookCtrlBase
{
public:
    wxNotebookBase& operator=(wxNotebookBase&&) = delete;

    // wxNotebook-specific additions to wxBookCtrlBase interface
    // ---------------------------------------------------------

    // get the number of rows for a control with wxNB_MULTILINE style (not all
    // versions support it - they will always return 1 then)
    virtual int GetRowCount() const { return 1; }

    // set the padding between tabs (in pixels)
    virtual void SetPadding(const wxSize& padding) = 0;

    // set the size of the tabs for wxNB_FIXEDWIDTH controls
    virtual void SetTabSize(const wxSize& sz) = 0;

    // implement some base class functions
    wxSize CalcSizeFromPage(const wxSize& sizePage) const override;

    // On platforms that support it, get the theme page background colour, else invalid colour
    virtual wxColour GetThemeBackgroundColour() const { return wxNullColour; }

    // send wxEVT_NOTEBOOK_PAGE_CHANGING/ED events

    // returns false if the change to nPage is vetoed by the program
    bool SendPageChangingEvent(int nPage);

    // sends the event about page change from old to new (or GetSelection() if
    // new is wxNOT_FOUND)
    void SendPageChangedEvent(int nPageOld, int nPageNew = wxNOT_FOUND);
};

// ----------------------------------------------------------------------------
// notebook event class and related stuff
// ----------------------------------------------------------------------------

// wxNotebookEvent is obsolete and defined for compatibility only (notice that
// we use #define and not typedef to also keep compatibility with the existing
// code which forward declares it)
#define wxNotebookEvent wxBookCtrlEvent
using wxNotebookEventFunction = wxBookCtrlEventFunction;
#define wxNotebookEventHandler(func) wxBookCtrlEventHandler(func)

wxDECLARE_EVENT( wxEVT_NOTEBOOK_PAGE_CHANGED, wxBookCtrlEvent );
wxDECLARE_EVENT( wxEVT_NOTEBOOK_PAGE_CHANGING, wxBookCtrlEvent );

#define EVT_NOTEBOOK_PAGE_CHANGED(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_NOTEBOOK_PAGE_CHANGED, winid, wxBookCtrlEventHandler(fn))

#define EVT_NOTEBOOK_PAGE_CHANGING(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_NOTEBOOK_PAGE_CHANGING, winid, wxBookCtrlEventHandler(fn))

// ----------------------------------------------------------------------------
// wxNotebook class itself
// ----------------------------------------------------------------------------

#if defined(__WXUNIVERSAL__)
    #include "wx/univ/notebook.h"
#elif defined(__WXMSW__)
    #include  "wx/msw/notebook.h"
#elif defined(__WXMOTIF__)
    #include  "wx/generic/notebook.h"
#elif defined(__WXGTK20__)
    #include  "wx/gtk/notebook.h"
#elif defined(__WXGTK__)
    #include  "wx/gtk1/notebook.h"
#elif defined(__WXMAC__)
    #include  "wx/osx/notebook.h"
#elif defined(__WXQT__)
    #include "wx/qt/notebook.h"
#endif

#endif // wxUSE_NOTEBOOK

#endif
    // _WX_NOTEBOOK_H_BASE_
