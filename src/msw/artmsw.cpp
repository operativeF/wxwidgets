/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/artmsw.cpp
// Purpose:     stock wxArtProvider instance with native MSW stock icons
// Author:      Vaclav Slavik
// Modified by:
// Created:     2008-10-15
// Copyright:   (c) Vaclav Slavik, 2008
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/artprov.h"

#include "wx/app.h"
#include "wx/dynlib.h"
#include "wx/volume.h"
#include "wx/msw/wrapshl.h"

#include "wx/msw/private.h"

#include <boost/nowide/convert.hpp>

import WX.WinDef;

import <string>;
import <vector>;

#ifdef SHGSI_ICON
    #define wxHAS_SHGetStockIconInfo
#endif

namespace
{

#ifdef SHDefExtractIcon
    #define MSW_SHDefExtractIconW SHDefExtractIconW
#else // !defined(SHDefExtractIcon)

// MinGW doesn't provide SHDefExtractIcon() up to at least the 5.3 version, so
// define it ourselves.
HRESULT
MSW_SHDefExtractIcon(LPCTSTR pszIconFile, int iIndex, WXUINT uFlags,
                     WXHICON *phiconLarge, WXHICON *phiconSmall, WXUINT nIconSize)
{
    typedef HRESULT
    (WINAPI *SHDefExtractIcon_t)(LPCTSTR, int, WXUINT, WXHICON*, WXHICON*, WXUINT);

    static SHDefExtractIcon_t s_SHDefExtractIcon = NULL;
    if ( !s_SHDefExtractIcon )
    {
        wxDynamicLibrary shell32("shell32.dll");
        wxDL_INIT_FUNC_AW(s_, SHDefExtractIcon, shell32);

        if ( !s_SHDefExtractIcon )
            return E_FAIL;

        // Prevent the DLL from being unloaded while we use its function.
        // Normally it's not a problem as shell32.dll is always loaded anyhow.
        shell32.Detach();
    }

    return (*s_SHDefExtractIcon)(pszIconFile, iIndex, uFlags,
                                 phiconLarge, phiconSmall, nIconSize);
}

#endif // !defined(SHDefExtractIcon)

#ifdef wxHAS_SHGetStockIconInfo

SHSTOCKICONID MSWGetStockIconIdForArtProviderId(const wxArtID& art_id)
{
    // try to find an equivalent MSW stock icon id for wxArtID
    if ( art_id == wxART_ERROR)             return SIID_ERROR;
    else if ( art_id == wxART_QUESTION )    return SIID_HELP;
    else if ( art_id == wxART_WARNING )     return SIID_WARNING;
    else if ( art_id == wxART_INFORMATION ) return SIID_INFO;
    else if ( art_id == wxART_HELP )        return SIID_HELP;
    else if ( art_id == wxART_FOLDER )      return SIID_FOLDER;
    else if ( art_id == wxART_FOLDER_OPEN ) return SIID_FOLDEROPEN;
    else if ( art_id == wxART_DELETE )      return SIID_DELETE;
    else if ( art_id == wxART_FIND )        return SIID_FIND;
    else if ( art_id == wxART_HARDDISK )    return SIID_DRIVEFIXED;
    else if ( art_id == wxART_FLOPPY )      return SIID_DRIVE35;
    else if ( art_id == wxART_CDROM )       return SIID_DRIVECD;
    else if ( art_id == wxART_REMOVABLE )   return SIID_DRIVEREMOVE;
    else if ( art_id == wxART_PRINT )       return SIID_PRINTER;
    else if ( art_id == wxART_EXECUTABLE_FILE ) return SIID_APPLICATION;
    else if ( art_id == wxART_NORMAL_FILE ) return SIID_DOCNOASSOC;

    return SIID_INVALID;
};


// try to load SHGetStockIconInfo dynamically, so this code runs
// even on pre-Vista Windows versions
HRESULT
MSW_SHGetStockIconInfo(SHSTOCKICONID siid,
                       WXUINT uFlags,
                       SHSTOCKICONINFO *psii)
{
    using PSHGETSTOCKICONINFO = HRESULT (WINAPI*)(SHSTOCKICONID, WXUINT, SHSTOCKICONINFO *);
    static PSHGETSTOCKICONINFO pSHGetStockIconInfo = (PSHGETSTOCKICONINFO)-1;

    if ( pSHGetStockIconInfo == (PSHGETSTOCKICONINFO)-1 )
    {
        wxDynamicLibrary shell32("shell32.dll");

        pSHGetStockIconInfo = (PSHGETSTOCKICONINFO)shell32.RawGetSymbol( "SHGetStockIconInfo" );
    }

    if ( !pSHGetStockIconInfo )
        return E_FAIL;

    return pSHGetStockIconInfo(siid, uFlags, psii);
}

#endif // #ifdef wxHAS_SHGetStockIconInfo

// Wrapper for SHDefExtractIcon().
wxBitmap
MSWGetBitmapFromIconLocation(const WCHAR* path, int index, const wxSize& size)
{
    WXHICON hIcon = nullptr;
    if ( MSW_SHDefExtractIconW(path, index, 0, &hIcon, nullptr, size.x) != S_OK )
        return wxNullBitmap;

    // Note that using "size.x" twice here is not a typo: normally size.y is
    // the same anyhow, of course, but if it isn't, the actual icon size would
    // be size.x in both directions as we only pass "x" to SHDefExtractIcon()
    // above.
    wxIcon icon;
    if ( !icon.InitFromHICON((WXHICON)hIcon, size) )
        return wxNullBitmap;

    return {icon};
}

wxBitmap
MSWGetBitmapForPath(const std::string& path, const wxSize& size, WXDWORD uFlags = 0)
{
    SHFILEINFOW fi;
    wxZeroMemory(fi);

    uFlags |= SHGFI_USEFILEATTRIBUTES | SHGFI_ICONLOCATION;

    if ( !::SHGetFileInfoW(boost::nowide::widen(path).c_str(),
                           FILE_ATTRIBUTE_DIRECTORY,
                           &fi,
                           sizeof(SHFILEINFO),
                           uFlags) )
       return wxNullBitmap;

    return MSWGetBitmapFromIconLocation(fi.szDisplayName, fi.iIcon, size);
}

#if wxUSE_FSVOLUME

wxBitmap
GetDriveBitmapForVolumeType(const wxFSVolumeKind& volKind, const wxSize& size)
{
    // get all volumes and try to find one with a matching type
    std::vector<std::string> volumes = wxFSVolume::GetVolumes();
    for ( const auto& volume : volumes )
    {
        wxFSVolume vol( volume );
        if ( vol.GetKind() == volKind )
        {
            return MSWGetBitmapForPath(volume, size);
        }
    }

    return wxNullBitmap;
}

#endif // wxUSE_FSVOLUME

} // anonymous namespace

