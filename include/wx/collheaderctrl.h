/////////////////////////////////////////////////////////////////////////////
// Name:        wx/collheaderctrl.h
// Purpose:     wxCollapsibleHeaderCtrl
// Author:      Tobias Taschner
// Created:     2015-09-19
// Copyright:   (c) 2015 wxWidgets development team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_COLLAPSIBLEHEADER_CTRL_H_
#define _WX_COLLAPSIBLEHEADER_CTRL_H_

#include "wx/defs.h"

#if wxUSE_COLLPANE

#include "wx/control.h"

import Utils.Geometry;

import <string>;

// class name
inline constexpr std::string_view wxCollapsibleHeaderCtrlNameStr = "collapsibleHeader";

//
// wxGenericCollapsibleHeaderCtrl
//

class wxCollapsibleHeaderCtrlBase : public wxControl
{
public:
    wxCollapsibleHeaderCtrlBase() = default;

    wxCollapsibleHeaderCtrlBase(wxWindow *parent,
        wxWindowID id,
        const std::string& label,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        unsigned int style = wxBORDER_NONE,
        const wxValidator& validator = wxDefaultValidator,
        std::string_view name = wxCollapsibleHeaderCtrlNameStr)
    {
        Create(parent, id, label, pos, size, style, validator, name);
    }

    wxCollapsibleHeaderCtrlBase& operator=(wxCollapsibleHeaderCtrlBase&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
        wxWindowID id,
        const std::string& label,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        unsigned int style = wxBORDER_NONE,
        const wxValidator& validator = wxDefaultValidator,
        std::string_view name = wxCollapsibleHeaderCtrlNameStr)
    {
        if ( !wxControl::Create(parent, id, pos, size, style, validator, name) )
            return false;

        SetLabel(label);

        return true;
    }

    virtual void SetCollapsed(bool collapsed = true) = 0;

    virtual bool IsCollapsed() const = 0;
};

wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_COLLAPSIBLEHEADER_CHANGED, wxCommandEvent);

#define wxCollapsibleHeaderChangedHandler(func) \
    wxEVENT_HANDLER_CAST(wxCommandEventFunction, func)

#define EVT_COLLAPSIBLEHEADER_CHANGED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_COLLAPSIBLEHEADER_CHANGED, id, wxCollapsibleHeaderChangedHandler(fn))

// Currently there is only the native implementation, use it for all ports.

#include "wx/generic/collheaderctrl.h"

class wxCollapsibleHeaderCtrl
    : public wxGenericCollapsibleHeaderCtrl
{
public:
    wxCollapsibleHeaderCtrl() = default;

    wxCollapsibleHeaderCtrl(wxWindow *parent,
        wxWindowID id,
        const std::string& label,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        unsigned int style = wxBORDER_NONE,
        const wxValidator& validator = wxDefaultValidator,
        std::string_view name = wxCollapsibleHeaderCtrlNameStr)
    {
        Create(parent, id, label, pos, size, style, validator, name);
    }

    wxCollapsibleHeaderCtrl& operator=(wxCollapsibleHeaderCtrl&&) = delete;
};

#endif // wxUSE_COLLPANE

#endif // _WX_COLLAPSIBLEHEADER_CTRL_H_
