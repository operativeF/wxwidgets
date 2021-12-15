///////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/filectrlg.cpp
// Purpose:     wxGenericFileCtrl Implementation
// Author:      Diaa M. Sami
// Created:     2007-07-07
// Copyright:   (c) Diaa M. Sami
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_FILECTRL

#include "wx/generic/filectrlg.h"

#include "wx/sizer.h"
#include "wx/stattext.h"
#include "wx/checkbox.h"
#include "wx/msgdlg.h"
#include "wx/log.h"
#include "wx/filedlg.h"
#include "wx/clntdata.h"
#include "wx/file.h"        // for wxS_IXXX constants only
#include "wx/generic/dirctrlg.h" // for wxFileIconsTable
#include "wx/dir.h"
#include "wx/tokenzr.h"
#include "wx/imaglist.h"

#include <boost/nowide/convert.hpp>

import WX.Utils.Settings;

#if defined(WX_WINDOWS)
#define IsTopMostDir(dir)   (dir.empty())
#else
#define IsTopMostDir(dir)   (dir == "/")
#endif


// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

static
int wxCALLBACK wxFileDataNameCompare( wxIntPtr data1, wxIntPtr data2, wxIntPtr sortOrder)
{
     wxFileData *fd1 = (wxFileData *)wxUIntToPtr(data1);
     wxFileData *fd2 = (wxFileData *)wxUIntToPtr(data2);

     if (fd1->GetFileName() == "..")
         return -sortOrder;
     if (fd2->GetFileName() == "..")
         return sortOrder;
     if (fd1->IsDir() && !fd2->IsDir())
         return -sortOrder;
     if (fd2->IsDir() && !fd1->IsDir())
         return sortOrder;

     return sortOrder*wxStrcmp( fd1->GetFileName(), fd2->GetFileName() );
}

static
int wxCALLBACK wxFileDataSizeCompare(wxIntPtr data1, wxIntPtr data2, wxIntPtr sortOrder)
{
     wxFileData *fd1 = (wxFileData *)wxUIntToPtr(data1);
     wxFileData *fd2 = (wxFileData *)wxUIntToPtr(data2);

     if (fd1->GetFileName() == "..")
         return -sortOrder;
     if (fd2->GetFileName() == "..")
         return sortOrder;
     if (fd1->IsDir() && !fd2->IsDir())
         return -sortOrder;
     if (fd2->IsDir() && !fd1->IsDir())
         return sortOrder;
     if (fd1->IsLink() && !fd2->IsLink())
         return -sortOrder;
     if (fd2->IsLink() && !fd1->IsLink())
         return sortOrder;

     return fd1->GetSize() > fd2->GetSize() ? sortOrder : -sortOrder;
}

static
int wxCALLBACK wxFileDataTypeCompare(wxIntPtr data1, wxIntPtr data2, wxIntPtr sortOrder)
{
     wxFileData *fd1 = (wxFileData *)wxUIntToPtr(data1);
     wxFileData *fd2 = (wxFileData *)wxUIntToPtr(data2);

     if (fd1->GetFileName() == "..")
         return -sortOrder;
     if (fd2->GetFileName() == "..")
         return sortOrder;
     if (fd1->IsDir() && !fd2->IsDir())
         return -sortOrder;
     if (fd2->IsDir() && !fd1->IsDir())
         return sortOrder;
     if (fd1->IsLink() && !fd2->IsLink())
         return -sortOrder;
     if (fd2->IsLink() && !fd1->IsLink())
         return sortOrder;

     return sortOrder*wxStrcmp( fd1->GetFileType(), fd2->GetFileType() );
}

static
int wxCALLBACK wxFileDataTimeCompare(wxIntPtr data1, wxIntPtr data2, wxIntPtr sortOrder)
{
     wxFileData *fd1 = (wxFileData *)wxUIntToPtr(data1);
     wxFileData *fd2 = (wxFileData *)wxUIntToPtr(data2);

     if (fd1->GetFileName() == "..")
         return -sortOrder;
     if (fd2->GetFileName() == "..")
         return sortOrder;
     if (fd1->IsDir() && !fd2->IsDir())
         return -sortOrder;
     if (fd2->IsDir() && !fd1->IsDir())
         return sortOrder;

     return fd1->GetDateTime().IsLaterThan(fd2->GetDateTime()) ? sortOrder : -sortOrder;
}

// defined in src/generic/dirctrlg.cpp
extern size_t wxGetAvailableDrives(std::vector<std::string> &paths, std::vector<std::string> &names, std::vector<int> &icon_ids);

//-----------------------------------------------------------------------------
//  wxFileData
//-----------------------------------------------------------------------------

wxFileData::wxFileData( const std::string& filePath, const std::string& fileName, fileType type, int image_id )
    : m_fileName(fileName),
      m_filePath(filePath),
      m_type(type),
      m_image(image_id)
{
    ReadData();
}


// FIXME: Default this?
void wxFileData::Copy( const wxFileData& fileData )
{
    m_fileName = fileData.GetFileName();
    m_filePath = fileData.GetFilePath();
    m_size = fileData.GetSize();
    m_dateTime = fileData.GetDateTime();
    m_permissions = fileData.GetPermissions();
    m_type = fileData.GetType();
    m_image = fileData.GetImageId();
}

