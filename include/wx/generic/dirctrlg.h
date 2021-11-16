/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/dirctrlg.h
// Purpose:     wxGenericDirCtrl class
//              Builds on wxDirCtrl class written by Robert Roebling for the
//              wxFile application, modified by Harm van der Heijden.
//              Further modified for Windows.
// Author:      Robert Roebling, Harm van der Heijden, Julian Smart et al
// Modified by:
// Created:     21/3/2000
// Copyright:   (c) Robert Roebling, Harm van der Heijden, Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DIRCTRL_H_
#define _WX_DIRCTRL_H_

#if wxUSE_DIRDLG || wxUSE_FILEDLG

#include "wx/treectrl.h"
#include "wx/dialog.h"
#include "wx/dirdlg.h"
#include "wx/choice.h"

//-----------------------------------------------------------------------------
// classes
//-----------------------------------------------------------------------------

class wxTextCtrl;
class wxHashTable;

//-----------------------------------------------------------------------------
// Extra styles for wxGenericDirCtrl
//-----------------------------------------------------------------------------

enum
{
    // Only allow directory viewing/selection, no files
    wxDIRCTRL_DIR_ONLY       = 0x0010,
    // When setting the default path, select the first file in the directory
    wxDIRCTRL_SELECT_FIRST   = 0x0020,
    // Show the filter list
    wxDIRCTRL_SHOW_FILTERS   = 0x0040,
    // Use 3D borders on internal controls
    wxDIRCTRL_3D_INTERNAL    = 0x0080,
    // Editable labels
    wxDIRCTRL_EDIT_LABELS    = 0x0100,
    // Allow multiple selection
    wxDIRCTRL_MULTIPLE       = 0x0200,

    wxDIRCTRL_DEFAULT_STYLE  = wxDIRCTRL_3D_INTERNAL
};

//-----------------------------------------------------------------------------
// wxDirItemData
//-----------------------------------------------------------------------------

class wxDirItemData : public wxTreeItemData
{
public:
    wxDirItemData(const std::string& path, const std::string& name, bool isDir);

    void SetNewDirName(const std::string& path);

    bool HasSubDirs() const;
    bool HasFiles(const std::string& spec = {}) const;

    std::string m_path, m_name;
    bool m_isHidden{false};
    bool m_isExpanded{false};
    bool m_isDir;
};

//-----------------------------------------------------------------------------
// wxDirCtrl
//-----------------------------------------------------------------------------

class wxDirFilterListCtrl;

class wxGenericDirCtrl: public wxControl
{
public:
    wxGenericDirCtrl() = default;
    wxGenericDirCtrl(wxWindow *parent, wxWindowID id = wxID_ANY,
              const std::string& dir = wxDirDialogDefaultFolderStr,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              unsigned int style = wxDIRCTRL_DEFAULT_STYLE,
              const std::string& filter = {},
              int defaultFilter = 0,
              const std::string& name = wxTreeCtrlNameStr)
    {
        Create(parent, id, dir, pos, size, style, filter, defaultFilter, name);
    }

    wxGenericDirCtrl(const wxGenericDirCtrl&) = delete;
	wxGenericDirCtrl& operator=(const wxGenericDirCtrl&) = delete;

    bool Create(wxWindow *parent, wxWindowID id = wxID_ANY,
              const std::string& dir = wxDirDialogDefaultFolderStr,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              unsigned int style = wxDIRCTRL_DEFAULT_STYLE,
              const std::string& filter = {},
              int defaultFilter = 0,
              const std::string& name = wxTreeCtrlNameStr);

    void OnExpandItem(wxTreeEvent &event );
    void OnCollapseItem(wxTreeEvent &event );
    void OnBeginEditItem(wxTreeEvent &event );
    void OnEndEditItem(wxTreeEvent &event );
    void OnTreeSelChange(wxTreeEvent &event);
    void OnItemActivated(wxTreeEvent &event);
    void OnSize(wxSizeEvent &event );

    // Try to expand as much of the given path as possible.
    virtual bool ExpandPath(const std::string& path);
    // collapse the path
    virtual bool CollapsePath(const std::string& path);

    // Accessors

    virtual inline std::string GetDefaultPath() const { return m_defaultPath; }
    virtual void SetDefaultPath(const std::string& path) { m_defaultPath = path; }

    // Get dir or filename
    virtual std::string GetPath() const;
    virtual std::vector<std::string> GetPaths() const;

    // Get selected filename path only (else empty string).
    // I.e. don't count a directory as a selection
    virtual std::string GetFilePath() const;
    virtual void GetFilePaths(std::vector<std::string>& paths) const;
    virtual void SetPath(const std::string& path);

    virtual void SelectPath(const std::string& path, bool select = true);
    virtual void SelectPaths(const std::vector<std::string>& paths);

    virtual void ShowHidden( bool show );
    virtual bool GetShowHidden() { return m_showHidden; }

    virtual std::string GetFilter() const { return m_filter; }
    virtual void SetFilter(const std::string& filter);

    virtual int GetFilterIndex() const { return m_currentFilter; }
    virtual void SetFilterIndex(int n);

    virtual wxTreeItemId GetRootId() { return m_rootId; }

    virtual wxTreeCtrl* GetTreeCtrl() const { return m_treeCtrl; }
    virtual wxDirFilterListCtrl* GetFilterListCtrl() const { return m_filterListCtrl; }

    virtual void UnselectAll();

    // Helper
    virtual void SetupSections();

