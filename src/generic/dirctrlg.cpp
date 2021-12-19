/////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/dirctrlg.cpp
// Purpose:     wxGenericDirCtrl
// Author:      Harm van der Heijden, Robert Roebling, Julian Smart
// Modified by:
// Created:     12/12/98
// Copyright:   (c) Harm van der Heijden, Robert Roebling and Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_DIRDLG || wxUSE_FILEDLG

#ifdef WX_WINDOWS
#include "wx/volume.h"

// MinGW has _getdrive() and _chdrive(), Cygwin doesn't.
#if defined(__GNUWIN32__) && !defined(__CYGWIN__)
    #define wxHAS_DRIVE_FUNCTIONS
#endif

#ifdef wxHAS_DRIVE_FUNCTIONS
    #include <direct.h>
#endif

#endif // WX_WINDOWS

#include "wx/generic/dirctrlg.h"

#include "wx/hash.h"
#include "wx/intl.h"
#include "wx/log.h"
#include "wx/utils.h"
#include "wx/button.h"
#include "wx/icon.h"
#include "wx/msgdlg.h"
#include "wx/choice.h"
#include "wx/textctrl.h"
#include "wx/sizer.h"
#include "wx/textdlg.h"
#include "wx/gdicmn.h"
#include "wx/module.h"
#include "wx/filedlg.h"
#include "wx/filename.h"
#include "wx/filefn.h"
#include "wx/imaglist.h"
#include "wx/dir.h"
#include "wx/artprov.h"
#include "wx/mimetype.h"

import WX.Image;

import Utils.Strings;
import WX.Utils.Settings;
import Utils.Geometry;

#if wxUSE_STATLINE
    #include "wx/statline.h"
#endif

#if defined(__WXMAC__)
    #include  "wx/osx/private.h"  // includes mac headers
#endif

#if defined(__WXMAC__)
//    #include "MoreFilesX.h"
#endif

// If compiled under Windows, this macro can cause problems
#ifdef GetFirstChild
#undef GetFirstChild
#endif

bool wxIsDriveAvailable(const std::string& dirName);

// ----------------------------------------------------------------------------
// events
// ----------------------------------------------------------------------------

wxDEFINE_EVENT( wxEVT_DIRCTRL_SELECTIONCHANGED, wxTreeEvent );
wxDEFINE_EVENT( wxEVT_DIRCTRL_FILEACTIVATED, wxTreeEvent );

// ----------------------------------------------------------------------------
// wxGetAvailableDrives, for WINDOWS, OSX, UNIX (returns "/")
// ----------------------------------------------------------------------------

// since the macOS implementation needs objective-C this is dirdlg.mm
#ifdef __WXOSX__
extern size_t wxGetAvailableDrives(std::vector<std::string> &paths, std::vector<std::string> &names, std::vector<int> &icon_ids);
#else
size_t wxGetAvailableDrives(std::vector<std::string> &paths, std::vector<std::string> &names, std::vector<int> &icon_ids)
{
#ifdef wxHAS_FILESYSTEM_VOLUMES

#if defined(__WIN32__) && wxUSE_FSVOLUME
    // TODO: this code (using wxFSVolumeBase) should be used for all platforms
    //       but unfortunately wxFSVolumeBase is not implemented everywhere
    const std::vector<std::string> as = wxFSVolumeBase::GetVolumes();

    for (const auto& path : as)
    {
        wxFSVolume vol(path);
        int imageId;
        switch (vol.GetKind())
        {
            case wxFSVolumeKind::Floppy:
                if ( (path == "a:\\") || (path == "b:\\") )
                    imageId = wxFileIconsTable::floppy;
                else
                    imageId = wxFileIconsTable::removeable;
                break;
            case wxFSVolumeKind::DVDROM:
            case wxFSVolumeKind::CDROM:
                imageId = wxFileIconsTable::cdrom;
                break;
            case wxFSVolumeKind::Network:
                if (path[0] == wxT('\\'))
                    continue; // skip "\\computer\folder"
                imageId = wxFileIconsTable::drive;
                break;
            case wxFSVolumeKind::Disk:
            case wxFSVolumeKind::Other:
            default:
                imageId = wxFileIconsTable::drive;
                break;
        }
        paths.push_back(path);
        names.push_back(vol.GetDisplayName());
        icon_ids.push_back(imageId);
    }
#else // !__WIN32__
    /* If we can switch to the drive, it exists. */
    for ( char drive = 'A'; drive <= 'Z'; drive++ )
    {
        const std::string
            path = wxFileName::GetVolumeString(drive, wxPATH_GET_SEPARATOR);

        if (wxIsDriveAvailable(path))
        {
            paths.Add(path);
            names.Add(wxFileName::GetVolumeString(drive, wxPATH_NO_SEPARATOR));
            icon_ids.Add(drive <= 2 ? wxFileIconsTable::floppy
                                    : wxFileIconsTable::drive);
        }
    }
#endif // __WIN32__/!__WIN32__

#elif defined(__UNIX__)
    paths.Add("/");
    names.Add("/");
    icon_ids.push_back(wxFileIconsTable::computer);
#else
    #error "Unsupported platform in wxGenericDirCtrl!"
#endif
    wxASSERT_MSG( (paths.size() == names.size()), "The number of paths and their human readable names should be equal in number.");
    wxASSERT_MSG( (paths.size() == icon_ids.size()), "Wrong number of icons for available drives.");
    return paths.size();
}
#endif

// ----------------------------------------------------------------------------
// wxIsDriveAvailable
// ----------------------------------------------------------------------------

#if defined(WX_WINDOWS)

int setdrive(int drive)
{
#if defined(wxHAS_DRIVE_FUNCTIONS)
    return _chdrive(drive);
#else
    wxChar  newdrive[4];

    if (drive < 1 || drive > 31)
        return -1;
    newdrive[0] = (wxChar)(wxT('A') + drive - 1);
    newdrive[1] = wxT(':');
    newdrive[2] = wxT('\0');
#if defined(WX_WINDOWS)
    if (::SetCurrentDirectoryW(newdrive))
#else
    // VA doesn't know what LPSTR is and has its own set
    if (!DosSetCurrentDir((PSZ)newdrive))
#endif
        return 0;
    else
        return -1;
#endif // !GNUWIN32
}

