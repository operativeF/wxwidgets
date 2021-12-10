/////////////////////////////////////////////////////////////////////////////
// Name:        wx/control.h
// Purpose:     wxControl common interface
// Author:      Vadim Zeitlin
// Modified by:
// Created:     26.07.99
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CONTROL_H_BASE_
#define _WX_CONTROL_H_BASE_

#if wxUSE_CONTROLS

#include "wx/window.h"      // base class
#include "wx/gdicmn.h"      // wxEllipsize...

import WX.Cfg.Flags;

import Utils.Geometry;

import <string>;
import <string_view>;

inline constexpr std::string_view wxControlNameStr = "control";


// ----------------------------------------------------------------------------
// wxControl is the base class for all controls
// ----------------------------------------------------------------------------

class wxControlBase : public wxWindow
{
public:
    wxControlBase& operator=(wxControlBase&&) = delete;

    // Create() function adds the validator parameter
    [[maybe_unused]] bool Create(wxWindow *parent, wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const wxValidator& validator = {},
                std::string_view name = wxControlNameStr);

    // get the control alignment (left/right/centre, top/bottom/centre)
    int GetAlignment() const { return m_windowStyle & wxALIGN_MASK; }

    // set label with mnemonics
    void SetLabel(std::string_view label) override
    {
        m_labelOrig = {label.begin(), label.end()};

        InvalidateBestSize();

        wxWindow::SetLabel(label);
    }

    // return the original string, as it was passed to SetLabel()
    // (i.e. with wx-style mnemonics)
    std::string GetLabel() const override { return m_labelOrig; }

    // set label text (mnemonics will be escaped)
    virtual void SetLabelText(const std::string& text)
    {
        SetLabel(EscapeMnemonics(text));
    }

    // get just the text of the label, without mnemonic characters ('&')
    virtual std::string GetLabelText() const { return GetLabelText(GetLabel()); }


#if wxUSE_MARKUP
    // Set the label with markup (and mnemonics). Markup is a simple subset of
    // HTML with tags such as <b>, <i> and <span>. By default it is not
    // supported i.e. all the markup is simply stripped and SetLabel() is
    // called but some controls in some ports do support this already and in
    // the future most of them should.
    //
    // Notice that, being HTML-like, markup also supports XML entities so '<'
    // should be encoded as "&lt;" and so on, a bare '<' in the input will
    // likely result in an error. As an exception, a bare '&' is allowed and
    // indicates that the next character is a mnemonic. To insert a literal '&'
    // in the control you need to use "&amp;" in the input string.
    //
    // Returns true if the label was set, even if the markup in it was ignored.
    // False is only returned if we failed to parse the label.
    bool SetLabelMarkup(const std::string& markup)
    {
        return DoSetLabelMarkup(markup);
    }
#endif // wxUSE_MARKUP


    // controls by default inherit the colours of their parents, if a
    // particular control class doesn't want to do it, it can override
    // ShouldInheritColours() to return false
    bool ShouldInheritColours() const override { return true; }


    // WARNING: this doesn't work for all controls nor all platforms!
    //
    // simulates the event of given type (i.e. wxButton::Command() is just as
    // if the button was clicked)
    virtual void Command(wxCommandEvent &event);

    bool SetFont(const wxFont& font) override;

    // wxControl-specific processing after processing the update event
    void DoUpdateWindowUI(wxUpdateUIEvent& event) override;

    wxSize GetSizeFromTextSize(int xlen, int ylen = -1) const
        { return DoGetSizeFromTextSize(xlen, ylen); }
    wxSize GetSizeFromTextSize(const wxSize& tsize) const
        { return DoGetSizeFromTextSize(tsize.x, tsize.y); }

    wxSize GetSizeFromText(const std::string& text) const
    {
        return GetSizeFromTextSize(GetTextExtent(text).x);
    }


    // static utilities for mnemonics char (&) handling
    // ------------------------------------------------

    // returns the given string without mnemonic characters ('&')
    static std::string GetLabelText(const std::string& label);

    // returns the given string without mnemonic characters ('&')
    // this function is identic to GetLabelText() and is provided for clarity
    // and for symmetry with the wxStaticText::RemoveMarkup() function.
    static std::string RemoveMnemonics(const std::string& str);

    // escapes (by doubling them) the mnemonics
    static std::string EscapeMnemonics(std::string_view str);


    // miscellaneous static utilities
    // ------------------------------

    // replaces parts of the given (multiline) string with an ellipsis if needed
    static std::string Ellipsize(std::string_view label, const wxDC& dc,
                              wxEllipsizeMode mode, int maxWidth,
                              EllipsizeFlags flags = wxEllipsizeFlags::Default);

    // return the accel index in the string or -1 if none and puts the modified
    // string into second parameter if non NULL
    static int FindAccelIndex(const std::string& label,
                              std::string* labelOnly = nullptr);

    // this is a helper for the derived class GetClassDefaultAttributes()
    // implementation: it returns the right colours for the classes which
    // contain something else (e.g. wxListBox, wxTextCtrl, ...) instead of
    // being simple controls (such as wxButton, wxCheckBox, ...)
    static wxVisualAttributes
        GetCompositeControlsDefaultAttributes(wxWindowVariant variant);

protected:
    // choose the default border for this window
    wxBorder GetDefaultBorder() const override;

    // creates the control (calls wxWindowBase::CreateBase inside) and adds it
    // to the list of parents children
    bool CreateControl(wxWindowBase *parent,
                       wxWindowID id,
                       const wxPoint& pos,
                       const wxSize& size,
                       unsigned int style,
                       const wxValidator& validator,
                       std::string_view name);

#if wxUSE_MARKUP
    // This function may be overridden in the derived classes to implement
    // support for labels with markup. The base class version simply strips the
    // markup and calls SetLabel() with the remaining text.
    virtual bool DoSetLabelMarkup(const std::string& markup);
#endif // wxUSE_MARKUP

    // override this to return the total control's size from a string size
    virtual wxSize DoGetSizeFromTextSize(int xlen, int ylen = -1) const;

    // initialize the common fields of wxCommandEvent
    void InitCommandEvent(wxCommandEvent& event) const;

#if wxUSE_MARKUP
    // Remove markup from the given string, returns empty string on error i.e.
    // if markup was syntactically invalid.
    static std::string RemoveMarkup(const std::string& markup);
#endif // wxUSE_MARKUP

    // this field contains the label in wx format, i.e. with '&' mnemonics,
    // as it was passed to the last SetLabel() call
    std::string m_labelOrig;
};

// ----------------------------------------------------------------------------
// include platform-dependent wxControl declarations
// ----------------------------------------------------------------------------

#if defined(__WXUNIVERSAL__)
    #include "wx/univ/control.h"
#elif defined(__WXMSW__)
    #include "wx/msw/control.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/control.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/control.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/control.h"
#elif defined(__WXMAC__)
    #include "wx/osx/control.h"
#elif defined(__WXQT__)
    #include "wx/qt/control.h"
#endif

#endif // wxUSE_CONTROLS

#endif
    // _WX_CONTROL_H_BASE_