    // Find the child that matches the first part of 'path'.
    // E.g. if a child path is "/usr" and 'path' is "/usr/include"
    // then the child for /usr is returned.
    // If the path string has been used (we're at the leaf), done is set to true
    virtual wxTreeItemId FindChild(wxTreeItemId parentId, const std::string& path, bool& done);

    std::string GetPath(wxTreeItemId itemId) const;

    // Resize the components of the control
    virtual void DoResize();

    // Collapse & expand the tree, thus re-creating it from scratch:
    virtual void ReCreateTree();

    // Collapse the entire tree
    virtual void CollapseTree();

    // overridden base class methods
    void SetFocus() override;

protected:
    virtual void ExpandRoot();
    virtual void ExpandDir(wxTreeItemId parentId);
    virtual void CollapseDir(wxTreeItemId parentId);
    virtual const wxTreeItemId AddSection(const std::string& path, const std::string& name, int imageId = 0);
    virtual wxTreeItemId AppendItem (const wxTreeItemId & parent,
                const std::string & text,
                int image = -1, int selectedImage = -1,
                wxTreeItemData * data = nullptr);
    //void FindChildFiles(wxTreeItemId id, int dirFlags, std::vector<std::string>& filenames);
    virtual wxTreeCtrl* CreateTreeCtrl(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, unsigned int treeStyle);

    // Extract description and actual filter from overall filter string
    bool ExtractWildcard(const std::string& filterStr, int n, std::string& filter, std::string& description);

private:
    void PopulateNode(wxTreeItemId node);

    std::string        m_defaultPath; // Starting path
    std::string        m_filter;  // Wildcards in same format as per wxFileDialog
    std::string        m_currentFilterStr; // Current filter string

    wxTreeItemId    m_rootId;

    wxDirItemData* GetItemData(wxTreeItemId itemId);
    wxTreeCtrl*     m_treeCtrl{nullptr};
    wxDirFilterListCtrl* m_filterListCtrl{nullptr};
    
    long            m_styleEx{}; // Extended style
    int             m_currentFilter{0}; // The current filter index

    bool            m_showHidden{false};

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_DYNAMIC_CLASS(wxGenericDirCtrl);
};

wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_DIRCTRL_SELECTIONCHANGED, wxTreeEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_DIRCTRL_FILEACTIVATED, wxTreeEvent );

#define wx__DECLARE_DIRCTRL_EVT(evt, id, fn) \
    wx__DECLARE_EVT1(wxEVT_DIRCTRL_ ## evt, id, wxTreeEventHandler(fn))

#define EVT_DIRCTRL_SELECTIONCHANGED(id, fn) wx__DECLARE_DIRCTRL_EVT(SELECTIONCHANGED, id, fn)
#define EVT_DIRCTRL_FILEACTIVATED(id, fn) wx__DECLARE_DIRCTRL_EVT(FILEACTIVATED, id, fn)

//-----------------------------------------------------------------------------
// wxDirFilterListCtrl
//-----------------------------------------------------------------------------

class wxDirFilterListCtrl: public wxChoice
{
public:
    wxDirFilterListCtrl() = default;
    wxDirFilterListCtrl(wxGenericDirCtrl* parent, wxWindowID id = wxID_ANY,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              unsigned int style = 0)
    {
        Create(parent, id, pos, size, style);
    }

    wxDirFilterListCtrl(const wxDirFilterListCtrl&) = delete;
	wxDirFilterListCtrl& operator=(const wxDirFilterListCtrl&) = delete;

    bool Create(wxGenericDirCtrl* parent, wxWindowID id = wxID_ANY,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              unsigned int style = 0);

    //// Operations
    void FillFilterList(const std::string& filter, int defaultFilter);

    //// Events
    void OnSelFilter(wxCommandEvent& event);

protected:
    wxGenericDirCtrl*    m_dirCtrl{nullptr};

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_CLASS(wxDirFilterListCtrl);
};

#if !defined(__WXMSW__) && !defined(__WXMAC__)
    #define wxDirCtrl wxGenericDirCtrl
#endif

// Symbols for accessing individual controls
#define wxID_TREECTRL          7000
#define wxID_FILTERLISTCTRL    7001

#endif // wxUSE_DIRDLG

//-------------------------------------------------------------------------
// wxFileIconsTable - use wxTheFileIconsTable which is created as necessary
//-------------------------------------------------------------------------

#if wxUSE_DIRDLG || wxUSE_FILEDLG || wxUSE_FILECTRL

class wxImageList;

class wxFileIconsTable
{
public:
    wxFileIconsTable();
    ~wxFileIconsTable();

    enum iconId_Type
    {
        folder,
        folder_open,
        computer,
        drive,
        cdrom,
        floppy,
        removeable,
        file,
        executable
    };

    int GetIconID(const std::string& extension, const std::string& mime = {});
    wxImageList *GetSmallImageList();

    const wxSize& GetSize() const { return m_size; }
    void SetSize(const wxSize& sz) { m_size = sz; }

    bool IsOk() const { return m_smallImageList != nullptr; }

protected:
    void Create(const wxSize& sz);  // create on first use

    wxImageList *m_smallImageList{nullptr};
    wxHashTable *m_HashTable{nullptr};
    wxSize  m_size;
};

// The global fileicons table
extern WXDLLIMPEXP_DATA_CORE(wxFileIconsTable *) wxTheFileIconsTable;

#endif // wxUSE_DIRDLG || wxUSE_FILEDLG || wxUSE_FILECTRL

#endif
    // _WX_DIRCTRLG_H_
