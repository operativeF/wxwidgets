/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/helpext.h
// Purpose:     an external help controller for wxWidgets
// Author:      Karsten Ballueder (Ballueder@usa.net)
// Modified by:
// Copyright:   (c) Karsten Ballueder 1998
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_HELPEXT_H_
#define __WX_HELPEXT_H_

#if wxUSE_HELP


// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wx/helpbase.h"


// ----------------------------------------------------------------------------
// wxExtHelpController
// ----------------------------------------------------------------------------

// This class implements help via an external browser.
class wxExtHelpController : public wxHelpControllerBase
{
public:
    wxExtHelpController(wxWindow* parentWindow = nullptr);
    ~wxExtHelpController();

    // Set viewer: new name for SetBrowser
    void SetViewer(const std::string& viewer = {},
                   unsigned int flags = wxHELP_NETSCAPE) override;

    bool Initialize(const std::string& dir, int WXUNUSED(server)) override
        { return Initialize(dir); }

    bool Initialize(const std::string& dir) override;
    bool LoadFile(const std::string& file = {}) override;
    bool DisplayContents() override;
    bool DisplaySection(int sectionNo) override;
    bool DisplaySection(const std::string& section) override;
    bool DisplayBlock(long blockNo) override;
    bool KeywordSearch(const std::string& k,
                                wxHelpSearchMode mode = wxHelpSearchMode::All) override;

    bool Quit() override;
    void OnQuit() override;

    virtual bool DisplayHelp(const std::string &) ;

    void SetFrameParameters(const std::string& WXUNUSED(title),
                                    [[maybe_unused]] const wxSize& size,
                                    [[maybe_unused]] const wxPoint& pos = wxDefaultPosition,
                                    bool WXUNUSED(newFrameEachTime) = false) override
        {
            // does nothing by default
        }

    wxFrame *GetFrameParameters(wxSize *WXUNUSED(size) = nullptr,
                                    wxPoint *WXUNUSED(pos) = nullptr,
                                    bool *WXUNUSED(newFrameEachTime) = nullptr) override
        {
            return nullptr; // does nothing by default
        }

protected:
    // Filename of currently active map file.
    std::string         m_helpDir;

    // A list containing all id,url,documentation triples.
    wxList          *m_MapList{nullptr};

    // How many entries do we have in the map file?
    int              m_NumOfEntries{0};
private:
    // How to call the html viewer.
    std::string         m_BrowserName;

    // parse a single line of the map file (called by LoadFile())
    //
    // return true if the line was valid or false otherwise
    bool ParseMapFileLine(const std::string& line);

    // Deletes the list and all objects.
    void DeleteList();

    // Is the viewer a variant of netscape?
    bool             m_BrowserIsNetscape{false};

    wxDECLARE_CLASS(wxExtHelpController);
};

#endif // wxUSE_HELP

#endif // __WX_HELPEXT_H_
