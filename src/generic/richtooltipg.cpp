///////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/richtooltipg.cpp
// Purpose:     Implementation of wxRichToolTip.
// Author:      Vadim Zeitlin
// Created:     2011-10-07
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_RICHTOOLTIP

#include "wx/dcmemory.h"
#include "wx/icon.h"
#include "wx/statbmp.h"
#include "wx/stattext.h"
#include "wx/timer.h"

#include "wx/private/richtooltip.h"
#include "wx/generic/private/richtooltip.h"

#include "wx/artprov.h"
#include "wx/custombgwin.h"
#include "wx/display.h"
#include "wx/graphics.h"
#include "wx/popupwin.h"
#include "wx/textwrapper.h"

#ifdef __WXMSW__
    #if wxUSE_UXTHEME
        #include "wx/msw/uxtheme.h"
        #define HAVE_MSW_THEME
    #endif
#endif

import WX.Core.Sizer;

import WX.Utils.Settings;

import <numbers>;

// ----------------------------------------------------------------------------
// wxRichToolTipPopup: the popup window used by wxRichToolTip.
// ----------------------------------------------------------------------------

class wxRichToolTipPopup :
    public wxCustomBackgroundWindow<wxPopupTransientWindow>
{
public:
    wxRichToolTipPopup(wxWindow* parent,
                       const std::string& title,
                       const std::string& message,
                       const wxIcon& icon,
                       wxTipKind tipKind,
                       const wxFont& titleFont_) :
        m_timer(this)
    {
        Create(parent, wxFRAME_SHAPED);


        wxBoxSizer* const sizerTitle = new wxBoxSizer(wxHORIZONTAL);
        if ( icon.IsOk() )
        {
            sizerTitle->Add(new wxStaticBitmap(this, wxID_ANY, icon),
                            wxSizerFlags().Centre().Border(wxRIGHT));
        }
        //else: Simply don't show any icon.

        wxStaticText* const labelTitle = new wxStaticText(this, wxID_ANY, "");
        labelTitle->SetLabelText(title);

        wxFont titleFont(titleFont_);
        if ( !titleFont.IsOk() )
        {
            // Determine the appropriate title font for the current platform.
            titleFont = labelTitle->GetFont();

#ifdef HAVE_MSW_THEME
            // When using themes MSW tooltips use larger bluish version of the
            // normal font.
            if ( UseTooltipTheme() )
            {
                titleFont.MakeLarger();

                COLORREF c;
                if ( FAILED(::GetThemeColor
                                   (
                                        wxUxThemeHandle(parent, "TOOLTIP"),
                                        TTP_BALLOONTITLE,
                                        0,
                                        TMT_TEXTCOLOR,
                                        &c
                                    )) )
                {
                    // Use the standard value of this colour as fallback.
                    c = 0x993300;
                }

                labelTitle->SetForegroundColour(wxRGBToColour(c));
            }
            else
#endif // HAVE_MSW_THEME
            {
                // Everything else, including "classic" MSW look uses just the
                // bold version of the base font.
                titleFont.MakeBold();
            }
        }

        labelTitle->SetFont(titleFont);
        sizerTitle->Add(labelTitle, wxSizerFlags().Centre());

        wxBoxSizer* const sizerTop = new wxBoxSizer(wxVERTICAL);
        sizerTop->Add(sizerTitle,
                        wxSizerFlags().DoubleBorder(wxLEFT|wxRIGHT|wxTOP));

        // Use a spacer as we don't want to have a double border between the
        // elements, just a simple one will do.
        sizerTop->AddSpacer(wxSizerFlags::GetDefaultBorder());

        wxTextSizerWrapper wrapper(this);
        wxSizer* sizerText = wrapper.CreateSizer(message, -1 /* No wrapping */);

#ifdef HAVE_MSW_THEME
        if ( icon.IsOk() && UseTooltipTheme() )
        {
            // Themed tooltips under MSW align the text with the title, not
            // with the icon, so use a helper horizontal sizer in this case.
            wxBoxSizer* const sizerTextIndent = new wxBoxSizer(wxHORIZONTAL);
            sizerTextIndent->AddSpacer(icon.GetWidth());
            sizerTextIndent->Add(sizerText,
                                    wxSizerFlags().Border(wxLEFT).Centre());

            sizerText = sizerTextIndent;
        }
#endif // HAVE_MSW_THEME
        sizerTop->Add(sizerText,
                        wxSizerFlags().DoubleBorder(wxLEFT|wxRIGHT|wxBOTTOM)
                                      .Centre());

        SetSizer(sizerTop);

        const int offsetY = SetTipShapeAndSize(tipKind, GetBestSize());
        if ( offsetY > 0 )
        {
            // Offset our contents by the tip height to make it appear in the
            // main rectangle.
            sizerTop->PrependSpacer(offsetY);
        }

        Layout();
    }

    wxRichToolTipPopup(const wxRichToolTipPopup&) = delete;
	wxRichToolTipPopup& operator=(const wxRichToolTipPopup&) = delete;

    void SetBackgroundColours(wxColour colStart, wxColour colEnd)
    {
        if ( !colStart.IsOk() )
        {
            // Determine the best colour(s) to use on our own.
#ifdef HAVE_MSW_THEME
            if ( UseTooltipTheme() )
            {
                wxUxThemeHandle hTheme(GetParent(), "TOOLTIP");

                COLORREF c1, c2;
                if ( FAILED(::GetThemeColor
                                   (
                                        hTheme,
                                        TTP_BALLOONTITLE,
                                        0,
                                        TMT_GRADIENTCOLOR1,
                                        &c1
                                    )) ||
                    FAILED(::GetThemeColor
                                  (
                                        hTheme,
                                        TTP_BALLOONTITLE,
                                        0,
                                        TMT_GRADIENTCOLOR2,
                                        &c2
                                  )) )
                {
                    c1 = 0xffffff;
                    c2 = 0xf0e5e4;
                }

                colStart = wxRGBToColour(c1);
                colEnd = wxRGBToColour(c2);
            }
            else
#endif // HAVE_MSW_THEME
            {
                colStart = wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK);
            }
        }

        if ( colEnd.IsOk() )
        {
            // Use gradient-filled background bitmap.
            const wxSize size = GetClientSize();
            wxBitmap bmp(size);
            {
                wxMemoryDC dc(bmp);
                dc.Clear();
                dc.GradientFillLinear(size, colStart, colEnd, wxDOWN);
            }

            SetBackgroundBitmap(bmp);
        }
        else // Use solid colour.
        {
            SetBackgroundColour(colStart);
        }
    }

    void SetPosition(const wxRect* rect)
    {
        wxPoint pos;

        if ( !rect || rect->IsEmpty() )
            pos = GetTipPoint();
        else
            pos = GetParent()->ClientToScreen( wxPoint( rect->x + rect->width / 2, rect->y + rect->height / 2 ) );

        // We want our anchor point to coincide with this position so offset
        // the position of the top left corner passed to Move() accordingly.
        pos -= m_anchorPos;

        Move(pos, wxSIZE_NO_ADJUSTMENTS);
    }

    void DoShow()
    {
        Popup();
    }

    void SetTimeoutAndShow(std::chrono::milliseconds timeout, std::chrono::milliseconds delay)
    {
        if ( timeout == 0ms && delay == 0ms)
        {
            DoShow();
            return;
        }

        Bind(wxEVT_TIMER, &wxRichToolTipPopup::OnTimer, this);

        m_timeout = timeout; // set for use in OnTimer if we have a delay
        m_delayShow = delay != 0ms;

        if ( !m_delayShow )
            DoShow();

        m_timer.Start((m_delayShow ? delay : timeout), true /* one shot */);
    }

