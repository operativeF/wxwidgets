///////////////////////////////////////////////////////////////////////////////
// Name:        wx/textentry.h
// Purpose:     declares wxTextEntry interface defining a simple text entry
// Author:      Vadim Zeitlin
// Created:     2007-09-24
// Copyright:   (c) 2007 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TEXTENTRY_H_
#define _WX_TEXTENTRY_H_

// wxTextPos is the position in the text (currently it's hardly used anywhere
// and should probably be replaced with int anyhow)
using wxTextPos = long;

class wxTextCompleter;
class wxTextEntryHintData;
class wxWindow;

#include "wx/filefn.h"              // for wxFILE and wxDIR only

import Utils.Geometry;

import <string>;
import <vector>;

// ----------------------------------------------------------------------------
// wxTextEntryBase
// ----------------------------------------------------------------------------

class wxTextEntryBase
{
public:
    virtual ~wxTextEntryBase() = default;

    // accessing the value
    // -------------------

    // SetValue() generates a text change event, ChangeValue() doesn't
    virtual void SetValue(std::string_view value)
        { DoSetValue(value, SetValue_SendEvent); }
    virtual void ChangeValue(std::string_view value);

    // writing text inserts it at the current position replacing any current
    // selection, appending always inserts it at the end and doesn't remove any
    // existing text (but it will reset the selection if there is any)
    virtual void WriteText(std::string_view text) = 0;
    virtual void AppendText(const std::string& text);

    virtual std::string GetValue() const;
    virtual std::string GetRange(long from, long to) const;
    bool IsEmpty() const { return GetLastPosition() <= 0; }


    // editing operations
    // ------------------

    virtual void Replace(long from, long to, const std::string& value);
    virtual void Remove(long from, long to) = 0;
    virtual void Clear() { Remove(0, -1); }
    void RemoveSelection();


    // clipboard operations
    // --------------------

    virtual void Copy() = 0;
    virtual void Cut() = 0;
    virtual void Paste() = 0;

    virtual bool CanCopy() const;
    virtual bool CanCut() const;
    virtual bool CanPaste() const;

    // undo/redo
    // ---------

    virtual void Undo() = 0;
    virtual void Redo() = 0;

    virtual bool CanUndo() const = 0;
    virtual bool CanRedo() const = 0;


    // insertion point
    // ---------------

    // note that moving insertion point removes any current selection
    virtual void SetInsertionPoint(long pos) = 0;
    virtual void SetInsertionPointEnd() { SetInsertionPoint(-1); }
    virtual long GetInsertionPoint() const = 0;
    virtual long GetLastPosition() const = 0;


    // selection
    // ---------

    virtual void SetSelection(long from, long to) = 0;
    virtual void SelectAll() { SetSelection(-1, -1); }
    virtual void SelectNone()
        { const long pos = GetInsertionPoint(); SetSelection(pos, pos); }
    virtual void GetSelection(long *from, long *to) const = 0;
    bool HasSelection() const;
    virtual std::string GetStringSelection() const;


    // auto-completion
    // ---------------

    // these functions allow to auto-complete the text already entered into the
    // control using either the given fixed list of strings, the paths from the
    // file system or an arbitrary user-defined completer
    //
    // they all return true if completion was enabled or false on error (most
    // commonly meaning that this functionality is not available under the
    // current platform)

    bool AutoComplete(const std::vector<std::string>& choices)
        { return DoAutoCompleteStrings(choices); }

    bool AutoCompleteFileNames()
        { return DoAutoCompleteFileNames(wxFILE); }

    bool AutoCompleteDirectories()
        { return DoAutoCompleteFileNames(wxDIR); }

    // notice that we take ownership of the pointer and will delete it
    //
    // if the pointer is NULL auto-completion is disabled
    bool AutoComplete(wxTextCompleter *completer)
        { return DoAutoCompleteCustom(completer); }


    // status
    // ------

    virtual bool IsEditable() const = 0;
    virtual void SetEditable(bool editable) = 0;


    // input restrictions
    // ------------------

    // set the max number of characters which may be entered in a single line
    // text control
    virtual void SetMaxLength([[maybe_unused]] unsigned long len) { }

    // convert any lower-case characters to upper-case on the fly in this entry
    virtual void ForceUpper();


    // hints
    // -----

    // hint is the (usually greyed out) text shown in the control as long as
    // it's empty and doesn't have focus, it is typically used in controls used
    // for searching to let the user know what is supposed to be entered there

    virtual bool SetHint(const std::string& hint);
    virtual std::string GetHint() const;


    // margins
    // -------

    // margins are the empty space between borders of control and the text
    // itself. When setting margin, use value -1 to indicate that specific
    // margin should not be changed.

    bool SetMargins(const wxPoint& pt)
        { return DoSetMargins(pt); }
    bool SetMargins(wxCoord left, wxCoord top = -1)
        { return DoSetMargins(wxPoint(left, top)); }
    wxPoint GetMargins() const
        { return DoGetMargins(); }


