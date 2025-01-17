/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/stattext.h
// Author:      Peter Most, Mariano Reingart
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_STATTEXT_H_
#define _WX_QT_STATTEXT_H_

class QLabel;

class wxStaticText : public wxStaticTextBase
{
public:
    wxStaticText();
    wxStaticText(wxWindow *parent,
                 wxWindowID id,
                 const wxString &label,
                 const wxPoint &pos = wxDefaultPosition,
                 const wxSize &size = wxDefaultSize,
                 unsigned int style = 0,
                 const wxString &name = wxASCII_STR(wxStaticTextNameStr) );

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString &label,
                const wxPoint &pos = wxDefaultPosition,
                const wxSize &size = wxDefaultSize,
                unsigned int style = 0,
                const wxString &name = wxASCII_STR(wxStaticTextNameStr) );

    void SetLabel(const wxString& label) override;

    QWidget *GetHandle() const override;

protected:
    wxString WXGetVisibleLabel() const override;
    void WXSetVisibleLabel(const wxString& str) override;

private:
    QLabel *m_qtLabel;

    wxDECLARE_DYNAMIC_CLASS( wxStaticText );
};

#endif // _WX_QT_STATTEXT_H_
