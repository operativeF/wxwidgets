///////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/datectrl.h
// Purpose:     Declaration of wxOSX-specific wxDatePickerCtrl class.
// Author:      Vadim Zeitlin
// Created:     2011-12-18
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_OSX_DATECTRL_H_
#define _WX_OSX_DATECTRL_H_

// ----------------------------------------------------------------------------
// wxDatePickerCtrl
// ----------------------------------------------------------------------------

class wxDatePickerCtrl : public wxDatePickerCtrlBase
{
public:
    // Constructors.
    wxDatePickerCtrl() { }

    wxDatePickerCtrl(wxWindow *parent,
                     wxWindowID id,
                     const wxDateTime& dt = wxDefaultDateTime,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = wxDP_DEFAULT | wxDP_SHOWCENTURY,
                     const wxValidator& validator = wxDefaultValidator,
                     const wxString& name = wxDatePickerCtrlNameStr)
    {
        Create(parent, id, dt, pos, size, style, validator, name);
    }

	wxDatePickerCtrl(const wxDatePickerCtrl&) = delete;
	wxDatePickerCtrl& operator=(const wxDatePickerCtrl&) = delete;

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxDateTime& dt = wxDefaultDateTime,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxDP_DEFAULT | wxDP_SHOWCENTURY,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxDatePickerCtrlNameStr);

    // Implement the base class pure virtuals.
    void SetRange(const wxDateTime& dt1, const wxDateTime& dt2) override;
    bool GetRange(wxDateTime *dt1, wxDateTime *dt2) const override;

    void OSXGenerateEvent(const wxDateTime& dt) override;

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_OSX_DATECTRL_H_
