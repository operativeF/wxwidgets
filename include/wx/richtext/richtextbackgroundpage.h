/////////////////////////////////////////////////////////////////////////////
// Name:        wx/richtext/richtextbackgroundpage.h
// Purpose:     Declares the rich text formatting dialog background
//              properties page.
// Author:      Julian Smart
// Modified by:
// Created:     13/11/2010 11:17:25
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _RICHTEXTBACKGROUNDPAGE_H_
#define _RICHTEXTBACKGROUNDPAGE_H_

/*!
 * Includes
 */

#include "wx/richtext/richtextdialogpage.h"

////@begin includes
#include "wx/statline.h"
////@end includes

/*!
 * Forward declarations
 */

class WXDLLIMPEXP_FWD_RICHTEXT wxRichTextColourSwatchCtrl;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_WXRICHTEXTBACKGROUNDPAGE_STYLE wxTAB_TRAVERSAL
#define SYMBOL_WXRICHTEXTBACKGROUNDPAGE_TITLE {}
#define SYMBOL_WXRICHTEXTBACKGROUNDPAGE_IDNAME ID_RICHTEXTBACKGROUNDPAGE
#define SYMBOL_WXRICHTEXTBACKGROUNDPAGE_SIZE wxSize(400, 300)
#define SYMBOL_WXRICHTEXTBACKGROUNDPAGE_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * wxRichTextBackgroundPage class declaration
 */

class WXDLLIMPEXP_RICHTEXT wxRichTextBackgroundPage: public wxRichTextDialogPage
{
    wxDECLARE_DYNAMIC_CLASS(wxRichTextBackgroundPage);
    wxDECLARE_EVENT_TABLE();
    DECLARE_HELP_PROVISION()

public:
    /// Constructors
    wxRichTextBackgroundPage() = default;
    wxRichTextBackgroundPage( wxWindow* parent, wxWindowID id = SYMBOL_WXRICHTEXTBACKGROUNDPAGE_IDNAME, const wxPoint& pos = SYMBOL_WXRICHTEXTBACKGROUNDPAGE_POSITION, const wxSize& size = SYMBOL_WXRICHTEXTBACKGROUNDPAGE_SIZE, unsigned int style = SYMBOL_WXRICHTEXTBACKGROUNDPAGE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_WXRICHTEXTBACKGROUNDPAGE_IDNAME, const wxPoint& pos = SYMBOL_WXRICHTEXTBACKGROUNDPAGE_POSITION, const wxSize& size = SYMBOL_WXRICHTEXTBACKGROUNDPAGE_SIZE, unsigned int style = SYMBOL_WXRICHTEXTBACKGROUNDPAGE_STYLE );

    /// Destructor
    ~wxRichTextBackgroundPage();

    /// Creates the controls and sizers
    void CreateControls();

    /// Gets the attributes from the formatting dialog
    wxRichTextAttr* GetAttributes();

    /// Data transfer
    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;

    /// Respond to colour swatch click
    void OnColourSwatch(wxCommandEvent& event);

////@begin wxRichTextBackgroundPage event handler declarations

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_SHADOW_HORIZONTAL_OFFSET
    void OnRichtextShadowUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTSHADOWCOLOURSWATCHCTRL
    void OnRichtextshadowcolourswatchctrlUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_SHADOW_SPREAD
    void OnRichtextShadowSpreadUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_SHADOW_BLUR_DISTANCE
    void OnRichtextShadowBlurUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXT_SHADOW_OPACITY
    void OnRichtextShadowOpacityUpdate( wxUpdateUIEvent& event );

////@end wxRichTextBackgroundPage event handler declarations

////@begin wxRichTextBackgroundPage member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const std::string& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const std::string& name );
////@end wxRichTextBackgroundPage member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin wxRichTextBackgroundPage member variables
    wxCheckBox* m_backgroundColourCheckBox{nullptr};
    wxRichTextColourSwatchCtrl* m_backgroundColourSwatch{nullptr};
    wxBoxSizer* m_shadowBox{nullptr};
    wxCheckBox* m_useShadow{nullptr};
    wxTextCtrl* m_offsetX{nullptr};
    wxComboBox* m_unitsHorizontalOffset{nullptr};
    wxTextCtrl* m_offsetY{nullptr};
    wxComboBox* m_unitsVerticalOffset{nullptr};
    wxCheckBox* m_shadowColourCheckBox{nullptr};
    wxRichTextColourSwatchCtrl* m_shadowColourSwatch{nullptr};
    wxCheckBox* m_useShadowSpread{nullptr};
    wxTextCtrl* m_spread{nullptr};
    wxComboBox* m_unitsShadowSpread{nullptr};
    wxCheckBox* m_useBlurDistance{nullptr};
    wxTextCtrl* m_blurDistance{nullptr};
    wxComboBox* m_unitsBlurDistance{nullptr};
    wxCheckBox* m_useShadowOpacity{nullptr};
    wxTextCtrl* m_opacity{nullptr};
    /// Control identifiers
    enum {
        ID_RICHTEXTBACKGROUNDPAGE = 10845,
        ID_RICHTEXT_BACKGROUND_COLOUR_CHECKBOX = 10846,
        ID_RICHTEXT_BACKGROUND_COLOUR_SWATCH = 10847,
        ID_RICHTEXT_USE_SHADOW = 10840,
        ID_RICHTEXT_SHADOW_HORIZONTAL_OFFSET = 10703,
        ID_RICHTEXT_SHADOW_HORIZONTAL_OFFSET_UNITS = 10712,
        ID_RICHTEXT_SHADOW_VERTICAL_OFFSET = 10841,
        ID_RICHTEXT_SHADOW_VERTICAL_OFFSET_UNITS = 10842,
        ID_RICHTEXT_USE_SHADOW_COLOUR = 10843,
        ID_RICHTEXTSHADOWCOLOURSWATCHCTRL = 10844,
        ID_RICHTEXT_USE_SHADOW_SPREAD = 10851,
        ID_RICHTEXT_SHADOW_SPREAD = 10848,
        ID_RICHTEXT_SHADOW_SPREAD_UNITS = 10849,
        ID_RICHTEXT_USE_BLUR_DISTANCE = 10855,
        ID_RICHTEXT_SHADOW_BLUR_DISTANCE = 10852,
        ID_RICHTEXT_SHADOW_BLUR_DISTANCE_UNITS = 10853,
        ID_RICHTEXT_USE_SHADOW_OPACITY = 10856,
        ID_RICHTEXT_SHADOW_OPACITY = 10854
    };
////@end wxRichTextBackgroundPage member variables
};

#endif
    // _RICHTEXTBACKGROUNDPAGE_H_
