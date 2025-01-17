/////////////////////////////////////////////////////////////////////////////
// Name:        wx/gtk/checkbox.h
// Purpose:
// Author:      Robert Roebling
// Copyright:   (c) 1998 Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GTKCHECKBOX_H_
#define _WX_GTKCHECKBOX_H_

// ----------------------------------------------------------------------------
// wxCheckBox
// ----------------------------------------------------------------------------

class wxCheckBox : public wxCheckBoxBase
{
public:
    wxCheckBox();
    ~wxCheckBox();
    wxCheckBox( wxWindow *parent, wxWindowID id, const wxString& label,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize, long style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxASCII_STR(wxCheckBoxNameStr))
    {
        Create(parent, id, label, pos, size, style, validator, name);
    }
    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& label,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxASCII_STR(wxCheckBoxNameStr) );

    void SetValue( bool state ) override;
    bool GetValue() const override;

    void SetLabel( const wxString& label ) override;

    static wxVisualAttributes
    GetClassDefaultAttributes(wxWindowVariant variant = wxWINDOW_VARIANT_NORMAL);

    // implementation
    void GTKDisableEvents();
    void GTKEnableEvents();

protected:
    void DoApplyWidgetStyle(GtkRcStyle *style) override;
    GdkWindow *GTKGetWindow(wxArrayGdkWindows& windows) const override;

    void DoEnable(bool enable) override;

    void DoSet3StateValue(wxCheckBoxState state) override;
    wxCheckBoxState DoGet3StateValue() const override;

private:
    typedef wxCheckBoxBase base_type;

    GtkWidget *m_widgetCheckbox;
    GtkWidget *m_widgetLabel;

    wxDECLARE_DYNAMIC_CLASS(wxCheckBox);
};

#endif // _WX_GTKCHECKBOX_H_
