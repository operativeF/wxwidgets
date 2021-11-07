/////////////////////////////////////////////////////////////////////////////
// Name:        webview.cpp
// Purpose:     Common interface and events for web view component
// Author:      Marianne Gagnon
// Copyright:   (c) 2010 Marianne Gagnon, 2011 Steven Lamerton
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_WEBVIEW

#include "wx/webview.h"

#if defined(__WXOSX__)
#include "wx/osx/webview_webkit.h"
#elif defined(__WXGTK__)
#include "wx/gtk/webview_webkit.h"
#elif defined(WX_WINDOWS)
#include "wx/msw/webview_edge.h"
#endif

// DLL options compatibility check:
#include "wx/app.h"
WX_CHECK_BUILD_OPTIONS("wxWEBVIEW")

wxIMPLEMENT_ABSTRACT_CLASS(wxWebView, wxControl);
wxIMPLEMENT_DYNAMIC_CLASS(wxWebViewEvent, wxCommandEvent);

wxDEFINE_EVENT( wxEVT_WEBVIEW_NAVIGATING, wxWebViewEvent );
wxDEFINE_EVENT( wxEVT_WEBVIEW_NAVIGATED, wxWebViewEvent );
wxDEFINE_EVENT( wxEVT_WEBVIEW_LOADED, wxWebViewEvent );
wxDEFINE_EVENT( wxEVT_WEBVIEW_ERROR, wxWebViewEvent );
wxDEFINE_EVENT( wxEVT_WEBVIEW_NEWWINDOW, wxWebViewEvent );
wxDEFINE_EVENT( wxEVT_WEBVIEW_TITLE_CHANGED, wxWebViewEvent );
wxDEFINE_EVENT( wxEVT_WEBVIEW_FULLSCREEN_CHANGED, wxWebViewEvent);
wxDEFINE_EVENT( wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED, wxWebViewEvent);

wxStringWebViewFactoryMap wxWebView::m_factoryMap;

wxWebViewZoom wxWebView::GetZoom() const
{
    float zoom = GetZoomFactor();

    // arbitrary way to map float zoom to our common zoom enum
    if (zoom <= 0.55f)
    {
        return wxWebViewZoom::Tiny;
    }
    if (zoom <= 0.85f)
    {
        return wxWebViewZoom::Small;
    }
    if (zoom <= 1.15f)
    {
        return wxWebViewZoom::Medium;
    }
    if (zoom <= 1.45f)
    {
        return wxWebViewZoom::Large;
    }

    return wxWebViewZoom::Largest;
}

void wxWebView::SetZoom(wxWebViewZoom zoom)
{
    // arbitrary way to map our common zoom enum to float zoom
    switch (zoom)
    {
        case wxWebViewZoom::Tiny:
            SetZoomFactor(0.4f);
            break;

        case wxWebViewZoom::Small:
            SetZoomFactor(0.7f);
            break;

        case wxWebViewZoom::Medium:
            SetZoomFactor(1.0f);
            break;

        case wxWebViewZoom::Large:
            SetZoomFactor(1.3f);
            break;

        case wxWebViewZoom::Largest:
            SetZoomFactor(1.6f);
            break;
    }
}

bool wxWebView::QueryCommandEnabled(const wxString& command) const
{
    wxString resultStr;
    RunScript(
        wxString::Format("function f(){ return document.queryCommandEnabled('%s'); } f();", command), &resultStr);
    return resultStr.IsSameAs("true", false);
}

void wxWebView::ExecCommand(const wxString& command)
{
    RunScript(wxString::Format("document.execCommand('%s');", command));
}

wxString wxWebView::GetPageSource() const
{
    wxString text;
    RunScript("document.documentElement.outerHTML;", &text);
    return text;
}

wxString wxWebView::GetPageText() const
{
    wxString text;
    RunScript("document.body.innerText;", &text);
    return text;
}

bool wxWebView::CanCut() const
{
    return QueryCommandEnabled("cut");
}

bool wxWebView::CanCopy() const
{
    return QueryCommandEnabled("copy");
}

bool wxWebView::CanPaste() const
{
    return QueryCommandEnabled("paste");
}

void wxWebView::Cut()
{
    ExecCommand("cut");
}

void wxWebView::Copy()
{
    ExecCommand("copy");
}

void wxWebView::Paste()
{
    ExecCommand("paste");
}

