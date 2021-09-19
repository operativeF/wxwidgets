///////////////////////////////////////////////////////////////////////////////
// Name:        wx/persist/window.h
// Purpose:     wxPersistentWindow declaration
// Author:      Vadim Zeitlin
// Created:     2009-01-23
// Copyright:   (c) 2009 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PERSIST_WINDOW_H_
#define _WX_PERSIST_WINDOW_H_

#include "wx/persist.h"

class WXDLLIMPEXP_FWD_CORE wxWindow;

// ----------------------------------------------------------------------------
// wxPersistentWindow: base class for persistent windows, uses the window name
//                     as persistent name by default and automatically reacts
//                     to the window destruction
// ----------------------------------------------------------------------------

// type-independent part of wxPersistentWindow
class wxPersistentWindowBase : public wxPersistentObject
{
public:
    wxPersistentWindowBase(wxWindow *win)
        : wxPersistentObject(win)
    {
        win->Bind(wxEVT_DESTROY, &wxPersistentWindowBase::HandleDestroy, this);
    }

    std::string GetName() const override
    {
        std::string name = GetWindow()->GetName();
        wxASSERT_MSG( !name.empty(), "persistent windows should be named!" );

        return name;
    }

protected:
    wxWindow *GetWindow() const { return static_cast<wxWindow *>(wxGetObject()); }

private:
    void HandleDestroy(wxWindowDestroyEvent& event)
    {
        event.Skip();

        // only react to the destruction of this object itself, not of any of
        // its children
        if ( event.GetEventObject() == wxGetObject() )
        {
            // this will delete this object itself
            wxPersistenceManager::Get().SaveAndUnregister(GetWindow());
        }
    }

    wxPersistentWindowBase(const wxPersistentWindowBase&) = delete;
	wxPersistentWindowBase& operator=(const wxPersistentWindowBase&) = delete;
};

template <class T>
class wxPersistentWindow : public wxPersistentWindowBase
{
public:
    typedef T WindowType;

    wxPersistentWindow(WindowType *win)
        : wxPersistentWindowBase(win)
    {
    }

    WindowType *Get() const { return static_cast<WindowType *>(GetWindow()); }
};

#endif // _WX_PERSIST_WINDOW_H_

