/////////////////////////////////////////////////////////////////////////////
// Name:        src/richtext/richtextstylepage.cpp
// Purpose:     Implements the rich text formatting dialog style name tab.
// Author:      Julian Smart
// Modified by:
// Created:     10/5/2006 11:34:55 AM
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_RICHTEXT

#include "wx/richtext/richtextstylepage.h"

/*!
 * wxRichTextStylePage type definition
 */

wxIMPLEMENT_DYNAMIC_CLASS(wxRichTextStylePage, wxRichTextDialogPage);

/*!
 * wxRichTextStylePage event table definition
 */

wxBEGIN_EVENT_TABLE(wxRichTextStylePage, wxRichTextDialogPage)

////@begin wxRichTextStylePage event table entries
    EVT_UPDATE_UI( ID_RICHTEXTSTYLEPAGE_NEXT_STYLE, wxRichTextStylePage::OnNextStyleUpdate )

////@end wxRichTextStylePage event table entries

wxEND_EVENT_TABLE()

IMPLEMENT_HELP_PROVISION(wxRichTextStylePage)

/*!
 * wxRichTextStylePage constructors
 */

wxRichTextStylePage::wxRichTextStylePage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, unsigned int style )
{
    Create(parent, id, pos, size, style);
}

/*!
 * wxRichTextStylePage creator
 */

bool wxRichTextStylePage::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, unsigned int style )
{
////@begin wxRichTextStylePage creation
    wxRichTextDialogPage::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end wxRichTextStylePage creation
    return true;
}

/*!
 * Control creation for wxRichTextStylePage
 */

