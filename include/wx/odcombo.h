/////////////////////////////////////////////////////////////////////////////
// Name:        wx/odcombo.h
// Purpose:     wxOwnerDrawnComboBox and wxVListBoxPopup
// Author:      Jaakko Salli
// Modified by:
// Created:     Apr-30-2006
// Copyright:   (c) Jaakko Salli
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_ODCOMBO_H_
#define _WX_ODCOMBO_H_

#if wxUSE_ODCOMBOBOX

#include "wx/combo.h"
#include "wx/ctrlsub.h"
#include "wx/vlbox.h"
#include "wx/timer.h"

import Utils.Geometry;

import <string>;
import <string_view>;
import <vector>;

//
// New window styles for wxOwnerDrawnComboBox
//
enum
{
    // Double-clicking cycles item if wxCB_READONLY is also used.
    wxODCB_DCLICK_CYCLES            = wxCC_SPECIAL_DCLICK,

    // If used, control itself is not custom paint using callback.
    // Even if this is not used, writable combo is never custom paint
    // until SetCustomPaintWidth is called
    wxODCB_STD_CONTROL_PAINT        = 0x1000
};


//
// Callback flags (see wxOwnerDrawnComboBox::OnDrawItem)
//
enum wxOwnerDrawnComboBoxPaintingFlags
{
    // when set, we are painting the selected item in control,
    // not in the popup
    wxODCB_PAINTING_CONTROL         = 0x0001,


    // when set, we are painting an item which should have
    // focus rectangle painted in the background. Text colour
    // and clipping region are then appropriately set in
    // the default OnDrawBackground implementation.
    wxODCB_PAINTING_SELECTED        = 0x0002
};


// ----------------------------------------------------------------------------
// wxVListBoxComboPopup is a wxVListBox customized to act as a popup control.
//
// Notes:
//   wxOwnerDrawnComboBox uses this as its popup. However, it always derives
//   from native wxComboCtrl. If you need to use this popup with
//   wxGenericComboControl, then remember that vast majority of item manipulation
//   functionality is implemented in the wxVListBoxComboPopup class itself.
//
// ----------------------------------------------------------------------------