protected:
    void OnDismiss() override
    {
        Destroy();
    }

private:
#ifdef HAVE_MSW_THEME
    static bool UseTooltipTheme()
    {
        return wxUxThemeIsActive();
    }
#endif // HAVE_MSW_THEME

    // For now we just hard code the tip height, would be nice to do something
    // smarter in the future.
    static int GetTipHeight()
    {
#ifdef HAVE_MSW_THEME
        if ( UseTooltipTheme() )
            return 20;
#endif // HAVE_MSW_THEME

        return 15;
    }

    // Get the point to which our tip should point.
    wxPoint GetTipPoint() const
    {
        // Currently we always use the middle of the window. It seems that MSW
        // native tooltips use a different point but it's not really clear how
        // do they determine it nor whether it's worth the trouble to emulate
        // their behaviour.
        const wxRect r = GetParent()->GetScreenRect();
        return {r.x + r.width/2, r.y + r.height/2};
    }

    // Choose the correct orientation depending on the window position.
    //
    // Also use the tip kind appropriate for the current environment. For MSW
    // the right triangles are used and for Mac the equilateral ones as this is
    // the prevailing kind under these systems. For everything else we go with
    // right triangles as well but without any real rationale so this could be
    // tweaked in the future.
    wxTipKind GetBestTipKind() const
    {
        const wxPoint pos = GetTipPoint();

        // Use GetFromWindow() and not GetFromPoint() here to try to get the
        // correct display even if the tip point itself is not visible.
        const wxRect rectDpy = wxDisplay(GetParent()).GetClientArea();

#ifdef __WXMAC__
        return pos.y > rectDpy.height/2 ? wxTipKind::Bottom : wxTipKind::Top;
#else // !__WXMAC__
        return pos.y > rectDpy.height/2
                    ? pos.x > rectDpy.width/2
                        ? wxTipKind::BottomRight
                        : wxTipKind::BottomLeft
                    : pos.x > rectDpy.width/2
                        ? wxTipKind::TopRight
                        : wxTipKind::TopLeft;
#endif // __WXMAC__/!__WXMAC__
    }

    // Set the size and shape of the tip window and returns the offset of its
    // content area from the top (horizontal offset is always 0 currently).
    int SetTipShapeAndSize(wxTipKind tipKind, const wxSize& contentSize)
    {
#if wxUSE_GRAPHICS_CONTEXT
        wxSize size = contentSize;

        // The size is the vertical size and the offset is the distance from
        // edge for asymmetric tips, currently hard-coded to be the same as the
        // size.
        const int tipSize = GetTipHeight();
        const int tipOffset = tipSize;

        // The horizontal position of the tip.
        int x = -1;

        // The vertical coordinates of the tip base and apex.
        int yBase = -1,
            yApex = -1;

        // The offset of the content part of the window.
        int dy = -1;

        // Define symbolic names for the rectangle corners and mid-way points
        // that we use below in an attempt to make the code more clear. Notice
        // that these values must be consecutive as we iterate over them.
        enum RectPoint
        {
            RectPoint_TopLeft,
            RectPoint_Top,
            RectPoint_TopRight,
            RectPoint_Right,
            RectPoint_BotRight,
            RectPoint_Bot,
            RectPoint_BotLeft,
            RectPoint_Left,
            RectPoint_Max
        };

        // The starting point for AddArcToPoint() calls below, we iterate over
        // all RectPoints from it.
        RectPoint pointStart = RectPoint_Max;


        // Hard-coded radius of the round main rectangle corners.
        static constexpr double RADIUS = 5;

        // Create a path defining the shape of the tooltip window.
        wxGraphicsPath
            path = wxGraphicsRenderer::GetDefaultRenderer()->CreatePath();

        if ( tipKind == wxTipKind::Auto )
            tipKind = GetBestTipKind();

        // Points defining the tip shape (in clockwise order as we must end at
        // tipPoints[0] after drawing the rectangle outline in this order).
        wxPoint2DFloat tipPoints[3];

        switch ( tipKind )
        {
            case wxTipKind::Auto:
                wxFAIL_MSG( "Impossible kind value" );
                break;

            case wxTipKind::TopLeft:
                x = tipOffset;
                yApex = 0;
                yBase = tipSize;
                dy = tipSize;

                tipPoints[0] = wxPoint2DFloat(x, yBase);
                tipPoints[1] = wxPoint2DFloat(x, yApex);
                tipPoints[2] = wxPoint2DFloat(x + tipSize, yBase);

                pointStart = RectPoint_TopRight;
                break;

            case wxTipKind::TopRight:
                x = size.x - tipOffset;
                yApex = 0;
                yBase = tipSize;
                dy = tipSize;

                tipPoints[0] = wxPoint2DFloat(x - tipSize, yBase);
                tipPoints[1] = wxPoint2DFloat(x, yApex);
                tipPoints[2] = wxPoint2DFloat(x, yBase);

                pointStart = RectPoint_TopRight;
                break;

            case wxTipKind::BottomLeft:
                x = tipOffset;
                yApex = size.y + tipSize;
                yBase = size.y;
                dy = 0;

                tipPoints[0] = wxPoint2DFloat(x + tipSize, yBase);
                tipPoints[1] = wxPoint2DFloat(x, yApex);
                tipPoints[2] = wxPoint2DFloat(x, yBase);

                pointStart = RectPoint_BotLeft;
                break;

            case wxTipKind::BottomRight:
                x = size.x - tipOffset;
                yApex = size.y + tipSize;
                yBase = size.y;
                dy = 0;

                tipPoints[0] = wxPoint2DFloat(x, yBase);
                tipPoints[1] = wxPoint2DFloat(x, yApex);
                tipPoints[2] = wxPoint2DFloat(x - tipSize, yBase);

                pointStart = RectPoint_BotLeft;
                break;

            case wxTipKind::Top:
                x = size.x/2;
                yApex = 0;
                yBase = tipSize;
                dy = tipSize;

                {
                    // A half-side of an equilateral triangle is its altitude
                    // divided by sqrt(3) (in our case inverse of sqrt(3))
                    const double halfside = tipSize * std::numbers::inv_sqrt3;

                    tipPoints[0] = wxPoint2DFloat(x - halfside, yBase);
                    tipPoints[1] = wxPoint2DFloat(x, yApex);
                    tipPoints[2] = wxPoint2DFloat(x + halfside, yBase);
                }

                pointStart = RectPoint_TopRight;
                break;

            case wxTipKind::Bottom:
                x = size.x/2;
                yApex = size.y + tipSize;
                yBase = size.y;
                dy = 0;

                {
                    const double halfside = tipSize * std::numbers::inv_sqrt3;

                    tipPoints[0] = wxPoint2DFloat(x + halfside, yBase);
                    tipPoints[1] = wxPoint2DFloat(x, yApex);
                    tipPoints[2] = wxPoint2DFloat(x - halfside, yBase);
                }

                pointStart = RectPoint_BotLeft;
                break;

            case wxTipKind::None:
                x = size.x/2;
                dy = 0;

                path.AddRoundedRectangle(0, 0, size.x, size.y, RADIUS);
                break;
        }

        wxASSERT_MSG( dy != -1, "Unknown tip kind?" );

        size.y += tipSize;
        SetSize(size);

        if ( tipKind != wxTipKind::None )
        {
            path.MoveToPoint(tipPoints[0]);
            path.AddLineToPoint(tipPoints[1]);
            path.AddLineToPoint(tipPoints[2]);

            const double xLeft = 0.;
            const double xMid = size.x/2.;
            const double xRight = size.x;

            const double yTop = dy;
            const double yMid = (dy + size.y)/2.;
            const double yBot = dy + contentSize.y;

            wxPoint2DDouble rectPoints[RectPoint_Max];
            rectPoints[RectPoint_TopLeft]  = wxPoint2DDouble(xLeft,  yTop);
            rectPoints[RectPoint_Top]      = wxPoint2DDouble(xMid,   yTop);
            rectPoints[RectPoint_TopRight] = wxPoint2DDouble(xRight, yTop);
            rectPoints[RectPoint_Right]    = wxPoint2DDouble(xRight, yMid);
            rectPoints[RectPoint_BotRight] = wxPoint2DDouble(xRight, yBot);
            rectPoints[RectPoint_Bot]      = wxPoint2DDouble(xMid,   yBot);
            rectPoints[RectPoint_BotLeft]  = wxPoint2DDouble(xLeft,  yBot);
            rectPoints[RectPoint_Left]     = wxPoint2DDouble(xLeft,  yMid);

            // Iterate over all rectangle rectPoints for the first 3 corners.
            unsigned n = pointStart;
            for ( unsigned corner = 0; corner < 3; corner++ )
            {
                const wxPoint2DDouble& pt1 = rectPoints[n];

                n = (n + 1) % RectPoint_Max;

                const wxPoint2DDouble& pt2 = rectPoints[n];

                path.AddArcToPoint(pt1.x, pt1.y, pt2.x, pt2.y, RADIUS);

                n = (n + 1) % RectPoint_Max;
            }

            // Last one wraps to the first point of the tip.
            const wxPoint2DDouble& pt1 = rectPoints[n];
            const wxPoint2DFloat& pt2 = tipPoints[0];

            path.AddArcToPoint(pt1.x, pt1.y, pt2.x, pt2.y, RADIUS);

            path.CloseSubpath();
        }

        SetShape(path);
#else // !wxUSE_GRAPHICS_CONTEXT
        wxUnusedVar(tipKind);

        int x = contentSize.x/2,
            yApex = 0,
            dy = 0;

        SetSize(contentSize);
#endif // wxUSE_GRAPHICS_CONTEXT/!wxUSE_GRAPHICS_CONTEXT

        m_anchorPos.x = x;
        m_anchorPos.y = yApex;

        return dy;
    }

    // Timer event handler hides the tooltip when the timeout expires.
    void OnTimer([[maybe_unused]] wxTimerEvent& event)
    {
        if ( !m_delayShow )
        {
            // Doing "Notify" here ensures that our OnDismiss() is called and so we
            // also Destroy() ourselves. We could use Dismiss() and call Destroy()
            // explicitly from here as well.
            DismissAndNotify();

            return;
        }

        m_delayShow = false;

        if ( m_timeout != 0ms )
            m_timer.Start(m_timeout, true);

        DoShow();
    }


    // The anchor point offset if we show a tip or the middle of the top side
    // otherwise.
    wxPoint m_anchorPos;

    // The timer counting down the time until we're hidden.
    wxTimer m_timer;

    // We will need to accesss the timeout period when delaying showing tooltip.
    std::chrono::milliseconds m_timeout;

    // If true, delay showing the tooltip.
    bool m_delayShow;
};

