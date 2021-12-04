/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/helpbest.h
// Purpose:     Tries to load MS HTML Help, falls back to wxHTML upon failure
// Author:      Mattia Barbon
// Modified by:
// Created:     02/04/2001
// Copyright:   (c) Mattia Barbon
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_HELPBEST_H_
#define _WX_HELPBEST_H_

#if wxUSE_HELP && wxUSE_MS_HTML_HELP \
    && wxUSE_WXHTML_HELP && !defined(__WXUNIVERSAL__)

#include "wx/helpbase.h"
#include "wx/html/helpfrm.h"        // for wxHF_DEFAULT_STYLE

import <string>;

class wxBestHelpController: public wxHelpControllerBase
{
public:
    wxBestHelpController(wxWindow* parentWindow = nullptr,
                         unsigned int style = wxHF_DEFAULT_STYLE)
        : wxHelpControllerBase(parentWindow),
          m_style{style}
    {
    }

    wxBestHelpController& operator=(wxBestHelpController&&) = delete;

    // Must call this to set the filename
    bool Initialize(const std::string& file) override;
    bool Initialize(const std::string& file, [[maybe_unused]] int server ) override { return Initialize( file ); }

    // If file is "", reloads file given in Initialize
    bool LoadFile(const std::string& file = {}) override
    {
        return m_helpController->LoadFile( GetValidFilename( file ) );
    }

    bool DisplayContents() override
    {
        return m_helpController->DisplayContents();
    }

    bool DisplaySection(int sectionNo) override
    {
        return m_helpController->DisplaySection( sectionNo );
    }

    bool DisplaySection(const std::string& section) override
    {
        return m_helpController->DisplaySection( section );
    }

    bool DisplayBlock(long blockNo) override
    {
        return m_helpController->DisplayBlock( blockNo );
    }

    bool DisplayContextPopup(int contextId) override
    {
        return m_helpController->DisplayContextPopup( contextId );
    }

    bool DisplayTextPopup(const std::string& text, const wxPoint& pos) override
    {
        return m_helpController->DisplayTextPopup( text, pos );
    }

    bool KeywordSearch(const std::string& k,
                               wxHelpSearchMode mode = wxHelpSearchMode::All) override
    {
        return m_helpController->KeywordSearch( k, mode );
    }

    bool Quit() override
    {
        return m_helpController->Quit();
    }

    // Allows one to override the default settings for the help frame.
    void SetFrameParameters(const std::string& title,
                                    const wxSize& size,
                                    const wxPoint& pos = wxDefaultPosition,
                                    bool newFrameEachTime = false) override
    {
        m_helpController->SetFrameParameters( title, size, pos,
                                              newFrameEachTime );
    }

    // Obtains the latest settings used by the help frame and the help frame.
    wxFrame *GetFrameParameters(wxSize *size = nullptr,
                                        wxPoint *pos = nullptr,
                                        bool *newFrameEachTime = nullptr) override
    {
        return m_helpController->GetFrameParameters( size, pos,
                                                     newFrameEachTime );
    }

    /// Set the window that can optionally be used for the help window's parent.
    void SetParentWindow(wxWindow* win) override { m_helpController->SetParentWindow(win); }

    /// Get the window that can optionally be used for the help window's parent.
    wxWindow* GetParentWindow() const override { return m_helpController->GetParentWindow(); }

protected:
    // Append/change extension if necessary.
    std::string GetValidFilename(const std::string& file) const;

private:
    enum class HelpControllerType { None, Html, Chm };

    HelpControllerType m_helpControllerType{HelpControllerType::None};
    std::unique_ptr<wxHelpControllerBase> m_helpController{};
    unsigned int m_style;
};

#endif // wxUSE_HELP && wxUSE_MS_HTML_HELP && wxUSE_WXHTML_HELP

#endif
    // _WX_HELPBEST_H_
