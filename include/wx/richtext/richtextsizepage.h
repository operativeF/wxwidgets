/////////////////////////////////////////////////////////////////////////////
// Name:        wx/richtext/richtextsizepage.h
// Purpose:     Declares the rich text formatting dialog size page.
// Author:      Julian Smart
// Modified by:
// Created:     20/10/2010 10:23:24
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _RICHTEXTSIZEPAGE_H_
#define _RICHTEXTSIZEPAGE_H_

/*!
 * Includes
 */

#include "wx/richtext/richtextdialogpage.h"

////@begin includes
#include "wx/statline.h"
#include "wx/valgen.h"
////@end includes
#include "wx/stattext.h"

/*!
 * Forward declarations
 */
class WXDLLIMPEXP_FWD_CORE wxBoxSizer;
class WXDLLIMPEXP_FWD_CORE wxCheckBox;
class WXDLLIMPEXP_FWD_CORE wxChoice;
class WXDLLIMPEXP_FWD_CORE wxComboBox;
class WXDLLIMPEXP_FWD_CORE wxFlexGridSizer;
class WXDLLIMPEXP_FWD_CORE wxStaticText;
class WXDLLIMPEXP_FWD_CORE wxTextCtrl;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_WXRICHTEXTSIZEPAGE_STYLE wxTAB_TRAVERSAL
#define SYMBOL_WXRICHTEXTSIZEPAGE_TITLE {}
#define SYMBOL_WXRICHTEXTSIZEPAGE_IDNAME ID_WXRICHTEXTSIZEPAGE
#define SYMBOL_WXRICHTEXTSIZEPAGE_SIZE wxSize(400, 300)
#define SYMBOL_WXRICHTEXTSIZEPAGE_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * wxRichTextSizePage class declaration
 */

class WXDLLIMPEXP_RICHTEXT wxRichTextSizePage: public wxRichTextDialogPage
{
    wxDECLARE_DYNAMIC_CLASS(wxRichTextSizePage);
    wxDECLARE_EVENT_TABLE();
    DECLARE_HELP_PROVISION()

public:
    /// Constructors
    wxRichTextSizePage() = default;
    wxRichTextSizePage( wxWindow* parent, wxWindowID id = SYMBOL_WXRICHTEXTSIZEPAGE_IDNAME, const wxPoint& pos = SYMBOL_WXRICHTEXTSIZEPAGE_POSITION, const wxSize& size = SYMBOL_WXRICHTEXTSIZEPAGE_SIZE, unsigned int style = SYMBOL_WXRICHTEXTSIZEPAGE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_WXRICHTEXTSIZEPAGE_IDNAME, const wxPoint& pos = SYMBOL_WXRICHTEXTSIZEPAGE_POSITION, const wxSize& size = SYMBOL_WXRICHTEXTSIZEPAGE_SIZE, unsigned int style = SYMBOL_WXRICHTEXTSIZEPAGE_STYLE );

    /// Destructor
    ~wxRichTextSizePage();

    /// Creates the controls and sizers
    void CreateControls();

    /// Gets the attributes from the formatting dialog
    wxRichTextAttr* GetAttributes();

    /// Data transfer
    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;

    /// Show/hide position controls
    static void ShowPositionControls(bool show) { sm_showPositionControls = show; }

    /// Show/hide minimum and maximum size controls
    static void ShowMinMaxSizeControls(bool show) { sm_showMinMaxSizeControls = show; }

    /// Show/hide position mode controls
    static void ShowPositionModeControls(bool show) { sm_showPositionModeControls = show; }

    /// Show/hide right/bottom position controls
    static void ShowRightBottomPositionControls(bool show) { sm_showRightBottomPositionControls = show; }

    /// Show/hide floating and alignment controls
    static void ShowFloatingAndAlignmentControls(bool show) { sm_showFloatingAndAlignmentControls = show; }

    /// Show/hide floating controls
    static void ShowFloatingControls(bool show) { sm_showFloatingControls = show; }

    /// Show/hide alignment controls
    static void ShowAlignmentControls(bool show) { sm_showAlignmentControls = show; }

    /// Enable the position and size units
    static void EnablePositionAndSizeUnits(bool enable) { sm_enablePositionAndSizeUnits = enable; }

    /// Enable the checkboxes for position and size
    static void EnablePositionAndSizeCheckboxes(bool enable) { sm_enablePositionAndSizeCheckboxes = enable; }

