/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/fldlgcmn.cpp
// Purpose:     wxFileDialog common functions
// Author:      John Labenski
// Modified by:
// Created:     14.06.03 (extracted from src/*/filedlg.cpp)
// Copyright:   (c) Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_FILEDLG

#include "wx/filedlg.h"
#include "wx/filefn.h"
#include "wx/dirdlg.h"
#include "wx/intl.h"
#include "wx/window.h"

import Utils.Strings;
import WX.File.Filename;

import <string>;

//----------------------------------------------------------------------------
// wxFileDialogBase
//----------------------------------------------------------------------------

bool wxFileDialogBase::Create(wxWindow *parent,
                              const std::string& message,
                              const std::string& defaultDir,
                              const std::string& defaultFile,
                              const std::string& wildCard,
                              unsigned int style,
                              [[maybe_unused]] const wxPoint& pos,
                              [[maybe_unused]] const wxSize& sz,
                              [[maybe_unused]] const std::string& name)
{
    m_message = message;
    m_dir = defaultDir;
    m_fileName = defaultFile;
    m_wildCard = wildCard;

    m_parent = parent;

#ifdef __WXOSX__
    /*
    [DS]
    Remove the (for OS X unnecessary) wxFD_FILE_MUST_EXIST flag. Using it
    causes problems when having an extra panel (either a custom one or
    by showing the filetype filters through calling
    wxSystemOptions::SetOption(wxOSX_FILEDIALOG_ALWAYS_SHOW_TYPES, 1) ).
    Presumably the style flag conflicts with other style flags and an
    assert in wxRegion::DoOffset is triggered later on.
    Another solution was to override GetWindowStyleFlag() to not include
    wxFD_FILE_MUST_EXIST in its return value, but as other wxFileDialog
    style flags (that are actually used) dont't seem to cause problems
    this seemed an easier solution.
    */
    style &= ~wxFD_FILE_MUST_EXIST;
#endif

    m_windowStyle = style;

    if (!HasFdFlag(wxFD_OPEN) && !HasFdFlag(wxFD_SAVE))
        m_windowStyle |= wxFD_OPEN;     // wxFD_OPEN is the default

    // check that the styles are not contradictory
    wxASSERT_MSG( !(HasFdFlag(wxFD_SAVE) && HasFdFlag(wxFD_OPEN)),
                  "can't specify both wxFD_SAVE and wxFD_OPEN at once" );

    wxASSERT_MSG( !HasFdFlag(wxFD_SAVE) ||
                    (!HasFdFlag(wxFD_MULTIPLE) && !HasFdFlag(wxFD_FILE_MUST_EXIST)),
                   "wxFD_MULTIPLE or wxFD_FILE_MUST_EXIST can't be used with wxFD_SAVE" );

    wxASSERT_MSG( !HasFdFlag(wxFD_OPEN) || !HasFdFlag(wxFD_OVERWRITE_PROMPT),
                  "wxFD_OVERWRITE_PROMPT can't be used with wxFD_OPEN" );

    if ( wildCard.empty() || wildCard == wxFileSelectorDefaultWildcardStr )
    {
        // FIXME: Translation removed for fmt lib
        m_wildCard = fmt::format("All files (%s)|%s",
                                      wxFileSelectorDefaultWildcardStr,
                                      wxFileSelectorDefaultWildcardStr);
    }
    else // have wild card
    {
        // convert m_wildCard from "*.bar" to "bar files (*.bar)|*.bar"
        if ( m_wildCard.find('|') == std::string::npos )
        {
            std::string::size_type nDot = m_wildCard.find("*.");
            if ( nDot != std::string::npos )
                nDot++;
            else
                nDot = 0;

            m_wildCard = fmt::format
                         (// FIXME: Translation removed for fmt lib
                            "%s files (%s)|%s",
                            wildCard.c_str() + nDot,
                            wildCard.c_str(),
                            wildCard.c_str()
                         );
        }
    }

    return true;
}

