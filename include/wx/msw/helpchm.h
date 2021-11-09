/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/helpchm.h
// Purpose:     Help system: MS HTML Help implementation
// Author:      Julian Smart
// Modified by:
// Created:     16/04/2000
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_HELPCHM_H_
#define _WX_MSW_HELPCHM_H_

#if wxUSE_MS_HTML_HELP

#include "wx/helpbase.h"

import <filesystem>;
import <string>;

class wxCHMHelpController : public wxHelpControllerBase
{
public:
    explicit wxCHMHelpController(wxWindow* parentWindow = nullptr): wxHelpControllerBase(parentWindow) { }

    // Must call this to set the filename
    bool Initialize(const std::string& file) override;
    bool Initialize(const std::string& file, int WXUNUSED(server) ) override { return Initialize( file ); }

    // If file is "", reloads file given in Initialize
    bool LoadFile(const std::string& file = {}) override;
    bool DisplayContents() override;
    bool DisplaySection(int sectionNo) override;
    bool DisplaySection(const std::string& section) override;
    bool DisplayBlock(long blockNo) override;
    bool DisplayContextPopup(int contextId) override;
    bool DisplayTextPopup(const std::string& text, const wxPoint& pos) override;
    bool KeywordSearch(const std::string& k,
                               wxHelpSearchMode mode = wxHelpSearchMode::All) override;
    bool Quit() override;

    const std::string& GetHelpFile() const { return m_helpFile; }

    // helper of DisplayTextPopup(), also used in wxSimpleHelpProvider::ShowHelp
    static bool ShowContextHelpPopup(const std::string& text,
                                     const wxPoint& pos,
                                     wxWindow *window);

protected:
    // get the name of the CHM file we use from our m_helpFile
    std::string GetValidFilename() const;

    // Call HtmlHelp() with the provided parameters (both overloads do the same
    // thing but allow to avoid casts in the calling code) and return false
    // (but don't crash) if HTML help is unavailable
    static bool CallHtmlHelp(wxWindow *win, const std::string& str,
                             unsigned cmd, WXWPARAM param);
    static bool CallHtmlHelp(wxWindow *win, const std::string& str,
                             unsigned cmd, const void *param = nullptr)
    {
        return CallHtmlHelp(win, str, cmd, reinterpret_cast<WXWPARAM>(param));
    }

    // even simpler wrappers using GetParentWindow() and GetValidFilename() as
    // the first 2 HtmlHelp() parameters
    bool CallHtmlHelp(unsigned cmd, WXWPARAM param)
    {
        return CallHtmlHelp(GetParentWindow(), GetValidFilename(),
                            cmd, param);
    }

    bool CallHtmlHelp(unsigned cmd, const void *param = nullptr)
    {
        return CallHtmlHelp(cmd, reinterpret_cast<WXWPARAM>(param));
    }

    // wrapper around CallHtmlHelp(HH_DISPLAY_TEXT_POPUP): only one of text and
    // contextId parameters can be non-NULL/non-zero
    static bool DoDisplayTextPopup(const std::string& text,
                                   const wxPoint& pos,
                                   int contextId,
                                   wxWindow *window);

private:
    std::string m_helpFile;

    wxDECLARE_DYNAMIC_CLASS(wxCHMHelpController);
};

#endif // wxUSE_MS_HTML_HELP

#endif // _WX_MSW_HELPCHM_H_
