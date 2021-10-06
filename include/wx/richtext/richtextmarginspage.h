/////////////////////////////////////////////////////////////////////////////
// Name:        wx/richtext/richtextmarginspage.h
// Purpose:     Declares the rich text formatting dialog margins page.
// Author:      Julian Smart
// Modified by:
// Created:     20/10/2010 10:27:34
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _RICHTEXTMARGINSPAGE_H_
#define _RICHTEXTMARGINSPAGE_H_

/*!
 * Includes
 */

#include "wx/richtext/richtextdialogpage.h"

/*!
 * Forward declarations
 */

class WXDLLIMPEXP_FWD_CORE wxCheckBox;
class WXDLLIMPEXP_FWD_CORE wxComboBox;
class WXDLLIMPEXP_FWD_CORE wxTextCtrl;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_WXRICHTEXTMARGINSPAGE_STYLE wxTAB_TRAVERSAL
#define SYMBOL_WXRICHTEXTMARGINSPAGE_TITLE wxEmptyString
#define SYMBOL_WXRICHTEXTMARGINSPAGE_IDNAME ID_WXRICHTEXTMARGINSPAGE
#define SYMBOL_WXRICHTEXTMARGINSPAGE_SIZE wxSize(400, 300)
#define SYMBOL_WXRICHTEXTMARGINSPAGE_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * wxRichTextMarginsPage class declaration
 */

class WXDLLIMPEXP_RICHTEXT wxRichTextMarginsPage: public wxRichTextDialogPage
{
    wxDECLARE_DYNAMIC_CLASS(wxRichTextMarginsPage);
    wxDECLARE_EVENT_TABLE();
    DECLARE_HELP_PROVISION()

public:
    /// Constructors
    wxRichTextMarginsPage() = default;
    wxRichTextMarginsPage( wxWindow* parent, wxWindowID id = SYMBOL_WXRICHTEXTMARGINSPAGE_IDNAME, const wxPoint& pos = SYMBOL_WXRICHTEXTMARGINSPAGE_POSITION, const wxSize& size = SYMBOL_WXRICHTEXTMARGINSPAGE_SIZE, unsigned int style = SYMBOL_WXRICHTEXTMARGINSPAGE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_WXRICHTEXTMARGINSPAGE_IDNAME, const wxPoint& pos = SYMBOL_WXRICHTEXTMARGINSPAGE_POSITION, const wxSize& size = SYMBOL_WXRICHTEXTMARGINSPAGE_SIZE, unsigned int style = SYMBOL_WXRICHTEXTMARGINSPAGE_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    /// Gets the attributes from the formatting dialog
    wxRichTextAttr* GetAttributes();

    /// Data transfer
    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;

////@begin wxRichTextMarginsPage event handler declarations

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_LEFT_MARGIN
    void OnRichtextLeftMarginUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_RIGHT_MARGIN
    void OnRichtextRightMarginUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_TOP_MARGIN
    void OnRichtextTopMarginUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_BOTTOM_MARGIN
    void OnRichtextBottomMarginUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_LEFT_PADDING
    void OnRichtextLeftPaddingUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_RIGHT_PADDING
    void OnRichtextRightPaddingUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_TOP_PADDING
    void OnRichtextTopPaddingUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_BOTTOM_PADDING
    void OnRichtextBottomPaddingUpdate( wxUpdateUIEvent& event );

////@end wxRichTextMarginsPage event handler declarations

////@begin wxRichTextMarginsPage member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const std::string& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const std::string& name );
////@end wxRichTextMarginsPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin wxRichTextMarginsPage member variables
    wxCheckBox* m_leftMarginCheckbox{nullptr};
    wxTextCtrl* m_marginLeft{nullptr};
    wxComboBox* m_unitsMarginLeft{nullptr};
    wxCheckBox* m_rightMarginCheckbox{nullptr};
    wxTextCtrl* m_marginRight{nullptr};
    wxComboBox* m_unitsMarginRight{nullptr};
    wxCheckBox* m_topMarginCheckbox{nullptr};
    wxTextCtrl* m_marginTop{nullptr};
    wxComboBox* m_unitsMarginTop{nullptr};
    wxCheckBox* m_bottomMarginCheckbox{nullptr};
    wxTextCtrl* m_marginBottom{nullptr};
    wxComboBox* m_unitsMarginBottom{nullptr};
    wxCheckBox* m_leftPaddingCheckbox{nullptr};
    wxTextCtrl* m_paddingLeft{nullptr};
    wxComboBox* m_unitsPaddingLeft{nullptr};
    wxCheckBox* m_rightPaddingCheckbox{nullptr};
    wxTextCtrl* m_paddingRight{nullptr};
    wxComboBox* m_unitsPaddingRight{nullptr};
    wxCheckBox* m_topPaddingCheckbox{nullptr};
    wxTextCtrl* m_paddingTop{nullptr};
    wxComboBox* m_unitsPaddingTop{nullptr};
    wxCheckBox* m_bottomPaddingCheckbox{nullptr};
    wxTextCtrl* m_paddingBottom{nullptr};
    wxComboBox* m_unitsPaddingBottom{nullptr};
    /// Control identifiers
    enum {
        ID_WXRICHTEXTMARGINSPAGE = 10750,
        ID_RICHTEXT_LEFT_MARGIN_CHECKBOX = 10751,
        ID_RICHTEXT_LEFT_MARGIN = 10752,
        ID_RICHTEXT_LEFT_MARGIN_UNITS = 10753,
        ID_RICHTEXT_RIGHT_MARGIN_CHECKBOX = 10754,
        ID_RICHTEXT_RIGHT_MARGIN = 10755,
        ID_RICHTEXT_RIGHT_MARGIN_UNITS = 10756,
        ID_RICHTEXT_TOP_MARGIN_CHECKBOX = 10757,
        ID_RICHTEXT_TOP_MARGIN = 10758,
        ID_RICHTEXT_TOP_MARGIN_UNITS = 10759,
        ID_RICHTEXT_BOTTOM_MARGIN_CHECKBOX = 10760,
        ID_RICHTEXT_BOTTOM_MARGIN = 10761,
        ID_RICHTEXT_BOTTOM_MARGIN_UNITS = 10762,
        ID_RICHTEXT_LEFT_PADDING_CHECKBOX = 10763,
        ID_RICHTEXT_LEFT_PADDING = 10764,
        ID_RICHTEXT_LEFT_PADDING_UNITS = 10765,
        ID_RICHTEXT_RIGHT_PADDING_CHECKBOX = 10766,
        ID_RICHTEXT_RIGHT_PADDING = 10767,
        ID_RICHTEXT_RIGHT_PADDING_UNITS = 10768,
        ID_RICHTEXT_TOP_PADDING_CHECKBOX = 10769,
        ID_RICHTEXT_TOP_PADDING = 10770,
        ID_RICHTEXT_TOP_PADDING_UNITS = 10771,
        ID_RICHTEXT_BOTTOM_PADDING_CHECKBOX = 10772,
        ID_RICHTEXT_BOTTOM_PADDING = 10773,
        ID_RICHTEXT_BOTTOM_PADDING_UNITS = 10774
    };
////@end wxRichTextMarginsPage member variables

    bool    m_ignoreUpdates{false};
};

#endif
    // _RICHTEXTMARGINSPAGE_H_
