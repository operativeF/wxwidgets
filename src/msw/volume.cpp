///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/volume.cpp
// Purpose:     wxFSVolume - encapsulates system volume information
// Author:      George Policello
// Modified by:
// Created:     28 Jan 02
// Copyright:   (c) 2002 George Policello
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_FSVOLUME

#include "wx/volume.h"

#if wxUSE_GUI
    #include "wx/icon.h"
#endif

#include "wx/intl.h"
#include "wx/log.h"
#include "wx/hashmap.h"
#include "wx/filefn.h"

#include "wx/dir.h"
#include "wx/dynlib.h"
#include "wx/arrimpl.cpp"

#include "wx/msw/wrapshl.h"

#include <boost/nowide/convert.hpp>
#include <boost/nowide/stackstring.hpp>

import Utils.Strings;

import WX.WinDef;

#if wxUSE_BASE

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Dynamic library function defs.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#if wxUSE_DYNLIB_CLASS
static wxDynamicLibrary s_mprLib;
#endif

using WNetOpenEnumPtr = DWORD(WINAPI*)(DWORD, DWORD, DWORD, LPNETRESOURCE, LPHANDLE);
using WNetEnumResourcePtr = DWORD (WINAPI*)(HANDLE, LPDWORD, LPVOID, LPDWORD);
using WNetCloseEnumPtr = DWORD (WINAPI*)(HANDLE);

static WNetOpenEnumPtr s_pWNetOpenEnum;
static WNetEnumResourcePtr s_pWNetEnumResource;
static WNetCloseEnumPtr s_pWNetCloseEnum;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Globals/Statics
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#if defined(__CYGWIN__) && defined(__LP64__)
    // We can't use long in 64 bit Cygwin build because Cygwin uses LP64 model
    // (unlike all the other MSW compilers) and long is 64 bits, while
    // InterlockedExchange(), with which this variable is used, requires a 32
    // bit-sized value, so use Cygwin-specific type with the right size.
    using wxInterlockedArg_t = __LONG32;
#else
    using wxInterlockedArg_t = long;
#endif

static wxInterlockedArg_t s_cancelSearch = FALSE;

struct FileInfo
{
    explicit FileInfo(unsigned flag=0, wxFSVolumeKind type=wxFSVolumeKind::Other) :
        m_flags(flag), m_type(type)
    {}

    unsigned m_flags;
    wxFSVolumeKind m_type;
};
WX_DECLARE_STRING_HASH_MAP(FileInfo, FileInfoMap);
// Cygwin bug (?) destructor for global s_fileInfo is called twice...
static FileInfoMap& GetFileInfoMap()
{
    static FileInfoMap s_fileInfo(25);

    return s_fileInfo;
}
#define s_fileInfo (GetFileInfoMap())

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Local helper functions.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//=============================================================================
// Function: GetBasicFlags
// Purpose: Set basic flags, primarily wxFS_VOL_REMOTE and wxFS_VOL_REMOVABLE.
// Notes: - Local and mapped drives are mounted by definition.  We have no
//          way to determine mounted status of network drives, so assume that
//          all drives are mounted, and let the caller decide otherwise.
//        - Other flags are 'best guess' from type of drive.  The system will
//          not report the file attributes with any degree of accuracy.
//=============================================================================
static unsigned GetBasicFlags(const std::string& filename)
{
    unsigned flags = wxFS_VOL_MOUNTED;

    //----------------------------------
    // 'Best Guess' based on drive type.
    //----------------------------------
    wxFSVolumeKind type;
    boost::nowide::wstackstring stackFileName{filename.c_str()};
    switch(::GetDriveTypeW(stackFileName.get()))
    {
    case DRIVE_FIXED:
        type = wxFSVolumeKind::Disk;
        break;

    case DRIVE_REMOVABLE:
        flags |= wxFS_VOL_REMOVABLE;
        type = wxFSVolumeKind::Floppy;
        break;

    case DRIVE_CDROM:
        flags |= wxFS_VOL_REMOVABLE | wxFS_VOL_READONLY;
        type = wxFSVolumeKind::CDROM;
        break;

    case DRIVE_REMOTE:
        flags |= wxFS_VOL_REMOTE;
        type = wxFSVolumeKind::Network;
        break;

    case DRIVE_NO_ROOT_DIR:
        flags &= ~wxFS_VOL_MOUNTED;
        type = wxFSVolumeKind::Other;
        break;

    default:
        type = wxFSVolumeKind::Other;
        break;
    }

    //-----------------------------------------------------------------------
    // The following most likely will not modify anything not set above,
    // and will not work at all for network shares or empty CD ROM drives.
    // But it is a good check if the Win API ever gets better about reporting
    // this information.
    //-----------------------------------------------------------------------
    SHFILEINFOW fi;

    if (!::SHGetFileInfoW(stackFileName.get(), 0, &fi, sizeof(fi), SHGFI_ATTRIBUTES))
    {
        // this error is not fatal, so don't show a message to the user about
        // it, otherwise it would appear every time a generic directory picker
        // dialog is used and there is a connected network drive
        wxLogLastError("SHGetFileInfo");
    }
    else
    {
        if (fi.dwAttributes & SFGAO_READONLY)
            flags |= wxFS_VOL_READONLY;
        if (fi.dwAttributes & SFGAO_REMOVABLE)
            flags |= wxFS_VOL_REMOVABLE;
    }

    //------------------
    // Flags are cached.
    //------------------
    s_fileInfo[filename] = FileInfo(flags, type);

    return flags;
} // GetBasicFlags

