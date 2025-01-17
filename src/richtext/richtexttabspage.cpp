/////////////////////////////////////////////////////////////////////////////
// Name:        src/richtext/richtexttabspage.cpp
// Purpose:     Implements the rich text formatting dialog tabs page.
// Author:      Julian Smart
// Modified by:
// Created:     10/4/2006 8:03:20 AM
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_RICHTEXT

#include "wx/richtext/richtexttabspage.h"

#include "wx/button.h"

/*!
 * wxRichTextTabsPage type definition
 */

wxIMPLEMENT_DYNAMIC_CLASS(wxRichTextTabsPage, wxRichTextDialogPage);

/*!
 * wxRichTextTabsPage event table definition
 */

wxBEGIN_EVENT_TABLE(wxRichTextTabsPage, wxRichTextDialogPage)

////@begin wxRichTextTabsPage event table entries
    EVT_LISTBOX( ID_RICHTEXTTABSPAGE_TABLIST, wxRichTextTabsPage::OnTablistSelected )
    EVT_BUTTON( ID_RICHTEXTTABSPAGE_NEW_TAB, wxRichTextTabsPage::OnNewTabClick )
    EVT_UPDATE_UI( ID_RICHTEXTTABSPAGE_NEW_TAB, wxRichTextTabsPage::OnNewTabUpdate )
    EVT_BUTTON( ID_RICHTEXTTABSPAGE_DELETE_TAB, wxRichTextTabsPage::OnDeleteTabClick )
    EVT_UPDATE_UI( ID_RICHTEXTTABSPAGE_DELETE_TAB, wxRichTextTabsPage::OnDeleteTabUpdate )
    EVT_BUTTON( ID_RICHTEXTTABSPAGE_DELETE_ALL_TABS, wxRichTextTabsPage::OnDeleteAllTabsClick )
    EVT_UPDATE_UI( ID_RICHTEXTTABSPAGE_DELETE_ALL_TABS, wxRichTextTabsPage::OnDeleteAllTabsUpdate )
////@end wxRichTextTabsPage event table entries

wxEND_EVENT_TABLE()

IMPLEMENT_HELP_PROVISION(wxRichTextTabsPage)

/*!
 * wxRichTextTabsPage constructors
 */

wxRichTextTabsPage::wxRichTextTabsPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, unsigned int style )
{
    Create(parent, id, pos, size, style);
}

/*!
 * wxRichTextTabsPage creator
 */

bool wxRichTextTabsPage::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, unsigned int style )
{
////@begin wxRichTextTabsPage creation
    wxRichTextDialogPage::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end wxRichTextTabsPage creation
    return true;
}

/*!
 * Control creation for wxRichTextTabsPage
 */

