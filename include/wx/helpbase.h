/////////////////////////////////////////////////////////////////////////////
// Name:        wx/helpbase.h
// Purpose:     Help system base classes
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_HELPBASEH__
#define _WX_HELPBASEH__

#include "wx/defs.h"

#if wxUSE_HELP

#include "wx/gdicmn.h"

#include <string>

class WXDLLIMPEXP_FWD_CORE wxFrame;
class WXDLLIMPEXP_FWD_CORE wxWindow;

// Flags for SetViewer
inline constexpr int wxHELP_NETSCAPE = 1;

// Search modes:
enum class wxHelpSearchMode
{
    Index,
    All
};

// Defines the API for help controllers
class WXDLLIMPEXP_CORE wxHelpControllerBase
{
public:
    wxHelpControllerBase(wxWindow* parentWindow = nullptr) { m_parentWindow = parentWindow; }
    virtual ~wxHelpControllerBase() = default;

    // Must call this to set the filename and server name.
    // server is only required when implementing TCP/IP-based
    // help controllers.
    virtual bool Initialize(const std::string& WXUNUSED(file), int WXUNUSED(server) ) { return false; }
    virtual bool Initialize(const std::string& WXUNUSED(file)) { return false; }

    // Set viewer: only relevant to some kinds of controller
    virtual void SetViewer(const std::string& WXUNUSED(viewer), long WXUNUSED(flags) = 0) {}

    // If file is "", reloads file given  in Initialize
    virtual bool LoadFile(const std::string& file = {}) = 0;

    // Displays the contents
    virtual bool DisplayContents() = 0;

    // Display the given section
    virtual bool DisplaySection(int sectionNo) = 0;

    // Display the section using a context id
    virtual bool DisplayContextPopup(int WXUNUSED(contextId)) { return false; }

    // Display the text in a popup, if possible
    virtual bool DisplayTextPopup(const std::string& WXUNUSED(text), const wxPoint& WXUNUSED(pos)) { return false; }

    // By default, uses KeywordSection to display a topic. Implementations
    // may override this for more specific behaviour.
    virtual bool DisplaySection(const std::string& section) { return KeywordSearch(section); }
    virtual bool DisplayBlock(long blockNo) = 0;
    virtual bool KeywordSearch(const std::string& k,
                               wxHelpSearchMode mode = wxHelpSearchMode::All) = 0;
    /// Allows one to override the default settings for the help frame.
    virtual void SetFrameParameters(const std::string& WXUNUSED(title),
        const wxSize& WXUNUSED(size),
        const wxPoint& WXUNUSED(pos) = wxDefaultPosition,
        bool WXUNUSED(newFrameEachTime) = false)
    {
        // does nothing by default
    }
    /// Obtains the latest settings used by the help frame and the help
    /// frame.
    virtual wxFrame *GetFrameParameters(wxSize *WXUNUSED(size) = nullptr,
        wxPoint *WXUNUSED(pos) = nullptr,
        bool *WXUNUSED(newFrameEachTime) = nullptr)
    {
        return nullptr; // does nothing by default
    }

    virtual bool Quit() = 0;
    virtual void OnQuit() {}

    /// Set the window that can optionally be used for the help window's parent.
    virtual void SetParentWindow(wxWindow* win) { m_parentWindow = win; }

    /// Get the window that can optionally be used for the help window's parent.
    virtual wxWindow* GetParentWindow() const { return m_parentWindow; }

protected:
    wxWindow* m_parentWindow;
};

#endif // wxUSE_HELP

#endif
// _WX_HELPBASEH__
