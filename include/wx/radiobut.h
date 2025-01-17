///////////////////////////////////////////////////////////////////////////////
// Name:        wx/radiobut.h
// Purpose:     wxRadioButton declaration
// Author:      Vadim Zeitlin
// Modified by:
// Created:     07.09.00
// Copyright:   (c) Vadim Zeitlin
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_RADIOBUT_H_BASE_
#define _WX_RADIOBUT_H_BASE_

#if wxUSE_RADIOBTN

#include "wx/control.h"

class wxRadioButton;

/*
 * wxRadioButton style flag
 */
#define wxRB_GROUP          0x0004
#define wxRB_SINGLE         0x0008

// TODO: In wxUniv, wxRadioButton must derive from wxCheckBox as it reuses
// much of its code. This should be fixed by refactoring wxCheckBox to allow
// this class to reuse its functionality without inheriting from it, but for
// now use this hack to allow the existing code to compile.
#ifdef __WXUNIVERSAL__
    #include "wx/checkbox.h"

    using wxRadioButtonBaseBase = wxCheckBox;
#else
    using wxRadioButtonBaseBase = wxControl;
#endif

class wxRadioButtonBase : public wxRadioButtonBaseBase
{
public:
    wxRadioButtonBase& operator=(wxRadioButtonBase&&) = delete;

    // Methods to be implemented by the derived classes:
    virtual void SetValue(bool value) = 0;
    virtual bool GetValue() const = 0;


    // Methods implemented by this class itself.
    wxRadioButton* GetFirstInGroup() const;
    wxRadioButton* GetLastInGroup() const;
    wxRadioButton* GetPreviousInGroup() const;
    wxRadioButton* GetNextInGroup() const;
};

inline constexpr std::string_view wxRadioButtonNameStr = "radioButton";

#if defined(__WXUNIVERSAL__)
    #include "wx/univ/radiobut.h"
#elif defined(__WXMSW__)
    #include "wx/msw/radiobut.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/radiobut.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/radiobut.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/radiobut.h"
#elif defined(__WXMAC__)
    #include "wx/osx/radiobut.h"
#elif defined(__WXQT__)
    #include "wx/qt/radiobut.h"
#endif

#endif // wxUSE_RADIOBTN

#endif
    // _WX_RADIOBUT_H_BASE_
