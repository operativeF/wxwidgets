/////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/choicdgg.cpp
// Purpose:     Choice dialogs
// Author:      Julian Smart
// Modified by: 03.11.00: VZ to add wxArrayString and multiple sel functions
// Created:     04/01/98
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_CHOICEDLG

#ifndef WX_PRECOMP
    import <string>;
    import <vector>;
#endif

#include "wx/button.h"
#include "wx/checklst.h"
#include "wx/dialog.h"
#include "wx/intl.h"
#include "wx/listbox.h"
#include "wx/sizer.h"
#include "wx/stattext.h"
#include "wx/utils.h"

#include "wx/statline.h"
#include "wx/settings.h"
#include "wx/generic/choicdgg.h"


// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

#define wxID_LISTBOX 3000

// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wrapper functions
// ----------------------------------------------------------------------------

std::string wxGetSingleChoice( const std::string& message,
                            const std::string& caption,
                            const std::vector<std::string>& choices,
                            wxWindow *parent,
                            [[maybe_unused]] int x, [[maybe_unused]] int y,
                            [[maybe_unused]] bool centre,
                            [[maybe_unused]] int width, [[maybe_unused]] int height,
                            int initialSelection)
{
    wxSingleChoiceDialog dialog(parent, message, caption, choices);

    dialog.SetSelection(initialSelection);
    return dialog.ShowModal() == wxID_OK ? dialog.GetStringSelection() : "";
}

int wxGetSingleChoiceIndex( const std::string& message,
                            const std::string& caption,
                            const std::vector<std::string>& choices,
                            wxWindow *parent,
                            [[maybe_unused]] int x, [[maybe_unused]] int y,
                            [[maybe_unused]] bool centre,
                            [[maybe_unused]] int width, [[maybe_unused]] int height,
                            int initialSelection)
{
    wxSingleChoiceDialog dialog(parent, message, caption, choices);

    dialog.SetSelection(initialSelection);
    return dialog.ShowModal() == wxID_OK ? dialog.GetSelection() : -1;
}

void *wxGetSingleChoiceData( const std::string& message,
                             const std::string& caption,
                             const std::vector<std::string>& choices,
                             void **client_data,
                             wxWindow *parent,
                             [[maybe_unused]] int x, [[maybe_unused]] int y,
                             [[maybe_unused]] bool centre,
                             [[maybe_unused]] int width, [[maybe_unused]] int height,
                             int initialSelection)
{
    wxSingleChoiceDialog dialog(parent, message, caption, choices,
                                client_data);

    dialog.SetSelection(initialSelection);
    return dialog.ShowModal() == wxID_OK ? dialog.GetSelectionData() : nullptr;
}

int wxGetSelectedChoices(std::vector<int>& selections,
                         const std::string& message,
                         const std::string& caption,
                         const std::vector<std::string>& choices,
                         wxWindow *parent,
                         [[maybe_unused]] int x, [[maybe_unused]] int y,
                         [[maybe_unused]] bool centre,
                         [[maybe_unused]] int width, [[maybe_unused]] int height)
{
    wxMultiChoiceDialog dialog(parent, message, caption, choices);

    // call this even if selections array is empty and this then (correctly)
    // deselects the first item which is selected by default
    dialog.SetSelections(selections);

    if ( dialog.ShowModal() != wxID_OK )
    {
        // NB: intentionally do not clear the selections array here, the caller
        //     might want to preserve its original contents if the dialog was
        //     cancelled
        return -1;
    }

    selections = dialog.GetSelections();
    return static_cast<int>(selections.size());
}

// ----------------------------------------------------------------------------
// wxAnyChoiceDialog
// ----------------------------------------------------------------------------

bool wxAnyChoiceDialog::Create(wxWindow *parent,
                               const std::string& message,
                               const std::string& caption,
                               const std::vector<std::string>& choices,
                               long styleDlg,
                               const wxPoint& pos,
                               long styleLbox)
{
    // extract the buttons styles from the dialog one and remove them from it
    const long styleBtns = styleDlg & (wxOK | wxCANCEL);
    styleDlg &= ~styleBtns;

    if ( !wxDialog::Create(GetParentForModalDialog(parent, styleDlg), wxID_ANY, caption, pos, wxDefaultSize, styleDlg) )
        return false;

    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

    // 1) text message
    topsizer->
        Add(CreateTextSizer(message), wxSizerFlags().Expand().TripleBorder());

    // 2) list box
    m_listbox = CreateList(choices, styleLbox);

    if ( !choices.empty() )
        m_listbox->SetSelection(0);

    topsizer->
        Add(m_listbox, wxSizerFlags().Expand().Proportion(1).TripleBorder(wxLEFT | wxRIGHT));

    // 3) buttons if any
    wxSizer *
        buttonSizer = CreateSeparatedButtonSizer(styleBtns);
    if ( buttonSizer )
    {
        topsizer->Add(buttonSizer, wxSizerFlags().Expand().DoubleBorder());
    }

    SetSizer( topsizer );

    topsizer->SetSizeHints( this );
    topsizer->Fit( this );

    if ( styleDlg & wxCENTRE )
        Centre(wxBOTH);

    m_listbox->SetFocus();

    return true;
}

