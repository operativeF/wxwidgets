/////////////////////////////////////////////////////////////////////////////
// Name:        wx/richtext/richtextindentspage.h
// Purpose:     Declares the rich text formatting dialog indent page.
// Author:      Julian Smart
// Modified by:
// Created:     10/3/2006 2:28:21 PM
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _RICHTEXTINDENTSPAGE_H_
#define _RICHTEXTINDENTSPAGE_H_

/*!
 * Includes
 */

#include "wx/richtext/richtextdialogpage.h"
#include "wx/radiobox.h"
#include "wx/radiobut.h"

////@begin includes
#include "wx/statline.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxRichTextCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_WXRICHTEXTINDENTSSPACINGPAGE_STYLE wxRESIZE_BORDER|wxTAB_TRAVERSAL
#define SYMBOL_WXRICHTEXTINDENTSSPACINGPAGE_TITLE {}
#define SYMBOL_WXRICHTEXTINDENTSSPACINGPAGE_IDNAME ID_RICHTEXTINDENTSSPACINGPAGE
#define SYMBOL_WXRICHTEXTINDENTSSPACINGPAGE_SIZE wxSize(400, 300)
#define SYMBOL_WXRICHTEXTINDENTSSPACINGPAGE_POSITION wxDefaultPosition
////@end control identifiers

/*!
 * wxRichTextIndentsSpacingPage class declaration
 */

class wxRichTextIndentsSpacingPage: public wxRichTextDialogPage
{
    wxDECLARE_DYNAMIC_CLASS(wxRichTextIndentsSpacingPage);
    wxDECLARE_EVENT_TABLE();
    DECLARE_HELP_PROVISION()

public:
    /// Constructors
    wxRichTextIndentsSpacingPage() = default;
    wxRichTextIndentsSpacingPage( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = SYMBOL_WXRICHTEXTINDENTSSPACINGPAGE_POSITION, const wxSize& size = SYMBOL_WXRICHTEXTINDENTSSPACINGPAGE_SIZE, unsigned int style = SYMBOL_WXRICHTEXTINDENTSSPACINGPAGE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = SYMBOL_WXRICHTEXTINDENTSSPACINGPAGE_POSITION, const wxSize& size = SYMBOL_WXRICHTEXTINDENTSSPACINGPAGE_SIZE, unsigned int style = SYMBOL_WXRICHTEXTINDENTSSPACINGPAGE_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    /// Transfer data from/to window
    bool TransferDataFromWindow() override;
    bool TransferDataToWindow() override;

    /// Updates the paragraph preview
    void UpdatePreview();

    /// Gets the attributes associated with the main formatting dialog
    wxRichTextAttr* GetAttributes();

////@begin wxRichTextIndentsSpacingPage event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_LEFT
    void OnAlignmentLeftSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_RIGHT
    void OnAlignmentRightSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_JUSTIFIED
    void OnAlignmentJustifiedSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_CENTRED
    void OnAlignmentCentredSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_INDETERMINATE
    void OnAlignmentIndeterminateSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_RICHTEXTINDENTSSPACINGPAGE_INDENT_LEFT
    void OnIndentLeftUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_RICHTEXTINDENTSSPACINGPAGE_INDENT_LEFT_FIRST
    void OnIndentLeftFirstUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_RICHTEXTINDENTSSPACINGPAGE_INDENT_RIGHT
    void OnIndentRightUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_RICHTEXTINDENTSSPACINGPAGE_OUTLINELEVEL
    void OnRichtextOutlinelevelSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_RICHTEXTINDENTSSPACINGPAGE_SPACING_BEFORE
    void OnSpacingBeforeUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_RICHTEXTINDENTSSPACINGPAGE_SPACING_AFTER
    void OnSpacingAfterUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_RICHTEXTINDENTSSPACINGPAGE_SPACING_LINE
    void OnSpacingLineSelected( wxCommandEvent& event );

////@end wxRichTextIndentsSpacingPage event handler declarations

////@begin wxRichTextIndentsSpacingPage member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end wxRichTextIndentsSpacingPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin wxRichTextIndentsSpacingPage member variables
    wxRadioButton* m_alignmentLeft{nullptr};
    wxRadioButton* m_alignmentRight{nullptr};
    wxRadioButton* m_alignmentJustified{nullptr};
    wxRadioButton* m_alignmentCentred{nullptr};
    wxRadioButton* m_alignmentIndeterminate{nullptr};
    wxTextCtrl* m_indentLeft{nullptr};
    wxTextCtrl* m_indentLeftFirst{nullptr};
    wxTextCtrl* m_indentRight{nullptr};
    wxComboBox* m_outlineLevelCtrl{nullptr};
    wxTextCtrl* m_spacingBefore{nullptr};
    wxTextCtrl* m_spacingAfter{nullptr};
    wxComboBox* m_spacingLine{nullptr};
    wxCheckBox* m_pageBreakCtrl{nullptr};
    wxRichTextCtrl* m_previewCtrl{nullptr};
    /// Control identifiers
    enum {
        ID_RICHTEXTINDENTSSPACINGPAGE = 10100,
        ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_LEFT = 10102,
        ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_RIGHT = 10110,
        ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_JUSTIFIED = 10111,
        ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_CENTRED = 10112,
        ID_RICHTEXTINDENTSSPACINGPAGE_ALIGNMENT_INDETERMINATE = 10101,
        ID_RICHTEXTINDENTSSPACINGPAGE_INDENT_LEFT = 10103,
        ID_RICHTEXTINDENTSSPACINGPAGE_INDENT_LEFT_FIRST = 10104,
        ID_RICHTEXTINDENTSSPACINGPAGE_INDENT_RIGHT = 10113,
        ID_RICHTEXTINDENTSSPACINGPAGE_OUTLINELEVEL = 10105,
        ID_RICHTEXTINDENTSSPACINGPAGE_SPACING_BEFORE = 10114,
        ID_RICHTEXTINDENTSSPACINGPAGE_SPACING_AFTER = 10116,
        ID_RICHTEXTINDENTSSPACINGPAGE_SPACING_LINE = 10115,
        ID_RICHTEXTINDENTSSPACINGPAGE_PAGEBREAK = 10106,
        ID_RICHTEXTINDENTSSPACINGPAGE_PREVIEW_CTRL = 10109
    };
////@end wxRichTextIndentsSpacingPage member variables

    bool m_dontUpdate{false};
};

#endif
    // _RICHTEXTINDENTSPAGE_H_
