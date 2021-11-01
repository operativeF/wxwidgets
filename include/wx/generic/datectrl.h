/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/datectrl.h
// Purpose:     generic wxDatePickerCtrl implementation
// Author:      Andreas Pflug
// Modified by:
// Created:     2005-01-19
// Copyright:   (c) 2005 Andreas Pflug <pgadmin@pse-consulting.de>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_DATECTRL_H_
#define _WX_GENERIC_DATECTRL_H_

#include "wx/compositewin.h"
#include "wx/containr.h"

class wxComboCtrl;

class wxCalendarCtrl;
class wxCalendarComboPopup;

using wxDatePickerCtrlGenericBase = wxDatePickerCtrlCommonBase<wxDateTimePickerCtrlBase>;

class wxDatePickerCtrlGeneric
    : public wxCompositeWindow< wxNavigationEnabled<wxDatePickerCtrlGenericBase> >
{
public:
    // creating the control
    wxDatePickerCtrlGeneric() = default;

    wxDatePickerCtrlGeneric(wxWindow *parent,
                            wxWindowID id,
                            const wxDateTime& date = wxDefaultDateTime,
                            const wxPoint& pos = wxDefaultPosition,
                            const wxSize& size = wxDefaultSize,
                            unsigned int style = wxDP_DEFAULT | wxDP_SHOWCENTURY,
                            const wxValidator& validator = wxDefaultValidator,
                            const std::string& name = wxDatePickerCtrlNameStr)
    {
        Create(parent, id, date, pos, size, style, validator, name);
    }

	wxDatePickerCtrlGeneric& operator=(wxDatePickerCtrlGeneric&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const wxDateTime& date = wxDefaultDateTime,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxDP_DEFAULT | wxDP_SHOWCENTURY,
                const wxValidator& validator = wxDefaultValidator,
                const std::string& name = wxDatePickerCtrlNameStr);

    // wxDatePickerCtrl methods
    void SetValue(const wxDateTime& date) override;
    wxDateTime GetValue() const override;

    bool GetRange(wxDateTime *dt1, wxDateTime *dt2) const override;
    void SetRange(const wxDateTime &dt1, const wxDateTime &dt2) override;

    bool SetDateRange(const wxDateTime& lowerdate = wxDefaultDateTime,
                      const wxDateTime& upperdate = wxDefaultDateTime);

    // extra methods available only in this (generic) implementation
    wxCalendarCtrl *GetCalendar() const;


    // implementation only from now on
    // -------------------------------

    // overridden base class methods
    bool Destroy() override;

protected:
    wxSize DoGetBestSize() const override;

private:
    

    // return the list of the windows composing this one
    wxWindowList GetCompositeWindowParts() const override;

    void OnText(wxCommandEvent &event);
    void OnSize(wxSizeEvent& event);

    wxComboCtrl* m_combo{nullptr};
    wxCalendarComboPopup* m_popup{nullptr};

    wxDECLARE_EVENT_TABLE();
};

#endif // _WX_GENERIC_DATECTRL_H_