//=============================================================================
// Function: FilteredAdd
// Purpose: Add a file to the list if it meets the filter requirement.
// Notes: - See GetBasicFlags for remarks about the Mounted flag.
//=============================================================================
static bool FilteredAdd(std::vector<std::string>& list, const std::string& filename,
                        unsigned int flagsSet, unsigned int flagsUnset)
{
    bool accept = true;
    const unsigned flags = GetBasicFlags(filename);

    if (flagsSet & wxFS_VOL_MOUNTED && !(flags & wxFS_VOL_MOUNTED))
        accept = false;
    else if (flagsUnset & wxFS_VOL_MOUNTED && (flags & wxFS_VOL_MOUNTED))
        accept = false;
    else if (flagsSet & wxFS_VOL_REMOVABLE && !(flags & wxFS_VOL_REMOVABLE))
        accept = false;
    else if (flagsUnset & wxFS_VOL_REMOVABLE && (flags & wxFS_VOL_REMOVABLE))
        accept = false;
    else if (flagsSet & wxFS_VOL_READONLY && !(flags & wxFS_VOL_READONLY))
        accept = false;
    else if (flagsUnset & wxFS_VOL_READONLY && (flags & wxFS_VOL_READONLY))
        accept = false;
    else if (flagsSet & wxFS_VOL_REMOTE && !(flags & wxFS_VOL_REMOTE))
        accept = false;
    else if (flagsUnset & wxFS_VOL_REMOTE && (flags & wxFS_VOL_REMOTE))
        accept = false;

    // Add to the list if passed the filter.
    if (accept)
        list.push_back(filename);

    return accept;
} // FilteredAdd

