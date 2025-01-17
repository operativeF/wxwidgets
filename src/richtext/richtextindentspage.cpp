/////////////////////////////////////////////////////////////////////////////
// Name:        src/richtext/richtextindentspage.cpp
// Purpose:     Implements the rich text formatting dialog indents page.
// Author:      Julian Smart
// Modified by:
// Created:     10/3/2006 2:28:21 PM
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_RICHTEXT

#include "wx/richtext/richtextindentspage.h"

/*!
 * wxRichTextIndentsSpacingPage type definition
 */

wxIMPLEMENT_DYNAMIC_CLASS(wxRichTextIndentsSpacingPage, wxRichTextDialogPage);

/*!
 * wxRichTextIndentsSpacingPage event table definition
 */

wxBEGIN_EVENT_TABLE( wxRichTextIndentsSpacingPage, wxRichTextDialogPage )

////@begin wxRichTextIndentsSpacingPage event table entries
    EVT_RADIOBUTTON( ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_LEFT, wxRichTextIndentsSpacingPage::OnAlignmentLeftSelected )
    EVT_RADIOBUTTON( ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_RIGHT, wxRichTextIndentsSpacingPage::OnAlignmentRightSelected )
    EVT_RADIOBUTTON( ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_JUSTIFIED, wxRichTextIndentsSpacingPage::OnAlignmentJustifiedSelected )
    EVT_RADIOBUTTON( ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_CENTRED, wxRichTextIndentsSpacingPage::OnAlignmentCentredSelected )
    EVT_RADIOBUTTON( ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_INDETERMINATE, wxRichTextIndentsSpacingPage::OnAlignmentIndeterminateSelected )
    EVT_TEXT( ID_RICHTEXTINDENTSSPACINGPAGE_INDENT_LEFT, wxRichTextIndentsSpacingPage::OnIndentLeftUpdated )
    EVT_TEXT( ID_RICHTEXTINDENTSSPACINGPAGE_INDENT_LEFT_FIRST, wxRichTextIndentsSpacingPage::OnIndentLeftFirstUpdated )
    EVT_TEXT( ID_RICHTEXTINDENTSSPACINGPAGE_INDENT_RIGHT, wxRichTextIndentsSpacingPage::OnIndentRightUpdated )
    EVT_COMBOBOX( ID_RICHTEXTINDENTSSPACINGPAGE_OUTLINELEVEL, wxRichTextIndentsSpacingPage::OnRichtextOutlinelevelSelected )
    EVT_TEXT( ID_RICHTEXTINDENTSSPACINGPAGE_SPACING_BEFORE, wxRichTextIndentsSpacingPage::OnSpacingBeforeUpdated )
    EVT_TEXT( ID_RICHTEXTINDENTSSPACINGPAGE_SPACING_AFTER, wxRichTextIndentsSpacingPage::OnSpacingAfterUpdated )
    EVT_COMBOBOX( ID_RICHTEXTINDENTSSPACINGPAGE_SPACING_LINE, wxRichTextIndentsSpacingPage::OnSpacingLineSelected )
////@end wxRichTextIndentsSpacingPage event table entries

wxEND_EVENT_TABLE()

IMPLEMENT_HELP_PROVISION(wxRichTextIndentsSpacingPage)

wxRichTextIndentsSpacingPage::wxRichTextIndentsSpacingPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, unsigned int style )
{
    Create(parent, id, pos, size, style);
}

/*!
 * wxRichTextIndentsSpacingPage creator
 */

bool wxRichTextIndentsSpacingPage::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, unsigned int style )
{
////@begin wxRichTextIndentsSpacingPage creation
    wxRichTextDialogPage::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end wxRichTextIndentsSpacingPage creation
    return true;
}

/*!
 * Control creation for wxRichTextIndentsSpacingPage
 */

