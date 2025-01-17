///////////////////////////////////////////////////////////////////////////////
// Name:        wx/caret.h
// Purpose:     wxCaretBase class - the interface of wxCaret
// Author:      Vadim Zeitlin
// Modified by:
// Created:     23.05.99
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CARET_H_BASE_
#define _WX_CARET_H_BASE_

#if wxUSE_CARET

// ---------------------------------------------------------------------------
// forward declarations
// ---------------------------------------------------------------------------

class wxWindow;
class wxWindowBase;

import Utils.Geometry;

// ----------------------------------------------------------------------------
// A caret is a blinking cursor showing the position where the typed text will
// appear. It can be either a solid block or a custom bitmap (TODO)
// ----------------------------------------------------------------------------

class wxCaretBase
{
public:
    // ctors
    // -----
        // default - use Create
    wxCaretBase() = default;
        // create the caret of given (in pixels) width and height and associate
        // with the given window
    wxCaretBase(wxWindowBase *window, const wxSize& size)
    {
        Create(window, size);
    }

    wxCaretBase& operator=(wxCaretBase&&) = delete;

    // a virtual dtor has been provided since this class has virtual members
    virtual ~wxCaretBase() = default;

    // Create() functions - same as ctor but returns the success code
    // --------------------------------------------------------------

    // same as ctor
    [[maybe_unused]] bool Create(wxWindowBase *window, wxSize sz)
        { return DoCreate(window, sz); }

        // is the caret valid?
    bool IsOk() const { return m_size != wxSize{0, 0}; }

        // is the caret currently shown?
    bool IsVisible() const { return m_countVisible > 0; }

        // get the caret position
    wxPoint GetPosition() const { return m_pos; }

        // get the caret size
    wxSize GetSize() const { return m_size; }

        // get the window we're associated with
    wxWindow *GetWindow() const { return (wxWindow *)m_window; }

        // change the size of the caret
    void SetSize(wxSize sz)
    {
        m_size = sz;
        DoSize();
    }

    // operations
    // ----------

    // move the caret to given position (in logical coords)
    void Move(const wxPoint& pt) { m_pos = pt; DoMove(); }

        // show/hide the caret (should be called by wxWindow when needed):
        // Show() must be called as many times as Hide() + 1 to make the caret
        // visible
    virtual void Show(bool show = true)
        {
            if ( show )
            {
                if ( m_countVisible++ == 0 )
                    DoShow();
            }
            else
            {
                if ( --m_countVisible == 0 )
                    DoHide();
            }
        }
    virtual void Hide() { Show(false); }

        // blink time is measured in milliseconds and is the time elapsed
        // between 2 inversions of the caret (blink time of the caret is common
        // to all carets in the Universe, so these functions are static)
    static int GetBlinkTime();
    static void SetBlinkTime(int milliseconds);

    // implementation from now on
    // --------------------------

    // these functions should be called by wxWindow when the window gets/loses
    // the focus - we create/show and hide/destroy the caret here
    virtual void OnSetFocus() { }
    virtual void OnKillFocus() { }

protected:
    // these functions may be overridden in the derived classes, but they
    // should call the base class version first
    virtual bool DoCreate(wxWindowBase *window, wxSize sz)
    {
        m_window = window;
        m_size = sz;
        DoSize();

        return true;
    }

    // pure virtuals to implement in the derived class
    virtual void DoShow() = 0;
    virtual void DoHide() = 0;
    virtual void DoMove() = 0;
    virtual void DoSize() = 0;

    // the size of the caret
    wxSize m_size{};

    // the position of the caret
    wxPoint m_pos{};

    // the window we're associated with
    wxWindowBase *m_window{nullptr};

    // visibility count: the caret is visible only if it's positive
    int m_countVisible{0};
};

// ---------------------------------------------------------------------------
// now include the real thing
// ---------------------------------------------------------------------------

#if defined(__WXMSW__)
    #include "wx/msw/caret.h"
#else
    #include "wx/generic/caret.h"
#endif // platform

// ----------------------------------------------------------------------------
// wxCaretSuspend: a simple class which hides the caret in its ctor and
// restores it in the dtor, this should be used when drawing on the screen to
// avoid overdrawing the caret
// ----------------------------------------------------------------------------

#ifdef wxHAS_CARET_USING_OVERLAYS

// we don't need to hide the caret if it's rendered using overlays
class wxCaretSuspend
{
public:
    wxCaretSuspend& operator=(wxCaretSuspend&&) = delete;
};

#else // !wxHAS_CARET_USING_OVERLAYS

class wxCaretSuspend
{
public:
    wxCaretSuspend(wxWindow *win)
    {
        m_caret = win->GetCaret();
        m_show = false;
        if ( m_caret && m_caret->IsVisible() )
        {
            m_caret->Hide();
            m_show = true;
        }
    }

    ~wxCaretSuspend()
    {
        if ( m_caret && m_show )
            m_caret->Show();
    }

    wxCaretSuspend& operator=(wxCaretSuspend&&) = delete;

private:
    wxCaret* m_caret;
    bool     m_show;
};

#endif // wxHAS_CARET_USING_OVERLAYS/!wxHAS_CARET_USING_OVERLAYS

#endif // wxUSE_CARET

#endif // _WX_CARET_H_BASE_