    /// Enable the move object controls
    static void ShowMoveObjectControls(bool enable) { sm_showMoveObjectControls = enable; }

////@begin wxRichTextSizePage event handler declarations

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_VERTICAL_ALIGNMENT_COMBOBOX
    void OnRichtextVerticalAlignmentComboboxUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_WIDTH
    void OnRichtextWidthUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_UNITS_W
    void OnRichtextWidthUnitsUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_HEIGHT
    void OnRichtextHeightUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_UNITS_H
    void OnRichtextHeightUnitsUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_MIN_WIDTH
    void OnRichtextMinWidthUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_MIN_HEIGHT
    void OnRichtextMinHeightUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_MAX_WIDTH
    void OnRichtextMaxWidthUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_MAX_HEIGHT
    void OnRichtextMaxHeightUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_LEFT
    void OnRichtextLeftUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_LEFT_UNITS
    void OnRichtextLeftUnitsUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_TOP
    void OnRichtextTopUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_TOP_UNITS
    void OnRichtextTopUnitsUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_RIGHT
    void OnRichtextRightUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_RIGHT_UNITS
    void OnRichtextRightUnitsUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_BOTTOM
    void OnRichtextBottomUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_BOTTOM_UNITS
    void OnRichtextBottomUnitsUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_RICHTEXT_PARA_UP
    void OnRichtextParaUpClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_RICHTEXT_PARA_DOWN
    void OnRichtextParaDownClick( wxCommandEvent& event );

////@end wxRichTextSizePage event handler declarations

////@begin wxRichTextSizePage member function declarations

