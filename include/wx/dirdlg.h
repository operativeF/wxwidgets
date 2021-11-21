/////////////////////////////////////////////////////////////////////////////
// Name:        wx/dirdlg.h
// Purpose:     wxDirDialog base class
// Author:      Robert Roebling
// Modified by:
// Created:
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DIRDLG_H_BASE_
#define _WX_DIRDLG_H_BASE_

#if wxUSE_DIRDLG

#include "wx/defs.h"

#include "wx/dialog.h"

import <string>;
import <vector>;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

inline constexpr std::string_view wxDirDialogNameStr = "wxDirCtrl";
inline constexpr std::string_view wxDirDialogDefaultFolderStr = "/";
inline constexpr std::string_view wxDirSelectorPromptStr = "Select a directory";


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

inline constexpr unsigned int wxDD_CHANGE_DIR         = 0x0100;
inline constexpr unsigned int wxDD_DIR_MUST_EXIST     = 0x0200;
inline constexpr unsigned int wxDD_MULTIPLE           = 0x0400;
inline constexpr unsigned int wxDD_SHOW_HIDDEN        = 0x0001;

inline constexpr unsigned int wxDD_DEFAULT_STYLE = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER;

//-------------------------------------------------------------------------
// wxDirDialogBase
//-------------------------------------------------------------------------

class wxDirDialogBase : public wxDialog
{
public:
    wxDirDialogBase() = default;
    wxDirDialogBase(wxWindow *parent,
                    std::string_view title = wxDirSelectorPromptStr,
                    const std::string& defaultPath = {},
                    unsigned int style = wxDD_DEFAULT_STYLE,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& sz = wxDefaultSize,
                    std::string_view name = wxDirDialogNameStr)
    {
        Create(parent, title, defaultPath, style, pos, sz, name);
    }

    [[maybe_unused]] bool Create(wxWindow *parent,
                std::string_view title = wxDirSelectorPromptStr,
                const std::string& defaultPath = {},
                unsigned int style = wxDD_DEFAULT_STYLE,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& sz = wxDefaultSize,
                std::string_view name = wxDirDialogNameStr)
    {
        if (!wxDialog::Create(parent, wxID_ANY, title, pos, sz, style, name))
            return false;
        m_path = defaultPath;
        m_message = title;
        return true;
    }

    virtual void SetMessage(const std::string& message) { m_message = message; }
    virtual void SetPath(const std::string& path) { m_path = path; }

    virtual const std::string& wxGetMessage() const { return m_message; }
    virtual const std::string& GetPath() const
    {
        // FIXME: returns local variable.
        //wxCHECK_MSG( !HasFlag(wxDD_MULTIPLE), "",
        //             "When using wxDD_MULTIPLE, must call GetPaths() instead" );
        return m_path;
    }

    virtual std::vector<std::string> GetPaths() const
    {
        return m_paths;
    }

protected:
    std::string m_message;
    std::string m_path;
    std::vector<std::string> m_paths;
};


// Universal and non-port related switches with need for generic implementation
#if defined(__WXUNIVERSAL__)
    #include "wx/generic/dirdlgg.h"
    using wxDirDialog = wxGenericDirDialog;
#elif defined(__WXMSW__) && !wxUSE_OLE
    #include "wx/generic/dirdlgg.h"
    using wxDirDialog = wxGenericDirDialog;
#elif defined(__WXMSW__)
    #include "wx/msw/dirdlg.h"  // Native MSW
#elif defined(__WXGTK20__)
    #include "wx/gtk/dirdlg.h"  // Native GTK for gtk2.4
#elif defined(__WXGTK__)
    #include "wx/generic/dirdlgg.h"
    using wxDirDialog = wxGenericDirDialog;
#elif defined(__WXMAC__)
    #include "wx/osx/dirdlg.h"      // Native Mac
#elif defined(__WXMOTIF__) || \
      defined(__WXX11__)
    #include "wx/generic/dirdlgg.h"     // Other ports use generic implementation
    using wxDirDialog = wxGenericDirDialog;
#elif defined(__WXQT__)
    #include "wx/qt/dirdlg.h"
#endif

// ----------------------------------------------------------------------------
// common ::wxDirSelector() function
// ----------------------------------------------------------------------------

wxString
wxDirSelector(std::string_view message = wxDirSelectorPromptStr,
              const wxString& defaultPath = {},
              unsigned int style = wxDD_DEFAULT_STYLE,
              const wxPoint& pos = wxDefaultPosition,
              wxWindow *parent = nullptr);

#endif // wxUSE_DIRDLG

#endif
    // _WX_DIRDLG_H_BASE_
