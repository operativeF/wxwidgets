/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/collpane.h
// Purpose:     wxCollapsiblePane
// Author:      Francesco Montorsi
// Modified by:
// Created:     8/10/2006
// Copyright:   (c) Francesco Montorsi
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_COLLAPSABLE_PANEL_H_GTK_
#define _WX_COLLAPSABLE_PANEL_H_GTK_

// ----------------------------------------------------------------------------
// wxCollapsiblePane
// ----------------------------------------------------------------------------

class wxCollapsiblePane : public wxCollapsiblePaneBase
{
public:
    wxCollapsiblePane() { Init(); }

    wxCollapsiblePane(wxWindow *parent,
                        wxWindowID winid,
                        const wxString& label,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        long style = wxCP_DEFAULT_STYLE,
                        const wxValidator& val = wxDefaultValidator,
                        const wxString& name = wxASCII_STR(wxCollapsiblePaneNameStr))
    {
        Init();

        Create(parent, winid, label, pos, size, style, val, name);
    }

    bool Create(wxWindow *parent,
                wxWindowID winid,
                const wxString& label,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxCP_DEFAULT_STYLE,
                const wxValidator& val = wxDefaultValidator,
                const wxString& name = wxASCII_STR(wxCollapsiblePaneNameStr));

    void Collapse(bool collapse = true) override;
    bool IsCollapsed() const override;
    void SetLabel(const wxString& str) override;

    wxWindow *GetPane() const override { return m_pPane; }
    wxString GetLabel() const override { return m_strLabel; }

protected:
    wxSize DoGetBestSize() const override;

public:     // used by GTK callbacks
    bool m_bIgnoreNextChange;
    wxSize m_szCollapsed;

    wxWindow *m_pPane;

    // the button label without ">>" or "<<"
    wxString m_strLabel;

private:
    void Init()
    {
        m_bIgnoreNextChange = false;
    }

    void OnSize(wxSizeEvent&);
    void AddChildGTK(wxWindowGTK* child) override;
    GdkWindow *GTKGetWindow(wxArrayGdkWindows& windows) const override;

    wxDECLARE_DYNAMIC_CLASS(wxCollapsiblePane);
    wxDECLARE_EVENT_TABLE();
};

#endif // _WX_COLLAPSABLE_PANEL_H_GTK_
