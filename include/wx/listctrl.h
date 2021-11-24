///////////////////////////////////////////////////////////////////////////////
// Name:        wx/listctrl.h
// Purpose:     wxListCtrl class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     04.12.99
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_LISTCTRL_H_BASE_
#define _WX_LISTCTRL_H_BASE_

#if wxUSE_LISTCTRL

#include "wx/listbase.h"

import <string_view>;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

inline constexpr std::string_view wxListCtrlNameStr = "listCtrl";

// ----------------------------------------------------------------------------
// include the wxListCtrl class declaration
// ----------------------------------------------------------------------------

#if defined(__WXMSW__) && !defined(__WXUNIVERSAL__)
    #include "wx/msw/listctrl.h"
#elif defined(__WXQT__) && !defined(__WXUNIVERSAL__)
    #include "wx/qt/listctrl.h"
#else
    #include "wx/generic/listctrl.h"
#endif

import <string>;

// ----------------------------------------------------------------------------
// wxListView: a class which provides a better API for list control
// ----------------------------------------------------------------------------

class wxListView : public wxListCtrl
{
public:
    wxListView() = default;
    wxListView( wxWindow *parent,
                wxWindowID winid = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxLC_REPORT,
                const wxValidator& validator = wxDefaultValidator,
                std::string_view name = wxListCtrlNameStr)
    {
        Create(parent, winid, pos, size, style, validator, name);
    }

    wxListView& operator=(wxListView&&) = delete;

    // focus/selection stuff
    // ---------------------

    // [de]select an item
    void Select(long n, bool on = true)
    {
        SetItemState(n, on ? ListStateFlags{ListStates::Selected} : ListStateFlags{}, ListStateFlags{ListStates::Selected});
    }

    // focus and show the given item
    void Focus(long index)
    {
        SetItemState(index, ListStates::Focused, ListStates::Focused);
        EnsureVisible(index);
    }

    // get the currently focused item or -1 if none
    long GetFocusedItem() const
    {
        return GetNextItem(-1, wxListGetNextItem::All, ListStates::Focused);
    }

    // get first and subsequent selected items, return -1 when no more
    long GetNextSelected(long item) const
        { return GetNextItem(item, wxListGetNextItem::All, ListStates::Selected); }
    long GetFirstSelected() const
        { return GetNextSelected(-1); }

    // return true if the item is selected
    bool IsSelected(long index) const
        { return !GetItemState(index, ListStates::Selected).empty(); }

    // columns
    // -------

    void SetColumnImage(int col, int image)
    {
        wxListItem item;
        item.SetMask(ListMaskFlags{ListMasks::Image});
        item.SetImage(image);
        SetColumn(col, item);
    }

    void ClearColumnImage(int col) { SetColumnImage(col, -1); }

public:
	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // wxUSE_LISTCTRL

#endif
    // _WX_LISTCTRL_H_BASE_