bool wxIsDriveAvailable(const std::string& dirName)
{
#ifdef __WIN32__
    WXUINT errorMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
#endif
    bool success = true;

    // Check if this is a root directory and if so,
    // whether the drive is available.
    if (dirName.length() == 3 && dirName[(size_t)1] == ':')
    {
        std::string dirNameLower = wx::utils::ToLowerCopy(dirName);
#ifndef wxHAS_DRIVE_FUNCTIONS
        success = wxDirExists(dirNameLower);
#else
        int currentDrive = _getdrive();
        int thisDrive = (int) (dirNameLower[(size_t)0] - 'a' + 1) ;
        int err = setdrive( thisDrive ) ;
        setdrive( currentDrive );

        if (err == -1)
        {
            success = false;
        }
#endif
    }
#ifdef __WIN32__
    SetErrorMode(errorMode);
#endif

    return success;
}
#endif // WX_WINDOWS

#endif // wxUSE_DIRDLG || wxUSE_FILEDLG

#if wxUSE_DIRDLG

// Function which is called by quick sort. We want to override the default std::vector<std::string> behaviour,
// and sort regardless of case.
static int wxCMPFUNC_CONV wxDirCtrlStringCompareFunction(const std::string& strFirst, const std::string& strSecond)
{
    return wx::utils::CmpNoCase(strFirst, strSecond);
}

//-----------------------------------------------------------------------------
// wxDirItemData
//-----------------------------------------------------------------------------

wxDirItemData::wxDirItemData(const std::string& path, const std::string& name,
                             bool isDir)
    : m_path(path)
    , m_name(name)
    , m_isDir(isDir)
{
    /* Insert logic to detect hidden files here
     * In UnixLand we just check whether the first char is a dot
     * For FileNameFromPath read LastDirNameInThisPath ;-) */
    // m_isHidden = (bool)(wxFileNameFromPath(*m_path)[0] == '.');
}

void wxDirItemData::SetNewDirName(const std::string& path)
{
    m_path = path;
    m_name = wxFileNameFromPath(path);
}

bool wxDirItemData::HasSubDirs() const
{
    if (m_path.empty())
        return false;

    wxDir dir;
    {
        wxLogNull nolog;
        if ( !dir.Open(m_path) )
            return false;
    }

    return dir.HasSubDirs();
}

bool wxDirItemData::HasFiles([[maybe_unused]] const std::string& spec) const
{
    if (m_path.empty())
        return false;

    wxDir dir;
    {
        wxLogNull nolog;
        if ( !dir.Open(m_path) )
            return false;
    }

    return dir.HasFiles();
}

//-----------------------------------------------------------------------------
// wxGenericDirCtrl
//-----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(wxGenericDirCtrl, wxControl)
  EVT_TREE_ITEM_EXPANDING     (wxID_TREECTRL, wxGenericDirCtrl::OnExpandItem)
  EVT_TREE_ITEM_COLLAPSED     (wxID_TREECTRL, wxGenericDirCtrl::OnCollapseItem)
  EVT_TREE_BEGIN_LABEL_EDIT   (wxID_TREECTRL, wxGenericDirCtrl::OnBeginEditItem)
  EVT_TREE_END_LABEL_EDIT     (wxID_TREECTRL, wxGenericDirCtrl::OnEndEditItem)
  EVT_TREE_SEL_CHANGED        (wxID_TREECTRL, wxGenericDirCtrl::OnTreeSelChange)
  EVT_TREE_ITEM_ACTIVATED     (wxID_TREECTRL, wxGenericDirCtrl::OnItemActivated)
  EVT_SIZE                    (wxGenericDirCtrl::OnSize)
wxEND_EVENT_TABLE()

void wxGenericDirCtrl::ExpandRoot()
{
    ExpandDir(m_rootId); // automatically expand first level

    // Expand and select the default path
    if (!m_defaultPath.empty())
    {
        ExpandPath(m_defaultPath);
    }
#ifdef __UNIX__
    else
    {
        // On Unix, there's only one node under the (hidden) root node. It
        // represents the / path, so the user would always have to expand it;
        // let's do it ourselves
        ExpandPath( "/" );
    }
#endif
}

bool wxGenericDirCtrl::Create(wxWindow *parent,
                              wxWindowID treeid,
                              std::string_view dir,
                              const wxPoint& pos,
                              const wxSize& size,
                              unsigned int style,
                              const std::string& filter,
                              int defaultFilter,
                              std::string_view name)
{
    if (!wxControl::Create(parent, treeid, pos, size, style, wxValidator{}, name))
        return false;

    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));

    unsigned int treeStyle = wxTR_HAS_BUTTONS;

    treeStyle |= wxTR_HIDE_ROOT;

#ifdef __WXGTK20__
    treeStyle |= wxTR_NO_LINES;
#endif

    if (style & wxDIRCTRL_EDIT_LABELS)
        treeStyle |= wxTR_EDIT_LABELS;

    if (style & wxDIRCTRL_MULTIPLE)
        treeStyle |= wxTR_MULTIPLE;

    if ((style & wxDIRCTRL_3D_INTERNAL) == 0)
        treeStyle |= wxNO_BORDER;

    m_treeCtrl = CreateTreeCtrl(this, wxID_TREECTRL,
                                wxPoint(0,0), GetClientSize(), treeStyle);

    if (!filter.empty() && (style & wxDIRCTRL_SHOW_FILTERS))
        m_filterListCtrl = new wxDirFilterListCtrl(this, wxID_FILTERLISTCTRL);

    m_defaultPath = dir;
    m_filter = filter;

    if (m_filter.empty())
        m_filter = wxFileSelectorDefaultWildcardStr;

    SetFilterIndex(defaultFilter);

    if (m_filterListCtrl)
        m_filterListCtrl->FillFilterList(filter, defaultFilter);

    // TODO: set the icon size according to current scaling for this window.
    // Currently, there's insufficient API in wxWidgets to determine what icons
    // are available and whether to take the nearest size according to a tolerance
    // instead of scaling.
    // if (!wxTheFileIconsTable->IsOk())
    //     wxTheFileIconsTable->SetSize(scaledSize);

    // Meanwhile, in your application initialisation, where you have better knowledge of what
    // icons are available and whether to scale, you can do this:
    //
    // wxTheFileIconsTable->SetSize(calculatedIconSizeForDPI);
    //
    // Obviously this can't take into account monitors with different DPI.
    m_treeCtrl->SetImageList(wxTheFileIconsTable->GetSmallImageList());

    m_showHidden = false;
    wxDirItemData* rootData = new wxDirItemData("", "", true);

