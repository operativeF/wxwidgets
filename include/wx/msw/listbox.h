/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/listbox.h
// Purpose:     wxListBox class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_LISTBOX_H_
#define _WX_LISTBOX_H_

#if wxUSE_LISTBOX

import WX.WinDef;

import Utils.Geometry;

#include <memory>
import <string>;
import <string_view>;
import <utility>;
import <vector>;

import WX.Utils.Cast;

#if wxUSE_OWNER_DRAWN
  #include <wx/ownerdrw.h>

  using wxListBoxItemsArray = std::vector<std::unique_ptr<wxOwnerDrawn>>;
#endif // wxUSE_OWNER_DRAWN

class wxListBox : public wxListBoxBase
{
public:
    wxListBox() = default;

    wxListBox(wxWindow *parent, wxWindowID id,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            const std::vector<std::string>& choices = {},
            unsigned int style = 0,
            const wxValidator& validator = {},
            std::string_view name = wxListBoxNameStr)
    {
        Create(parent, id, pos, size, choices, style, validator, name);
    }
    
    [[maybe_unused]] bool Create(wxWindow *parent, wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                const std::vector<std::string>& choices = {},
                unsigned int style = 0,
                const wxValidator& validator = {},
                std::string_view name = wxListBoxNameStr);

    ~wxListBox();

    wxListBox& operator=(wxListBox&&) = delete;

    size_t GetCount() const override;
    std::string GetString(unsigned int n) const override;
    void SetString(unsigned int n, const std::string& s) override;
    int FindString(std::string_view s, bool bCase = false) const override;

    bool IsSelected(int n) const override;
    int GetSelection() const override;
    int GetSelections(std::vector<int>& aSelections) const override;

    // return the index of the item at this position or wxNOT_FOUND
    int HitTest(const wxPoint& pt) const { return DoHitTestList(pt); }
    int HitTest(wxCoord x, wxCoord y) const { return DoHitTestList(wxPoint(x, y)); }

    void EnsureVisible(int n) override;

    int GetTopItem() const override;
    int GetCountPerPage() const override;

    // ownerdrawn wxListBox and wxCheckListBox support
#if wxUSE_OWNER_DRAWN
    // override base class virtuals
    bool SetFont(const wxFont &font) override;

    bool MSWOnMeasure(WXMEASUREITEMSTRUCT *item) override;
    bool MSWOnDraw(WXDRAWITEMSTRUCT *item) override;

    // plug-in for derived classes
    virtual std::unique_ptr<wxOwnerDrawn> CreateLboxItem(size_t n);

    // allows to get the item and use SetXXX functions to set it's appearance
    wxOwnerDrawn* GetItem(size_t n) const { return m_aItems[n].get(); }

    // get the index of the given item
    // FIXME: Use iterators.
    std::ptrdiff_t GetItemIndex(wxOwnerDrawn *item) const
    {
        const auto index = std::distance(m_aItems.begin(),
                                         std::ranges::find_if(m_aItems,
                                            [item](auto&& aItem){ return aItem.get() == item; }));

        if(std::cmp_equal(index, m_aItems.size()))
        {
            return -1;
        }

        return index;
    }

    // get rect of the given item index
    bool GetItemRect(size_t n, wxRect& rect) const;

    // redraw the given item
    bool RefreshItem(size_t n);
#endif // wxUSE_OWNER_DRAWN

    // Windows-specific code to update the horizontal extent of the listbox, if
    // necessary. If s is non-empty, the horizontal extent is increased to the
    // length of this string if it's currently too short, otherwise the maximum
    // extent of all strings is used. In any case calls InvalidateBestSize()
    virtual void SetHorizontalExtent(const std::string& s = {});

    // This is a wrapper for LB_SETTABSTOPS message and takes tab stops in
    // dialog units, with the same conventions as LB_SETTABSTOPS uses.
    virtual bool MSWSetTabStops(const std::vector<int>& tabStops);

    // Windows callbacks
    bool MSWCommand(WXUINT param, WXWORD id) override;
    WXDWORD MSWGetStyle(unsigned int style, WXDWORD *exstyle) const override;

    // under XP when using "transition effect for menus and tooltips" if we
    // return true for WM_PRINTCLIENT here then it causes noticeable slowdown
    bool MSWShouldPropagatePrintChild() override
    {
        return false;
    }

    wxVisualAttributes GetDefaultAttributes() const override
    {
        return GetClassDefaultAttributes(GetWindowVariant());
    }

    static wxVisualAttributes
    GetClassDefaultAttributes(wxWindowVariant variant = wxWindowVariant::Normal)
    {
        return GetCompositeControlsDefaultAttributes(variant);
    }

    // returns true if the platform should explicitly apply a theme border
    bool CanApplyThemeBorder() const override { return false; }

    void OnInternalIdle() override;

protected:
    wxSize DoGetBestClientSize() const override;

    void DoClear() override;
    void DoDeleteOneItem(unsigned int n) override;

    void DoSetSelection(int n, bool select) override;

    int DoInsertItems(const std::vector<std::string>& items,
                              unsigned int pos,
                              void **clientData, wxClientDataType type) override;

    void DoSetFirstItem(int n) override;
    void DoSetItemClientData(unsigned int n, void* clientData) override;
    void* DoGetItemClientData(unsigned int n) const override;

    // this can't be called DoHitTest() because wxWindow already has this method
    virtual int DoHitTestList(const wxPoint& point) const;

    // This is a hook for wxCheckListBox, which uses it to add the checkbox
    // width to the item width and to make it at least as tall as the checkbox.
    virtual wxSize MSWGetFullItemSize(int w, int h) const
    {
        return wxSize(w, h);
    }

    void MSWUpdateFontOnDPIChange(const wxSize& newDPI) override;

    // free memory (common part of Clear() and dtor)
    void Free();

    size_t m_noItems{0};

#if wxUSE_OWNER_DRAWN
    // control items
    wxListBoxItemsArray m_aItems;
#endif

private:
    // call this when items are added to or deleted from the listbox or an
    // items text changes
    void MSWOnItemsChanged();

    // flag indicating whether the max horizontal extent should be updated,
    // i.e. if we need to call SetHorizontalExtent() from OnInternalIdle()
    bool m_updateHorizontalExtent{false};


public:
	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // wxUSE_LISTBOX

#endif // _WX_LISTBOX_H_
