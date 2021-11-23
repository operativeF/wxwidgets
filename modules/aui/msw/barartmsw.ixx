/////////////////////////////////////////////////////////////////////////////
// Name:        wx/aui/barartmsw.h
// Purpose:     Interface of wxAuiMSWToolBarArt
// Author:      Tobias Taschner
// Created:     2015-09-22
// Copyright:   (c) 2015 wxWidgets development team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/dc.h"
#include "wx/font.h"
#include "wx/window.h"

export module WX.AUI.ToolBarArt;

export import WX.AUI.ToolBarArt.Generic;
import WX.AUI.ToolBar.Item;

import Utils.Geometry;

export
{

class wxAuiMSWToolBarArt : public wxAuiGenericToolBarArt
{
public:
    wxAuiMSWToolBarArt();

    wxAuiToolBarArt* Clone() override;

    void DrawBackground(
        wxDC& dc,
        wxWindow* wnd,
        const wxRect& rect) override;

    void DrawLabel(
        wxDC& dc,
        wxWindow* wnd,
        const wxAuiToolBarItem& item,
        const wxRect& rect) override;

    void DrawButton(
        wxDC& dc,
        wxWindow* wnd,
        const wxAuiToolBarItem& item,
        const wxRect& rect) override;

    void DrawDropDownButton(
        wxDC& dc,
        wxWindow* wnd,
        const wxAuiToolBarItem& item,
        const wxRect& rect) override;

    void DrawControlLabel(
        wxDC& dc,
        wxWindow* wnd,
        const wxAuiToolBarItem& item,
        const wxRect& rect) override;

    void DrawSeparator(
        wxDC& dc,
        wxWindow* wnd,
        const wxRect& rect) override;

    void DrawGripper(
        wxDC& dc,
        wxWindow* wnd,
        const wxRect& rect) override;

    void DrawOverflowButton(
        wxDC& dc,
        wxWindow* wnd,
        const wxRect& rect,
        int state) override;

    wxSize GetLabelSize(
        wxDC& dc,
        wxWindow* wnd,
        const wxAuiToolBarItem& item) override;

    wxSize GetToolSize(
        wxDC& dc,
        wxWindow* wnd,
        const wxAuiToolBarItem& item) override;

    int GetElementSize(int element) override;
    void SetElementSize(int elementId, int size) override;

    int ShowDropDown(wxWindow* wnd,
        const wxAuiToolBarItemArray& items) override;

private:
    wxSize m_buttonSize;
    bool m_themed;
};

using wxAuiDefaultToolBarArt = wxAuiMSWToolBarArt;

} // export