void wxRichTextIndentsSpacingPage::CreateControls()
{
////@begin wxRichTextIndentsSpacingPage content construction
    wxRichTextIndentsSpacingPage* itemRichTextDialogPage1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemRichTextDialogPage1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxGROW, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer5, 0, wxGROW, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, _("&Alignment"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_LEFT | wxDirection::wxLEFT | wxDirection::wxRIGHT | wxDirection::wxTOP, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer5->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    itemBoxSizer7->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL, 5);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer7->Add(itemBoxSizer9, 0, wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    m_alignmentLeft = new wxRadioButton( itemRichTextDialogPage1, ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_LEFT, _("&Left"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_alignmentLeft->SetValue(false);
    m_alignmentLeft->SetHelpText(_("Left-align text."));
    if (wxRichTextIndentsSpacingPage::ShowToolTips())
        m_alignmentLeft->SetToolTip(_("Left-align text."));
    itemBoxSizer9->Add(m_alignmentLeft, 0, wxALIGN_LEFT|wxALL, 5);

    m_alignmentRight = new wxRadioButton( itemRichTextDialogPage1, ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_RIGHT, _("&Right"), wxDefaultPosition, wxDefaultSize, 0 );
    m_alignmentRight->SetValue(false);
    m_alignmentRight->SetHelpText(_("Right-align text."));
    if (wxRichTextIndentsSpacingPage::ShowToolTips())
        m_alignmentRight->SetToolTip(_("Right-align text."));
    itemBoxSizer9->Add(m_alignmentRight, 0, wxALIGN_LEFT|wxALL, 5);

    m_alignmentJustified = new wxRadioButton( itemRichTextDialogPage1, ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_JUSTIFIED, _("&Justified"), wxDefaultPosition, wxDefaultSize, 0 );
    m_alignmentJustified->SetValue(false);
    m_alignmentJustified->SetHelpText(_("Justify text left and right."));
    if (wxRichTextIndentsSpacingPage::ShowToolTips())
        m_alignmentJustified->SetToolTip(_("Justify text left and right."));
    itemBoxSizer9->Add(m_alignmentJustified, 0, wxALIGN_LEFT|wxALL, 5);

    m_alignmentCentred = new wxRadioButton( itemRichTextDialogPage1, ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_CENTRED, _("Cen&tred"), wxDefaultPosition, wxDefaultSize, 0 );
    m_alignmentCentred->SetValue(false);
    m_alignmentCentred->SetHelpText(_("Centre text."));
    if (wxRichTextIndentsSpacingPage::ShowToolTips())
        m_alignmentCentred->SetToolTip(_("Centre text."));
    itemBoxSizer9->Add(m_alignmentCentred, 0, wxALIGN_LEFT|wxALL, 5);

    m_alignmentIndeterminate = new wxRadioButton( itemRichTextDialogPage1, ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_INDETERMINATE, _("&Indeterminate"), wxDefaultPosition, wxDefaultSize, 0 );
    m_alignmentIndeterminate->SetValue(false);
    m_alignmentIndeterminate->SetHelpText(_("Use the current alignment setting."));
    if (wxRichTextIndentsSpacingPage::ShowToolTips())
        m_alignmentIndeterminate->SetToolTip(_("Use the current alignment setting."));
    itemBoxSizer9->Add(m_alignmentIndeterminate, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer4->Add(2, 1, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    wxStaticLine* itemStaticLine16 = new wxStaticLine( itemRichTextDialogPage1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
    itemBoxSizer4->Add(itemStaticLine16, 0, wxGROW | wxDirection::wxLEFT | wxDirection::wxBOTTOM, 5);

    itemBoxSizer4->Add(2, 1, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer18 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer18, 0, wxGROW, 5);

    wxStaticText* itemStaticText19 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, _("&Indentation (tenths of a mm)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemStaticText19, 0, wxALIGN_LEFT | wxDirection::wxLEFT | wxDirection::wxRIGHT | wxDirection::wxTOP, 5);

    wxBoxSizer* itemBoxSizer20 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer18->Add(itemBoxSizer20, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer20->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL, 5);

    wxFlexGridSizer* itemFlexGridSizer22 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer20->Add(itemFlexGridSizer22, 0, wxALIGN_CENTER_VERTICAL, 5);

    wxStaticText* itemStaticText23 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, _("&Left:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer22->Add(itemStaticText23, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_indentLeft = new wxTextCtrl( itemRichTextDialogPage1, ID_RICHTEXTINDENTSSPACINGPAGE_INDENT_LEFT, "", wxDefaultPosition, wxSize(50, -1), 0 );
    m_indentLeft->SetHelpText(_("The left indent."));
    if (wxRichTextIndentsSpacingPage::ShowToolTips())
        m_indentLeft->SetToolTip(_("The left indent."));
    itemFlexGridSizer22->Add(m_indentLeft, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText25 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, _("Left (&first line):"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer22->Add(itemStaticText25, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_indentLeftFirst = new wxTextCtrl( itemRichTextDialogPage1, ID_RICHTEXTINDENTSSPACINGPAGE_INDENT_LEFT_FIRST, "", wxDefaultPosition, wxSize(50, -1), 0 );
    m_indentLeftFirst->SetHelpText(_("The first line indent."));
    if (wxRichTextIndentsSpacingPage::ShowToolTips())
        m_indentLeftFirst->SetToolTip(_("The first line indent."));
    itemFlexGridSizer22->Add(m_indentLeftFirst, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText27 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, _("&Right:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer22->Add(itemStaticText27, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_indentRight = new wxTextCtrl( itemRichTextDialogPage1, ID_RICHTEXTINDENTSSPACINGPAGE_INDENT_RIGHT, "", wxDefaultPosition, wxSize(50, -1), 0 );
    m_indentRight->SetHelpText(_("The right indent."));
    if (wxRichTextIndentsSpacingPage::ShowToolTips())
        m_indentRight->SetToolTip(_("The right indent."));
    itemFlexGridSizer22->Add(m_indentRight, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText29 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, _("&Outline level:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer22->Add(itemStaticText29, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    std::vector<std::string> m_outlineLevelCtrlStrings;
    m_outlineLevelCtrl = new wxComboBox( itemRichTextDialogPage1, ID_RICHTEXTINDENTSSPACINGPAGE_OUTLINELEVEL, "", wxDefaultPosition, wxSize(85, -1), m_outlineLevelCtrlStrings, wxCB_READONLY );
    m_outlineLevelCtrl->SetHelpText(_("The outline level."));
    if (wxRichTextIndentsSpacingPage::ShowToolTips())
        m_outlineLevelCtrl->SetToolTip(_("The outline level."));
    itemFlexGridSizer22->Add(m_outlineLevelCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer4->Add(2, 1, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    wxStaticLine* itemStaticLine32 = new wxStaticLine( itemRichTextDialogPage1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
    itemBoxSizer4->Add(itemStaticLine32, 0, wxGROW|wxTOP|wxBOTTOM, 5);

    itemBoxSizer4->Add(2, 1, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer34 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer34, 0, wxGROW, 5);

    wxStaticText* itemStaticText35 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, _("&Spacing (tenths of a mm)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer34->Add(itemStaticText35, 0, wxALIGN_LEFT | wxDirection::wxLEFT | wxDirection::wxRIGHT | wxDirection::wxTOP, 5);

    wxBoxSizer* itemBoxSizer36 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer34->Add(itemBoxSizer36, 0, wxALIGN_LEFT | wxDirection::wxALL, 5);

    itemBoxSizer36->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL, 5);

    wxFlexGridSizer* itemFlexGridSizer38 = new wxFlexGridSizer(30, 2, 0, 0);
    itemBoxSizer36->Add(itemFlexGridSizer38, 0, wxALIGN_CENTER_VERTICAL, 5);

    wxStaticText* itemStaticText39 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, _("&Before a paragraph:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer38->Add(itemStaticText39, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL | wxDirection::wxALL, 5);

    m_spacingBefore = new wxTextCtrl( itemRichTextDialogPage1, ID_RICHTEXTINDENTSSPACINGPAGE_SPACING_BEFORE, "", wxDefaultPosition, wxSize(50, -1), 0 );
    m_spacingBefore->SetHelpText(_("The spacing before the paragraph."));
    if (wxRichTextIndentsSpacingPage::ShowToolTips())
        m_spacingBefore->SetToolTip(_("The spacing before the paragraph."));
    itemFlexGridSizer38->Add(m_spacingBefore, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText41 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, _("&After a paragraph:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer38->Add(itemStaticText41, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL | wxDirection::wxALL, 5);

    m_spacingAfter = new wxTextCtrl( itemRichTextDialogPage1, ID_RICHTEXTINDENTSSPACINGPAGE_SPACING_AFTER, "", wxDefaultPosition, wxSize(50, -1), 0 );
    if (wxRichTextIndentsSpacingPage::ShowToolTips())
        m_spacingAfter->SetToolTip(_("The spacing after the paragraph."));
    itemFlexGridSizer38->Add(m_spacingAfter, 0, wxGROW|wxALIGN_CENTER_VERTICAL | wxDirection::wxALL, 5);

    wxStaticText* itemStaticText43 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, _("L&ine spacing:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer38->Add(itemStaticText43, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL | wxDirection::wxALL, 5);

    std::vector<std::string> m_spacingLineStrings;
    m_spacingLineStrings.push_back("(none)");
    m_spacingLineStrings.push_back("Single");
    m_spacingLineStrings.push_back("1.1");
    m_spacingLineStrings.push_back("1.2");
    m_spacingLineStrings.push_back("1.3");
    m_spacingLineStrings.push_back("1.4");
    m_spacingLineStrings.push_back("1.5");
    m_spacingLineStrings.push_back("1.6");
    m_spacingLineStrings.push_back("1.7");
    m_spacingLineStrings.push_back("1.8");
    m_spacingLineStrings.push_back("1.9");
    m_spacingLineStrings.push_back("2");
    m_spacingLine = new wxComboBox( itemRichTextDialogPage1, ID_RICHTEXTINDENTSSPACINGPAGE_SPACING_LINE, _("(none)"), wxDefaultPosition, wxSize(85, -1), m_spacingLineStrings, wxCB_READONLY );
    m_spacingLine->SetStringSelection(_("(none)"));
    m_spacingLine->SetHelpText(_("The line spacing."));
    if (wxRichTextIndentsSpacingPage::ShowToolTips())
        m_spacingLine->SetToolTip(_("The line spacing."));
    itemFlexGridSizer38->Add(m_spacingLine, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_pageBreakCtrl = new wxCheckBox( itemRichTextDialogPage1, ID_RICHTEXTINDENTSSPACINGPAGE_PAGEBREAK, _("&Page Break"), wxDefaultPosition, wxDefaultSize, 0 );
    m_pageBreakCtrl->SetValue(false);
    m_pageBreakCtrl->SetHelpText(_("Inserts a page break before the paragraph."));
    if (wxRichTextIndentsSpacingPage::ShowToolTips())
        m_pageBreakCtrl->SetToolTip(_("Inserts a page break before the paragraph."));
    itemBoxSizer34->Add(m_pageBreakCtrl, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer3->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL, 5);

    m_previewCtrl = new wxRichTextCtrl( itemRichTextDialogPage1, ID_RICHTEXTINDENTSSPACINGPAGE_PREVIEW_CTRL, "", wxDefaultPosition, wxSize(350, 100), wxBORDER_THEME|wxVSCROLL|wxTE_READONLY );
    m_previewCtrl->SetHelpText(_("Shows a preview of the paragraph settings."));
    if (wxRichTextIndentsSpacingPage::ShowToolTips())
        m_previewCtrl->SetToolTip(_("Shows a preview of the paragraph settings."));
    itemBoxSizer3->Add(m_previewCtrl, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end wxRichTextIndentsSpacingPage content construction

    std::vector<std::string> outlineLevelCtrlStrings;
    outlineLevelCtrlStrings.push_back("(none)");
    outlineLevelCtrlStrings.push_back("1");
    outlineLevelCtrlStrings.push_back("2");
    outlineLevelCtrlStrings.push_back("3");
    outlineLevelCtrlStrings.push_back("4");
    outlineLevelCtrlStrings.push_back("5");
    outlineLevelCtrlStrings.push_back("6");
    outlineLevelCtrlStrings.push_back("7");
    outlineLevelCtrlStrings.push_back("8");
    outlineLevelCtrlStrings.push_back("9");
    outlineLevelCtrlStrings.push_back("10");

    m_outlineLevelCtrl->Freeze();
    m_outlineLevelCtrl->Append(outlineLevelCtrlStrings);
    m_outlineLevelCtrl->Thaw();
    m_outlineLevelCtrl->SetStringSelection("(none)");
}

wxRichTextAttr* wxRichTextIndentsSpacingPage::GetAttributes()
{
    return wxRichTextFormattingDialog::GetDialogAttributes(this);
}

/// Updates the font preview
void wxRichTextIndentsSpacingPage::UpdatePreview()
{
    static constexpr char s_para1[] = "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. \
Nullam ante sapien, vestibulum nonummy, pulvinar sed, luctus ut, lacus.\n";

    static constexpr char s_para2[] = "Duis pharetra consequat dui. Cum sociis natoque penatibus \
et magnis dis parturient montes, nascetur ridiculus mus. Nullam vitae justo id mauris lobortis interdum.\n";

    static constexpr char s_para3[] = "Integer convallis dolor at augue \
iaculis malesuada. Donec bibendum ipsum ut ante porta fringilla.\n";

    TransferDataFromWindow();
    wxRichTextAttr attr(*GetAttributes());
    attr.SetFlags(attr.GetFlags() &
      (wxTEXT_ATTR_ALIGNMENT|wxTEXT_ATTR_LEFT_INDENT|wxTEXT_ATTR_RIGHT_INDENT|wxTEXT_ATTR_PARA_SPACING_BEFORE|wxTEXT_ATTR_PARA_SPACING_AFTER|
       wxTEXT_ATTR_LINE_SPACING|
       wxTEXT_ATTR_BULLET_STYLE|wxTEXT_ATTR_BULLET_NUMBER|wxTEXT_ATTR_BULLET_TEXT));

    wxFont font(m_previewCtrl->GetFont());
    font.SetPointSize(9);
    m_previewCtrl->SetFont(font);

    wxRichTextAttr normalParaAttr;
    normalParaAttr.SetFont(font);
    normalParaAttr.SetTextColour(wxColour("LIGHT GREY"));

    m_previewCtrl->Freeze();
    m_previewCtrl->Clear();

    m_previewCtrl->BeginStyle(normalParaAttr);
    m_previewCtrl->WriteText(s_para1);
    m_previewCtrl->EndStyle();

    m_previewCtrl->BeginStyle(attr);
    m_previewCtrl->WriteText(s_para2);
    m_previewCtrl->EndStyle();

    m_previewCtrl->BeginStyle(normalParaAttr);
    m_previewCtrl->WriteText(s_para3);
    m_previewCtrl->EndStyle();

    m_previewCtrl->Thaw();
}

/// Transfer data from/to window
bool wxRichTextIndentsSpacingPage::TransferDataFromWindow()
{
    wxPanel::TransferDataFromWindow();

    wxRichTextAttr* attr = GetAttributes();

    if (m_alignmentLeft->GetValue())
        attr->SetAlignment(wxTextAttrAlignment::Left);
    else if (m_alignmentCentred->GetValue())
        attr->SetAlignment(wxTextAttrAlignment::Centre);
    else if (m_alignmentRight->GetValue())
        attr->SetAlignment(wxTextAttrAlignment::Right);
    else if (m_alignmentJustified->GetValue())
        attr->SetAlignment(wxTextAttrAlignment::Justified);
    else
    {
        attr->SetAlignment(wxTextAttrAlignment::Default);
        attr->SetFlags(attr->GetFlags() & (~wxTEXT_ATTR_ALIGNMENT));
    }

    wxString leftIndent(m_indentLeft->GetValue());
    wxString leftFirstIndent(m_indentLeftFirst->GetValue());
    if (!leftIndent.empty() || !leftFirstIndent.empty())
    {
        int visualLeftIndent = 0;
        if (!leftIndent.empty())
            visualLeftIndent = wxAtoi(leftIndent);

        int visualLeftFirstIndent = wxAtoi(leftFirstIndent);
        int actualLeftIndent = visualLeftFirstIndent;
        int actualLeftSubIndent = visualLeftIndent - visualLeftFirstIndent;

        attr->SetLeftIndent(actualLeftIndent, actualLeftSubIndent);
    }
    else
        attr->SetFlags(attr->GetFlags() & (~wxTEXT_ATTR_LEFT_INDENT));

    wxString rightIndent(m_indentRight->GetValue());
    if (!rightIndent.empty())
        attr->SetRightIndent(wxAtoi(rightIndent));
    else
        attr->SetFlags(attr->GetFlags() & (~wxTEXT_ATTR_RIGHT_INDENT));

    wxString spacingAfter(m_spacingAfter->GetValue());
    if (!spacingAfter.empty())
        attr->SetParagraphSpacingAfter(wxAtoi(spacingAfter));
    else
        attr->SetFlags(attr->GetFlags() & (~wxTEXT_ATTR_PARA_SPACING_AFTER));

    wxString spacingBefore(m_spacingBefore->GetValue());
    if (!spacingBefore.empty())
        attr->SetParagraphSpacingBefore(wxAtoi(spacingBefore));
    else
        attr->SetFlags(attr->GetFlags() & (~wxTEXT_ATTR_PARA_SPACING_BEFORE));

    int spacingIndex = m_spacingLine->GetSelection() - 1;
    int lineSpacing = 0;
    if (spacingIndex > -1)
        lineSpacing = 10 + spacingIndex;

    if (lineSpacing == 0)
        attr->SetFlags(attr->GetFlags() & (~wxTEXT_ATTR_LINE_SPACING));
    else
        attr->SetLineSpacing(lineSpacing);

    int outlineLevel = m_outlineLevelCtrl->GetSelection();
    if (outlineLevel == wxNOT_FOUND || outlineLevel == 0)
    {
        attr->SetOutlineLevel(-1);
        attr->SetFlags(attr->GetFlags() & (~wxTEXT_ATTR_OUTLINE_LEVEL));
    }
    else
        attr->SetOutlineLevel(outlineLevel-1);

    attr->SetPageBreak(m_pageBreakCtrl->GetValue());

    return true;
}

bool wxRichTextIndentsSpacingPage::TransferDataToWindow()
{
    m_dontUpdate = true;

    wxPanel::TransferDataToWindow();

    wxRichTextAttr* attr = GetAttributes();

    if (attr->HasAlignment())
    {
        if (attr->GetAlignment() == wxTextAttrAlignment::Left)
            m_alignmentLeft->SetValue(true);
        else if (attr->GetAlignment() == wxTextAttrAlignment::Right)
            m_alignmentRight->SetValue(true);
        else if (attr->GetAlignment() == wxTextAttrAlignment::Centre)
            m_alignmentCentred->SetValue(true);
        else if (attr->GetAlignment() == wxTextAttrAlignment::Justified)
            m_alignmentJustified->SetValue(true);
        else
            m_alignmentIndeterminate->SetValue(true);
    }
    else
        m_alignmentIndeterminate->SetValue(true);

    if (attr->HasLeftIndent())
    {
        wxString leftIndent(wxString::Format("%ld", attr->GetLeftIndent() + attr->GetLeftSubIndent()));
        wxString leftFirstIndent(wxString::Format("%ld", attr->GetLeftIndent()));

        m_indentLeft->SetValue(leftIndent);
        m_indentLeftFirst->SetValue(leftFirstIndent);
    }
    else
    {
        m_indentLeft->SetValue("");
        m_indentLeftFirst->SetValue("");
    }

    if (attr->HasRightIndent())
    {
        wxString rightIndent(wxString::Format("%ld", attr->GetRightIndent()));

        m_indentRight->SetValue(rightIndent);
    }
    else
        m_indentRight->SetValue("");

    if (attr->HasParagraphSpacingAfter())
    {
        wxString spacingAfter(wxString::Format("%d", attr->GetParagraphSpacingAfter()));

        m_spacingAfter->SetValue(spacingAfter);
    }
    else
        m_spacingAfter->SetValue("");

    if (attr->HasParagraphSpacingBefore())
    {
        wxString spacingBefore(wxString::Format("%d", attr->GetParagraphSpacingBefore()));

        m_spacingBefore->SetValue(spacingBefore);
    }
    else
        m_spacingBefore->SetValue("");

    if (attr->HasLineSpacing())
    {
        int index = 0;

        int lineSpacing = attr->GetLineSpacing();
        if (lineSpacing >= 10 && lineSpacing <= 20)
            index = (lineSpacing - 10) + 1;
        else
            index = 0;

        m_spacingLine->SetSelection(index);
    }
    else
        m_spacingLine->SetSelection(0);

    if (attr->HasOutlineLevel())
    {
        int outlineLevel = attr->GetOutlineLevel();
        if (outlineLevel < 0)
            outlineLevel = 0;
        if (outlineLevel > 9)
            outlineLevel = 9;

        m_outlineLevelCtrl->SetSelection(outlineLevel+1);
    }
    else
        m_outlineLevelCtrl->SetSelection(0);

    m_pageBreakCtrl->SetValue(attr->HasPageBreak());

    UpdatePreview();

    m_dontUpdate = false;

    return true;
}


/*!
 * Should we show tooltips?
 */

bool wxRichTextIndentsSpacingPage::ShowToolTips()
{
    return wxRichTextFormattingDialog::ShowToolTips();
}

/*!
 * Get bitmap resources
 */

wxBitmap wxRichTextIndentsSpacingPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin wxRichTextIndentsSpacingPage bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end wxRichTextIndentsSpacingPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon wxRichTextIndentsSpacingPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin wxRichTextIndentsSpacingPage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end wxRichTextIndentsSpacingPage icon retrieval
}
/*!
 * wxEVT_RADIOBUTTON event handler for ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_LEFT
 */

void wxRichTextIndentsSpacingPage::OnAlignmentLeftSelected( [[maybe_unused]] wxCommandEvent& event )
{
    if (!m_dontUpdate)
        UpdatePreview();
}


/*!
 * wxEVT_RADIOBUTTON event handler for ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_RIGHT
 */

void wxRichTextIndentsSpacingPage::OnAlignmentRightSelected( [[maybe_unused]] wxCommandEvent& event )
{
    if (!m_dontUpdate)
        UpdatePreview();
}


/*!
 * wxEVT_RADIOBUTTON event handler for ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_JUSTIFIED
 */

void wxRichTextIndentsSpacingPage::OnAlignmentJustifiedSelected( [[maybe_unused]] wxCommandEvent& event )
{
    if (!m_dontUpdate)
        UpdatePreview();
}


/*!
 * wxEVT_RADIOBUTTON event handler for ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_CENTRED
 */

void wxRichTextIndentsSpacingPage::OnAlignmentCentredSelected( [[maybe_unused]] wxCommandEvent& event )
{
    if (!m_dontUpdate)
        UpdatePreview();
}


/*!
 * wxEVT_RADIOBUTTON event handler for ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_INDETERMINATE
 */

void wxRichTextIndentsSpacingPage::OnAlignmentIndeterminateSelected( [[maybe_unused]] wxCommandEvent& event )
{
    if (!m_dontUpdate)
        UpdatePreview();
}


/*!
 * wxEVT_TEXT event handler for ID_RICHTEXTINDENTSSPACINGPAGE_INDENT_LEFT
 */

void wxRichTextIndentsSpacingPage::OnIndentLeftUpdated( [[maybe_unused]] wxCommandEvent& event )
{
    if (!m_dontUpdate)
        UpdatePreview();
}


/*!
 * wxEVT_TEXT event handler for ID_RICHTEXTINDENTSSPACINGPAGE_INDENT_LEFT_FIRST
 */

void wxRichTextIndentsSpacingPage::OnIndentLeftFirstUpdated( [[maybe_unused]] wxCommandEvent& event )
{
    if (!m_dontUpdate)
        UpdatePreview();
}


/*!
 * wxEVT_TEXT event handler for ID_RICHTEXTINDENTSSPACINGPAGE_INDENT_RIGHT
 */

void wxRichTextIndentsSpacingPage::OnIndentRightUpdated( [[maybe_unused]] wxCommandEvent& event )
{
    if (!m_dontUpdate)
        UpdatePreview();
}


/*!
 * wxEVT_TEXT event handler for ID_RICHTEXTINDENTSSPACINGPAGE_SPACING_BEFORE
 */

void wxRichTextIndentsSpacingPage::OnSpacingBeforeUpdated( [[maybe_unused]] wxCommandEvent& event )
{
    if (!m_dontUpdate)
        UpdatePreview();
}


/*!
 * wxEVT_TEXT event handler for ID_RICHTEXTINDENTSSPACINGPAGE_SPACING_AFTER
 */

void wxRichTextIndentsSpacingPage::OnSpacingAfterUpdated( [[maybe_unused]] wxCommandEvent& event )
{
    if (!m_dontUpdate)
        UpdatePreview();
}

/*!
 * wxEVT_COMBOBOX event handler for ID_RICHTEXTINDENTSSPACINGPAGE_SPACING_LINE
 */

void wxRichTextIndentsSpacingPage::OnSpacingLineSelected( [[maybe_unused]] wxCommandEvent& event )
{
    if (!m_dontUpdate)
        UpdatePreview();
}

/*!
 * wxEVT_COMBOBOX event handler for ID_RICHTEXTINDENTSSPACINGPAGE_OUTLINELEVEL
 */

void wxRichTextIndentsSpacingPage::OnRichtextOutlinelevelSelected( [[maybe_unused]] wxCommandEvent& event )
{
    if (!m_dontUpdate)
        UpdatePreview();
}

#endif // wxUSE_RICHTEXT
