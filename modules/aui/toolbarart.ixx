module;

export module WX.AUI.ToolBarArt.Base;

import WX.AUI.ToolBar.Item;

import Utils.Geometry;

export
{

class wxDC;
class wxFont;
class wxWindow;

class wxAuiToolBarArt
{
public:
    virtual ~wxAuiToolBarArt() = default;

    virtual wxAuiToolBarArt* Clone() = 0;
    virtual void SetFlags(unsigned int flags) = 0;
    virtual unsigned int GetFlags() = 0;
    virtual void SetFont(const wxFont& font) = 0;
    virtual wxFont GetFont() = 0;
    virtual void SetTextOrientation(int orientation) = 0;
    virtual int GetTextOrientation() = 0;

    virtual void DrawBackground(
                        wxDC& dc,
                        wxWindow* wnd,
                        const wxRect& rect) = 0;

    virtual void DrawPlainBackground(
                                wxDC& dc,
                                wxWindow* wnd,
                                const wxRect& rect) = 0;

    virtual void DrawLabel(
                        wxDC& dc,
                        wxWindow* wnd,
                        const wxAuiToolBarItem& item,
                        const wxRect& rect) = 0;

    virtual void DrawButton(
                        wxDC& dc,
                        wxWindow* wnd,
                        const wxAuiToolBarItem& item,
                        const wxRect& rect) = 0;

    virtual void DrawDropDownButton(
                        wxDC& dc,
                        wxWindow* wnd,
                        const wxAuiToolBarItem& item,
                        const wxRect& rect) = 0;

    virtual void DrawControlLabel(
                        wxDC& dc,
                        wxWindow* wnd,
                        const wxAuiToolBarItem& item,
                        const wxRect& rect) = 0;

    virtual void DrawSeparator(
                        wxDC& dc,
                        wxWindow* wnd,
                        const wxRect& rect) = 0;

    virtual void DrawGripper(
                        wxDC& dc,
                        wxWindow* wnd,
                        const wxRect& rect) = 0;

    virtual void DrawOverflowButton(
                        wxDC& dc,
                        wxWindow* wnd,
                        const wxRect& rect,
                        int state) = 0;

    virtual wxSize GetLabelSize(
                        wxDC& dc,
                        wxWindow* wnd,
                        const wxAuiToolBarItem& item) = 0;

    virtual wxSize GetToolSize(
                        wxDC& dc,
                        wxWindow* wnd,
                        const wxAuiToolBarItem& item) = 0;

    // Note that these functions work with the size in DIPs, not physical
    // pixels.
    virtual int GetElementSize(int elementId) = 0;
    virtual void SetElementSize(int elementId, int size) = 0;

    virtual int ShowDropDown(
                        wxWindow* wnd,
                        const wxAuiToolBarItemArray& items) = 0;

    // Provide opportunity for subclasses to recalculate colours
    virtual void UpdateColoursFromSystem() {}
}; // export wxAuiToolBarArt

} // export
