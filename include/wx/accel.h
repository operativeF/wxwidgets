///////////////////////////////////////////////////////////////////////////////
// Name:        wx/accel.h
// Purpose:     wxAcceleratorEntry and wxAcceleratorTable classes
// Author:      Julian Smart, Robert Roebling, Vadim Zeitlin
// Modified by:
// Created:     31.05.01 (extracted from other files)
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_ACCEL_H_BASE_
#define _WX_ACCEL_H_BASE_

#include "wx/defs.h"

#if wxUSE_ACCEL

#include "wx/object.h"

#include <optional>

class WXDLLIMPEXP_FWD_CORE wxAcceleratorTable;
class WXDLLIMPEXP_FWD_CORE wxMenuItem;
class WXDLLIMPEXP_FWD_CORE wxKeyEvent;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// wxAcceleratorEntry flags
enum wxAcceleratorEntryFlags
{
    wxACCEL_NORMAL  = 0x0000,   // no modifiers
    wxACCEL_ALT     = 0x0001,   // hold Alt key down
    wxACCEL_CTRL    = 0x0002,   // hold Ctrl key down
    wxACCEL_SHIFT   = 0x0004,   // hold Shift key down
#if defined(__WXMAC__)
    wxACCEL_RAW_CTRL= 0x0008,   //
#else
    wxACCEL_RAW_CTRL= wxACCEL_CTRL,
#endif
    wxACCEL_CMD     = wxACCEL_CTRL
};

// ----------------------------------------------------------------------------
// an entry in wxAcceleratorTable corresponds to one accelerator
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxAcceleratorEntry
{
public:
    wxAcceleratorEntry() = default;
    wxAcceleratorEntry(int flags,
                       int keyCode,
                       int cmd,
                       wxMenuItem *item)
        : m_flags(flags)
        , m_keyCode(keyCode)
        , m_command(cmd)
        , m_item(item)
    {}

    // create accelerator corresponding to the specified string, return NULL if
    // string couldn't be parsed or a pointer to be deleted by the caller
    static std::optional<wxAcceleratorEntry> Create(const std::string& str);

    void Set(int flags, int keyCode, int cmd, wxMenuItem *item = nullptr)
    {
        m_flags = flags;
        m_keyCode = keyCode;
        m_command = cmd;
        m_item = item;
    }

    void SetMenuItem(wxMenuItem *item) { m_item = item; }

    int GetFlags() const { return m_flags; }
    int GetKeyCode() const { return m_keyCode; }
    int GetCommand() const { return m_command; }

    wxMenuItem *GetMenuItem() const { return m_item; }

    bool operator==(const wxAcceleratorEntry& entry) const
    {
        return m_flags == entry.m_flags &&
               m_keyCode == entry.m_keyCode &&
               m_command == entry.m_command &&
               m_item == entry.m_item;
    }

    bool operator!=(const wxAcceleratorEntry& entry) const
        { return !(*this == entry); }

#if defined(__WXMOTIF__)
    // Implementation use only
    bool MatchesEvent(const wxKeyEvent& event) const;
#endif

    bool IsOk() const
    {
        return  m_keyCode != 0;
    }


    // string <-> wxAcceleratorEntry conversion
    // ----------------------------------------

    // returns a std::string for the this accelerator.
    // this function formats it using the <flags>-<keycode> format
    // where <flags> maybe a hyphen-separated list of "shift|alt|ctrl"
    std::string ToString() const { return AsPossiblyLocalizedString(true); }

    // same as above but without translating, useful if the string is meant to
    // be stored in a file or otherwise stored, instead of being shown to the
    // user
    std::string ToRawString() const { return AsPossiblyLocalizedString(false); }

    // returns true if the given string correctly initialized this object
    // (i.e. if IsOk() returns true after this call)
    bool FromString(const std::string& str);


private:
    std::string AsPossiblyLocalizedString(bool localized) const;

    // common part of Create() and FromString()
    static bool ParseAccel(const std::string& str, int *flags, int *keycode);


    int m_flags{0};    // combination of wxACCEL_XXX constants
    int m_keyCode{0};  // ASCII or virtual keycode
    int m_command{0};  // Command id to generate

    // the menu item this entry corresponds to, may be NULL
    wxMenuItem *m_item{nullptr};

    // for compatibility with old code, use accessors now!
    friend class WXDLLIMPEXP_FWD_CORE wxMenu;
};

// ----------------------------------------------------------------------------
// include wxAcceleratorTable class declaration, it is only used by the library
// and so doesn't have any published user visible interface
// ----------------------------------------------------------------------------

#if defined(__WXUNIVERSAL__)
    #include "wx/generic/accel.h"
#elif defined(__WXMSW__)
    #include "wx/msw/accel.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/accel.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/accel.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/accel.h"
#elif defined(__WXMAC__)
    #include "wx/osx/accel.h"
#elif defined(__WXQT__)
    #include "wx/qt/accel.h"
#endif

extern WXDLLIMPEXP_DATA_CORE(wxAcceleratorTable) wxNullAcceleratorTable;

#endif // wxUSE_ACCEL

#endif
    // _WX_ACCEL_H_BASE_
