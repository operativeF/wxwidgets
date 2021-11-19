///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/textentrycmn.cpp
// Purpose:     wxTextEntryBase implementation
// Author:      Vadim Zeitlin
// Created:     2007-09-26
// Copyright:   (c) 2007 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////




#if wxUSE_TEXTCTRL || wxUSE_COMBOBOX

#include "wx/window.h"
#include "wx/dataobj.h"
#include "wx/textctrl.h"            // Only needed for wxTE_PASSWORD.
#include "wx/textentry.h"
#include "wx/textcompleter.h"
#include "wx/clipbrd.h"

import Utils.Strings;

// ----------------------------------------------------------------------------
// text accessors
// ----------------------------------------------------------------------------

std::string wxTextEntryBase::GetValue() const
{
    return m_hintData ? m_hintData->GetText() : DoGetValue();
}

std::string wxTextEntryBase::GetRange(long from, long to) const
{
    std::string sel;
    std::string value = GetValue();

    if ( from < to && (long)value.length() >= to )
    {
        sel = value.substr(from, to - from);
    }

    return sel;
}

// ----------------------------------------------------------------------------
// text operations
// ----------------------------------------------------------------------------

void wxTextEntryBase::ChangeValue(std::string_view value)
{
    DoSetValue(value, SetValue_NoEvent);

    // As we didn't generate any events for wxTextEntryHintData to catch,
    // notify it explicitly about our changed contents.
    if ( m_hintData )
        m_hintData->HandleTextUpdate(value);
}

void wxTextEntryBase::AppendText(const std::string& text)
{
    SetInsertionPointEnd();
    WriteText(text);
}

void wxTextEntryBase::DoSetValue(std::string_view value, unsigned int flags)
{
    if ( value != DoGetValue() )
    {
        EventsSuppressor noeventsIf(this, !(flags & SetValue_SendEvent));

        SelectAll();
        WriteText(value);

        SetInsertionPoint(0);
    }
    else // Same value, no need to do anything.
    {
        // Except that we still need to generate the event for consistency with
        // the normal case when the text does change.
        if ( flags & SetValue_SendEvent )
            SendTextUpdatedEvent(GetEditableWindow());
    }
}

void wxTextEntryBase::Replace(long from, long to, const std::string& value)
{
    {
        EventsSuppressor noevents(this);
        Remove(from, to);
    }

    SetInsertionPoint(from);
    WriteText(value);
}

// ----------------------------------------------------------------------------
// selection
// ----------------------------------------------------------------------------

bool wxTextEntryBase::HasSelection() const
{
    // TODO: Return pair
    long from, to;
    GetSelection(&from, &to);

    return from < to;
}

void wxTextEntryBase::RemoveSelection()
{
    long from, to;
    GetSelection(& from, & to);
    if (from != -1 && to != -1)
        Remove(from, to);
}

std::string wxTextEntryBase::GetStringSelection() const
{
    long from, to;
    GetSelection(&from, &to);

    return GetRange(from, to);
}

// ----------------------------------------------------------------------------
// clipboard
// ----------------------------------------------------------------------------

bool wxTextEntryBase::CanCopy() const
{
    return HasSelection();
}

bool wxTextEntryBase::CanCut() const
{
    return CanCopy() && IsEditable();
}

bool wxTextEntryBase::CanPaste() const
{
    if ( IsEditable() )
    {
#if wxUSE_CLIPBOARD
        // check if there is any text on the clipboard
        if ( wxTheClipboard->IsSupported(wxDF_TEXT)
                || wxTheClipboard->IsSupported(wxDF_UNICODETEXT)
           )
        {
            return true;
        }
#endif // wxUSE_CLIPBOARD
    }

    return false;
}

// ----------------------------------------------------------------------------
// input restrictions
// ----------------------------------------------------------------------------

#ifndef wxHAS_NATIVE_TEXT_FORCEUPPER

namespace
{

// Poor man's lambda: helper for binding ConvertToUpperCase() to the event
struct ForceUpperFunctor
{
    // This class must have default ctor in wxNO_RTTI case, so allow creating
    // it with null entry even if this never actually happens in practice.
    explicit ForceUpperFunctor(wxTextEntryBase* entry = NULL)
        : m_entry(entry)
    {
    }

