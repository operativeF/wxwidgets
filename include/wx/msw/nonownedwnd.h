///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/nonownedwnd.h
// Purpose:     wxNonOwnedWindow declaration for wxMSW.
// Author:      Vadim Zeitlin
// Created:     2011-10-09
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_NONOWNEDWND_H_
#define _WX_MSW_NONOWNEDWND_H_

class wxNonOwnedWindowShapeImpl;

// ----------------------------------------------------------------------------
// wxNonOwnedWindow
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxNonOwnedWindow : public wxNonOwnedWindowBase
{
public:
    wxNonOwnedWindow();
    ~wxNonOwnedWindow() override;

    wxNonOwnedWindow(const wxNonOwnedWindow&) = delete;
    wxNonOwnedWindow& operator=(const wxNonOwnedWindow&) = delete;
    wxNonOwnedWindow(wxNonOwnedWindow&&) = default;
    wxNonOwnedWindow& operator=(wxNonOwnedWindow&&) = default;
    
    bool Reparent(wxWindowBase* newParent) override;
    void InheritAttributes() override;
    bool IsThisEnabled() const override;

protected:
    bool DoClearShape() override;
    bool DoSetRegionShape(const wxRegion& region) override;
#if wxUSE_GRAPHICS_CONTEXT
    bool DoSetPathShape(const wxGraphicsPath& path) override;
#endif // wxUSE_GRAPHICS_CONTEXT

    WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam) override;

private:
    bool HandleDPIChange(const wxSize& newDPI, const wxRect& newRect);

#if wxUSE_GRAPHICS_CONTEXT
    wxNonOwnedWindowShapeImpl* m_shapeImpl{nullptr};
#endif // wxUSE_GRAPHICS_CONTEXT

    // Keep track of the DPI used in this window. So when per-monitor dpi
    // awareness is enabled, both old and new DPI are known for
    // wxDPIChangedEvent and wxWindow::MSWUpdateOnDPIChange.
    wxSize m_activeDPI{wxDefaultSize};

    // This window supports handling per-monitor DPI awareness when the
    // application manifest contains <dpiAwareness>PerMonitorV2</dpiAwareness>.
    bool m_perMonitorDPIaware{false};
};

#endif // _WX_MSW_NONOWNEDWND_H_
