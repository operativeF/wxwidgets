/////////////////////////////////////////////////////////////////////////////
// Name:        include/wx/msw/taskbarbutton.h
// Purpose:     Defines wxTaskBarButtonImpl class.
// Author:      Chaobin Zhang <zhchbin@gmail.com>
// Created:     2014-06-01
// Copyright:   (c) 2014 wxWidgets development team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef  _WX_MSW_TASKBARBUTTON_H_
#define  _WX_MSW_TASKBARBUTTON_H_

#include "wx/defs.h"

#if wxUSE_TASKBARBUTTON

#include <vector>
#include "wx/taskbarbutton.h"

class WXDLLIMPEXP_FWD_CORE wxWindow;
class WXDLLIMPEXP_FWD_CORE wxITaskbarList3;

class WXDLLIMPEXP_CORE wxTaskBarButtonImpl : public wxTaskBarButton
{
public:
    ~wxTaskBarButtonImpl();

    wxTaskBarButtonImpl(const wxTaskBarButtonImpl&) = delete;
    wxTaskBarButtonImpl& operator=(const wxTaskBarButtonImpl&) = delete;
    wxTaskBarButtonImpl(wxTaskBarButtonImpl&&) = default;
    wxTaskBarButtonImpl& operator=(wxTaskBarButtonImpl&&) = default;
    
    void SetProgressRange(int range) override;
    void SetProgressValue(int value) override;
    void PulseProgress() override;
    void Show(bool show = true) override;
    void Hide() override;
    void SetThumbnailTooltip(const std::string& tooltip) override;
    void SetProgressState(wxTaskBarButtonState state) override;
    void SetOverlayIcon(const wxIcon& icon,
        const std::string& description = {}) override;
    void SetThumbnailClip(const wxRect& rect) override;
    void SetThumbnailContents(const wxWindow *child) override;
    bool InsertThumbBarButton(size_t pos,
                                      wxThumbBarButton *button) override;
    bool AppendThumbBarButton(wxThumbBarButton *button) override;
    bool AppendSeparatorInThumbBar() override;
    wxThumbBarButton* RemoveThumbBarButton(
        wxThumbBarButton *button) override;
    wxThumbBarButton* RemoveThumbBarButton(int id) override;
    wxThumbBarButton* GetThumbBarButtonByIndex(size_t index);
    bool InitOrUpdateThumbBarButtons();
    void Realize() override;

private:
    // This ctor is only used by wxTaskBarButton::New()
    wxTaskBarButtonImpl(wxITaskbarList3* taskbarList, wxWindow* parent);

    wxWindow* m_parent;
    wxITaskbarList3 *m_taskbarList;

    using wxThumbBarButtons = std::vector<wxThumbBarButton *>;
    wxThumbBarButtons m_thumbBarButtons;

    int m_progressRange;
    int m_progressValue;
    wxTaskBarButtonState m_progressState;
    std::string m_thumbnailTooltip;
    wxIcon m_overlayIcon;
    std::string m_overlayIconDescription;
    wxRect m_thumbnailClipRect;
    bool m_hasInitThumbnailToolbar;

    friend wxTaskBarButton* wxTaskBarButton::New(wxWindow*);
};

#endif // wxUSE_TASKBARBUTTON

#endif  // _WX_MSW_TASKBARBUTTON_H_
