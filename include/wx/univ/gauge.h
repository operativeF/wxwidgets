///////////////////////////////////////////////////////////////////////////////
// Name:        wx/univ/gauge.h
// Purpose:     wxUniversal wxGauge declaration
// Author:      Vadim Zeitlin
// Modified by:
// Created:     20.02.01
// Copyright:   (c) 2001 SciTech Software, Inc. (www.scitechsoft.com)
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_UNIV_GAUGE_H_
#define _WX_UNIV_GAUGE_H_

// ----------------------------------------------------------------------------
// wxGauge: a progress bar
// ----------------------------------------------------------------------------

class wxGauge : public wxGaugeBase
{
public:
    wxGauge() { Init(); }

    wxGauge(wxWindow *parent,
            wxWindowID id,
            int range,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            unsigned int style = wxGA_HORIZONTAL,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxASCII_STR(wxGaugeNameStr))
    {
        Init();

        (void)Create(parent, id, range, pos, size, style, validator, name);
    }

    bool Create(wxWindow *parent,
                wxWindowID id,
                int range,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxGA_HORIZONTAL,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxASCII_STR(wxGaugeNameStr));

    // implement base class virtuals
    void SetRange(int range) override;
    void SetValue(int pos) override;

    // wxUniv-specific methods

    // is it a smooth progress bar or a discrete one?
    bool IsSmooth() const { return (wxGetWindowStyle() & wxGA_SMOOTH) != 0; }

    // is it a vertica; progress bar or a horizontal one?
    bool IsVertical() const { return (wxGetWindowStyle() & wxGA_VERTICAL) != 0; }

protected:
    // common part of all ctors
    void Init();

    // return the def border for a progress bar
    wxBorder GetDefaultBorder() const override;

    // return the default size
    wxSize DoGetBestClientSize() const override;

    // draw the control
    void DoDraw(wxControlRenderer *renderer) override;

    wxDECLARE_DYNAMIC_CLASS(wxGauge);
};

#endif // _WX_UNIV_GAUGE_H_
