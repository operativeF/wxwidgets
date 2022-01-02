/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/filedlg.cpp
// Purpose:     wxFileDialog
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_FILEDLG

#include "wx/filedlg.h"
#include "wx/msw/wrapcdlg.h"

#include "wx/utils.h"
#include "wx/filefn.h"
#include "wx/intl.h"
#include "wx/log.h"

#include "wx/dynlib.h"

#include "wx/scopeguard.h"
#include "wx/modalhook.h"
#include "wx/msw/private/dpiaware.h"

#include <memory>

import WX.WinDef;
import WX.File.Filename;

import Utils.Strings;
import Utils.Geometry;

import <algorithm>;
import <string>;
import <tuple>;
import <vector>;


// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

constexpr int wxMAXPATH =   65534;
constexpr int wxMAXFILE =   1024;
constexpr int wxMAXEXT =    5;

// ----------------------------------------------------------------------------
// globals
// ----------------------------------------------------------------------------

// standard dialog size for the old Windows systems where the dialog wasn't
// resizable
static wxRect gs_rectDialog(0, 0, 428, 266);

// ----------------------------------------------------------------------------

namespace
{

#if wxUSE_DYNLIB_CLASS

using GetProcessUserModeExceptionPolicy_t = BOOL (WINAPI*)(LPDWORD);
using SetProcessUserModeExceptionPolicy_t = BOOL (WINAPI*)(WXDWORD);

GetProcessUserModeExceptionPolicy_t gs_pfnGetProcessUserModeExceptionPolicy =
    (GetProcessUserModeExceptionPolicy_t) -1;

SetProcessUserModeExceptionPolicy_t gs_pfnSetProcessUserModeExceptionPolicy =
    (SetProcessUserModeExceptionPolicy_t) -1;

WXDWORD gs_oldExceptionPolicyFlags = 0;

bool gs_changedPolicy = false;

#endif // #if wxUSE_DYNLIB_CLASS

/*
Since Windows 7 by default (callback) exceptions aren't swallowed anymore
with native x64 applications. Exceptions can occur in a file dialog when
using the hook procedure in combination with third-party utilities.
Since Windows 7 SP1 the swallowing of exceptions can be enabled again
by using SetProcessUserModeExceptionPolicy.
*/
void ChangeExceptionPolicy()
{
#if wxUSE_DYNLIB_CLASS
    gs_changedPolicy = false;

    wxLoadedDLL dllKernel32("kernel32.dll");

    if ( gs_pfnGetProcessUserModeExceptionPolicy
        == (GetProcessUserModeExceptionPolicy_t) -1)
    {
        wxDL_INIT_FUNC(gs_pfn, GetProcessUserModeExceptionPolicy, dllKernel32);
        wxDL_INIT_FUNC(gs_pfn, SetProcessUserModeExceptionPolicy, dllKernel32);
    }

    if ( !gs_pfnGetProcessUserModeExceptionPolicy
        || !gs_pfnSetProcessUserModeExceptionPolicy
        || !gs_pfnGetProcessUserModeExceptionPolicy(&gs_oldExceptionPolicyFlags) )
    {
        return;
    }

    if ( gs_pfnSetProcessUserModeExceptionPolicy(gs_oldExceptionPolicyFlags
        | 0x1 /* PROCESS_CALLBACK_FILTER_ENABLED */ ) )
    {
        gs_changedPolicy = true;
    }

#endif // wxUSE_DYNLIB_CLASS
}

void RestoreExceptionPolicy()
{
#if wxUSE_DYNLIB_CLASS
    if (gs_changedPolicy)
    {
        gs_changedPolicy = false;
        std::ignore = gs_pfnSetProcessUserModeExceptionPolicy(gs_oldExceptionPolicyFlags);
    }
#endif // wxUSE_DYNLIB_CLASS
}

} // unnamed namespace

// ----------------------------------------------------------------------------
// hook function for moving the dialog
// ----------------------------------------------------------------------------

