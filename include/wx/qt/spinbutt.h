/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/spinbutt.h
// Author:      Peter Most, Mariano Reingart
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_SPINBUTT_H_
#define _WX_QT_SPINBUTT_H_

#include "wx/spinbutt.h"
class QSpinBox;

class wxSpinButton : public wxSpinButtonBase
{
public:
    wxSpinButton();
    wxSpinButton(wxWindow *parent,
                 wxWindowID id = -1,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 unsigned int style = wxSP_VERTICAL,
                 const wxString& name = wxSPIN_BUTTON_NAME);

    bool Create(wxWindow *parent,
                wxWindowID id = -1,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxSP_VERTICAL,
                const wxString& name = wxSPIN_BUTTON_NAME);

    int GetValue() const override;
    void SetValue(int val) override;
    void SetRange(int min, int max) override;

    QWidget *GetHandle() const override;

private:
    QSpinBox *m_qtSpinBox;

    wxDECLARE_DYNAMIC_CLASS( wxSpinButton );
};

#endif // _WX_QT_SPINBUTT_H_
