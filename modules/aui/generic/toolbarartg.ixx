module;

#include "wx/dc.h"
#include "wx/font.h"
#include "wx/window.h"
#include "wx/menu.h"
#include "wx/pen.h"

export module WX.AUI.ToolBarArt.Generic;

import WX.AUI.ToolBarArt.Base;

import WX.AUI.ToolBar.Item;

import WX.AUI.DockArt; // FIXME: imported for wxAuiBitmapFromBits utility function
import WX.AUI.Flags;
import WX.AUI.FrameManager;
import WX.AUI.ToolBarArt.Base;

import WX.Utils.Settings;

import Utils.Geometry;

export
{

class wxAuiGenericToolBarArt : public wxAuiToolBarArt
{
public:
    wxAuiGenericToolBarArt();
    ~wxAuiGenericToolBarArt();

    wxAuiToolBarArt* Clone() override;
    void SetFlags(unsigned int flags) override;
    unsigned int GetFlags() override;
    void SetFont(const wxFont& font) override;
    wxFont GetFont() override;
    void SetTextOrientation(int orientation) override;
    int GetTextOrientation() override;

    void DrawBackground(
                wxDC& dc,
                wxWindow* wnd,
                const wxRect& rect) override;

    void DrawPlainBackground(wxDC& dc,
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

    void UpdateColoursFromSystem() override;

protected:

    wxBitmap m_buttonDropDownBmp;
    wxBitmap m_disabledButtonDropDownBmp;
    wxBitmap m_overflowBmp;
    wxBitmap m_disabledOverflowBmp;
    wxColour m_baseColour;
    wxColour m_highlightColour;
    wxFont m_font;
    unsigned int m_flags;
    int m_textOrientation;

    wxPen m_gripperPen1;
    wxPen m_gripperPen2;
    wxPen m_gripperPen3;

    // These values are in DIPs and not physical pixels.
    int m_separatorSize;
    int m_gripperSize;
    int m_overflowSize;
    int m_dropdownSize;
};

#if !(defined(__WXMSW__) && wxUSE_UXTHEME)
using wxAuiDefaultToolBarArt = wxAuiGenericToolBarArt;
#endif

} // export
