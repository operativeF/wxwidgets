/////////////////////////////////////////////////////////////////////////////
// Name:        wx/richtext/richtextstyledlg.h
// Purpose:     Declares the rich text style editor dialog.
// Author:      Julian Smart
// Modified by:
// Created:     10/5/2006 12:05:31 PM
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _RICHTEXTSTYLEDLG_H_
#define _RICHTEXTSTYLEDLG_H_

/*!
 * Includes
 */

#include "wx/dialog.h"

#include "wx/richtext/richtextuicustomization.h"

////@begin includes
////@end includes

#include "wx/richtext/richtextbuffer.h"
#include "wx/richtext/richtextstyles.h"
#include "wx/richtext/richtextctrl.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
class wxRichTextStyleListCtrl;
class wxRichTextCtrl;
class wxStdDialogButtonSizer;
////@end forward declarations

class WXDLLIMPEXP_FWD_CORE wxButton;
class WXDLLIMPEXP_FWD_CORE wxCheckBox;

/*!
 * Control identifiers
 */

enum {
    ID_RICHTEXTSTYLEORGANISERDIALOG = 10500,
    ID_RICHTEXTSTYLEORGANISERDIALOG_STYLES = 10501,
    ID_RICHTEXTSTYLEORGANISERDIALOG_CURRENT_STYLE = 10510,
    ID_RICHTEXTSTYLEORGANISERDIALOG_PREVIEW = 10509,
    ID_RICHTEXTSTYLEORGANISERDIALOG_NEW_CHAR = 10504,
    ID_RICHTEXTSTYLEORGANISERDIALOG_NEW_PARA = 10505,
    ID_RICHTEXTSTYLEORGANISERDIALOG_NEW_LIST = 10508,
    ID_RICHTEXTSTYLEORGANISERDIALOG_NEW_BOX = 10512,
    ID_RICHTEXTSTYLEORGANISERDIALOG_APPLY = 10503,
    ID_RICHTEXTSTYLEORGANISERDIALOG_RENAME = 10502,
    ID_RICHTEXTSTYLEORGANISERDIALOG_EDIT = 10506,
    ID_RICHTEXTSTYLEORGANISERDIALOG_DELETE = 10507,
    ID_RICHTEXTSTYLEORGANISERDIALOG_RESTART_NUMBERING = 10511
};


#define SYMBOL_WXRICHTEXTSTYLEORGANISERDIALOG_TITLE wxGetTranslation(wxASCII_STR("Style Organiser"))

constexpr unsigned int SYMBOL_WXRICHTEXTSTYLEORGANISERDIALOG_STYLE = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX;
constexpr unsigned int SYMBOL_WXRICHTEXTSTYLEORGANISERDIALOG_IDNAME = ID_RICHTEXTSTYLEORGANISERDIALOG;
constexpr wxSize SYMBOL_WXRICHTEXTSTYLEORGANISERDIALOG_SIZE = {400, 300};
constexpr wxPoint SYMBOL_WXRICHTEXTSTYLEORGANISERDIALOG_POSITION = wxDefaultPosition;

/*!
 * Flags for specifying permitted operations
 */

constexpr unsigned int wxRICHTEXT_ORGANISER_DELETE_STYLES  = 0x0001;
constexpr unsigned int wxRICHTEXT_ORGANISER_CREATE_STYLES  = 0x0002;
constexpr unsigned int wxRICHTEXT_ORGANISER_APPLY_STYLES   = 0x0004;
constexpr unsigned int wxRICHTEXT_ORGANISER_EDIT_STYLES    = 0x0008;
constexpr unsigned int wxRICHTEXT_ORGANISER_RENAME_STYLES  = 0x0010;
constexpr unsigned int wxRICHTEXT_ORGANISER_OK_CANCEL      = 0x0020;
constexpr unsigned int wxRICHTEXT_ORGANISER_RENUMBER       = 0x0040;

// The permitted style types to show
constexpr unsigned int wxRICHTEXT_ORGANISER_SHOW_CHARACTER = 0x0100;
constexpr unsigned int wxRICHTEXT_ORGANISER_SHOW_PARAGRAPH = 0x0200;
constexpr unsigned int wxRICHTEXT_ORGANISER_SHOW_LIST      = 0x0400;
constexpr unsigned int wxRICHTEXT_ORGANISER_SHOW_BOX       = 0x0800;
constexpr unsigned int wxRICHTEXT_ORGANISER_SHOW_ALL       = 0x1000;