    // implementation only
    // -------------------

    // generate the wxEVT_TEXT event for GetEditableWindow(),
    // like SetValue() does and return true if the event was processed
    //
    // NB: this is public for wxRichTextCtrl use only right now, do not call it
    static bool SendTextUpdatedEvent(wxWindow *win);

    // generate the wxEVT_TEXT event for this window
    bool SendTextUpdatedEvent()
    {
        return SendTextUpdatedEvent(GetEditableWindow());
    }


    // generate the wxEVT_TEXT event for this window if the
    // events are not currently disabled
    void SendTextUpdatedEventIfAllowed()
    {
        if ( EventsAllowed() )
            SendTextUpdatedEvent();
    }

    // this function is provided solely for the purpose of forwarding text
    // change notifications state from one control to another, e.g. it can be
    // used by a wxComboBox which derives from wxTextEntry if it delegates all
    // of its methods to another wxTextCtrl
    void ForwardEnableTextChangedEvents(bool enable)
    {
        // it's important to call the functions which update m_eventsBlock here
        // and not just our own EnableTextChangedEvents() because our state
        // (i.e. the result of EventsAllowed()) must change as well
        if ( enable )
            ResumeTextChangedEvents();
        else
            SuppressTextChangedEvents();
    }

    // change the entry value to be in upper case only, if needed (i.e. if it's
    // not already the case)
    void ConvertToUpperCase();

protected:
    // flags for DoSetValue(): common part of SetValue() and ChangeValue() and
    // also used to implement WriteText() in wxMSW
    enum
    {
        SetValue_NoEvent = 0,
        SetValue_SendEvent = 1,
        SetValue_SelectionOnly = 2
    };

    virtual void DoSetValue(std::string_view value, unsigned int flags);
    virtual std::string DoGetValue() const = 0;

    // override this to return the associated window, it will be used for event
    // generation and also by generic hints implementation
    virtual wxWindow *GetEditableWindow() = 0;

    // margins functions
    virtual bool DoSetMargins(const wxPoint& pt);
    virtual wxPoint DoGetMargins() const;

    // the derived classes should override these virtual methods to implement
    // auto-completion, they do the same thing as their public counterparts but
    // have different names to allow overriding just one of them without hiding
    // the other one(s)
    virtual bool DoAutoCompleteStrings([[maybe_unused]] const std::vector<std::string>& choices)
        { return false; }
    virtual bool DoAutoCompleteFileNames([[maybe_unused]] unsigned int flags) // wxFILE | wxDIR
        { return false; }
    virtual bool DoAutoCompleteCustom(wxTextCompleter *completer);


    // class which should be used to temporarily disable text change events
    //
    // if suppress argument in ctor is false, nothing is done
    class EventsSuppressor
    {
    public:
        EventsSuppressor(wxTextEntryBase *text, bool suppress = true)
            : m_text(text),
              m_suppress(suppress)
        {
            if ( m_suppress )
                m_text->SuppressTextChangedEvents();
        }

        ~EventsSuppressor()
        {
            if ( m_suppress )
                m_text->ResumeTextChangedEvents();
        }

    private:
        wxTextEntryBase *m_text;
        bool m_suppress;
    };

    friend class EventsSuppressor;

private:
    // suppress or resume the text changed events generation: don't use these
    // functions directly, use EventsSuppressor class above instead
    void SuppressTextChangedEvents()
    {
        if ( !m_eventsBlock++ )
            EnableTextChangedEvents(false);
    }

    void ResumeTextChangedEvents()
    {
        if ( !--m_eventsBlock )
            EnableTextChangedEvents(true);
    }


    // this must be overridden in the derived classes if our implementation of
    // SetValue() or Replace() is used to disable (and enable back) generation
    // of the text changed events
    //
    // initially the generation of the events is enabled
    virtual void EnableTextChangedEvents([[maybe_unused]] bool enable) { }

    // return true if the events are currently not suppressed
    bool EventsAllowed() const { return m_eventsBlock == 0; }

    // hint-related stuff, only allocated if/when SetHint() is used
    std::unique_ptr<wxTextEntryHintData> m_hintData;

    // if this counter is non-null, events are blocked
    unsigned int m_eventsBlock{0};

    // It needs to call our Do{Get,Set}Value() to work with the real control
    // contents.
    friend class wxTextEntryHintData;
};


// ----------------------------------------------------------------------------
// wxTextEntryHintData
// ----------------------------------------------------------------------------