std::string wxFileDialogBase::AppendExtension(const std::string &filePath,
                                           const std::string &extensionList)
{
    // strip off path, to avoid problems with "path.bar/foo"
    std::string fileName = wx::utils::AfterLast(filePath, wxFILE_SEP_PATH);

    // if fileName is of form "foo.bar" it's ok, return it
    const auto idx_dot = fileName.rfind('.');
    if ((idx_dot != std::string::npos) && (idx_dot < (int)fileName.length() - 1))
        return filePath;

    // get the first extension from extensionList, or all of it
    std::string ext = wx::utils::BeforeFirst(extensionList, ';');

    // if ext == "foo" or "foo." there's no extension
    const auto idx_ext_dot = ext.rfind('.');
    if ((idx_ext_dot == wxNOT_FOUND) || (idx_ext_dot == (int)ext.length() - 1))
        return filePath;
    else
        ext = wx::utils::AfterLast(ext, '.');

    // if ext == "*" or "bar*" or "b?r" or " " then its not valid
    if ((wx::utils::StripAllSpace(ext).empty()) ||
        (ext.find('*') != std::string::npos) ||
        (ext.find('?') != std::string::npos))
        return filePath;

    // if fileName doesn't have a '.' then add one
    if (filePath.back() != '.')
        ext = "." + ext;

    return filePath + ext;
}

bool wxFileDialogBase::SetExtraControlCreator(ExtraControlCreatorFunction creator)
{
    wxCHECK_MSG( !m_extraControlCreator, false,
                 "wxFileDialog::SetExtraControl() called second time" );

    m_extraControlCreator = creator;
    return SupportsExtraControl();
}

bool wxFileDialogBase::CreateExtraControl()
{
    if (!m_extraControlCreator || m_extraControl)
        return false;
    m_extraControl = (*m_extraControlCreator)(this);
    return true;
}

wxSize wxFileDialogBase::GetExtraControlSize()
{
    if ( !m_extraControlCreator )
        return wxDefaultSize;

    // create the extra control in an empty dialog just to find its size: this
    // is not terribly efficient but we do need to know the size before
    // creating the native dialog and this seems to be the only way
    wxDialog dlg(nullptr, wxID_ANY, std::string());
    return (*m_extraControlCreator)(&dlg)->GetSize();
}

