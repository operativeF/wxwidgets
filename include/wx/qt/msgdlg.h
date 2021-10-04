/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/msgdlg.h
// Author:      Peter Most, Javier Torres
// Copyright:   (c) Peter Most, Javier Torres
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_MSGDLG_H_
#define _WX_QT_MSGDLG_H_

#include "wx/msgdlg.h"

class QMessageBox;

class WXDLLIMPEXP_CORE wxMessageDialog : public wxMessageDialogBase
{
public:
    wxMessageDialog(wxWindow *parent, const wxString& message,
                    const wxString& caption = wxASCII_STR(wxMessageBoxCaptionStr),
                    unsigned int style = wxOK|wxCENTRE,
                    const wxPoint& pos = wxDefaultPosition);
    virtual ~wxMessageDialog();

	wxMessageDialog(const wxMessageDialog&) = delete;
	wxMessageDialog& operator=(const wxMessageDialog&) = delete;

    // Reimplemented to translate return codes from Qt to wx
    int ShowModal() override;

	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_QT_MSGDLG_H_
