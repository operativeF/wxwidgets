/////////////////////////////////////////////////////////////////////////////
// Name:        include/wx/osx/webkit.h
// Purpose:     wxWebViewWebKit - embeddable web kit control,
//                             OS X implementation of web view component
// Author:      Jethro Grassie / Kevin Ollivier / Marianne Gagnon
// Modified by:
// Created:     2004-4-16
// Copyright:   (c) Jethro Grassie / Kevin Ollivier / Marianne Gagnon
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_WEBKIT_H
#define _WX_WEBKIT_H

#include "wx/defs.h"

#if wxUSE_WEBVIEW && wxUSE_WEBVIEW_WEBKIT && defined(__WXOSX__)

#include "wx/control.h"
#include "wx/webview.h"

#include "wx/osx/core/objcid.h"

// ----------------------------------------------------------------------------
// Web Kit Control
// ----------------------------------------------------------------------------

WX_DECLARE_STRING_HASH_MAP(wxSharedPtr<wxWebViewHandler>, wxStringToWebHandlerMap);

class WXDLLIMPEXP_WEBVIEW wxWebViewWebKit : public wxWebView
{
public:
    wxDECLARE_DYNAMIC_CLASS(wxWebViewWebKit);

    wxWebViewWebKit() {}
    wxWebViewWebKit(wxWindow *parent,
                    wxWindowID winID = wxID_ANY,
                    const wxString& strURL = wxASCII_STR(wxWebViewDefaultURLStr),
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize, long style = 0,
                    const wxString& name = wxASCII_STR(wxWebViewNameStr))
    {
        Create(parent, winID, strURL, pos, size, style, name);
    }
    bool Create(wxWindow *parent,
                wxWindowID winID = wxID_ANY,
                const wxString& strURL = wxASCII_STR(wxWebViewDefaultURLStr),
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize, long style = 0,
                const wxString& name = wxASCII_STR(wxWebViewNameStr)) override;
    virtual ~wxWebViewWebKit();

    bool CanGoBack() const override;
    bool CanGoForward() const override;
    void GoBack() override;
    void GoForward() override;
    void Reload(wxWebViewReloadFlags flags = wxWEBVIEW_RELOAD_DEFAULT) override;
    void Stop() override;

    void Print() override;

    void LoadURL(const wxString& url) override;
    wxString GetCurrentURL() const override;
    wxString GetCurrentTitle() const override;
    float GetZoomFactor() const override;
    void SetZoomFactor(float zoom) override;

    void SetZoomType(wxWebViewZoomType zoomType) override;
    wxWebViewZoomType GetZoomType() const override;
    bool CanSetZoomType(wxWebViewZoomType type) const override;

    bool IsBusy() const override;

    bool IsAccessToDevToolsEnabled() const override;
    void EnableAccessToDevTools(bool enable = true) override;
    bool SetUserAgent(const wxString& userAgent) override;

    //History functions
    void ClearHistory() override;
    void EnableHistory(bool enable = true) override;
    wxVector<wxSharedPtr<wxWebViewHistoryItem> > GetBackwardHistory() override;
    wxVector<wxSharedPtr<wxWebViewHistoryItem> > GetForwardHistory() override;
    void LoadHistoryItem(wxSharedPtr<wxWebViewHistoryItem> item) override;

    void Paste() override;

    //Undo / redo functionality
    bool CanUndo() const override;
    bool CanRedo() const override;
    void Undo() override;
    void Redo() override;

    //Editing functions
    void SetEditable(bool enable = true) override;
    bool IsEditable() const override;

    bool RunScript(const wxString& javascript, wxString* output = NULL) const override;
    bool AddScriptMessageHandler(const wxString& name) override;
    bool RemoveScriptMessageHandler(const wxString& name) override;
    virtual bool AddUserScript(const wxString& javascript,
        wxWebViewUserScriptInjectionTime injectionTime = wxWEBVIEW_INJECT_AT_DOCUMENT_START) override;
    void RemoveAllUserScripts() override;

    //Virtual Filesystem Support
    void RegisterHandler(wxSharedPtr<wxWebViewHandler> handler) override;

    void* GetNativeBackend() const override { return m_webView; }

protected:
    void DoSetPage(const wxString& html, const wxString& baseUrl) override;

    wxDECLARE_EVENT_TABLE();

private:
    OSXWebViewPtr m_webView;
    wxStringToWebHandlerMap m_handlers;
    wxString m_customUserAgent;

    WX_NSObject m_navigationDelegate;
    WX_NSObject m_UIDelegate;

    bool RunScriptSync(const wxString& javascript, wxString* output = NULL) const;
};

class WXDLLIMPEXP_WEBVIEW wxWebViewFactoryWebKit : public wxWebViewFactory
{
public:
    wxWebView* Create() override { return new wxWebViewWebKit; }
    virtual wxWebView* Create(wxWindow* parent,
                              wxWindowID id,
                              const wxString& url = wxWebViewDefaultURLStr,
                              const wxPoint& pos = wxDefaultPosition,
                              const wxSize& size = wxDefaultSize,
                              long style = 0,
                              const wxString& name = wxASCII_STR(wxWebViewNameStr)) override
    { return new wxWebViewWebKit(parent, id, url, pos, size, style, name); }
    wxVersionInfo GetVersionInfo() override;
};

#endif // wxUSE_WEBVIEW && wxUSE_WEBVIEW_WEBKIT

#endif // _WX_WEBKIT_H_