//=============================================================================
// Function: BuildListFromNN
// Purpose: Append or remove items from the list
// Notes: - There is no way to find all disconnected NN items, or even to find
//          all items while determining which are connected and not.  So this
//          function will find either all items or connected items.
//=============================================================================
static void BuildListFromNN(std::vector<std::string>& list, NETRESOURCEW* pResSrc,
                            unsigned flagsSet, unsigned flagsUnset)
{
    HANDLE hEnum;
    int rc;

    //-----------------------------------------------
    // Scope may be all drives or all mounted drives.
    //-----------------------------------------------
    unsigned scope = RESOURCE_GLOBALNET;
    if (flagsSet & wxFS_VOL_MOUNTED)
        scope = RESOURCE_CONNECTED;

    //----------------------------------------------------------------------
    // Enumerate all items, adding only non-containers (ie. network shares).
    // Containers cause a recursive call to this function for their own
    // enumeration.
    //----------------------------------------------------------------------
    if (rc = s_pWNetOpenEnum(scope, RESOURCETYPE_DISK, 0, pResSrc, &hEnum), rc == NO_ERROR)
    {
        DWORD count = 1;
        DWORD size = 256;
        NETRESOURCEW* pRes = (NETRESOURCEW*)malloc(size);
        memset(pRes, 0, sizeof(NETRESOURCEW));
        while (rc = s_pWNetEnumResource(hEnum, &count, pRes, &size), rc == NO_ERROR || rc == ERROR_MORE_DATA)
        {
            if (s_cancelSearch)
                break;

            if (rc == ERROR_MORE_DATA)
            {
                pRes = (NETRESOURCEW*)realloc(pRes, size);
                count = 1;
            }
            else if (count == 1)
            {
                // Enumerate the container.
                if (pRes->dwUsage & RESOURCEUSAGE_CONTAINER)
                {
                    BuildListFromNN(list, pRes, flagsSet, flagsUnset);
                }

                // Add the network share.
                else
                {
                    std::string filename = boost::nowide::narrow(pRes->lpRemoteName);

                    // if the drive is unavailable, FilteredAdd() can hang for
                    // a long time and, moreover, its failure appears to be not
                    // cached so this will happen every time we use it, so try
                    // a much quicker wxDirExists() test (which still hangs but
                    // for much shorter time) for locally mapped drives first
                    // to try to avoid this
                    if ( pRes->lpLocalName &&
                            *pRes->lpLocalName &&
                                !wxDirExists(boost::nowide::narrow(pRes->lpLocalName)) )
                        continue;

                    if (!filename.empty())
                    {
                        if (filename.back() != '\\')
                            filename.push_back('\\');

                        // The filter function will not know mounted from unmounted, and neither do we unless
                        // we are iterating using RESOURCE_CONNECTED, in which case they all are mounted.
                        // Volumes on disconnected servers, however, will correctly show as unmounted.
                        FilteredAdd(list, filename, flagsSet, flagsUnset&~wxFS_VOL_MOUNTED);
                        if (scope == RESOURCE_GLOBALNET)
                            s_fileInfo[filename].m_flags &= ~wxFS_VOL_MOUNTED;
                    }
                }
            }
            else if (count == 0)
                break;
        }
        free(pRes);
        s_pWNetCloseEnum(hEnum);
    }
} // BuildListFromNN

//=============================================================================
// Function: CompareFcn
// Purpose: Used to sort the NN list alphabetically, case insensitive.
//=============================================================================
static int CompareFcn(const wxString& first, const wxString& second)
{
    return wxStricmp(first.c_str(), second.c_str());
} // CompareFcn

