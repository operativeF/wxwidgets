/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/slider.h
// Purpose:     wxSlider class implementation using trackbar control
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SLIDER_H_
#define _WX_SLIDER_H_

import Utils.Geometry;

import <string>;

#include <fmt/core.h>

class wxSubwindows;

class wxSlider : public wxSliderBase
{
public:
    wxSlider() = default;

    wxSlider(wxWindow *parent,
             wxWindowID id,
             int value,
             int minValue,
             int maxValue,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             unsigned int style = wxSL_HORIZONTAL,
             const wxValidator& validator = wxDefaultValidator,
             std::string_view name = wxSliderNameStr)
    {
        Create(parent, id, value, minValue, maxValue,
                     pos, size, style, validator, name);
    }

    wxSlider& operator=(wxSlider&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                int value,
                int minValue, int maxValue,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxSL_HORIZONTAL,
                const wxValidator& validator = wxDefaultValidator,
                std::string_view name = wxSliderNameStr);

    ~wxSlider();

    // slider methods
    int GetValue() const override;
    void SetValue(int) override;

    void SetRange(int minValue, int maxValue) override;

    int GetMin() const override { return m_rangeMin; }
    int GetMax() const override { return m_rangeMax; }

    // Win32-specific slider methods
    int GetTickFreq() const override { return m_tickFreq; }
    void SetPageSize(int pageSize) override;
    int GetPageSize() const override;
    void ClearSel() override;
    void ClearTicks() override;
    void SetLineSize(int lineSize) override;
    int GetLineSize() const override;
    int GetSelEnd() const override;
    int GetSelStart() const override;
    void SetSelection(int minPos, int maxPos) override;
    void SetThumbLength(int len) override;
    int GetThumbLength() const override;
    void SetTick(int tickPos) override;

    // implementation only from now on
    WXHWND GetStaticMin() const;
    WXHWND GetStaticMax() const;
    WXHWND GetEditValue() const;
    bool ContainsHWND(WXHWND hWnd) const override;

    // we should let background show through the slider (and its labels)
    bool HasTransparentBackground() override { return true; }

    // returns true if the platform should explicitly apply a theme border
    bool CanApplyThemeBorder() const override { return false; }

    void Command(wxCommandEvent& event) override;
    bool MSWOnScroll(int orientation, WXWORD wParam,
                             WXWORD pos, WXHWND control) override;

    bool Show(bool show = true) override;
    bool Enable(bool show = true) override;
    bool SetFont(const wxFont& font) override;
    bool SetForegroundColour(const wxColour& colour) override;
    bool SetBackgroundColour(const wxColour& colour) override;

    DWORD MSWGetStyle(unsigned int flags, DWORD *exstyle = nullptr) const override;

protected:
    // format an integer value as string
    static std::string Format(int n) { return fmt::format("{:d}", n); }

    // get the boundig box for the slider and possible labels
    wxRect GetBoundingBox() const;

    // Get the height and, if the pointers are non NULL, widths of both labels.
    //
    // Notice that the return value will be 0 if we don't have wxSL_LABELS
    // style but we do fill widthMin and widthMax even if we don't have
    // wxSL_MIN_MAX_LABELS style set so the caller should account for it.
    int GetLabelsSize(int *widthMin = nullptr, int *widthMax = nullptr) const;


    // overridden base class virtuals
    wxPoint DoGetPosition() const override;
    wxSize DoGetSize() const override;
    void DoMoveWindow(wxRect boundary) override;
    wxSize DoGetBestSize() const override;

    WXHBRUSH DoMSWControlColor(WXHDC pDC, wxColour colBg, WXHWND hWnd) override;

    void MSWUpdateFontOnDPIChange(const wxSize& newDPI) override;

    void OnDPIChanged(wxDPIChangedEvent& event);

private:
    // the labels windows, if any
    wxSubwindows  *m_labels{nullptr};

    // Last background brush we returned from DoMSWControlColor(), see there.
    WXHBRUSH m_hBrushBg{nullptr};

    int           m_rangeMin{0};
    int           m_rangeMax{0};
    int           m_pageSize{1};
    int           m_lineSize{1};
    int           m_tickFreq{0};

    // flag needed to detect whether we're getting THUMBRELEASE event because
    // of dragging the thumb or scrolling the mouse wheel
    bool m_isDragging{false};

    // Platform-specific implementation of SetTickFreq
    void DoSetTickFreq(int freq) override;

public:
	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
};

#endif // _WX_SLIDER_H_