#if defined(WX_WINDOWS)
    std::string rootName = _("Computer");
#else
    std::string rootName = _("Sections");
#endif

    m_rootId = m_treeCtrl->AddRoot( rootName, 3, -1, rootData);
    m_treeCtrl->SetItemHasChildren(m_rootId);

    ExpandRoot();

    SetInitialSize(size);
    DoResize();

    return true;
}

wxTreeCtrl* wxGenericDirCtrl::CreateTreeCtrl(wxWindow *parent, wxWindowID treeid, const wxPoint& pos, const wxSize& size, unsigned int treeStyle)
{
    return new wxTreeCtrl(parent, treeid, pos, size, treeStyle);
}

void wxGenericDirCtrl::ShowHidden( bool show )
{
    if ( m_showHidden == show )
        return;

    m_showHidden = show;

    if ( HasFlag(wxDIRCTRL_MULTIPLE) )
    {
        const std::vector<std::string> paths = GetPaths();
        ReCreateTree();
        for ( const auto& path : paths )
        {
            ExpandPath(path);
        }
    }
    else
    {
        const std::string path = GetPath();
        ReCreateTree();
        SetPath(path);
    }
}

const wxTreeItemId
wxGenericDirCtrl::AddSection(const std::string& path, const std::string& name, int imageId)
{
    wxDirItemData *dir_item = new wxDirItemData(path,name,true);

    wxTreeItemId treeid = AppendItem( m_rootId, name, imageId, -1, dir_item);

    m_treeCtrl->SetItemHasChildren(treeid);

    return treeid;
}

void wxGenericDirCtrl::SetupSections()
{
    std::vector<std::string> paths, names;
    std::vector<int> icons;

    size_t count = wxGetAvailableDrives(paths, names, icons);

#ifdef __WXGTK20__
    std::string home = wxGetHomeDir();
    AddSection( home, _("Home directory"), 1);
    home += "/Desktop";
    AddSection( home, _("Desktop"), 1);
#endif

    for (size_t n = 0; n < count; n++)
        AddSection(paths[n], names[n], icons[n]);
}

void wxGenericDirCtrl::SetFocus()
{
    // we don't need focus ourselves, give it to the tree so that the user
    // could navigate it
    if (m_treeCtrl)
        m_treeCtrl->SetFocus();
}

void wxGenericDirCtrl::OnBeginEditItem(wxTreeEvent &event)
{
    // don't rename the main entry "Sections"
    if (event.GetItem() == m_rootId)
    {
        event.Veto();
        return;
    }

    // don't rename the individual sections
    if (m_treeCtrl->GetItemParent( event.GetItem() ) == m_rootId)
    {
        event.Veto();
        return;
    }
}

void wxGenericDirCtrl::OnEndEditItem(wxTreeEvent &event)
{
    if (event.IsEditCancelled())
        return;

    if ((event.GetLabel().empty()) ||
        (event.GetLabel() == ".") ||
        (event.GetLabel() == "..") ||
        (event.GetLabel().find('/') != std::string::npos) ||
        (event.GetLabel().find('\\') != std::string::npos) ||
        (event.GetLabel().find('|') != std::string::npos))
    {
        wxMessageDialog dialog(this, _("Illegal directory name."), _("Error"), wxOK | wxICON_ERROR );
        dialog.ShowModal();
        event.Veto();
        return;
    }

    wxTreeItemId treeid = event.GetItem();
    wxDirItemData *data = GetItemData( treeid );
    wxASSERT( data );

    std::string new_name( wxPathOnly( data->m_path ) );
    new_name += std::string{wxFILE_SEP_PATH};
    new_name += event.GetLabel();

    wxLogNull log;

    if (wxFileExists(new_name))
    {
        wxMessageDialog dialog(this, _("File name exists already."), _("Error"), wxOK | wxICON_ERROR );
        dialog.ShowModal();
        event.Veto();
    }

    if (wxRenameFile(data->m_path,new_name))
    {
        data->SetNewDirName( new_name );
    }
    else
    {
        wxMessageDialog dialog(this, _("Operation not permitted."), _("Error"), wxOK | wxICON_ERROR );
        dialog.ShowModal();
        event.Veto();
    }
}

void wxGenericDirCtrl::OnTreeSelChange(wxTreeEvent &event)
{
    wxTreeEvent changedEvent(wxEVT_DIRCTRL_SELECTIONCHANGED, GetId());

    changedEvent.SetEventObject(this);
    changedEvent.SetItem(event.GetItem());
    changedEvent.SetClientObject(m_treeCtrl->GetItemData(event.GetItem()));

    if (GetEventHandler()->SafelyProcessEvent(changedEvent) && !changedEvent.IsAllowed())
        event.Veto();
    else
        event.Skip();
}

void wxGenericDirCtrl::OnItemActivated(wxTreeEvent &event)
{
    wxTreeItemId treeid = event.GetItem();
    const wxDirItemData *data = GetItemData(treeid);

    if (data->m_isDir)
    {
        // is dir
        event.Skip();
    }
    else
    {
        // is file
        wxTreeEvent changedEvent(wxEVT_DIRCTRL_FILEACTIVATED, GetId());

        changedEvent.SetEventObject(this);
        changedEvent.SetItem(treeid);
        changedEvent.SetClientObject(m_treeCtrl->GetItemData(treeid));

        if (GetEventHandler()->SafelyProcessEvent(changedEvent) && !changedEvent.IsAllowed())
            event.Veto();
        else
            event.Skip();
    }
}

