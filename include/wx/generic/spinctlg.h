/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/spinctlg.h
// Purpose:     generic wxSpinCtrl class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     28.10.99
// Copyright:   (c) Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_SPINCTRL_H_
#define _WX_GENERIC_SPINCTRL_H_

// ----------------------------------------------------------------------------
// wxSpinCtrl is a combination of wxSpinButton and wxTextCtrl, so if
// wxSpinButton is available, this is what we do - but if it isn't, we still
// define wxSpinCtrl class which then has the same appearance as wxTextCtrl but
// the different interface. This allows to write programs using wxSpinCtrl
// without tons of #ifdefs.
// ----------------------------------------------------------------------------

#if wxUSE_SPINBTN

#include "wx/compositewin.h"

import Utils.Geometry;

class wxSpinButton;
class wxTextCtrl;

class wxSpinCtrlTextGeneric; // wxTextCtrl used for the wxSpinCtrlGenericBase

// The !wxUSE_SPINBTN version's GetValue() function conflicts with the
// wxTextCtrl's GetValue() and so you have to input a dummy int value.
#define wxSPINCTRL_GETVALUE_FIX

// ----------------------------------------------------------------------------
// wxSpinCtrlGeneric is a combination of wxTextCtrl and wxSpinButton
//
// This class manages a double valued generic spinctrl through the DoGet/SetXXX
// functions that are made public as Get/SetXXX functions for int or double
// for the wxSpinCtrl and wxSpinCtrlDouble classes respectively to avoid
// function ambiguity.
// ----------------------------------------------------------------------------

class wxSpinCtrlGenericBase
                : public wxNavigationEnabled<wxCompositeWindow<wxSpinCtrlBase> >
{
public:
    wxSpinCtrlGenericBase() = default;

    bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                std::string_view value = {},
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxSP_ARROW_KEYS,
                double min = 0, double max = 100, double initial = 0,
                double inc = 1,
                std::string_view name = std::string_view{"wxSpinCtrl"});

    ~wxSpinCtrlGenericBase();

    std::string GetTextValue() const override;
    // T GetValue() const
    // T GetMin() const
    // T GetMax() const
    // T GetIncrement() const
    bool GetSnapToTicks() const override { return m_snap_to_ticks; }
    // unsigned GetDigits() const                   - wxSpinCtrlDouble only

    // operations
    void SetValue(std::string_view text) override;
    // void SetValue(T val)
    // void SetRange(T minVal, T maxVal)
    // void SetIncrement(T inc)
    void SetSnapToTicks(bool snap_to_ticks) override;
    // void SetDigits(unsigned digits)              - wxSpinCtrlDouble only

    // Select text in the textctrl
    void SetSelection(long from, long to) override;

    // implementation from now on

    // forward these functions to all subcontrols
    bool Enable(bool enable = true) override;
    bool Show(bool show = true) override;

    bool SetBackgroundColour(const wxColour& colour) override;

    // get the subcontrols
    wxTextCtrl   *GetText() const       { return m_textCtrl; }
    wxSpinButton *GetSpinButton() const { return m_spinButton; }

    // forwarded events from children windows
    void OnSpinButton(wxSpinEvent& event);
    void OnTextLostFocus(wxFocusEvent& event);
    void OnTextChar(wxKeyEvent& event);

    // this window itself is used only as a container for its sub windows so it
    // shouldn't accept the focus at all and any attempts to explicitly set
    // focus to it should give focus to its text constol part
    bool AcceptsFocus() const override { return false; }
    void SetFocus() override;

    friend class wxSpinCtrlTextGeneric;

protected:
    // override the base class virtuals involved into geometry calculations
    wxSize DoGetBestSize() const override;
    wxSize DoGetSizeFromTextSize(int xlen, int ylen = -1) const override;
    void DoMoveWindow(wxRect boundary) override;

#ifdef __WXMSW__
    // and, for MSW, enabling this window itself
    void DoEnable(bool enable) override;
