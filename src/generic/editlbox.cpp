/////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/editlbox.cpp
// Purpose:     ListBox with editable items
// Author:      Vaclav Slavik
// Copyright:   (c) Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_EDITABLELISTBOX

#include "wx/editlbox.h"
#include "wx/listctrl.h"
#include "wx/artprov.h"
#include "wx/bmpbuttn.h"
#include "wx/stattext.h"

import WX.Core.Sizer;

import WX.Utils.Settings;

// ============================================================================
// implementation
// ============================================================================

// list control with auto-resizable column:
class CleverListCtrl : public wxListCtrl
{
public:
   explicit CleverListCtrl(wxWindow *parent,
                  wxWindowID id = wxID_ANY,
                  const wxPoint &pos = wxDefaultPosition,
                  const wxSize &size = wxDefaultSize,
                  unsigned int style = wxLC_ICON,
                  const wxValidator& validator = {},
                  std::string_view name = wxListCtrlNameStr)
         : wxListCtrl(parent, id, pos, size, style, validator, name)
    {
        CreateColumns();
    }

    void CreateColumns()
    {
        InsertColumn(0, "item");
        SizeColumns();
    }

    void SizeColumns()
    {
         int w = GetSize().x;
#ifdef __WXMSW__
         w -= wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, this) + 6;
#else
         w -= 2*wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, this);
#endif
         if (w < 0) w = 0;
         SetColumnWidth(0, w);
    }

private:
    wxDECLARE_EVENT_TABLE();
    void OnSize(wxSizeEvent& event)
    {
        SizeColumns();
        event.Skip();
    }
};

wxBEGIN_EVENT_TABLE(CleverListCtrl, wxListCtrl)
   EVT_SIZE(CleverListCtrl::OnSize)
wxEND_EVENT_TABLE()


// ----------------------------------------------------------------------------
// wxEditableListBox
// ----------------------------------------------------------------------------

// NB: generate the IDs at runtime to avoid conflict with XRCID values,
//     they could cause XRCCTRL() failures in XRC-based dialogs
const wxWindowIDRef wxID_ELB_DELETE = wxWindow::NewControlId();
const wxWindowIDRef wxID_ELB_EDIT = wxWindow::NewControlId();
const wxWindowIDRef wxID_ELB_NEW = wxWindow::NewControlId();
const wxWindowIDRef wxID_ELB_UP = wxWindow::NewControlId();
const wxWindowIDRef wxID_ELB_DOWN = wxWindow::NewControlId();
const wxWindowIDRef wxID_ELB_LISTCTRL = wxWindow::NewControlId();

wxBEGIN_EVENT_TABLE(wxEditableListBox, wxPanel)
    EVT_LIST_ITEM_SELECTED(wxID_ELB_LISTCTRL, wxEditableListBox::OnItemSelected)
    EVT_LIST_END_LABEL_EDIT(wxID_ELB_LISTCTRL, wxEditableListBox::OnEndLabelEdit)
    EVT_BUTTON(wxID_ELB_NEW, wxEditableListBox::OnNewItem)
    EVT_BUTTON(wxID_ELB_UP, wxEditableListBox::OnUpItem)
    EVT_BUTTON(wxID_ELB_DOWN, wxEditableListBox::OnDownItem)
    EVT_BUTTON(wxID_ELB_EDIT, wxEditableListBox::OnEditItem)
    EVT_BUTTON(wxID_ELB_DELETE, wxEditableListBox::OnDelItem)
wxEND_EVENT_TABLE()

bool wxEditableListBox::Create(wxWindow *parent, wxWindowID id,
                          const std::string& label,
                          const wxPoint& pos, const wxSize& size,
                          unsigned int style,
                          std::string_view name)
{
    if (!wxPanel::Create(parent, id, pos, size, wxTAB_TRAVERSAL, name))
        return false;

    m_style = style;

    wxSizer *sizer = new wxBoxSizer(wxVERTICAL);

    wxPanel *subp = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                wxSUNKEN_BORDER | wxTAB_TRAVERSAL);
    wxSizer *subsizer = new wxBoxSizer(wxHORIZONTAL);

    subsizer->Add(new wxStaticText(subp, wxID_ANY, label),
                  wxSizerFlags(1).Center().Border(wxDirection::wxLEFT));

    const wxSizerFlags flagsCentered = wxSizerFlags().Center();

    if ( m_style & wxEL_ALLOW_EDIT )
    {
        m_bEdit = new wxBitmapButton(subp, wxID_ELB_EDIT,
                                     wxArtProvider::GetBitmap(wxART_EDIT, wxART_BUTTON));
        m_bEdit->SetToolTip(_("Edit item"));
        subsizer->Add(m_bEdit, flagsCentered);
    }

    if ( m_style & wxEL_ALLOW_NEW )
    {
        m_bNew = new wxBitmapButton(subp, wxID_ELB_NEW,
                                    wxArtProvider::GetBitmap(wxART_NEW, wxART_BUTTON));
        m_bNew->SetToolTip(_("New item"));
        subsizer->Add(m_bNew, flagsCentered);
    }

    if ( m_style & wxEL_ALLOW_DELETE )
    {
        m_bDel = new wxBitmapButton(subp, wxID_ELB_DELETE,
                                    wxArtProvider::GetBitmap(wxART_DELETE, wxART_BUTTON));
        m_bDel->SetToolTip(_("Delete item"));
        subsizer->Add(m_bDel, flagsCentered);
    }

    if (!(m_style & wxEL_NO_REORDER))
    {
        m_bUp = new wxBitmapButton(subp, wxID_ELB_UP,
                                   wxArtProvider::GetBitmap(wxART_GO_UP, wxART_BUTTON));
        m_bUp->SetToolTip(_("Move up"));
        subsizer->Add(m_bUp, flagsCentered);

        m_bDown = new wxBitmapButton(subp, wxID_ELB_DOWN,
                                     wxArtProvider::GetBitmap(wxART_GO_DOWN, wxART_BUTTON));
        m_bDown->SetToolTip(_("Move down"));
        subsizer->Add(m_bDown, flagsCentered);
    }

    subp->SetSizer(subsizer);
    subsizer->Fit(subp);

    sizer->Add(subp, wxSizerFlags().Expand());

    unsigned int st = wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL | wxSUNKEN_BORDER;
    if ( style & wxEL_ALLOW_EDIT )
         st |= wxLC_EDIT_LABELS;
    m_listCtrl = new CleverListCtrl(this, wxID_ELB_LISTCTRL,
                                    wxDefaultPosition, wxDefaultSize, st);
    std::vector<std::string> empty_ar;
    SetStrings(empty_ar);

    sizer->Add(m_listCtrl, wxSizerFlags(1).Expand());

    SetSizer(sizer);
    Layout();

    return true;
}