void wxGenericDirCtrl::OnExpandItem(wxTreeEvent &event)
{
    wxTreeItemId parentId = event.GetItem();

    // VS: this is needed because the event handler is called from wxTreeCtrl
    //     ctor when wxTR_HIDE_ROOT was specified

    if (!m_rootId.IsOk())
        m_rootId = m_treeCtrl->GetRootItem();

    ExpandDir(parentId);
}

void wxGenericDirCtrl::OnCollapseItem(wxTreeEvent &event )
{
    CollapseDir(event.GetItem());
}

void wxGenericDirCtrl::CollapseDir(wxTreeItemId parentId)
{
    wxTreeItemId child;

    wxDirItemData *data = GetItemData(parentId);
    if (!data->m_isExpanded)
        return;

    data->m_isExpanded = false;

    m_treeCtrl->Freeze();
    if (parentId != m_treeCtrl->GetRootItem())
        m_treeCtrl->CollapseAndReset(parentId);
    m_treeCtrl->DeleteChildren(parentId);
    m_treeCtrl->Thaw();
}

void wxGenericDirCtrl::PopulateNode(wxTreeItemId parentId)
{
    wxDirItemData *data = GetItemData(parentId);

    if (data->m_isExpanded)
        return;

    data->m_isExpanded = true;

    if (parentId == m_treeCtrl->GetRootItem())
    {
        SetupSections();
        return;
    }

    wxASSERT(data);

    std::string path;

    std::string dirName = data->m_path;

#if defined(WX_WINDOWS)
    // Check if this is a root directory and if so,
    // whether the drive is available.
    if (!wxIsDriveAvailable(dirName))
    {
        data->m_isExpanded = false;
        //wxMessageBox("Sorry, this drive is not available.");
        return;
    }
#endif

    // This may take a longish time. Go to busy cursor
    wxBusyCursor busy;

#if defined(WX_WINDOWS)
    if (dirName.back() == ':')
        dirName += fmt::format("{}", wxFILE_SEP_PATH);
#endif

    std::vector<std::string> dirs;
    std::vector<std::string> filenames;

    wxDir d;
    std::string eachFilename;

    wxLogNull log;
    d.Open(dirName);

    if (d.IsOpened())
    {
        int style = wxDIR_DIRS;
        if (m_showHidden) style |= wxDIR_HIDDEN;
        if (d.GetFirst(&eachFilename, {}, style))
        {
            do
            {
                if ((eachFilename != ".") && (eachFilename != ".."))
                {
                    dirs.push_back(eachFilename);
                }
            }
            while (d.GetNext(&eachFilename));
        }
    }
    
    std::ranges::sort(dirs, wxDirCtrlStringCompareFunction);

    // Now do the filenames -- but only if we're allowed to
    if (!HasFlag(wxDIRCTRL_DIR_ONLY))
    {
        d.Open(dirName);

        if (d.IsOpened())
        {
            int style = wxDIR_FILES;
            if (m_showHidden) style |= wxDIR_HIDDEN;
            // Process each filter (ex: "JPEG Files (*.jpg;*.jpeg)|*.jpg;*.jpeg")
            wxStringTokenizer strTok;
            std::string curFilter;
            strTok.SetString(m_currentFilterStr,";");
            while(strTok.HasMoreTokens())
            {
                curFilter = strTok.GetNextToken();
                if (d.GetFirst(& eachFilename, curFilter, style))
                {
                    do
                    {
                        if ((eachFilename != ".") && (eachFilename != ".."))
                        {
                            filenames.push_back(eachFilename);
                        }
                    }
                    while (d.GetNext(& eachFilename));
                }
            }
        }

        std::ranges::sort(filenames, wxDirCtrlStringCompareFunction);
    }

    // Now we really know whether we have any children so tell the tree control
    // about it.
    m_treeCtrl->SetItemHasChildren(parentId, !dirs.empty() || !filenames.empty());

    // Add the sorted dirs
    for (size_t i = 0; i < dirs.size(); i++)
    {
        eachFilename = dirs[i];
        path = dirName;
        if (!wxEndsWithPathSeparator(path))
            path += fmt::format("{}", wxFILE_SEP_PATH);
        path += eachFilename;

        wxDirItemData *dir_item = new wxDirItemData(path,eachFilename,true);
        wxTreeItemId treeid = AppendItem( parentId, eachFilename,
                                      wxFileIconsTable::folder, -1, dir_item);
        m_treeCtrl->SetItemImage( treeid, wxFileIconsTable::folder_open,
                                  wxTreeItemIcon_Expanded );

        // assume that it does have children by default as it can take a long
        // time to really check for this (think remote drives...)
        //
        // and if we're wrong, we'll correct the icon later if
        // the user really tries to open this item
        m_treeCtrl->SetItemHasChildren(treeid);
    }

    // Add the sorted filenames
    if (!HasFlag(wxDIRCTRL_DIR_ONLY))
    {
        for (size_t i = 0; i < filenames.size(); i++)
        {
            eachFilename = filenames[i];
            path = dirName;
            if (!wxEndsWithPathSeparator(path))
                path += fmt::format("{}", wxFILE_SEP_PATH);
            path += eachFilename;
            //path = dirName + std::string("/") + eachFilename;
            wxDirItemData *dir_item = new wxDirItemData(path,eachFilename,false);
            int image_id = wxFileIconsTable::file;
            if (eachFilename.find('.') != std::string::npos)
                image_id = wxTheFileIconsTable->GetIconID(wx::utils::AfterLast(eachFilename, '.'));
            (void) AppendItem( parentId, eachFilename, image_id, -1, dir_item);
        }
    }
}

