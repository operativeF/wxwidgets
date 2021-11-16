/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/filehistorycmn.cpp
// Purpose:     wxFileHistory class
// Author:      Julian Smart, Vaclav Slavik, Vadim Zeitlin
// Created:     2010-05-03
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_FILE_HISTORY

#include "wx/filehistory.h"

#include "wx/menu.h"
#include "wx/confbase.h"
#include "wx/filename.h"

#include <boost/nowide/convert.hpp>
#include <fmt/core.h>

import Utils.Strings;

import <filesystem>;

namespace
{

// return the string used for the MRU list items in the menu
//
// NB: the index n is 0-based, as usual, but the strings start from 1
wxString GetMRUEntryLabel(int n, const fs::path& path)
{
    // we need to quote '&' characters which are used for mnemonics
    auto pathInMenu = path.wstring();
    wx::utils::ReplaceAll(pathInMenu, L"&", L"&&");

#ifdef __WXMSW__
    // absolute paths always start with Latin characters even in RTL
    // environments and should therefore be rendered as LTR text (possibly with
    // RTL chunks in it). Ensure this on Windows by prepending
    // LEFT-TO-RIGHT EMBEDDING (other platforms detect this automatically)
    pathInMenu.insert(0, 1, wchar_t(0x202a));
#endif

    return fmt::format("&{:d} {:s}", n + 1, boost::nowide::narrow(pathInMenu));
}

} // anonymous namespace

// ----------------------------------------------------------------------------
// File history (a.k.a. MRU, most recently used, files list)
// ----------------------------------------------------------------------------

wxFileHistoryBase::wxFileHistoryBase(size_t maxFiles, wxWindowID idBase)
    : m_fileMaxFiles(maxFiles),
      m_idBase(idBase)
{
}

/* static */
std::string wxFileHistoryBase::NormalizeFileName(const wxFileName& fn)
{
    // We specifically exclude wxPATH_NORM_LONG here as it can take a long time
    // (several seconds) for network file paths under MSW, resulting in huge
    // delays when opening a program using wxFileHistory. We also exclude
    // wxPATH_NORM_ENV_VARS as the file names here are supposed to be "real"
    // file names and not have any environment variables in them.
    wxFileName fnNorm(fn);
    fnNorm.Normalize(wxPATH_NORM_DOTS |
                     wxPATH_NORM_TILDE |
                     wxPATH_NORM_CASE |
                     wxPATH_NORM_ABSOLUTE);
    return fnNorm.GetFullPath();
}

void wxFileHistoryBase::AddFileToHistory(const fs::path& file)
{
    // Check if we don't already have this file. Notice that we avoid
    // wxFileName::operator==(wxString) here as it converts the string to
    // wxFileName and then normalizes it using all normalizations which is too
    // slow (see the comment above), so we use our own quick normalization
    // functions and a string comparison.
    const fs::path fnNew = file;
    const fs::path newFile = fs::canonical(fnNew);
    size_t numFiles = m_fileHistory.size();
    for ( size_t i = 0; i < numFiles; i++ )
    {
        if ( newFile == fs::canonical(m_fileHistory[i]) )
        {
            // we do have it, move it to the top of the history
            RemoveFileFromHistory(i);
            numFiles--;
            break;
        }
    }

    // if we already have a full history, delete the one at the end
    if ( numFiles == m_fileMaxFiles )
    {
        RemoveFileFromHistory(--numFiles);
    }

    // add a new menu item to all file menus (they will be updated below)
    for ( wxList::compatibility_iterator node = m_fileMenus.GetFirst();
        node;
        node = node->GetNext() )
    {
        wxMenu * const menu = (wxMenu *)node->GetData();

        if ( !numFiles && menu->GetMenuItemCount() )
            menu->AppendSeparator();

        // label doesn't matter, it will be set below anyhow, but it can't
        // be empty (this is supposed to indicate a stock item)
        menu->Append(m_idBase + numFiles, " ");
    }

    // insert the new file in the beginning of the file history
    m_fileHistory.insert(m_fileHistory.begin(), file);
    numFiles++;

    DoRefreshLabels();
}

void wxFileHistoryBase::DoRefreshLabels()
{
    const size_t numFiles = m_fileHistory.size();

    // If no files, then no need to refresh the menu
    if ( numFiles == 0 )
        return;

    // Remember the path in case we need to compare with it below.
    const auto firstPath = m_fileHistory[0].relative_path();

    // Update the labels in all menus
    for ( size_t i = 0; i < numFiles; i++ )
    {
        const fs::path currFn = m_fileHistory[i].filename();

        const fs::path pathInMenu = [this, currFn, firstPath]() {
            switch ( m_menuPathStyle )
            {
                case wxFileHistoryMenuPathStyle::ShowIfDifferent:
                    if ( currFn.has_filename() && currFn.relative_path() == firstPath )
                        return currFn.filename();
                    else
                        return currFn.relative_path();

                case wxFileHistoryMenuPathStyle::ShowNever:
                    // Only show the filename + extension and not the path.
                    return currFn.filename();

                case wxFileHistoryMenuPathStyle::ShowAlways:
                default: // Default to just showing the full path.
                    // Always show full path.
                    return currFn.relative_path();
            }
        }();

        for ( wxList::compatibility_iterator node = m_fileMenus.GetFirst();
              node;
              node = node->GetNext() )
        {
            wxMenu * const menu = (wxMenu *)node->GetData();

            menu->SetLabel(m_idBase + i, GetMRUEntryLabel(i, pathInMenu));
        }
    }
}

