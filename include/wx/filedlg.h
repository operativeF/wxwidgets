/////////////////////////////////////////////////////////////////////////////
// Name:        wx/filedlg.h
// Purpose:     wxFileDialog base header
// Author:      Robert Roebling
// Modified by:
// Created:     8/17/99
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FILEDLG_H_BASE_
#define _WX_FILEDLG_H_BASE_

#if wxUSE_FILEDLG

#include "wx/defs.h"

#include "wx/dialog.h"

import <string>;

// this symbol is defined for the platforms which support multiple
// ('|'-separated) filters in the file dialog
#if defined(__WXMSW__) || defined(__WXGTK__) || defined(__WXMAC__)
    #define wxHAS_MULTIPLE_FILEDLG_FILTERS
#endif

//----------------------------------------------------------------------------
// wxFileDialog data
//----------------------------------------------------------------------------

/*
    The flags below must coexist with the following flags in m_windowStyle
    #define wxCAPTION               0x20000000
    #define wxMAXIMIZE              0x00002000
    #define wxCLOSE_BOX             0x00001000
    #define wxSYSTEM_MENU           0x00000800
    wxBORDER_NONE   =               0x00200000
    #define wxRESIZE_BORDER         0x00000040
    #define wxDIALOG_NO_PARENT      0x00000020
*/

enum
{
    wxFD_OPEN              = 0x0001,
    wxFD_SAVE              = 0x0002,
    wxFD_OVERWRITE_PROMPT  = 0x0004,
    wxFD_NO_FOLLOW         = 0x0008,
    wxFD_FILE_MUST_EXIST   = 0x0010,
    wxFD_CHANGE_DIR        = 0x0080,
    wxFD_PREVIEW           = 0x0100,
    wxFD_MULTIPLE          = 0x0200,
    wxFD_SHOW_HIDDEN       = 0x0400
};

inline constexpr unsigned int wxFD_DEFAULT_STYLE = wxFD_OPEN;

inline constexpr char wxFileDialogNameStr[] = "filedlg";
inline constexpr char wxFileSelectorPromptStr[] = "Select a file";

inline constexpr char wxFileSelectorDefaultWildcardStr[] =
#if defined(__WXMSW__)
    "*.*"
#else // Unix/Mac
    "*"
#endif
    ;

//----------------------------------------------------------------------------
// wxFileDialogBase
//----------------------------------------------------------------------------

class wxFileDialogBase: public wxDialog
{
public:
    wxFileDialogBase() = default;

    wxFileDialogBase(wxWindow *parent,
                     const std::string& message = wxFileSelectorPromptStr,
                     const std::string& defaultDir = {},
                     const std::string& defaultFile = {},
                     const std::string& wildCard = wxFileSelectorDefaultWildcardStr,
                     unsigned int style = wxFD_DEFAULT_STYLE,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& sz = wxDefaultSize,
                     const std::string& name = wxFileDialogNameStr)
    {
        Create(parent, message, defaultDir, defaultFile, wildCard, style, pos, sz, name);
    }

    wxFileDialogBase& operator=(wxFileDialogBase&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                const std::string& message = wxFileSelectorPromptStr,
                const std::string& defaultDir = {},
                const std::string& defaultFile = {},
                const std::string& wildCard = wxFileSelectorDefaultWildcardStr,
                unsigned int style = wxFD_DEFAULT_STYLE,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& sz = wxDefaultSize,
                const std::string& name = wxFileDialogNameStr);

    bool HasFdFlag(unsigned int flag) const { return HasFlag(flag); }

    virtual void SetMessage(const std::string& message) { m_message = message; }
    virtual void SetPath(const std::string& path);
    virtual void SetDirectory(const std::string& dir);
    virtual void SetFilename(const std::string& name);
    virtual void SetWildcard(const std::string& wildCard) { m_wildCard = wildCard; }
    virtual void SetFilterIndex(int filterIndex) { m_filterIndex = filterIndex; }

    virtual std::string wxGetMessage() const { return m_message; }
    virtual std::string GetPath() const
    {
        wxCHECK_MSG( !HasFlag(wxFD_MULTIPLE), std::string(), "When using wxFD_MULTIPLE, must call GetPaths() instead" );
        return m_path;
    }

    virtual std::vector<std::string> GetPaths() const
    {
        return {m_path};
    }

    virtual std::string GetDirectory() const { return m_dir; }
    virtual std::string GetFilename() const
    {
        wxCHECK_MSG( !HasFlag(wxFD_MULTIPLE), std::string(), "When using wxFD_MULTIPLE, must call GetFilenames() instead" );
        return m_fileName;
    }
    virtual std::vector<std::string> GetFilenames() const
    {
        return {m_fileName};
    }
    virtual std::string GetWildcard() const { return m_wildCard; }
    virtual int GetFilterIndex() const { return m_filterIndex; }

