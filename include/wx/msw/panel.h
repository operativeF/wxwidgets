///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/panel.h
// Purpose:     wxMSW-specific wxPanel class.
// Author:      Vadim Zeitlin
// Created:     2011-03-18
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_PANEL_H_
#define _WX_MSW_PANEL_H_

class WXDLLIMPEXP_FWD_CORE wxBrush;

// ----------------------------------------------------------------------------
// wxPanel
// ----------------------------------------------------------------------------

struct WXDLLIMPEXP_CORE wxPanel : public wxPanelBase
{
    wxPanel() = default;

    wxPanel(wxWindow *parent,
            wxWindowID winid = wxID_ANY,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxTAB_TRAVERSAL | wxNO_BORDER,
            const std::string& name = wxPanelNameStr)
    {
        Create(parent, winid, pos, size, style, name);
    }

    wxPanel(const wxPanel&) = delete;
    wxPanel& operator=(const wxPanel&) = delete;
    wxPanel(wxPanel&&) = default;
    wxPanel& operator=(wxPanel&&) = default;
    ~wxPanel() = default;

	wxClassInfo *GetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_MSW_PANEL_H_