void wxFileHistoryBase::SetMenuPathStyle(wxFileHistoryMenuPathStyle style)
{
    if ( style != m_menuPathStyle )
    {
        m_menuPathStyle = style;
        DoRefreshLabels();
    }
}

void wxFileHistoryBase::RemoveFileFromHistory(size_t i)
{
    auto numFiles = m_fileHistory.size();
    wxCHECK_RET( i < numFiles,
                 "invalid index in wxFileHistoryBase::RemoveFileFromHistory" );

    // delete the element from the array
    m_fileHistory.erase(std::begin(m_fileHistory) + i);
    numFiles--;

    for ( wxList::compatibility_iterator node = m_fileMenus.GetFirst();
          node;
          node = node->GetNext() )
    {
        wxMenu * const menu = (wxMenu *) node->GetData();

        // shift filenames up
        for ( size_t j = i; j < numFiles; j++ )
        {
            menu->SetLabel(m_idBase + j, GetMRUEntryLabel(j, m_fileHistory[j]));
        }

        // delete the last menu item which is unused now
        const wxWindowID lastItemId = m_idBase + numFiles;
        if ( menu->FindItem(lastItemId) )
            menu->Delete(lastItemId);

        // delete the last separator too if no more files are left
        if ( m_fileHistory.empty() )
        {
            const wxMenuItemList::compatibility_iterator
                nodeLast = menu->GetMenuItems().GetLast();
            if ( nodeLast )
            {
                wxMenuItem * const lastMenuItem = nodeLast->GetData();
                if ( lastMenuItem->IsSeparator() )
                    menu->Delete(lastMenuItem);
            }
            //else: menu is empty somehow
        }
    }
}

void wxFileHistoryBase::UseMenu(wxMenu *menu)
{
    if ( !m_fileMenus.Member(menu) )
        m_fileMenus.Append(menu);
}

void wxFileHistoryBase::RemoveMenu(wxMenu *menu)
{
    m_fileMenus.DeleteObject(menu);
}

#if wxUSE_CONFIG
void wxFileHistoryBase::Load(const wxConfigBase& config)
{
    RemoveExistingHistory();

    m_fileHistory.clear();

    std::string buf = fmt::format("file{:d}", 1);

    std::string historyFile; // TODO: Need to have config read filesystem paths.
    while ((m_fileHistory.size() < m_fileMaxFiles) &&
           config.Read(buf, &historyFile) && !historyFile.empty())
    {
        m_fileHistory.push_back(fs::path{historyFile});

        buf += fmt::format("file{:d}", m_fileHistory.size() + 1);
        historyFile.clear();
    }

    AddFilesToMenu();
}

void wxFileHistoryBase::Save(wxConfigBase& config)
{
    for (size_t i = 0; i < m_fileMaxFiles; i++)
    {
        std::string buf = fmt::format("file%d", (int)i+1);
        if (i < m_fileHistory.size())
            config.Write(buf, m_fileHistory[i].string());
        else
            config.Write(buf, std::string{});
    }
}
#endif // wxUSE_CONFIG

void wxFileHistoryBase::AddFilesToMenu()
{
    if ( m_fileHistory.empty() )
        return;

    for ( wxList::compatibility_iterator node = m_fileMenus.GetFirst();
          node;
          node = node->GetNext() )
    {
        AddFilesToMenu((wxMenu *) node->GetData());
    }
}

void wxFileHistoryBase::AddFilesToMenu(wxMenu* menu)
{
    if ( m_fileHistory.empty() )
        return;

    if ( menu->GetMenuItemCount() )
        menu->AppendSeparator();

    for ( size_t i = 0; i < m_fileHistory.size(); i++ )
    {
        menu->Append(m_idBase + i, GetMRUEntryLabel(i, m_fileHistory[i]));
    }
}

void wxFileHistoryBase::RemoveExistingHistory()
{
    const size_t count = m_fileHistory.size();
    if ( !count )
        return;

    for ( wxList::compatibility_iterator node = m_fileMenus.GetFirst();
          node;
          node = node->GetNext() )
    {
        auto* const menu = dynamic_cast<wxMenu *>(node->GetData());

        // Notice that we remove count+1 items from the menu as we also remove
        // the separator preceding them.
        for ( size_t n = 0; n <= count; n++ )
        {
            const wxMenuItemList::compatibility_iterator
                nodeLast = menu->GetMenuItems().GetLast();
            if ( nodeLast )
            {
                wxMenuItem * const lastMenuItem = nodeLast->GetData();
                menu->Delete(lastMenuItem);
            }
        }
    }
}

#endif // wxUSE_FILE_HISTORY