// ----------------------------------------------------------------------------
// wxWindowsArtProvider
// ----------------------------------------------------------------------------

class wxWindowsArtProvider : public wxArtProvider
{
protected:
    wxBitmap CreateBitmap(const wxArtID& id, const wxArtClient& client,
                                  const wxSize& size) override;
};

static wxBitmap CreateFromStdIcon(const char *iconName,
                                  const wxArtClient& client)
{
    wxIcon icon(iconName);
    wxBitmap bmp;
    bmp.CopyFromIcon(icon);

    // The standard native message box icons are in message box size (32x32).
    // If they are requested in any size other than the default or message
    // box size, they must be rescaled first.
    if ( client != wxART_MESSAGE_BOX && client != wxART_OTHER )
    {
        const wxSize size = wxArtProvider::GetNativeSizeHint(client);
        if ( size != wxDefaultSize )
        {
            wxArtProvider::RescaleBitmap(bmp, size);
        }
    }

    return bmp;
}

wxBitmap wxWindowsArtProvider::CreateBitmap(const wxArtID& id,
                                            const wxArtClient& client,
                                            const wxSize& size)
{
    wxBitmap bitmap;

#ifdef wxHAS_SHGetStockIconInfo
    // first try to use SHGetStockIconInfo, available only on Vista and higher
    SHSTOCKICONID stockIconId = MSWGetStockIconIdForArtProviderId( id );
    if ( stockIconId != SIID_INVALID )
    {
        WinStruct<SHSTOCKICONINFO> sii;

        WXUINT uFlags = SHGSI_ICONLOCATION | SHGSI_SYSICONINDEX;

        HRESULT res = MSW_SHGetStockIconInfo(stockIconId, uFlags, &sii);
        if ( res == S_OK )
        {
            const wxSize
                sizeNeeded = size.IsFullySpecified()
                                ? size
                                : wxArtProvider::GetNativeSizeHint(client);

            bitmap = MSWGetBitmapFromIconLocation(sii.szPath, sii.iIcon,
                                                  sizeNeeded);
            if ( bitmap.IsOk() )
            {
                if ( bitmap.GetSize() != sizeNeeded )
                {
                    wxArtProvider::RescaleBitmap(bitmap, sizeNeeded);
                }

                return bitmap;
            }
        }
    }
#endif // wxHAS_SHGetStockIconInfo


#if wxUSE_FSVOLUME
    // now try SHGetFileInfo
    wxFSVolumeKind volKind = wxFSVolumeKind::Other;
    if ( id == wxART_HARDDISK )
        volKind = wxFSVolumeKind::Disk;
    else if ( id == wxART_FLOPPY )
        volKind = wxFSVolumeKind::Floppy;
    else if ( id == wxART_CDROM )
        volKind = wxFSVolumeKind::CDROM;

    if ( volKind != wxFSVolumeKind::Other )
    {
        bitmap = GetDriveBitmapForVolumeType(volKind, size);
        if ( bitmap.IsOk() )
            return bitmap;
    }
#endif // wxUSE_FSVOLUME

    // notice that the directory used here doesn't need to exist
    if ( id == wxART_FOLDER )
        bitmap = MSWGetBitmapForPath("C:\\wxdummydir\\", size );
    else if ( id == wxART_FOLDER_OPEN )
        bitmap = MSWGetBitmapForPath("C:\\wxdummydir\\", size, SHGFI_OPENICON );

    if ( !bitmap.IsOk() )
    {
        // handle message box icons specially (wxIcon ctor treat these names
        // as special cases via wxICOResourceHandler::LoadIcon):
        const char *name = nullptr;
        if ( id == wxART_ERROR )
            name = "wxICON_ERROR";
        else if ( id == wxART_INFORMATION )
            name = "wxICON_INFORMATION";
        else if ( id == wxART_WARNING )
            name = "wxICON_WARNING";
        else if ( id == wxART_QUESTION )
            name = "wxICON_QUESTION";

        if ( name )
            return CreateFromStdIcon(name, client);
    }

    // for anything else, fall back to generic provider:
    return bitmap;
}