void wxFileData::ReadData()
{
    if (IsDrive())
    {
        m_size = 0;
        return;
    }

#if defined(WX_WINDOWS)
    // c:\.. is a drive don't stat it
    if ((m_fileName == "..") && (m_filePath.length() <= 5))
    {
        m_type = is_drive;
        m_size = 0;
        return;
    }
#endif // WX_WINDOWS

    // OTHER PLATFORMS

    wxStructStat buff;

#if defined(__UNIX__) && !defined(__VMS)
    const bool hasStat = lstat( m_filePath.fn_str(), &buff ) == 0;
    if ( hasStat )
        m_type |= S_ISLNK(buff.st_mode) ? is_link : 0;
#else // no lstat()
    const bool hasStat = wxStat( m_filePath, &buff ) == 0;
#endif

    if ( hasStat )
    {
        m_type |= (buff.st_mode & S_IFDIR) != 0 ? is_dir : 0;
        m_type |= (buff.st_mode & wxS_IXUSR) != 0 ? is_exe : 0;

        m_size = buff.st_size;

        m_dateTime = buff.st_mtime;
    }

#if defined(__UNIX__)
    if ( hasStat )
    {
        m_permissions.Printf("%c%c%c%c%c%c%c%c%c",
                             buff.st_mode & wxS_IRUSR ? wxT('r') : wxT('-'),
                             buff.st_mode & wxS_IWUSR ? wxT('w') : wxT('-'),
                             buff.st_mode & wxS_IXUSR ? wxT('x') : wxT('-'),
                             buff.st_mode & wxS_IRGRP ? wxT('r') : wxT('-'),
                             buff.st_mode & wxS_IWGRP ? wxT('w') : wxT('-'),
                             buff.st_mode & wxS_IXGRP ? wxT('x') : wxT('-'),
                             buff.st_mode & wxS_IROTH ? wxT('r') : wxT('-'),
                             buff.st_mode & wxS_IWOTH ? wxT('w') : wxT('-'),
                             buff.st_mode & wxS_IXOTH ? wxT('x') : wxT('-'));
    }
#elif defined(__WIN32__)
    DWORD attribs = ::GetFileAttributesW(boost::nowide::widen(m_filePath).c_str());
    if (attribs != (DWORD)-1)
    {
        m_permissions = fmt::format("%c%c%c%c",
                             attribs & FILE_ATTRIBUTE_ARCHIVE  ? 'A' : ' ',
                             attribs & FILE_ATTRIBUTE_READONLY ? 'R' : ' ',
                             attribs & FILE_ATTRIBUTE_HIDDEN   ? 'H' : ' ',
                             attribs & FILE_ATTRIBUTE_SYSTEM   ? 'S' : ' ');
    }
#endif

    // try to get a better icon
    if (m_image == wxFileIconsTable::file)
    {
        if (m_fileName.rfind('.') != std::string::npos)
        {
            m_image = wxTheFileIconsTable->GetIconID( wx::utils::AfterLast(m_fileName, '.'));
        } else if (IsExe())
        {
            m_image = wxFileIconsTable::executable;
        }
    }
}

std::string wxFileData::GetFileType() const
{
    if (IsDir())
        return _("<DIR>").ToStdString();
    else if (IsLink())
        return _("<LINK>").ToStdString();
    else if (IsDrive())
        return _("<DRIVE>").ToStdString();
    else if (m_fileName.rfind('.') != std::string::npos)
        return wx::utils::AfterLast(m_fileName, '.');

    return {};
}

std::string wxFileData::GetModificationTime() const
{
    // want time as 01:02 so they line up nicely, no %r in WIN32
    return m_dateTime.FormatDate() + " " + m_dateTime.Format(wxT("%I:%M:%S %p"));
}

std::string wxFileData::GetHint() const
{
    std::string s = m_filePath;
    s += "  ";

    if (IsDir())
        s += _("<DIR>");
    else if (IsLink())
        s += _("<LINK>");
    else if (IsDrive())
        s += _("<DRIVE>");
    else // plain file // FIXME: was wxPLURAL("%ld byte", "%ld bytes", m_size), but fmt lib doesn't work with it yet.
        s += fmt::format("%ld bytes", wxLongLong(m_size).ToString().c_str());

    s += ' ';

    if ( !IsDrive() )
    {
        s += fmt::format("{}  {}", GetModificationTime(), m_permissions);
    }

    return s;
}

std::string wxFileData::GetEntry( fileListFieldType num ) const
{
    std::string s;
    switch ( num )
    {
        case FileList_Name:
            s = m_fileName;
            break;

        case FileList_Size:
            if (!IsDir() && !IsLink() && !IsDrive())
                s = wxLongLong(m_size).ToString();
            break;

        case FileList_Type:
            s = GetFileType();
            break;

        case FileList_Time:
            if (!IsDrive())
                s = GetModificationTime();
            break;

#if defined(__UNIX__) || defined(__WIN32__)
        case FileList_Perm:
            s = m_permissions;
            break;
#endif // defined(__UNIX__) || defined(__WIN32__)

        default:
            wxFAIL_MSG( "unexpected field in wxFileData::GetEntry()" );
    }

    return s;
}

void wxFileData::SetNewName( const std::string &filePath, const std::string &fileName )
{
    m_fileName = fileName;
    m_filePath = filePath;
}

void wxFileData::MakeItem( wxListItem &item )
{
    item.m_text = boost::nowide::widen(m_fileName);
    item.ClearAttributes();
    if (IsExe())
        item.SetTextColour(*wxRED);
    if (IsDir())
        item.SetTextColour(*wxBLUE);

    item.m_image = m_image;

    if (IsLink())
    {
        wxColour dg = wxTheColourDatabase->Find( "MEDIUM GREY" );
        if ( dg.IsOk() )
            item.SetTextColour(dg);
    }
    item.m_data = wxPtrToUInt(this);
}

