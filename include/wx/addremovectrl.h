///////////////////////////////////////////////////////////////////////////////
// Name:        wx/addremovectrl.h
// Purpose:     wxAddRemoveCtrl declaration.
// Author:      Vadim Zeitlin
// Created:     2015-01-29
// Copyright:   (c) 2015 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_ADDREMOVECTRL_H_
#define _WX_ADDREMOVECTRL_H_

#if wxUSE_ADDREMOVECTRL

#include "wx/panel.h"

import Utils.Geometry.Size;
import <string>;

inline constexpr std::string_view wxAddRemoveCtrlNameStr = "wxAddRemoveCtrl";

// ----------------------------------------------------------------------------
// wxAddRemoveAdaptor: used by wxAddRemoveCtrl to work with the list control
// ----------------------------------------------------------------------------

struct wxAddRemoveAdaptor
{
    virtual ~wxAddRemoveAdaptor() = default;

    wxAddRemoveAdaptor& operator=(wxAddRemoveAdaptor&&) = delete;

    // Override to return the associated control.
    virtual wxWindow* GetItemsCtrl() const = 0;

    // Override to return whether a new item can be added to the control.
    virtual bool CanAdd() const = 0;

    // Override to return whether the currently selected item (if any) can be
    // removed from the control.
    virtual bool CanRemove() const = 0;

    // Called when an item should be added, can only be called if CanAdd()
    // currently returns true.
    virtual void OnAdd() = 0;

    // Called when the current item should be removed, can only be called if
    // CanRemove() currently returns true.
    virtual void OnRemove() = 0;
};

// ----------------------------------------------------------------------------
// wxAddRemoveCtrl: a list-like control combined with add/remove buttons
// ----------------------------------------------------------------------------

class wxAddRemoveCtrl : public wxPanel
{
public:
    wxAddRemoveCtrl() = default;

    wxAddRemoveCtrl(wxWindow* parent,
                    wxWindowID winid = wxID_ANY,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    unsigned int style = 0,
                    std::string_view name = wxAddRemoveCtrlNameStr)
    {
        Create(parent, winid, pos, size, style, name);
    }

    wxAddRemoveCtrl& operator=(wxAddRemoveCtrl&&) = delete;

    [[maybe_unused]] bool Create(wxWindow* parent,
                wxWindowID winid = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                std::string_view name = wxAddRemoveCtrlNameStr);

    ~wxAddRemoveCtrl();

    // Must be called for the control to be usable, takes ownership of the
    // pointer.
    void SetAdaptor(wxAddRemoveAdaptor* adaptor);

    // Set tooltips to use for the add and remove buttons.
    void SetButtonsToolTips(const std::string& addtip, const std::string& removetip);

protected:
    wxSize DoGetBestClientSize() const override;

private:    
    class wxAddRemoveImpl* m_impl{nullptr};
};

#endif // wxUSE_ADDREMOVECTRL

#endif // _WX_ADDREMOVECTRL_H_