void wxGenericDirCtrl::ExpandDir(wxTreeItemId parentId)
{
    // ExpandDir() will not actually expand the tree node, just populate it
    PopulateNode(parentId);
}

void wxGenericDirCtrl::ReCreateTree()
{
    CollapseDir(m_treeCtrl->GetRootItem());
    ExpandRoot();
}

void wxGenericDirCtrl::CollapseTree()
{
    wxTreeItemIdValue cookie;
    wxTreeItemId child = m_treeCtrl->wxGetFirstChild(m_rootId, cookie);
    while (child.IsOk())
    {
        CollapseDir(child);
        child = m_treeCtrl->GetNextChild(m_rootId, cookie);
    }
}

// Find the child that matches the first part of 'path'.
// E.g. if a child path is "/usr" and 'path' is "/usr/include"
// then the child for /usr is returned.
wxTreeItemId wxGenericDirCtrl::FindChild(wxTreeItemId parentId, const std::string& path, bool& done)
{
    std::string path2(path);

    // Make sure all separators are as per the current platform
    wx::utils::ReplaceAll(path2, "\\", std::string{wxFILE_SEP_PATH});
    wx::utils::ReplaceAll(path2, "/", std::string{wxFILE_SEP_PATH});

    // Append a separator to foil bogus substring matching
    path2 += fmt::format("{}", wxFILE_SEP_PATH);

    // In MSW case is not significant
#if defined(WX_WINDOWS)
    wx::utils::ToLower(path2);
#endif

    wxTreeItemIdValue cookie;
    wxTreeItemId childId = m_treeCtrl->wxGetFirstChild(parentId, cookie);
    while (childId.IsOk())
    {
        wxDirItemData* data = GetItemData(childId);

        if (data && !data->m_path.empty())
        {
            std::string childPath(data->m_path);
            if (!wxEndsWithPathSeparator(childPath))
                childPath += fmt::format("{}", wxFILE_SEP_PATH);

            // In MSW case is not significant
#if defined(WX_WINDOWS)
            wx::utils::ToLower(childPath);
#endif

            if (childPath.length() <= path2.length())
            {
                std::string path3 = path2.substr(0, childPath.length());
                if (childPath == path3)
                {
                    done = path3.length() == path2.length();
                    return childId;
                }
            }
        }

        childId = m_treeCtrl->GetNextChild(parentId, cookie);
    }
    wxTreeItemId invalid;
    return invalid;
}

// Try to expand as much of the given path as possible,
// and select the given tree item.
bool wxGenericDirCtrl::ExpandPath(const std::string& path)
{
    bool done = false;
    wxTreeItemId treeid = FindChild(m_rootId, path, done);
    wxTreeItemId lastId = treeid; // The last non-zero treeid
    while (treeid.IsOk() && !done)
    {
        ExpandDir(treeid);

        treeid = FindChild(treeid, path, done);
        if (treeid.IsOk())
            lastId = treeid;
    }
    if (!lastId.IsOk())
        return false;

    wxDirItemData *data = GetItemData(lastId);
    if (data->m_isDir)
    {
        m_treeCtrl->Expand(lastId);
    }
    if (HasFlag(wxDIRCTRL_SELECT_FIRST) && data->m_isDir)
    {
        // Find the first file in this directory
        wxTreeItemIdValue cookie;
        wxTreeItemId childId = m_treeCtrl->wxGetFirstChild(lastId, cookie);
        bool selectedChild = false;
        while (childId.IsOk())
        {
            data = GetItemData(childId);

            if (data && !data->m_path.empty() && !data->m_isDir)
            {
                m_treeCtrl->SelectItem(childId);
                m_treeCtrl->EnsureVisible(childId);
                selectedChild = true;
                break;
            }
            childId = m_treeCtrl->GetNextChild(lastId, cookie);
        }
        if (!selectedChild)
        {
            m_treeCtrl->SelectItem(lastId);
            m_treeCtrl->EnsureVisible(lastId);
        }
    }
    else
    {
        m_treeCtrl->SelectItem(lastId);
        m_treeCtrl->EnsureVisible(lastId);
    }

    return true;
}


bool wxGenericDirCtrl::CollapsePath(const std::string& path)
{
    bool done           = false;
    wxTreeItemId treeid     = FindChild(m_rootId, path, done);
    wxTreeItemId lastId = treeid; // The last non-zero treeid

    while ( treeid.IsOk() && !done )
    {
        CollapseDir(treeid);

        treeid = FindChild(treeid, path, done);

        if ( treeid.IsOk() )
            lastId = treeid;
    }

    if ( !lastId.IsOk() )
        return false;

    m_treeCtrl->SelectItem(lastId);
    m_treeCtrl->EnsureVisible(lastId);

    return true;
}

wxDirItemData* wxGenericDirCtrl::GetItemData(wxTreeItemId itemId)
{
    return dynamic_cast<wxDirItemData*>(m_treeCtrl->GetItemData(itemId));
}

std::string wxGenericDirCtrl::GetPath(wxTreeItemId itemId) const
{
    const auto* data = dynamic_cast<wxDirItemData*>(m_treeCtrl->GetItemData(itemId));

    return data ? data->m_path : std::string();
}

std::string wxGenericDirCtrl::GetPath() const
{
    // Allow calling GetPath() in multiple selection from OnSelFilter
    if (m_treeCtrl->HasFlag(wxTR_MULTIPLE))
    {
        wxArrayTreeItemIds items;
        m_treeCtrl->GetSelections(items);
        if (items.size() > 0)
        {
            // return first string only
            wxTreeItemId treeid = items[0];
            return GetPath(treeid);
        }

        return {};
    }

    wxTreeItemId treeid = m_treeCtrl->GetSelection();
    if (treeid)
    {
        return GetPath(treeid);
    }
    else
        return {};
}

std::vector<std::string> wxGenericDirCtrl::GetPaths() const
{
    std::vector<std::string> paths;

    wxArrayTreeItemIds items;
    m_treeCtrl->GetSelections(items);
    for ( unsigned n = 0; n < items.size(); n++ )
    {
        wxTreeItemId treeid = items[n];
        paths.push_back(GetPath(treeid));
    }

    return paths;
}

