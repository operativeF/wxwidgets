///////////////////////////////////////////////////////////////////////////////
// Name:        wx/preferences.h
// Purpose:     Declaration of wxPreferencesEditor class.
// Author:      Vaclav Slavik
// Created:     2013-02-19
// Copyright:   (c) 2013 Vaclav Slavik <vslavik@fastmail.fm>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PREFERENCES_H_
#define _WX_PREFERENCES_H_

#if wxUSE_PREFERENCES_EDITOR

#include "wx/bitmap.h"

import <vector>;

class wxWindow;

class wxPreferencesEditorImpl;

#if defined(__WXOSX_COCOA__)
    // GetLargeIcon() is used
    #define wxHAS_PREF_EDITOR_ICONS
    // Changes should be applied immediately
    #define wxHAS_PREF_EDITOR_APPLY_IMMEDIATELY
    // The dialog is shown non-modally.
    #define wxHAS_PREF_EDITOR_MODELESS
#elif defined(__WXGTK__)
    // Changes should be applied immediately
    #define wxHAS_PREF_EDITOR_APPLY_IMMEDIATELY
    // The dialog is shown non-modally.
    #define wxHAS_PREF_EDITOR_MODELESS
#endif

// ----------------------------------------------------------------------------
// wxPreferencesEditor: Native preferences editing
// ----------------------------------------------------------------------------

// One page of a preferences window
class wxPreferencesPage
{
public:
    virtual ~wxPreferencesPage() = default;
    wxPreferencesPage& operator=(wxPreferencesPage&&) = delete;

    // Name of the page, used e.g. for tabs
    virtual std::string GetName() const = 0;

    // Return 32x32 icon used for the page. Currently only used on OS X, where
    // implementation is required; unused on other platforms. Because of this,
    // the method is only pure virtual on platforms that use it.
#ifdef wxHAS_PREF_EDITOR_ICONS
    virtual wxBitmap GetLargeIcon() const = 0;
#else
    virtual wxBitmap GetLargeIcon() const { return wxBitmap(); }
#endif

    // Create a window (usually a wxPanel) for this page. The caller takes
    // ownership of the returned window.
    virtual wxWindow *wxCreateWindow(wxWindow *parent) = 0;
};


// Helper for implementing some common pages (General, Advanced)
class wxStockPreferencesPage : public wxPreferencesPage
{
public:
    enum class Kind
    {
        General,
        Advanced
    };

    wxStockPreferencesPage(Kind kind) : m_kind{kind} {}
    Kind GetKind() const { return m_kind; }

    std::string GetName() const override;
#ifdef __WXOSX_COCOA__
    wxBitmap GetLargeIcon() const override;
#endif

private:
    Kind m_kind;
};


// Notice that this class does not inherit from wxWindow.
class wxPreferencesEditor
{
public:
    // Ctor creates an empty editor, use AddPage() to add controls to it.
    wxPreferencesEditor(const std::string& title = {});

    wxPreferencesEditor& operator=(wxPreferencesEditor&&) = delete;
    
    // Dtor destroys the dialog if still shown.
    virtual ~wxPreferencesEditor();

    // Add a new page to the editor. The editor takes ownership of the page
    // and won't delete it until it is destroyed itself.
    void AddPage(wxPreferencesPage *page);

    // Show the preferences dialog or bring it to the top if it's already
    // shown. Notice that this method may or may not block depending on the
    // platform, i.e. depending on whether the dialog is modal or not.
    virtual void Show(wxWindow* parent);

    // Hide the currently shown dialog, if any. This is typically used to
    // dismiss the dialog if the object whose preferences it is editing was
    // closed.
    void Dismiss();

    // Whether changes to values in the pages should be applied immediately
    // (OS X, GTK+) or only when the user clicks OK/Apply (Windows)
    static bool ShouldApplyChangesImmediately()
    {
#ifdef wxHAS_PREF_EDITOR_APPLY_IMMEDIATELY
        return true;
#else
        return false;
#endif
    }

    // Whether the dialog is shown modally, i.e. Show() blocks, or not.
    static bool ShownModally()
    {
#ifdef wxHAS_PREF_EDITOR_MODELESS
        return false;
#else
        return true;
#endif
    }

private:
    wxPreferencesEditorImpl* m_impl;
};

#endif // wxUSE_PREFERENCES_EDITOR

#endif // _WX_PREFERENCES_H_
