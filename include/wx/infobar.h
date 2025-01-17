///////////////////////////////////////////////////////////////////////////////
// Name:        wx/infobar.h
// Purpose:     declaration of wxInfoBarBase defining common API of wxInfoBar
// Author:      Vadim Zeitlin
// Created:     2009-07-28
// Copyright:   (c) 2009 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_INFOBAR_H_
#define _WX_INFOBAR_H_

#if wxUSE_INFOBAR

#include "wx/control.h"

import <string>;

// ----------------------------------------------------------------------------
// wxInfoBar shows non-critical but important information to the user
// ----------------------------------------------------------------------------

class wxInfoBarBase : public wxControl
{
public:
    // real ctors are provided by the derived classes, just notice that unlike
    // most of the other windows, info bar is created hidden and must be
    // explicitly shown when it is needed (this is done because it is supposed
    // to be shown only intermittently and hiding it after creating it from the
    // user code would result in flicker)
    wxInfoBarBase& operator=(wxInfoBarBase&&) = delete;

    // show the info bar with the given message and optionally an icon
    virtual void ShowMessage(const std::string& msg,
                             unsigned int flags) = 0;

    // hide the info bar
    virtual void Dismiss() = 0;

    // add an extra button to the bar, near the message (replacing the default
    // close button which is only shown if no extra buttons are used)
    virtual void AddButton(wxWindowID btnid,
                           const std::string& label) = 0;

    // remove a button previously added by AddButton()
    virtual void RemoveButton(wxWindowID btnid) = 0;

    // get information about the currently shown buttons
    virtual size_t GetButtonCount() const = 0;
    virtual wxWindowID GetButtonId(size_t idx) const = 0;
    virtual bool HasButtonId(wxWindowID btnid) const = 0;
};

// currently only GTK+ has a native implementation
#if defined(__WXGTK218__) && !defined(__WXUNIVERSAL__)
    #include "wx/gtk/infobar.h"
    #define wxHAS_NATIVE_INFOBAR
#endif // wxGTK2

// if the generic version is the only one we have, use it
#ifndef wxHAS_NATIVE_INFOBAR
    #include "wx/generic/infobar.h"
    using wxInfoBar = wxInfoBarGeneric;
#endif

#endif // wxUSE_INFOBAR

#endif // _WX_INFOBAR_H_
