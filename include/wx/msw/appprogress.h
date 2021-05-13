/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/appprogress.h
// Purpose:     wxAppProgressIndicator interface.
// Author:      Chaobin Zhang <zhchbin@gmail.com>
// Created:     2014-09-05
// Copyright:   (c) 2014 wxWidgets development team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_APPPROG_H_
#define _WX_MSW_APPPROG_H_

#include <vector>

class WXDLLIMPEXP_FWD_CORE wxTaskBarButton;

class WXDLLIMPEXP_CORE wxAppProgressIndicator
    : public wxAppProgressIndicatorBase
{
public:
    wxAppProgressIndicator(wxWindow* parent = NULL, int maxValue = 100);
    virtual ~wxAppProgressIndicator();

    bool IsAvailable() const override;

    void SetValue(int value) override;
    void SetRange(int range) override;
    void Pulse() override;
    void Reset() override;

private:
    int m_maxValue;

    std::vector<wxTaskBarButton*> m_taskBarButtons;

    wxAppProgressIndicator(const wxAppProgressIndicator&) = delete;
	wxAppProgressIndicator& operator=(const wxAppProgressIndicator&) = delete;
};

#endif  // _WX_MSW_APPPROG_H_