//-----------------------------------------------------------------------------
//  wxFileListCtrl
//-----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(wxFileListCtrl,wxListCtrl)
    EVT_LIST_DELETE_ITEM(wxID_ANY, wxFileListCtrl::OnListDeleteItem)
    EVT_LIST_DELETE_ALL_ITEMS(wxID_ANY, wxFileListCtrl::OnListDeleteAllItems)
    EVT_LIST_END_LABEL_EDIT(wxID_ANY, wxFileListCtrl::OnListEndLabelEdit)
    EVT_LIST_COL_CLICK(wxID_ANY, wxFileListCtrl::OnListColClick)
    EVT_SIZE (wxFileListCtrl::OnSize)
wxEND_EVENT_TABLE()

wxFileListCtrl::wxFileListCtrl(wxWindow *win,
                       wxWindowID id,
                       const std::string& wild,
                       bool showHidden,
                       const wxPoint& pos,
                       const wxSize& size,
                       unsigned int style,
                       const wxValidator &validator,
                       const std::string &name)
          : wxListCtrl(win, id, pos, size, style, validator, name),
            m_wild(wild),
            m_showHidden(showHidden),
            m_dirName("*")
{
    wxImageList *imageList = wxTheFileIconsTable->GetSmallImageList();

    SetImageList( imageList, wxIMAGE_LIST_SMALL );

    if (style & wxLC_REPORT)
        ChangeToReportMode();
}

void wxFileListCtrl::ChangeToListMode()
{
    ClearAll();
    SetSingleStyle( wxLC_LIST );
    UpdateFiles();
}

void wxFileListCtrl::ChangeToReportMode()
{
    ClearAll();
    SetSingleStyle( wxLC_REPORT );

    // do this since WIN32 does mm/dd/yy UNIX does mm/dd/yyyy
    // don't hardcode since mm/dd is dd/mm elsewhere
    int w, h;
    wxDateTime dt(22, wxDateTime::Dec, 2002, 22, 22, 22);
    std::string txt = dt.FormatDate() + "22" + dt.Format(wxT("%I:%M:%S %p"));
    GetTextExtent(txt, &w, &h);

    InsertColumn( 0, _("Name"), wxListColumnFormat::Left, w );
    InsertColumn( 1, _("Size"), wxListColumnFormat::Right, w/2 );
    InsertColumn( 2, _("Type"), wxListColumnFormat::Left, w/2 );
    InsertColumn( 3, _("Modified"), wxListColumnFormat::Left, w );
#if defined(__UNIX__)
    GetTextExtent("Permissions 2", &w, &h);
    InsertColumn( 4, _("Permissions"), wxListColumnFormat::Left, w );
#elif defined(__WIN32__)
    GetTextExtent("Attributes 2", &w, &h);
    InsertColumn( 4, _("Attributes"), wxListColumnFormat::Left, w );
#endif

    UpdateFiles();
}

void wxFileListCtrl::ChangeToSmallIconMode()
{
    ClearAll();
    SetSingleStyle( wxLC_SMALL_ICON );
    UpdateFiles();
}

void wxFileListCtrl::ShowHidden( bool show )
{
    m_showHidden = show;
    UpdateFiles();
}

long wxFileListCtrl::Add( wxFileData *fd, wxListItem &item )
{
    long ret = -1;
    item.m_mask.set(ListMasks::Text, ListMasks::Data, ListMasks::Image);
    fd->MakeItem( item );
    unsigned int my_style = GetWindowStyleFlag();
    if (my_style & wxLC_REPORT)
    {
        ret = InsertItem( item );
        for (int i = 1; i < wxFileData::FileList_Max; i++)
            SetItem( item.m_itemId, i, fd->GetEntry((wxFileData::fileListFieldType)i) );
    }
    else if ((my_style & wxLC_LIST) || (my_style & wxLC_SMALL_ICON))
    {
        ret = InsertItem( item );
    }
    return ret;
}

void wxFileListCtrl::UpdateItem(const wxListItem &item)
{
    wxFileData *fd = (wxFileData*)GetItemData(item);
    wxCHECK_RET(fd, "invalid filedata");

    fd->ReadData();

    SetItemText(item, fd->GetFileName());
    SetItemImage(item, fd->GetImageId());

    if (GetWindowStyleFlag() & wxLC_REPORT)
    {
        for (int i = 1; i < wxFileData::FileList_Max; i++)
            SetItem( item.m_itemId, i, fd->GetEntry((wxFileData::fileListFieldType)i) );
    }
}

