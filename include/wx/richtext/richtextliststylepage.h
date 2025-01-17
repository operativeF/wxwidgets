/////////////////////////////////////////////////////////////////////////////
// Name:        wx/richtext/richtextliststylepage.h
// Purpose:     Declares the rich text formatting dialog list style page.
// Author:      Julian Smart
// Modified by:
// Created:     10/18/2006 11:36:37 AM
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _RICHTEXTLISTSTYLEPAGE_H_
#define _RICHTEXTLISTSTYLEPAGE_H_

/*!
 * Includes
 */

#include "wx/richtext/richtextdialogpage.h"

////@begin includes
#include "wx/spinctrl.h"
#include "wx/notebook.h"
#include "wx/radiobox.h"
#include "wx/statline.h"
////@end includes

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_WXRICHTEXTLISTSTYLEPAGE_STYLE wxRESIZE_BORDER|wxTAB_TRAVERSAL
inline constexpr char SYMBOL_WXRICHTEXTLISTSTYLEPAGE_TITLE[] = "";
#define SYMBOL_WXRICHTEXTLISTSTYLEPAGE_IDNAME ID_RICHTEXTLISTSTYLEPAGE
inline constexpr wxSize SYMBOL_WXRICHTEXTLISTSTYLEPAGE_SIZE = {400, 300};
inline constexpr wxPoint SYMBOL_WXRICHTEXTLISTSTYLEPAGE_POSITION = wxDefaultPosition;
////@end control identifiers

/*!
 * wxRichTextListStylePage class declaration
 */

