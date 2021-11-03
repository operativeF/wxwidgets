///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/headerctrl.h
// Purpose:     wxMSW native wxHeaderCtrl
// Author:      Vadim Zeitlin
// Created:     2008-12-01
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_HEADERCTRL_H_
#define _WX_MSW_HEADERCTRL_H_

#include "wx/compositewin.h"

#include <string>
#include <vector>

class wxMSWHeaderCtrl;

// ----------------------------------------------------------------------------
// wxHeaderCtrl
// ----------------------------------------------------------------------------

class wxHeaderCtrl : public wxCompositeWindow<wxHeaderCtrlBase>
{
public:
    wxHeaderCtrl() = default;

    wxHeaderCtrl(wxWindow *parent,
                 wxWindowID id = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 unsigned int style = wxHD_DEFAULT_STYLE,
                 const std::string& name = wxHeaderCtrlNameStr)
    {
        Create(parent, id, pos, size, style, name);
    }

    wxHeaderCtrl& operator=(wxHeaderCtrl&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxHD_DEFAULT_STYLE,
                const std::string& name = wxHeaderCtrlNameStr);

    // Window style handling.
    void SetWindowStyleFlag(unsigned int style) override;

protected:
    // Override wxWindow methods which must be implemented by a new control.
    wxSize DoGetBestSize() const override;

private:
    void DoSetCount(unsigned int count) override;
    unsigned int DoGetCount() const override;
    void DoUpdate(unsigned int idx) override;

    void DoScrollHorz(int dx) override;

    void DoSetColumnsOrder(const std::vector<int>& order) override;
    std::vector<int> DoGetColumnsOrder() const override;

    // Pure virtual method inherited from wxCompositeWindow.
    wxWindowList GetCompositeWindowParts() const override;    

    // Events.
    void OnSize(wxSizeEvent& event);

    // The native header control.
    wxMSWHeaderCtrl* m_nativeControl{nullptr};
    friend class wxMSWHeaderCtrl;
};

#endif // _WX_MSW_HEADERCTRL_H_