void wxFileListCtrl::UpdateFiles()
{
    // don't do anything before ShowModal() call which sets m_dirName
    if ( m_dirName == "*" )
        return;

    wxBusyCursor bcur; // this may take a while...

    DeleteAllItems();

    wxListItem item;
    item.m_itemId = 0;
    item.m_col = 0;

#if defined(WX_WINDOWS) || defined(__WXMAC__)
    if ( IsTopMostDir(m_dirName) )
    {
        std::vector<std::string> names, paths;
        std::vector<int> icons;
        const size_t count = wxGetAvailableDrives(paths, names, icons);

        for ( size_t n = 0; n < count; n++ )
        {
            // use paths[n] as the drive name too as our HandleAction() can't
            // deal with the drive names (of the form "System (C:)") currently
            // as it mistakenly treats them as file names
            //
            // it would be preferable to show names, and not paths, in the
            // dialog just as the native dialog does but for this we must:
            //  a) store the item type as item data and modify HandleAction()
            //     to use it instead of wxDirExists() to check whether the item
            //     is a directory
            //  b) store the drives by their drive letters and not their
            //     descriptions as otherwise it's pretty confusing to the user
            wxFileData *fd = new wxFileData(paths[n], paths[n],
                                            wxFileData::is_drive, icons[n]);
            if (Add(fd, item) != -1)
                item.m_itemId++;
            else
                delete fd;
        }
    }
    else
#endif // defined(WX_WINDOWS) || defined(__WXMAC__)
    {
        // Real directory...
        if ( !m_dirName.empty() )
        {
            std::string p = wxPathOnly(m_dirName);
#if defined(__UNIX__)
            if (p.empty()) p = "/";
#endif // __UNIX__
            wxFileData *fd = new wxFileData(p, "..", wxFileData::is_dir, wxFileIconsTable::folder);
            if (Add(fd, item) != -1)
                item.m_itemId++;
            else
                delete fd;
        }

        std::string dirname{m_dirName};
#if defined(WX_WINDOWS)
        if (dirname.length() == 2 && dirname[1u] == ':')
            dirname += '\\';
#endif // defined(WX_WINDOWS)

        if (dirname.empty())
            dirname = wxFILE_SEP_PATH;

        wxLogNull logNull;
        wxDir dir{dirname};

        if ( dir.IsOpened() )
        {
            std::string dirPrefix(dirname);
            if (dirPrefix.back() != wxFILE_SEP_PATH)
                dirPrefix += wxFILE_SEP_PATH;

            int hiddenFlag = m_showHidden ? wxDIR_HIDDEN : 0;

            bool cont;

            std::string f;

            // Get the directories first (not matched against wildcards):
            cont = dir.GetFirst(&f, {}, wxDIR_DIRS | hiddenFlag);
            while (cont)
            {
                wxFileData *fd = new wxFileData(dirPrefix + f, f, wxFileData::is_dir, wxFileIconsTable::folder);
                if (Add(fd, item) != -1)
                    item.m_itemId++;
                else
                    delete fd;

                cont = dir.GetNext(&f);
            }

            // Tokenize the wildcard string, so we can handle more than 1
            // search pattern in a wildcard.
            wxStringTokenizer tokenWild(m_wild, ";");
            while ( tokenWild.HasMoreTokens() )
            {
                cont = dir.GetFirst(&f, tokenWild.GetNextToken(),
                                        wxDIR_FILES | hiddenFlag);
                while (cont)
                {
                    wxFileData *fd = new wxFileData(dirPrefix + f, f, wxFileData::is_file, wxFileIconsTable::file);
                    if (Add(fd, item) != -1)
                        item.m_itemId++;
                    else
                        delete fd;

                    cont = dir.GetNext(&f);
                }
            }
        }
    }

    SortItems(m_sort_field, m_sort_forward);
}

void wxFileListCtrl::SetWild( const std::string &wild )
{
    if (wild.find('|') != std::string::npos)
        return;

    m_wild = wild;
    UpdateFiles();
}

void wxFileListCtrl::MakeDir()
{
    std::string new_name{_("NewName").ToStdString()};
    std::string path{m_dirName};
    path += wxFILE_SEP_PATH;
    path += new_name;
    if (wxFileExists(path))
    {
        // try NewName0, NewName1 etc.
        int i = 0;
        do {
            new_name = _("NewName");
            std::string num;
            num = fmt::format( "%d", i );
            new_name += num;

            path = m_dirName;
            path += wxFILE_SEP_PATH;
            path += new_name;
            i++;
        } while (wxFileExists(path));
    }

    wxLogNull log;
    if (!wxMkdir(path))
    {
        wxMessageDialog dialog(this, _("Operation not permitted.").ToStdString(), _("Error").ToStdString(), wxOK | wxICON_ERROR );
        dialog.ShowModal();
        return;
    }

    wxFileData *fd = new wxFileData( path, new_name, wxFileData::is_dir, wxFileIconsTable::folder );
    wxListItem item;
    item.m_itemId = 0;
    item.m_col = 0;
    long itemid = Add( fd, item );

    if (itemid != -1)
    {
        SortItems(m_sort_field, m_sort_forward);
        itemid = FindItem( 0, wxPtrToUInt(fd) );
        EnsureVisible( itemid );
        EditLabel( itemid );
    }
    else
        delete fd;
}

void wxFileListCtrl::GoToParentDir()
{
    if (!IsTopMostDir(m_dirName))
    {
        size_t len = m_dirName.length();
        if (wxEndsWithPathSeparator(m_dirName))
            m_dirName.pop_back();
        std::string fname( wxFileNameFromPath(m_dirName) );
        m_dirName = wxPathOnly( m_dirName );
#if defined(WX_WINDOWS)
        if (!m_dirName.empty())
        {
            if (m_dirName.back() == '.')
                m_dirName.clear();
        }
#elif defined(__UNIX__)
        if (m_dirName.empty())
            m_dirName = "/";
#endif
        UpdateFiles();
        long id = FindItem( 0, fname );
        if (id != wxNOT_FOUND)
        {
            SetItemState( id, ListStates::Selected, ListStates::Selected );
            EnsureVisible( id );
        }
    }
}

void wxFileListCtrl::GoToHomeDir()
{
    std::string s = wxGetUserHome( std::string() );
    GoToDir(s);
}

void wxFileListCtrl::GoToDir( const std::string &dir )
{
    if (!wxDirExists(dir)) return;

    m_dirName = dir;
    UpdateFiles();

    SetItemState( 0, ListStates::Selected, ListStates::Selected );

    EnsureVisible( 0 );
}

void wxFileListCtrl::FreeItemData(wxListItem& item)
{
    if ( item.m_data )
    {
        wxFileData *fd = (wxFileData*)item.m_data;
        delete fd;

        item.m_data = 0;
    }
}

void wxFileListCtrl::OnListDeleteItem( wxListEvent &event )
{
    FreeItemData(event.m_item);
}

void wxFileListCtrl::OnListDeleteAllItems( [[maybe_unused]] wxListEvent & event )
{
    FreeAllItemsData();
}

