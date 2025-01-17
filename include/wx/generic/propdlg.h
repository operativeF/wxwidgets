/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/propdlg.h
// Purpose:     wxPropertySheetDialog
// Author:      Julian Smart
// Modified by:
// Created:     2005-03-12
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PROPDLG_H_
#define _WX_PROPDLG_H_

#if wxUSE_BOOKCTRL

#include "wx/dialog.h"

class wxBookCtrlBase;

//-----------------------------------------------------------------------------
// wxPropertySheetDialog
// A platform-independent properties dialog with a notebook and standard
// buttons.
//
// To use this class, call Create from your derived class.
// Then create pages and add to the book control. Finally call CreateButtons and
// LayoutDialog.
//
// For example:
//
// MyPropertySheetDialog::Create(...)
// {
//     wxPropertySheetDialog::Create(...);
//
//     // Add page
//     wxPanel* panel = new wxPanel(GetBookCtrl(), ...);
//     GetBookCtrl()->AddPage(panel, "General");
//
//     CreateButtons();
//     LayoutDialog();
// }
//
// Override CreateBookCtrl and AddBookCtrl to create and add a different
// kind of book control.
//-----------------------------------------------------------------------------

enum wxPropertySheetDialogFlags
{
    // Use the platform default
    wxPROPSHEET_DEFAULT = 0x0001,

    // Use a notebook
    wxPROPSHEET_NOTEBOOK = 0x0002,

    // Use a toolbook
    wxPROPSHEET_TOOLBOOK = 0x0004,

    // Use a choicebook
    wxPROPSHEET_CHOICEBOOK = 0x0008,

    // Use a listbook
    wxPROPSHEET_LISTBOOK = 0x0010,

    // Use a wxButtonToolBar toolbook
    wxPROPSHEET_BUTTONTOOLBOOK = 0x0020,

    // Use a treebook
    wxPROPSHEET_TREEBOOK = 0x0040,

    // Shrink dialog to fit current page
    wxPROPSHEET_SHRINKTOFIT = 0x0100
};

class wxPropertySheetDialog : public wxDialog
{
public:
    wxPropertySheetDialog() = default;

    wxPropertySheetDialog(wxWindow* parent, wxWindowID id,
                       const std::string& title,
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& sz = wxDefaultSize,
                       unsigned int style = wxDEFAULT_DIALOG_STYLE,
                       std::string_view name = wxDialogNameStr)
    {
        Create(parent, id, title, pos, sz, style, name);
    }

    bool Create(wxWindow* parent, wxWindowID id,
                       const std::string& title,
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& sz = wxDefaultSize,
                       unsigned int style = wxDEFAULT_DIALOG_STYLE,
                       std::string_view name = wxDialogNameStr);

//// Accessors

    // Set and get the notebook
    void SetBookCtrl(wxBookCtrlBase* book) { m_bookCtrl = book; }
    wxBookCtrlBase* GetBookCtrl() const { return m_bookCtrl; }

    // Override function in base
    wxWindow* GetContentWindow() const override;

    // Set and get the inner sizer
    void SetInnerSizer(wxSizer* sizer) { m_innerSizer = sizer; }
    wxSizer* GetInnerSizer() const { return m_innerSizer ; }

    // Set and get the book style
    void SetSheetStyle(long sheetStyle) { m_sheetStyle = sheetStyle; }
    long GetSheetStyle() const { return m_sheetStyle ; }

    // Set and get the border around the whole dialog
    void SetSheetOuterBorder(int border) { m_sheetOuterBorder = border; }
    int GetSheetOuterBorder() const { return m_sheetOuterBorder ; }

    // Set and get the border around the book control only
    void SetSheetInnerBorder(int border) { m_sheetInnerBorder = border; }
    int GetSheetInnerBorder() const { return m_sheetInnerBorder ; }

/// Operations

    // Creates the buttons
    virtual void CreateButtons(unsigned int flags = wxOK|wxCANCEL);

    // Lay out the dialog, to be called after pages have been created
    virtual void LayoutDialog(unsigned int centreFlags = wxBOTH);

/// Implementation

    // Creates the book control. If you want to use a different kind of
    // control, override.
    virtual wxBookCtrlBase* CreateBookCtrl();

    // Adds the book control to the inner sizer.
    virtual void AddBookCtrl(wxSizer* sizer);

    // Resize dialog if necessary
    void OnIdle(wxIdleEvent& event);

protected:
    wxBookCtrlBase* m_bookCtrl{nullptr};
    wxSizer*        m_innerSizer{nullptr}; // sizer for extra space
    long            m_sheetStyle{wxPROPSHEET_DEFAULT};
    int             m_sheetOuterBorder{2};
    int             m_sheetInnerBorder{5};
    int             m_selectedPage{};

    wxDECLARE_EVENT_TABLE();
};

#endif // wxUSE_BOOKCTRL

#endif // _WX_PROPDLG_H_

