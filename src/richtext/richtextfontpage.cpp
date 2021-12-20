/////////////////////////////////////////////////////////////////////////////
// Name:        src/richtext/richtextfontpage.cpp
// Purpose:     Font page for wxRichTextFormattingDialog
// Author:      Julian Smart
// Modified by:
// Created:     2006-10-02
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/richtext/richtextfontpage.h"

#include "wx/combobox.h"
#include "wx/checkbox.h"
#include "wx/listbox.h"
#include "wx/stattext.h"

import WX.Core.Sizer;

/*!
 * wxRichTextFontPage type definition
 */

wxIMPLEMENT_DYNAMIC_CLASS(wxRichTextFontPage, wxRichTextDialogPage);

/*!
 * wxRichTextFontPage event table definition
 */

wxBEGIN_EVENT_TABLE( wxRichTextFontPage, wxRichTextDialogPage )
    EVT_LISTBOX( ID_RICHTEXTFONTPAGE_FACELISTBOX, wxRichTextFontPage::OnFaceListBoxSelected )
    EVT_BUTTON( ID_RICHTEXTFONTPAGE_COLOURCTRL, wxRichTextFontPage::OnColourClicked )
    EVT_BUTTON( ID_RICHTEXTFONTPAGE_BGCOLOURCTRL, wxRichTextFontPage::OnColourClicked )

////@begin wxRichTextFontPage event table entries
    EVT_IDLE( wxRichTextFontPage::OnIdle )
    EVT_TEXT( ID_RICHTEXTFONTPAGE_FACETEXTCTRL, wxRichTextFontPage::OnFaceTextCtrlUpdated )
    EVT_TEXT( ID_RICHTEXTFONTPAGE_SIZETEXTCTRL, wxRichTextFontPage::OnSizeTextCtrlUpdated )
    EVT_SPIN_UP( ID_RICHTEXTFONTPAGE_SPINBUTTONS, wxRichTextFontPage::OnRichtextfontpageSpinbuttonsUp )
    EVT_SPIN_DOWN( ID_RICHTEXTFONTPAGE_SPINBUTTONS, wxRichTextFontPage::OnRichtextfontpageSpinbuttonsDown )
    EVT_CHOICE( ID_RICHTEXTFONTPAGE_SIZE_UNITS, wxRichTextFontPage::OnRichtextfontpageSizeUnitsSelected )
    EVT_LISTBOX( ID_RICHTEXTFONTPAGE_SIZELISTBOX, wxRichTextFontPage::OnSizeListBoxSelected )
    EVT_COMBOBOX( ID_RICHTEXTFONTPAGE_STYLECTRL, wxRichTextFontPage::OnStyleCtrlSelected )
    EVT_COMBOBOX( ID_RICHTEXTFONTPAGE_WEIGHTCTRL, wxRichTextFontPage::OnWeightCtrlSelected )
    EVT_COMBOBOX( ID_RICHTEXTFONTPAGE_UNDERLINING_CTRL, wxRichTextFontPage::OnUnderliningCtrlSelected )
    EVT_CHECKBOX( ID_RICHTEXTFONTPAGE_COLOURCTRL_LABEL, wxRichTextFontPage::OnUnderliningCtrlSelected )
    EVT_CHECKBOX( ID_RICHTEXTFONTPAGE_BGCOLOURCTRL_LABEL, wxRichTextFontPage::OnUnderliningCtrlSelected )
    EVT_CHECKBOX( ID_RICHTEXTFONTPAGE_STRIKETHROUGHCTRL, wxRichTextFontPage::OnStrikethroughctrlClick )
    EVT_CHECKBOX( ID_RICHTEXTFONTPAGE_CAPSCTRL, wxRichTextFontPage::OnCapsctrlClick )
    EVT_CHECKBOX( ID_RICHTEXTFONTPAGE_SMALLCAPSCTRL, wxRichTextFontPage::OnCapsctrlClick )
    EVT_CHECKBOX( ID_RICHTEXTFONTPAGE_SUPERSCRIPT, wxRichTextFontPage::OnRichtextfontpageSuperscriptClick )
    EVT_CHECKBOX( ID_RICHTEXTFONTPAGE_SUBSCRIPT, wxRichTextFontPage::OnRichtextfontpageSubscriptClick )
////@end wxRichTextFontPage event table entries

wxEND_EVENT_TABLE()

IMPLEMENT_HELP_PROVISION(wxRichTextFontPage)

/*!
 * wxRichTextFontPage constructors
 */

wxRichTextFontPage::wxRichTextFontPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, unsigned int style )
{
    Create(parent, id, pos, size, style);
}

/*!
 * wxRichTextFontPage creator
 */

bool wxRichTextFontPage::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, unsigned int style )
{
////@begin wxRichTextFontPage creation
    wxRichTextDialogPage::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end wxRichTextFontPage creation
    return true;
}

/*!
 * Control creation for wxRichTextFontPage
 */

