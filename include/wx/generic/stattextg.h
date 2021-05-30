/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/stattextg.h
// Purpose:     wxGenericStaticText header
// Author:      Marcin Wojdyr
// Created:     2008-06-26
// Copyright:   Marcin Wojdyr
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_STATTEXTG_H_
#define _WX_GENERIC_STATTEXTG_H_

// prevent it from including the platform-specific wxStaticText declaration as
// this is not going to compile if it derives from wxGenericStaticText defined
// below (currently this is only the case in wxUniv but it could also happen
// with other ports)
#define wxNO_PORT_STATTEXT_INCLUDE
#include "wx/stattext.h"
#undef wxNO_PORT_STATTEXT_INCLUDE

class WXDLLIMPEXP_CORE wxGenericStaticText : public wxStaticTextBase
{
public:
    wxGenericStaticText() { 
#if wxUSE_MARKUP
        m_markupText = nullptr;
#endif // wxUSE_MARKUP
     }

    wxGenericStaticText(wxWindow *parent,
                 wxWindowID id,
                 const wxString& label,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = 0,
                 const wxString& name = wxASCII_STR(wxStaticTextNameStr))
    {
        
#if wxUSE_MARKUP
        m_markupText = nullptr;
#endif // wxUSE_MARKUP
    

        Create(parent, id, label, pos, size, style, name);
    }

    wxGenericStaticText(const wxGenericStaticText&) = delete;
	wxGenericStaticText& operator=(const wxGenericStaticText&) = delete;

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& label,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = wxASCII_STR(wxStaticTextNameStr));

    ~wxGenericStaticText() override;


    // overridden base class virtual methods
    void SetLabel(const wxString& label) override;
    bool SetFont(const wxFont &font) override;

protected:
    wxSize DoGetBestClientSize() const override;

    wxString WXGetVisibleLabel() const override { return m_label; }
    void WXSetVisibleLabel(const wxString& label) override;

    void DoSetSize(int x, int y, int width, int height, int sizeFlags) override;

#if wxUSE_MARKUP
    bool DoSetLabelMarkup(const wxString& markup) override;
#endif // wxUSE_MARKUP

private:
    

    void OnPaint(wxPaintEvent& event);

    void DoDrawLabel(wxDC& dc, const wxRect& rect);

    // These fields are only used if m_markupText == NULL.
    wxString m_label;
    int m_mnemonic;

#if wxUSE_MARKUP
    class wxMarkupText *m_markupText;
#endif // wxUSE_MARKUP

public:
	wxClassInfo *GetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_GENERIC_STATTEXTG_H_

