/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/fontdlg.h
// Purpose:     wxFontDialog class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_FONTDLG_H_
#define _WX_MSW_FONTDLG_H_

import <string>;

// ----------------------------------------------------------------------------
// wxFontDialog
// ----------------------------------------------------------------------------

class wxFontDialog : public wxFontDialogBase
{
public:
    wxFontDialog() = default;
    explicit wxFontDialog(wxWindow *parent)
        : wxFontDialogBase(parent) { Create(parent); }
    wxFontDialog(wxWindow *parent, const wxFontData& data)
        : wxFontDialogBase(parent, data) { Create(parent, data); }

    wxFontDialog& operator=(wxFontDialog&&) = delete;

    int ShowModal() override;
    void SetTitle(const std::string& title) override;
    std::string GetTitle() const override;

private:
    std::string m_title;
};

#endif
    // _WX_MSW_FONTDLG_H_
