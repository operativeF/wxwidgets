module;

#include "wx/bitmap.h"
#include "wx/sizer.h"
#include "wx/window.h"

export module WX.AUI.ToolBar.Item;

import <string>;

export
{

class wxAuiToolBarItem
{
    friend class wxAuiToolBar;

public:
    void SetWindow(wxWindow* w) { m_window = w; }
    wxWindow* GetWindow() { return m_window; }

    void SetId(int newId) { m_toolId = newId; }
    int GetId() const { return m_toolId; }

    void SetKind(int newKind) { m_kind = newKind; }
    int GetKind() const { return m_kind; }

    void SetState(int newState) { m_state = newState; }
    int GetState() const { return m_state; }

    void SetSizerItem(wxSizerItem* s) { m_sizerItem = s; }
    wxSizerItem* GetSizerItem() const { return m_sizerItem; }

    void SetLabel(const std::string& s) { m_label = s; }
    const std::string& GetLabel() const { return m_label; }

    void SetBitmap(const wxBitmap& bmp) { m_bitmap = bmp; }
    const wxBitmap& GetBitmap() const { return m_bitmap; }

    void SetDisabledBitmap(const wxBitmap& bmp) { m_disabledBitmap = bmp; }
    const wxBitmap& GetDisabledBitmap() const { return m_disabledBitmap; }

    void SetHoverBitmap(const wxBitmap& bmp) { m_hoverBitmap = bmp; }
    const wxBitmap& GetHoverBitmap() const { return m_hoverBitmap; }

    void SetShortHelp(const std::string& s) { m_shortHelp = s; }
    const std::string& GetShortHelp() const { return m_shortHelp; }

    void SetLongHelp(const std::string& s) { m_longHelp = s; }
    const std::string& GetLongHelp() const { return m_longHelp; }

    void SetMinSize(const wxSize& s) { m_minSize = s; }
    const wxSize& GetMinSize() const { return m_minSize; }

    void SetSpacerPixels(int s) { m_spacerPixels = s; }
    int GetSpacerPixels() const { return m_spacerPixels; }

    void SetProportion(int p) { m_proportion = p; }
    int GetProportion() const { return m_proportion; }

    void SetActive(bool b) { m_active = b; }
    bool IsActive() const { return m_active; }

    void SetHasDropDown(bool b)
    {
        wxCHECK_RET( !b || m_kind == wxITEM_NORMAL,
                     "Only normal tools can have drop downs" );

        m_dropDown = b;
    }

    bool HasDropDown() const { return m_dropDown; }

    void SetSticky(bool b) { m_sticky = b; }
    bool IsSticky() const { return m_sticky; }

    void SetUserData(long l) { m_userData = l; }
    long GetUserData() const { return m_userData; }

    void SetAlignment(int l) { m_alignment = l; }
    int GetAlignment() const { return m_alignment; }

    bool CanBeToggled() const
    {
        return m_kind == wxITEM_CHECK || m_kind == wxITEM_RADIO;
    }

private:
    std::string m_shortHelp;     // short help (for tooltip)
    std::string m_longHelp;      // long help (for status bar)
    std::string m_label;         // label displayed on the item

    wxBitmap m_bitmap;           // item's bitmap
    wxBitmap m_disabledBitmap;  // item's disabled bitmap
    wxBitmap m_hoverBitmap;     // item's hover bitmap

    wxWindow* m_window{nullptr};          // item's associated window
    wxSizerItem* m_sizerItem{nullptr};   // sizer item
    
    wxSize m_minSize;           // item's minimum size

    long m_userData{0};            // user-specified data

    int m_spacerPixels{0};         // size of a spacer
    int m_toolId{0};                // item's id
    int m_kind{wxITEM_NORMAL};                  // item's kind
    int m_state{0};                 // state
    int m_proportion{0};            // proportion
    int m_alignment{wxALIGN_CENTER};   // sizer alignment flag, defaults to wxCENTER, may be wxEXPAND or any other

    bool m_active{true};               // true if the item is currently active
    bool m_dropDown{true};             // true if the item has a dropdown button
    bool m_sticky{true};               // overrides button states if true (always active)
};

using wxAuiToolBarItemArray = std::vector<wxAuiToolBarItem>;

} // export
