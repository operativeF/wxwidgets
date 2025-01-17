///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/rearrangectrl.cpp
// Purpose:     implementation of classes in wx/rearrangectrl.h
// Author:      Vadim Zeitlin
// Created:     2008-12-15
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_REARRANGECTRL

#include "wx/button.h"
#include "wx/stattext.h"
#include "wx/rearrangectrl.h"

import WX.Core.Sizer;

import WX.Utils.Cast;

// ============================================================================
// wxRearrangeList implementation
// ============================================================================

wxBEGIN_EVENT_TABLE(wxRearrangeList, wxCheckListBox)
    EVT_CHECKLISTBOX(wxID_ANY, wxRearrangeList::OnCheck)
wxEND_EVENT_TABLE()

bool wxRearrangeList::Create(wxWindow *parent,
                             wxWindowID id,
                             const wxPoint& pos,
                             const wxSize& size,
                             const std::vector<int>& order,
                             const std::vector<std::string>& items,
                             unsigned int style,
                             const wxValidator& validator,
                             std::string_view name)
{
    // construct the array of items in the order in which they should appear in
    // the control
    const size_t count = items.size();
    wxCHECK_MSG( order.size() == count, false, "arrays not in sync" );

    std::vector<std::string> itemsInOrder;
    itemsInOrder.reserve(count);
    // FIXME: Indexing in vector with a different array's size.
    for ( size_t n = 0; n < count; n++ )
    {
        int idx = order[n];
        if ( idx < 0 )
            idx = -idx - 1;
        itemsInOrder.push_back(items[idx]);
    }

    // do create the real control
    if ( !wxCheckListBox::Create(parent, id, pos, size, itemsInOrder,
                                 style, validator, name) )
        return false;

    // and now check all the items which should be initially checked
    // FIXME: Indexing in vector with a different array's size.
    for ( size_t n = 0; n < count; n++ )
    {
        if ( order[n] >= 0 )
        {
            // Be careful to call the base class version here and not our own
            // which would also update m_order itself.
            wxCheckListBox::Check(n);
        }
    }

    m_order = order;

    return true;
}

bool wxRearrangeList::CanMoveCurrentUp() const
{
    const int sel = GetSelection();
    return sel != wxNOT_FOUND && sel != 0;
}

bool wxRearrangeList::CanMoveCurrentDown() const
{
    const int sel = GetSelection();
    return sel != wxNOT_FOUND && wx::narrow_cast<unsigned>(sel) != GetCount() - 1;
}

bool wxRearrangeList::MoveCurrentUp()
{
    const int sel = GetSelection();
    if ( sel == wxNOT_FOUND || sel == 0 )
        return false;

    Swap(sel, sel - 1);
    SetSelection(sel - 1);

    return true;
}

bool wxRearrangeList::MoveCurrentDown()
{
    const int sel = GetSelection();
    if ( sel == wxNOT_FOUND || wx::narrow_cast<unsigned>(sel) == GetCount() - 1 )
        return false;

    Swap(sel, sel + 1);
    SetSelection(sel + 1);

    return true;
}

void wxRearrangeList::Swap(int pos1, int pos2)
{
    // update the internally stored order
    std::swap(m_order[pos1], m_order[pos2]);


    // and now also swap all the attributes of the items

    // first the label
    const std::string stringTmp = GetString(pos1);
    SetString(pos1, GetString(pos2));
    SetString(pos2, stringTmp);

    // then the checked state
    const bool checkedTmp = IsChecked(pos1);
    wxCheckListBox::Check(pos1, IsChecked(pos2));
    wxCheckListBox::Check(pos2, checkedTmp);

    // and finally the client data, if necessary
    switch ( GetClientDataType() )
    {
        case wxClientDataType::None:
            // nothing to do
            break;

        case wxClientDataType::Object:
            {
                wxClientData * const dataTmp = DetachClientObject(pos1);
                SetClientObject(pos1, DetachClientObject(pos2));
                SetClientObject(pos2, dataTmp);
            }
            break;

        case wxClientDataType::Void:
            {
                void * const dataTmp = GetClientData(pos1);
                SetClientData(pos1, GetClientData(pos2));
                SetClientData(pos2, dataTmp);
            }
            break;
    }
}

void wxRearrangeList::Check(unsigned int item, bool check)
{
    if ( check == IsChecked(item) )
        return;

    wxCheckListBox::Check(item, check);

    m_order[item] = ~m_order[item];
}

void wxRearrangeList::OnCheck(wxCommandEvent& event)
{
    // update the internal state to match the new item state
    const int n = event.GetInt();

    if ( (m_order[n] >= 0) != IsChecked(n) )
        m_order[n] = ~m_order[n];
}

int wxRearrangeList::DoInsertItems(const std::vector<std::string>& items, unsigned int pos,
                                   void **clientData, wxClientDataType type)
{
    int ret = wxCheckListBox::DoInsertItems(items, pos, clientData, type);
    const size_t numItems = items.size();
    for ( size_t i = 0; i < numItems; i++ )
    {
        // Item is not checked initially.
        const int idx = ~m_order.size();
        m_order.insert(std::begin(m_order) + pos + i, idx);
    }
    return ret;
}

void wxRearrangeList::DoDeleteOneItem(unsigned int n)
{
    wxCheckListBox::DoDeleteOneItem(n);
    int idxDeleted = m_order[n];
    if ( idxDeleted < 0 )
        idxDeleted = ~idxDeleted;
    m_order.erase(std::begin(m_order) + n);
    // Remaining items have to be reindexed.
    for( size_t i = 0; i < m_order.size(); i++ )
    {
        int idx = m_order[i];
        if ( idx < 0 )
        {
            idx = ~idx;
            if ( idx > idxDeleted )
                m_order[i] = ~(idx-1);
        }
        else
        {
            if ( idx > idxDeleted )
                m_order[i] = idx-1;
        }
    }
}

