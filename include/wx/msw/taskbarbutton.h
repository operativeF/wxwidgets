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

#if wxUSE_TASKBARBUTTON

import Utils.Geometry;

import <vector>;
#include "wx/taskbarbutton.h"

class wxWindow;
class wxITaskbarList3;

class wxTaskBarButtonImpl : public wxTaskBarButton
{
public:
    ~wxTaskBarButtonImpl();

    wxTaskBarButtonImpl& operator=(wxTaskBarButtonImpl&&) = delete;
    
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

    // This ctor is only used by wxTaskBarButton::Create()
    // TODO: In public access because unique_ptr doesn't play well (?)
    wxTaskBarButtonImpl(wxITaskbarList3* taskbarList, wxWindow* parent);
private:
    std::string m_thumbnailTooltip;
    std::string m_overlayIconDescription;

    using wxThumbBarButtons = std::vector<wxThumbBarButton *>;
    wxThumbBarButtons m_thumbBarButtons;

    wxIcon m_overlayIcon;
    wxRect m_thumbnailClipRect;

    wxWindow* m_parent;
    wxITaskbarList3 *m_taskbarList;

    int m_progressRange;
    int m_progressValue;
    
    wxTaskBarButtonState m_progressState;

    bool m_hasInitThumbnailToolbar;

    // TODO: This doesn't work, so the corresponding constructor is in public access.
    friend std::unique_ptr<wxTaskBarButton> wxTaskBarButton::Create(wxWindow*);
};

#endif // wxUSE_TASKBARBUTTON

#endif  // _WX_MSW_TASKBARBUTTON_H_
