/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/gauge.h
// Purpose:     wxGauge class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GAUGE_H_
#define _WX_GAUGE_H_

#include "wx/control.h"

// Group box
class wxGauge: public wxGaugeBase
{
 public:
  wxGauge() { }

  wxGauge(wxWindow *parent, wxWindowID id,
           int range,
           const wxPoint& pos = wxDefaultPosition,
           const wxSize& size = wxDefaultSize,
           long style = wxGA_HORIZONTAL,
           const wxValidator& validator = wxDefaultValidator,
           const wxString& name = wxASCII_STR(wxGaugeNameStr))
  {
    Create(parent, id, range, pos, size, style, validator, name);
  }

	wxGauge(const wxGauge&) = delete;
	wxGauge& operator=(const wxGauge&) = delete;

  bool Create(wxWindow *parent, wxWindowID id,
           int range,
           const wxPoint& pos = wxDefaultPosition,
           const wxSize& size = wxDefaultSize,
           long style = wxGA_HORIZONTAL,
           const wxValidator& validator = wxDefaultValidator,
           const wxString& name = wxASCII_STR(wxGaugeNameStr));

    // set gauge range/value
    void SetRange(int range) override;
    void SetValue(int pos) override;
    int  GetValue() const  override;

    void Pulse() override;

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif
    // _WX_GAUGE_H_
