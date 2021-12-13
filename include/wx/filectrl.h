///////////////////////////////////////////////////////////////////////////////
// Name:        wx/filectrl.h
// Purpose:     Header for wxFileCtrlBase and other common functions used by
//              platform-specific wxFileCtrl's
// Author:      Diaa M. Sami
// Modified by:
// Created:     Jul-07-2007
// Copyright:   (c) Diaa M. Sami
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FILECTRL_H_BASE_
#define _WX_FILECTRL_H_BASE_

#if wxUSE_FILECTRL

#include "wx/event.h"

import <vector>;

enum
{
    wxFC_OPEN              = 0x0001,
    wxFC_SAVE              = 0x0002,
    wxFC_MULTIPLE          = 0x0004,
    wxFC_NOSHOWHIDDEN      = 0x0008
};

inline constexpr unsigned int wxFC_DEFAULT_STYLE = wxFC_OPEN;

inline constexpr std::string_view wxFileCtrlNameStr = "wxfilectrl";

class wxFileCtrlBase
{
public:
    virtual ~wxFileCtrlBase() = default;

    virtual void SetWildcard( const std::string& wildCard ) = 0;
    virtual void SetFilterIndex( int filterindex ) = 0;
    virtual bool SetDirectory( const std::string& dir ) = 0;

    // Selects a certain file.
    // In case the filename specified isn't found/couldn't be shown with
    // currently selected filter, false is returned and nothing happens
    virtual bool SetFilename( const std::string& name ) = 0;

    // chdirs to a certain directory and selects a certain file.
    // In case the filename specified isn't found/couldn't be shown with
    // currently selected filter, false is returned and if directory exists
    // it's chdir'ed to
    virtual bool SetPath( const std::string& path ) = 0;

    virtual std::string GetFilename() const = 0;
    virtual std::string GetDirectory() const = 0;
    virtual std::string GetWildcard() const = 0;
    virtual std::string GetPath() const = 0;
    virtual std::vector<std::string> GetPaths() const = 0;
    virtual std::vector<std::string> GetFilenames() const = 0;
    virtual int GetFilterIndex() const = 0;

    virtual bool HasMultipleFileSelection() const = 0;
    virtual void ShowHidden(bool show) = 0;
};

void wxGenerateFilterChangedEvent( wxFileCtrlBase *fileCtrl, wxWindow *wnd );
void wxGenerateFolderChangedEvent( wxFileCtrlBase *fileCtrl, wxWindow *wnd );
void wxGenerateSelectionChangedEvent( wxFileCtrlBase *fileCtrl, wxWindow *wnd );
void wxGenerateFileActivatedEvent( wxFileCtrlBase *fileCtrl, wxWindow *wnd, const std::string& filename = {} );

#if defined(__WXGTK20__) && !defined(__WXUNIVERSAL__)
    #include "wx/gtk/filectrl.h"
    #define wxFileCtrl wxGtkFileCtrl
#else
    #include "wx/generic/filectrlg.h"
    #define wxFileCtrl wxGenericFileCtrl
#endif

// Some documentation
// On wxEVT_FILECTRL_FILTERCHANGED, only the value returned by GetFilterIndex is
// valid and it represents the (new) current filter index for the wxFileCtrl.
// On wxEVT_FILECTRL_FOLDERCHANGED, only the value returned by GetDirectory is
// valid and it represents the (new) current directory for the wxFileCtrl.
// On wxEVT_FILECTRL_FILEACTIVATED, GetDirectory returns the current directory
// for the wxFileCtrl and GetFiles returns the names of the file(s) activated.
// On wxEVT_FILECTRL_SELECTIONCHANGED, GetDirectory returns the current directory
// for the wxFileCtrl and GetFiles returns the names of the currently selected
// file(s).
// In wxGTK, after each wxEVT_FILECTRL_FOLDERCHANGED, wxEVT_FILECTRL_SELECTIONCHANGED
// is fired automatically once or more with 0 files.
class wxFileCtrlEvent : public wxCommandEvent
{
public:
    wxFileCtrlEvent() = default;
    wxFileCtrlEvent( wxEventType type, wxObject *evtObject, int id )
            : wxCommandEvent( type, id )
    {
        SetEventObject( evtObject );
    }

	wxFileCtrlEvent& operator=(const wxFileCtrlEvent&) = delete;

    // no need for the copy constructor as the default one will be fine.
    std::unique_ptr<wxEvent> Clone() const override { return std::make_unique<wxFileCtrlEvent>( *this ); }

    void SetFiles( const std::vector<std::string>& files ) { m_files = files; }
    void SetDirectory( const std::string &directory ) { m_directory = directory; }
    void SetFilterIndex( int filterIndex ) { m_filterIndex = filterIndex; }

    std::vector<std::string> GetFiles() const { return m_files; }
    std::string GetDirectory() const { return m_directory; }
    int GetFilterIndex() const { return m_filterIndex; }

    std::string GetFile() const;

protected:
    std::string m_directory;
    std::vector<std::string> m_files;
    int m_filterIndex{};
};

typedef void ( wxEvtHandler::*wxFileCtrlEventFunction )( wxFileCtrlEvent& );

wxDECLARE_EVENT( wxEVT_FILECTRL_SELECTIONCHANGED, wxFileCtrlEvent );
wxDECLARE_EVENT( wxEVT_FILECTRL_FILEACTIVATED, wxFileCtrlEvent );
wxDECLARE_EVENT( wxEVT_FILECTRL_FOLDERCHANGED, wxFileCtrlEvent );
wxDECLARE_EVENT( wxEVT_FILECTRL_FILTERCHANGED, wxFileCtrlEvent );

#define wxFileCtrlEventHandler(func) \
    wxEVENT_HANDLER_CAST( wxFileCtrlEventFunction, func )

#define EVT_FILECTRL_FILEACTIVATED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_FILECTRL_FILEACTIVATED, id, wxFileCtrlEventHandler(fn))

#define EVT_FILECTRL_SELECTIONCHANGED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_FILECTRL_SELECTIONCHANGED, id, wxFileCtrlEventHandler(fn))

#define EVT_FILECTRL_FOLDERCHANGED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_FILECTRL_FOLDERCHANGED, id, wxFileCtrlEventHandler(fn))

#define EVT_FILECTRL_FILTERCHANGED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_FILECTRL_FILTERCHANGED, id, wxFileCtrlEventHandler(fn))

#endif // wxUSE_FILECTRL

#endif // _WX_FILECTRL_H_BASE_