std::string wxGenericDirCtrl::GetFilePath() const
{
    wxTreeItemId treeid = m_treeCtrl->GetSelection();
    if (treeid)
    {
        wxDirItemData* data = (wxDirItemData*) m_treeCtrl->GetItemData(treeid);
        if (data->m_isDir)
            return {};
        else
            return data->m_path;
    }
    else
        return {};
}

void wxGenericDirCtrl::GetFilePaths(std::vector<std::string>& paths) const
{
    paths.clear();

    wxArrayTreeItemIds items;
    m_treeCtrl->GetSelections(items);
    for ( unsigned n = 0; n < items.size(); n++ )
    {
        wxTreeItemId treeid = items[n];
        wxDirItemData* data = (wxDirItemData*) m_treeCtrl->GetItemData(treeid);
        if ( !data->m_isDir )
            paths.push_back(data->m_path);
    }
}

void wxGenericDirCtrl::SetPath(const std::string& path)
{
    m_defaultPath = path;
    if (m_rootId)
        ExpandPath(path);
}

void wxGenericDirCtrl::SelectPath(const std::string& path, bool select)
{
    bool done = false;
    wxTreeItemId treeid = FindChild(m_rootId, path, done);
    wxTreeItemId lastId = treeid; // The last non-zero treeid
    while ( treeid.IsOk() && !done )
    {
        treeid = FindChild(treeid, path, done);
        if ( treeid.IsOk() )
            lastId = treeid;
    }
    if ( !lastId.IsOk() )
        return;

    if ( done )
    {
        m_treeCtrl->SelectItem(treeid, select);
    }
}

void wxGenericDirCtrl::SelectPaths(const std::vector<std::string>& paths)
{
    if ( HasFlag(wxDIRCTRL_MULTIPLE) )
    {
        UnselectAll();
        for ( unsigned n = 0; n < paths.size(); n++ )
        {
            SelectPath(paths[n]);
        }
    }
}

void wxGenericDirCtrl::UnselectAll()
{
    m_treeCtrl->UnselectAll();
}

// Not used
#if 0
void wxGenericDirCtrl::FindChildFiles(wxTreeItemId treeid, int dirFlags, std::vector<std::string>& filenames)
{
    wxDirItemData *data = (wxDirItemData *) m_treeCtrl->GetItemData(treeid);

    // This may take a longish time. Go to busy cursor
    wxBusyCursor busy;

    wxASSERT(data);

    std::string search,path,filename;

    std::string dirName(data->m_path);

#if defined(WX_WINDOWS)
    if (dirName.Last() == ':')
        dirName += std::string(wxFILE_SEP_PATH);
#endif

    wxDir d;
    std::string eachFilename;

    wxLogNull log;
    d.Open(dirName);

    if (d.IsOpened())
    {
        if (d.GetFirst(& eachFilename, m_currentFilterStr, dirFlags))
        {
            do
            {
                if ((eachFilename != ".")) && (eachFilename != wxT(".."))
                {
                    filenames.Add(eachFilename);
                }
            }
            while (d.GetNext(& eachFilename)) ;
        }
    }
}
#endif

void wxGenericDirCtrl::SetFilterIndex(int n)
{
    m_currentFilter = n;

    std::string f, d;
    if (ExtractWildcard(m_filter, n, f, d))
        m_currentFilterStr = f;
    else
#ifdef __UNIX__
        m_currentFilterStr = "*";
#else
        m_currentFilterStr = "*.*";
#endif
}

void wxGenericDirCtrl::SetFilter(const std::string& filter)
{
    m_filter = filter;

    if (!filter.empty() && !m_filterListCtrl && HasFlag(wxDIRCTRL_SHOW_FILTERS))
        m_filterListCtrl = new wxDirFilterListCtrl(this, wxID_FILTERLISTCTRL);
    else if (filter.empty() && m_filterListCtrl)
    {
        m_filterListCtrl->Destroy();
        m_filterListCtrl = nullptr;
    }

    std::string f, d;
    if (ExtractWildcard(m_filter, m_currentFilter, f, d))
        m_currentFilterStr = f;
    else
#ifdef __UNIX__
        m_currentFilterStr = "*";
#else
        m_currentFilterStr = "*.*";
#endif
    // current filter index is meaningless after filter change, set it to zero
    SetFilterIndex(0);
    if (m_filterListCtrl)
        m_filterListCtrl->FillFilterList(m_filter, 0);
}

// Extract description and actual filter from overall filter string
bool wxGenericDirCtrl::ExtractWildcard(const std::string& filterStr, int n, std::string& filter, std::string& description)
{
    std::vector<std::string> filters, descriptions;
    int count = wxParseCommonDialogsFilter(filterStr, descriptions, filters);
    if (count > 0 && n < count)
    {
        filter = filters[n];
        description = descriptions[n];
        return true;
    }

    return false;
}


void wxGenericDirCtrl::DoResize()
{
    wxSize sz = GetClientSize();
    int verticalSpacing = 3;
    if (m_treeCtrl)
    {
        wxSize filterSz ;
        if (m_filterListCtrl)
        {
            filterSz = m_filterListCtrl->GetBestSize();
            sz.y -= (filterSz.y + verticalSpacing);
        }
        m_treeCtrl->SetSize(wxRect{wxPoint{0, 0}, sz});
        if (m_filterListCtrl)
        {
            m_filterListCtrl->SetSize(wxRect{0, sz.y + verticalSpacing, sz.x, filterSz.y});
            // Don't know why, but this needs refreshing after a resize (wxMSW)
            m_filterListCtrl->Refresh();
        }
    }
}


void wxGenericDirCtrl::OnSize([[maybe_unused]] wxSizeEvent& event)
{
    DoResize();
}

