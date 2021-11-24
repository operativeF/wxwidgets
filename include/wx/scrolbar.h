/////////////////////////////////////////////////////////////////////////////
// Name:        wx/scrolbar.h
// Purpose:     wxScrollBar base header
// Author:      Julian Smart
// Modified by:
// Created:
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SCROLBAR_H_BASE_
#define _WX_SCROLBAR_H_BASE_

#if wxUSE_SCROLLBAR

#include "wx/control.h"

import <string>;
import <string_view>;

inline constexpr std::string_view wxScrollBarNameStr = "scrollBar";

// ----------------------------------------------------------------------------
// wxScrollBar: a scroll bar control
// ----------------------------------------------------------------------------

class wxScrollBarBase : public wxControl
{
public:
    wxScrollBarBase& operator=(wxScrollBarBase&&) = delete;

    /*
        Derived classes should provide the following method and ctor with the
        same parameters:

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxSB_HORIZONTAL,
                const wxValidator& validator = wxDefaultValidator,
                const std::string& name = wxScrollBarNameStr);
    */

    virtual int GetThumbPosition() const = 0;
    virtual int GetThumbSize() const = 0;
    virtual int GetPageSize() const = 0;
    virtual int GetRange() const = 0;

    bool IsVertical() const { return (m_windowStyle & wxVERTICAL) != 0; }

    // operations
    virtual void SetThumbPosition(int viewStart) = 0;
    void SetScrollbar(int position, int thumbSize,
                              int range, int pageSize,
                              bool refresh = true) override = 0;

    // implementation-only
    bool IsNeeded() const { return GetRange() > GetThumbSize(); }
};

#if defined(__WXUNIVERSAL__)
    #include "wx/univ/scrolbar.h"
#elif defined(__WXMSW__)
    #include "wx/msw/scrolbar.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/scrolbar.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/scrolbar.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/scrolbar.h"
#elif defined(__WXMAC__)
    #include "wx/osx/scrolbar.h"
#elif defined(__WXQT__)
    #include "wx/qt/scrolbar.h"
#endif

#endif // wxUSE_SCROLLBAR

#endif
    // _WX_SCROLBAR_H_BASE_
