/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/helpchm.cpp
// Purpose:     Help system: MS HTML Help implementation
// Author:      Julian Smart
// Modified by: Vadim Zeitlin at 2008-03-01: refactoring, simplification
// Created:     16/04/2000
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_HELP && wxUSE_MS_HTML_HELP

#include "wx/msw/helpchm.h"

#include "wx/dynload.h"

#include "wx/msw/private.h"

#include "wx/intl.h"
#include "wx/app.h"

#include "wx/msw/htmlhelp.h"

#include <boost/nowide/stackstring.hpp>

import WX.Utils.Cast;
import WX.File.Filename;

// ----------------------------------------------------------------------------
// utility functions to manage the loading/unloading
// of hhctrl.ocx
// ----------------------------------------------------------------------------

using HTMLHELP = WXHWND ( WINAPI*)( WXHWND, LPCWSTR, WXUINT, ULONG_PTR );
constexpr char HTMLHELP_NAME[] = "HtmlHelpW";

HTMLHELP GetHtmlHelpFunction()
{
    static HTMLHELP s_htmlHelp = nullptr;

    if ( !s_htmlHelp )
    {
        static wxDynamicLibrary s_dllHtmlHelp("HHCTRL.OCX", wxDL_VERBATIM);

        if ( !s_dllHtmlHelp.IsLoaded() )
        {
            wxLogError(_("MS HTML Help functions are unavailable because the MS HTML Help library is not installed on this machine. Please install it."));
        }
        else
        {
            s_htmlHelp = (HTMLHELP)s_dllHtmlHelp.GetSymbol(HTMLHELP_NAME);
            if ( !s_htmlHelp )
            {
                wxLogError(_("Failed to initialize MS HTML Help."));
            }
        }
    }

    return s_htmlHelp;
}

// find the window to use in HtmlHelp() call: use the given one by default but
// fall back to the top level app window and then the desktop if it's NULL
static WXHWND GetSuitableHWND(wxWindow *win)
{
    if ( !win )
        win = wxApp::GetMainTopWindow();

    return win ? GetHwndOf(win) : ::GetDesktopWindow();
}


bool wxCHMHelpController::Initialize(const std::string& filename)
{
    if ( !GetHtmlHelpFunction() )
        return false;

    m_helpFile = filename;
    return true;
}

bool wxCHMHelpController::LoadFile(const std::string& file)
{
    if (!file.empty())
        m_helpFile = file;
    return true;
}

/* static */ bool
wxCHMHelpController::CallHtmlHelp(wxWindow *win,
                                  const std::string& str,
                                  unsigned cmd,
                                  WXWPARAM param)
{
    HTMLHELP htmlHelp = GetHtmlHelpFunction();

    return htmlHelp && htmlHelp(GetSuitableHWND(win), boost::nowide::widen(str).c_str(), cmd, param);
}

bool wxCHMHelpController::DisplayContents()
{
    if (m_helpFile.empty())
        return false;

    return CallHtmlHelp(HH_DISPLAY_TOC);
}

// Use topic or HTML filename
bool wxCHMHelpController::DisplaySection(const std::string& section)
{
    if (m_helpFile.empty())
        return false;

    // Is this an HTML file or a keyword?
    if ( section.find(".htm") != std::string::npos )
    {
        // interpret as a file name
        return CallHtmlHelp(HH_DISPLAY_TOPIC, boost::nowide::widen(section).c_str());
    }

    return KeywordSearch(section);
}

// Use context number
bool wxCHMHelpController::DisplaySection(int section)
{
    if (m_helpFile.empty())
        return false;

    // Treat -1 as a special context number that displays the index
    if (section == -1)
    {
        return CallHtmlHelp(HH_DISPLAY_INDEX);
    }

    return CallHtmlHelp(HH_HELP_CONTEXT, section);
}

/* static */
bool
wxCHMHelpController::DoDisplayTextPopup(const std::string& text,
                                        const wxPoint& pos,
                                        int contextId,
                                        wxWindow *window)
{
    boost::nowide::wstackstring stackText(text.c_str());

    HH_POPUP popup = {
        .cbStruct{sizeof(popup)},
        .hinst{(WXHINSTANCE) wxGetInstance()},
        .idString{wx::narrow_cast<WXUINT>(contextId)},
        .pszText{stackText.get()},
        .pt{pos.x, pos.y},
        .clrForeground{::GetSysColor(COLOR_INFOTEXT)},
        .clrBackground{::GetSysColor(COLOR_INFOBK)},
        .rcMargins{-1, -1, -1, -1},
        .pszFont{nullptr}
    };

    return CallHtmlHelp(window, "", HH_DISPLAY_TEXT_POPUP, &popup);
}

bool wxCHMHelpController::DisplayContextPopup(int contextId)
{
    return DoDisplayTextPopup("", wxGetMousePosition(), contextId,
                              GetParentWindow());
}

bool
wxCHMHelpController::DisplayTextPopup(const std::string& text, const wxPoint& pos)
{
    return ShowContextHelpPopup(text, pos, GetParentWindow());
}

/* static */
bool wxCHMHelpController::ShowContextHelpPopup(const std::string& text,
                                               const wxPoint& pos,
                                               wxWindow *window)
{
    return DoDisplayTextPopup(text, pos, 0, window);
}

bool wxCHMHelpController::DisplayBlock(long block)
{
    return DisplaySection(block);
}

bool wxCHMHelpController::KeywordSearch(const std::string& k,
                                        [[maybe_unused]] wxHelpSearchMode mode)
{
    if (m_helpFile.empty())
        return false;

    if (k.empty())
    {
        HH_FTS_QUERY oQuery
        {
            .cbStruct{sizeof(HH_FTS_QUERY)},
            .fUniCodeStrings{0},
            .pszSearchQuery{L""},
            .iProximity{0},
            .fStemmedSearch{0},
            .fTitleOnly{0},
            .fExecute{1},
            .pszWindow{L""}
        };

        return CallHtmlHelp(HH_DISPLAY_SEARCH, &oQuery);
    }
    else
    {
        boost::nowide::wstackstring stackK(k.c_str());

        HH_AKLINK link = {
            .cbStruct{sizeof(HH_AKLINK)},
            .fReserved{FALSE},
            .pszKeywords{stackK.get()},
            .pszUrl{nullptr},
            .pszMsgText{nullptr},
            .pszMsgTitle{nullptr},
            .pszWindow{nullptr},
            .fIndexOnFail{TRUE}
        };

        return CallHtmlHelp(HH_KEYWORD_LOOKUP, &link);
    }
}

bool wxCHMHelpController::Quit()
{
    return CallHtmlHelp(nullptr, "", HH_CLOSE_ALL);
}

std::string wxCHMHelpController::GetValidFilename() const
{
    std::string path, name, ext;
    wxFileName::SplitPath(m_helpFile, &path, &name, &ext);

    std::string fullName;
    if (path.empty())
        fullName = name + ".chm";
    else if (path.back() == '\\')
        fullName = path + name + ".chm";
    else
        fullName = path + "\\" + name + ".chm";
    return fullName;
}

#endif // wxUSE_HELP
