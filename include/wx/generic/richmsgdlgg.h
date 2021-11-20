/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/richmsgdlgg.h
// Purpose:     wxGenericRichMessageDialog
// Author:      Rickard Westerlund
// Created:     2010-07-04
// Copyright:   (c) 2010 wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_RICHMSGDLGG_H_
#define _WX_GENERIC_RICHMSGDLGG_H_

#include "wx/collpane.h"

import <string>;
import <string_view>;

class wxCheckBox;
class wxCollapsiblePaneEvent;

class wxGenericRichMessageDialog
                        : public wxRichMessageDialogBase
{
public:
    wxGenericRichMessageDialog(wxWindow *parent,
                               std::string_view message,
                               std::string_view caption = wxMessageBoxCaptionStr,
                               unsigned int style = wxOK | wxCENTRE)
        : wxRichMessageDialogBase( parent, message, caption, style ),
          m_checkBox(nullptr),
          m_detailsPane(nullptr)
    {}

    wxGenericRichMessageDialog(const wxGenericRichMessageDialog&) = delete;
    wxGenericRichMessageDialog& operator=(const wxGenericRichMessageDialog&) = delete;

    bool IsCheckBoxChecked() const override;

protected:
    wxCheckBox *m_checkBox;
    wxCollapsiblePane *m_detailsPane;

    // overrides methods in the base class
    void AddMessageDialogCheckBox(wxSizer *sizer) override;
    void AddMessageDialogDetails(wxSizer *sizer) override;

private:
    void OnPaneChanged(wxCollapsiblePaneEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // _WX_GENERIC_RICHMSGDLGG_H_
