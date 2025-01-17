///////////////////////////////////////////////////////////////////////////////
// Name:        wx/univ/panel.h
// Purpose:     wxUniversal-specific wxPanel class.
// Author:      Vadim Zeitlin
// Created:     2011-03-18
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_UNIV_PANEL_H_
#define _WX_UNIV_PANEL_H_

// ----------------------------------------------------------------------------
// wxPanel
// ----------------------------------------------------------------------------

struct wxPanel : public wxPanelBase
{
    wxPanel() { }

    wxPanel(wxWindow *parent,
            wxWindowID winid = wxID_ANY,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            unsigned int style = wxTAB_TRAVERSAL | wxNO_BORDER,
            const wxString& name = wxASCII_STR(wxPanelNameStr))
    {
        Create(parent, winid, pos, size, style, name);
    }

	wxPanel(const wxPanel&) = delete;
	wxPanel& operator=(const wxPanel&) = delete;

    virtual bool IsCanvasWindow() const { return true; }

	wxClassInfo *wxGetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_UNIV_PANEL_H_