//=============================================================================
// Function: BuildRemoteList
// Purpose: Append Network Neighborhood items to the list.
// Notes: - Mounted gets transalated into Connected.  FilteredAdd is told
//          to ignore the Mounted flag since we need to handle it in a weird
//          way manually.
//        - The resulting list is sorted alphabetically.
//=============================================================================
static bool BuildRemoteList(std::vector<std::string>& list, NETRESOURCEW* pResSrc,
                            unsigned flagsSet, unsigned flagsUnset)
{
    // NN query depends on dynamically loaded library.
    if (!s_pWNetOpenEnum || !s_pWNetEnumResource || !s_pWNetCloseEnum)
    {
        wxLogError(_("Failed to load mpr.dll."));
        return false;
    }

    // Don't waste time doing the work if the flags conflict.
    if (flagsSet & wxFS_VOL_MOUNTED && flagsUnset & wxFS_VOL_MOUNTED)
        return false;

    //----------------------------------------------
    // Generate the list according to the flags set.
    //----------------------------------------------
    BuildListFromNN(list, pResSrc, flagsSet, flagsUnset);

    std::sort(list.begin(), list.end(), CompareFcn);

    //-------------------------------------------------------------------------
    // If mounted only is requested, then we only need one simple pass.
    // Otherwise, we need to build a list of all NN volumes and then apply the
    // list of mounted drives to it.
    //-------------------------------------------------------------------------
    if (!(flagsSet & wxFS_VOL_MOUNTED))
    {
        // generate.
        std::vector<std::string> mounted;
        BuildListFromNN(mounted, pResSrc, flagsSet | wxFS_VOL_MOUNTED, flagsUnset & ~wxFS_VOL_MOUNTED);
        
        std::sort(mounted.begin(), mounted.end(), CompareFcn);

        // apply list from bottom to top to preserve indexes if removing items.
        ssize_t iList = list.size() - 1;
        for (ssize_t iMounted = mounted.size() - 1; iMounted >= 0 && iList >= 0; iMounted--)
        {
            int compare;
            std::string all = list[iList];
            std::string mount = mounted[iMounted];

            while (compare =
                     wxStricmp(list[iList].c_str(), mounted[iMounted].c_str()),
                   compare > 0 && iList >= 0)
            {
                iList--;
                all = list[iList];
            }


            if (compare == 0)
            {
                // Found the element.  Remove it or mark it mounted.
                if (flagsUnset & wxFS_VOL_MOUNTED)
                    list.erase(std::begin(list) + iList);
                else
                    s_fileInfo[list[iList]].m_flags |= wxFS_VOL_MOUNTED;

            }

            iList--;
        }
    }

    return true;
} // BuildRemoteList

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// wxFSVolume
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//=============================================================================
// Function: GetVolumes
// Purpose: Generate and return a list of all volumes (drives) available.
// Notes:
//=============================================================================
std::vector<std::string> wxFSVolumeBase::GetVolumes(unsigned int flagsSet, unsigned int flagsUnset)
{
    ::InterlockedExchange(&s_cancelSearch, FALSE);     // reset

#if wxUSE_DYNLIB_CLASS
    if (!s_mprLib.IsLoaded() && s_mprLib.Load("mpr.dll"))
    {
        s_pWNetOpenEnum = (WNetOpenEnumPtr)s_mprLib.GetSymbol("WNetOpenEnumW");
        s_pWNetEnumResource = (WNetEnumResourcePtr)s_mprLib.GetSymbol("WNetEnumResourceW");

        s_pWNetCloseEnum = (WNetCloseEnumPtr)s_mprLib.GetSymbol("WNetCloseEnum");
    }
#endif

    std::vector<std::string> list;

    //-------------------------------
    // Local and mapped drives first.
    //-------------------------------
    // Allocate the required space for the API call.
    const DWORD chars = ::GetLogicalDriveStringsW(0, nullptr);

    std::vector<WCHAR> buf;
    buf.resize(chars + 1);

    // Get the list of drives.
    ::GetLogicalDriveStringsW(chars, buf.data());

    // Parse the list into an array, applying appropriate filters.
    boost::nowide::stackstring stackBuf;

    stackBuf.convert(buf.data(), buf.data() + buf.size());

    std::string convertedBuffer{stackBuf.get(), stackBuf.buffer_size};

    std::vector<std::string> vols = wx::utils::JoinChArray(convertedBuffer);

    for(auto&& vol : vols)
    {
        FilteredAdd(list, vol, flagsSet, flagsUnset);
    }

    //---------------------------
    // Network Neighborhood next.
    //---------------------------

    // not exclude remote and not removable
    if (!(flagsUnset & wxFS_VOL_REMOTE) &&
        !(flagsSet & wxFS_VOL_REMOVABLE)
       )
    {
        // The returned list will be sorted alphabetically.  We don't pass
        // our in since we don't want to change to order of the local drives.
        std::vector<std::string> nn;
        if (BuildRemoteList(nn, nullptr, flagsSet, flagsUnset))
        {
            for (size_t idx = 0; idx < nn.size(); idx++)
                list.push_back(nn[idx]);
        }
    }

    return list;
} // GetVolumes

//=============================================================================
// Function: CancelSearch
// Purpose: Instruct an active search to stop.
// Notes: - This will only sensibly be called by a thread other than the one
//          performing the search.  This is the only thread-safe function
//          provided by the class.
//=============================================================================
void wxFSVolumeBase::CancelSearch()
{
    ::InterlockedExchange(&s_cancelSearch, TRUE);
} // CancelSearch

wxFSVolumeBase::wxFSVolumeBase(const wxString& name)
{
    Create(name);
} // wxVolume

