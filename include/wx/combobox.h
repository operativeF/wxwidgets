///////////////////////////////////////////////////////////////////////////////
// Name:        wx/combobox.h
// Purpose:     wxComboBox declaration
// Author:      Vadim Zeitlin
// Modified by:
// Created:     24.12.00
// Copyright:   (c) 1996-2000 wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_COMBOBOX_H_BASE_
#define _WX_COMBOBOX_H_BASE_

#include "wx/defs.h"

#if wxUSE_COMBOBOX

inline constexpr std::string_view wxComboBoxNameStr = "comboBox";

// ----------------------------------------------------------------------------
// wxComboBoxBase: this interface defines the methods wxComboBox must implement
// ----------------------------------------------------------------------------

#include "wx/ctrlsub.h"
#include "wx/textentry.h"

class wxComboBoxBase : public wxItemContainer,
                                        public wxTextEntry
{
public:
    // override these methods to disambiguate between two base classes versions
    void Clear() override
    {
        wxItemContainer::Clear();
        wxTextEntry::Clear();
    }

    // IsEmpty() is ambiguous because we inherit it from both wxItemContainer
    // and wxTextEntry, and even if defined it here to help the compiler with
    // choosing one of them, it would still be confusing for the human users of
    // this class. So instead define the clearly named methods below and leave
    // IsEmpty() ambiguous to trigger a compilation error if it's used.
    bool IsListEmpty() const { return wxItemContainer::IsEmpty(); }
    bool IsTextEmpty() const { return wxTextEntry::IsEmpty(); }

    // also bring in GetSelection() versions of both base classes in scope
    //
    // NB: GetSelection(from, to) could be already implemented in wxTextEntry
    //     but still make it pure virtual because for some platforms it's not
    //     implemented there and also because the derived class has to override
    //     it anyhow to avoid ambiguity with the other GetSelection()
    int GetSelection() const override = 0;
    void GetSelection(long *from, long *to) const override = 0;

    virtual void Popup() { wxFAIL_MSG( "Not implemented" ); }
    virtual void Dismiss() { wxFAIL_MSG( "Not implemented" ); }

    // may return value different from GetSelection() when the combobox
    // dropdown is shown and the user selected, but not yet accepted, a value
    // different from the old one in it
    virtual int GetCurrentSelection() const { return GetSelection(); }
};

// ----------------------------------------------------------------------------
// include the platform-dependent header defining the real class
// ----------------------------------------------------------------------------

#if defined(__WXUNIVERSAL__)
    #include "wx/univ/combobox.h"
#elif defined(__WXMSW__)
    #include "wx/msw/combobox.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/combobox.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/combobox.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/combobox.h"
#elif defined(__WXMAC__)
    #include "wx/osx/combobox.h"
#elif defined(__WXQT__)
    #include "wx/qt/combobox.h"
#endif

#endif // wxUSE_COMBOBOX

#endif // _WX_COMBOBOX_H_BASE_
