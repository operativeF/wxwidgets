/////////////////////////////////////////////////////////////////////////////
// Name:        wx/validate.h
// Purpose:     wxValidator class
// Author:      Julian Smart
// Modified by:
// Created:     29/01/98
// Copyright:   (c) 1998 Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_VALIDATE_H_
#define _WX_VALIDATE_H_

#if wxUSE_VALIDATORS

#include "wx/defs.h"

#include "wx/event.h"

class wxWindow;

/*
 A validator has up to three purposes:

 1) To validate the data in the window that's associated
    with the validator.
 2) To transfer data to and from the window.
 3) To filter input, using its role as a wxEvtHandler
    to intercept e.g. OnChar.

 Note that wxValidator and derived classes use reference counting.
*/

class wxValidator : public wxEvtHandler
{
public:
    wxValidator() = default;
    wxValidator(const wxValidator& other)
        : 
         m_validatorWindow(other.m_validatorWindow)
    {
    }

    wxValidator& operator=(const wxValidator&) = delete;

    // Make a clone of this validator (or return NULL) - currently necessary
    // if you're passing a reference to a validator.
    // Another possibility is to always pass a pointer to a new validator
    // (so the calling code can use a copy constructor of the relevant class).
    virtual wxObject *Clone() const
        { return nullptr; }
    bool Copy(const wxValidator& val)
        { m_validatorWindow = val.m_validatorWindow; return true; }

    // Called when the value in the window must be validated.
    // This function can pop up an error message.
    virtual bool Validate(wxWindow *WXUNUSED(parent)) { return false; }

    // Called to transfer data to the window
    virtual bool TransferToWindow() { return false; }

    // Called to transfer data from the window
    virtual bool TransferFromWindow() { return false; }

    // Called when the validator is associated with a window, may be useful to
    // override if it needs to somehow initialize the window.
    virtual void SetWindow(wxWindow *win) { m_validatorWindow = win; }

    wxWindow *GetWindow() const { return m_validatorWindow; }

    // validators beep by default if invalid key is pressed, this function
    // allows to change this
    static void SuppressBellOnError(bool suppress = true)
        { ms_isSilent = suppress; }

    // test if beep is currently disabled
    static bool IsSilent() { return ms_isSilent; }

protected:
    wxWindow *m_validatorWindow{nullptr};

private:
    inline static bool ms_isSilent{true};

    wxDECLARE_DYNAMIC_CLASS(wxValidator);
};

#define wxVALIDATOR_PARAM(val) val

extern WXDLLIMPEXP_DATA_CORE(const wxValidator) wxDefaultValidator;

#else // !wxUSE_VALIDATORS
    // wxWidgets is compiled without support for wxValidator, but we still
    // want to be able to pass wxDefaultValidator to the functions which take
    // a wxValidator parameter to avoid using "#if wxUSE_VALIDATORS"
    // everywhere
    class wxValidator { };
    #define wxDefaultValidator wxValidator()

    // this macro allows to avoid warnings about unused parameters when
    // wxUSE_VALIDATORS == 0
    #define wxVALIDATOR_PARAM(val)
#endif // wxUSE_VALIDATORS/!wxUSE_VALIDATORS

#endif // _WX_VALIDATE_H_
