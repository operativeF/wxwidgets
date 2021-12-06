///////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/activityindicator.cpp
// Purpose:     Generic wxActivityIndicator implementation.
// Author:      Vadim Zeitlin
// Created:     2015-03-06
// Copyright:   (c) 2015 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_ACTIVITYINDICATOR && !defined(__WXGTK3__)

#include "wx/activityindicator.h"
#include "wx/dcclient.h"
#include "wx/timer.h"
#include "wx/graphics.h"

#include <chrono>

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// For now the appearance is fixed, we could make these constants customizable
// later if really needed.

// Total number of "running" dots.
constexpr int NUM_DOTS = 8;

using namespace std::chrono_literals;

// Delay between the consecutive updates in milliseconds.
constexpr auto FRAME_DELAY = 150ms;

// ----------------------------------------------------------------------------
// wxActivityIndicatorImpl: class containing the real implementation.
// ----------------------------------------------------------------------------

class wxActivityIndicatorImpl
{
public:
    explicit wxActivityIndicatorImpl(wxWindow* win)
        : m_timer(this),
          m_win(win)
    {
        m_frame = 0;

        win->Bind(wxEVT_PAINT, &wxActivityIndicatorImpl::OnPaint, this);
    }


    wxActivityIndicatorImpl(const wxActivityIndicatorImpl&) = delete;
	wxActivityIndicatorImpl& operator=(const wxActivityIndicatorImpl&) = delete;

    void Start()
    {
        // Avoid restarting the timer if it's already running, this could
        // result in jumps in appearance of the next frame while calling
        // Start() is not supposed to have any effect at all in this case.
        if ( m_timer.IsRunning() )
            return;

        m_timer.Start(FRAME_DELAY);
    }

    void Stop()
    {
        // Unlike above, it's not a problem to call Stop() even if we're not
        // running.
        m_timer.Stop();
    }

    bool IsRunning() const
    {
        return m_timer.IsRunning();
    }

    // This one is only called by AdvanceTimer.
    void Advance()
    {
        if ( ++m_frame == NUM_DOTS )
            m_frame = 0;

        m_win->Refresh();
    }

private:
    class AdvanceTimer : public wxTimer
    {
    public:
        explicit AdvanceTimer(wxActivityIndicatorImpl* owner)
            : m_owner(owner)
        {
        }

        AdvanceTimer(const AdvanceTimer&) = delete;
	    AdvanceTimer& operator=(const AdvanceTimer&) = delete;

        void Notify() override
        {
            m_owner->Advance();
        }

    private:
        wxActivityIndicatorImpl* const m_owner;
    };

    void OnPaint([[maybe_unused]] wxPaintEvent& event)
    {
        wxPaintDC pdc(m_win);

        std::unique_ptr<wxGraphicsContext> const
            gc(wxGraphicsRenderer::GetDefaultRenderer()->CreateContext(pdc));

        const wxSize size = m_win->GetClientSize();

        // Centre everything.
        gc->Translate(size.x/2., size.y/2.);

        // Radius of 1/10th allows to have reasonably sized dots with a bit of
        // separation between them and so subjectively looks a bit nicer than
        // perhaps more natural 1/8th.
        static constexpr double RADIUS_FACTOR = 10;

        const double r = std::min(size.x, size.y) / RADIUS_FACTOR;

        // The initial dot touches the top border.
        wxGraphicsPath path = gc->CreatePath();
        path.AddCircle(0, -(RADIUS_FACTOR / 2. - 1.)*r, r);

        // Subsequent dots are rotated by this angle with respect to the
        // previous one.
        constexpr float angle = wx::narrow_cast<float>(wxDegToRad(360. / NUM_DOTS));

        // And the animation effect is achieved just by starting to draw from
        // the next position every time.
        gc->Rotate(m_frame*angle);

        const bool isEnabled = m_win->IsThisEnabled();
        for ( int n = 0; n < NUM_DOTS; n++ )
        {
            // Draw all dots uniformly grey when the window is disabled,
            // otherwise each subsequent dot is slightly more opaque.
            const int opacityIndex = isEnabled ? n + 1 : 2;

            // wxALPHA_OPAQUE+1 is used just because it is divisible by the
            // default NUM_DOTS value, and we need -1 because of this to keep
            // it in 0..wxALPHA_OPAQUE range.
            const int opacity = opacityIndex*(wxALPHA_OPAQUE + 1)/NUM_DOTS - 1;

            gc->SetBrush(wxBrush(wxColour(0, 0, 0, opacity)));

            gc->FillPath(path);
            gc->Rotate(angle);
        }
    }

    AdvanceTimer m_timer;

    wxWindow* const m_win;

    int m_frame;
};

// ============================================================================
// implementation
// ============================================================================

bool
wxActivityIndicatorGeneric::Create(wxWindow* parent,
                                   wxWindowID winid,
                                   const wxPoint& pos,
                                   const wxSize& size,
                                   unsigned int style,
                                   std::string_view name)
{
    // Notice that we skip wxControl version, we don't need the validator
    // support that it adds.
    if ( !wxWindow::Create(parent, winid, pos, size, style, name) )
        return false;

    m_impl = new wxActivityIndicatorImpl(this);

    return true;
}

wxActivityIndicatorGeneric::~wxActivityIndicatorGeneric()
{
    delete m_impl;
}

void wxActivityIndicatorGeneric::Start()
{
    wxCHECK_RET( m_impl, "Must be created first" );

    m_impl->Start();
}

void wxActivityIndicatorGeneric::Stop()
{
    wxCHECK_RET( m_impl, "Must be created first" );

    m_impl->Stop();
}

bool wxActivityIndicatorGeneric::IsRunning() const
{
    return m_impl && m_impl->IsRunning();
}

wxSize wxActivityIndicatorGeneric::DoGetBestClientSize() const
{
    int size = 0;
    switch ( GetWindowVariant() )
    {
        case wxWindowVariant::Max:
            wxFAIL_MSG("Invalid window variant");
            [[fallthrough]];

        case wxWindowVariant::Normal:
            size = 24;
            break;

        case wxWindowVariant::Small:
            size = 16;
            break;

        case wxWindowVariant::Mini:
            size = 12;
            break;

        case wxWindowVariant::Large:
            size = 32;
            break;
    }

    wxASSERT_MSG( size, "Unknown window variant" );

    return FromDIP(wxSize(size, size));
}

#endif // wxUSE_ACTIVITYINDICATOR && !__WXGTK3__
