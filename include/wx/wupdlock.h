///////////////////////////////////////////////////////////////////////////////
// Name:        wx/wupdlock.h
// Purpose:     wxWindowUpdateLocker prevents window redrawing
// Author:      Vadim Zeitlin
// Created:     2006-03-06
// Copyright:   (c) 2006 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_WUPDLOCK_H_
#define _WX_WUPDLOCK_H_

class wxWindow;

// ----------------------------------------------------------------------------
// wxWindowUpdateLocker prevents updates to the window during its lifetime
// ----------------------------------------------------------------------------

class wxWindowUpdateLocker
{
public:
    // Prefer using the ctor below if possible, this ctor is only useful if
    // Lock() must be called only conditionally.
    wxWindowUpdateLocker()  = default;

    // create an object preventing updates of the given window (which must have
    // a lifetime at least as great as ours)
    explicit wxWindowUpdateLocker(wxWindow *win) : m_win(win) { win->Freeze(); }

    // May be called only for the object constructed using the default ctor.
    void Lock(wxWindow *win)
    {
        wxASSERT( !m_win );

        m_win = win;
        win->Freeze();
    }

    // dtor thaws the window to permit updates again
    ~wxWindowUpdateLocker()
    {
        if ( m_win )
            m_win->Thaw();
    }

   wxWindowUpdateLocker& operator=(wxWindowUpdateLocker&&) = delete;

private:
    wxWindow *m_win{nullptr};
};

#endif // _WX_WUPDLOCK_H_

