///////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/filectrlg.h
// Purpose:     wxGenericFileCtrl Header
// Author:      Diaa M. Sami
// Modified by:
// Created:     Jul-07-2007
// Copyright:   (c) Diaa M. Sami
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_FILECTRL_H_
#define _WX_GENERIC_FILECTRL_H_

#if wxUSE_FILECTRL

#include "wx/generic/dirctrlg.h"
#include "wx/containr.h"
#include "wx/listctrl.h"
#include "wx/filectrl.h"
#include "wx/filedlg.h"
#include "wx/filename.h"

import <string>;
import <vector>;

class wxCheckBox;
class wxChoice;
class wxStaticText;
class wxTextCtrl;

//-----------------------------------------------------------------------------
//  wxFileData - a class to hold the file info for the wxFileListCtrl
//-----------------------------------------------------------------------------

class wxFileData
{
public:
    enum fileType
    {
        is_file  = 0x0000,
        is_dir   = 0x0001,
        is_link  = 0x0002,
        is_exe   = 0x0004,
        is_drive = 0x0008
    };

    wxFileData() = default;

    // Full copy constructor
    wxFileData( const wxFileData& fileData ) { Copy(fileData); }
    // Create a filedata from this information
    wxFileData( const std::string& filePath, const std::string& fileName,
                fileType type, int image_id );

    // make a full copy of the other wxFileData
    void Copy( const wxFileData &other );

    // (re)read the extra data about the file from the system
    void ReadData();

    // get the name of the file, dir, drive
    std::string GetFileName() const { return m_fileName; }
    // get the full path + name of the file, dir, path
    std::string GetFilePath() const { return m_filePath; }
    // Set the path + name and name of the item
    void SetNewName( const std::string &filePath, const std::string &fileName );

    // Get the size of the file in bytes
    wxFileOffset GetSize() const { return m_size; }
    // Get the type of file, either file extension or <DIR>, <LINK>, <DRIVE>
    std::string GetFileType() const;
    // get the last modification time
    wxDateTime GetDateTime() const { return m_dateTime; }
    // Get the time as a formatted string
    std::string GetModificationTime() const;
    // in UNIX get rwx for file, in MSW get attributes ARHS
    std::string GetPermissions() const { return m_permissions; }
    // Get the id of the image used in a wxImageList
    int GetImageId() const { return m_image; }

    bool IsFile() const  { return !IsDir() && !IsLink() && !IsDrive(); }
    bool IsDir() const   { return (m_type & is_dir  ) != 0; }
    bool IsLink() const  { return (m_type & is_link ) != 0; }
    bool IsExe() const   { return (m_type & is_exe  ) != 0; }
    bool IsDrive() const { return (m_type & is_drive) != 0; }

    // Get/Set the type of file, file/dir/drive/link
    int GetType() const { return m_type; }

    // the wxFileListCtrl fields in report view
    enum fileListFieldType
    {
        FileList_Name,
        FileList_Size,
        FileList_Type,
        FileList_Time,
#if defined(__UNIX__) || defined(__WIN32__)
        FileList_Perm,
#endif // defined(__UNIX__) || defined(__WIN32__)
        FileList_Max
    };

    // Get the entry for report view of wxFileListCtrl
    std::string GetEntry( fileListFieldType num ) const;

    // Get a string representation of the file info
    std::string GetHint() const;
    // initialize a wxListItem attributes
    void MakeItem( wxListItem &item );

    // operators
    wxFileData& operator = (const wxFileData& fd) { Copy(fd); return *this; }

protected:
    std::string m_fileName;
    std::string   m_filePath;
    std::string m_permissions;

    wxDateTime m_dateTime;

    wxFileOffset m_size{0};
    unsigned int m_type{wxFileData::is_file};
    int      m_image{wxFileIconsTable::file};    
};

//-----------------------------------------------------------------------------
//  wxFileListCtrl
//-----------------------------------------------------------------------------

class wxFileListCtrl : public wxListCtrl
{
public:
    wxFileListCtrl() = default;
    wxFileListCtrl( wxWindow *win,
                wxWindowID id,
                const std::string &wild,
                bool showHidden,
                const wxPoint &pos = wxDefaultPosition,
                const wxSize &size = wxDefaultSize,
                unsigned int style = wxLC_LIST,
                const wxValidator &validator = {},
                const std::string &name = "filelist" );
    ~wxFileListCtrl();

    virtual void ChangeToListMode();
    virtual void ChangeToReportMode();
    virtual void ChangeToSmallIconMode();
    virtual void ShowHidden( bool show = true );
    bool GetShowHidden() const { return m_showHidden; }

    virtual long Add( wxFileData *fd, wxListItem &item );
    virtual void UpdateItem(const wxListItem &item);
    virtual void UpdateFiles();
    virtual void MakeDir();
    virtual void GoToParentDir();
    virtual void GoToHomeDir();
    virtual void GoToDir( const std::string &dir );
    virtual void SetWild( const std::string &wild );
    std::string GetWild() const { return m_wild; }
    std::string GetDir() const { return m_dirName; }

