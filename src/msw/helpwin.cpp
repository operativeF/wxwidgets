/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/helpwin.cpp
// Purpose:     Help system: WinHelp implementation
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_HELP

#ifndef WX_PRECOMP
#endif

#include "wx/filename.h"
#include "wx/msw/helpwin.h"

#include <ctime>

#ifdef __WXMSW__
#include "wx/msw/private.h"
#endif

#include <cstring>

static HWND GetSuitableHWND(wxWinHelpController* controller)
{
    if (controller->GetParentWindow())
        return (HWND) controller->GetParentWindow()->GetHWND();
    else if (wxTheApp->GetTopWindow())
        return (HWND) wxTheApp->GetTopWindow()->GetHWND();
    else
        return GetDesktopWindow();
}

bool wxWinHelpController::Initialize(const std::string& filename)
{
    m_helpFile = filename;
    return true;
}

bool wxWinHelpController::LoadFile(const std::string& file)
{
    if (!file.empty())
        m_helpFile = file;
    return true;
}

bool wxWinHelpController::DisplayContents()
{
    if (m_helpFile.empty()) return false;

    std::string str = GetValidFilename(m_helpFile);

    return (WinHelpW(GetSuitableHWND(this), boost::nowide::widen(str).c_str(), HELP_FINDER, 0L) != 0);
}

bool wxWinHelpController::DisplaySection(int section)
{
    // Use context number
    if (m_helpFile.empty()) return false;

    std::string str = GetValidFilename(m_helpFile);

    return (WinHelpW(GetSuitableHWND(this), boost::nowide::widen(str).c_str(), HELP_CONTEXT, (DWORD)section) != 0);
}

bool wxWinHelpController::DisplayContextPopup(int contextId)
{
    if (m_helpFile.empty()) return false;

    std::string str = GetValidFilename(m_helpFile);

    return (WinHelpW(GetSuitableHWND(this), boost::nowide::widen(str).c_str(), HELP_CONTEXTPOPUP, (DWORD) contextId) != 0);
}

bool wxWinHelpController::DisplayBlock(long block)
{
    DisplaySection(block);
    return true;
}

bool wxWinHelpController::KeywordSearch(const std::string& k,
                                        wxHelpSearchMode WXUNUSED(mode))
{
    if (m_helpFile.empty()) return false;

    std::string str = GetValidFilename(m_helpFile);

    return WinHelpW(GetSuitableHWND(this), boost::nowide::widen(str).c_str(), HELP_PARTIALKEY,
                   (ULONG_PTR)boost::nowide::widen(k).c_str()) != 0;
}

// Can't close the help window explicitly in WinHelp
bool wxWinHelpController::Quit()
{
    return WinHelpW(GetSuitableHWND(this), nullptr, HELP_QUIT, 0) != 0;
}

// Append extension if necessary.
std::string wxWinHelpController::GetValidFilename(const std::string& file) const
{
    wxString path, name, ext;
    wxFileName::SplitPath(file, & path, & name, & ext);

    std::string fullName;
    if (path.empty())
        fullName = name + wxT(".hlp");
    else if (path.Last() == wxT('\\'))
        fullName = path + name + wxT(".hlp");
    else
        fullName = path + wxT("\\") + name + wxT(".hlp");
    return fullName;
}

#endif // wxUSE_HELP