// Common combinations
constexpr unsigned int wxRICHTEXT_ORGANISER_ORGANISE = (wxRICHTEXT_ORGANISER_SHOW_ALL|wxRICHTEXT_ORGANISER_DELETE_STYLES|wxRICHTEXT_ORGANISER_CREATE_STYLES|wxRICHTEXT_ORGANISER_APPLY_STYLES|wxRICHTEXT_ORGANISER_EDIT_STYLES|wxRICHTEXT_ORGANISER_RENAME_STYLES);
constexpr unsigned int wxRICHTEXT_ORGANISER_BROWSE = (wxRICHTEXT_ORGANISER_SHOW_ALL|wxRICHTEXT_ORGANISER_OK_CANCEL);
constexpr unsigned int wxRICHTEXT_ORGANISER_BROWSE_NUMBERING = (wxRICHTEXT_ORGANISER_SHOW_LIST|wxRICHTEXT_ORGANISER_OK_CANCEL|wxRICHTEXT_ORGANISER_RENUMBER);

/*!
 * wxRichTextStyleOrganiserDialog class declaration
 */

class WXDLLIMPEXP_RICHTEXT wxRichTextStyleOrganiserDialog: public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(wxRichTextStyleOrganiserDialog);
    wxDECLARE_EVENT_TABLE();
    DECLARE_HELP_PROVISION()

