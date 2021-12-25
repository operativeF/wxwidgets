/////////////////////////////////////////////////////////////////////////////
// Name:        wx/filehistory.h
// Purpose:     wxFileHistory class
// Author:      Julian Smart, Vaclav Slavik
// Created:     2010-05-03
// Copyright:   (c) Julian Smart, Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FILEHISTORY_H_
#define _WX_FILEHISTORY_H_

#if wxUSE_FILE_HISTORY

#include "wx/list.h"

import WX.Base.WinId;

import <filesystem>;
import <vector>;

class wxMenu;
class wxConfigBase;
class wxFileName;

enum class wxFileHistoryMenuPathStyle
{
    ShowIfDifferent,
    ShowNever,
    ShowAlways
};

namespace fs = std::filesystem;

// ----------------------------------------------------------------------------
// File history management
// ----------------------------------------------------------------------------

class wxFileHistoryBase
{
public:
    wxFileHistoryBase(size_t maxFiles = 9, wxWindowID idBase = wxID_FILE1);

	wxFileHistoryBase& operator=(wxFileHistoryBase&&) = delete;

    virtual ~wxFileHistoryBase() = default;

    // Operations
    virtual void AddFileToHistory(const fs::path& file);
    virtual void RemoveFileFromHistory(size_t i);
    virtual int GetMaxFiles() const { return (int)m_fileMaxFiles; }
    virtual void UseMenu(wxMenu *menu);

    // Remove menu from the list (MDI child may be closing)
    virtual void RemoveMenu(wxMenu *menu);

#if wxUSE_CONFIG
    virtual void Load(const wxConfigBase& config);
    virtual void Save(wxConfigBase& config);
#endif // wxUSE_CONFIG

    virtual void AddFilesToMenu();
    virtual void AddFilesToMenu(wxMenu* menu); // Single menu

    // Accessors
    virtual fs::path GetHistoryFile(size_t i) const { return m_fileHistory[i]; }
    virtual size_t GetCount() const { return m_fileHistory.size(); }

    const wxList& GetMenus() const { return m_fileMenus; }

    // Set/get base id
    void SetBaseId(wxWindowID baseId) { m_idBase = baseId; }
    wxWindowID GetBaseId() const { return m_idBase; }

    void SetMenuPathStyle(wxFileHistoryMenuPathStyle style);
    wxFileHistoryMenuPathStyle GetMenuPathStyle() const { return m_menuPathStyle; }

protected:
    // Last n files
    std::vector<fs::path>       m_fileHistory;

    // Menus to maintain (may need several for an MDI app)
    wxList                      m_fileMenus;

    // Max files to maintain
    size_t                      m_fileMaxFiles;

    // Style of the paths in the menu labels
    wxFileHistoryMenuPathStyle m_menuPathStyle{wxFileHistoryMenuPathStyle::ShowIfDifferent};

private:
    void DoRefreshLabels();

    // The ID of the first history menu item (Doesn't have to be wxID_FILE1)
    wxWindowID m_idBase;

    // Normalize a file name to canonical form. We have a special function for
    // this to ensure the same normalization is used everywhere.
    static std::string NormalizeFileName(const wxFileName& filename);

    // Remove any existing entries from the associated menus.
    void RemoveExistingHistory();
};

#if defined(__WXGTK20__)
    #include "wx/gtk/filehistory.h"
#else
    // no platform-specific implementation of wxFileHistory yet
    class wxFileHistory : public wxFileHistoryBase
    {
    public:
        wxFileHistory(size_t maxFiles = 9, wxWindowID idBase = wxID_FILE1)
            : wxFileHistoryBase(maxFiles, idBase) {}
    };
#endif

#endif // wxUSE_FILE_HISTORY

#endif // _WX_FILEHISTORY_H_
