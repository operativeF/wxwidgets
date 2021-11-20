/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/richmsgdlg.h
// Purpose:     wxRichMessageDialog
// Author:      Rickard Westerlund
// Created:     2010-07-04
// Copyright:   (c) 2010 wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_RICHMSGDLG_H_
#define _WX_MSW_RICHMSGDLG_H_

import <string>;

class wxRichMessageDialog : public wxGenericRichMessageDialog
{
public:
    wxRichMessageDialog(wxWindow *parent,
                        const std::string& message,
                        std::string_view caption = wxMessageBoxCaptionStr,
                        unsigned int style = wxOK | wxCENTRE)
        : wxGenericRichMessageDialog(parent, message, caption, style)
    {}

    wxRichMessageDialog& operator=(wxRichMessageDialog&&) = delete;

    // overridden base class method showing the native task dialog if possible
    int ShowModal() override;

	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_MSW_RICHMSGDLG_H_
