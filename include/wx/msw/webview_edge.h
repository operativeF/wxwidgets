/////////////////////////////////////////////////////////////////////////////
// Name:        include/wx/msw/webview_edge.h
// Purpose:     wxMSW Edge Chromium wxWebView backend
// Author:      Markus Pingel
// Created:     2019-12-15
// Copyright:   (c) 2019 wxWidgets development team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef wxWebViewEdge_H
#define wxWebViewEdge_H

#if wxUSE_WEBVIEW && wxUSE_WEBVIEW_EDGE && defined(__WXMSW__)

#include "wx/control.h"
#include "wx/webview.h"

class wxWebViewEdgeImpl;

class wxWebViewEdge : public wxWebView
{
public:

    wxWebViewEdge();

    wxWebViewEdge(wxWindow* parent,
        wxWindowID id,
        const std::string& url = wxWebViewDefaultURLStr,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        unsigned int style = 0,
        const std::string& name = wxWebViewNameStr);

    ~wxWebViewEdge();

    [[maybe_unused]] bool Create(wxWindow* parent,
        wxWindowID id,
        const std::string& url = wxWebViewDefaultURLStr,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        unsigned int style = 0,
        const std::string& name = wxWebViewNameStr) override;

    void LoadURL(const wxString& url) override;
    void LoadHistoryItem(std::shared_ptr<wxWebViewHistoryItem> item) override;
    std::vector<std::shared_ptr<wxWebViewHistoryItem> > GetBackwardHistory() override;
    std::vector<std::shared_ptr<wxWebViewHistoryItem> > GetForwardHistory() override;

    bool CanGoForward() const override;
    bool CanGoBack() const override;
    void GoBack() override;
    void GoForward() override;
    void ClearHistory() override;
    void EnableHistory(bool enable = true) override;
    void Stop() override;
    void Reload(wxWebViewReloadFlags flags = wxWebViewReloadFlags::Default) override;

    bool IsBusy() const override;
    wxString GetCurrentURL() const override;
    wxString GetCurrentTitle() const override;

    void SetZoomType(wxWebViewZoomType) override;
    wxWebViewZoomType GetZoomType() const override;
    bool CanSetZoomType(wxWebViewZoomType type) const override;

    void Print() override;

    float GetZoomFactor() const override;
    void SetZoomFactor(float zoom) override;

    //Undo / redo functionality
    bool CanUndo() const override;
    bool CanRedo() const override;
    void Undo() override;
    void Redo() override;

    //Editing functions
    void SetEditable(bool enable = true) override;
    bool IsEditable() const override;

    void EnableContextMenu(bool enable = true) override;
    bool IsContextMenuEnabled() const override;

    void EnableAccessToDevTools(bool enable = true) override;
    bool IsAccessToDevToolsEnabled() const override;

    bool SetUserAgent(const wxString& userAgent) override;

    bool RunScript(const wxString& javascript, wxString* output = NULL) const override;
    bool AddScriptMessageHandler(const wxString& name) override;
    bool RemoveScriptMessageHandler(const wxString& name) override;
    virtual bool AddUserScript(const wxString& javascript,
        wxWebViewUserScriptInjectionTime injectionTime = wxWebViewUserScriptInjectionTime::DocStart) override;
    void RemoveAllUserScripts() override;

    void RegisterHandler(std::shared_ptr<wxWebViewHandler> handler) override;

    void* GetNativeBackend() const override;

    static void MSWSetBrowserExecutableDir(const wxString& path);

protected:
    void DoSetPage(const wxString& html, const wxString& baseUrl) override;

private:
    wxWebViewEdgeImpl* m_impl;

    void OnSize(wxSizeEvent& event);

    void OnTopLevelParentIconized(wxIconizeEvent& event);

    bool RunScriptSync(const wxString& javascript, wxString* output = NULL) const;
};

class wxWebViewFactoryEdge : public wxWebViewFactory
{
public:
    wxWebView* Create() override { return new wxWebViewEdge; }
    virtual wxWebView* Create(wxWindow* parent,
        wxWindowID id,
        const std::string& url = wxWebViewDefaultURLStr,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        unsigned int style = 0,
        const std::string& name = wxWebViewNameStr) override
    {
        return new wxWebViewEdge(parent, id, url, pos, size, style, name);
    }
    bool IsAvailable() override;
    wxVersionInfo GetVersionInfo() override;
};

#endif // wxUSE_WEBVIEW && wxUSE_WEBVIEW_EDGE && defined(__WXMSW__)

#endif // wxWebViewEdge_H