    int GetPositionMode() const { return m_positionMode ; }
    void SetPositionMode(int value) { m_positionMode = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const std::string& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const std::string& name );
////@end wxRichTextSizePage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin wxRichTextSizePage member variables
    wxBoxSizer* m_parentSizer{nullptr};
    wxBoxSizer* m_floatingAlignmentSizer{nullptr};
    wxBoxSizer* m_floatingSizer{nullptr};
    wxChoice* m_float{nullptr};
    wxBoxSizer* m_alignmentSizer{nullptr};
    wxCheckBox* m_verticalAlignmentCheckbox{nullptr};
    wxChoice* m_verticalAlignmentComboBox{nullptr};
    wxFlexGridSizer* m_sizeSizer{nullptr};
    wxBoxSizer* m_widthSizer{nullptr};
    wxCheckBox* m_widthCheckbox{nullptr};
    wxStaticText* m_widthLabel{nullptr};
    wxTextCtrl* m_width{nullptr};
    wxComboBox* m_unitsW{nullptr};
    wxBoxSizer* m_heightSizer{nullptr};
    wxCheckBox* m_heightCheckbox{nullptr};
    wxStaticText* m_heightLabel{nullptr};
    wxTextCtrl* m_height{nullptr};
    wxComboBox* m_unitsH{nullptr};
    wxCheckBox* m_minWidthCheckbox{nullptr};
    wxBoxSizer* m_minWidthSizer{nullptr};
    wxTextCtrl* m_minWidth{nullptr};
    wxComboBox* m_unitsMinW{nullptr};
    wxCheckBox* m_minHeightCheckbox{nullptr};
    wxBoxSizer* m_minHeightSizer{nullptr};
    wxTextCtrl* m_minHeight{nullptr};
    wxComboBox* m_unitsMinH{nullptr};
    wxCheckBox* m_maxWidthCheckbox{nullptr};
    wxBoxSizer* m_maxWidthSizer{nullptr};
    wxTextCtrl* m_maxWidth{nullptr};
    wxComboBox* m_unitsMaxW{nullptr};
    wxCheckBox* m_maxHeightCheckbox{nullptr};
    wxBoxSizer* m_maxHeightSizer{nullptr};
    wxTextCtrl* m_maxHeight{nullptr};
    wxComboBox* m_unitsMaxH{nullptr};
    wxBoxSizer* m_positionControls{nullptr};
    wxBoxSizer* m_moveObjectParentSizer{nullptr};
    wxBoxSizer* m_positionModeSizer{nullptr};
    wxChoice* m_positionModeCtrl{nullptr};
    wxFlexGridSizer* m_positionGridSizer{nullptr};
    wxBoxSizer* m_leftSizer{nullptr};
    wxCheckBox* m_positionLeftCheckbox{nullptr};
    wxStaticText* m_leftLabel{nullptr};
    wxTextCtrl* m_left{nullptr};
    wxComboBox* m_unitsLeft{nullptr};
    wxBoxSizer* m_topSizer{nullptr};
    wxCheckBox* m_positionTopCheckbox{nullptr};
    wxStaticText* m_topLabel{nullptr};
    wxTextCtrl* m_top{nullptr};
    wxComboBox* m_unitsTop{nullptr};
    wxBoxSizer* m_rightSizer{nullptr};
    wxCheckBox* m_positionRightCheckbox{nullptr};
    wxStaticText* m_rightLabel{nullptr};
    wxBoxSizer* m_rightPositionSizer{nullptr};
    wxTextCtrl* m_right{nullptr};
    wxComboBox* m_unitsRight{nullptr};
    wxBoxSizer* m_bottomSizer{nullptr};
    wxCheckBox* m_positionBottomCheckbox{nullptr};
    wxStaticText* m_bottomLabel{nullptr};
    wxBoxSizer* m_bottomPositionSizer{nullptr};
    wxTextCtrl* m_bottom{nullptr};
    wxComboBox* m_unitsBottom{nullptr};
    wxBoxSizer* m_moveObjectSizer{nullptr};
    int m_positionMode{};
    /// Control identifiers
    enum {
        ID_WXRICHTEXTSIZEPAGE = 10700,
        ID_RICHTEXT_FLOATING_MODE = 10701,
        ID_RICHTEXT_VERTICAL_ALIGNMENT_CHECKBOX = 10708,
        ID_RICHTEXT_VERTICAL_ALIGNMENT_COMBOBOX = 10709,
        ID_RICHTEXT_WIDTH_CHECKBOX = 10702,
        ID_RICHTEXT_WIDTH = 10703,
        ID_RICHTEXT_UNITS_W = 10704,
        ID_RICHTEXT_HEIGHT_CHECKBOX = 10705,
        ID_RICHTEXT_HEIGHT = 10706,
        ID_RICHTEXT_UNITS_H = 10707,
        ID_RICHTEXT_MIN_WIDTH_CHECKBOX = 10715,
        ID_RICHTEXT_MIN_WIDTH = 10716,
        ID_RICHTEXT_UNITS_MIN_W = 10717,
        ID_RICHTEXT_MIN_HEIGHT_CHECKBOX = 10718,
        ID_RICHTEXT_MIN_HEIGHT = 10719,
        ID_RICHTEXT_UNITS_MIN_H = 10720,
        ID_RICHTEXT_MAX_WIDTH_CHECKBOX = 10721,
        ID_RICHTEXT_MAX_WIDTH = 10722,
        ID_RICHTEXT_UNITS_MAX_W = 10723,
        ID_RICHTEXT_MAX_HEIGHT_CHECKBOX = 10724,
        ID_RICHTEXT_MAX_HEIGHT = 10725,
        ID_RICHTEXT_UNITS_MAX_H = 10726,
        ID_RICHTEXT_POSITION_MODE = 10735,
        ID_RICHTEXT_LEFT_CHECKBOX = 10710,
        ID_RICHTEXT_LEFT = 10711,
        ID_RICHTEXT_LEFT_UNITS = 10712,
        ID_RICHTEXT_TOP_CHECKBOX = 10710,
        ID_RICHTEXT_TOP = 10728,
        ID_RICHTEXT_TOP_UNITS = 10729,
        ID_RICHTEXT_RIGHT_CHECKBOX = 10727,
        ID_RICHTEXT_RIGHT = 10730,
        ID_RICHTEXT_RIGHT_UNITS = 10731,
        ID_RICHTEXT_BOTTOM_CHECKBOX = 10732,
        ID_RICHTEXT_BOTTOM = 10733,
        ID_RICHTEXT_BOTTOM_UNITS = 10734,
        ID_RICHTEXT_PARA_UP = 10713,
        ID_RICHTEXT_PARA_DOWN = 10714
    };
////@end wxRichTextSizePage member variables

    inline static bool sm_showFloatingControls{true};
    inline static bool sm_showPositionControls{true};
    inline static bool sm_showMinMaxSizeControls{true};
    inline static bool sm_showPositionModeControls{true};
    inline static bool sm_showRightBottomPositionControls{true};
    inline static bool sm_showAlignmentControls{true};
    inline static bool sm_showFloatingAndAlignmentControls{true};
    inline static bool sm_enablePositionAndSizeUnits{true};
    inline static bool sm_enablePositionAndSizeCheckboxes{true};
    inline static bool sm_showMoveObjectControls{true};
};

#endif
    // _RICHTEXTSIZEPAGE_H_