UINT_PTR APIENTRY
wxFileDialogHookFunction(WXHWND      hDlg,
                         WXUINT      iMsg,
                         [[maybe_unused]] WXWPARAM    wParam,
                         WXLPARAM    lParam)
{
    switch ( iMsg )
    {
        case WM_INITDIALOG:
            {
                OPENFILENAME* ofn = reinterpret_cast<OPENFILENAME *>(lParam);
                reinterpret_cast<wxFileDialog *>(ofn->lCustData)
                    ->MSWOnInitDialogHook((WXHWND)hDlg);
            }
            break;

        case WM_NOTIFY:
            {
                NMHDR* const pNM = reinterpret_cast<NMHDR*>(lParam);
                if ( pNM->code > CDN_LAST && pNM->code <= CDN_FIRST )
                {
                    OFNOTIFY* const
                        pNotifyCode = reinterpret_cast<OFNOTIFY *>(lParam);
                    wxFileDialog* const
                        dialog = reinterpret_cast<wxFileDialog *>(
                                        pNotifyCode->lpOFN->lCustData
                                    );

                    switch ( pNotifyCode->hdr.code )
                    {
                        case CDN_INITDONE:
                            dialog->MSWOnInitDone((WXHWND)hDlg);
                            break;

                        case CDN_SELCHANGE:
                            dialog->MSWOnSelChange((WXHWND)hDlg);
                            break;

                        case CDN_TYPECHANGE:
                            dialog->MSWOnTypeChange
                                    (
                                        (WXHWND)hDlg,
                                        pNotifyCode->lpOFN->nFilterIndex
                                    );
                            break;
                    }
                }
            }
            break;

        case WM_DESTROY:
            // reuse the position used for the dialog the next time by default
            //
            // NB: at least under Windows 2003 this is useless as after the
            //     first time it's shown the dialog always remembers its size
            //     and position itself and ignores any later SetWindowPos calls
            wxCopyRECTToRect(wxGetWindowRect(::GetParent(hDlg)), gs_rectDialog);
            break;
    }

    // do the default processing
    return 0;
}

// ----------------------------------------------------------------------------
// wxFileDialog
// ----------------------------------------------------------------------------

wxFileDialog::wxFileDialog(wxWindow *parent,
                           const std::string& message,
                           const std::string& defaultDir,
                           const std::string& defaultFileName,
                           const std::string& wildCard,
                           unsigned int style,
                           const wxPoint& pos,
                           const wxSize& sz,
                           const std::string& name)
            : wxFileDialogBase(parent, message, defaultDir, defaultFileName,
                               wildCard, style, pos, sz, name)

{
    // NB: all style checks are done by wxFileDialogBase::Create

    m_centreDir = 0;

    // Must set to zero, otherwise the wx routines won't size the window
    // the second time you call the file dialog, because it thinks it is
    // already at the requested size.. (when centering)
    gs_rectDialog.x =
    gs_rectDialog.y = 0;
}

std::vector<std::string> wxFileDialog::GetPaths() const
{
    std::vector<std::string> paths;

    std::string dir(m_dir);
    if ( m_dir.empty() || m_dir.back() != '\\' )
        dir += '\\';

    for ( const auto& filename : m_fileNames )
    {
        if (wxFileName(filename).IsAbsolute())
            paths.push_back(filename);
        else
            paths.push_back(dir + filename);
    }

    return paths;
}

std::vector<std::string> wxFileDialog::GetFilenames() const
{
    return m_fileNames;
}

wxPoint wxFileDialog::DoGetPosition() const
{
    return {gs_rectDialog.x, gs_rectDialog.y};
}

wxSize wxFileDialog::DoGetSize() const
{
    return { gs_rectDialog.width, gs_rectDialog.height };
}