    virtual std::string GetCurrentlySelectedFilename() const
        { return m_currentlySelectedFilename; }

    virtual int GetCurrentlySelectedFilterIndex () const
        { return m_currentlySelectedFilterIndex; }

    // this function is called with wxFileDialog as parameter and should
    // create the window containing the extra controls we want to show in it
    typedef wxWindow *(*ExtraControlCreatorFunction)(wxWindow*);

    virtual bool SupportsExtraControl() const { return false; }

    bool SetExtraControlCreator(ExtraControlCreatorFunction creator);
    wxWindow *GetExtraControl() const { return m_extraControl; }

    // Utility functions

    // Append first extension to filePath from a ';' separated extensionList
    // if filePath = "path/foo.bar" just return it as is
    // if filePath = "foo[.]" and extensionList = "*.jpg;*.png" return "foo.jpg"
    // if the extension is "*.j?g" (has wildcards) or "jpg" then return filePath
    static std::string AppendExtension(const std::string &filePath,
                                    const std::string &extensionList);

    // Set the filter index to match the given extension.
    //
    // This is always valid to call, even if the extension is empty or the
    // filter list doesn't contain it, the function will just do nothing in
    // these cases.
    void SetFilterIndexFromExt(const std::string& ext);

protected:
    std::string      m_message;
    std::string      m_dir;
    std::string   m_path;       // Full path
    std::string      m_fileName;
    std::string      m_wildCard;
    
    // Currently selected, but not yet necessarily accepted by the user, file.
    // This should be updated whenever the selection in the control changes by
    // the platform-specific code to provide a useful implementation of
    // GetCurrentlySelectedFilename().
    std::string      m_currentlySelectedFilename;

    wxWindow*     m_extraControl{nullptr};

    int           m_filterIndex{0};

    // Currently selected, but not yet necessarily accepted by the user, file
    // type (a.k.a. filter) index. This should be updated whenever the
    // selection in the control changes by the platform-specific code to
    // provide a useful implementation of GetCurrentlySelectedFilterIndex().
    int           m_currentlySelectedFilterIndex{wxNOT_FOUND};

    // returns true if control is created (if it already exists returns false)
    bool CreateExtraControl();
    // return true if SetExtraControlCreator() was called
    bool HasExtraControlCreator() const
        { return m_extraControlCreator != nullptr; }
    // get the size of the extra control by creating and deleting it
    wxSize GetExtraControlSize();
    // Helper function for native file dialog usage where no wx events
    // are processed.
    void UpdateExtraControlUI();

private:
    ExtraControlCreatorFunction m_extraControlCreator{nullptr};

    wxDECLARE_DYNAMIC_CLASS(wxFileDialogBase);
};


//----------------------------------------------------------------------------
// wxFileDialog convenience functions
//----------------------------------------------------------------------------

// File selector - backward compatibility
std::string
wxFileSelector(const std::string& message = wxFileSelectorPromptStr,
               const std::string& default_path = {},
               const std::string& default_filename = {},
               const std::string& default_extension = {},
               const std::string& wildcard = wxFileSelectorDefaultWildcardStr,
               unsigned int flags = 0,
               wxWindow *parent = nullptr,
               int x = wxDefaultCoord, int y = wxDefaultCoord);

// An extended version of wxFileSelector
std::string
wxFileSelectorEx(const std::string& message = wxFileSelectorPromptStr,
                 const std::string& default_path = {},
                 const std::string& default_filename = {},
                 int *indexDefaultExtension = nullptr,
                 const std::string& wildcard = wxFileSelectorDefaultWildcardStr,
                 unsigned int flags = 0,
                 wxWindow *parent = nullptr,
                 int x = wxDefaultCoord, int y = wxDefaultCoord);

// Ask for filename to load
std::string
wxLoadFileSelector(const std::string& what,
                   const std::string& extension,
                   const std::string& default_name = {},
                   wxWindow *parent = nullptr);

// Ask for filename to save
std::string
wxSaveFileSelector(const std::string& what,
                   const std::string& extension,
                   const std::string& default_name = {},
                   wxWindow *parent = nullptr);


#if defined (__WXUNIVERSAL__)
    #define wxHAS_GENERIC_FILEDIALOG
    #include "wx/generic/filedlgg.h"
#elif defined(__WXMSW__)
    #include "wx/msw/filedlg.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/filedlg.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/filedlg.h"     // GTK+ > 2.4 has native version
#elif defined(__WXGTK__)
    #include "wx/gtk1/filedlg.h"
#elif defined(__WXMAC__)
    #include "wx/osx/filedlg.h"
#elif defined(__WXQT__)
    #include "wx/qt/filedlg.h"
#endif

#endif // wxUSE_FILEDLG

#endif // _WX_FILEDLG_H_BASE_