class wxVListBoxComboPopup : public wxVListBox,
                                             public wxComboPopup
{
    friend class wxOwnerDrawnComboBox;
public:
    ~wxVListBoxComboPopup();

    [[maybe_unused]] bool Create(wxWindow* parent) override;
    void SetFocus() override;
    wxWindow *GetControl() override { return this; }
    void SetStringValue( const std::string& value ) override;
    std::string GetStringValue() const override;

    // more customization
    void OnPopup() override;
    wxSize GetAdjustedSize( int minWidth, int prefHeight, int maxHeight ) override;
    void PaintComboControl( wxDC& dc, const wxRect& rect ) override;
    void OnComboKeyEvent( wxKeyEvent& event ) override;
    void OnComboCharEvent( wxKeyEvent& event ) override;
    void OnComboDoubleClick() override;
    bool LazyCreate() override;
    bool FindItem(std::string_view item, std::string* trueItem) override;
    virtual void OnDPIChanged(wxDPIChangedEvent& event);

    // Item management
    void SetSelection( int item );
    void Insert( const wxString& item, int pos );
    int Append(const wxString& item);
    void Clear();
    void Delete( unsigned int item );
    void SetItemClientData(unsigned int n, void* clientData, wxClientDataType clientDataItemsType);
    void *GetItemClientData(unsigned int n) const;
    void SetString( int item, const std::string& str );
    std::string GetString( int item ) const;
    unsigned int GetCount() const;
    int FindString(std::string_view s, bool bCase = false) const;
    int GetSelection() const;

    //void Populate( int n, const wxString choices[] );
    void Populate( const std::vector<std::string>& choices );
    void ClearClientDatas();

    // helpers
    int GetItemAtPosition( const wxPoint& pos ) { return HitTest(pos); }
    wxCoord GetTotalHeight() const { return EstimateTotalHeight(); }
    wxCoord GetLineHeight(int line) const { return OnGetRowHeight(line); }

protected:

    // Called by OnComboDoubleClick and OnCombo{Key,Char}Event
    bool HandleKey( int keycode, bool saturate, wxChar keychar = 0 );

    // sends combobox select event from the parent combo control
    void SendComboBoxEvent( int selection );

    // gets value, sends event and dismisses
    void DismissWithEvent();

    // OnMeasureItemWidth will be called on next GetAdjustedSize.
    void ItemWidthChanged(unsigned int item)
    {
        m_widths[item] = -1;
        m_widthsDirty = true;
    }

    // Callbacks for drawing and measuring items. Override in a derived class for
    // owner-drawnness. Font, background and text colour have been prepared according
    // to selection, focus and such.
    //
    // item: item index to be drawn, may be wxNOT_FOUND when painting combo control itself
    //       and there is no valid selection
    // flags: wxODCB_PAINTING_CONTROL is set if painting to combo control instead of list
    //
    // NOTE: If wxVListBoxComboPopup is used with a wxComboCtrl class not derived from
    //       wxOwnerDrawnComboBox, this method must be overridden.
    virtual void OnDrawItem( wxDC& dc, const wxRect& rect, int item, unsigned int flags) const;

    // This is same as in wxVListBox
    wxCoord OnMeasureItem( size_t item ) const override;

    // Return item width, or -1 for calculating from text extent (default)
    virtual wxCoord OnMeasureItemWidth( size_t item ) const;

    // Draw item and combo control background. Flags are same as with OnDrawItem.
    // NB: Can't use name OnDrawBackground because of virtual function hiding warnings.
    virtual void OnDrawBg(wxDC& dc, const wxRect& rect, int item, unsigned int flags) const;

    // Additional wxVListBox implementation (no need to override in derived classes)
    void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const override;
    void OnDrawBackground(wxDC& dc, const wxRect& rect, size_t n) const override;

    // filter mouse move events happening outside the list box
    // move selection with cursor
    void OnMouseMove(wxMouseEvent& event);
    void OnKey(wxKeyEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnLeftClick(wxMouseEvent& event);

    // Return the widest item width (recalculating it if necessary)
    int GetWidestItemWidth() { CalcWidths(); return m_widestWidth; }

    // Return the index of the widest item (recalculating it if necessary)
    int GetWidestItem() { CalcWidths(); return m_widestItem; }

    // Stop partial completion (when some other event occurs)
    void StopPartialCompletion();

    std::vector<std::string>    m_strings;
    std::vector<void*>          m_clientDatas;

    wxFont                  m_useFont;

    //wxString                m_stringValue; // displayed text (may be different from m_strings[m_value])
    int                     m_value{-1}; // selection
    int                     m_itemHover{-1}; // on which item the cursor is
    int                     m_itemHeight{0}; // default item height (calculate from font size
                                          // and used in the absence of callback)
    wxClientDataType        m_clientDataItemsType{wxClientDataType::None};

private:

    // Cached item widths (in pixels).
    std::vector<int>           m_widths;

    // Width of currently widest item.
    int                     m_widestWidth{0};

    // Index of currently widest item.
    int                     m_widestItem{-1};

    // Measure some items in next GetAdjustedSize?
    bool                    m_widthsDirty{false};

    // Find widest item in next GetAdjustedSize?
    bool                    m_findWidest{false};

    // has the mouse been released on this control?
    bool                    m_clicked;

    // Recalculate widths if they are dirty
    void CalcWidths();

    // Partial completion string
    wxString                m_partialCompletionString;

    wxString                m_stringValue;

#if wxUSE_TIMER
    // Partial completion timer
    wxTimer                 m_partialCompletionTimer;
#endif // wxUSE_TIMER

    wxDECLARE_EVENT_TABLE();
};


// ----------------------------------------------------------------------------
// wxOwnerDrawnComboBox: a generic wxComboBox that allows custom paint items
// in addition to many other types of customization already allowed by
// the wxComboCtrl.
// ----------------------------------------------------------------------------

class wxOwnerDrawnComboBox :
    public wxWindowWithItems<wxComboCtrl, wxItemContainer>
{
    //friend class wxComboPopupWindow;
    friend class wxVListBoxComboPopup;
public:

    
    wxOwnerDrawnComboBox() = default;

    wxOwnerDrawnComboBox(wxWindow *parent,
                         wxWindowID id,
                         const wxString& value,
                         const wxPoint& pos,
                         const wxSize& size,
                         int n,
                         const wxString choices[],
                         unsigned int style = 0,
                         const wxValidator& validator = wxDefaultValidator,
                         std::string_view name = wxComboCtrlNameStr)
    {
        


        Create(parent, id, value, pos, size, n,
                     choices, style, validator, name);
    }

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& value = {},
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const wxValidator& validator = wxDefaultValidator,
                std::string_view name = wxComboCtrlNameStr);

    wxOwnerDrawnComboBox(wxWindow *parent,
                         wxWindowID id,
                         const wxString& value = {},
                         const wxPoint& pos = wxDefaultPosition,
                         const wxSize& size = wxDefaultSize,
                         const std::vector<std::string>& choices = {},
                         unsigned int style = 0,
                         const wxValidator& validator = wxDefaultValidator,
                         std::string_view name = wxComboCtrlNameStr);

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& value,
                const wxPoint& pos,
                const wxSize& size,
                int n,
                const wxString choices[],
                unsigned int style = 0,
                const wxValidator& validator = wxDefaultValidator,
                std::string_view name = wxComboCtrlNameStr);

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& value,
                const wxPoint& pos,
                const wxSize& size,
                const std::vector<std::string>& choices,
                unsigned int style = 0,
                const wxValidator& validator = wxDefaultValidator,
                std::string_view name = wxComboCtrlNameStr);

    ~wxOwnerDrawnComboBox();

    // Prevent app from using wxComboPopup
    void SetPopupControl(wxVListBoxComboPopup* popup)
    {
        DoSetPopupControl(popup);
    }

    // wxControlWithItems methods
    size_t GetCount() const override;
    std::string GetString(unsigned int n) const override;
    void SetString(unsigned int n, const std::string& s) override;
    int FindString(std::string_view s, bool bCase = false) const override;
    virtual void Select(int n);
    int GetSelection() const override;

    // See wxComboBoxBase discussion of IsEmpty().
    bool IsListEmpty() const { return wxItemContainer::IsEmpty(); }
    bool IsTextEmpty() const { return wxTextEntry::IsEmpty(); }

    // Override these just to maintain consistency with virtual methods
    // between classes.
    void Clear() override;
    void GetSelection(long *from, long *to) const override;

    void SetSelection(int n) override { Select(n); }


    // Prevent a method from being hidden
    void SetSelection(long from, long to) override
    {
        wxComboCtrl::SetSelection(from,to);
    }

    // Return the widest item width (recalculating it if necessary)
    virtual int GetWidestItemWidth() { EnsurePopupControl(); return GetVListBoxComboPopup()->GetWidestItemWidth(); }

    // Return the index of the widest item (recalculating it if necessary)
    virtual int GetWidestItem() { EnsurePopupControl(); return GetVListBoxComboPopup()->GetWidestItem(); }

    bool IsSorted() const override { return HasFlag(wxCB_SORT); }