wxListBoxBase *wxAnyChoiceDialog::CreateList(const std::vector<std::string>& choices, long styleLbox)
{
    return new wxListBox( this, wxID_LISTBOX,
                          wxDefaultPosition, wxDefaultSize,
                          choices,
                          styleLbox );
}

// ----------------------------------------------------------------------------
// wxSingleChoiceDialog
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(wxSingleChoiceDialog, wxDialog)
    EVT_BUTTON(wxID_OK, wxSingleChoiceDialog::OnOK)
    EVT_LISTBOX_DCLICK(wxID_LISTBOX, wxSingleChoiceDialog::OnListBoxDClick)
wxEND_EVENT_TABLE()

wxIMPLEMENT_DYNAMIC_CLASS(wxSingleChoiceDialog, wxDialog);

bool wxSingleChoiceDialog::Create( wxWindow *parent,
                                   const std::string& message,
                                   const std::string& caption,
                                   const std::vector<std::string>& choices,
                                   void **clientData,
                                   unsigned int style,
                                   const wxPoint& pos )
{
    if ( !wxAnyChoiceDialog::Create(parent, message, caption,
                                    choices,
                                    style, pos) )
        return false;

    m_selection = !choices.empty() ? 0 : -1;

    if (clientData)
    {
        for (size_t i = 0; i < choices.size(); i++)
            m_listbox->SetClientData(i, clientData[i]);
    }

    return true;
}

// Set the selection
void wxSingleChoiceDialog::SetSelection(int sel)
{
    wxCHECK_RET( sel >= 0 && (unsigned)sel < m_listbox->GetCount(),
                 "Invalid initial selection" );

    m_listbox->SetSelection(sel);
    m_selection = sel;
}

void wxSingleChoiceDialog::OnOK([[maybe_unused]] wxCommandEvent& event)
{
    DoChoice();
}

void wxSingleChoiceDialog::OnListBoxDClick([[maybe_unused]] wxCommandEvent& event)
{
    DoChoice();
}

void wxSingleChoiceDialog::DoChoice()
{
    m_selection = m_listbox->GetSelection();
    m_stringSelection = m_listbox->GetStringSelection();

    if ( m_listbox->HasClientUntypedData() )
        SetClientData(m_listbox->GetClientData(m_selection));

    EndModal(wxID_OK);
}

// ----------------------------------------------------------------------------
// wxMultiChoiceDialog
// ----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxMultiChoiceDialog, wxDialog);

bool wxMultiChoiceDialog::Create( wxWindow *parent,
                                  const std::string& message,
                                  const std::string& caption,
                                  const std::vector<std::string>& choices,
                                  unsigned int style,
                                  const wxPoint& pos )
{
    long styleLbox;
#if wxUSE_CHECKLISTBOX
    styleLbox = wxLB_ALWAYS_SB;
#else
    styleLbox = wxLB_ALWAYS_SB | wxLB_EXTENDED;
#endif

    return wxAnyChoiceDialog::Create(parent, message, caption,
                                    choices,
                                    style, pos,
                                    styleLbox);
}

void wxMultiChoiceDialog::SetSelections(const std::vector<int>& selections)
{
#if wxUSE_CHECKLISTBOX
    wxCheckListBox* checkListBox = wxDynamicCast(m_listbox, wxCheckListBox);
    if (checkListBox)
    {
        // first clear all currently selected items
        size_t n,
            count = checkListBox->GetCount();
        for ( n = 0; n < count; ++n )
        {
            if (checkListBox->IsChecked(n))
                checkListBox->Check(n, false);
        }

        // now select the ones which should be selected
        count = selections.size();
        for ( n = 0; n < count; n++ )
        {
            checkListBox->Check(selections[n]);
        }

        return;
    }
#endif

    // first clear all currently selected items
    size_t n,
           count = m_listbox->GetCount();
    for ( n = 0; n < count; ++n )
    {
        m_listbox->Deselect(n);
    }

    // now select the ones which should be selected
    count = selections.size();
    for ( n = 0; n < count; n++ )
    {
        m_listbox->Select(selections[n]);
    }
}

bool wxMultiChoiceDialog::TransferDataFromWindow()
{
    m_selections.clear();

#if wxUSE_CHECKLISTBOX
    wxCheckListBox* checkListBox = wxDynamicCast(m_listbox, wxCheckListBox);
    if (checkListBox)
    {
        size_t count = checkListBox->GetCount();
        for ( size_t n = 0; n < count; n++ )
        {
            if ( checkListBox->IsChecked(n) )
                m_selections.push_back(n);
        }
        return true;
    }
#endif

    size_t count = m_listbox->GetCount();
    for ( size_t n = 0; n < count; n++ )
    {
        if ( m_listbox->IsSelected(n) )
            m_selections.push_back(n);
    }

    return true;
}

#if wxUSE_CHECKLISTBOX

wxListBoxBase *wxMultiChoiceDialog::CreateList(const std::vector<std::string>& choices, long styleLbox)
{
    return new wxCheckListBox( this, wxID_LISTBOX,
                               wxDefaultPosition, wxDefaultSize,
                               choices,
                               styleLbox );
}

#endif // wxUSE_CHECKLISTBOX

#endif // wxUSE_CHOICEDLG
