///////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/popupwin.h
// Purpose:     wxPopupWindow class for wxMac
// Author:      Stefan Csomor
// Modified by:
// Created:
// Copyright:   (c) 2006 Stefan Csomor
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MAC_POPUPWIN_H_
#define _WX_MAC_POPUPWIN_H_

// ----------------------------------------------------------------------------
// wxPopupWindow
// ----------------------------------------------------------------------------

class wxPopupWindow : public wxPopupWindowBase
{
public:
    wxPopupWindow() { }
    ~wxPopupWindow();

    wxPopupWindow(wxWindow *parent, int flags = wxBORDER_NONE)
        { (void)Create(parent, flags); }

	wxPopupWindow(const wxPopupWindow&) = delete;
	wxPopupWindow& operator=(const wxPopupWindow&) = delete;

    bool Create(wxWindow *parent, int flags = wxBORDER_NONE);

    bool Show(bool show = true) override;

	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_MAC_POPUPWIN_H_