void wxFileListCtrl::FreeAllItemsData()
{
    wxListItem item;
    item.m_mask.set(ListMasks::Data);

    item.m_itemId = GetNextItem( -1, wxListGetNextItem::All );
    while ( item.m_itemId != -1 )
    {
        GetItem( item );
        FreeItemData(item);
        item.m_itemId = GetNextItem( item.m_itemId, wxListGetNextItem::All );
    }
}

void wxFileListCtrl::OnListEndLabelEdit( wxListEvent &event )
{
    wxFileData *fd = (wxFileData*)event.m_item.m_data;
    wxASSERT( fd );

    if ((event.GetLabel().empty()) ||
        (event.GetLabel() == ".") ||
        (event.GetLabel() == "..") ||
        (event.GetLabel().find( wxFILE_SEP_PATH ) != std::string::npos))
    {
        wxMessageDialog dialog(this, _("Illegal directory name.").ToStdString(), _("Error").ToStdString(), wxOK | wxICON_ERROR );
        dialog.ShowModal();
        event.Veto();
        return;
    }

    std::string new_name( wxPathOnly( fd->GetFilePath() ) );
    new_name += wxFILE_SEP_PATH;
    new_name += event.GetLabel();

    wxLogNull log;

    if (wxFileExists(new_name))
    {
        wxMessageDialog dialog(this, _("File name exists already.").ToStdString(), _("Error").ToStdString(), wxOK | wxICON_ERROR );
        dialog.ShowModal();
        event.Veto();
    }

    if (wxRenameFile(fd->GetFilePath(),new_name))
    {
        fd->SetNewName( new_name, event.GetLabel() );

        SetItemState( event.GetItem(), ListStates::Selected, ListStates::Selected );

        UpdateItem( event.GetItem() );
        EnsureVisible( event.GetItem() );
    }
    else
    {
        wxMessageDialog dialog(this, _("Operation not permitted.").ToStdString(), _("Error").ToStdString(), wxOK | wxICON_ERROR );
        dialog.ShowModal();
        event.Veto();
    }
}

void wxFileListCtrl::OnListColClick( wxListEvent &event )
{
    int col = event.GetColumn();

    switch (col)
    {
        case wxFileData::FileList_Name :
        case wxFileData::FileList_Size :
        case wxFileData::FileList_Type :
        case wxFileData::FileList_Time : break;
        default : return;
    }

    if ((wxFileData::fileListFieldType)col == m_sort_field)
        m_sort_forward = !m_sort_forward;
    else
        m_sort_field = (wxFileData::fileListFieldType)col;

    SortItems(m_sort_field, m_sort_forward);
}

void wxFileListCtrl::OnSize( wxSizeEvent &event )
{
    event.Skip();

    if ( InReportView() )
    {
        // In report mode, set name column to use remaining width.
        int newNameWidth = GetClientSize().x;
        for ( int i = 1; i < GetColumnCount(); i++ )
        {
            newNameWidth -= GetColumnWidth(i);
            if ( newNameWidth <= 0 )
                return;
        }

        SetColumnWidth(0, newNameWidth);
    }
}

void wxFileListCtrl::SortItems(wxFileData::fileListFieldType field, bool forward)
{
    m_sort_field = field;
    m_sort_forward = forward;
    const long sort_dir = forward ? 1 : -1;

    switch (m_sort_field)
    {
        case wxFileData::FileList_Size :
            wxListCtrl::SortItems(wxFileDataSizeCompare, sort_dir);
            break;

        case wxFileData::FileList_Type :
            wxListCtrl::SortItems(wxFileDataTypeCompare, sort_dir);
            break;

        case wxFileData::FileList_Time :
            wxListCtrl::SortItems(wxFileDataTimeCompare, sort_dir);
            break;

        case wxFileData::FileList_Name :
        default :
            wxListCtrl::SortItems(wxFileDataNameCompare, sort_dir);
            break;
    }
}

wxFileListCtrl::~wxFileListCtrl()
{
    // Normally the data are freed via an EVT_LIST_DELETE_ALL_ITEMS event and
    // wxFileListCtrl::OnListDeleteAllItems. But if the event is generated after
    // the destruction of the wxFileListCtrl we need to free any data here:
    FreeAllItemsData();
}

#define  ID_CHOICE        (wxID_FILECTRL + 1)
#define  ID_TEXT          (wxID_FILECTRL + 2)
#define  ID_FILELIST_CTRL (wxID_FILECTRL + 3)
#define  ID_CHECK         (wxID_FILECTRL + 4)

///////////////////////////////////////////////////////////////////////////////
// wxGenericFileCtrl implementation
///////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE( wxGenericFileCtrl, wxNavigationEnabled<wxControl> )
    EVT_LIST_ITEM_SELECTED( ID_FILELIST_CTRL, wxGenericFileCtrl::OnSelected )
    EVT_LIST_ITEM_ACTIVATED( ID_FILELIST_CTRL, wxGenericFileCtrl::OnActivated )
    EVT_CHOICE( ID_CHOICE, wxGenericFileCtrl::OnChoiceFilter )
    EVT_TEXT_ENTER( ID_TEXT, wxGenericFileCtrl::OnTextEnter )
    EVT_TEXT( ID_TEXT, wxGenericFileCtrl::OnTextChange )
    EVT_CHECKBOX( ID_CHECK, wxGenericFileCtrl::OnCheck )
wxEND_EVENT_TABLE()

