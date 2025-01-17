/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/gauge.h
// Purpose:
// Author:      Robert Roebling
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTK_GAUGE_H_
#define _WX_GTK_GAUGE_H_

//-----------------------------------------------------------------------------
// wxGauge
//-----------------------------------------------------------------------------

class wxGauge: public wxGaugeBase
{
public:
    wxGauge() { Init(); }

    wxGauge( wxWindow *parent,
             wxWindowID id,
             int range,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = wxGA_HORIZONTAL,
             const wxValidator& validator = wxDefaultValidator,
             const wxString& name = wxASCII_STR(wxGaugeNameStr) )
    {
        Init();

        Create(parent, id, range, pos, size, style, validator, name);
    }

    bool Create( wxWindow *parent,
                 wxWindowID id, int range,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxGA_HORIZONTAL,
                 const wxValidator& validator = wxDefaultValidator,
                 const wxString& name = wxASCII_STR(wxGaugeNameStr) );

    // implement base class virtual methods
    void SetRange(int range) override;
    int GetRange() const override;

    void SetValue(int pos) override;
    int GetValue() const override;

    void Pulse() override;

    static wxVisualAttributes
    GetClassDefaultAttributes(wxWindowVariant variant = wxWINDOW_VARIANT_NORMAL);

    wxVisualAttributes GetDefaultAttributes() const override;

    // implementation
    // -------------

    // the max and current gauge values
    int m_rangeMax,
        m_gaugePos;

protected:
    // set the gauge value to the value of m_gaugePos
    void DoSetGauge();

private:
    void Init() { m_rangeMax = m_gaugePos = 0; }

    wxDECLARE_DYNAMIC_CLASS(wxGauge);
};

#endif
    // _WX_GTK_GAUGE_H_