bool wxFSVolumeBase::Create(const wxString& name)
{
    // assume fail.
    m_isOk = false;

    // supplied.
    m_volName = name;

    // Display name.
    SHFILEINFO fi;
    const auto rc = ::SHGetFileInfoW(m_volName.t_str(), 0, &fi, sizeof(fi), SHGFI_DISPLAYNAME);
    if (!rc)
    {
        wxLogError(_("Cannot read typename from '%s'!"), m_volName.c_str());
        return false;
    }
    m_dispName = fi.szDisplayName;

    // all tests passed.
    m_isOk = true;
    return true;
} // Create

//=============================================================================
// Function: IsOk
// Purpose: returns true if the volume is legal.
// Notes: For fixed disks, it must exist.  For removable disks, it must also
//        be present.  For Network Shares, it must also be logged in, etc.
//=============================================================================
bool wxFSVolumeBase::IsOk() const
{
    return m_isOk;
} // IsOk

//=============================================================================
// Function: GetKind
// Purpose: Return the type of the volume.
//=============================================================================
wxFSVolumeKind wxFSVolumeBase::GetKind() const
{
    if (!m_isOk)
        return wxFSVolumeKind::Other;

    FileInfoMap::iterator itr = s_fileInfo.find(m_volName);
    if (itr == s_fileInfo.end())
        return wxFSVolumeKind::Other;

    return itr->second.m_type;
}

//=============================================================================
// Function: GetFlags
// Purpose: Return the caches flags for this volume.
// Notes: - Returns -1 if no flags were cached.
//=============================================================================
int wxFSVolumeBase::GetFlags() const
{
    if (!m_isOk)
        return -1;

    FileInfoMap::iterator itr = s_fileInfo.find(m_volName);
    if (itr == s_fileInfo.end())
        return -1;

    return itr->second.m_flags;
} // GetFlags

#endif // wxUSE_BASE

// ============================================================================
// wxFSVolume
// ============================================================================

#if wxUSE_GUI

void wxFSVolume::InitIcons()
{
    m_icons.Alloc(wxFS_VOL_ICO_MAX);
    wxIcon null;
    for (int idx = 0; idx < wxFS_VOL_ICO_MAX; idx++)
        m_icons.Add(null);
}

//=============================================================================
// Function: GetIcon
// Purpose: return the requested icon.
//=============================================================================

wxIcon wxFSVolume::GetIcon(wxFSIconType type) const
{
    wxCHECK_MSG( type >= 0 && (size_t)type < m_icons.GetCount(), wxNullIcon,
                 "wxFSIconType::GetIcon(): invalid icon index" );

#ifdef __WXMSW__
    // Load on demand.
    if (m_icons[type].IsNull())
    {
        UINT flags = SHGFI_ICON;
        switch (type)
        {
        case wxFS_VOL_ICO_SMALL:
            flags |= SHGFI_SMALLICON;
            break;

        case wxFS_VOL_ICO_LARGE:
            flags |= SHGFI_SHELLICONSIZE;
            break;

        case wxFS_VOL_ICO_SEL_SMALL:
            flags |= SHGFI_SMALLICON | SHGFI_OPENICON;
            break;

        case wxFS_VOL_ICO_SEL_LARGE:
            flags |= SHGFI_SHELLICONSIZE | SHGFI_OPENICON;
            break;

        case wxFS_VOL_ICO_MAX:
            wxFAIL_MSG("wxFS_VOL_ICO_MAX is not valid icon type");
            break;
        }

        SHFILEINFO fi;
        long rc = SHGetFileInfo(m_volName.t_str(), 0, &fi, sizeof(fi), flags);
        if (!rc || !fi.hIcon)
        {
            wxLogError(_("Cannot load icon from '%s'."), m_volName.c_str());
        }
        else
        {
            m_icons[type].CreateFromHICON((WXHICON)fi.hIcon);
        }
    }

    return m_icons[type];
#else
    wxFAIL_MSG("Can't convert WXHICON to wxIcon in this port.");
    return wxNullIcon;
#endif
} // GetIcon

#endif // wxUSE_GUI

#endif // wxUSE_FSVOLUME