    void operator()(wxCommandEvent& event)
    {
        event.Skip();
        m_entry->ConvertToUpperCase();
    }

    wxTextEntryBase* const m_entry;
};

} // anonymous namespace

#endif // !wxHAS_NATIVE_TEXT_FORCEUPPER

void wxTextEntryBase::ConvertToUpperCase()
{
    std::string valueOld = GetValue();
    std::string valueNew = wx::utils::ToUpperCopy(valueOld);

    if ( valueNew != valueOld )
    {
        long from, to;
        GetSelection(&from, &to);
        ChangeValue(valueNew);
        SetSelection(from, to);
    }
}

void wxTextEntryBase::ForceUpper()
{
    // Do nothing if this method is never called because a native override is
    // provided: this is just a tiny size-saving optimization, nothing else.
#ifndef wxHAS_NATIVE_TEXT_FORCEUPPER
    wxWindow* const win = GetEditableWindow();
    wxCHECK_RET( win, "can't be called before creating the window" );

    // Convert the current control contents to upper case
    ConvertToUpperCase();

    // And ensure that any text entered in the future is converted too
    win->Bind(wxEVT_TEXT, ForceUpperFunctor(this));
#endif // !wxHAS_NATIVE_TEXT_FORCEUPPER
}

// ----------------------------------------------------------------------------
// hints support
// ----------------------------------------------------------------------------

bool wxTextEntryBase::SetHint(const std::string& hint)
{
    // Hint contents would be shown hidden in a password text entry anyhow, so
    // we just can't support hints in this case.
    if ( GetEditableWindow()->HasFlag(wxTE_PASSWORD) )
        return false;

    if ( !hint.empty() )
    {
        if ( !m_hintData )
            m_hintData = std::make_unique<wxTextEntryHintData>(this, GetEditableWindow());

        m_hintData->SetHintString(hint);
    }
    else if ( m_hintData )
    {
        // Setting empty hint removes any currently set one.
        m_hintData.reset();
    }
    //else: Setting empty hint when we don't have any doesn't do anything.

    return true;
}

std::string wxTextEntryBase::GetHint() const
{
    return m_hintData ? m_hintData->GetHintString() : "";
}

// ----------------------------------------------------------------------------
// margins support
// ----------------------------------------------------------------------------

bool wxTextEntryBase::DoSetMargins(const wxPoint& WXUNUSED(pt))
{
    return false;
}

wxPoint wxTextEntryBase::DoGetMargins() const
{
    return {-1, -1};
}

// ----------------------------------------------------------------------------
// events
// ----------------------------------------------------------------------------

/* static */
bool wxTextEntryBase::SendTextUpdatedEvent(wxWindow *win)
{
    wxCHECK_MSG( win, false, "can't send an event without a window" );

    wxCommandEvent event(wxEVT_TEXT, win->GetId());

    // do not do this as it could be very inefficient if the text control
    // contains a lot of text and we're not using ref-counted std::string
    // implementation -- instead, event.GetString() will query the control for
    // its current text if needed
    //event.SetString(win->GetValue());

    event.SetEventObject(win);
    return win->HandleWindowEvent(event);
}

// ----------------------------------------------------------------------------
// auto-completion stubs
// ----------------------------------------------------------------------------

bool wxTextCompleterSimple::Start(const std::string& prefix)
{
    m_index = 0;
    m_completions.clear();
    m_completions = GetCompletions(prefix);

    return !m_completions.empty();
}

std::string wxTextCompleterSimple::GetNext()
{
    if ( m_index == m_completions.size() )
        return {};

    return m_completions[m_index++];
}

bool wxTextEntryBase::DoAutoCompleteCustom(wxTextCompleter *completer)
{
    // We don't do anything here but we still need to delete the completer for
    // consistency with the ports that do implement this method and take
    // ownership of the pointer.
    delete completer;

    return false;
}

#endif // wxUSE_TEXTCTRL || wxUSE_COMBOBOX
