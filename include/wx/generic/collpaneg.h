/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/collpaneg.h
// Purpose:     wxGenericCollapsiblePane
// Author:      Francesco Montorsi
// Modified by:
// Created:     8/10/2006
// Copyright:   (c) Francesco Montorsi
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_COLLAPSABLE_PANE_H_GENERIC_
#define _WX_COLLAPSABLE_PANE_H_GENERIC_

// forward declared
class wxCollapsibleHeaderCtrl;

#include "wx/containr.h"

#include <string>

// ----------------------------------------------------------------------------
// wxGenericCollapsiblePane
// ----------------------------------------------------------------------------

class wxGenericCollapsiblePane :
    public wxNavigationEnabled<wxCollapsiblePaneBase>
{
public:
    wxGenericCollapsiblePane() = default;

    wxGenericCollapsiblePane(wxWindow *parent,
                        wxWindowID winid,
                        const std::string& label,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        unsigned int style = wxCP_DEFAULT_STYLE,
                        const wxValidator& val = wxDefaultValidator,
                        const std::string& name = wxCollapsiblePaneNameStr)
    {
        Create(parent, winid, label, pos, size, style, val, name);
    }

    ~wxGenericCollapsiblePane();

    bool Create(wxWindow *parent,
                wxWindowID winid,
                const std::string& label,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxCP_DEFAULT_STYLE,
                const wxValidator& val = wxDefaultValidator,
                const std::string& name = wxCollapsiblePaneNameStr);

    // public wxCollapsiblePane API
    void Collapse(bool collapse = true) override;
    void SetLabel(const std::string& label) override;

    bool IsCollapsed() const override
        { return m_pPane==nullptr || !m_pPane->IsShown(); }
    wxWindow *GetPane() const override
        { return m_pPane; }
    std::string GetLabel() const override;

    bool Layout() override;


    // for the generic collapsible pane only:
    wxControl* GetControlWidget() const
        { return (wxControl*)m_pButton; }

    // implementation only, don't use
    void OnStateChange(const wxSize& sizeNew);

protected:
    // overridden methods
    wxSize DoGetBestClientSize() const override;

    int GetBorder() const;

    // child controls
    wxCollapsibleHeaderCtrl *m_pButton{nullptr};
    wxWindow *m_pPane{nullptr};
    wxSizer *m_sz{nullptr};

private:
    // event handlers
    void OnButton(wxCommandEvent &ev);
    void OnSize(wxSizeEvent &ev);

    wxDECLARE_DYNAMIC_CLASS(wxGenericCollapsiblePane);
    wxDECLARE_EVENT_TABLE();
};

#endif // _WX_COLLAPSABLE_PANE_H_GENERIC_
