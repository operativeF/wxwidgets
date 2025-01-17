/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/toolbar.h
// Purpose:     wxToolBar class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_TBAR95_H_
#define _WX_MSW_TBAR95_H_

#if wxUSE_TOOLBAR

#include "wx/imaglist.h"

import WX.WinDef;

import <string>;

class wxBitmap;

class wxToolBar : public wxToolBarBase
{
public:
    wxToolBar() = default;

    wxToolBar(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxTB_DEFAULT_STYLE,
                std::string_view name = wxToolBarNameStr)
    {
        Create(parent, id, pos, size, style, name);
    }

    wxToolBar& operator=(wxToolBar&&) = delete;

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = wxTB_DEFAULT_STYLE,
                std::string_view name = wxToolBarNameStr);

    ~wxToolBar();

    // override/implement base class virtuals
    wxToolBarToolBase *FindToolForPosition(wxCoord x, wxCoord y) const override;

    bool Realize() override;

    void SetToolBitmapSize(const wxSize& size) override;
    wxSize GetToolSize() const override;

    void SetRows(int nRows) override;

    void SetToolNormalBitmap(int id, const wxBitmap& bitmap) override;
    void SetToolDisabledBitmap(int id, const wxBitmap& bitmap) override;

    void SetToolPacking(unsigned int packing) override;

    // implementation only from now on
    // -------------------------------

    void SetWindowStyleFlag(unsigned int style) override;

    bool MSWCommand(WXUINT param, WXWORD id) override;
    bool MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM *result) override;

    void OnMouseEvent(wxMouseEvent& event);
    void OnSysColourChanged(wxSysColourChangedEvent& event);
    void OnEraseBackground(wxEraseEvent& event);

    void SetFocus() override {}

    static WXHBITMAP MapBitmap(WXHBITMAP bitmap, int width, int height);

    // override WndProc mainly to process WM_SIZE
    WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam) override;

    WXDWORD MSWGetStyle(unsigned int style, WXDWORD *exstyle) const override;

    // returns true if the platform should explicitly apply a theme border
    bool CanApplyThemeBorder() const override { return false; }

#ifdef wxHAS_MSW_BACKGROUND_ERASE_HOOK
    bool MSWEraseBgHook(WXHDC hDC) override;
    WXHBRUSH MSWGetBgBrushForChild(WXHDC hDC, wxWindowMSW *child) override;
#endif // wxHAS_MSW_BACKGROUND_ERASE_HOOK

    wxToolBarToolBase *CreateTool(int id,
                                          const std::string& label,
                                          const wxBitmap& bmpNormal,
                                          const wxBitmap& bmpDisabled = wxNullBitmap,
                                          wxItemKind kind = wxITEM_NORMAL,
                                          wxObject *clientData = nullptr,
                                          const std::string& shortHelp = {},
                                          const std::string& longHelp = {}) override;

    wxToolBarToolBase *CreateTool(wxControl *control,
                                          const std::string& label) override;
protected:
    // create the native toolbar control
    bool MSWCreateToolbar(const wxPoint& pos, const wxSize& size);

    // recreate the control completely
    void Recreate();

    bool DoInsertTool(size_t pos, wxToolBarToolBase *tool) override;
    bool DoDeleteTool(size_t pos, wxToolBarToolBase *tool) override;

    void DoEnableTool(wxToolBarToolBase *tool, bool enable) override;
    void DoToggleTool(wxToolBarToolBase *tool, bool toggle) override;
    void DoSetToggle(wxToolBarToolBase *tool, bool toggle) override;

    // return the appropriate size and flags for the toolbar control
    wxSize DoGetBestSize() const override;

    // handlers for various events
    bool HandleSize(WXWPARAM wParam, WXLPARAM lParam);
#ifdef wxHAS_MSW_BACKGROUND_ERASE_HOOK
    bool HandlePaint(WXWPARAM wParam, WXLPARAM lParam);
#endif // wxHAS_MSW_BACKGROUND_ERASE_HOOK
    void HandleMouseMove(WXWPARAM wParam, WXLPARAM lParam);

    // should be called whenever the toolbar size changes
    void UpdateSize();

    // create m_disabledImgList (but doesn't fill it), set it to NULL if it is
    // unneeded
    void CreateDisabledImageList();

    // get the Windows toolbar style of this control
    long GetMSWToolbarStyle() const;

    // set native toolbar padding
    void MSWSetPadding(WXWORD padding);

    void RealizeHelper();
    void OnDPIChanged(wxDPIChangedEvent& event);

private:
    // the big bitmap containing all bitmaps of the toolbar buttons
    WXHBITMAP m_hBitmap{nullptr};

    // the image list with disabled images, may be NULL if we use
    // system-provided versions of them
    wxImageList *m_disabledImgList{nullptr};

    // the tool the cursor is in
    wxToolBarToolBase *m_pInTool{nullptr};

    // the total number of toolbar elements
    size_t m_nButtons{0};

    // the sum of the sizes of the fixed items (i.e. excluding stretchable
    // spaces) in the toolbar direction
    int m_totalFixedSize{0};

    // makes sure tool bitmap size is sufficient for all tools
    void AdjustToolBitmapSize();

    // update the sizes of stretchable spacers to consume all extra space we
    // have
    void UpdateStretchableSpacersSize();

#ifdef wxHAS_MSW_BACKGROUND_ERASE_HOOK
    // do erase the toolbar background, always do it for the entire control as
    // the caller sets the clipping region correctly to exclude parts which
    // should not be erased
    void MSWDoEraseBackground(WXHDC hDC);

    // return the brush to use for erasing the toolbar background
    WXHBRUSH MSWGetToolbarBgBrush();
#endif // wxHAS_MSW_BACKGROUND_ERASE_HOOK

    // Return true if we're showing the labels for the embedded controls: we
    // only do it if text is enabled and, somewhat less expectedly, if icons
    // are enabled too because showing both the control and its label when only
    // text is shown for the other buttons is too inconsistent to be useful.
    bool AreControlLabelsShown() const
    {
        return HasFlag(wxTB_TEXT) && !HasFlag(wxTB_NOICONS);
    }

    // Return the size required to accommodate the given tool which must be of
    // "control" type.
    wxSize MSWGetFittingtSizeForControl(class wxToolBarTool* tool) const;

    wxDECLARE_EVENT_TABLE();
};

#endif // wxUSE_TOOLBAR

#endif // _WX_MSW_TBAR95_H_