void wxRichTextTabsPage::CreateControls()
{
////@begin wxRichTextTabsPage content construction
    wxRichTextTabsPage* itemRichTextDialogPage1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemRichTextDialogPage1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer4, 1, wxGROW, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer5, 0, wxGROW, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, _("&Position (tenths of a mm):"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_LEFT | wxDirection::wxLEFT | wxDirection::wxRIGHT | wxDirection::wxTOP, 5);

    m_tabEditCtrl = new wxTextCtrl( itemRichTextDialogPage1, ID_RICHTEXTTABSPAGE_TABEDIT, "", wxDefaultPosition, wxDefaultSize, 0 );
    m_tabEditCtrl->SetHelpText(_("The tab position."));
    if (wxRichTextTabsPage::ShowToolTips())
        m_tabEditCtrl->SetToolTip(_("The tab position."));
    itemBoxSizer5->Add(m_tabEditCtrl, 0, wxGROW|wxALL, 5);

    std::vector<std::string> m_tabListCtrlStrings;
    m_tabListCtrlStrings.push_back("The tab positions.");
    m_tabListCtrl = new wxListBox( itemRichTextDialogPage1, ID_RICHTEXTTABSPAGE_TABLIST, wxDefaultPosition, wxSize(80, 200), m_tabListCtrlStrings, wxLB_SINGLE );
    itemBoxSizer5->Add(m_tabListCtrl, 1, wxGROW | wxDirection::wxLEFT | wxDirection::wxRIGHT | wxDirection::wxBOTTOM, 5);

    itemBoxSizer4->Add(2, 1, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer10, 0, wxGROW, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, "", wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemStaticText11, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM, 5);

    wxButton* itemButton12 = new wxButton( itemRichTextDialogPage1, ID_RICHTEXTTABSPAGE_NEW_TAB, _("&New"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton12->SetHelpText(_("Click to create a new tab position."));
    if (wxRichTextTabsPage::ShowToolTips())
        itemButton12->SetToolTip(_("Click to create a new tab position."));
    itemBoxSizer10->Add(itemButton12, 0, wxGROW|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemRichTextDialogPage1, ID_RICHTEXTTABSPAGE_DELETE_TAB, _("&Delete"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton13->SetHelpText(_("Click to delete the selected tab position."));
    if (wxRichTextTabsPage::ShowToolTips())
        itemButton13->SetToolTip(_("Click to delete the selected tab position."));
    itemBoxSizer10->Add(itemButton13, 0, wxGROW | wxDirection::wxLEFT | wxDirection::wxRIGHT | wxDirection::wxBOTTOM, 5);

    wxButton* itemButton14 = new wxButton( itemRichTextDialogPage1, ID_RICHTEXTTABSPAGE_DELETE_ALL_TABS, _("Delete A&ll"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton14->SetHelpText(_("Click to delete all tab positions."));
    if (wxRichTextTabsPage::ShowToolTips())
        itemButton14->SetToolTip(_("Click to delete all tab positions."));
    itemBoxSizer10->Add(itemButton14, 0, wxGROW | wxDirection::wxLEFT | wxDirection::wxRIGHT | wxDirection::wxBOTTOM, 5);

////@end wxRichTextTabsPage content construction
}

/// Transfer data from/to window
bool wxRichTextTabsPage::TransferDataFromWindow()
{
    wxPanel::TransferDataFromWindow();

    wxRichTextAttr* attr = GetAttributes();

    if (m_tabsPresent)
    {
        std::vector<int> tabs;
        size_t i;
        for (i = 0; i < m_tabListCtrl->GetCount(); i++)
        {
            tabs.push_back(wxAtoi(m_tabListCtrl->GetString(i)));
        }
        attr->SetTabs(tabs);
    }
    return true;
}

bool wxRichTextTabsPage::TransferDataToWindow()
{
    wxPanel::TransferDataToWindow();

    wxRichTextAttr* attr = GetAttributes();

    m_tabListCtrl->Clear();
    m_tabEditCtrl->SetValue("");

    if (attr->HasTabs())
    {
        m_tabsPresent = true;
        size_t i;
        for (i = 0; i < attr->GetTabs().size(); i++)
        {
            wxString s(wxString::Format("%d", attr->GetTabs()[i]));
            m_tabListCtrl->Append(s);
        }
    }

    return true;
}

/// Sorts the tab array
// FIXME: Add a test.
void wxRichTextTabsPage::SortTabs()
{
    std::vector<int> tabs;

    for (size_t i = 0; i < m_tabListCtrl->GetCount(); i++)
    {
        tabs.push_back(wxAtoi(m_tabListCtrl->GetString(i)));
    }

    std::sort(tabs.begin(), tabs.end());

    m_tabListCtrl->Clear();

    for (size_t i = 0; i < tabs.size(); i++)
    {
        wxString s(wxString::Format("%d", tabs[i]));
        m_tabListCtrl->Append(s);
    }
}

wxRichTextAttr* wxRichTextTabsPage::GetAttributes()
{
    return wxRichTextFormattingDialog::GetDialogAttributes(this);
}

/*!
 * Should we show tooltips?
 */

bool wxRichTextTabsPage::ShowToolTips()
{
    return wxRichTextFormattingDialog::ShowToolTips();
}

/*!
 * Get bitmap resources
 */

wxBitmap wxRichTextTabsPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin wxRichTextTabsPage bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end wxRichTextTabsPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon wxRichTextTabsPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin wxRichTextTabsPage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end wxRichTextTabsPage icon retrieval
}

/*!
 * wxEVT_BUTTON event handler for ID_RICHTEXTTABSPAGE_NEW_TAB
 */

void wxRichTextTabsPage::OnNewTabClick( [[maybe_unused]] wxCommandEvent& event )
{
    wxString str = m_tabEditCtrl->GetValue();
    if (!str.empty() && str.IsNumber())
    {
        wxString s(wxString::Format("%d", wxAtoi(str)));

        m_tabListCtrl->Append(s);
        m_tabsPresent = true;

        SortTabs();
    }
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_RICHTEXTTABSPAGE_NEW_TAB
 */

void wxRichTextTabsPage::OnNewTabUpdate( wxUpdateUIEvent& event )
{
    // This may be a bit expensive - consider updating New button when text
    // changes in edit control
    wxString str = m_tabEditCtrl->GetValue();
    if (!str.empty() && str.IsNumber())
    {
        std::string s(wxString::Format("%d", wxAtoi(str)));
        event.Enable(m_tabListCtrl->FindString(s) == wxNOT_FOUND);
    }
    else
        event.Enable(false);
}


/*!
 * wxEVT_BUTTON event handler for ID_RICHTEXTTABSPAGE_DELETE_TAB
 */

void wxRichTextTabsPage::OnDeleteTabClick( [[maybe_unused]] wxCommandEvent& event )
{
    if (m_tabsPresent && m_tabListCtrl->GetCount() > 0 && m_tabListCtrl->GetSelection() != wxNOT_FOUND)
    {
        m_tabListCtrl->Delete(m_tabListCtrl->GetSelection());
    }
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_RICHTEXTTABSPAGE_DELETE_TAB
 */

void wxRichTextTabsPage::OnDeleteTabUpdate( wxUpdateUIEvent& event )
{
    event.Enable( m_tabsPresent && m_tabListCtrl->GetCount() > 0 && m_tabListCtrl->GetSelection() != wxNOT_FOUND );
}


/*!
 * wxEVT_BUTTON event handler for ID_RICHTEXTTABSPAGE_DELETE_ALL_TABS
 */

void wxRichTextTabsPage::OnDeleteAllTabsClick( [[maybe_unused]] wxCommandEvent& event )
{
    if (m_tabsPresent && m_tabListCtrl->GetCount() > 0)
    {
        m_tabListCtrl->Clear();
        m_tabEditCtrl->SetValue("");
    }
}

/*!
 * wxEVT_UPDATE_UI event handler for ID_RICHTEXTTABSPAGE_DELETE_ALL_TABS
 */

void wxRichTextTabsPage::OnDeleteAllTabsUpdate( wxUpdateUIEvent& event )
{
    event.Enable( m_tabsPresent && m_tabListCtrl->GetCount() > 0 );
}


/*!
 * wxEVT_LISTBOX event handler for ID_RICHTEXTTABSPAGE_TABLIST
 */

void wxRichTextTabsPage::OnTablistSelected( [[maybe_unused]] wxCommandEvent& event )
{
    wxString str = m_tabListCtrl->GetStringSelection();
    if (!str.empty())
        m_tabEditCtrl->SetValue(str);
}

#endif // wxUSE_RICHTEXT