// ----------------------------------------------------------------------------
// wxArtProvider::InitNativeProvider()
// ----------------------------------------------------------------------------

/*static*/ void wxArtProvider::InitNativeProvider()
{
    PushBack(new wxWindowsArtProvider);
}

// ----------------------------------------------------------------------------
// wxArtProvider::GetNativeSizeHint()
// ----------------------------------------------------------------------------

/*static*/
wxSize wxArtProvider::GetNativeSizeHint(const wxArtClient& client)
{
    const wxWindow* win = wxApp::GetMainTopWindow();
    if ( client == wxART_TOOLBAR )
    {
        return wxWindow::FromDIP(wxSize(24, 24), win);
    }
    else if ( client == wxART_MENU )
    {
        return wxWindow::FromDIP(wxSize(16, 16), win);
    }
    else if ( client == wxART_FRAME_ICON )
    {
        return {wxGetSystemMetrics(SM_CXSMICON, win),
                wxGetSystemMetrics(SM_CYSMICON, win)};
    }
    else if ( client == wxART_CMN_DIALOG ||
              client == wxART_MESSAGE_BOX )
    {
        return {wxGetSystemMetrics(SM_CXICON, win),
                wxGetSystemMetrics(SM_CYICON, win)};
    }
    else if (client == wxART_BUTTON)
    {
        return wxWindow::FromDIP(wxSize(16, 16), win);
    }
    else if (client == wxART_LIST)
    {
        return wxWindow::FromDIP(wxSize(16, 16), win);
    }

    return wxDefaultSize;
}
