/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/dirdlg.h
// Purpose:     wxDirDialog class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DIRDLG_H_
#define _WX_DIRDLG_H_

#include <string>

class WXDLLIMPEXP_CORE wxDirDialog : public wxDirDialogBase
{
public:
    wxDirDialog(wxWindow *parent,
                const std::string& message = wxDirSelectorPromptStr,
                const std::string& defaultPath = {},
                unsigned int style = wxDD_DEFAULT_STYLE,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                const std::string& name = wxDirDialogNameStr);

    wxDirDialog& operator=(wxDirDialog&&) = delete;

    void SetPath(const std::string& path) override;

    int ShowModal() override;

private:
    // The real implementations of ShowModal(), used for Windows versions
    // before and since Vista.
    int ShowSHBrowseForFolder(WXHWND owner);
    int ShowIFileOpenDialog(WXHWND owner);

public:
	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif
    // _WX_DIRDLG_H_
