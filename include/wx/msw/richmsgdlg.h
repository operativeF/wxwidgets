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

class WXDLLIMPEXP_CORE wxRichMessageDialog : public wxGenericRichMessageDialog
{
public:
    wxRichMessageDialog(wxWindow *parent,
                        const wxString& message,
                        const wxString& caption = wxASCII_STR(wxMessageBoxCaptionStr),
                        long style = wxOK | wxCENTRE)
        : wxGenericRichMessageDialog(parent, message, caption, style)
        { }

	wxRichMessageDialog(const wxRichMessageDialog&) = delete;
	wxRichMessageDialog& operator=(const wxRichMessageDialog&) = delete;

    // overridden base class method showing the native task dialog if possible
    int ShowModal() override;

	wxClassInfo *GetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_MSW_RICHMSGDLG_H_