bool wxGenericFileCtrl::Create( wxWindow *parent,
                                wxWindowID id,
                                const std::string& defaultDirectory,
                                const std::string& defaultFileName,
                                const std::string& wildCard,
                                unsigned int style,
                                const wxPoint& pos,
                                const wxSize& size,
                                std::string_view name )
{
    this->m_style = style;
    m_inSelected = false;
    m_noSelChgEvent = false;
    m_check = nullptr;

    // check that the styles are not contradictory
    wxASSERT_MSG( !( ( m_style & wxFC_SAVE ) && ( m_style & wxFC_OPEN ) ),
                  "can't specify both wxFC_SAVE and wxFC_OPEN at once" );

    wxASSERT_MSG( !( ( m_style & wxFC_SAVE ) && ( m_style & wxFC_MULTIPLE ) ),
                  "wxFC_MULTIPLE can't be used with wxFC_SAVE" );

    wxNavigationEnabled<wxControl>::Create( parent, id,
                                            pos, size,
                                            wxTAB_TRAVERSAL,
                                            wxValidator{},
                                            name );

    m_dir = defaultDirectory;

    m_ignoreChanges = true;

    if ( ( m_dir.empty() ) || ( m_dir == wxT( "." ) ) )
    {
        m_dir = wxGetCwd();
        if ( m_dir.empty() )
            m_dir = wxFILE_SEP_PATH;
    }

    const size_t len = m_dir.length();
    if ( ( len > 1 ) && ( wxEndsWithPathSeparator( m_dir ) ) )
        m_dir.pop_back();

    m_filterExtension.clear();

    // layout

    const bool is_pda = ( wxSystemSettings::GetScreenType() <= wxSYS_SCREEN_PDA );

    wxBoxSizer *mainsizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer *staticsizer = new wxBoxSizer( wxHORIZONTAL );
    if ( is_pda )
        staticsizer->Add( new wxStaticText( this, wxID_ANY, _( "Current directory:" ).ToStdString()),
                          wxSizerFlags().DoubleBorder(wxRIGHT) );
    m_static = new wxStaticText( this, wxID_ANY, m_dir );
    staticsizer->Add( m_static, 1 );
    mainsizer->Add( staticsizer, wxSizerFlags().Expand().Border());

    long style2 = wxLC_LIST;
    if ( !( m_style & wxFC_MULTIPLE ) )
        style2 |= wxLC_SINGLE_SEL;

    style2 |= wxSUNKEN_BORDER;

    m_list = new wxFileListCtrl( this, ID_FILELIST_CTRL,
                                 {}, false,
                                 wxDefaultPosition, wxSize( 400, 140 ),
                                 style2 );

    m_text = new wxTextCtrl( this, ID_TEXT, "",
                             wxDefaultPosition, wxDefaultSize,
                             wxTE_PROCESS_ENTER );
    m_choice = new wxChoice( this, ID_CHOICE, wxDefaultPosition, wxDefaultSize );

    if ( is_pda )
    {
        // PDAs have a different screen layout
        mainsizer->Add( m_list, wxSizerFlags( 1 ).Expand().HorzBorder() );

        wxBoxSizer *textsizer = new wxBoxSizer( wxHORIZONTAL );
        textsizer->Add( m_text, wxSizerFlags( 1 ).Centre().Border() );
        textsizer->Add( m_choice, wxSizerFlags( 1 ).Centre().Border() );
        mainsizer->Add( textsizer, wxSizerFlags().Expand() );

    }
    else // !is_pda
    {
        mainsizer->Add( m_list, wxSizerFlags( 1 ).Expand().Border() );
        mainsizer->Add( m_text, wxSizerFlags().Expand().Border() );

        wxBoxSizer *choicesizer = new wxBoxSizer( wxHORIZONTAL );
        choicesizer->Add( m_choice, wxSizerFlags( 1 ).Centre() );

        if ( !( m_style & wxFC_NOSHOWHIDDEN ) )
        {
            m_check = new wxCheckBox( this, ID_CHECK, _( "Show &hidden files" ) );
            choicesizer->Add( m_check, wxSizerFlags().Centre().DoubleBorder(wxLEFT) );
        }

        mainsizer->Add( choicesizer, wxSizerFlags().Expand().Border() );
    }

    SetWildcard( wildCard );

    SetAutoLayout( true );
    SetSizer( mainsizer );

    if ( !is_pda )
    {
        mainsizer->Fit( this );
    }

    m_list->GoToDir( m_dir );
    UpdateControls();
    m_text->SetValue( m_fileName );

    m_ignoreChanges = false;

    // must be after m_ignoreChanges = false
    SetFilename( defaultFileName );

    return true;
}

// NB: there is an unfortunate mismatch between wxFileName and wxFileDialog
//     method names but our GetDirectory() does correspond to wxFileName::
//     GetPath() while our GetPath() is wxFileName::GetFullPath()
std::string wxGenericFileCtrl::GetPath() const
{
    wxASSERT_MSG ( !(m_style & wxFC_MULTIPLE), "use GetPaths() instead" );

    return DoGetFileName().GetFullPath();
}

std::string wxGenericFileCtrl::GetFilename() const
{
    wxASSERT_MSG ( !(m_style & wxFC_MULTIPLE), "use GetFilenames() instead" );

    return DoGetFileName().GetFullName();
}

std::string wxGenericFileCtrl::GetDirectory() const
{
    // don't check for wxFC_MULTIPLE here, this one is probably safe to call in
    // any case as it can be always taken to mean "current directory"
    return DoGetFileName().GetPath();
}

wxFileName wxGenericFileCtrl::DoGetFileName() const
{
    wxFileName fn;

    std::string value = m_text->GetValue();
    if ( value.empty() )
    {
        // nothing in the text control, get the selected file from the list
        wxListItem item;
        item.m_itemId = m_list->GetNextItem(-1, wxListGetNextItem::All,
                                            ListStates::Selected);

        // ... if anything is selected in the list
        if ( item.m_itemId != wxNOT_FOUND )
        {
            m_list->GetItem(item);

            fn.Assign(m_list->GetDir(), boost::nowide::narrow(item.m_text));
        }
    }
    else // user entered the value
    {
        // the path can be either absolute or relative
        fn.Assign(value);
        if ( fn.IsRelative() )
            fn.MakeAbsolute(m_list->GetDir());
    }

    return fn;
}