wxString wxWebView::GetSelectedText() const
{
    wxString text;
    RunScript("window.getSelection().toString();", &text);
    return text;
}

wxString wxWebView::GetSelectedSource() const
{
    // TODO: could probably be implemented by script similar to GetSelectedText()
    return {};
}

void wxWebView::DeleteSelection()
{
    ExecCommand("delete");
}

bool wxWebView::HasSelection() const
{
    wxString rangeCountStr;
    RunScript("window.getSelection().rangeCount;", &rangeCountStr);
    return rangeCountStr != "0";
}

void wxWebView::ClearSelection()
{
    //We use javascript as selection isn't exposed at the moment in webkit
    RunScript("window.getSelection().removeAllRanges();");
}

void wxWebView::SelectAll()
{
    RunScript("window.getSelection().selectAllChildren(document.body);");
}

long wxWebView::Find(const wxString& text, unsigned int flags)
{
    if (text != m_findText)
        ClearSelection();
    m_findText = text;
    wxString output;
    RunScript(wxString::Format("window.find('%s', %s, %s, %s, %s)",
        text,
        (flags & wxWEBVIEW_FIND_MATCH_CASE) ? "true" : "false",
        (flags & wxWEBVIEW_FIND_BACKWARDS) ? "true" : "false",
        (flags & wxWEBVIEW_FIND_WRAP) ? "true" : "false",
        (flags & wxWEBVIEW_FIND_ENTIRE_WORD) ? "true" : "false"
        ), &output);
    if (output.IsSameAs("false", false))
        return wxNOT_FOUND;
    else
        return 1;
}

wxString wxWebView::GetUserAgent() const
{
    wxString userAgent;
    RunScript("navigator.userAgent", &userAgent);
    return userAgent;
}

// static
wxWebView* wxWebView::New(const std::string& backend)
{
    wxStringWebViewFactoryMap::iterator iter = FindFactory(backend);

    if(iter == m_factoryMap.end())
        return nullptr;
    else
        return (*iter).second->Create();
}

// static
wxWebView* wxWebView::New(wxWindow* parent,
                          wxWindowID id,
                          const std::string& url,
                          const wxPoint& pos,
                          const wxSize& size,
                          const std::string& backend,
                          unsigned int style,
                          const std::string& name)
{
    wxStringWebViewFactoryMap::iterator iter = FindFactory(backend);

    if(iter == m_factoryMap.end())
        return nullptr;
    else
        return (*iter).second->Create(parent, id, url, pos, size, style, name);

}

// static
void wxWebView::RegisterFactory(const wxString& backend,
                                std::shared_ptr<wxWebViewFactory> factory)
{
    m_factoryMap[backend] = factory;
}

// static
bool wxWebView::IsBackendAvailable(const wxString& backend)
{
    wxStringWebViewFactoryMap::iterator iter = FindFactory(backend);
    if (iter != m_factoryMap.end())
        return iter->second->IsAvailable();
    else
        return false;
}

wxVersionInfo wxWebView::GetBackendVersionInfo(const wxString& backend)
{
    wxStringWebViewFactoryMap::iterator iter = FindFactory(backend);
    if (iter != m_factoryMap.end())
        return iter->second->GetVersionInfo();
    else
        return {};
}

// static
wxStringWebViewFactoryMap::iterator wxWebView::FindFactory(const std::string& backend)
{
    // Initialise the map, it checks internally for existing factories
    InitFactoryMap();

#ifdef WX_WINDOWS
    // Use edge as default backend on MSW if available
    if (backend.empty())
    {
        return m_factoryMap.find(wxWebViewBackendEdge);
    }
    else
#endif
        return m_factoryMap.find(backend);
}

// static
void wxWebView::InitFactoryMap()
{
#ifdef WX_WINDOWS
    if (m_factoryMap.find(wxWebViewBackendEdge) == m_factoryMap.end())
        RegisterFactory(wxWebViewBackendEdge, std::shared_ptr<wxWebViewFactory>
        (new wxWebViewFactoryEdge));
#else
    if(m_factoryMap.find(wxWebViewBackendWebKit) == m_factoryMap.end())
        RegisterFactory(wxWebViewBackendWebKit, std::shared_ptr<wxWebViewFactory>
                                                       (new wxWebViewFactoryWebKit));
#endif
}

#endif // wxUSE_WEBVIEW