class wxRichTextListStylePage: public wxRichTextDialogPage
{
    wxDECLARE_DYNAMIC_CLASS(wxRichTextListStylePage);
    wxDECLARE_EVENT_TABLE();
    DECLARE_HELP_PROVISION()

public:
    /// Constructors
    wxRichTextListStylePage() = default;
    wxRichTextListStylePage( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = SYMBOL_WXRICHTEXTLISTSTYLEPAGE_POSITION, const wxSize& size = SYMBOL_WXRICHTEXTLISTSTYLEPAGE_SIZE, unsigned int style = SYMBOL_WXRICHTEXTLISTSTYLEPAGE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = SYMBOL_WXRICHTEXTLISTSTYLEPAGE_POSITION, const wxSize& size = SYMBOL_WXRICHTEXTLISTSTYLEPAGE_SIZE, unsigned int style = SYMBOL_WXRICHTEXTLISTSTYLEPAGE_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    /// Updates the bullets preview
    void UpdatePreview();

    /// Transfer data from/to window
    bool TransferDataFromWindow() override;
    bool TransferDataToWindow() override;

    /// Get attributes for selected level
    wxRichTextAttr* GetAttributesForSelection();

    /// Update for symbol-related controls
    void OnSymbolUpdate( wxUpdateUIEvent& event );

    /// Update for number-related controls
    void OnNumberUpdate( wxUpdateUIEvent& event );

    /// Update for standard bullet-related controls
    void OnStandardBulletUpdate( wxUpdateUIEvent& event );

    /// Just transfer to the window
    void DoTransferDataToWindow();

    /// Transfer from the window and preview
    void TransferAndPreview();

////@begin wxRichTextListStylePage event handler declarations

    /// wxEVT_SPINCTRL event handler for ID_RICHTEXTLISTSTYLEPAGE_LEVEL
    void OnLevelUpdated( wxSpinEvent& event );

    /// wxEVT_SCROLL_LINEUP event handler for ID_RICHTEXTLISTSTYLEPAGE_LEVEL
    void OnLevelUp( wxSpinEvent& event );

    /// wxEVT_SCROLL_LINEDOWN event handler for ID_RICHTEXTLISTSTYLEPAGE_LEVEL
    void OnLevelDown( wxSpinEvent& event );

    /// wxEVT_TEXT event handler for ID_RICHTEXTLISTSTYLEPAGE_LEVEL
    void OnLevelTextUpdated( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTLISTSTYLEPAGE_LEVEL
    void OnLevelUIUpdate( wxUpdateUIEvent& event );

    /// wxEVT_BUTTON event handler for ID_RICHTEXTLISTSTYLEPAGE_CHOOSE_FONT
    void OnChooseFontClick( wxCommandEvent& event );

    /// wxEVT_LISTBOX event handler for ID_RICHTEXTLISTSTYLEPAGE_STYLELISTBOX
    void OnStylelistboxSelected( wxCommandEvent& event );

    /// wxEVT_CHECKBOX event handler for ID_RICHTEXTLISTSTYLEPAGE_PERIODCTRL
    void OnPeriodctrlClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTLISTSTYLEPAGE_PERIODCTRL
    void OnPeriodctrlUpdate( wxUpdateUIEvent& event );

    /// wxEVT_CHECKBOX event handler for ID_RICHTEXTLISTSTYLEPAGE_PARENTHESESCTRL
    void OnParenthesesctrlClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTLISTSTYLEPAGE_PARENTHESESCTRL
    void OnParenthesesctrlUpdate( wxUpdateUIEvent& event );

    /// wxEVT_CHECKBOX event handler for ID_RICHTEXTLISTSTYLEPAGE_RIGHTPARENTHESISCTRL
    void OnRightParenthesisCtrlClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTLISTSTYLEPAGE_RIGHTPARENTHESISCTRL
    void OnRightParenthesisCtrlUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMBOBOX event handler for ID_RICHTEXTLISTSTYLEPAGE_BULLETALIGNMENTCTRL
    void OnBulletAlignmentCtrlSelected( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTLISTSTYLEPAGE_SYMBOLSTATIC
    void OnSymbolstaticUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMBOBOX event handler for ID_RICHTEXTLISTSTYLEPAGE_SYMBOLCTRL
    void OnSymbolctrlSelected( wxCommandEvent& event );

    /// wxEVT_TEXT event handler for ID_RICHTEXTLISTSTYLEPAGE_SYMBOLCTRL
    void OnSymbolctrlUpdated( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTLISTSTYLEPAGE_SYMBOLCTRL
    void OnSymbolctrlUIUpdate( wxUpdateUIEvent& event );

    /// wxEVT_BUTTON event handler for ID_RICHTEXTLISTSTYLEPAGE_CHOOSE_SYMBOL
    void OnChooseSymbolClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTLISTSTYLEPAGE_CHOOSE_SYMBOL
    void OnChooseSymbolUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMBOBOX event handler for ID_RICHTEXTLISTSTYLEPAGE_SYMBOLFONTCTRL
    void OnSymbolfontctrlSelected( wxCommandEvent& event );

    /// wxEVT_TEXT event handler for ID_RICHTEXTLISTSTYLEPAGE_SYMBOLFONTCTRL
    void OnSymbolfontctrlUpdated( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTLISTSTYLEPAGE_SYMBOLFONTCTRL
    void OnSymbolfontctrlUIUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTLISTSTYLEPAGE_NAMESTATIC
    void OnNamestaticUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMBOBOX event handler for ID_RICHTEXTLISTSTYLEPAGE_NAMECTRL
    void OnNamectrlSelected( wxCommandEvent& event );

    /// wxEVT_TEXT event handler for ID_RICHTEXTLISTSTYLEPAGE_NAMECTRL
    void OnNamectrlUpdated( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTLISTSTYLEPAGE_NAMECTRL
    void OnNamectrlUIUpdate( wxUpdateUIEvent& event );

    /// wxEVT_RADIOBUTTON event handler for ID_RICHTEXTLISTSTYLEPAGE_ALIGNLEFT
    void OnRichtextliststylepageAlignleftSelected( wxCommandEvent& event );

    /// wxEVT_RADIOBUTTON event handler for ID_RICHTEXTLISTSTYLEPAGE_ALIGNRIGHT
    void OnRichtextliststylepageAlignrightSelected( wxCommandEvent& event );

    /// wxEVT_RADIOBUTTON event handler for ID_RICHTEXTLISTSTYLEPAGE_JUSTIFIED
    void OnRichtextliststylepageJustifiedSelected( wxCommandEvent& event );

    /// wxEVT_RADIOBUTTON event handler for ID_RICHTEXTLISTSTYLEPAGE_CENTERED
    void OnRichtextliststylepageCenteredSelected( wxCommandEvent& event );

    /// wxEVT_RADIOBUTTON event handler for ID_RICHTEXTLISTSTYLEPAGE_ALIGNINDETERMINATE
    void OnRichtextliststylepageAlignindeterminateSelected( wxCommandEvent& event );

    /// wxEVT_TEXT event handler for ID_RICHTEXTLISTSTYLEPAGE_INDENTLEFT
    void OnIndentLeftUpdated( wxCommandEvent& event );

    /// wxEVT_TEXT event handler for ID_RICHTEXTLISTSTYLEPAGE_INDENTFIRSTLINE
    void OnIndentFirstLineUpdated( wxCommandEvent& event );

    /// wxEVT_TEXT event handler for ID_RICHTEXTLISTSTYLEPAGE_INDENTRIGHT
    void OnIndentRightUpdated( wxCommandEvent& event );

    /// wxEVT_TEXT event handler for ID_RICHTEXTLISTSTYLEPAGE_SPACINGBEFORE
    void OnSpacingBeforeUpdated( wxCommandEvent& event );

    /// wxEVT_TEXT event handler for ID_RICHTEXTLISTSTYLEPAGE_SPACINGAFTER
    void OnSpacingAfterUpdated( wxCommandEvent& event );

    /// wxEVT_COMBOBOX event handler for ID_RICHTEXTLISTSTYLEPAGE_LINESPACING
    void OnLineSpacingSelected( wxCommandEvent& event );

////@end wxRichTextListStylePage event handler declarations

////@begin wxRichTextListStylePage member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end wxRichTextListStylePage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin wxRichTextListStylePage member variables
    wxSpinCtrl* m_levelCtrl{nullptr};
    wxListBox* m_styleListBox{nullptr};
    wxCheckBox* m_periodCtrl{nullptr};
    wxCheckBox* m_parenthesesCtrl{nullptr};
    wxCheckBox* m_rightParenthesisCtrl{nullptr};
    wxComboBox* m_bulletAlignmentCtrl{nullptr};
    wxComboBox* m_symbolCtrl{nullptr};
    wxComboBox* m_symbolFontCtrl{nullptr};
    wxComboBox* m_bulletNameCtrl{nullptr};
    wxRadioButton* m_alignmentLeft{nullptr};
    wxRadioButton* m_alignmentRight{nullptr};
    wxRadioButton* m_alignmentJustified{nullptr};
    wxRadioButton* m_alignmentCentred{nullptr};
    wxRadioButton* m_alignmentIndeterminate{nullptr};
    wxTextCtrl* m_indentLeft{nullptr};
    wxTextCtrl* m_indentLeftFirst{nullptr};
    wxTextCtrl* m_indentRight{nullptr};
    wxTextCtrl* m_spacingBefore{nullptr};
    wxTextCtrl* m_spacingAfter{nullptr};
    wxComboBox* m_spacingLine{nullptr};
    wxRichTextCtrl* m_previewCtrl{nullptr};
    /// Control identifiers
    enum {
        ID_RICHTEXTLISTSTYLEPAGE = 10616,
        ID_RICHTEXTLISTSTYLEPAGE_LEVEL = 10617,
        ID_RICHTEXTLISTSTYLEPAGE_CHOOSE_FONT = 10604,
        ID_RICHTEXTLISTSTYLEPAGE_NOTEBOOK = 10618,
        ID_RICHTEXTLISTSTYLEPAGE_BULLETS = 10619,
        ID_RICHTEXTLISTSTYLEPAGE_STYLELISTBOX = 10620,
        ID_RICHTEXTLISTSTYLEPAGE_PERIODCTRL = 10627,
        ID_RICHTEXTLISTSTYLEPAGE_PARENTHESESCTRL = 10626,
        ID_RICHTEXTLISTSTYLEPAGE_RIGHTPARENTHESISCTRL = 10602,
        ID_RICHTEXTLISTSTYLEPAGE_BULLETALIGNMENTCTRL = 10603,
        ID_RICHTEXTLISTSTYLEPAGE_SYMBOLSTATIC = 10621,
        ID_RICHTEXTLISTSTYLEPAGE_SYMBOLCTRL = 10622,
        ID_RICHTEXTLISTSTYLEPAGE_CHOOSE_SYMBOL = 10623,
        ID_RICHTEXTLISTSTYLEPAGE_SYMBOLFONTCTRL = 10625,
        ID_RICHTEXTLISTSTYLEPAGE_NAMESTATIC = 10600,
        ID_RICHTEXTLISTSTYLEPAGE_NAMECTRL = 10601,
        ID_RICHTEXTLISTSTYLEPAGE_SPACING = 10628,
        ID_RICHTEXTLISTSTYLEPAGE_ALIGNLEFT = 10629,
        ID_RICHTEXTLISTSTYLEPAGE_ALIGNRIGHT = 10630,
        ID_RICHTEXTLISTSTYLEPAGE_JUSTIFIED = 10631,
        ID_RICHTEXTLISTSTYLEPAGE_CENTERED = 10632,
        ID_RICHTEXTLISTSTYLEPAGE_ALIGNINDETERMINATE = 10633,
        ID_RICHTEXTLISTSTYLEPAGE_INDENTLEFT = 10634,
        ID_RICHTEXTLISTSTYLEPAGE_INDENTFIRSTLINE = 10635,
        ID_RICHTEXTLISTSTYLEPAGE_INDENTRIGHT = 10636,
        ID_RICHTEXTLISTSTYLEPAGE_SPACINGBEFORE = 10637,
        ID_RICHTEXTLISTSTYLEPAGE_SPACINGAFTER = 10638,
        ID_RICHTEXTLISTSTYLEPAGE_LINESPACING = 10639,
        ID_RICHTEXTLISTSTYLEPAGE_RICHTEXTCTRL = 10640
    };
////@end wxRichTextListStylePage member variables

    int m_currentLevel{1};
    bool m_dontUpdate{false};
};

#endif
    // _RICHTEXTLISTSTYLEPAGE_H_
