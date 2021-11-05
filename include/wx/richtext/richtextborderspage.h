/////////////////////////////////////////////////////////////////////////////
// Name:        wx/richtext/richtextborderspage.h
// Purpose:     A border editing page for the wxRTC formatting dialog.
// Author:      Julian Smart
// Modified by:
// Created:     21/10/2010 11:34:24
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _RICHTEXTBORDERSPAGE_H_
#define _RICHTEXTBORDERSPAGE_H_

/*!
 * Includes
 */

#include "wx/richtext/richtextdialogpage.h"

////@begin includes
#include "wx/notebook.h"
#include "wx/statline.h"
////@end includes

/*!
 * Forward declarations
 */

class wxRichTextColourSwatchCtrl;
class wxRichTextBorderPreviewCtrl;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_WXRICHTEXTBORDERSPAGE_STYLE wxTAB_TRAVERSAL
#define SYMBOL_WXRICHTEXTBORDERSPAGE_TITLE {}
#define SYMBOL_WXRICHTEXTBORDERSPAGE_IDNAME ID_RICHTEXTBORDERSPAGE
#define SYMBOL_WXRICHTEXTBORDERSPAGE_SIZE wxSize(400, 300)
#define SYMBOL_WXRICHTEXTBORDERSPAGE_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * wxRichTextBordersPage class declaration
 */