void wxFileDialog::DoMoveWindow(wxRect boundary)
{
    gs_rectDialog.x = boundary.x;
    gs_rectDialog.y = boundary.y;

    // our WXHWND is only set when we're called from MSWOnInitDone(), test if
    // this is the case
    WXHWND hwnd = GetHwnd();
    if ( hwnd )
    {
        // size of the dialog can't be changed because the controls are not
        // laid out correctly then
       ::SetWindowPos(hwnd, HWND_TOP, boundary.x, boundary.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }
    else // just remember that we were requested to move the window
    {
        m_bMovedWindow = true;

        // if Centre() had been called before, it shouldn't be taken into
        // account now
        m_centreDir = 0;
    }
}

void wxFileDialog::DoCentre(unsigned int dir)
{
    m_centreDir = dir;
    m_bMovedWindow = true;

    // it's unnecessary to do anything else at this stage as we'll redo it in
    // MSWOnInitDone() anyhow
}

void wxFileDialog::MSWOnInitDone(WXHWND hDlg)
{
    // note the dialog is the parent window: hDlg is a child of it when
    // OFN_EXPLORER is used
    WXHWND hFileDlg = ::GetParent((WXHWND)hDlg);

    // set WXHWND so that our DoMoveWindow() works correctly
    TempHWNDSetter set(this, (WXHWND)hFileDlg);

    if ( m_centreDir )
    {
        // now we have the real dialog size, remember it
        RECT rect;
        GetWindowRect(hFileDlg, &rect);
        gs_rectDialog = wxRectFromRECT(rect);

        // and position the window correctly: notice that we must use the base
        // class version as our own doesn't do anything except setting flags
        wxFileDialogBase::DoCentre(m_centreDir);
    }
    else // need to just move it to the correct place
    {
        SetPosition(gs_rectDialog.GetPosition());
    }

    // Call selection change handler so that update handler will be
    // called once with no selection.
    MSWOnSelChange(hDlg);
}

void wxFileDialog::MSWOnSelChange(WXHWND hDlg)
{
    boost::nowide::wstackstring buf;

    LRESULT len = ::SendMessageW(::GetParent(hDlg), CDM_GETFILEPATH,
                              MAX_PATH, reinterpret_cast<WXLPARAM>(buf.get()));

    if ( len > 0 )
        m_currentlySelectedFilename = boost::nowide::narrow(buf.get());
    else
        m_currentlySelectedFilename.clear();

    UpdateExtraControlUI();
}

void wxFileDialog::MSWOnTypeChange([[maybe_unused]] WXHWND hDlg, int nFilterIndex)
{
    // Filter indices are 1-based, while we want to use 0-based index, as
    // usual. However the input index can apparently also be 0 in some
    // circumstances, so take care before decrementing it.
    m_currentlySelectedFilterIndex = nFilterIndex ? nFilterIndex - 1 : 0;

    UpdateExtraControlUI();
}

// helper used below in ShowCommFileDialog(): style is used to determine
// whether to show the "Save file" dialog (if it contains wxFD_SAVE bit) or
// "Open file" one; returns true on success or false on failure in which case
// err is filled with the CDERR_XXX constant
static bool DoShowCommFileDialog(OPENFILENAMEW *of, unsigned int style, WXDWORD *err)
{
    // Extra controls do not handle per-monitor DPI, fall back to system DPI
    // so entire file-dialog is resized.
    std::unique_ptr<wxMSWImpl::AutoSystemDpiAware> dpiAwareness;
    if ( of->Flags & OFN_ENABLEHOOK )
        dpiAwareness.reset(new wxMSWImpl::AutoSystemDpiAware());

    if ( style & wxFD_SAVE ? ::GetSaveFileNameW(of) : ::GetOpenFileNameW(of) )
        return true;

    if ( err )
    {
        *err = CommDlgExtendedError();
    }

    return false;
}

static bool ShowCommFileDialog(OPENFILENAMEW *of, unsigned int style)
{
    WXDWORD errCode;
    bool success = DoShowCommFileDialog(of, style, &errCode);

    if ( !success &&
            errCode == FNERR_INVALIDFILENAME &&
                of->lpstrFile[0] )
    {
        // this can happen if the default file name is invalid, try without it
        // now
        of->lpstrFile[0] = wxT('\0');
        success = DoShowCommFileDialog(of, style, &errCode);
    }

    if ( !success )
    {
        // common dialog failed - why?
        if ( errCode != 0 )
        {
            wxLogError(_("File dialog failed with error code %0lx."), errCode);
        }
        //else: it was just cancelled

        return false;
    }

    return true;
}

void wxFileDialog::MSWOnInitDialogHook(WXHWND hwnd)
{
    TempHWNDSetter set(this, hwnd);

    CreateExtraControl();
}

int wxFileDialog::ShowModal()
{
    WX_HOOK_MODAL_DIALOG();

    wxWindow* const parent = GetParentForModalDialog(m_parent, wxGetWindowStyle());
    WXHWND hWndParent = parent ? GetHwndOf(parent) : nullptr;

    std::string fileNameBuffer; // the filename
    fileNameBuffer.resize(wxMAXPATH);

    std::string titleBuffer;
    titleBuffer.resize(wxMAXFILE + wxMAXEXT + 1); // the file-name, without path

    unsigned int msw_flags = OFN_HIDEREADONLY;

    if ( HasFdFlag(wxFD_NO_FOLLOW) )
        msw_flags |= OFN_NODEREFERENCELINKS;

    if ( HasFdFlag(wxFD_FILE_MUST_EXIST) )
        msw_flags |= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if ( HasFlag(wxFD_SHOW_HIDDEN) )
        msw_flags |= OFN_FORCESHOWHIDDEN;
    /*
        If the window has been moved the programmer is probably
        trying to center or position it.  Thus we set the callback
        or hook function so that we can actually adjust the position.
        Without moving or centering the dlg, it will just stay
        in the upper left of the frame, it does not center
        automatically.
    */
    if (m_bMovedWindow || HasExtraControlCreator()) // we need these flags.
    {
        ChangeExceptionPolicy();
        msw_flags |= OFN_EXPLORER|OFN_ENABLEHOOK;
        msw_flags |= OFN_ENABLESIZING;
    }

    wxON_BLOCK_EXIT0(RestoreExceptionPolicy);

    if ( HasFdFlag(wxFD_MULTIPLE) )
    {
        // OFN_EXPLORER must always be specified with OFN_ALLOWMULTISELECT
        msw_flags |= OFN_EXPLORER | OFN_ALLOWMULTISELECT;
    }

    // if wxFD_CHANGE_DIR flag is not given we shouldn't change the CWD which the
    // standard dialog does by default (notice that under NT it does it anyhow,
    // OFN_NOCHANGEDIR or not, see below)
    if ( !HasFdFlag(wxFD_CHANGE_DIR) )
    {
        msw_flags |= OFN_NOCHANGEDIR;
    }

    if ( HasFdFlag(wxFD_OVERWRITE_PROMPT) )
    {
        msw_flags |= OFN_OVERWRITEPROMPT;
    }

    OPENFILENAMEW of;
    wxZeroMemory(of);

    boost::nowide::wstackstring stackTitle{m_message.c_str()};
    boost::nowide::wstackstring stackTitleBuffer{titleBuffer.c_str()};
    of.lStructSize       = sizeof(OPENFILENAMEW);
    of.hwndOwner         = hWndParent;
    of.lpstrTitle        = stackTitle.get();
    of.lpstrFileTitle    = stackTitleBuffer.get();
    of.nMaxFileTitle     = wxMAXFILE + 1 + wxMAXEXT;

    GlobalPtr hgbl;
    if ( HasExtraControlCreator() )
    {
        msw_flags |= OFN_ENABLETEMPLATEHANDLE;

        hgbl.Init(256, GMEM_ZEROINIT);
        GlobalPtrLock hgblLock(hgbl);
        LPDLGTEMPLATE lpdt = static_cast<LPDLGTEMPLATE>(hgblLock.Get());

        // Define a dialog box.

        lpdt->style = DS_CONTROL | WS_CHILD | WS_CLIPSIBLINGS;
        lpdt->cdit = 0;         // Number of controls
        lpdt->x = 0;
        lpdt->y = 0;

        // convert the size of the extra controls to the dialog units
        const wxSize extraSize = GetExtraControlSize();
        const LONG baseUnits = ::GetDialogBaseUnits();
        lpdt->cx = ::MulDiv(extraSize.x, 4, LOWORD(baseUnits));
        lpdt->cy = ::MulDiv(extraSize.y, 8, HIWORD(baseUnits));

        // after the DLGTEMPLATE there are 3 additional WORDs for dialog menu,
        // class and title, all three set to zeros.

        of.hInstance = (WXHINSTANCE)lpdt;
    }

    // Convert forward slashes to backslashes (file selector doesn't like
    // forward slashes) and also squeeze multiple consecutive slashes into one
    // as it doesn't like two backslashes in a row neither

    std::string  dir;
    size_t    len = m_dir.length();
    dir.reserve(len);
    for (size_t i = 0; i < len; i++ )
    {
        wxChar ch = m_dir[i];
        switch ( ch )
        {
            case wxT('/'):
                // convert to backslash
                ch = wxT('\\');
                [[fallthrough]];

            case wxT('\\'):
                while ( i < len - 1 )
                {
                    wxChar chNext = m_dir[i + 1];
                    if ( chNext != wxT('\\') && chNext != wxT('/') )
                        break;

                    // ignore the next one, unless it is at the start of a UNC path
                    if (i > 0)
                        i++;
                    else
                        break;
                }
                [[fallthrough]];

            default:
                // normal char
                dir += ch;
        }
    }

    of.lpstrInitialDir   = boost::nowide::widen(dir).c_str();

    of.Flags             = msw_flags;
    of.lpfnHook          = wxFileDialogHookFunction;
    of.lCustData         = (WXLPARAM)this;

    std::vector<std::string> wildDescriptions;
    std::vector<std::string> wildFilters;

    size_t items = wxParseCommonDialogsFilter(m_wildCard, wildDescriptions, wildFilters);

    wxASSERT_MSG( items > 0 , "empty wildcard list" );

    std::string filterBuffer;

    for (size_t i = 0; i < items ; i++)
    {
        filterBuffer += fmt::format("{}|{}|", wildDescriptions[i], wildFilters[i]);
    }

    std::replace(filterBuffer.begin(), filterBuffer.end(), '|', '\0');

    boost::nowide::wstackstring stackFilterBuffer{filterBuffer.c_str()};

    of.lpstrFilter  = stackFilterBuffer.get();
    of.nFilterIndex = m_filterIndex + 1;
    m_currentlySelectedFilterIndex = m_filterIndex;

    //=== Setting defaultFileName >>=========================================

    fileNameBuffer = m_fileName;

    boost::nowide::wstackstring stackFileNameBuf{fileNameBuffer.c_str()};
    of.lpstrFile = stackFileNameBuf.get();  // holds returned filename
    of.nMaxFile  = wxMAXPATH;

    // we must set the default extension because otherwise Windows would check
    // for the existing of a wrong file with wxFD_OVERWRITE_PROMPT (i.e. if the
    // user types "foo" and the default extension is ".bar" we should force it
    // to check for "foo.bar" existence and not "foo")
    std::string defextBuffer; // we need it to be alive until GetSaveFileName()!
    
    if (HasFdFlag(wxFD_SAVE))
    {
        const char* extension = filterBuffer.c_str();
        int maxFilter = (int)(of.nFilterIndex*2L) - 1;

        for( int j = 0; j < maxFilter; j++ )           // get extension
            extension = extension + wxStrlen( extension ) + 1;

        // use dummy name a to avoid assert in AppendExtension
        defextBuffer = AppendExtension("a", extension);
        if (defextBuffer.rfind("a.", 0) == 0)
        {
            defextBuffer = defextBuffer.substr(2); // remove "a."
            of.lpstrDefExt = boost::nowide::widen(defextBuffer).c_str();
        }
    }

    // Create a temporary struct to restore the CWD when we exit this function
    // store off before the standard windows dialog can possibly change it
    struct CwdRestore
    {
        std::string value;
        ~CwdRestore()
        {
            if (!value.empty())
                wxSetWorkingDirectory(value);
        }
    } cwdOrig;

    // GetOpenFileName will always change the current working directory
    // (according to MSDN) because the flag OFN_NOCHANGEDIR has no effect.
    // If the user did not specify wxFD_CHANGE_DIR let's restore the
    // current working directory to what it was before the dialog was shown.
    if (msw_flags & OFN_NOCHANGEDIR)
        cwdOrig.value = wxGetCwd();

    //== Execute FileDialog >>=================================================

    if ( !::ShowCommFileDialog(&of, m_windowStyle) )
        return wxID_CANCEL;

    m_fileNames.clear();

    if ( ( HasFdFlag(wxFD_MULTIPLE) ) &&
         ( fileNameBuffer[of.nFileOffset-1] == '\0' )
       )
    {
        m_dir = fileNameBuffer;
        auto i = of.nFileOffset;
        m_fileName = &fileNameBuffer[i];
        m_fileNames.push_back(m_fileName);
        i += m_fileName.length() + 1;

        while (fileNameBuffer[i] != '\0')
        {
            m_fileNames.push_back(fileNameBuffer);
            i += wxStrlen(&fileNameBuffer[i]) + 1;
        }

        m_path = m_dir;
        if ( m_dir.back() != '\\' )
            m_path += '\\';

        m_path += m_fileName;
        m_filterIndex = (int)of.nFilterIndex - 1;
    }
    else
    {
        //=== Adding the correct extension >>=================================

        m_filterIndex = (int)of.nFilterIndex - 1;

        if ( !of.nFileExtension || fileNameBuffer[of.nFileExtension] == '\0' )
        {
            // User has typed a filename without an extension:
            const char* extension = filterBuffer.c_str();
            int   maxFilter = (int)(of.nFilterIndex*2L) - 1;

            for( int j = 0; j < maxFilter; j++ )           // get extension
                extension = extension + wxStrlen( extension ) + 1;

            m_fileName = AppendExtension(fileNameBuffer, extension);
            fileNameBuffer = m_fileName;
        }

        m_path = fileNameBuffer;
        m_fileName = wxFileNameFromPath(fileNameBuffer);
        m_fileNames.push_back(m_fileName);
        m_dir = wxPathOnly(fileNameBuffer);
    }

    return wxID_OK;

}

#endif // wxUSE_FILEDLG
