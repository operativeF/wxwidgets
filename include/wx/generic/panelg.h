/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/panelg.h
// Purpose:     wxPanel: a container for child controls
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_PANELG_H_
#define _WX_GENERIC_PANELG_H_

#include "wx/bitmap.h"

struct WXDLLIMPEXP_CORE wxPanel : public wxPanelBase
{
    wxPanel() = default;

    // Constructor
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

	wxClassInfo *wxGetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_GENERIC_PANELG_H_
