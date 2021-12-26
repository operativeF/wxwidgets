/////////////////////////////////////////////////////////////////////////////
// Name:        wx/cshelp.h
// Purpose:     Context-sensitive help support classes
// Author:      Julian Smart, Vadim Zeitlin
// Modified by:
// Created:     08/09/2000
// Copyright:   (c) 2000 Julian Smart, Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CSHELP_H_
#define _WX_CSHELP_H_

#if wxUSE_HELP

#include "wx/help.h"

#if wxUSE_BMPBUTTON
#include "wx/bmpbuttn.h"
#endif

#include "wx/event.h"

import <string>;
import <unordered_map>;

// ----------------------------------------------------------------------------
// classes used to implement context help UI
// ----------------------------------------------------------------------------

/*
 * wxContextHelp
 * Invokes context-sensitive help. When the user
 * clicks on a window, a wxEVT_HELP event will be sent to that
 * window for the application to display help for.
 */

class wxContextHelp
{
public:
    wxContextHelp(wxWindow* win = nullptr, bool beginHelp = true);
    ~wxContextHelp();

    bool BeginContextHelp(wxWindow* win);
    bool EndContextHelp();

    bool EventLoop();
    bool DispatchEvent(wxWindow* win, const wxPoint& pt);

    void SetStatus(bool status) { m_status = status; }

private:
    bool    m_inHelp{false};
    bool    m_status; // true if the user left-clicked
};

#if wxUSE_BMPBUTTON
/*
 * wxContextHelpButton
 * You can add this to your dialogs (especially on non-Windows platforms)
 * to put the application into context help mode.
 */

class wxContextHelpButton : public wxBitmapButton
{
public:
    wxContextHelpButton() = default;

    wxContextHelpButton(wxWindow* parent,
                        wxWindowID id = wxID_CONTEXT_HELP,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        unsigned int style = 0)
    {
        Create(parent, id, pos, size, style);
    }


    [[maybe_unused]] bool Create(wxWindow* parent,
                wxWindowID id = wxID_CONTEXT_HELP,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0);


    void OnContextHelp(wxCommandEvent& event);

    wxContextHelpButton& operator=(wxContextHelpButton&&) = delete;

	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();

private:
    wxDECLARE_EVENT_TABLE();
};

#endif

// ----------------------------------------------------------------------------
// classes used to implement context help support
// ----------------------------------------------------------------------------

// wxHelpProvider is an abstract class used by the program implementing context help to
// show the help text (or whatever: it may be HTML page or anything else) for
// the given window.
//
// The current help provider must be explicitly set by the application using
// wxHelpProvider::Set().
//
// Special note about ShowHelpAtPoint() and ShowHelp(): we want to be able to
// override ShowHelpAtPoint() when we need to use different help messages for
// different parts of the window, but it should also be possible to override
// just ShowHelp() both for backwards compatibility and just because most
// often the help does not, in fact, depend on the position and so
// implementing just ShowHelp() is simpler and more natural, so by default
// ShowHelpAtPoint() forwards to ShowHelp(). But this means that
// wxSimpleHelpProvider has to override ShowHelp() and not ShowHelpAtPoint()
// for backwards compatibility as otherwise the existing code deriving from it
// and overriding ShowHelp() but calling the base class version wouldn't work
// any more, which forces us to use a rather ugly hack and pass the extra
// parameters of ShowHelpAtPoint() to ShowHelp() via member variables.
class wxHelpProvider
{
public:
    // get/set the current (application-global) help provider (Set() returns
    // the previous one)
    static wxHelpProvider *Set(wxHelpProvider *helpProvider)
    {
        wxHelpProvider *helpProviderOld = ms_helpProvider;
        ms_helpProvider = helpProvider;
        return helpProviderOld;
    }

    // unlike some other class, the help provider is not created on demand,
    // this must be explicitly done by the application
    static wxHelpProvider *Get() { return ms_helpProvider; }

    // get the help string (whose interpretation is help provider dependent
    // except that empty string always means that no help is associated with
    // the window) for this window
    virtual std::string GetHelp(const wxWindowBase *window) = 0;

