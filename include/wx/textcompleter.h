///////////////////////////////////////////////////////////////////////////////
// Name:        wx/textcompleter.h
// Purpose:     Declaration of wxTextCompleter class.
// Author:      Vadim Zeitlin
// Created:     2011-04-13
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TEXTCOMPLETER_H_
#define _WX_TEXTCOMPLETER_H_

#include "wx/defs.h"

// ----------------------------------------------------------------------------
// wxTextCompleter: used by wxTextEnter::AutoComplete()
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxTextCompleter
{
public:
    wxTextCompleter() = default;

    // The virtual functions to be implemented by the derived classes: the
    // first one is called to start preparing for completions for the given
    // prefix and, if it returns true, GetNext() is called until it returns an
    // empty string indicating that there are no more completions.
    virtual bool Start(const wxString& prefix) = 0;
    virtual wxString GetNext() = 0;

    virtual ~wxTextCompleter();

   wxTextCompleter(const wxTextCompleter&) = delete;
   wxTextCompleter& operator=(const wxTextCompleter&) = delete;
   wxTextCompleter(wxTextCompleter&&) = default;
   wxTextCompleter& operator=(wxTextCompleter&&) = default;
};

// ----------------------------------------------------------------------------
// wxTextCompleterSimple: returns the entire set of completions at once
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxTextCompleterSimple : public wxTextCompleter
{
public:
    wxTextCompleterSimple() = default;

   wxTextCompleterSimple(const wxTextCompleterSimple&) = delete;
   wxTextCompleterSimple& operator=(const wxTextCompleterSimple&) = delete;
   wxTextCompleterSimple(wxTextCompleterSimple&&) = default;
   wxTextCompleterSimple& operator=(wxTextCompleterSimple&&) = default;

    // Must be implemented to return all the completions for the given prefix.
    virtual const std::vector<wxString>& GetCompletions(const wxString& prefix) = 0;

    bool Start(const wxString& prefix) override;
    wxString GetNext() override;

private:
    std::vector<wxString> m_completions;
    unsigned m_index{0};
};

// ----------------------------------------------------------------------------
// wxTextCompleterFixed: Trivial wxTextCompleter implementation which always
// returns the same fixed array of completions.
// ----------------------------------------------------------------------------

// NB: This class is private and intentionally not documented as it is
//     currently used only for implementation of completion with the fixed list
//     of strings only by wxWidgets itself, do not use it outside of wxWidgets.

class wxTextCompleterFixed : public wxTextCompleterSimple
{
public:
    void SetCompletions(const std::vector<wxString>& strings)
    {
        m_strings = strings;
    }

    const std::vector<wxString>& GetCompletions(const wxString& WXUNUSED(prefix)) override
    {
        return m_strings;
    }

private:
    std::vector<wxString> m_strings;
};


#endif // _WX_TEXTCOMPLETER_H_

