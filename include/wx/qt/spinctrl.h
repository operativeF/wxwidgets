/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/spinctrl.h
// Author:      Peter Most, Mariano Reingart
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_SPINCTRL_H_
#define _WX_QT_SPINCTRL_H_

class QSpinBox;
class QDoubleSpinBox;

// Take advantage of the Qt compile time polymorphy and use a template to avoid
// copy&paste code for the usage of QSpinBox/QDoubleSpinBox.

template < typename T, typename Widget >
class wxSpinCtrlQt : public wxSpinCtrlBase
{
public:
    wxSpinCtrlQt();
    wxSpinCtrlQt( wxWindow *parent, wxWindowID id, const wxString& value,
        const wxPoint& pos, const wxSize& size, unsigned int style,
        T min, T max, T initial, T inc,
        const wxString& name );

    bool Create( wxWindow *parent, wxWindowID id, const wxString& value,
        const wxPoint& pos, const wxSize& size, unsigned int style,
        T min, T max, T initial, T inc,
        const wxString& name );

    wxString GetTextValue() const override;
    void SetValue(const wxString&) override {}

    void SetSnapToTicks(bool snap_to_ticks) override;
    bool GetSnapToTicks() const override;

    void SetSelection(long from, long to) override;

    virtual void SetValue(T val);
    void SetRange(T minVal, T maxVal);
    void SetIncrement(T inc);

    T GetValue() const;
    T GetMin() const;
    T GetMax() const;
    T GetIncrement() const;

    QWidget *GetHandle() const override;

protected:
    Widget *m_qtSpinBox;

};

class wxSpinCtrl : public wxSpinCtrlQt< int, QSpinBox >
{
public:
    wxSpinCtrl();
    wxSpinCtrl(wxWindow *parent,
               wxWindowID id = wxID_ANY,
               const wxString& value = wxEmptyString,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               unsigned int style = wxSP_ARROW_KEYS,
               int min = 0, int max = 100, int initial = 0,
               const wxString& name = wxT("wxSpinCtrl"));

    bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                const wxString& value = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxSP_ARROW_KEYS,
                int min = 0, int max = 100, int initial = 0,
                const wxString& name = wxT("wxSpinCtrl"));
    int GetBase() const override { return m_base; }
    bool SetBase(int base) override;
    void SetValue(const wxString & val) override;
    void SetValue(int val) override { wxSpinCtrlQt<int,QSpinBox>::SetValue(val); }

private:
    // Common part of all ctors.
    void Init()
    {
        m_base = 10;
    }
    int m_base;
    wxDECLARE_DYNAMIC_CLASS(wxSpinCtrl);
};

class wxSpinCtrlDouble : public wxSpinCtrlQt< double, QDoubleSpinBox >
{
public:
    wxSpinCtrlDouble();
    wxSpinCtrlDouble(wxWindow *parent,
                     wxWindowID id = wxID_ANY,
                     const wxString& value = wxEmptyString,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     unsigned int style = wxSP_ARROW_KEYS,
                     double min = 0, double max = 100, double initial = 0,
                     double inc = 1,
                     const wxString& name = wxT("wxSpinCtrlDouble"));

    bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                const wxString& value = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxSP_ARROW_KEYS,
                double min = 0, double max = 100, double initial = 0,
                double inc = 1,
                const wxString& name = wxT("wxSpinCtrlDouble"));

    void SetDigits(unsigned digits);
    unsigned GetDigits() const;

    int GetBase() const override { return 10; }
    bool SetBase(int WXUNUSED(base)) override { return false; }
    void SetValue(const wxString & val) override;
    void SetValue(double val) override { wxSpinCtrlQt<double,QDoubleSpinBox>::SetValue(val); }

private:
    wxDECLARE_DYNAMIC_CLASS( wxSpinCtrlDouble );
};

#endif // _WX_QT_SPINCTRL_H_