protected:
    void DoClear() override;
    void DoDeleteOneItem(unsigned int n) override;

    // Callback for drawing. Font, background and text colour have been
    // prepared according to selection, focus and such.
    // item: item index to be drawn, may be wxNOT_FOUND when painting combo control itself
    //       and there is no valid selection
    // flags: wxODCB_PAINTING_CONTROL is set if painting to combo control instead of list
    virtual void OnDrawItem( wxDC& dc, const wxRect& rect, int item, unsigned int flags ) const;

    // Callback for item height, or -1 for default
    virtual wxCoord OnMeasureItem( size_t item ) const;

    // Callback for item width, or -1 for default/undetermined
    virtual wxCoord OnMeasureItemWidth( size_t item ) const;

    // override base implementation so we can return the size for the
    // largest item
    wxSize DoGetBestSize() const override;

    // Callback for background drawing. Flags are same as with
    // OnDrawItem.
    virtual void OnDrawBackground( wxDC& dc, const wxRect& rect, int item, unsigned int flags ) const;

    // NULL popup can be used to indicate default interface
    void DoSetPopupControl(wxComboPopup* popup) override;

    // clears all allocated client data
    void ClearClientDatas();

    wxVListBoxComboPopup* GetVListBoxComboPopup() const
    {
        return (wxVListBoxComboPopup*) m_popupInterface;
    }

    int DoInsertItems(const std::vector<std::string>& items,
                              unsigned int pos,
                              void **clientData, wxClientDataType type) override;
    void DoSetItemClientData(unsigned int n, void* clientData) override;
    void* DoGetItemClientData(unsigned int n) const override;

    // temporary storage for the initial choices
    //const wxString*         m_baseChoices;
    //int                     m_baseChoicesCount;
    std::vector<std::string>  m_initChs;

private:
    

    wxDECLARE_EVENT_TABLE();

    wxDECLARE_DYNAMIC_CLASS(wxOwnerDrawnComboBox);
};


#endif // wxUSE_ODCOMBOBOX

#endif
    // _WX_ODCOMBO_H_