class wxTextEntryHintData : public wxEvtHandler
{
public:
    wxTextEntryHintData(wxTextEntryBase *entry, wxWindow *win)
        : m_entry(entry),
          m_win(win),
          m_text(m_entry->GetValue())
    {
        // We push ourselves as the event handler because this allows us to
        // handle events before the user-defined handlers and notably process
        // wxEVT_TEXT even if the user code already handles it, which is vital
        // as if we don't get this event, we would always set the control text
        // to the hint when losing focus, instead of preserving the text
        // entered by user. Of course, the same problem could still happen if
        // the user code pushed their own event handler before this one and
        // didn't skip wxEVT_TEXT in it, but there doesn't seem anything we can
        // do about this anyhow and this at least takes care of the much more
        // common case.
        m_win->PushEventHandler(this);

        Bind(wxEVT_SET_FOCUS, &wxTextEntryHintData::OnSetFocus, this);
        Bind(wxEVT_KILL_FOCUS, &wxTextEntryHintData::OnKillFocus, this);
        Bind(wxEVT_TEXT, &wxTextEntryHintData::OnTextChanged, this);
    }

    ~wxTextEntryHintData()
    {
        m_win->PopEventHandler();
    }

    wxTextEntryHintData(const wxTextEntryHintData&) = delete;
	wxTextEntryHintData& operator=(const wxTextEntryHintData&) = delete;

    // Get the real text of the control such as it was before we replaced it
    // with the hint.
    const std::string& GetText() const { return m_text; }

    // Set the hint to show, shouldn't be empty normally.
    //
    // This should be called after creating a new wxTextEntryHintData object
    // and may be called more times in the future.
    void SetHintString(const std::string& hint)
    {
        m_hint = hint;

        if ( !m_win->HasFocus() )
            ShowHintIfAppropriate();
        //else: The new hint will be shown later when we lose focus.
    }

    const std::string& GetHintString() const { return m_hint; }

    // This is called whenever the text control contents changes.
    //
    // We call it ourselves when this change generates an event but it's also
    // necessary to call it explicitly from wxTextEntry::ChangeValue() as it,
    // by design, does not generate any events.
    void HandleTextUpdate(std::string_view text)
    {
        m_text = {text.begin(), text.end()};

        // If we're called because of a call to Set or ChangeValue(), the
        // control may still have the hint text colour, reset it in this case.
        RestoreTextColourIfNecessary();
    }

private:
    // Show the hint in the window if we should do it, i.e. if the window
    // doesn't have any text of its own.
    void ShowHintIfAppropriate()
    {
        // Never overwrite existing window text.
        if ( !m_text.empty() )
            return;

        // Save the old text colour and set a more inconspicuous one for the
        // hint.
        if (!m_colFg.IsOk())
        {
            m_colFg = m_win->GetForegroundColour();
            m_win->SetForegroundColour(*wxLIGHT_GREY);
        }

        m_entry->DoSetValue(m_hint, wxTextEntryBase::SetValue_NoEvent);
    }

    // Restore the original text colour if we had changed it to show the hint
    // and not restored it yet.
    void RestoreTextColourIfNecessary()
    {
        if ( m_colFg.IsOk() )
        {
            m_win->SetForegroundColour(m_colFg);
            m_colFg = wxColour();
        }
    }

    void OnSetFocus(wxFocusEvent& event)
    {
        // If we had been showing the hint before, remove it now and restore
        // the normal colour.
        if ( m_text.empty() )
        {
            RestoreTextColourIfNecessary();

            m_entry->DoSetValue("", wxTextEntryBase::SetValue_NoEvent);
        }

        event.Skip();
    }

    void OnKillFocus(wxFocusEvent& event)
    {
        // Restore the hint if the user didn't enter anything.
        ShowHintIfAppropriate();

        event.Skip();
    }

    void OnTextChanged(wxCommandEvent& event)
    {
        // Update the stored window text.
        //
        // Notice that we can't use GetValue() nor wxCommandEvent::GetString()
        // which uses it internally because this would just forward back to us
        // so go directly to the private method which returns the real control
        // contents.
        HandleTextUpdate(m_entry->DoGetValue());

        event.Skip();
    }


    // the text control we're associated with (as its interface and its window)
    wxTextEntryBase * const m_entry;
    wxWindow * const m_win;

    // the original foreground colour of m_win before we changed it
    wxColour m_colFg;

    // The hint passed to wxTextEntry::SetHint(), never empty.
    std::string m_hint;

    // The real text of the window.
    std::string m_text;

};

#ifdef __WXUNIVERSAL__
    // TODO: we need to use wxTextEntryDelegate here, but for now just prevent
    //       the GTK/MSW classes from being used in wxUniv build
    class wxTextEntry : public wxTextEntryBase
    {
    };
#elif defined(__WXGTK20__)
    #include "wx/gtk/textentry.h"
#elif defined(__WXMAC__)
    #include "wx/osx/textentry.h"
#elif defined(__WXMSW__)
    #include "wx/msw/textentry.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/textentry.h"
#elif defined(__WXQT__)
    #include "wx/qt/textentry.h"
#else
    // no platform-specific implementation of wxTextEntry yet
    class wxTextEntry : public wxTextEntryBase
    {
    };
#endif

#endif // _WX_TEXTENTRY_H_

