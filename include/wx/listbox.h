///////////////////////////////////////////////////////////////////////////////
// Name:        wx/listbox.h
// Purpose:     wxListBox class interface
// Author:      Vadim Zeitlin
// Modified by:
// Created:     22.10.99
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_LISTBOX_H_BASE_
#define _WX_LISTBOX_H_BASE_

#if wxUSE_LISTBOX

#include "wx/ctrlsub.h"         // base class
#include "wx/defs.h"

import <string>;
import <vector>;

// ----------------------------------------------------------------------------
// global data
// ----------------------------------------------------------------------------

inline constexpr char wxListBoxNameStr[] = "listBox";

/*
 * Styles for wxListBox
 */
inline constexpr unsigned int wxLB_SORT           = 0x0010;
inline constexpr unsigned int wxLB_SINGLE         = 0x0020;
inline constexpr unsigned int wxLB_MULTIPLE       = 0x0040;
inline constexpr unsigned int wxLB_EXTENDED       = 0x0080;

// ----------------------------------------------------------------------------
// wxListBox interface is defined by the class wxListBoxBase
// ----------------------------------------------------------------------------

class wxListBoxBase : public wxControlWithItems
{
public:
    wxListBoxBase& operator=(wxListBoxBase&&) = delete;

    void InsertItems(const std::vector<std::string>& items, unsigned int pos)
        { Insert(items, pos); }

    // multiple selection logic
    virtual bool IsSelected(int n) const = 0;
    void SetSelection(int n) override;
    void SetSelection(int n, bool select) { DoSetSelection(n, select); }
    void Deselect(int n) { DoSetSelection(n, false); }
    void DeselectAll(int itemToLeaveSelected = -1);

    virtual bool SetStringSelection(std::string_view s, bool select);
    virtual bool SetStringSelection(std::string_view s)
    {
        return SetStringSelection(s, true);
    }

    // works for single as well as multiple selection listboxes (unlike
    // GetSelection which only works for listboxes with single selection)
    virtual int GetSelections(std::vector<int>& aSelections) const = 0;

    // set the specified item at the first visible item or scroll to max
    // range.
    void SetFirstItem(int n) { DoSetFirstItem(n); }
    void SetFirstItem(std::string_view s);

    // ensures that the given item is visible scrolling the listbox if
    // necessary
    virtual void EnsureVisible(int n);

    virtual int GetTopItem() const { return wxNOT_FOUND; }
    virtual int GetCountPerPage() const { return -1; }

    // a combination of Append() and EnsureVisible(): appends the item to the
    // listbox and ensures that it is visible i.e. not scrolled out of view
    void AppendAndEnsureVisible(const std::string& s);

    // return true if the listbox allows multiple selection
    bool HasMultipleSelection() const
    {
        return (m_windowStyle & wxLB_MULTIPLE) ||
               (m_windowStyle & wxLB_EXTENDED);
    }

    // override wxItemContainer::IsSorted
    bool IsSorted() const override { return HasFlag( wxLB_SORT ); }

    // emulate selecting or deselecting the item event.GetInt() (depending on
    // event.GetExtraLong())
    void Command(wxCommandEvent& event) override;

    // return the index of the item at this position or wxNOT_FOUND
    int HitTest(const wxPoint& point) const { return DoListHitTest(point); }


protected:
    virtual void DoSetFirstItem(int n) = 0;

    virtual void DoSetSelection(int n, bool select) = 0;

    // there is already wxWindow::DoHitTest() so call this one differently
    virtual int DoListHitTest(const wxPoint& WXUNUSED(point)) const
        { return wxNOT_FOUND; }

    // Helper for the code generating events in single selection mode: updates
    // m_oldSelections and return true if the selection really changed.
    // Otherwise just returns false.
    bool DoChangeSingleSelection(int item);

    // Helper for generating events in multiple and extended mode: compare the
    // current selections with the previously recorded ones (in
    // m_oldSelections) and send the appropriate event if they differ,
    // otherwise just return false.
    bool CalcAndSendEvent();

    // Send a listbox (de)selection or double click event.
    //
    // Returns true if the event was processed.
    bool SendEvent(wxEventType evtType, int item, bool selected);

    // Array storing the indices of all selected items that we already notified
    // the user code about for multi selection list boxes.
    //
    // For single selection list boxes, we reuse this array to store the single
    // currently selected item, this is used by DoChangeSingleSelection().
    //
    // TODO-OPT: wxSelectionStore would be more efficient for big list boxes.
    std::vector<int> m_oldSelections;

    // Update m_oldSelections with currently selected items (does nothing in
    // single selection mode on platforms other than MSW).
    void UpdateOldSelections();
};

// ----------------------------------------------------------------------------
// include the platform-specific class declaration
// ----------------------------------------------------------------------------

#if defined(__WXUNIVERSAL__)
    #include "wx/univ/listbox.h"
#elif defined(__WXMSW__)
    #include "wx/msw/listbox.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/listbox.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/listbox.h"
#elif defined(__WXGTK__)
  #include "wx/gtk1/listbox.h"
#elif defined(__WXMAC__)
    #include "wx/osx/listbox.h"
#elif defined(__WXQT__)
    #include "wx/qt/listbox.h"
#endif

#endif // wxUSE_LISTBOX

#endif
    // _WX_LISTBOX_H_BASE_