std::vector<std::string> wxGenericFileCtrl::DoGetFilenames(bool fullPath) const
{
    std::vector<std::string> filenames;

    const std::string dir = m_list->GetDir();

    const std::string value = m_text->GetValue();
    
    if ( !value.empty() )
    {
        wxFileName fn(value);
        if ( fn.IsRelative() )
            fn.MakeAbsolute(dir);

        filenames.push_back(fullPath ? fn.GetFullPath() : fn.GetFullName());
        return filenames;
    }

    const int numSel = m_list->GetSelectedItemCount();
    if ( !numSel )
        return {};

    filenames.reserve(numSel);

    wxListItem item;
    item.m_mask = ListMasks::Text;
    item.m_itemId = -1;
    for ( ;; )
    {
        item.m_itemId = m_list->GetNextItem(item.m_itemId, wxListGetNextItem::All,
                                            ListStates::Selected);

        if ( item.m_itemId == -1 )
            break;

        m_list->GetItem(item);

        const wxFileName fn(dir, boost::nowide::narrow(item.m_text));
        // FIXME: We're checking this every time.
        filenames.push_back(fullPath ? fn.GetFullPath() : fn.GetFullName());
    }

    return filenames;
}

bool wxGenericFileCtrl::SetDirectory( const std::string& dir )
{
    m_ignoreChanges = true;
    m_list->GoToDir( dir );
    UpdateControls();
    m_ignoreChanges = false;

    return wxFileName( dir ).SameAs( m_list->GetDir() );
}

bool wxGenericFileCtrl::SetFilename( const std::string& name )
{
    std::string dir, fn, ext;
    wxFileName::SplitPath(name, &dir, &fn, &ext);
    wxCHECK_MSG( dir.empty(), false,
                 "can't specify directory component to SetFilename" );

    m_noSelChgEvent = true;

    m_text->ChangeValue( name );

    // Deselect previously selected items
    {
        const int numSelectedItems = m_list->GetSelectedItemCount();

        if ( numSelectedItems > 0 )
        {
            long itemIndex = -1;

            for ( ;; )
            {
                itemIndex = m_list->GetNextItem( itemIndex, wxListGetNextItem::All, ListStates::Selected );
                if ( itemIndex == wxNOT_FOUND )
                    break;

                m_list->SetItemState( itemIndex, ListStateFlags{}, ListStates::Selected );
            }
        }
    }

    // Select new filename if it's in the list
    long item = m_list->FindItem(wxNOT_FOUND, name);

    if ( item != wxNOT_FOUND )
    {
        m_list->SetItemState( item, ListStates::Selected, ListStates::Selected );
        m_list->EnsureVisible( item );
    }

    m_noSelChgEvent = false;

    return true;
}

void wxGenericFileCtrl::DoSetFilterIndex( int filterindex )
{
    wxClientData *pcd = m_choice->GetClientObject( filterindex );
    if ( !pcd )
        return;

    const std::string& str = ((dynamic_cast<wxStringClientData *>(pcd))->GetData());
    m_list->SetWild( str );
    m_filterIndex = filterindex;
    if ( str.substr(0, 2) == "*.")
    {
        m_filterExtension = str.substr( 1 );
        if ( m_filterExtension == wxT( ".*" ) )
            m_filterExtension.clear();
    }
    else
    {
        m_filterExtension.clear();
    }

    wxGenerateFilterChangedEvent( this, this );
}

void wxGenericFileCtrl::SetWildcard( const std::string& wildCard )
{
    if ( wildCard.empty() || wildCard == wxFileSelectorDefaultWildcardStr )
    {
        // FIXME: Removed translation for fmt lib
        m_wildCard = fmt::format( "All files (%s)|%s",
                                       wxFileSelectorDefaultWildcardStr,
                                       wxFileSelectorDefaultWildcardStr );
    }
    else
        m_wildCard = wildCard;

    std::vector<std::string> wildDescriptions, wildFilters;
    const size_t count = wxParseCommonDialogsFilter( m_wildCard,
                         wildDescriptions,
                         wildFilters );
    wxCHECK_RET( count, "wxFileDialog: bad wildcard string" );

    m_choice->Clear();

    for ( size_t n = 0; n < count; n++ )
    {
        m_choice->Append(wildDescriptions[n], new wxStringClientData(wildFilters[n]));
    }

    SetFilterIndex( 0 );
}

void wxGenericFileCtrl::SetFilterIndex( int filterindex )
{
    m_choice->SetSelection( filterindex );

    DoSetFilterIndex( filterindex );
}

void wxGenericFileCtrl::OnChoiceFilter( wxCommandEvent &event )
{
    DoSetFilterIndex( ( int )event.GetInt() );
}

void wxGenericFileCtrl::OnCheck( wxCommandEvent &event )
{
    m_list->ShowHidden( event.GetInt() != 0 );
}

void wxGenericFileCtrl::OnActivated( wxListEvent &event )
{
    HandleAction( boost::nowide::narrow(event.m_item.m_text) );
}

void wxGenericFileCtrl::OnTextEnter( [[maybe_unused]] wxCommandEvent& event )
{
    HandleAction( m_text->GetValue() );
}

