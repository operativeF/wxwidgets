///////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/timectrl.h
// Purpose:     Declaration of wxOSX-specific wxTimePickerCtrl class.
// Author:      Vadim Zeitlin
// Created:     2011-12-18
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_OSX_TIMECTRL_H_
#define _WX_OSX_TIMECTRL_H_

// ----------------------------------------------------------------------------
// wxTimePickerCtrl
// ----------------------------------------------------------------------------

class wxTimePickerCtrl : public wxTimePickerCtrlBase
{
public:
    // Constructors.
    wxTimePickerCtrl() { }

    wxTimePickerCtrl(wxWindow *parent,
                     wxWindowID id,
                     const wxDateTime& dt = wxDefaultDateTime,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = wxTP_DEFAULT,
                     const wxValidator& validator = wxDefaultValidator,
                     const wxString& name = wxTimePickerCtrlNameStr)
    {
        Create(parent, id, dt, pos, size, style, validator, name);
    }

	wxTimePickerCtrl(const wxTimePickerCtrl&) = delete;
	wxTimePickerCtrl& operator=(const wxTimePickerCtrl&) = delete;

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxDateTime& dt = wxDefaultDateTime,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxTP_DEFAULT,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxTimePickerCtrlNameStr);

    void OSXGenerateEvent(const wxDateTime& dt) override;

public:
	wxClassInfo *GetClassInfo() const;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_OSX_TIMECTRL_H_