#endif // __WXMSW__

    enum class SendEvent
    {
        None,
        Text
    };

    // generic double valued functions
    double DoGetValue() const { return m_value; }
    bool DoSetValue(double val, SendEvent sendEvent);
    void DoSetRange(double min_val, double max_val);
    void DoSetIncrement(double inc);

    // update our value to reflect the text control contents (if it has been
    // modified by user, do nothing otherwise)
    //
    // can also change the text control if its value is invalid
    //
    // return true if our value has changed
    bool SyncSpinToText(SendEvent sendEvent);

    // Send the correct event type
    virtual void DoSendEvent() = 0;

    // Convert the text to/from the corresponding value.
    virtual bool DoTextToValue(const wxString& text, double *val) = 0;
    virtual std::string DoValueToText(double val) = 0;

    // check if the value is in range
    bool InRange(double n) const { return (n >= m_min) && (n <= m_max); }

    // ensure that the value is in range wrapping it round if necessary
    double AdjustToFitInRange(double value) const;

    // Assign validator with current parameters
    virtual void ResetTextValidator() = 0;

    // the subcontrols
    wxTextCtrl   *m_textCtrl{nullptr};
    wxSpinButton *m_spinButton{nullptr};

    double m_value{0.0};
    double m_min{0.0};
    double m_max{100.0};
    double m_increment{1.0};

    int m_spin_value{0};

    bool   m_snap_to_ticks{false};

private:
    // Implement pure virtual function inherited from wxCompositeWindow.
    wxWindowList GetCompositeWindowParts() const override;

    wxDECLARE_EVENT_TABLE();
};

#else // !wxUSE_SPINBTN

#define wxSPINCTRL_GETVALUE_FIX int = 1

// ----------------------------------------------------------------------------
// wxSpinCtrl is just a text control
// ----------------------------------------------------------------------------

#include "wx/textctrl.h"

class wxSpinCtrlGenericBase : public wxTextCtrl
{
public:
    wxSpinCtrlGenericBase() : m_value(0), m_min(0), m_max(100),
                              m_increment(1), m_snap_to_ticks(false),
                              m_format("%g") { }

    bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                std::string_view value = {},
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxSP_ARROW_KEYS,
                double min = 0, double max = 100, double initial = 0,
                double inc = 1,
                std::string_view name = std::string_view{"wxSpinCtrl"})
    {
        m_min = min;
        m_max = max;
        m_value = initial;
        m_increment = inc;

        bool ok = wxTextCtrl::Create(parent, id, value, pos, size, style,
                                     wxDefaultValidator, name);
        DoSetValue(initial, SendEvent::None);

        return ok;
    }

    // T GetValue() const
    // T GetMin() const
    // T GetMax() const
    // T GetIncrement() const
    virtual bool GetSnapToTicks() const { return m_snap_to_ticks; }
    // unsigned GetDigits() const                   - wxSpinCtrlDouble only

    // operations
    virtual void SetValue(const wxString& text) { wxTextCtrl::SetValue(text); }
    // void SetValue(T val)
    // void SetRange(T minVal, T maxVal)
    // void SetIncrement(T inc)
    virtual void SetSnapToTicks(bool snap_to_ticks)
        { m_snap_to_ticks = snap_to_ticks; }
    // void SetDigits(unsigned digits)              - wxSpinCtrlDouble only

    // Select text in the textctrl
    //void SetSelection(long from, long to);

protected:
    // generic double valued
    double DoGetValue() const
    {
        double n;
        if ( (wxSscanf(wxTextCtrl::GetValue(), "%lf", &n) != 1) )
            n = std::numeric_limits<int>::min();

        return n;
    }

    bool DoSetValue(double val, SendEvent sendEvent)
    {
        wxString str(wxString::Format(m_format, val));
        switch ( sendEvent )
        {
            case SendEvent::None:
                wxTextCtrl::ChangeValue(str);
                break;

            case SendEvent::Text:
                wxTextCtrl::SetValue(str);
                break;
        }

        return true;
    }
    void DoSetRange(double min_val, double max_val)
    {
        m_min = min_val;
        m_max = max_val;
    }
    void DoSetIncrement(double inc) { m_increment = inc; } // Note: unused

    wxString m_format;

    double m_value;
    double m_min;
    double m_max;
    double m_increment;
    
    bool   m_snap_to_ticks;
};

#endif // wxUSE_SPINBTN/!wxUSE_SPINBTN

#if !defined(wxHAS_NATIVE_SPINCTRL)