void wxFileDialogBase::UpdateExtraControlUI()
{
    if ( m_extraControl )
        m_extraControl->UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

void wxFileDialogBase::SetPath(const std::string& path)
{
    std::string ext;
    wxFileName::SplitPath(path, &m_dir, &m_fileName, &ext);
    if ( !ext.empty() )
    {
        SetFilterIndexFromExt(ext);

        m_fileName += '.' + ext;
    }

    m_path = path;
}

void wxFileDialogBase::SetDirectory(const std::string& dir)
{
    m_dir = dir;
    m_path = wxFileName(m_dir, m_fileName).GetFullPath();
}

void wxFileDialogBase::SetFilename(const std::string& name)
{
    m_fileName = name;
    m_path = wxFileName(m_dir, m_fileName).GetFullPath();
}

void wxFileDialogBase::SetFilterIndexFromExt(const std::string& ext)
{
    // if filter is of form "All files (*)|*|..." set correct filter index
    if ( !ext.empty() && m_wildCard.find('|') != std::string::npos )
    {
        int filterIndex = -1;

        std::vector<std::string> descriptions;
        std::vector<std::string> filters;
        // don't care about errors, handled already by wxFileDialog
        std::ignore = wxParseCommonDialogsFilter(m_wildCard, descriptions, filters);
        
        for (size_t n=0; n != filters.size(); n++)
        {
            if (wx::utils::Contains(filters[n], ext))
            {
                filterIndex = n;
                break;
            }
        }

        if (filterIndex >= 0)
            SetFilterIndex(filterIndex);
    }
}

//----------------------------------------------------------------------------
// wxFileDialog convenience functions
//----------------------------------------------------------------------------

std::string wxFileSelector(const std::string& title,
                        const std::string& defaultDir,
                        const std::string& defaultFileName,
                        const std::string& defaultExtension,
                        const std::string& filter,
                        unsigned int flags,
                        wxWindow *parent,
                        int x, int y)
{
    // The defaultExtension, if non-empty, is
    // appended to the filename if the user fails to type an extension. The new
    // implementation (taken from wxFileSelectorEx) appends the extension
    // automatically, by looking at the filter specification. In fact this
    // should be better than the native Microsoft implementation because
    // Windows only allows *one* default extension, whereas here we do the
    // right thing depending on the filter the user has chosen.

    // If there's a default extension specified but no filter, we create a
    // suitable filter.

    std::string filter2;
    if ( !defaultExtension.empty() && filter.empty() )
        filter2 = "*." + defaultExtension;
    else if ( !filter.empty() )
        filter2 = filter;

    wxFileDialog fileDialog(parent, title, defaultDir,
                            defaultFileName, filter2,
                            flags, wxPoint(x, y));

    fileDialog.SetFilterIndexFromExt(defaultExtension);

    std::string filename;
    if ( fileDialog.ShowModal() == wxID_OK )
    {
        filename = fileDialog.GetPath();
    }

    return filename;
}

//----------------------------------------------------------------------------
// wxFileSelectorEx
//----------------------------------------------------------------------------

std::string wxFileSelectorEx(const std::string& title,
                          const std::string& defaultDir,
                          const std::string& defaultFileName,
                          int*            defaultFilterIndex,
                          const std::string& filter,
                          int             flags,
                          wxWindow*       parent,
                          int             x,
                          int             y)

{
    wxFileDialog fileDialog(parent,
                            title,
                            defaultDir,
                            defaultFileName,
                            filter,
                            flags, wxPoint(x, y));

    std::string filename;
    if ( fileDialog.ShowModal() == wxID_OK )
    {
        if ( defaultFilterIndex )
            *defaultFilterIndex = fileDialog.GetFilterIndex();

        filename = fileDialog.GetPath();
    }

    return filename;
}

//----------------------------------------------------------------------------
// wxDefaultFileSelector - Generic load/save dialog (for internal use only)
//----------------------------------------------------------------------------

static std::string wxDefaultFileSelector(bool load,
                                      const std::string& what,
                                      const std::string& extension,
                                      const std::string& default_name,
                                      wxWindow *parent)
{
    std::string str;
    if (load)
        str = _("Load %s file");
    else
        str = _("Save %s file");
    
    std::string prompt = str + what;

    std::string wild;
    std::string ext;
    if ( !extension.empty() )
    {
        if ( extension[0u] == '.' )
            ext = extension.substr(1);
        else
            ext = extension;

        wild = fmt::format("*.%s", ext);
    }
    else // no extension specified
    {
        wild = wxFileSelectorDefaultWildcardStr;
    }

    return wxFileSelector(prompt, {}, default_name, ext, wild,
                          load ? (wxFD_OPEN | wxFD_FILE_MUST_EXIST) : wxFD_SAVE,
                          parent);
}

//----------------------------------------------------------------------------
// wxLoadFileSelector
//----------------------------------------------------------------------------

std::string wxLoadFileSelector(const std::string& what,
                                        const std::string& extension,
                                        const std::string& default_name,
                                        wxWindow *parent)
{
    return wxDefaultFileSelector(true, what, extension, default_name, parent);
}

//----------------------------------------------------------------------------
// wxSaveFileSelector
//----------------------------------------------------------------------------

std::string wxSaveFileSelector(const std::string& what,
                                        const std::string& extension,
                                        const std::string& default_name,
                                        wxWindow *parent)
{
    return wxDefaultFileSelector(false, what, extension, default_name, parent);
}


//----------------------------------------------------------------------------
// wxDirDialogBase
//----------------------------------------------------------------------------

#endif // wxUSE_FILEDLG