wxTreeItemId wxGenericDirCtrl::AppendItem (const wxTreeItemId & parent,
                                           const std::string & text,
                                           int image, int selectedImage,
                                           wxTreeItemData * data)
{
  wxTreeCtrl *treeCtrl = GetTreeCtrl ();

  wxASSERT (treeCtrl);

  if (treeCtrl)
  {
    return treeCtrl->AppendItem (parent, text, image, selectedImage, data);
  }
  else
  {
    return {};
  }
}


//-----------------------------------------------------------------------------
// wxDirFilterListCtrl
//-----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(wxDirFilterListCtrl, wxChoice)
    EVT_CHOICE(wxID_ANY, wxDirFilterListCtrl::OnSelFilter)
wxEND_EVENT_TABLE()

bool wxDirFilterListCtrl::Create(wxGenericDirCtrl* parent,
                                 wxWindowID treeid,
                                 const wxPoint& pos,
                                 const wxSize& size,
                                 unsigned int style)
{
    m_dirCtrl = parent;
    return wxChoice::Create(parent, treeid, pos, size, {}, style);
}

void wxDirFilterListCtrl::OnSelFilter([[maybe_unused]] wxCommandEvent& event)
{
    int sel = GetSelection();

    if (m_dirCtrl->HasFlag(wxDIRCTRL_MULTIPLE))
    {
        const std::vector<std::string> paths = m_dirCtrl->GetPaths();

        m_dirCtrl->SetFilterIndex(sel);

        // If the filter has changed, the view is out of date, so
        // collapse the tree.
        m_dirCtrl->ReCreateTree();

        // Expand and select the previously selected paths
        for (const auto& path : paths)
        {
            m_dirCtrl->ExpandPath(path);
        }
    }
    else
    {
        std::string currentPath = m_dirCtrl->GetPath();

        m_dirCtrl->SetFilterIndex(sel);
        m_dirCtrl->ReCreateTree();

        // Try to restore the selection, or at least the directory
        m_dirCtrl->ExpandPath(currentPath);
    }
}

void wxDirFilterListCtrl::FillFilterList(const std::string& filter, int defaultFilter)
{
    Clear();
    std::vector<std::string> descriptions, filters;
    size_t n = (size_t) wxParseCommonDialogsFilter(filter, descriptions, filters);

    if (n > 0 && defaultFilter < (int) n)
    {
        for (size_t i = 0; i < n; i++)
            Append(descriptions[i]);
        SetSelection(defaultFilter);
    }
}
#endif // wxUSE_DIRDLG

#if wxUSE_DIRDLG || wxUSE_FILEDLG

// ----------------------------------------------------------------------------
// wxFileIconsTable icons
// ----------------------------------------------------------------------------

#if 0
#ifndef __WXGTK20__
/* Computer (c) Julian Smart */
static const char* const file_icons_tbl_computer_xpm[] = {
/* columns rows colors chars-per-pixel */
"16 16 42 1",
"r c #4E7FD0",
"$ c #7198D9",
"; c #DCE6F6",
"q c #FFFFFF",
"u c #4A7CCE",
"# c #779DDB",
"w c #95B2E3",
"y c #7FA2DD",
"f c #3263B4",
"= c #EAF0FA",
"< c #B1C7EB",
"% c #6992D7",
"9 c #D9E4F5",
"o c #9BB7E5",
"6 c #F7F9FD",
", c #BED0EE",
"3 c #F0F5FC",
"1 c #A8C0E8",
"  c None",
"0 c #FDFEFF",
"4 c #C4D5F0",
"@ c #81A4DD",
"e c #4377CD",
"- c #E2EAF8",
"i c #9FB9E5",
"> c #CCDAF2",
"+ c #89A9DF",
"s c #5584D1",
"t c #5D89D3",
": c #D2DFF4",
"5 c #FAFCFE",
"2 c #F5F8FD",
"8 c #DFE8F7",
"& c #5E8AD4",
"X c #638ED5",
"a c #CEDCF2",
"p c #90AFE2",
"d c #2F5DA9",
"* c #5282D0",
"7 c #E5EDF9",
". c #A2BCE6",
"O c #8CACE0",
/* pixels */
"                ",
"  .XXXXXXXXXXX  ",
"  oXO++@#$%&*X  ",
"  oX=-;:>,<1%X  ",
"  oX23=-;:4,$X  ",
"  oX5633789:@X  ",
"  oX05623=78+X  ",
"  oXqq05623=OX  ",
"  oX,,,,,<<<$X  ",
"  wXXXXXXXXXXe  ",
"  XrtX%$$y@+O,, ",
"  uyiiiiiiiii@< ",
" ouiiiiiiiiiip<a",
" rustX%$$y@+Ow,,",
" dfffffffffffffd",
"                "
};
#endif // !GTK+ 2
#endif

// ----------------------------------------------------------------------------
// wxFileIconsTable & friends
// ----------------------------------------------------------------------------

// global instance of a wxFileIconsTable
wxFileIconsTable* wxTheFileIconsTable = nullptr;

// A module to allow icons table cleanup

class wxFileIconsTableModule: public wxModule
{
    wxDECLARE_DYNAMIC_CLASS(wxFileIconsTableModule);
public:
    bool OnInit() override { wxTheFileIconsTable = new wxFileIconsTable; return true; }
    void OnExit() override
    {
        wxDELETE(wxTheFileIconsTable);
    }
};

wxIMPLEMENT_DYNAMIC_CLASS(wxFileIconsTableModule, wxModule);

class wxFileIconEntry : public wxObject
{
public:
    explicit wxFileIconEntry(int i)
        : iconid(i)
    {}

    int iconid;
};

wxFileIconsTable::wxFileIconsTable()
    : m_size(16, 16)
{
}

wxFileIconsTable::~wxFileIconsTable()
{
    if (m_HashTable)
    {
        WX_CLEAR_HASH_TABLE(*m_HashTable);
        delete m_HashTable;
    }
    delete m_smallImageList;
}

