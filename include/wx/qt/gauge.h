/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/gauge.h
// Author:      Peter Most, Mariano Reingart
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_GAUGE_H_
#define _WX_QT_GAUGE_H_

class QProgressBar;

class wxGauge : public wxGaugeBase
{
public:
    wxGauge();

    wxGauge(wxWindow *parent,
            wxWindowID id,
            int range,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            unsigned int style = wxGA_HORIZONTAL,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxASCII_STR(wxGaugeNameStr));

    bool Create(wxWindow *parent,
                wxWindowID id,
                int range,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxGA_HORIZONTAL,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxASCII_STR(wxGaugeNameStr));

    QWidget *GetHandle() const override;

    // set/get the control range
    void SetRange(int range) override;
    int GetRange() const override;

    void SetValue(int pos) override;
    int GetValue() const override;

private:
    QProgressBar *m_qtProgressBar;

    wxDECLARE_DYNAMIC_CLASS(wxGauge);
};

#endif // _WX_QT_GAUGE_H_