//-----------------------------------------------------------------------------
// wxSpinCtrl
//-----------------------------------------------------------------------------

class wxSpinCtrl : public wxSpinCtrlGenericBase
{
public:
    wxSpinCtrl() = default;
    wxSpinCtrl(wxWindow *parent,
               wxWindowID id = wxID_ANY,
               std::string_view value = {},
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               unsigned int style = wxSP_ARROW_KEYS,
               int min = 0, int max = 100, int initial = 0,
               std::string_view name = std::string_view{"wxSpinCtrl"})
    {
        Create(parent, id, value, pos, size, style, min, max, initial, name);
    }

    bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                std::string_view value = {},
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxSP_ARROW_KEYS,
                int min = 0, int max = 100, int initial = 0,
                std::string_view name = std::string_view{"wxSpinCtrl"})
    {
        return wxSpinCtrlGenericBase::Create(parent, id, value, pos, size,
                                             style, min, max, initial, 1, name);
    }

    int GetValue(wxSPINCTRL_GETVALUE_FIX) const { return int(DoGetValue()); }
    int GetMin() const { return int(m_min); }
    int GetMax() const { return int(m_max); }
    int GetIncrement() const { return int(m_increment); }

    void SetValue(const wxString& value) override
        { wxSpinCtrlGenericBase::SetValue(value); }
    void SetValue( int value )              { DoSetValue(value, SendEvent::None); }
    void SetRange( int minVal, int maxVal ) { DoSetRange(minVal, maxVal); }
    void SetIncrement(int inc) { DoSetIncrement(inc); }

    int GetBase() const override { return m_base; }
    bool SetBase(int base) override;

protected:
    void DoSendEvent() override;

    bool DoTextToValue(const wxString& text, double *val) override;
    std::string DoValueToText(double val) override;
    void ResetTextValidator() override;

private:
    int m_base{10};
};

#endif // wxHAS_NATIVE_SPINCTRL

//-----------------------------------------------------------------------------
// wxSpinCtrlDouble
//-----------------------------------------------------------------------------

class wxSpinCtrlDouble : public wxSpinCtrlGenericBase
{
public:
    wxSpinCtrlDouble() = default;
    wxSpinCtrlDouble(wxWindow *parent,
                     wxWindowID id = wxID_ANY,
                     std::string_view value = {},
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     unsigned int style = wxSP_ARROW_KEYS,
                     double min = 0, double max = 100, double initial = 0,
                     double inc = 1,
                     std::string_view name = "wxSpinCtrlDouble")
    {
        Create(parent, id, value, pos, size, style,
               min, max, initial, inc, name);
    }

    bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                std::string_view value = {},
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxSP_ARROW_KEYS,
                double min = 0, double max = 100, double initial = 0,
                double inc = 1,
                std::string_view name = std::string_view{"wxSpinCtrlDouble"})
    {
        DetermineDigits(inc);
        return wxSpinCtrlGenericBase::Create(parent, id, value, pos, size,
                                             style, min, max, initial,
                                             inc, name);
    }

    double GetValue(wxSPINCTRL_GETVALUE_FIX) const { return DoGetValue(); }
    double GetMin() const { return m_min; }
    double GetMax() const { return m_max; }
    double GetIncrement() const { return m_increment; }
    unsigned GetDigits() const { return m_digits; }

    // operations
    void SetValue(std::string_view value) override
        { wxSpinCtrlGenericBase::SetValue(value); }
    void SetValue(double value)                 { DoSetValue(value, SendEvent::None); }
    void SetRange(double minVal, double maxVal) { DoSetRange(minVal, maxVal); }
    void SetIncrement(double inc)               { DoSetIncrement(inc); }
    void SetDigits(unsigned digits);

    // We don't implement bases support for floating point numbers, this is not
    // very useful in practice.
    int GetBase() const override { return 10; }
    bool SetBase([[maybe_unused]] int base) override { return false; }

private:
    wxString m_format{"%0.0f"};

protected:
    void DoSendEvent() override;

    bool DoTextToValue(const wxString& text, double *val) override;
    std::string DoValueToText(double val) override;
    void ResetTextValidator() override;
    void DetermineDigits(double inc);

    unsigned m_digits{0};
};

#endif // _WX_GENERIC_SPINCTRL_H_