public:
    /// Constructors
    wxRichTextStyleOrganiserDialog() = default;
    wxRichTextStyleOrganiserDialog( int flags, wxRichTextStyleSheet* sheet, wxRichTextCtrl* ctrl, wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& caption = SYMBOL_WXRICHTEXTSTYLEORGANISERDIALOG_TITLE, const wxPoint& pos = SYMBOL_WXRICHTEXTSTYLEORGANISERDIALOG_POSITION, const wxSize& size = SYMBOL_WXRICHTEXTSTYLEORGANISERDIALOG_SIZE, unsigned int style = SYMBOL_WXRICHTEXTSTYLEORGANISERDIALOG_STYLE );

    /// Creation
    bool Create( int flags, wxRichTextStyleSheet* sheet, wxRichTextCtrl* ctrl, wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& caption = SYMBOL_WXRICHTEXTSTYLEORGANISERDIALOG_TITLE, const wxPoint& pos = SYMBOL_WXRICHTEXTSTYLEORGANISERDIALOG_POSITION, const wxSize& size = SYMBOL_WXRICHTEXTSTYLEORGANISERDIALOG_SIZE, unsigned int style = SYMBOL_WXRICHTEXTSTYLEORGANISERDIALOG_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

    /// Transfer data from/to window
    bool TransferDataFromWindow() override;
    bool TransferDataToWindow() override;

    /// Set/get style sheet
    void SetStyleSheet(wxRichTextStyleSheet* sheet) { m_richTextStyleSheet = sheet; }
    wxRichTextStyleSheet* GetStyleSheet() const { return m_richTextStyleSheet; }

    /// Set/get control
    void SetRichTextCtrl(wxRichTextCtrl* ctrl) { m_richTextCtrl = ctrl; }
    wxRichTextCtrl* GetRichTextCtrl() const { return m_richTextCtrl; }

    /// Set/get flags
    void SetFlags(int flags) { m_flags = flags; }
    int GetFlags() const { return m_flags; }

    /// Show preview for given or selected preview
    void ShowPreview(int sel = -1);

    /// Clears the preview
    void ClearPreview();

    /// List selection
    void OnListSelection(wxCommandEvent& event);

    /// Get/set restart numbering boolean
    bool GetRestartNumbering() const { return m_restartNumbering; }
    void SetRestartNumbering(bool restartNumbering) { m_restartNumbering = restartNumbering; }

    /// Get selected style name or definition
    wxString GetSelectedStyle() const;
    wxRichTextStyleDefinition* GetSelectedStyleDefinition() const;

    /// Apply the style
    bool ApplyStyle(wxRichTextCtrl* ctrl = nullptr);

    /// Should we show tooltips?
    static bool ShowToolTips() { return sm_showToolTips; }

    /// Determines whether tooltips will be shown
    static void SetShowToolTips(bool show) { sm_showToolTips = show; }

////@begin wxRichTextStyleOrganiserDialog event handler declarations

    /// wxEVT_BUTTON event handler for ID_RICHTEXTSTYLEORGANISERDIALOG_NEW_CHAR
    void OnNewCharClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTSTYLEORGANISERDIALOG_NEW_CHAR
    void OnNewCharUpdate( wxUpdateUIEvent& event );

    /// wxEVT_BUTTON event handler for ID_RICHTEXTSTYLEORGANISERDIALOG_NEW_PARA
    void OnNewParaClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTSTYLEORGANISERDIALOG_NEW_PARA
    void OnNewParaUpdate( wxUpdateUIEvent& event );

    /// wxEVT_BUTTON event handler for ID_RICHTEXTSTYLEORGANISERDIALOG_NEW_LIST
    void OnNewListClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTSTYLEORGANISERDIALOG_NEW_LIST
    void OnNewListUpdate( wxUpdateUIEvent& event );

    /// wxEVT_BUTTON event handler for ID_RICHTEXTSTYLEORGANISERDIALOG_NEW_BOX
    void OnNewBoxClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTSTYLEORGANISERDIALOG_NEW_BOX
    void OnNewBoxUpdate( wxUpdateUIEvent& event );

    /// wxEVT_BUTTON event handler for ID_RICHTEXTSTYLEORGANISERDIALOG_APPLY
    void OnApplyClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTSTYLEORGANISERDIALOG_APPLY
    void OnApplyUpdate( wxUpdateUIEvent& event );

    /// wxEVT_BUTTON event handler for ID_RICHTEXTSTYLEORGANISERDIALOG_RENAME
    void OnRenameClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTSTYLEORGANISERDIALOG_RENAME
    void OnRenameUpdate( wxUpdateUIEvent& event );

    /// wxEVT_BUTTON event handler for ID_RICHTEXTSTYLEORGANISERDIALOG_EDIT
    void OnEditClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTSTYLEORGANISERDIALOG_EDIT
    void OnEditUpdate( wxUpdateUIEvent& event );

    /// wxEVT_BUTTON event handler for ID_RICHTEXTSTYLEORGANISERDIALOG_DELETE
    void OnDeleteClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_RICHTEXTSTYLEORGANISERDIALOG_DELETE
    void OnDeleteUpdate( wxUpdateUIEvent& event );

    /// wxEVT_BUTTON event handler for wxID_HELP
    void OnHelpClick( wxCommandEvent& event );

////@end wxRichTextStyleOrganiserDialog event handler declarations

////@begin wxRichTextStyleOrganiserDialog member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end wxRichTextStyleOrganiserDialog member function declarations

////@begin wxRichTextStyleOrganiserDialog member variables
    wxBoxSizer* m_innerSizer{nullptr};
    wxBoxSizer* m_buttonSizerParent{nullptr};
    wxRichTextStyleListCtrl* m_stylesListBox{nullptr};
    wxRichTextCtrl* m_previewCtrl{nullptr};
    wxBoxSizer* m_buttonSizer{nullptr};
    wxButton* m_newCharacter{nullptr};
    wxButton* m_newParagraph{nullptr};
    wxButton* m_newList{nullptr};
    wxButton* m_newBox{nullptr};
    wxButton* m_applyStyle{nullptr};
    wxButton* m_renameStyle{nullptr};
    wxButton* m_editStyle{nullptr};
    wxButton* m_deleteStyle{nullptr};
    wxButton* m_closeButton{nullptr};
    wxBoxSizer* m_bottomButtonSizer{nullptr};
    wxCheckBox* m_restartNumberingCtrl{nullptr};
    wxStdDialogButtonSizer* m_stdButtonSizer{nullptr};
    wxButton* m_okButton{nullptr};
    wxButton* m_cancelButton{nullptr};

private:

    wxRichTextCtrl*         m_richTextCtrl{nullptr};
    wxRichTextStyleSheet*   m_richTextStyleSheet{nullptr};

    unsigned int            m_flags{};

    bool                    m_dontUpdate{false};
    bool                    m_restartNumbering{true};

    inline static bool      sm_showToolTips{false};
};

#endif
    // _RICHTEXTSTYLEDLG_H_