// delayed initialization - wait until first use (wxArtProv not created yet)
void wxFileIconsTable::Create(const wxSize& sz)
{
    wxCHECK_RET(!m_smallImageList && !m_HashTable, "creating icons twice");
    m_HashTable = new wxHashTable(wxKEY_STRING);
    m_smallImageList = new wxImageList(sz.x, sz.y);

    // folder:
    m_smallImageList->Add(wxArtProvider::GetBitmap(wxART_FOLDER,
                                                   wxART_CMN_DIALOG,
                                                   sz));
    // folder_open
    m_smallImageList->Add(wxArtProvider::GetBitmap(wxART_FOLDER_OPEN,
                                                   wxART_CMN_DIALOG,
                                                   sz));
    // computer
#ifdef __WXGTK20__
    // GTK24 uses this icon in the file open dialog
    m_smallImageList->Add(wxArtProvider::GetBitmap(wxART_HARDDISK,
                                                   wxART_CMN_DIALOG,
                                                   sz));
#else
    m_smallImageList->Add(wxArtProvider::GetBitmap(wxART_HARDDISK,
                                                   wxART_CMN_DIALOG,
                                                   sz));
    // TODO: add computer icon if really necessary
    //m_smallImageList->Add(wxIcon(file_icons_tbl_computer_xpm));
#endif
    // drive
    m_smallImageList->Add(wxArtProvider::GetBitmap(wxART_HARDDISK,
                                                   wxART_CMN_DIALOG,
                                                   sz));
    // cdrom
    m_smallImageList->Add(wxArtProvider::GetBitmap(wxART_CDROM,
                                                   wxART_CMN_DIALOG,
                                                   sz));
    // floppy
    m_smallImageList->Add(wxArtProvider::GetBitmap(wxART_FLOPPY,
                                                   wxART_CMN_DIALOG,
                                                   sz));
    // removeable
    m_smallImageList->Add(wxArtProvider::GetBitmap(wxART_REMOVABLE,
                                                   wxART_CMN_DIALOG,
                                                   sz));
    // file
    m_smallImageList->Add(wxArtProvider::GetBitmap(wxART_NORMAL_FILE,
                                                   wxART_CMN_DIALOG,
                                                   sz));
    // executable
    if (GetIconID({}, "application/x-executable") == file)
    {
        m_smallImageList->Add(wxArtProvider::GetBitmap(wxART_EXECUTABLE_FILE,
                                                       wxART_CMN_DIALOG,
                                                       sz));
        delete m_HashTable->Get("exe");
        m_HashTable->Delete("exe");
        m_HashTable->Put("exe", new wxFileIconEntry(executable));
    }
    /* else put into list by GetIconID
       (KDE defines application/x-executable for *.exe and has nice icon)
     */
}

wxImageList *wxFileIconsTable::GetSmallImageList()
{
    if (!m_smallImageList)
        Create(m_size);

    return m_smallImageList;
}

int wxFileIconsTable::GetIconID(const std::string& extension, const std::string& mime)
{
    if (!m_smallImageList)
        Create(m_size);

#if wxUSE_MIMETYPE
    if (!extension.empty())
    {
        wxFileIconEntry *entry = (wxFileIconEntry*) m_HashTable->Get(extension);
        if (entry) return (entry -> iconid);
    }

    wxFileType *ft = (mime.empty()) ?
                   wxTheMimeTypesManager -> GetFileTypeFromExtension(extension) :
                   wxTheMimeTypesManager -> GetFileTypeFromMimeType(mime);

    wxIconLocation iconLoc;
    wxIcon ic;

    {
        wxLogNull logNull;
        if ( ft && ft->GetIcon(&iconLoc) )
        {
            ic = wxIcon( iconLoc );
        }
    }

    delete ft;

    if ( !ic.IsOk() )
    {
        int newid = file;
        m_HashTable->Put(extension, new wxFileIconEntry(newid));
        return newid;
    }

    wxBitmap bmp;
    bmp.CopyFromIcon(ic);

    if ( !bmp.IsOk() )
    {
        int newid = file;
        m_HashTable->Put(extension, new wxFileIconEntry(newid));
        return newid;
    }

    int size = m_size.x;

    int treeid = m_smallImageList->GetImageCount();
    if ((bmp.GetWidth() == (int) size) && (bmp.GetHeight() == (int) size))
    {
        m_smallImageList->Add(bmp);
    }
#if wxUSE_IMAGE && (!defined(WX_WINDOWS) || wxUSE_WXDIB)
    else
    {
        wxImage img = bmp.ConvertToImage();

        if (img.HasMask())
            img.InitAlpha();

        wxBitmap bmp2;
        if ((img.GetWidth() != size) || (img.GetHeight() != size))
        {
            // TODO: replace with public API that gets the bitmap scale.
            // But this function may be called from code that doesn't pass a window,
            // so we will need to be able to get the scaling factor of the current
            // display, somehow. We could use wxTheApp->GetTopWindow() but sometimes
            // this won't be available.
#if defined(__WXOSX_COCOA__)
            if (wxOSXGetMainScreenContentScaleFactor() > 1.0)
            {
                img.Rescale(2*size, 2*size, wxImageResizeQuality::High);
                bmp2 = wxBitmap(img, -1, 2.0);
            }
            else
#endif
            {
                // Double, using normal quality scaling.
                img.Rescale(2*img.GetWidth(), 2*img.GetHeight());

                // Then scale to the desired size. This gives the best quality,
                // and better than CreateAntialiasedBitmap.
                if ((img.GetWidth() != size) || (img.GetHeight() != size))
                    img.Rescale(size, size, wxImageResizeQuality::High);

                bmp2 = wxBitmap(img);
            }
        }
        else
            bmp2 = wxBitmap(img);

        m_smallImageList->Add(bmp2);
    }
#endif // wxUSE_IMAGE

    m_HashTable->Put(extension, new wxFileIconEntry(treeid));
    return treeid;

#else // !wxUSE_MIMETYPE

    wxUnusedVar(mime);
    if (extension == "exe")
        return executable;
    else
        return file;
#endif // wxUSE_MIMETYPE/!wxUSE_MIMETYPE
}

#endif // wxUSE_DIRDLG || wxUSE_FILEDLG
