///////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/timectrl.h
// Purpose:     Generic implementation of wxTimePickerCtrl.
// Author:      Paul Breen, Vadim Zeitlin
// Created:     2011-09-22
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_TIMECTRL_H_
#define _WX_GENERIC_TIMECTRL_H_

#include "wx/containr.h"
#include "wx/compositewin.h"

using wxTimePickerCtrlGenericBase = wxTimePickerCtrlCommonBase<wxDateTimePickerCtrlBase>;

class wxTimePickerCtrlGeneric
    : public wxCompositeWindow< wxNavigationEnabled<wxTimePickerCtrlGenericBase> >
{
public:
    using Base = wxCompositeWindow< wxNavigationEnabled<wxTimePickerCtrlGenericBase> >;

    // Creating the control.
    wxTimePickerCtrlGeneric() { Init(); }
    ~wxTimePickerCtrlGeneric();
    wxTimePickerCtrlGeneric(wxWindow *parent,
                            wxWindowID id,
                            const wxDateTime& date = wxDefaultDateTime,
                            const wxPoint& pos = wxDefaultPosition,
                            const wxSize& size = wxDefaultSize,
                            unsigned int style = wxTP_DEFAULT,
                            const wxValidator& validator = wxDefaultValidator,
                            std::string_view name = wxTimePickerCtrlNameStr)
    {
        Init();

        (void)Create(parent, id, date, pos, size, style, validator, name);
    }

    wxTimePickerCtrlGeneric& operator=(wxTimePickerCtrlGeneric&&) = delete;

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxDateTime& date = wxDefaultDateTime,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxTP_DEFAULT,
                const wxValidator& validator = wxDefaultValidator,
                std::string_view name = wxTimePickerCtrlNameStr);

    // Implement pure virtual wxTimePickerCtrlBase methods.
    void SetValue(const wxDateTime& date) override;
    wxDateTime GetValue() const override;

protected:
    wxSize DoGetBestSize() const override;

    void DoMoveWindow(int x, int y, int width, int height) override;

private:
    void Init();

    // Return the list of the windows composing this one.
    wxWindowList GetCompositeWindowParts() const override;

    // Implementation data.
    class wxTimePickerGenericImpl* m_impl;
};

#endif // _WX_GENERIC_TIMECTRL_H_