    void OnListDeleteItem( wxListEvent &event );
    void OnListDeleteAllItems( wxListEvent &event );
    void OnListEndLabelEdit( wxListEvent &event );
    void OnListColClick( wxListEvent &event );
    void OnSize( wxSizeEvent &event );

    virtual void SortItems(wxFileData::fileListFieldType field, bool forward);
    bool GetSortDirection() const { return m_sort_forward; }
    wxFileData::fileListFieldType GetSortField() const { return m_sort_field; }

protected:
    void FreeItemData(wxListItem& item);
    void FreeAllItemsData();

    std::string      m_dirName;
    std::string      m_wild;

    wxFileData::fileListFieldType m_sort_field{wxFileData::FileList_Name};

    bool m_showHidden{false};
    bool m_sort_forward{true};

private:
    wxDECLARE_EVENT_TABLE();
};

class wxGenericFileCtrl : public wxNavigationEnabled<wxControl>,
                                           public wxFileCtrlBase
{
public:
    wxGenericFileCtrl() = default;

    wxGenericFileCtrl ( wxWindow *parent,
                        wxWindowID id,
                        const std::string& defaultDirectory = {},
                        const std::string& defaultFilename = {},
                        const std::string& wildCard = wxFileSelectorDefaultWildcardStr,
                        unsigned int style = wxFC_DEFAULT_STYLE,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        std::string_view name = wxFileCtrlNameStr)
    {
        m_ignoreChanges = false;
        Create(parent, id, defaultDirectory, defaultFilename, wildCard,
               style, pos, size, name );
    }

    bool Create( wxWindow *parent,
                 wxWindowID id,
                 const std::string& defaultDirectory = {},
                 const std::string& defaultFileName = {},
                 const std::string& wildCard = wxFileSelectorDefaultWildcardStr,
                 unsigned int style = wxFC_DEFAULT_STYLE,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 std::string_view name = wxFileCtrlNameStr);

    void SetWildcard( const std::string& wildCard ) override;
    void SetFilterIndex( int filterindex ) override;
    bool SetDirectory( const std::string& dir ) override;

    // Selects a certain file.
    // In case the filename specified isn't found/couldn't be shown with
    // currently selected filter, false is returned and nothing happens
    bool SetFilename( const std::string& name ) override;

    // Changes to a certain directory and selects a certain file.
    // In case the filename specified isn't found/couldn't be shown with
    // currently selected filter, false is returned and if directory exists
    // it's chdir'ed to
    bool SetPath( const std::string& path ) override;

    std::string GetFilename() const override;
    std::string GetDirectory() const override;
    std::string GetWildcard() const override { return this->m_wildCard; }
    std::string GetPath() const override;
    std::vector<std::string> GetPaths() const override;
    std::vector<std::string> GetFilenames() const override;
    int GetFilterIndex() const override { return m_filterIndex; }

    bool HasMultipleFileSelection() const override
        { return HasFlag(wxFC_MULTIPLE); }
    void ShowHidden(bool show) override { m_list->ShowHidden( show ); }

    void GoToParentDir();
    void GoToHomeDir();

    // get the directory currently shown in the control: this can be different
    // from GetDirectory() if the user entered a full path (with a path other
    // than the one currently shown in the control) in the text control
    // manually
    std::string GetShownDirectory() const { return m_list->GetDir(); }

    wxFileListCtrl *GetFileList() { return m_list; }

    void ChangeToReportMode() { m_list->ChangeToReportMode(); }
    void ChangeToListMode() { m_list->ChangeToListMode(); }


private:
    void OnChoiceFilter( wxCommandEvent &event );
    void OnCheck( wxCommandEvent &event );
    void OnActivated( wxListEvent &event );
    void OnTextEnter( [[maybe_unused]] wxCommandEvent& event );
    void OnTextChange( [[maybe_unused]] wxCommandEvent& event );
    void OnSelected( wxListEvent &event );
    void HandleAction( const std::string &fn );

    void DoSetFilterIndex( int filterindex );
    void UpdateControls();

    // the first of these methods can only be used for the controls with single
    // selection (i.e. without wxFC_MULTIPLE style), the second one in any case
    wxFileName DoGetFileName() const;
    std::vector<std::string> DoGetFilenames(bool fullPath ) const;

    unsigned int m_style;

    std::string         m_filterExtension;
    wxChoice        *m_choice;
    wxTextCtrl      *m_text;
    wxFileListCtrl  *m_list;
    wxCheckBox      *m_check;
    wxStaticText    *m_static;

    std::string        m_dir;
    std::string        m_fileName;
    std::string        m_wildCard; // wild card in one string as we got it

    int     m_filterIndex{};
    bool    m_inSelected{};
    bool    m_ignoreChanges{false};
    bool    m_noSelChgEvent{}; // suppress selection changed events.

    wxDECLARE_EVENT_TABLE();
};

#endif // wxUSE_FILECTRL

#endif // _WX_GENERIC_FILECTRL_H_
