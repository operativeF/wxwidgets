///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/statusbar.h
// Purpose:     native implementation of wxStatusBar
// Author:      Vadim Zeitlin
// Modified by:
// Created:     04.04.98
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_STATUSBAR_H_
#define _WX_MSW_STATUSBAR_H_

#if wxUSE_NATIVE_STATUSBAR

import WX.WinDef;
import Utils.Geometry;

import <vector>;

class wxClientDC;
class wxToolTip;

class wxStatusBar : public wxStatusBarBase
{
public:
    
    wxStatusBar();
    wxStatusBar(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                unsigned int style = wxSTB_DEFAULT_STYLE,
                std::string_view name = wxStatusBarNameStr)
    {
        Create(parent, id, style, name);
    }

    [[maybe_unused]] bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                unsigned int style = wxSTB_DEFAULT_STYLE,
                std::string_view name = wxStatusBarNameStr);

    ~wxStatusBar();

    wxStatusBar& operator=(wxStatusBar&&) = delete;

    // implement base class methods
    void SetFieldsCount(int number = 1, const int *widths = nullptr) override;
    void SetStatusWidths(int n, const int widths_field[]) override;
    void SetStatusStyles(int n, const int styles[]) override;
    void SetMinHeight(int height) override;
    bool GetFieldRect(int i, wxRect& rect) const override;

    int GetBorderX() const override;
    int GetBorderY() const override;

    // override some wxWindow virtual methods too
    bool SetFont(const wxFont& font) override;

    WXLRESULT MSWWindowProc(WXUINT nMsg,
                                    WXWPARAM wParam,
                                    WXLPARAM lParam) override;

protected:
    // implement base class pure virtual method
    void DoUpdateStatusText(int number) override;

    // override some base class virtuals
    WXDWORD MSWGetStyle(unsigned int flags, WXDWORD *exstyle = nullptr) const override;
    wxSize DoGetBestSize() const override;
    void DoMoveWindow(wxRect boundary) override;
#if wxUSE_TOOLTIPS
    bool MSWProcessMessage(WXMSG* pMsg) override;
    bool MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM* result) override;
#endif

    // implementation of the public SetStatusWidths()
    void MSWUpdateFieldsWidths();

    void MSWUpdateFontOnDPIChange(const wxSize& newDPI) override;

    // used by DoUpdateStatusText()

private:
    struct MSWBorders
    {
        int horz;
        int vert;
        int between;
    };

    wxClientDC *m_pDC{nullptr};

#if wxUSE_TOOLTIPS
    // the tooltips used when wxSTB_SHOW_TIPS is given
    std::vector<wxToolTip*> m_tooltips;
#endif

    // retrieve all status bar borders using SB_GETBORDERS
    MSWBorders MSWGetBorders() const;

    // return the size of the border between the fields
    int MSWGetBorderWidth() const;

    struct MSWMetrics
    {
        int gripWidth;
        int textMargin;
    };

    // return the various status bar metrics
    static const MSWMetrics& MSWGetMetrics();
};

#endif  // wxUSE_NATIVE_STATUSBAR

#endif // _WX_MSW_STATUSBAR_H_