// ----------------------------------------------------------------------------
// wxRichToolTipGenericImpl: generic implementation of wxRichToolTip.
// ----------------------------------------------------------------------------

void
wxRichToolTipGenericImpl::SetBackgroundColour(const wxColour& col,
                                              const wxColour& colEnd)
{
    m_colStart = col;
    m_colEnd = colEnd;
}

void wxRichToolTipGenericImpl::SetCustomIcon(const wxIcon& icon)
{
    m_icon = icon;
}

void wxRichToolTipGenericImpl::SetStandardIcon(int icon)
{
    switch ( icon & wxICON_MASK )
    {
        case wxICON_WARNING:
        case wxICON_ERROR:
        case wxICON_INFORMATION:
            // Although we don't use this icon in a list, we need a smallish
            // icon here and not an icon of a typical message box size so use
            // wxART_LIST to get it.
            m_icon = wxArtProvider::GetIcon
                     (
                        wxArtProvider::GetMessageBoxIconId(icon),
                        wxART_LIST
                     );
            break;

        case wxICON_QUESTION:
            wxFAIL_MSG("Question icon doesn't make sense for a tooltip");
            break;

        case wxICON_NONE:
            m_icon = wxNullIcon;
            break;
    }
}

void wxRichToolTipGenericImpl::SetTimeout(std::chrono::milliseconds timeout,
                                          std::chrono::milliseconds delay)
{
    m_delay = delay;
    m_timeout = timeout;
}

void wxRichToolTipGenericImpl::SetTipKind(wxTipKind tipKind)
{
    m_tipKind = tipKind;
}

void wxRichToolTipGenericImpl::SetTitleFont(const wxFont& font)
{
    m_titleFont = font;
}

void wxRichToolTipGenericImpl::ShowFor(wxWindow* win, const wxRect* rect)
{
    wxRichToolTipPopup* const popup = new wxRichToolTipPopup
                                          (
                                            win,
                                            m_title,
                                            m_message,
                                            m_icon,
                                            m_tipKind,
                                            m_titleFont
                                          );

    popup->SetBackgroundColours(m_colStart, m_colEnd);

    popup->SetPosition(rect);
    // show or start the timer to delay showing the popup
    popup->SetTimeoutAndShow( m_timeout, m_delay );
}

// Currently only wxMSW provides a native implementation.
#ifndef __WXMSW__

/* static */
wxRichToolTipImpl*
wxRichToolTipImpl::Create(const std::string& title, const std::string& message)
{
    return new wxRichToolTipGenericImpl(title, message);
}

#endif // !__WXMSW__

#endif // wxUSE_RICHTOOLTIP
