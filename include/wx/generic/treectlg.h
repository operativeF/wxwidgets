/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/treectlg.h
// Purpose:     wxTreeCtrl class
// Author:      Robert Roebling
// Modified by:
// Created:     01/02/97
// Copyright:   (c) 1997,1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _GENERIC_TREECTRL_H_
#define _GENERIC_TREECTRL_H_

#if wxUSE_TREECTRL

#include "wx/brush.h"
#include "wx/pen.h"
#include "wx/scrolwin.h"

import Utils.Geometry;

import <string>;

// -----------------------------------------------------------------------------
// forward declaration
// -----------------------------------------------------------------------------

class wxGenericTreeItem;

class wxTreeItemData;

class wxTreeRenameTimer;
class wxTreeFindTimer;
class wxTreeTextCtrl;
class wxTextCtrl;

// -----------------------------------------------------------------------------
// wxGenericTreeCtrl - the tree control
// -----------------------------------------------------------------------------

class wxGenericTreeCtrl : public wxTreeCtrlBase,
                                      public wxScrollHelper
{
public:
    wxGenericTreeCtrl() : wxScrollHelper(this) { Init(); }

    wxGenericTreeCtrl(wxWindow *parent, wxWindowID id = wxID_ANY,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               unsigned int style = wxTR_DEFAULT_STYLE,
               const wxValidator &validator = {},
               std::string_view name = wxTreeCtrlNameStr)
        : 
          wxScrollHelper(this)
    {
        Init();
        Create(parent, id, pos, size, style, validator, name);
    }

    ~wxGenericTreeCtrl();

	wxGenericTreeCtrl& operator=(wxGenericTreeCtrl&&) = delete;

    bool Create(wxWindow *parent, wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxTR_DEFAULT_STYLE,
                const wxValidator &validator = {},
                std::string_view name = wxTreeCtrlNameStr);

    
    // ----------------------------------

    unsigned int GetCount() const override;

    unsigned int GetIndent() const override { return m_indent; }
    void SetIndent(unsigned int indent) override;


    void SetImageList(wxImageList *imageList) override;
    void SetStateImageList(wxImageList *imageList) override;

    std::string GetItemText(const wxTreeItemId& item) const override;
    int GetItemImage(const wxTreeItemId& item,
                     wxTreeItemIcon which = wxTreeItemIcon_Normal) const override;
    wxTreeItemData *GetItemData(const wxTreeItemId& item) const override;
    wxColour GetItemTextColour(const wxTreeItemId& item) const override;
    wxColour GetItemBackgroundColour(const wxTreeItemId& item) const override;
    wxFont GetItemFont(const wxTreeItemId& item) const override;

    void SetItemText(const wxTreeItemId& item, const std::string& text) override;
    void SetItemImage(const wxTreeItemId& item,
                              int image,
                              wxTreeItemIcon which = wxTreeItemIcon_Normal) override;
    void SetItemData(const wxTreeItemId& item, wxTreeItemData *data) override;

    void SetItemHasChildren(const wxTreeItemId& item, bool has = true) override;
    void SetItemBold(const wxTreeItemId& item, bool bold = true) override;
    void SetItemDropHighlight(const wxTreeItemId& item, bool highlight = true) override;
    void SetItemTextColour(const wxTreeItemId& item, const wxColour& col) override;
    void SetItemBackgroundColour(const wxTreeItemId& item, const wxColour& col) override;
    void SetItemFont(const wxTreeItemId& item, const wxFont& font) override;

    bool IsVisible(const wxTreeItemId& item) const override;
    bool ItemHasChildren(const wxTreeItemId& item) const override;
    bool IsExpanded(const wxTreeItemId& item) const override;
    bool IsSelected(const wxTreeItemId& item) const override;
    bool IsBold(const wxTreeItemId& item) const override;

    size_t GetChildrenCount(const wxTreeItemId& item,
                                    bool recursively = true) const override;

    // navigation
    // ----------

    wxTreeItemId GetRootItem() const override { return m_anchor; }
    wxTreeItemId GetSelection() const override
    {
        wxASSERT_MSG( !HasFlag(wxTR_MULTIPLE),
                      "must use GetSelections() with this control" );

        return m_current;
    }
    size_t GetSelections(wxArrayTreeItemIds&) const override;
    wxTreeItemId GetFocusedItem() const override { return m_current; }

    void ClearFocusedItem() override;
    void SetFocusedItem(const wxTreeItemId& item) override;

    wxTreeItemId GetItemParent(const wxTreeItemId& item) const override;
    wxTreeItemId wxGetFirstChild(const wxTreeItemId& item,
                                       wxTreeItemIdValue& cookie) const override;
    wxTreeItemId GetNextChild(const wxTreeItemId& item,
                                      wxTreeItemIdValue& cookie) const override;
    wxTreeItemId GetLastChild(const wxTreeItemId& item) const override;
    wxTreeItemId wxGetNextSibling(const wxTreeItemId& item) const override;
    wxTreeItemId wxGetPrevSibling(const wxTreeItemId& item) const override;

    wxTreeItemId GetFirstVisibleItem() const override;
    wxTreeItemId GetNextVisible(const wxTreeItemId& item) const override;
    wxTreeItemId GetPrevVisible(const wxTreeItemId& item) const override;


    // operations
    // ----------

    wxTreeItemId AddRoot(const std::string& text,
                         int image = -1, int selectedImage = -1,
                         wxTreeItemData *data = nullptr) override;

    void Delete(const wxTreeItemId& item) override;
    void DeleteChildren(const wxTreeItemId& item) override;
    void DeleteAllItems() override;

    void Expand(const wxTreeItemId& item) override;
    void Collapse(const wxTreeItemId& item) override;
    void CollapseAndReset(const wxTreeItemId& item) override;
    void Toggle(const wxTreeItemId& item) override;

    void Unselect() override;
    void UnselectAll() override;
    void SelectItem(const wxTreeItemId& item, bool select = true) override;
    void SelectChildren(const wxTreeItemId& parent) override;

    void EnsureVisible(const wxTreeItemId& item) override;
    void ScrollTo(const wxTreeItemId& item) override;

    wxTextCtrl *EditLabel(const wxTreeItemId& item,
                          wxClassInfo* textCtrlClass = wxCLASSINFO(wxTextCtrl)) override;
    wxTextCtrl *GetEditControl() const override;
    void EndEditLabel(const wxTreeItemId& item,
                              bool discardChanges = false) override;

    void EnableBellOnNoMatch(bool on = true) override;

    void SortChildren(const wxTreeItemId& item) override;

    // items geometry
    // --------------

    bool GetBoundingRect(const wxTreeItemId& item,
                                 wxRect& rect,
                                 bool textOnly = false) const override;


    // this version specific methods
    // -----------------------------

    wxImageList *GetButtonsImageList() const { return m_imageListButtons; }
    void SetButtonsImageList(wxImageList *imageList);
    void AssignButtonsImageList(wxImageList *imageList);

    void SetDropEffectAboveItem( bool above = false ) { m_dropEffectAboveItem = above; }
    bool GetDropEffectAboveItem() const { return m_dropEffectAboveItem; }

    wxTreeItemId GetNext(const wxTreeItemId& item) const;

    // implementation only from now on

    // overridden base class virtuals
    bool SetBackgroundColour(const wxColour& colour) override;
    bool SetForegroundColour(const wxColour& colour) override;

    void Refresh(bool eraseBackground = true, const wxRect *rect = nullptr) override;

    bool SetFont( const wxFont &font ) override;
    void SetWindowStyleFlag(unsigned int styles) override;

    // callbacks
    void OnPaint( wxPaintEvent &event );
    void OnSetFocus( wxFocusEvent &event );
    void OnKillFocus( wxFocusEvent &event );
    void OnKeyDown( wxKeyEvent &event );
    void OnChar( wxKeyEvent &event );
    void OnMouse( wxMouseEvent &event );
    void OnGetToolTip( wxTreeEvent &event );
    void OnSize( wxSizeEvent &event );
    void OnInternalIdle( ) override;

    wxVisualAttributes GetDefaultAttributes() const override
    {
        return GetClassDefaultAttributes(GetWindowVariant());
    }

    static wxVisualAttributes
    GetClassDefaultAttributes(wxWindowVariant variant = wxWindowVariant::Normal);

    // implementation helpers
    void AdjustMyScrollbars();

    WX_FORWARD_TO_SCROLL_HELPER()

protected:
    friend class wxGenericTreeItem;
    friend class wxTreeRenameTimer;
    friend class wxTreeFindTimer;
    friend class wxTreeTextCtrl;

    wxFont               m_normalFont;
    wxFont               m_boldFont;

    wxGenericTreeItem   *m_anchor;
    wxGenericTreeItem   *m_current,
                        *m_key_current,
                        // A hint to select a parent item after deleting a child
                        *m_select_me;
    unsigned short       m_indent;
    int                  m_lineHeight;
    wxPen                m_dottedPen;
    wxBrush              m_hilightBrush,
                         m_hilightUnfocusedBrush;
    bool                 m_hasFocus;
    bool                 m_dirty;
    bool                 m_ownsImageListButtons;
    bool                 m_isDragging; // true between BEGIN/END drag events
    bool                 m_lastOnSame;  // last click on the same item as prev
    wxImageList         *m_imageListButtons;

    int                  m_dragCount;
    wxPoint              m_dragStart;
    wxGenericTreeItem   *m_dropTarget;
    wxCursor             m_oldCursor;  // cursor is changed while dragging
    wxGenericTreeItem   *m_oldSelection;
    wxGenericTreeItem   *m_underMouse; // for visual effects

    enum class DNDEffect
    {
        None,
        Border,
        Above,
        Below
    };
    
    DNDEffect m_dndEffect;
    
    wxGenericTreeItem   *m_dndEffectItem;

    wxTreeTextCtrl      *m_textCtrl;


    wxTimer             *m_renameTimer;

    // incremental search data
    std::string             m_findPrefix;
    wxTimer             *m_findTimer;
    // This flag is set to 0 if the bell is disabled, 1 if it is enabled and -1
    // if it is globally enabled but has been temporarily disabled because we
    // had already beeped for this particular search.
    int                  m_findBell;

    bool                 m_dropEffectAboveItem;

    // the common part of all ctors
    void Init();

    // overridden wxWindow methods
    void DoThaw() override;

    // misc helpers
    void SendDeleteEvent(wxGenericTreeItem *itemBeingDeleted);

    void DrawBorder(const wxTreeItemId& item);
    void DrawLine(const wxTreeItemId& item, bool below);
    void DrawDropEffect(wxGenericTreeItem *item);

    void DoSelectItem(const wxTreeItemId& id,
                      bool unselect_others = true,
                      bool extended_select = false);

    int DoGetItemState(const wxTreeItemId& item) const override;
    void DoSetItemState(const wxTreeItemId& item, int state) override;

    wxTreeItemId DoInsertItem(const wxTreeItemId& parent,
                                      size_t previous,
                                      const std::string& text,
                                      int image,
                                      int selectedImage,
                                      wxTreeItemData *data) override;
    wxTreeItemId DoInsertAfter(const wxTreeItemId& parent,
                                       const wxTreeItemId& idPrevious,
                                       const std::string& text,
                                       int image = -1, int selImage = -1,
                                       wxTreeItemData *data = nullptr) override;
    wxTreeItemId DoTreeHitTest(const wxPoint& point, unsigned int& flags) const override;

    // called by wxTextTreeCtrl when it marks itself for deletion
    void ResetTextControl();

    // find the first item starting with the given prefix after the given item
    wxTreeItemId FindItem(const wxTreeItemId& id, const std::string& prefix) const;

    bool HasButtons() const { return HasFlag(wxTR_HAS_BUTTONS); }

    void CalculateLineHeight();
    int  GetLineHeight(wxGenericTreeItem *item) const;
    void PaintLevel( wxGenericTreeItem *item, wxDC& dc, int level, int &y );
    void PaintItem( wxGenericTreeItem *item, wxDC& dc);

    void CalculateLevel( wxGenericTreeItem *item, wxDC &dc, int level, int &y );
    void CalculatePositions();

    void RefreshSubtree( wxGenericTreeItem *item );
    void RefreshLine( wxGenericTreeItem *item );

    // redraw all selected items
    void RefreshSelected();

    // RefreshSelected() recursive helper
    void RefreshSelectedUnder(wxGenericTreeItem *item);

    void OnRenameTimer();
    bool OnRenameAccept(wxGenericTreeItem *item, const std::string& value);
    void OnRenameCancelled(wxGenericTreeItem *item);

    void FillArray(wxGenericTreeItem*, wxArrayTreeItemIds&) const;
    void SelectItemRange( wxGenericTreeItem *item1, wxGenericTreeItem *item2 );
    bool TagAllChildrenUntilLast(wxGenericTreeItem *crt_item, wxGenericTreeItem *last_item, bool select);
    bool TagNextChildren(wxGenericTreeItem *crt_item, wxGenericTreeItem *last_item, bool select);
    void UnselectAllChildren( wxGenericTreeItem *item );
    void ChildrenClosing(wxGenericTreeItem* item);

    void DoDirtyProcessing();

    wxSize DoGetBestSize() const override;

private:
    void OnSysColourChanged([[maybe_unused]] wxSysColourChangedEvent& event)
    {
        InitVisualAttributes();
    }

    // (Re)initialize colours, fonts, pens, brushes used by the control using
    // the current system colours and font.
    void InitVisualAttributes();

    // Reset the state of the last find (i.e. keyboard incremental search)
    // operation.
    void ResetFindState();


    // True if we're using custom colours/font, respectively, or false if we're
    // using the default colours and should update them whenever system colours
    // change.
    bool m_hasExplicitFgCol;
    bool m_hasExplicitBgCol;
    bool m_hasExplicitFont;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_DYNAMIC_CLASS(wxGenericTreeCtrl);
};

// Also define wxTreeCtrl to be wxGenericTreeCtrl on all platforms without a
// native version, i.e. all but MSW and Qt.
#if !(defined(__WXMSW__) || defined(__WXQT__)) || defined(__WXUNIVERSAL__)
/*
 * wxTreeCtrl has to be a real class or we have problems with
 * the run-time information.
 */

class wxTreeCtrl: public wxGenericTreeCtrl
{
    wxDECLARE_DYNAMIC_CLASS(wxTreeCtrl);

public:
    wxTreeCtrl() = default;

    wxTreeCtrl(wxWindow *parent, wxWindowID id = wxID_ANY,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               unsigned int style = wxTR_DEFAULT_STYLE,
               const wxValidator &validator = {},
               const std::string& name = wxTreeCtrlNameStr)
    : wxGenericTreeCtrl(parent, id, pos, size, style, validator, name)
    {
    }
};
#endif // !(__WXMSW__ || __WXQT__) || __WXUNIVERSAL__

#endif // wxUSE_TREECTRL

#endif // _GENERIC_TREECTRL_H_
