/////////////////////////////////////////////////////////////////////////////
// Name:        wx/aui/tabartmsw.h
// Purpose:     wxAuiMSWTabArt declaration
// Author:      Tobias Taschner
// Created:     2015-09-26
// Copyright:   (c) 2015 wxWidgets development team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_AUI_TABARTMSW_H_
#define _WX_AUI_TABARTMSW_H_

#include "wx/geometry/rect.h"

#include <string>

class wxAuiMSWTabArt : public wxAuiGenericTabArt
{
public:

    wxAuiMSWTabArt();

    wxAuiTabArt* Clone() override;
    void SetSizingInfo(const wxSize& tabCtrlSize,
        size_t tabCount) override;

    void DrawBorder(
        wxDC& dc,
        wxWindow* wnd,
        const wxRect& rect) override;

    void DrawBackground(
        wxDC& dc,
        wxWindow* wnd,
        const wxRect& rect) override;

    void DrawTab(wxDC& dc,
        wxWindow* wnd,
        const wxAuiNotebookPage& pane,
        const wxRect& inRect,
        int closeButtonState,
        wxRect* outTabRect,
        wxRect* outButtonRect,
        int* xExtent) override;

    void DrawButton(
        wxDC& dc,
        wxWindow* wnd,
        const wxRect& inRect,
        int bitmapId,
        int buttonState,
        int orientation,
        wxRect* outRect) override;

    int GetIndentSize() override;

    int GetBorderWidth(
        wxWindow* wnd) override;

    int GetAdditionalBorderSpace(
        wxWindow* wnd) override;

    wxSize GetTabSize(
        wxDC& dc,
        wxWindow* wnd,
        const std::string& caption,
        const wxBitmap& bitmap,
        bool active,
        int closeButtonState,
        int* xExtent) override;

    int ShowDropDown(
        wxWindow* wnd,
        const wxAuiNotebookPageArray& items,
        int activeIdx) override;

    int GetBestTabCtrlSize(wxWindow* wnd,
        const wxAuiNotebookPageArray& pages,
        const wxSize& requiredBmpSize) override;

private:
    bool m_themed;
    wxSize m_closeBtnSize{wxDefaultSize};
    wxSize m_tabSize;
    int m_maxTabHeight{0};

    void InitSizes(wxWindow* wnd, wxDC& dc);

    bool IsThemed() const;
};

#endif // _WX_AUI_TABARTMSW_H_