void wxGenericFileCtrl::OnTextChange( [[maybe_unused]] wxCommandEvent& event )
{
    if ( !m_ignoreChanges )
    {
        // Clear selections.  Otherwise when the user types in a value they may
        // not get the file whose name they typed.
        if ( m_list->GetSelectedItemCount() > 0 )
        {
            long item = m_list->GetNextItem( -1, wxListGetNextItem::All,
                                             ListStates::Selected );
            while ( item != -1 )
            {
                m_list->SetItemState( item, ListStateFlags{}, ListStates::Selected );
                item = m_list->GetNextItem( item, wxListGetNextItem::All, ListStates::Selected );
            }
        }
    }
}

void wxGenericFileCtrl::OnSelected( wxListEvent &event )
{
    if ( m_ignoreChanges )
        return;

    if ( m_inSelected )
        return;

    m_inSelected = true;
    const std::string filename( boost::nowide::narrow(event.m_item.m_text) );

    if ( filename == wxT( ".." ) )
    {
        m_inSelected = false;
        return;
    }

    std::string dir = m_list->GetDir();
    if ( !IsTopMostDir( dir ) )
        dir += wxFILE_SEP_PATH;
    dir += filename;
    if ( wxDirExists( dir ) )
    {
        m_inSelected = false;

        return;
    }


    m_ignoreChanges = true;
    m_text->SetValue( filename );

    if ( m_list->GetSelectedItemCount() > 1 )
    {
        m_text->Clear();
    }

    if ( !m_noSelChgEvent )
        wxGenerateSelectionChangedEvent( this, this );

    m_ignoreChanges = false;
    m_inSelected = false;
}

void wxGenericFileCtrl::HandleAction( const std::string &fn )
{
    if ( m_ignoreChanges )
        return;

    std::string filename( fn );
    if ( filename.empty() )
    {
        return;
    }
    if ( filename == wxT( "." ) ) return;

    std::string dir = m_list->GetDir();

    // "some/place/" means they want to chdir not try to load "place"
    const bool want_dir = filename.back() == wxFILE_SEP_PATH;
    if ( want_dir )
        filename.pop_back();

    if ( filename == "..")
    {
        m_ignoreChanges = true;
        m_list->GoToParentDir();

        wxGenerateFolderChangedEvent( this, this );

        UpdateControls();
        m_ignoreChanges = false;
        return;
    }

#ifdef __UNIX__
    if ( filename == "~")
    {
        m_ignoreChanges = true;
        m_list->GoToHomeDir();

        wxGenerateFolderChangedEvent( this, this );

        UpdateControls();
        m_ignoreChanges = false;
        return;
    }

    if ( wx::utils::BeforeFirst(filename, '/') == "~" )
    {
        filename = std::string( wxGetUserHome() ) + filename.erase(0);
    }
#endif // __UNIX__

    if ( !( m_style & wxFC_SAVE ) )
    {
        if ( ( filename.find('*') != std::string::npos ) ||
                ( filename.find('?') != std::string::npos ) )
        {
            if ( filename.find( wxFILE_SEP_PATH ) != std::string::npos )
            {
                wxMessageBox( _( "Illegal file specification." ).ToStdString(),
                              _( "Error" ).ToStdString(), wxOK | wxICON_ERROR, this );
                return;
            }
            m_list->SetWild( filename );
            return;
        }
    }

    if ( !IsTopMostDir( dir ) )
        dir += wxFILE_SEP_PATH;
    if ( !wxIsAbsolutePath( filename ) )
    {
        dir += filename;
        filename = dir;
    }

    if ( wxDirExists( filename ) )
    {
        m_ignoreChanges = true;
        m_list->GoToDir( filename );
        UpdateControls();

        wxGenerateFolderChangedEvent( this, this );

        m_ignoreChanges = false;
        return;
    }

    // they really wanted a dir, but it doesn't exist
    if ( want_dir )
    {
        wxMessageBox( _( "Directory doesn't exist." ).ToStdString(), _( "Error" ).ToStdString(),
                      wxOK | wxICON_ERROR, this );
        return;
    }

    // append the default extension to the filename if it doesn't have any
    //
    // VZ: the logic of testing for !wxFileExists() only for the open file
    //     dialog is not entirely clear to me, why don't we allow saving to a
    //     file without extension as well?
    if ( !( m_style & wxFC_OPEN ) || !wxFileExists( filename ) )
    {
        filename = wxFileDialogBase::AppendExtension( filename, m_filterExtension );
        wxGenerateFileActivatedEvent( this, this, wxFileName( filename ).GetFullName() );
        return;
    }

    wxGenerateFileActivatedEvent( this, this );
}

bool wxGenericFileCtrl::SetPath( const std::string& path )
{
    std::string dir, fn, ext;
    wxFileName::SplitPath(path, &dir, &fn, &ext);

    if ( !dir.empty() && !wxFileName::DirExists(dir) )
        return false;

    m_dir = dir;
    m_fileName = fn;
    if ( !ext.empty() || path.back() == '.' )
    {
        m_fileName += ".";
        m_fileName += ext;
    }

    SetDirectory( m_dir );
    SetFilename( m_fileName );

    return true;
}

std::vector<std::string> wxGenericFileCtrl::GetPaths() const
{
    return DoGetFilenames(true);
}

std::vector<std::string> wxGenericFileCtrl::GetFilenames() const
{
    return DoGetFilenames( false );
}

void wxGenericFileCtrl::UpdateControls()
{
    const std::string dir = m_list->GetDir();
    m_static->SetLabel( dir );
}

void wxGenericFileCtrl::GoToParentDir()
{
    m_list->GoToParentDir();
    UpdateControls();
}

void wxGenericFileCtrl::GoToHomeDir()
{
    m_list->GoToHomeDir();
    UpdateControls();
}

#endif // wxUSE_FILECTRL
