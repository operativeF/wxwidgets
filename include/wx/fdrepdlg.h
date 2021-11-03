/////////////////////////////////////////////////////////////////////////////
// Name:        wx/fdrepdlg.h
// Purpose:     wxFindReplaceDialog class
// Author:      Markus Greither and Vadim Zeitlin
// Modified by:
// Created:     23/03/2001
// Copyright:   (c) Markus Greither
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FINDREPLACEDLG_H_
#define _WX_FINDREPLACEDLG_H_

#include "wx/defs.h"

#if wxUSE_FINDREPLDLG

#include "wx/bitflags.h"
#include "wx/dialog.h"

#include <cstdint>
#include <string>

class WXDLLIMPEXP_FWD_CORE wxFindDialogEvent;
class WXDLLIMPEXP_FWD_CORE wxFindReplaceDialog;
class WXDLLIMPEXP_FWD_CORE wxFindReplaceData;
class WXDLLIMPEXP_FWD_CORE wxFindReplaceDialogImpl;

// ----------------------------------------------------------------------------
// Flags for wxFindReplaceData.Flags
// ----------------------------------------------------------------------------

// flags used by wxFindDialogEvent::GetFlags()
enum class wxFindReplaceFlags
{
    Down,      // downward search/replace selected (otherwise - upwards)
    WholeWord, // whole word search/replace selected
    MatchCase, // case sensitive search/replace selected (otherwise - case insensitive)
    _max_size
};

using FindReplaceFlags = InclBitfield<wxFindReplaceFlags>;

// these flags can be specified in wxFindReplaceDialog ctor or Create()
enum wxFindReplaceDialogStyles
{
    // replace dialog (otherwise find dialog)
    wxFR_REPLACEDIALOG = 1,

    // don't allow changing the search direction
    wxFR_NOUPDOWN      = 2,

    // don't allow case sensitive searching
    wxFR_NOMATCHCASE   = 4,

    // don't allow whole word searching
    wxFR_NOWHOLEWORD   = 8
};

// ----------------------------------------------------------------------------
// wxFindReplaceData: holds Setup Data/Feedback Data for wxFindReplaceDialog
// ----------------------------------------------------------------------------

class wxFindReplaceData
{
public:
    wxFindReplaceData() = default;
    wxFindReplaceData(FindReplaceFlags flags) { SetFlags(flags); }

    const std::string& GetFindString() const { return m_FindWhat; }
    const std::string& GetReplaceString() const { return m_ReplaceWith; }

    FindReplaceFlags GetFlags() const { return m_flags; }

    // setters: may only be called before showing the dialog, no effect later
    // FIXME: Then get rid of this and put it in the constructor.
    void SetFlags(FindReplaceFlags flags) { m_flags = flags; }

    void SetFindString(const std::string& str) { m_FindWhat = str; }
    void SetReplaceString(const std::string& str) { m_ReplaceWith = str; }

private:
    std::string      m_FindWhat;
    std::string      m_ReplaceWith;

    FindReplaceFlags m_flags{};

    friend class wxFindReplaceDialogBase;
};

// ----------------------------------------------------------------------------
// wxFindReplaceDialogBase
// ----------------------------------------------------------------------------

class wxFindReplaceDialogBase : public wxDialog
{
public:
    
    wxFindReplaceDialogBase() = default;
    wxFindReplaceDialogBase(wxWindow * WXUNUSED(parent),
                            wxFindReplaceData *data,
                            const std::string& WXUNUSED(title),
                            int WXUNUSED(style) = 0)
        : m_FindReplaceData{data}
    {
    }

    wxFindReplaceDialogBase& operator=(wxFindReplaceDialogBase&&) = delete;
    
    // find dialog data access
    const wxFindReplaceData *GetData() const { return m_FindReplaceData; }
    void SetData(wxFindReplaceData *data) { m_FindReplaceData = data; }

    // implementation only, don't use
    void Send(wxFindDialogEvent& event);

protected:
    wxFindReplaceData *m_FindReplaceData {nullptr};

    // the last string we searched for
    std::string m_lastSearch;
};

// include wxFindReplaceDialog declaration
#if defined(__WXMSW__) && !defined(__WXUNIVERSAL__)
    #include "wx/msw/fdrepdlg.h"
#else
    #define wxGenericFindReplaceDialog wxFindReplaceDialog

    #include "wx/generic/fdrepdlg.h"
#endif

// ----------------------------------------------------------------------------
// wxFindReplaceDialog events
// ----------------------------------------------------------------------------

class wxFindDialogEvent : public wxCommandEvent
{
public:
    wxFindDialogEvent(wxEventType commandType = wxEVT_NULL, int id = 0)
        : wxCommandEvent(commandType, id) { }
    wxFindDialogEvent(const wxFindDialogEvent& event) = default;

	wxFindDialogEvent& operator=(const wxFindDialogEvent&) = delete;

    int GetFlags() const { return GetInt(); }
    std::string GetFindString() const { return GetString(); }
    const std::string& GetReplaceString() const { return m_strReplace; }

    wxFindReplaceDialog *GetDialog() const
        { return wxStaticCast(GetEventObject(), wxFindReplaceDialog); }

    // implementation only
    void SetFlags(FindReplaceFlags flags) { SetInt(flags.as_value()); }
    void SetFindString(const std::string& str) { SetString(str); }
    void SetReplaceString(const std::string& str) { m_strReplace = str; }

    wxEvent *Clone() const override { return new wxFindDialogEvent(*this); }

private:
    std::string m_strReplace;

public:
	wxClassInfo *wxGetClassInfo() const override ;
	static wxClassInfo ms_classInfo; 
	static wxObject* wxCreateObject();
};

wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_FIND, wxFindDialogEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_FIND_NEXT, wxFindDialogEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_FIND_REPLACE, wxFindDialogEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_FIND_REPLACE_ALL, wxFindDialogEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_FIND_CLOSE, wxFindDialogEvent );

typedef void (wxEvtHandler::*wxFindDialogEventFunction)(wxFindDialogEvent&);

#define wxFindDialogEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxFindDialogEventFunction, func)

#define EVT_FIND(id, fn) \
    wx__DECLARE_EVT1(wxEVT_FIND, id, wxFindDialogEventHandler(fn))

#define EVT_FIND_NEXT(id, fn) \
    wx__DECLARE_EVT1(wxEVT_FIND_NEXT, id, wxFindDialogEventHandler(fn))

#define EVT_FIND_REPLACE(id, fn) \
    wx__DECLARE_EVT1(wxEVT_FIND_REPLACE, id, wxFindDialogEventHandler(fn))

#define EVT_FIND_REPLACE_ALL(id, fn) \
    wx__DECLARE_EVT1(wxEVT_FIND_REPLACE_ALL, id, wxFindDialogEventHandler(fn))

#define EVT_FIND_CLOSE(id, fn) \
    wx__DECLARE_EVT1(wxEVT_FIND_CLOSE, id, wxFindDialogEventHandler(fn))

// old wxEVT_COMMAND_* constants
#define wxEVT_COMMAND_FIND               wxEVT_FIND
#define wxEVT_COMMAND_FIND_NEXT          wxEVT_FIND_NEXT
#define wxEVT_COMMAND_FIND_REPLACE       wxEVT_FIND_REPLACE
#define wxEVT_COMMAND_FIND_REPLACE_ALL   wxEVT_FIND_REPLACE_ALL
#define wxEVT_COMMAND_FIND_CLOSE         wxEVT_FIND_CLOSE

#endif // wxUSE_FINDREPLDLG

#endif
    // _WX_FDREPDLG_H