void wxRichTextStylePage::CreateControls()
{
////@begin wxRichTextStylePage content construction
    wxRichTextStylePage* itemRichTextDialogPage1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemRichTextDialogPage1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxGROW, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer5, 1, wxGROW, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, _("&Style:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_LEFT | wxDirection::wxLEFT | wxDirection::wxRIGHT | wxDirection::wxTOP, 5);

    m_styleName = new wxTextCtrl( itemRichTextDialogPage1, ID_RICHTEXTSTYLEPAGE_STYLE_NAME, "", wxDefaultPosition, wxSize(300, -1), wxTE_READONLY );
    m_styleName->SetHelpText(_("The style name."));
    if (wxRichTextStylePage::ShowToolTips())
        m_styleName->SetToolTip(_("The style name."));
    itemBoxSizer5->Add(m_styleName, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, _("&Based on:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText8, 0, wxALIGN_LEFT | wxDirection::wxLEFT | wxDirection::wxRIGHT | wxDirection::wxTOP, 5);

    std::vector<std::string> m_basedOnStrings;
    m_basedOn = new wxComboBox( itemRichTextDialogPage1, ID_RICHTEXTSTYLEPAGE_BASED_ON, "", wxDefaultPosition, wxSize(300, -1), m_basedOnStrings, wxCB_DROPDOWN );
    m_basedOn->SetHelpText(_("The style on which this style is based."));
    if (wxRichTextStylePage::ShowToolTips())
        m_basedOn->SetToolTip(_("The style on which this style is based."));
    itemBoxSizer5->Add(m_basedOn, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, _("&Next style:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText10, 0, wxALIGN_LEFT | wxDirection::wxLEFT | wxDirection::wxRIGHT | wxDirection::wxTOP, 5);

    std::vector<std::string> m_nextStyleStrings;
    m_nextStyle = new wxComboBox( itemRichTextDialogPage1, ID_RICHTEXTSTYLEPAGE_NEXT_STYLE, "", wxDefaultPosition, wxSize(300, -1), m_nextStyleStrings, wxCB_DROPDOWN );
    m_nextStyle->SetHelpText(_("The default style for the next paragraph."));
    if (wxRichTextStylePage::ShowToolTips())
        m_nextStyle->SetToolTip(_("The default style for the next paragraph."));
    itemBoxSizer5->Add(m_nextStyle, 0, wxGROW|wxALL, 5);

    itemBoxSizer3->Add(5, 5, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end wxRichTextStylePage content construction
}

/// Transfer data from/to window
bool wxRichTextStylePage::TransferDataFromWindow()
{
    wxPanel::TransferDataFromWindow();

    wxRichTextStyleDefinition* def = wxRichTextFormattingDialog::GetDialogStyleDefinition(this);
    if (def)
    {
        wxRichTextParagraphStyleDefinition* paraDef = dynamic_cast<wxRichTextParagraphStyleDefinition*>(def);
        if (paraDef)
            paraDef->SetNextStyle(m_nextStyle->GetValue());

        def->SetName(m_styleName->GetValue());
        def->SetBaseStyle(m_basedOn->GetValue());
    }

    return true;
}

bool wxRichTextStylePage::TransferDataToWindow()
{
    m_styleName->SetEditable(GetNameIsEditable());

    wxPanel::TransferDataToWindow();

    wxRichTextStyleDefinition* def = wxRichTextFormattingDialog::GetDialogStyleDefinition(this);
    if (def)
    {
        m_basedOn->Freeze();
        m_nextStyle->Freeze();

        wxRichTextParagraphStyleDefinition* paraDef = dynamic_cast<wxRichTextParagraphStyleDefinition*>(def);
        wxRichTextListStyleDefinition* listDef = dynamic_cast<wxRichTextListStyleDefinition*>(def);
        wxRichTextCharacterStyleDefinition* charDef = dynamic_cast<wxRichTextCharacterStyleDefinition*>(def);
        wxRichTextStyleSheet* sheet = wxRichTextFormattingDialog::GetDialog(this)->GetStyleSheet();
        wxRichTextBoxStyleDefinition* boxDef = dynamic_cast<wxRichTextBoxStyleDefinition*>(def);

        m_styleName->SetValue(def->GetName());

        if (listDef)
        {
            if (m_nextStyle->GetCount() == 0)
            {
                if (sheet)
                {
                    size_t i;
                    for (i = 0; i < sheet->GetListStyleCount(); i++)
                    {
                        wxRichTextListStyleDefinition* p = dynamic_cast<wxRichTextListStyleDefinition*>(sheet->GetListStyle(i));
                        if (p)
                            m_nextStyle->Append(p->GetName());
                    }
                }
            }
            m_nextStyle->SetValue(listDef->GetNextStyle());
        }
        else if (paraDef)
        {
            if (m_nextStyle->GetCount() == 0)
            {
                if (sheet)
                {
                    size_t i;
                    for (i = 0; i < sheet->GetParagraphStyleCount(); i++)
                    {
                        wxRichTextParagraphStyleDefinition* p = dynamic_cast<wxRichTextParagraphStyleDefinition*>(sheet->GetParagraphStyle(i));
                        if (p)
                            m_nextStyle->Append(p->GetName());
                    }
                }
            }
            m_nextStyle->SetValue(paraDef->GetNextStyle());
        }

        if (m_basedOn->GetCount() == 0)
        {
            if (sheet)
            {
                if (listDef)
                {
                    size_t i;
                    for (i = 0; i < sheet->GetListStyleCount(); i++)
                    {
                        wxRichTextListStyleDefinition* p = dynamic_cast<wxRichTextListStyleDefinition*>(sheet->GetListStyle(i));
                        if (p)
                            m_basedOn->Append(p->GetName());
                    }
                }
                else if (paraDef)
                {
                    size_t i;
                    for (i = 0; i < sheet->GetParagraphStyleCount(); i++)
                    {
                        wxRichTextParagraphStyleDefinition* p = dynamic_cast<wxRichTextParagraphStyleDefinition*>(sheet->GetParagraphStyle(i));
                        if (p)
                            m_basedOn->Append(p->GetName());
                    }
                }
                else if (boxDef)
                {
                    size_t i;
                    for (i = 0; i < sheet->GetBoxStyleCount(); i++)
                    {
                        wxRichTextBoxStyleDefinition* p = dynamic_cast<wxRichTextBoxStyleDefinition*>(sheet->GetBoxStyle(i));
                        if (p)
                            m_basedOn->Append(p->GetName());
                    }
                }
                else if (charDef)
                {
                    size_t i;
                    for (i = 0; i < sheet->GetCharacterStyleCount(); i++)
                    {
                        wxRichTextCharacterStyleDefinition* p = dynamic_cast<wxRichTextCharacterStyleDefinition*>(sheet->GetCharacterStyle(i));
                        if (p)
                            m_basedOn->Append(p->GetName());
                    }
                }
            }
        }

        m_basedOn->SetValue(def->GetBaseStyle());

        m_nextStyle->Thaw();
        m_basedOn->Thaw();
    }

    return true;
}

wxRichTextAttr* wxRichTextStylePage::GetAttributes()
{
    return wxRichTextFormattingDialog::GetDialogAttributes(this);
}

/*!
 * Should we show tooltips?
 */

bool wxRichTextStylePage::ShowToolTips()
{
    return wxRichTextFormattingDialog::ShowToolTips();
}

/*!
 * Get bitmap resources
 */

wxBitmap wxRichTextStylePage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin wxRichTextStylePage bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end wxRichTextStylePage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon wxRichTextStylePage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin wxRichTextStylePage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end wxRichTextStylePage icon retrieval
}
/*!
 * wxEVT_UPDATE_UI event handler for ID_RICHTEXTSTYLEPAGE_NEXT_STYLE
 */

void wxRichTextStylePage::OnNextStyleUpdate( wxUpdateUIEvent& event )
{
    wxRichTextStyleDefinition* def = wxRichTextFormattingDialog::GetDialogStyleDefinition(this);
    event.Enable(dynamic_cast<wxRichTextParagraphStyleDefinition*>(def) != nullptr);
}

#endif // wxUSE_RICHTEXT
