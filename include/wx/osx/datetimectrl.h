///////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/datetimectrl.h
// Purpose:     Declaration of wxOSX-specific wxDateTimePickerCtrl class.
// Author:      Vadim Zeitlin
// Created:     2011-12-18
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_OSX_DATETIMECTRL_H_
#define _WX_OSX_DATETIMECTRL_H_

class wxDateTimeWidgetImpl;

// ----------------------------------------------------------------------------
// wxDateTimePickerCtrl
// ----------------------------------------------------------------------------

class wxDateTimePickerCtrl : public wxDateTimePickerCtrlBase
{
public:
    // Implement the base class pure virtuals.
    void SetValue(const wxDateTime& dt) override;
    wxDateTime GetValue() const override;

    // Implementation only.
    virtual void OSXGenerateEvent(const wxDateTime& dt) = 0;

protected:
    wxDateTimeWidgetImpl* GetDateTimePeer() const;
};

#endif // _WX_OSX_DATETIMECTRL_H_