void wxRearrangeList::DoClear()
{
    wxCheckListBox::DoClear();
    m_order.clear();
}

// ============================================================================
// wxRearrangeCtrl implementation
// ============================================================================

wxBEGIN_EVENT_TABLE(wxRearrangeCtrl, wxPanel)
    EVT_UPDATE_UI(wxID_UP, wxRearrangeCtrl::OnUpdateButtonUI)
    EVT_UPDATE_UI(wxID_DOWN, wxRearrangeCtrl::OnUpdateButtonUI)

    EVT_BUTTON(wxID_UP, wxRearrangeCtrl::OnButton)
    EVT_BUTTON(wxID_DOWN, wxRearrangeCtrl::OnButton)
wxEND_EVENT_TABLE()



bool
wxRearrangeCtrl::Create(wxWindow *parent,
                        wxWindowID id,
                        const wxPoint& pos,
                        const wxSize& size,
                        const std::vector<int>& order,
                        const std::vector<std::string>& items,
                        unsigned int style,
                        const wxValidator& validator,
                        std::string_view name)
{
    // create all the windows
    if ( !wxPanel::Create(parent, id, pos, size, wxTAB_TRAVERSAL, name) )
        return false;

    m_list = new wxRearrangeList(this, wxID_ANY,
                                 wxDefaultPosition, wxDefaultSize,
                                 order, items,
                                 style, validator);
    wxButton * const btnUp = new wxButton(this, wxID_UP);
    wxButton * const btnDown = new wxButton(this, wxID_DOWN);

    // arrange them in a sizer
    wxSizer * const sizerBtns = new wxBoxSizer(wxVERTICAL);
    sizerBtns->Add(btnUp, wxSizerFlags().Centre().Border(wxDirection::wxBOTTOM));
    sizerBtns->Add(btnDown, wxSizerFlags().Centre().Border(wxDirection::wxTOP));

    wxSizer * const sizerTop = new wxBoxSizer(wxHORIZONTAL);
    sizerTop->Add(m_list, wxSizerFlags(1).Expand().Border(wxDirection::wxRIGHT));
    sizerTop->Add(sizerBtns, wxSizerFlags(0).Centre().Border(wxDirection::wxLEFT));
    SetSizer(sizerTop);

    m_list->SetFocus();

    return true;
}

void wxRearrangeCtrl::OnUpdateButtonUI(wxUpdateUIEvent& event)
{
    event.Enable( event.GetId() == wxID_UP ? m_list->CanMoveCurrentUp()
                                           : m_list->CanMoveCurrentDown() );
}

void wxRearrangeCtrl::OnButton(wxCommandEvent& event)
{
    if ( event.GetId() == wxID_UP )
        m_list->MoveCurrentUp();
    else
        m_list->MoveCurrentDown();
}

// ============================================================================
// wxRearrangeDialog implementation
// ============================================================================

namespace
{

enum wxRearrangeDialogSizerPositions
{
    Pos_Label,
    Pos_Ctrl,
    Pos_Buttons,
    Pos_Max
};

} // anonymous namespace

bool wxRearrangeDialog::Create(wxWindow *parent,
                               const std::string& message,
                               const std::string& title,
                               const std::vector<int>& order,
                               const std::vector<std::string>& items,
                               const wxPoint& pos,
                               std::string_view name)
{
    if ( !wxDialog::Create(parent, wxID_ANY, title,
                           pos, wxDefaultSize,
                           wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
                           name) )
        return false;

    m_ctrl = new wxRearrangeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                 order, items);

    // notice that the items in this sizer should be inserted accordingly to
    // wxRearrangeDialogSizerPositions order
    wxSizer * const sizerTop = new wxBoxSizer(wxVERTICAL);

    if ( !message.empty() )
    {
        sizerTop->Add(new wxStaticText(this, wxID_ANY, message),
                      wxSizerFlags().Border());
    }
    else
    {
        // for convenience of other wxRearrangeDialog code that depends on
        // positions of sizer items, insert a dummy zero-sized item
        sizerTop->AddSpacer(0);
    }

    sizerTop->Add(m_ctrl,
                  wxSizerFlags(1).Expand().Border());
    sizerTop->Add(CreateSeparatedButtonSizer(wxOK | wxCANCEL),
                  wxSizerFlags().Expand().Border());
    SetSizerAndFit(sizerTop);

    return true;
}

void wxRearrangeDialog::AddExtraControls(wxWindow *win)
{
    wxSizer * const sizer = GetSizer();
    wxCHECK_RET( sizer, "the dialog must be created first" );

    wxASSERT_MSG( sizer->GetChildren().GetCount() == Pos_Max,
                  "calling AddExtraControls() twice?" );

    sizer->Insert(Pos_Buttons, win, wxSizerFlags().Expand().Border());

    win->MoveAfterInTabOrder(m_ctrl);

    // we need to update the initial/minimal window size
    sizer->SetSizeHints(this);
}

wxRearrangeList *wxRearrangeDialog::GetList() const
{
    wxCHECK_MSG( m_ctrl, nullptr, "the dialog must be created first" );

    return m_ctrl->GetList();
}

std::vector<int> wxRearrangeDialog::GetOrder() const
{
    wxCHECK_MSG( m_ctrl, std::vector<int>(), "the dialog must be created first" );

    return m_ctrl->GetList()->GetCurrentOrder();
}

#endif // wxUSE_REARRANGECTRL
