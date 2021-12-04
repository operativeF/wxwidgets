/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/helpwin.h
// Purpose:     Help system: WinHelp implementation
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_HELPWIN_H_
#define _WX_HELPWIN_H_

#if wxUSE_HELP

#include "wx/helpbase.h"

import <string>;

class wxWinHelpController: public wxHelpControllerBase
{
public:
    explicit wxWinHelpController(wxWindow* parentWindow = nullptr): wxHelpControllerBase(parentWindow) {}

    // Must call this to set the filename
    bool Initialize(const std::string& file) override;
    bool Initialize(const std::string& file, [[maybe_unused]] int server ) override { return Initialize( file ); }

    // If file is empty, reloads file given in Initialize
    bool LoadFile(const std::string& file = {}) override;
    bool DisplayContents() override;
    bool DisplaySection(int sectionNo) override;
    bool DisplaySection(const std::string& section) override { return KeywordSearch(section); }
    bool DisplayBlock(long blockNo) override;
    bool DisplayContextPopup(int contextId) override;
    bool KeywordSearch(const std::string& k,
                               wxHelpSearchMode mode = wxHelpSearchMode::All) override;
    bool Quit() override;

    std::string GetHelpFile() const { return m_helpFile; }

protected:
    // Append extension if necessary.
    std::string GetValidFilename(const std::string& file) const;

private:
    std::string m_helpFile;
};

#endif // wxUSE_HELP
#endif
// _WX_HELPWIN_H_