    // do show help for the given window (uses window->GetHelpAtPoint()
    // internally if applicable), return true if it was done or false
    // if no help available for this window
    virtual bool ShowHelpAtPoint(wxWindowBase *window,
                                 const wxPoint& pt,
                                 wxHelpEvent::Origin origin)
    {
        wxCHECK_MSG( window, false, "window must not be NULL" );

        m_helptextAtPoint = pt;
        m_helptextOrigin = origin;

        return ShowHelp(window);
    }

    // show help for the given window, see ShowHelpAtPoint() above
    virtual bool ShowHelp([[maybe_unused]] wxWindowBase * window) { return false; }

    // associate the text with the given window or id: although all help
    // providers have these functions to allow making wxWindow::SetHelpText()
    // work, not all of them implement them
    virtual void AddHelp(wxWindowBase *window, const std::string& text);

    // this version associates the given text with all window with this id
    // (may be used to set the same help string for all [Cancel] buttons in
    // the application, for example)
    virtual void AddHelp(wxWindowID id, const std::string& text);

    // removes the association
    virtual void RemoveHelp(wxWindowBase* window);

    // virtual dtor for any base class
    virtual ~wxHelpProvider() = default;

protected:
    wxHelpProvider()
        : m_helptextAtPoint(wxDefaultPosition)
          
    {
    }

    // helper method used by ShowHelp(): returns the help string to use by
    // using m_helptextAtPoint/m_helptextOrigin if they're set or just GetHelp
    // otherwise
    std::string GetHelpTextMaybeAtPoint(wxWindowBase *window);

    // parameters of the last ShowHelpAtPoint() call, used by ShowHelp()
    wxPoint m_helptextAtPoint;
    wxHelpEvent::Origin m_helptextOrigin{wxHelpEvent::Origin::Unknown};

private:
    inline static wxHelpProvider *ms_helpProvider{nullptr};
};

using wxSimpleHelpProviderHashMap = std::unordered_map<wxUIntPtr, std::string>;

// wxSimpleHelpProvider is an implementation of wxHelpProvider which supports
// only plain text help strings and shows the string associated with the
// control (if any) in a tooltip
class wxSimpleHelpProvider : public wxHelpProvider
{
public:
    // implement wxHelpProvider methods
    std::string GetHelp(const wxWindowBase *window) override;

    // override ShowHelp() and not ShowHelpAtPoint() as explained above
    bool ShowHelp(wxWindowBase *window) override;

    void AddHelp(wxWindowBase *window, const std::string& text) override;
    void AddHelp(wxWindowID id, const std::string& text) override;
    void RemoveHelp(wxWindowBase* window) override;

private:
    // we use 2 hashes for storing the help strings associated with windows
    // and the ids
    wxSimpleHelpProviderHashMap m_hashWindows,
                                m_hashIds;
};

// wxHelpControllerHelpProvider is an implementation of wxHelpProvider which supports
// both context identifiers and plain text help strings. If the help text is an integer,
// it is passed to wxHelpController::DisplayContextPopup. Otherwise, it shows the string
// in a tooltip as per wxSimpleHelpProvider.
class wxHelpControllerHelpProvider : public wxSimpleHelpProvider
{
public:
    // Note that it doesn't own the help controller. The help controller
    // should be deleted separately.
    wxHelpControllerHelpProvider(wxHelpControllerBase* hc = nullptr);

    wxHelpControllerHelpProvider& operator=(wxHelpControllerHelpProvider&&) = delete;
    
    // implement wxHelpProvider methods

    // again (see above): this should be ShowHelpAtPoint() but we need to
    // override ShowHelp() to avoid breaking existing code
    bool ShowHelp(wxWindowBase *window) override;

    // Other accessors
    void SetHelpController(wxHelpControllerBase* hc) { m_helpController = hc; }
    wxHelpControllerBase* GetHelpController() const { return m_helpController; }

private:
    wxHelpControllerBase*   m_helpController;
};

// Convenience function for turning context id into std::string
std::string wxContextId(int id);

#endif // wxUSE_HELP

#endif // _WX_CSHELP_H_