void wxEditableListBox::SetStrings(const std::vector<std::string>& strings)
{
    m_listCtrl->DeleteAllItems();

    for (size_t i = 0; i < strings.size(); i++)
        m_listCtrl->InsertItem(i, strings[i]);

    m_listCtrl->InsertItem(strings.size(), "");
    m_listCtrl->SetItemState(0, ListStates::Selected, ListStates::Selected);
}

std::vector<std::string> wxEditableListBox::GetStrings() const
{
    std::vector<std::string> listBoxStrings;

    listBoxStrings.reserve(m_listCtrl->GetItemCount() - 1);

    for (std::size_t i{}; i != m_listCtrl->GetItemCount() - 1; i++)
        listBoxStrings.push_back(m_listCtrl->GetItemText(i));

    return listBoxStrings;
}

void wxEditableListBox::OnItemSelected(wxListEvent& event)
{
    m_selection = event.GetIndex();
    if (!(m_style & wxEL_NO_REORDER))
    {
        m_bUp->Enable(m_selection != 0 && m_selection < m_listCtrl->GetItemCount()-1);
        m_bDown->Enable(m_selection < m_listCtrl->GetItemCount()-2);
    }

    if (m_style & wxEL_ALLOW_EDIT)
        m_bEdit->Enable(m_selection < m_listCtrl->GetItemCount()-1);
    if (m_style & wxEL_ALLOW_DELETE)
        m_bDel->Enable(m_selection < m_listCtrl->GetItemCount()-1);
}

void wxEditableListBox::OnNewItem([[maybe_unused]] wxCommandEvent& event)
{
    m_listCtrl->SetItemState(m_listCtrl->GetItemCount()-1,
                             ListStates::Selected, ListStates::Selected);
    m_listCtrl->EditLabel(m_selection);
}

void wxEditableListBox::OnEndLabelEdit(wxListEvent& event)
{
    if ( event.GetIndex() == m_listCtrl->GetItemCount()-1 &&
         !event.GetText().empty() )
    {
        // The user edited last (empty) line, i.e. added new entry. We have to
        // add new empty line here so that adding one more line is still
        // possible:
        m_listCtrl->InsertItem(m_listCtrl->GetItemCount(), "");

        // Simulate a wxEVT_LIST_ITEM_SELECTED event for the new item,
        // so that the buttons are enabled/disabled properly
        wxListEvent selectionEvent(wxEVT_LIST_ITEM_SELECTED, m_listCtrl->GetId());
        selectionEvent.m_itemIndex = event.GetIndex();
        m_listCtrl->GetEventHandler()->ProcessEvent(selectionEvent);
    }
}

void wxEditableListBox::OnDelItem([[maybe_unused]] wxCommandEvent& event)
{
    m_listCtrl->DeleteItem(m_selection);
    m_listCtrl->SetItemState(m_selection,
                             ListStates::Selected, ListStates::Selected);
}

void wxEditableListBox::OnEditItem([[maybe_unused]] wxCommandEvent& event)
{
    m_listCtrl->EditLabel(m_selection);
}

void wxEditableListBox::SwapItems(long i1, long i2)
{
    // swap the text
    std::string t1 = m_listCtrl->GetItemText(i1);
    std::string t2 = m_listCtrl->GetItemText(i2);
    m_listCtrl->SetItemText(i1, t2);
    m_listCtrl->SetItemText(i2, t1);

    // swap the item data
    wxUIntPtr d1 = m_listCtrl->GetItemData(i1);
    wxUIntPtr d2 = m_listCtrl->GetItemData(i2);
    m_listCtrl->SetItemPtrData(i1, d2);
    m_listCtrl->SetItemPtrData(i2, d1);
}


void wxEditableListBox::OnUpItem([[maybe_unused]] wxCommandEvent& event)
{
    SwapItems(m_selection - 1, m_selection);
    m_listCtrl->SetItemState(m_selection - 1,
                             ListStates::Selected, ListStates::Selected);
}

void wxEditableListBox::OnDownItem([[maybe_unused]] wxCommandEvent& event)
{
    SwapItems(m_selection + 1, m_selection);
    m_listCtrl->SetItemState(m_selection + 1,
                             ListStates::Selected, ListStates::Selected);
}

#endif // wxUSE_EDITABLELISTBOX
