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

import <string>;

class wxBrush;

// ----------------------------------------------------------------------------
// wxPanel
// ----------------------------------------------------------------------------

struct wxPanel : public wxPanelBase
{
    wxPanel() = default;

    wxPanel(wxWindow *parent,
            wxWindowID winid = wxID_ANY,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            unsigned int style = wxTAB_TRAVERSAL | wxNO_BORDER,
            const std::string& name = wxPanelNameStr)
    {
        Create(parent, winid, pos, size, style, name);
    }

    wxPanel& operator=(wxPanel&&) = delete;

	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_MSW_PANEL_H_
