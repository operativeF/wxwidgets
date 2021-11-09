/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/colordlg.h
// Purpose:     wxColourDialog class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_COLORDLG_H_
#define _WX_COLORDLG_H_

#include "wx/dialog.h"
#include "wx/geometry/rect.h"

import <string>;

// ----------------------------------------------------------------------------
// wxColourDialog: dialog for choosing a colours
// ----------------------------------------------------------------------------

class wxColourDialog : public wxDialog
{
public:
    wxColourDialog() = default;
    wxColourDialog(wxWindow *parent, const wxColourData *data = nullptr)
    {
        // reset to zero, otherwise the wx routines won't size the window the
        // second time the dialog is shown, because they would believe it already
        // has the requested size/position
        // It's defaulted to zero.

        m_parent = parent;
        
        if (data)
            m_colourData = *data;

    }

    wxColourDialog& operator=(wxColourDialog&&) = delete;

    wxColourData& GetColourData() { return m_colourData; }

    // override some base class virtuals
    void SetTitle(const std::string& title) override;
    std::string GetTitle() const override;

    int ShowModal() override;

    // wxMSW-specific implementation from now on
    // -----------------------------------------

    // called from the hook procedure on WM_INITDIALOG reception
    virtual void MSWOnInitDone(WXHWND hDlg);

    // called from the hook procedure
    void MSWCheckIfCurrentChanged(WXCOLORREF currentCol);

protected:
    wxPoint DoGetPosition() const override;
    wxSize DoGetSize() const override;
    wxSize DoGetClientSize() const override;
    void DoMoveWindow(wxRect boundary) override;
    void DoCentre(unsigned int dir) override;

    wxColourData        m_colourData;
    std::string         m_title;

    // Currently selected colour, used while the dialog is being shown.
    WXCOLORREF m_currentCol{};

    // indicates that the dialog should be centered in this direction if non 0
    // (set by DoCentre(), used by MSWOnInitDone())
    unsigned int m_centreDir{};

    // true if DoMoveWindow() had been called
    bool m_movedWindow{false};

    // standard colors dialog size for the Windows systems
    // this is ok if color dialog created with standard color
    // and "Define Custom Colors" extension not shown
    inline static wxRect s_rectDialog{ 0, 0, 222, 324 };

public:
	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_COLORDLG_H_