class WXDLLIMPEXP_RICHTEXT wxRichTextBordersPage: public wxRichTextDialogPage
{
    wxDECLARE_DYNAMIC_CLASS(wxRichTextBordersPage);
    wxDECLARE_EVENT_TABLE();
    DECLARE_HELP_PROVISION()

public:
    /// Constructors
    wxRichTextBordersPage() = default;
    wxRichTextBordersPage( wxWindow* parent, wxWindowID id = SYMBOL_WXRICHTEXTBORDERSPAGE_IDNAME, const wxPoint& pos = SYMBOL_WXRICHTEXTBORDERSPAGE_POSITION, const wxSize& size = SYMBOL_WXRICHTEXTBORDERSPAGE_SIZE, unsigned int style = SYMBOL_WXRICHTEXTBORDERSPAGE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_WXRICHTEXTBORDERSPAGE_IDNAME, const wxPoint& pos = SYMBOL_WXRICHTEXTBORDERSPAGE_POSITION, const wxSize& size = SYMBOL_WXRICHTEXTBORDERSPAGE_SIZE, unsigned int style = SYMBOL_WXRICHTEXTBORDERSPAGE_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    /// Gets the attributes from the formatting dialog
    wxRichTextAttr* GetAttributes();

    /// Data transfer
    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;

    /// Updates the synchronization checkboxes to reflect the state of the attributes
    void UpdateSyncControls();

    /// Updates the preview
    void OnCommand(wxCommandEvent& event);

    /// Fill style combo
    virtual void FillStyleComboBox(wxComboBox* styleComboBox);

    /// Set the border controls
    static void SetBorderValue(wxTextAttrBorder& border, wxTextCtrl* widthValueCtrl, wxComboBox* widthUnitsCtrl, wxCheckBox* checkBox,
        wxComboBox* styleCtrl, wxRichTextColourSwatchCtrl* colourCtrl, const std::vector<int>& borderStyles);

    /// Get data from the border controls
    static void GetBorderValue(wxTextAttrBorder& border, wxTextCtrl* widthValueCtrl, wxComboBox* widthUnitsCtrl, wxCheckBox* checkBox,
        wxComboBox* styleCtrl, wxRichTextColourSwatchCtrl* colourCtrl, const std::vector<int>& borderStyles);

////@begin wxRichTextBordersPage event handler declarations

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_RICHTEXT_BORDER_LEFT_CHECKBOX
    void OnRichtextBorderCheckboxClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_RICHTEXT_BORDER_LEFT
    void OnRichtextBorderLeftValueTextUpdated( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_BORDER_LEFT
    void OnRichtextBorderLeftUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_RICHTEXT_BORDER_LEFT_UNITS
    void OnRichtextBorderLeftUnitsSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_RICHTEXT_BORDER_LEFT_STYLE
    void OnRichtextBorderLeftStyleSelected( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_BORDER_RIGHT_CHECKBOX
    void OnRichtextBorderOtherCheckboxUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_BORDER_RIGHT
    void OnRichtextBorderRightUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_BORDER_TOP
    void OnRichtextBorderTopUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_BORDER_BOTTOM
    void OnRichtextBorderBottomUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_RICHTEXT_BORDER_SYNCHRONIZE
    void OnRichtextBorderSynchronizeClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_BORDER_SYNCHRONIZE
    void OnRichtextBorderSynchronizeUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_RICHTEXT_OUTLINE_LEFT
    void OnRichtextOutlineLeftTextUpdated( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_OUTLINE_LEFT
    void OnRichtextOutlineLeftUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_RICHTEXT_OUTLINE_LEFT_UNITS
    void OnRichtextOutlineLeftUnitsSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_RICHTEXT_OUTLINE_LEFT_STYLE
    void OnRichtextOutlineLeftStyleSelected( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_OUTLINE_RIGHT_CHECKBOX
    void OnRichtextOutlineOtherCheckboxUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_OUTLINE_RIGHT
    void OnRichtextOutlineRightUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_OUTLINE_TOP
    void OnRichtextOutlineTopUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_OUTLINE_BOTTOM
    void OnRichtextOutlineBottomUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_RICHTEXT_OUTLINE_SYNCHRONIZE
    void OnRichtextOutlineSynchronizeClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_OUTLINE_SYNCHRONIZE
    void OnRichtextOutlineSynchronizeUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTBORDERSPAGE_CORNER_TEXT
    void OnRichtextborderspageCornerUpdate( wxUpdateUIEvent& event );

////@end wxRichTextBordersPage event handler declarations

////@begin wxRichTextBordersPage member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const std::string& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const std::string& name );
////@end wxRichTextBordersPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin wxRichTextBordersPage member variables

    std::vector<int> m_borderStyles;
    std::vector<std::string> m_borderStyleNames;

    wxCheckBox* m_leftBorderCheckbox{nullptr};
    wxTextCtrl* m_leftBorderWidth{nullptr};
    wxComboBox* m_leftBorderWidthUnits{nullptr};
    wxComboBox* m_leftBorderStyle{nullptr};
    wxRichTextColourSwatchCtrl* m_leftBorderColour{nullptr};
    wxCheckBox* m_rightBorderCheckbox{nullptr};
    wxTextCtrl* m_rightBorderWidth{nullptr};
    wxComboBox* m_rightBorderWidthUnits{nullptr};
    wxComboBox* m_rightBorderStyle{nullptr};
    wxRichTextColourSwatchCtrl* m_rightBorderColour{nullptr};
    wxCheckBox* m_topBorderCheckbox{nullptr};
    wxTextCtrl* m_topBorderWidth{nullptr};
    wxComboBox* m_topBorderWidthUnits{nullptr};
    wxComboBox* m_topBorderStyle{nullptr};
    wxRichTextColourSwatchCtrl* m_topBorderColour{nullptr};
    wxCheckBox* m_bottomBorderCheckbox{nullptr};
    wxTextCtrl* m_bottomBorderWidth{nullptr};
    wxComboBox* m_bottomBorderWidthUnits{nullptr};
    wxComboBox* m_bottomBorderStyle{nullptr};
    wxRichTextColourSwatchCtrl* m_bottomBorderColour{nullptr};
    wxCheckBox* m_borderSyncCtrl{nullptr};
    wxCheckBox* m_leftOutlineCheckbox{nullptr};
    wxTextCtrl* m_leftOutlineWidth{nullptr};
    wxComboBox* m_leftOutlineWidthUnits{nullptr};
    wxComboBox* m_leftOutlineStyle{nullptr};
    wxRichTextColourSwatchCtrl* m_leftOutlineColour{nullptr};
    wxCheckBox* m_rightOutlineCheckbox{nullptr};
    wxTextCtrl* m_rightOutlineWidth{nullptr};
    wxComboBox* m_rightOutlineWidthUnits{nullptr};
    wxComboBox* m_rightOutlineStyle{nullptr};
    wxRichTextColourSwatchCtrl* m_rightOutlineColour{nullptr};
    wxCheckBox* m_topOutlineCheckbox{nullptr};
    wxTextCtrl* m_topOutlineWidth{nullptr};
    wxComboBox* m_topOutlineWidthUnits{nullptr};
    wxComboBox* m_topOutlineStyle{nullptr};
    wxRichTextColourSwatchCtrl* m_topOutlineColour{nullptr};
    wxCheckBox* m_bottomOutlineCheckbox{nullptr};
    wxTextCtrl* m_bottomOutlineWidth{nullptr};
    wxComboBox* m_bottomOutlineWidthUnits{nullptr};
    wxComboBox* m_bottomOutlineStyle{nullptr};
    wxRichTextColourSwatchCtrl* m_bottomOutlineColour{nullptr};
    wxCheckBox* m_outlineSyncCtrl{nullptr};
    wxCheckBox* m_cornerRadiusCheckBox{nullptr};
    wxTextCtrl* m_cornerRadiusText{nullptr};
    wxComboBox* m_cornerRadiusUnits{nullptr};
    wxRichTextBorderPreviewCtrl* m_borderPreviewCtrl{nullptr};
    /// Control identifiers
    enum {
        ID_RICHTEXTBORDERSPAGE = 10800,
        ID_RICHTEXTBORDERSPAGE_NOTEBOOK = 10801,
        ID_RICHTEXTBORDERSPAGE_BORDERS = 10802,
        ID_RICHTEXT_BORDER_LEFT_CHECKBOX = 10803,
        ID_RICHTEXT_BORDER_LEFT = 10804,
        ID_RICHTEXT_BORDER_LEFT_UNITS = 10805,
        ID_RICHTEXT_BORDER_LEFT_STYLE = 10806,
        ID_RICHTEXT_BORDER_LEFT_COLOUR = 10807,
        ID_RICHTEXT_BORDER_RIGHT_CHECKBOX = 10808,
        ID_RICHTEXT_BORDER_RIGHT = 10809,
        ID_RICHTEXT_BORDER_RIGHT_UNITS = 10810,
        ID_RICHTEXT_BORDER_RIGHT_STYLE = 10811,
        ID_RICHTEXT_BORDER_RIGHT_COLOUR = 10812,
        ID_RICHTEXT_BORDER_TOP_CHECKBOX = 10813,
        ID_RICHTEXT_BORDER_TOP = 10814,
        ID_RICHTEXT_BORDER_TOP_UNITS = 10815,
        ID_RICHTEXT_BORDER_TOP_STYLE = 10816,
        ID_RICHTEXT_BORDER_TOP_COLOUR = 10817,
        ID_RICHTEXT_BORDER_BOTTOM_CHECKBOX = 10818,
        ID_RICHTEXT_BORDER_BOTTOM = 10819,
        ID_RICHTEXT_BORDER_BOTTOM_UNITS = 10820,
        ID_RICHTEXT_BORDER_BOTTOM_STYLE = 10821,
        ID_RICHTEXT_BORDER_BOTTOM_COLOUR = 10822,
        ID_RICHTEXT_BORDER_SYNCHRONIZE = 10845,
        ID_RICHTEXTBORDERSPAGE_OUTLINE = 10823,
        ID_RICHTEXT_OUTLINE_LEFT_CHECKBOX = 10824,
        ID_RICHTEXT_OUTLINE_LEFT = 10825,
        ID_RICHTEXT_OUTLINE_LEFT_UNITS = 10826,
        ID_RICHTEXT_OUTLINE_LEFT_STYLE = 10827,
        ID_RICHTEXT_OUTLINE_LEFT_COLOUR = 10828,
        ID_RICHTEXT_OUTLINE_RIGHT_CHECKBOX = 10829,
        ID_RICHTEXT_OUTLINE_RIGHT = 10830,
        ID_RICHTEXT_OUTLINE_RIGHT_UNITS = 10831,
        ID_RICHTEXT_OUTLINE_RIGHT_STYLE = 10832,
        ID_RICHTEXT_OUTLINE_RIGHT_COLOUR = 10833,
        ID_RICHTEXT_OUTLINE_TOP_CHECKBOX = 10834,
        ID_RICHTEXT_OUTLINE_TOP = 10835,
        ID_RICHTEXT_OUTLINE_TOP_UNITS = 10836,
        ID_RICHTEXT_OUTLINE_TOP_STYLE = 10837,
        ID_RICHTEXT_OUTLINE_TOP_COLOUR = 10838,
        ID_RICHTEXT_OUTLINE_BOTTOM_CHECKBOX = 10839,
        ID_RICHTEXT_OUTLINE_BOTTOM = 10840,
        ID_RICHTEXT_OUTLINE_BOTTOM_UNITS = 10841,
        ID_RICHTEXT_OUTLINE_BOTTOM_STYLE = 10842,
        ID_RICHTEXT_OUTLINE_BOTTOM_COLOUR = 10843,
        ID_RICHTEXT_OUTLINE_SYNCHRONIZE = 10846,
        ID_RICHTEXTBORDERSPAGE_CORNER = 10847,
        ID_RICHTEXTBORDERSPAGE_CORNER_CHECKBOX = 10848,
        ID_RICHTEXTBORDERSPAGE_CORNER_TEXT = 10849,
        ID_RICHTEXTBORDERSPAGE_CORNER_UNITS = 10850,
        ID_RICHTEXT_BORDER_PREVIEW = 10844
    };

    bool m_ignoreUpdates{false};
    
    ////@end wxRichTextBordersPage member variables
};

class WXDLLIMPEXP_RICHTEXT wxRichTextBorderPreviewCtrl : public wxWindow
{
public:
    wxRichTextBorderPreviewCtrl(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& sz = wxDefaultSize, unsigned int style = 0);

    void SetAttributes(wxRichTextAttr* attr) { m_attributes = attr; }
    wxRichTextAttr* GetAttributes() const { return m_attributes; }

private:
    wxRichTextAttr* m_attributes;

    void OnPaint(wxPaintEvent& event);
    wxDECLARE_EVENT_TABLE();
};

#endif
    // _RICHTEXTBORDERSPAGE_H_
