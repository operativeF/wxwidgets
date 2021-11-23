module;

#include "wx/dc.h"
#include "wx/font.h"
#include "wx/window.h"

export module WX.AUI.ToolBarArt.Base;

import WX.AUI.ToolBar.Item;

export
{
   
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

};

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