void wxRichTextFontPage::CreateControls()
{
////@begin wxRichTextFontPage content construction
    wxRichTextFontPage* itemRichTextDialogPage1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemRichTextDialogPage1->SetSizer(itemBoxSizer2);

    m_innerSizer = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(m_innerSizer, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    m_innerSizer->Add(itemBoxSizer4, 0, wxGROW, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer5, 1, wxGROW, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, _("&Font:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP, 5);

    m_faceTextCtrl = new wxTextCtrl( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_FACETEXTCTRL, "", wxDefaultPosition, wxDefaultSize, 0 );
    m_faceTextCtrl->SetHelpText(_("Type a font name."));
    if (wxRichTextFontPage::ShowToolTips())
        m_faceTextCtrl->SetToolTip(_("Type a font name."));
    itemBoxSizer5->Add(m_faceTextCtrl, 0, wxGROW|wxLEFT|wxRIGHT|wxTOP, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer8, 0, wxGROW, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, _("&Size:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(itemStaticText9, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer8->Add(itemBoxSizer10, 0, wxGROW, 5);

    m_sizeTextCtrl = new wxTextCtrl( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_SIZETEXTCTRL, "", wxDefaultPosition, wxSize(50, -1), 0 );
    m_sizeTextCtrl->SetHelpText(_("Type a size in points."));
    if (wxRichTextFontPage::ShowToolTips())
        m_sizeTextCtrl->SetToolTip(_("Type a size in points."));
    itemBoxSizer10->Add(m_sizeTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP, 5);

    m_fontSizeSpinButtons = new wxSpinButton( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_SPINBUTTONS, wxDefaultPosition, wxSize(-1, 20), wxSP_VERTICAL );
    m_fontSizeSpinButtons->SetRange(0, 100);
    m_fontSizeSpinButtons->SetValue(0);
    itemBoxSizer10->Add(m_fontSizeSpinButtons, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP, 5);

    std::vector<std::string> m_sizeUnitsCtrlStrings;
    m_sizeUnitsCtrlStrings.push_back("pt");
    m_sizeUnitsCtrlStrings.push_back("px");
    m_sizeUnitsCtrl = new wxChoice( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_SIZE_UNITS, wxDefaultPosition, wxDefaultSize, m_sizeUnitsCtrlStrings, 0 );
    m_sizeUnitsCtrl->SetStringSelection(_("pt"));
    m_sizeUnitsCtrl->SetHelpText(_("The font size units, points or pixels."));
    if (wxRichTextFontPage::ShowToolTips())
        m_sizeUnitsCtrl->SetToolTip(_("The font size units, points or pixels."));
    itemBoxSizer10->Add(m_sizeUnitsCtrl, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP, 5);

    m_fontListBoxParent = new wxBoxSizer(wxHORIZONTAL);
    m_innerSizer->Add(m_fontListBoxParent, 1, wxGROW, 5);

    m_faceListBox = new wxRichTextFontListBox( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_FACELISTBOX, wxDefaultPosition, wxSize(200, 100), 0 );
    m_faceListBox->SetHelpText(_("Lists the available fonts."));
    if (wxRichTextFontPage::ShowToolTips())
        m_faceListBox->SetToolTip(_("Lists the available fonts."));
    m_fontListBoxParent->Add(m_faceListBox, 1, wxGROW|wxALL|wxFIXED_MINSIZE, 5);

    std::vector<std::string> m_sizeListBoxStrings;
    m_sizeListBox = new wxListBox( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_SIZELISTBOX, wxDefaultPosition, wxSize(50, -1), m_sizeListBoxStrings, wxLB_SINGLE );
    m_sizeListBox->SetHelpText(_("Lists font sizes in points."));
    if (wxRichTextFontPage::ShowToolTips())
        m_sizeListBox->SetToolTip(_("Lists font sizes in points."));
    m_fontListBoxParent->Add(m_sizeListBox, 0, wxGROW|wxALL|wxFIXED_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxHORIZONTAL);
    m_innerSizer->Add(itemBoxSizer17, 0, wxGROW, 5);

    wxBoxSizer* itemBoxSizer18 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer17->Add(itemBoxSizer18, 0, wxGROW, 5);

    wxStaticText* itemStaticText19 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, _("Font st&yle:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemStaticText19, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP, 5);

    std::vector<std::string> m_styleCtrlStrings;
    m_styleCtrl = new wxComboBox( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_STYLECTRL, "", wxDefaultPosition, wxSize(110, -1), m_styleCtrlStrings, wxCB_READONLY );
    m_styleCtrl->SetHelpText(_("Select regular or italic style."));
    if (wxRichTextFontPage::ShowToolTips())
        m_styleCtrl->SetToolTip(_("Select regular or italic style."));
    itemBoxSizer18->Add(m_styleCtrl, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer21 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer17->Add(itemBoxSizer21, 0, wxGROW, 5);

    wxStaticText* itemStaticText22 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, _("Font &weight:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer21->Add(itemStaticText22, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP, 5);

    std::vector<std::string> m_weightCtrlStrings;
    m_weightCtrl = new wxComboBox( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_WEIGHTCTRL, "", wxDefaultPosition, wxSize(110, -1), m_weightCtrlStrings, wxCB_READONLY );
    m_weightCtrl->SetHelpText(_("Select regular or bold."));
    if (wxRichTextFontPage::ShowToolTips())
        m_weightCtrl->SetToolTip(_("Select regular or bold."));
    itemBoxSizer21->Add(m_weightCtrl, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer24 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer17->Add(itemBoxSizer24, 0, wxGROW, 5);

    wxStaticText* itemStaticText25 = new wxStaticText( itemRichTextDialogPage1, wxID_STATIC, _("&Underlining:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer24->Add(itemStaticText25, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP, 5);

    std::vector<std::string> m_underliningCtrlStrings;
    m_underliningCtrl = new wxComboBox( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_UNDERLINING_CTRL, "", wxDefaultPosition, wxSize(110, -1), m_underliningCtrlStrings, wxCB_READONLY );
    m_underliningCtrl->SetHelpText(_("Select underlining or no underlining."));
    if (wxRichTextFontPage::ShowToolTips())
        m_underliningCtrl->SetToolTip(_("Select underlining or no underlining."));
    itemBoxSizer24->Add(m_underliningCtrl, 0, wxGROW|wxALL, 5);

    itemBoxSizer17->Add(0, 0, 1, wxALIGN_CENTER_VERTICAL|wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer17->Add(itemBoxSizer28, 0, wxGROW, 5);

    m_textColourLabel = new wxCheckBox( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_COLOURCTRL_LABEL, _("&Colour:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_textColourLabel->SetValue(false);
    itemBoxSizer28->Add(m_textColourLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_colourCtrl = new wxRichTextColourSwatchCtrl( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_COLOURCTRL, wxDefaultPosition, wxSize(40, 20), 0 );
    m_colourCtrl->SetHelpText(_("Click to change the text colour."));
    if (wxRichTextFontPage::ShowToolTips())
        m_colourCtrl->SetToolTip(_("Click to change the text colour."));
    itemBoxSizer28->Add(m_colourCtrl, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer31 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer17->Add(itemBoxSizer31, 0, wxGROW, 5);

    m_bgColourLabel = new wxCheckBox( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_BGCOLOURCTRL_LABEL, _("&Bg colour:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_bgColourLabel->SetValue(false);
    itemBoxSizer31->Add(m_bgColourLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT|wxTOP, 5);

    m_bgColourCtrl = new wxRichTextColourSwatchCtrl( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_BGCOLOURCTRL, wxDefaultPosition, wxSize(40, 20), 0 );
    m_bgColourCtrl->SetHelpText(_("Click to change the text background colour."));
    if (wxRichTextFontPage::ShowToolTips())
        m_bgColourCtrl->SetToolTip(_("Click to change the text background colour."));
    itemBoxSizer31->Add(m_bgColourCtrl, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer34 = new wxBoxSizer(wxHORIZONTAL);
    m_innerSizer->Add(itemBoxSizer34, 0, wxGROW, 5);

    m_strikethroughCtrl = new wxCheckBox( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_STRIKETHROUGHCTRL, _("&Strikethrough"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER );
    m_strikethroughCtrl->SetValue(false);
    m_strikethroughCtrl->SetHelpText(_("Check to show a line through the text."));
    if (wxRichTextFontPage::ShowToolTips())
        m_strikethroughCtrl->SetToolTip(_("Check to show a line through the text."));
    itemBoxSizer34->Add(m_strikethroughCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_capitalsCtrl = new wxCheckBox( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_CAPSCTRL, _("Ca&pitals"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER );
    m_capitalsCtrl->SetValue(false);
    m_capitalsCtrl->SetHelpText(_("Check to show the text in capitals."));
    if (wxRichTextFontPage::ShowToolTips())
        m_capitalsCtrl->SetToolTip(_("Check to show the text in capitals."));
    itemBoxSizer34->Add(m_capitalsCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_smallCapitalsCtrl = new wxCheckBox( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_SMALLCAPSCTRL, _("Small C&apitals"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER );
    m_smallCapitalsCtrl->SetValue(false);
    m_smallCapitalsCtrl->SetHelpText(_("Check to show the text in small capitals."));
    if (wxRichTextFontPage::ShowToolTips())
        m_smallCapitalsCtrl->SetToolTip(_("Check to show the text in small capitals."));
    itemBoxSizer34->Add(m_smallCapitalsCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_superscriptCtrl = new wxCheckBox( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_SUPERSCRIPT, _("Supe&rscript"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER );
    m_superscriptCtrl->SetValue(false);
    m_superscriptCtrl->SetHelpText(_("Check to show the text in superscript."));
    if (wxRichTextFontPage::ShowToolTips())
        m_superscriptCtrl->SetToolTip(_("Check to show the text in superscript."));
    itemBoxSizer34->Add(m_superscriptCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_subscriptCtrl = new wxCheckBox( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_SUBSCRIPT, _("Subscrip&t"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER );
    m_subscriptCtrl->SetValue(false);
    m_subscriptCtrl->SetHelpText(_("Check to show the text in subscript."));
    if (wxRichTextFontPage::ShowToolTips())
        m_subscriptCtrl->SetToolTip(_("Check to show the text in subscript."));
    itemBoxSizer34->Add(m_subscriptCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_rtlParentSizer = new wxBoxSizer(wxHORIZONTAL);
    m_innerSizer->Add(m_rtlParentSizer, 0, wxALIGN_LEFT, 5);

    m_rtlCtrl = new wxCheckBox( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_RIGHT_TO_LEFT, _("Rig&ht-to-left"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER );
    m_rtlCtrl->SetValue(false);
    m_rtlCtrl->SetHelpText(_("Check to indicate right-to-left text layout."));
    if (wxRichTextFontPage::ShowToolTips())
        m_rtlCtrl->SetToolTip(_("Check to indicate right-to-left text layout."));
    m_rtlParentSizer->Add(m_rtlCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_suppressHyphenationCtrl = new wxCheckBox( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_SUBSCRIPT_SUPPRESS_HYPHENATION, _("Suppress hyphe&nation"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER );
    m_suppressHyphenationCtrl->SetValue(false);
    m_suppressHyphenationCtrl->SetHelpText(_("Check to suppress hyphenation."));
    if (wxRichTextFontPage::ShowToolTips())
        m_suppressHyphenationCtrl->SetToolTip(_("Check to suppress hyphenation."));
    m_rtlParentSizer->Add(m_suppressHyphenationCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_innerSizer->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL, 5);

    m_previewCtrl = new wxRichTextFontPreviewCtrl( itemRichTextDialogPage1, ID_RICHTEXTFONTPAGE_PREVIEWCTRL, wxDefaultPosition, wxSize(100, 60), 0 );
    m_previewCtrl->SetHelpText(_("Shows a preview of the font settings."));
    if (wxRichTextFontPage::ShowToolTips())
        m_previewCtrl->SetToolTip(_("Shows a preview of the font settings."));
    m_innerSizer->Add(m_previewCtrl, 0, wxGROW|wxALL, 5);

////@end wxRichTextFontPage content construction

    m_fontSizeSpinButtons->SetRange(0, 999);

    if ((GetAllowedTextEffects() & wxTEXT_ATTR_EFFECT_RTL) == 0)
        m_rtlParentSizer->Show(m_rtlCtrl, false);
    if ((GetAllowedTextEffects() & wxTEXT_ATTR_EFFECT_SUPPRESS_HYPHENATION) == 0)
        m_rtlParentSizer->Show(m_suppressHyphenationCtrl, false);

    if ((GetAllowedTextEffects() & (wxTEXT_ATTR_EFFECT_SUPPRESS_HYPHENATION|wxTEXT_ATTR_EFFECT_RTL)) == 0)        
        m_innerSizer->Show(m_rtlParentSizer, false);

    m_faceListBox->UpdateFonts();

    m_styleCtrl->Append(_("(none)"));
    m_styleCtrl->Append(_("Regular"));
    m_styleCtrl->Append(_("Italic"));

    m_weightCtrl->Append(_("(none)"));
    m_weightCtrl->Append(_("Regular"));
    m_weightCtrl->Append(_("Bold"));

    m_underliningCtrl->Append(_("(none)"));
    m_underliningCtrl->Append(_("Not underlined"));
    m_underliningCtrl->Append(_("Underlined"));

    wxString nStr;
    int i;
    for (i = 8; i < 40; i++)
    {
        nStr.Printf("%d", i);
        m_sizeListBox->Append(nStr);
    }
    m_sizeListBox->Append("48");
    m_sizeListBox->Append("72");
}

/// Transfer data from/to window
bool wxRichTextFontPage::TransferDataFromWindow()
{
    wxPanel::TransferDataFromWindow();

    wxRichTextAttr* attr = GetAttributes();

    if (!m_faceTextCtrl->GetValue().empty())
    {
        std::string faceName = m_faceTextCtrl->GetValue();
        attr->SetFontFaceName(faceName);
    }
    else
        attr->SetFlags(attr->GetFlags() & (~ wxTEXT_ATTR_FONT_FACE));

    std::string strSize = m_sizeTextCtrl->GetValue();
    if (!strSize.empty())
    {
        int sz = wxAtoi(strSize);
        if (sz > 0)
        {
            if (m_sizeUnitsCtrl->GetSelection() == 0)
                attr->SetFontPointSize(sz);
            else
                attr->SetFontPixelSize(sz);
        }
    }
    else
        attr->SetFlags(attr->GetFlags() & (~ wxTEXT_ATTR_FONT_SIZE));

    if (m_styleCtrl->GetSelection() != wxNOT_FOUND && m_styleCtrl->GetSelection() != 0)
    {
        wxFontStyle style;
        if (m_styleCtrl->GetSelection() == 2)
            style = wxFontStyle::Italic;
        else
            style = wxFontStyle::Normal;

        attr->SetFontStyle(style);
    }
    else
        attr->SetFlags(attr->GetFlags() & (~ wxTEXT_ATTR_FONT_ITALIC));

    if (m_weightCtrl->GetSelection() != wxNOT_FOUND && m_weightCtrl->GetSelection() != 0)
    {
        wxFontWeight weight;
        if (m_weightCtrl->GetSelection() == 2)
            weight = wxFONTWEIGHT_BOLD;
        else
            weight = wxFONTWEIGHT_NORMAL;

        attr->SetFontWeight(weight);
    }
    else
        attr->SetFlags(attr->GetFlags() & (~ wxTEXT_ATTR_FONT_WEIGHT));

    if (m_underliningCtrl->GetSelection() != wxNOT_FOUND && m_underliningCtrl->GetSelection() != 0)
    {
        bool underlined;
        underlined = m_underliningCtrl->GetSelection() == 2;

        attr->SetFontUnderlined(underlined);
    }
    else
        attr->SetFlags(attr->GetFlags() & (~ wxTEXT_ATTR_FONT_UNDERLINE));

    if (m_textColourLabel->GetValue())
    {
        attr->SetTextColour(m_colourCtrl->GetColour());
    }
    else
        attr->SetFlags(attr->GetFlags() & (~ wxTEXT_ATTR_TEXT_COLOUR));

    if (m_bgColourLabel->GetValue())
    {
        attr->SetBackgroundColour(m_bgColourCtrl->GetColour());
    }
    else
        attr->SetFlags(attr->GetFlags() & (~ wxTEXT_ATTR_BACKGROUND_COLOUR));

    if (m_strikethroughCtrl->Get3StateValue() != wxCheckBoxState::Indeterminate)
    {
        attr->SetTextEffectFlags(attr->GetTextEffectFlags() | wxTEXT_ATTR_EFFECT_STRIKETHROUGH);

        if (m_strikethroughCtrl->Get3StateValue() == wxCheckBoxState::Checked)
            attr->SetTextEffects(attr->GetTextEffects() | wxTEXT_ATTR_EFFECT_STRIKETHROUGH);
        else
            attr->SetTextEffects(attr->GetTextEffects() & ~wxTEXT_ATTR_EFFECT_STRIKETHROUGH);
    }
    else
    {
        attr->SetTextEffectFlags(attr->GetTextEffectFlags() & ~wxTEXT_ATTR_EFFECT_STRIKETHROUGH);
        attr->SetTextEffects(attr->GetTextEffects() & ~wxTEXT_ATTR_EFFECT_STRIKETHROUGH);
    }

    if (m_capitalsCtrl->Get3StateValue() != wxCheckBoxState::Indeterminate)
    {
        attr->SetTextEffectFlags(attr->GetTextEffectFlags() | wxTEXT_ATTR_EFFECT_CAPITALS);

        if (m_capitalsCtrl->Get3StateValue() == wxCheckBoxState::Checked)
            attr->SetTextEffects(attr->GetTextEffects() | wxTEXT_ATTR_EFFECT_CAPITALS);
        else
            attr->SetTextEffects(attr->GetTextEffects() & ~wxTEXT_ATTR_EFFECT_CAPITALS);
    }
    else
    {
        attr->SetTextEffectFlags(attr->GetTextEffectFlags() & ~wxTEXT_ATTR_EFFECT_CAPITALS);
        attr->SetTextEffects(attr->GetTextEffects() & ~wxTEXT_ATTR_EFFECT_CAPITALS);
    }

    if (m_smallCapitalsCtrl->Get3StateValue() != wxCheckBoxState::Indeterminate)
    {
        attr->SetTextEffectFlags(attr->GetTextEffectFlags() | wxTEXT_ATTR_EFFECT_SMALL_CAPITALS);

        if (m_smallCapitalsCtrl->Get3StateValue() == wxCheckBoxState::Checked)
            attr->SetTextEffects(attr->GetTextEffects() | wxTEXT_ATTR_EFFECT_SMALL_CAPITALS);
        else
            attr->SetTextEffects(attr->GetTextEffects() & ~wxTEXT_ATTR_EFFECT_SMALL_CAPITALS);
    }
    else
    {
        attr->SetTextEffectFlags(attr->GetTextEffectFlags() & ~wxTEXT_ATTR_EFFECT_SMALL_CAPITALS);
        attr->SetTextEffects(attr->GetTextEffects() & ~wxTEXT_ATTR_EFFECT_SMALL_CAPITALS);
    }

    if (m_superscriptCtrl->Get3StateValue() == wxCheckBoxState::Checked)
    {
        attr->SetTextEffectFlags(attr->GetTextEffectFlags() | wxTEXT_ATTR_EFFECT_SUPERSCRIPT);
        attr->SetTextEffects(attr->GetTextEffects() | wxTEXT_ATTR_EFFECT_SUPERSCRIPT);
        attr->SetTextEffects(attr->GetTextEffects() & ~wxTEXT_ATTR_EFFECT_SUBSCRIPT);
    }
    else if (m_subscriptCtrl->Get3StateValue() == wxCheckBoxState::Checked)
    {
        attr->SetTextEffectFlags(attr->GetTextEffectFlags() | wxTEXT_ATTR_EFFECT_SUBSCRIPT);
        attr->SetTextEffects(attr->GetTextEffects() | wxTEXT_ATTR_EFFECT_SUBSCRIPT);
        attr->SetTextEffects(attr->GetTextEffects() & ~wxTEXT_ATTR_EFFECT_SUPERSCRIPT);
    }
    else
    {
        // If they are undetermined, we don't want to include these flags in the text effects - the objects
        // should retain their original style.
        attr->SetTextEffectFlags(attr->GetTextEffectFlags() & ~(wxTEXT_ATTR_EFFECT_SUBSCRIPT|wxTEXT_ATTR_EFFECT_SUPERSCRIPT) );
        attr->SetTextEffects(attr->GetTextEffects() & ~(wxTEXT_ATTR_EFFECT_SUBSCRIPT|wxTEXT_ATTR_EFFECT_SUPERSCRIPT) );
    }

    if (GetAllowedTextEffects() & wxTEXT_ATTR_EFFECT_RTL)
    {
        if (m_rtlCtrl->Get3StateValue() != wxCheckBoxState::Indeterminate)
        {
            attr->SetTextEffectFlags(attr->GetTextEffectFlags() | wxTEXT_ATTR_EFFECT_RTL);

            if (m_rtlCtrl->Get3StateValue() == wxCheckBoxState::Checked)
                attr->SetTextEffects(attr->GetTextEffects() | wxTEXT_ATTR_EFFECT_RTL);
            else
                attr->SetTextEffects(attr->GetTextEffects() & ~wxTEXT_ATTR_EFFECT_RTL);
        }
        else
        {
            attr->SetTextEffectFlags(attr->GetTextEffectFlags() & ~wxTEXT_ATTR_EFFECT_RTL);
            attr->SetTextEffects(attr->GetTextEffects() & ~wxTEXT_ATTR_EFFECT_RTL);
        }
    }

    if (GetAllowedTextEffects() & wxTEXT_ATTR_EFFECT_SUPPRESS_HYPHENATION)
    {
        if (m_suppressHyphenationCtrl->Get3StateValue() != wxCheckBoxState::Indeterminate)
        {
            attr->SetTextEffectFlags(attr->GetTextEffectFlags() | wxTEXT_ATTR_EFFECT_SUPPRESS_HYPHENATION);

            if (m_suppressHyphenationCtrl->Get3StateValue() == wxCheckBoxState::Checked)
                attr->SetTextEffects(attr->GetTextEffects() | wxTEXT_ATTR_EFFECT_SUPPRESS_HYPHENATION);
            else
                attr->SetTextEffects(attr->GetTextEffects() & ~wxTEXT_ATTR_EFFECT_SUPPRESS_HYPHENATION);
        }
        else
        {
            attr->SetTextEffectFlags(attr->GetTextEffectFlags() & ~wxTEXT_ATTR_EFFECT_SUPPRESS_HYPHENATION);
            attr->SetTextEffects(attr->GetTextEffects() & ~wxTEXT_ATTR_EFFECT_SUPPRESS_HYPHENATION);
        }
    }

    if (attr->GetTextEffectFlags() == 0)
        attr->SetFlags(attr->GetFlags() & ~wxTEXT_ATTR_EFFECTS);

    return true;
}

bool wxRichTextFontPage::TransferDataToWindow()
{
    wxPanel::TransferDataToWindow();

    m_dontUpdate = true;
    wxRichTextAttr* attr = GetAttributes();

    if (attr->HasFontFaceName())
    {
        m_faceTextCtrl->SetValue(attr->GetFontFaceName());
        m_faceListBox->SetFaceNameSelection(attr->GetFont().GetFaceName());
    }
    else
    {
        m_faceTextCtrl->SetValue("");
        m_faceListBox->SetFaceNameSelection("");
    }

    if (attr->HasFontPointSize())
    {
        std::string strSize = wxString::Format("%d", attr->GetFontSize()).ToStdString();
        m_sizeTextCtrl->SetValue(strSize);
        m_fontSizeSpinButtons->SetValue(attr->GetFontSize());
        m_sizeUnitsCtrl->SetSelection(0);
        if (m_sizeListBox->FindString(strSize) != wxNOT_FOUND)
            m_sizeListBox->SetStringSelection(strSize);
    }
    else if (attr->HasFontPixelSize())
    {
        std::string strSize = wxString::Format("%d", attr->GetFontSize()).ToStdString();
        m_sizeTextCtrl->SetValue(strSize);
        m_fontSizeSpinButtons->SetValue(attr->GetFontSize());
        m_sizeUnitsCtrl->SetSelection(1);
        m_sizeListBox->SetSelection(wxNOT_FOUND);
    }
    else
    {
        m_sizeTextCtrl->SetValue("");
        m_sizeListBox->SetSelection(wxNOT_FOUND);
    }

    if (attr->HasFontWeight())
    {
        if (attr->GetFontWeight() == wxFONTWEIGHT_BOLD)
            m_weightCtrl->SetSelection(2);
        else
            m_weightCtrl->SetSelection(1);
    }
    else
    {
        m_weightCtrl->SetSelection(0);
    }

    if (attr->HasFontItalic())
    {
        if (attr->GetFontStyle() == wxFontStyle::Italic)
            m_styleCtrl->SetSelection(2);
        else
            m_styleCtrl->SetSelection(1);
    }
    else
    {
        m_styleCtrl->SetSelection(0);
    }

    if (attr->HasFontUnderlined())
    {
        if (attr->GetFontUnderlined())
            m_underliningCtrl->SetSelection(2);
        else
            m_underliningCtrl->SetSelection(1);
    }
    else
    {
        m_underliningCtrl->SetSelection(0);
    }

    if (attr->HasTextColour())
    {
        m_colourCtrl->SetColour(attr->GetTextColour());
        m_textColourLabel->SetValue(true);
        m_colourPresent = true;
    }
    else
    {
        m_colourCtrl->SetColour(*wxBLACK);
        m_textColourLabel->SetValue(false);
    }

    if (attr->HasBackgroundColour())
    {
        m_bgColourCtrl->SetColour(attr->GetBackgroundColour());
        m_bgColourLabel->SetValue(true);
        m_bgColourPresent = true;
    }
    else
    {
        m_bgColourCtrl->SetColour(*wxWHITE);
        m_bgColourLabel->SetValue(false);
    }

    if (attr->HasTextEffects())
    {
        if (attr->GetTextEffectFlags() & wxTEXT_ATTR_EFFECT_STRIKETHROUGH)
        {
            if (attr->GetTextEffects() & wxTEXT_ATTR_EFFECT_STRIKETHROUGH)
                m_strikethroughCtrl->Set3StateValue(wxCheckBoxState::Checked);
            else
                m_strikethroughCtrl->Set3StateValue(wxCheckBoxState::Unchecked);
        }
        else
            m_strikethroughCtrl->Set3StateValue(wxCheckBoxState::Indeterminate);

        if (attr->GetTextEffectFlags() & wxTEXT_ATTR_EFFECT_CAPITALS)
        {
            if (attr->GetTextEffects() & wxTEXT_ATTR_EFFECT_CAPITALS)
                m_capitalsCtrl->Set3StateValue(wxCheckBoxState::Checked);
            else
                m_capitalsCtrl->Set3StateValue(wxCheckBoxState::Unchecked);
        }
        else
            m_capitalsCtrl->Set3StateValue(wxCheckBoxState::Indeterminate);

        if (attr->GetTextEffectFlags() & wxTEXT_ATTR_EFFECT_SMALL_CAPITALS)
        {
            if (attr->GetTextEffects() & wxTEXT_ATTR_EFFECT_SMALL_CAPITALS)
                m_smallCapitalsCtrl->Set3StateValue(wxCheckBoxState::Checked);
            else
                m_smallCapitalsCtrl->Set3StateValue(wxCheckBoxState::Unchecked);
        }
        else
            m_smallCapitalsCtrl->Set3StateValue(wxCheckBoxState::Indeterminate);

        if ( attr->GetTextEffectFlags() & (wxTEXT_ATTR_EFFECT_SUPERSCRIPT | wxTEXT_ATTR_EFFECT_SUBSCRIPT) )
        {
            if (attr->GetTextEffects() & wxTEXT_ATTR_EFFECT_SUPERSCRIPT)
            {
                m_superscriptCtrl->Set3StateValue(wxCheckBoxState::Checked);
                m_subscriptCtrl->Set3StateValue(wxCheckBoxState::Unchecked);
            }
            else if (attr->GetTextEffects() & wxTEXT_ATTR_EFFECT_SUBSCRIPT)
            {
                m_superscriptCtrl->Set3StateValue(wxCheckBoxState::Unchecked);
                m_subscriptCtrl->Set3StateValue(wxCheckBoxState::Checked);
            }
            else
            {
                m_superscriptCtrl->Set3StateValue(wxCheckBoxState::Unchecked);
                m_subscriptCtrl->Set3StateValue(wxCheckBoxState::Unchecked);
            }
        }
        else
        {
            m_superscriptCtrl->Set3StateValue(wxCheckBoxState::Indeterminate);
            m_subscriptCtrl->Set3StateValue(wxCheckBoxState::Indeterminate);
        }

        if (GetAllowedTextEffects() & wxTEXT_ATTR_EFFECT_RTL)
        {
            if (attr->GetTextEffectFlags() & wxTEXT_ATTR_EFFECT_RTL)
            {
                if (attr->GetTextEffects() & wxTEXT_ATTR_EFFECT_RTL)
                    m_rtlCtrl->Set3StateValue(wxCheckBoxState::Checked);
                else
                    m_rtlCtrl->Set3StateValue(wxCheckBoxState::Unchecked);
            }
            else
                m_rtlCtrl->Set3StateValue(wxCheckBoxState::Indeterminate);        
        }

        if (GetAllowedTextEffects() & wxTEXT_ATTR_EFFECT_SUPPRESS_HYPHENATION)
        {
            if (attr->GetTextEffectFlags() & wxTEXT_ATTR_EFFECT_SUPPRESS_HYPHENATION)
            {
                if (attr->GetTextEffects() & wxTEXT_ATTR_EFFECT_SUPPRESS_HYPHENATION)
                    m_suppressHyphenationCtrl->Set3StateValue(wxCheckBoxState::Checked);
                else
                    m_suppressHyphenationCtrl->Set3StateValue(wxCheckBoxState::Unchecked);
            }
            else
                m_suppressHyphenationCtrl->Set3StateValue(wxCheckBoxState::Indeterminate);        
        }
    }
    else
    {
        m_strikethroughCtrl->Set3StateValue(wxCheckBoxState::Indeterminate);
        m_capitalsCtrl->Set3StateValue(wxCheckBoxState::Indeterminate);
        m_smallCapitalsCtrl->Set3StateValue(wxCheckBoxState::Indeterminate);
        m_superscriptCtrl->Set3StateValue(wxCheckBoxState::Indeterminate);
        m_subscriptCtrl->Set3StateValue(wxCheckBoxState::Indeterminate);
        m_rtlCtrl->Set3StateValue(wxCheckBoxState::Indeterminate);
        m_suppressHyphenationCtrl->Set3StateValue(wxCheckBoxState::Indeterminate);
    }

    UpdatePreview();

    m_dontUpdate = false;

    return true;
}

wxRichTextAttr* wxRichTextFontPage::GetAttributes()
{
    return wxRichTextFormattingDialog::GetDialogAttributes(this);
}

/// Updates the font preview
void wxRichTextFontPage::UpdatePreview()
{
    wxRichTextAttr attr;

    if (m_textColourLabel->GetValue())
        m_previewCtrl->SetForegroundColour(m_colourCtrl->GetColour());
    else
    {
        m_previewCtrl->SetForegroundColour(*wxBLACK);
        if (!(m_colourCtrl->GetColour() == *wxBLACK))
        {
            m_colourCtrl->SetColour(*wxBLACK);
            m_colourCtrl->Refresh();
        }
    }

    if (m_bgColourLabel->GetValue())
        m_previewCtrl->SetBackgroundColour(m_bgColourCtrl->GetColour());
    else
    {
        m_previewCtrl->SetBackgroundColour(*wxWHITE);

        if (!(m_bgColourCtrl->GetColour() == *wxWHITE))
        {
            m_bgColourCtrl->SetColour(*wxWHITE);
            m_bgColourCtrl->Refresh();
        }
    }

    if (m_faceListBox->GetSelection() != wxNOT_FOUND)
    {
        std::string faceName = m_faceListBox->GetFaceName(m_faceListBox->GetSelection());
        attr.SetFontFaceName(faceName);
    }

    std::string strSize = m_sizeTextCtrl->GetValue();
    if (!strSize.empty())
    {
        int sz = wxAtoi(strSize);
        if (sz > 0)
        {
            if (m_sizeUnitsCtrl->GetSelection() == 1)
                attr.SetFontPixelSize(sz);
            else
                attr.SetFontPointSize(sz);
        }
    }

    if (m_styleCtrl->GetSelection() != wxNOT_FOUND && m_styleCtrl->GetSelection() != 0)
    {
        wxFontStyle style;
        if (m_styleCtrl->GetSelection() == 2)
            style = wxFontStyle::Italic;
        else
            style = wxFontStyle::Normal;

        attr.SetFontStyle(style);
    }

    if (m_weightCtrl->GetSelection() != wxNOT_FOUND && m_weightCtrl->GetSelection() != 0)
    {
        wxFontWeight weight;
        if (m_weightCtrl->GetSelection() == 2)
            weight = wxFONTWEIGHT_BOLD;
        else
            weight = wxFONTWEIGHT_NORMAL;

        attr.SetFontWeight(weight);
    }

    if (m_underliningCtrl->GetSelection() != wxNOT_FOUND && m_underliningCtrl->GetSelection() != 0)
    {
        bool underlined;
        underlined = m_underliningCtrl->GetSelection() == 2;

        attr.SetFontUnderlined(underlined);
    }

    int textEffects = 0;

    if (m_strikethroughCtrl->Get3StateValue() == wxCheckBoxState::Checked)
    {
        textEffects |= wxTEXT_ATTR_EFFECT_STRIKETHROUGH;
    }

    if (m_capitalsCtrl->Get3StateValue() == wxCheckBoxState::Checked)
    {
        textEffects |= wxTEXT_ATTR_EFFECT_CAPITALS;
    }

    if (m_smallCapitalsCtrl->Get3StateValue() == wxCheckBoxState::Checked)
    {
        textEffects |= wxTEXT_ATTR_EFFECT_SMALL_CAPITALS;
    }

    if ( m_superscriptCtrl->Get3StateValue() == wxCheckBoxState::Checked )
        textEffects |= wxTEXT_ATTR_EFFECT_SUPERSCRIPT;
    else if ( m_subscriptCtrl->Get3StateValue() == wxCheckBoxState::Checked )
        textEffects |= wxTEXT_ATTR_EFFECT_SUBSCRIPT;

    wxFont font = attr.GetFont();
    m_previewCtrl->SetFont(font);
    m_previewCtrl->SetTextEffects(textEffects);
    m_previewCtrl->Refresh();
}

/*!
 * Should we show tooltips?
 */

bool wxRichTextFontPage::ShowToolTips()
{
    return wxRichTextFormattingDialog::ShowToolTips();
}

/*!
 * Get bitmap resources
 */

wxBitmap wxRichTextFontPage::GetBitmapResource( const std::string& name )
{
    // Bitmap retrieval
////@begin wxRichTextFontPage bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end wxRichTextFontPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon wxRichTextFontPage::GetIconResource( const std::string& name )
{
    // Icon retrieval
////@begin wxRichTextFontPage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end wxRichTextFontPage icon retrieval
}

/*!
 * wxEVT_TEXT event handler for ID_RICHTEXTFONTPAGE_FACETEXTCTRL
 */

void wxRichTextFontPage::OnFaceTextCtrlUpdated( [[maybe_unused]] wxCommandEvent& event )
{
    if (m_dontUpdate)
        return;

    std::string facename = m_faceTextCtrl->GetValue();
    if (!facename.empty())
    {
        if (m_faceListBox->HasFaceName(facename))
        {
            m_faceListBox->SetFaceNameSelection(facename);
            UpdatePreview();
        }
        else
        {
            // Try to find a partial match
            const std::vector<std::string>& arr = m_faceListBox->GetFaceNames();
            wx::utils::ToLower(facename);

            for (size_t i = 0; i < arr.size(); i++)
            {
                auto subString = arr[i].substr(0, facename.length());
                wx::utils::ToLower(subString);

                if (subString == facename)
                {
                    m_faceListBox->ScrollToRow(i);
                    break;
                }
            }
        }
    }
}


/*!
 * wxEVT_TEXT event handler for ID_RICHTEXTFONTPAGE_SIZETEXTCTRL
 */

void wxRichTextFontPage::OnSizeTextCtrlUpdated( [[maybe_unused]] wxCommandEvent& event )
{
    if (m_dontUpdate)
        return;

    m_dontUpdate = true;

    std::string strSize = m_sizeTextCtrl->GetValue();
    if (!strSize.empty() && m_sizeListBox->FindString(strSize) != wxNOT_FOUND)
        m_sizeListBox->SetStringSelection(strSize);
    if (!strSize.empty())
        m_fontSizeSpinButtons->SetValue(wxAtoi(strSize));

    m_dontUpdate = false;

    UpdatePreview();
}


/*!
 * wxEVT_LISTBOX event handler for ID_RICHTEXTFONTPAGE_SIZELISTBOX
 */

void wxRichTextFontPage::OnSizeListBoxSelected( wxCommandEvent& event )
{
    bool oldDontUpdate = m_dontUpdate;
    m_dontUpdate = true;

    m_sizeTextCtrl->SetValue(event.GetString());
    if (!event.GetString().empty())
        m_fontSizeSpinButtons->SetValue(wxAtoi(event.GetString()));

    m_dontUpdate = oldDontUpdate;

    if (m_dontUpdate)
        return;

    UpdatePreview();
}

/*!
 * wxEVT_LISTBOX event handler for ID_RICHTEXTFONTPAGE_FACELISTBOX
 */

void wxRichTextFontPage::OnFaceListBoxSelected( [[maybe_unused]] wxCommandEvent& event )
{
    bool oldDontUpdate = m_dontUpdate;
    m_dontUpdate = true;

    m_faceTextCtrl->SetValue(m_faceListBox->GetFaceName(m_faceListBox->GetSelection()));

    m_dontUpdate = oldDontUpdate;

    if (m_dontUpdate)
        return;

    UpdatePreview();
}

/*!
 * wxEVT_COMBOBOX event handler for ID_RICHTEXTFONTPAGE_STYLECTRL
 */

void wxRichTextFontPage::OnStyleCtrlSelected( [[maybe_unused]] wxCommandEvent& event )
{
    if (m_dontUpdate)
        return;

    UpdatePreview();
}


/*!
 * wxEVT_COMBOBOX event handler for ID_RICHTEXTFONTPAGE_UNDERLINING_CTRL
 */

void wxRichTextFontPage::OnUnderliningCtrlSelected( [[maybe_unused]] wxCommandEvent& event )
{
    if (m_dontUpdate)
        return;

    UpdatePreview();
}


/*!
 * wxEVT_COMBOBOX event handler for ID_RICHTEXTFONTPAGE_WEIGHTCTRL
 */

void wxRichTextFontPage::OnWeightCtrlSelected( [[maybe_unused]] wxCommandEvent& event )
{
    if (m_dontUpdate)
        return;

    UpdatePreview();
}

void wxRichTextFontPage::OnColourClicked( wxCommandEvent& event )
{
    if (event.GetId() == m_colourCtrl->GetId())
        m_colourPresent = true;
    else if (event.GetId() == m_bgColourCtrl->GetId())
        m_bgColourPresent = true;

    m_dontUpdate = true;

    if (event.GetId() == m_colourCtrl->GetId())
    {
        m_textColourLabel->SetValue(true);
    }
    else if (event.GetId() == m_bgColourCtrl->GetId())
    {
        m_bgColourLabel->SetValue(true);
    }

    m_dontUpdate = false;

    UpdatePreview();
}
/*!
 * wxEVT_CHECKBOX event handler for ID_RICHTEXTFONTPAGE_STRIKETHROUGHCTRL
 */

void wxRichTextFontPage::OnStrikethroughctrlClick( [[maybe_unused]] wxCommandEvent& event )
{
    if (m_dontUpdate)
        return;

    UpdatePreview();
}

/*!
 * wxEVT_CHECKBOX event handler for ID_RICHTEXTFONTPAGE_CAPSCTRL
 */

void wxRichTextFontPage::OnCapsctrlClick( [[maybe_unused]] wxCommandEvent& event )
{
    if (m_dontUpdate)
        return;

    UpdatePreview();
}

/*!
 * wxEVT_CHECKBOX event handler for ID_RICHTEXTFONTPAGE_SUPERSCRIPT
 */

void wxRichTextFontPage::OnRichtextfontpageSuperscriptClick( [[maybe_unused]] wxCommandEvent& event )
{
    if (m_dontUpdate)
        return;

    if ( m_superscriptCtrl->Get3StateValue() == wxCheckBoxState::Checked)
        m_subscriptCtrl->Set3StateValue( wxCheckBoxState::Unchecked );

    UpdatePreview();
}

/*!
 * wxEVT_CHECKBOX event handler for ID_RICHTEXTFONTPAGE_SUBSCRIPT
 */

void wxRichTextFontPage::OnRichtextfontpageSubscriptClick( [[maybe_unused]] wxCommandEvent& event )
{
    if (m_dontUpdate)
        return;

    if ( m_subscriptCtrl->Get3StateValue() == wxCheckBoxState::Checked)
        m_superscriptCtrl->Set3StateValue( wxCheckBoxState::Unchecked );

    UpdatePreview();
}

/*!
 * wxEVT_CHOICE event handler for ID_RICHTEXTFONTPAGE_SIZE_UNITS
 */

void wxRichTextFontPage::OnRichtextfontpageSizeUnitsSelected( [[maybe_unused]] wxCommandEvent& event )
{
    if (m_dontUpdate)
        return;

    UpdatePreview();
}

/*!
 * wxEVT_SPINCTRL event handler for ID_RICHTEXTFONTPAGE_SPINBUTTONS
 */

void wxRichTextFontPage::OnRichtextfontpageSpinbuttonsUp( [[maybe_unused]] wxSpinEvent& event )
{
    if (m_dontUpdate)
        return;

    m_dontUpdate = true;

    std::string text = m_sizeTextCtrl->GetValue();
    int size = 12;
    if (!text.empty())
    {
        size = wxAtoi(text);
        size ++;
    }

    if (size < 1 || size > 999)
        size = 12;

    if (m_fontSizeSpinButtons->GetValue() != size)
        m_fontSizeSpinButtons->SetValue(size);

    std::string newText(wxString::Format("%d", size));

    m_sizeTextCtrl->SetValue(newText);
    if (!newText.empty() && m_sizeListBox->FindString(newText) != wxNOT_FOUND)
        m_sizeListBox->SetStringSelection(newText);
    UpdatePreview();

    m_dontUpdate = false;
}

/*!
 * wxEVT_SCROLL_LINEDOWN event handler for ID_RICHTEXTFONTPAGE_SPINBUTTONS
 */

void wxRichTextFontPage::OnRichtextfontpageSpinbuttonsDown( [[maybe_unused]] wxSpinEvent& event )
{
    if (m_dontUpdate)
        return;

    m_dontUpdate = true;

    std::string text = m_sizeTextCtrl->GetValue();
    int size = 12;
    if (!text.empty())
    {
        size = wxAtoi(text);
        if (size > 1)
            size --;
    }

    if (size < 1 || size > 999)
        size = 12;

    if (m_fontSizeSpinButtons->GetValue() != size)
        m_fontSizeSpinButtons->SetValue(size);

    std::string newText(wxString::Format("%d", size));

    m_sizeTextCtrl->SetValue(newText);
    if (!newText.empty() && m_sizeListBox->FindString(newText) != wxNOT_FOUND)
        m_sizeListBox->SetStringSelection(newText);
    UpdatePreview();

    m_dontUpdate = false;
}

/*!
 * wxEVT_IDLE event handler for ID_RICHTEXTFONTPAGE
 */

void wxRichTextFontPage::OnIdle( [[maybe_unused]] wxIdleEvent& event )
{
    if (!m_sizeUnitsCtrl)
        return;

    if (m_sizeUnitsCtrl->GetSelection() == 1 && m_sizeListBox->IsShown())
    {
        m_fontListBoxParent->Show(m_sizeListBox, false);
        Layout();
    }
    else if (m_sizeUnitsCtrl->GetSelection() == 0 && !m_sizeListBox->IsShown())
    {
        m_fontListBoxParent->Show(m_sizeListBox, true);
        Layout();
    }
    if (!wxRichTextFormattingDialog::GetDialog(this)->HasOption(wxRichTextFormattingDialog::Option_AllowPixelFontSize) &&
        m_sizeUnitsCtrl->IsEnabled())
    {
        m_sizeUnitsCtrl->Disable();
    }
}
