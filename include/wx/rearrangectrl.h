///////////////////////////////////////////////////////////////////////////////
// Name:        wx/rearrangectrl.h
// Purpose:     various controls for rearranging the items interactively
// Author:      Vadim Zeitlin
// Created:     2008-12-15
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_REARRANGECTRL_H_
#define _WX_REARRANGECTRL_H_

#if wxUSE_REARRANGECTRL

#include "wx/checklst.h"
#include "wx/panel.h"
#include "wx/dialog.h"

import <string>;
import <vector>;

inline constexpr char wxRearrangeListNameStr[] = "wxRearrangeList";
inline constexpr char wxRearrangeDialogNameStr[] = "wxRearrangeDlg";

// ----------------------------------------------------------------------------
// wxRearrangeList: a (check) list box allowing to move items around
// ----------------------------------------------------------------------------

// This class works allows to change the order of the items shown in it as well
// as to check or uncheck them individually. The data structure used to allow
// this is the order array which contains the items indices indexed by their
// position with an added twist that the unchecked items are represented by the
// bitwise complement of the corresponding index (for any architecture using
// two's complement for negative numbers representation (i.e. just about any at
// all) this means that a checked item N is represented by -N-1 in unchecked
// state).
//
// So, for example, the array order [1 -3 0] used in conjunction with the items
// array ["first", "second", "third"] means that the items are displayed in the
// order "second", "third", "first" and the "third" item is unchecked while the
// other two are checked.
class wxRearrangeList : public wxCheckListBox
{
public:
    // default ctor, call Create() later
    wxRearrangeList() = default;

    // ctor creating the control, the arguments are the same as for
    // wxCheckListBox except for the extra order array which defines the
    // (initial) display order of the items as well as their statuses, see the
    // description above
    wxRearrangeList(wxWindow *parent,
                    wxWindowID id,
                    const wxPoint& pos,
                    const wxSize& size,
                    const std::vector<int>& order,
                    const std::vector<std::string>& items,
                    unsigned int style = 0,
                    const wxValidator& validator = wxDefaultValidator,
                    const std::string& name = wxRearrangeListNameStr)
    {
        Create(parent, id, pos, size, order, items, style, validator, name);
    }

    wxRearrangeList& operator=(wxRearrangeList&&) = delete;

    // Create() function takes the same parameters as the base class one and
    // the order array determining the initial display order
    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos,
                const wxSize& size,
                const std::vector<int>& order,
                const std::vector<std::string>& items,
                unsigned int style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const std::string& name = wxRearrangeListNameStr);


    // items order
    // -----------

    // get the current items order; the returned array uses the same convention
    // as the one passed to the ctor
    const std::vector<int>& GetCurrentOrder() const { return m_order; }

    // return true if the current item can be moved up or down (i.e. just that
    // it's not the first or the last one)
    bool CanMoveCurrentUp() const;
    bool CanMoveCurrentDown() const;

    // move the current item one position up or down, return true if it was moved
    // or false if the current item was the first/last one and so nothing was done
    bool MoveCurrentUp();
    bool MoveCurrentDown();


    // Override this to keep our m_order array in sync with the real item state.
    void Check(unsigned int item, bool check = true) override;

    int DoInsertItems(const std::vector<std::string>& items, unsigned int pos,
                      void **clientData, wxClientDataType type) override;
    void DoDeleteOneItem(unsigned int n) override;
    void DoClear() override;

private:
    // swap two items at the given positions in the listbox
    void Swap(int pos1, int pos2);

    // event handler for item checking/unchecking
    void OnCheck(wxCommandEvent& event);

    // the current order array
    std::vector<int> m_order;

    wxDECLARE_EVENT_TABLE();
};

// ----------------------------------------------------------------------------
// wxRearrangeCtrl: composite control containing a wxRearrangeList and buttons
// ----------------------------------------------------------------------------

class wxRearrangeCtrl : public wxPanel
{
public:
    // ctors/Create function are the same as for wxRearrangeList
    wxRearrangeCtrl() = default;

    wxRearrangeCtrl(wxWindow *parent,
                    wxWindowID id,
                    const wxPoint& pos,
                    const wxSize& size,
                    const std::vector<int>& order,
                    const std::vector<std::string>& items,
                    unsigned int style = 0,
                    const wxValidator& validator = wxDefaultValidator,
                    const std::string& name = wxRearrangeListNameStr)
    {
        Create(parent, id, pos, size, order, items, style, validator, name);
    }

    wxRearrangeCtrl& operator=(wxRearrangeCtrl&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos,
                const wxSize& size,
                const std::vector<int>& order,
                const std::vector<std::string>& items,
                unsigned int style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const std::string& name = wxRearrangeListNameStr);

    // get the underlying listbox
    wxRearrangeList *GetList() const { return m_list; }

private:
    // event handlers for the buttons
    void OnUpdateButtonUI(wxUpdateUIEvent& event);
    void OnButton(wxCommandEvent& event);

    wxRearrangeList *m_list{nullptr};

    wxDECLARE_EVENT_TABLE();
};

// ----------------------------------------------------------------------------
// wxRearrangeDialog: dialog containing a wxRearrangeCtrl
// ----------------------------------------------------------------------------

class wxRearrangeDialog : public wxDialog
{
public:
    // default ctor, use Create() later
    wxRearrangeDialog() = default;

    // ctor for the dialog: message is shown inside the dialog itself, order
    // and items are passed to wxRearrangeList used internally
    wxRearrangeDialog(wxWindow *parent,
                      const std::string& message,
                      const std::string& title,
                      const std::vector<int>& order,
                      const std::vector<std::string>& items,
                      const wxPoint& pos = wxDefaultPosition,
                      const std::string& name = wxRearrangeDialogNameStr)
    {
        Create(parent, message, title, order, items, pos, name);
    }

    wxRearrangeDialog& operator=(wxRearrangeDialog&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                const std::string& message,
                const std::string& title,
                const std::vector<int>& order,
                const std::vector<std::string>& items,
                const wxPoint& pos = wxDefaultPosition,
                const std::string& name = wxRearrangeDialogNameStr);


    // methods for the dialog customization

    // add extra contents to the dialog below the wxRearrangeCtrl part: the
    // given window (usually a wxPanel containing more control inside it) must
    // have the dialog as its parent and will be inserted into it at the right
    // place by this method
    void AddExtraControls(wxWindow *win);

    // return the wxRearrangeList control used by the dialog
    wxRearrangeList *GetList() const;


    // get the order of items after it was modified by the user
    std::vector<int> GetOrder() const;

private:
    wxRearrangeCtrl *m_ctrl{nullptr};
};

#endif // wxUSE_REARRANGECTRL

#endif // _WX_REARRANGECTRL_H_

