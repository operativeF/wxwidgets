/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/stattext.h
// Purpose:     wxStaticText class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_STATTEXT_H_
#define _WX_STATTEXT_H_

class wxStaticText: public wxStaticTextBase
{
public:
    wxStaticText() { }

    wxStaticText(wxWindow *parent, wxWindowID id,
           const wxString& label,
           const wxPoint& pos = wxDefaultPosition,
           const wxSize& size = wxDefaultSize,
           long style = 0,
           const wxString& name = wxASCII_STR(wxStaticTextNameStr))
  {
    Create(parent, id, label, pos, size, style, name);
  }

	wxStaticText(const wxStaticText&) = delete;
	wxStaticText& operator=(const wxStaticText&) = delete;

  bool Create(wxWindow *parent, wxWindowID id,
           const wxString& label,
           const wxPoint& pos = wxDefaultPosition,
           const wxSize& size = wxDefaultSize,
           long style = 0,
           const wxString& name = wxASCII_STR(wxStaticTextNameStr));

  // accessors
  void SetLabel( const wxString &str ) override;
  bool SetFont( const wxFont &font ) override;

    bool AcceptsFocus() const override { return false; }

protected :

    wxString WXGetVisibleLabel() const override;
    void WXSetVisibleLabel(const wxString& str) override;

  wxSize DoGetBestSize() const override;

#if wxUSE_MARKUP && wxOSX_USE_COCOA
    bool DoSetLabelMarkup(const wxString& markup) override;
#endif // wxUSE_MARKUP && wxOSX_USE_COCOA

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif
    // _WX_STATTEXT_H_
