///////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/animateg.cpp
// Purpose:     wxAnimationGenericImpl and wxGenericAnimationCtrl
// Author:      Julian Smart and Guillermo Rodriguez Garcia
// Modified by: Francesco Montorsi
// Created:     13/8/99
// Copyright:   (c) Julian Smart and Guillermo Rodriguez Garcia
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_ANIMATIONCTRL

#include "wx/animate.h"
#include "wx/generic/animate.h"
#include "wx/generic/private/animate.h"

#include "wx/log.h"
#include "wx/dcmemory.h"
#include "wx/dcclient.h"

#include <chrono>

import WX.Cmn.WFStream;

import WX.Image;

// ----------------------------------------------------------------------------
// wxAnimation
// ----------------------------------------------------------------------------

#ifndef wxHAS_NATIVE_ANIMATIONCTRL

/* static */
wxAnimationImpl *wxAnimationImpl::CreateDefault()
{
    return new wxAnimationGenericImpl();
}

#endif // !wxHAS_NATIVE_ANIMATIONCTRL

bool wxAnimationGenericImpl::IsCompatibleWith(wxClassInfo* ci) const
{
    return ci->IsKindOf(&wxGenericAnimationCtrl::ms_classInfo);
}

wxSize wxAnimationGenericImpl::GetSize() const
{
    return m_decoder->GetAnimationSize();
}

unsigned int wxAnimationGenericImpl::GetFrameCount() const
{
    return m_decoder->GetFrameCount();
}

wxImage wxAnimationGenericImpl::GetFrame(unsigned int i) const
{
    wxImage ret;
    if (!m_decoder->ConvertToImage(i, &ret))
        return wxNullImage;
    return ret;
}

std::chrono::milliseconds wxAnimationGenericImpl::GetDelay(unsigned int i) const
{
    return m_decoder->GetDelay(i);
}

wxPoint wxAnimationGenericImpl::GetFramePosition(unsigned int frame) const
{
    return m_decoder->GetFramePosition(frame);
}

wxSize wxAnimationGenericImpl::GetFrameSize(unsigned int frame) const
{
    return m_decoder->GetFrameSize(frame);
}

wxAnimationDisposal wxAnimationGenericImpl::GetDisposalMethod(unsigned int frame) const
{
    return m_decoder->GetDisposalMethod(frame);
}

wxColour wxAnimationGenericImpl::GetTransparentColour(unsigned int frame) const
{
    return m_decoder->GetTransparentColour(frame);
}

wxColour wxAnimationGenericImpl::GetBackgroundColour() const
{
    return m_decoder->GetBackgroundColour();
}

bool wxAnimationGenericImpl::LoadFile(const std::string& filename, wxAnimationType type)
{
    wxFileInputStream stream(filename);
    if ( !stream.IsOk() )
        return false;

    return Load(stream, type);
}

bool wxAnimationGenericImpl::Load(wxInputStream &stream, wxAnimationType type)
{
    UnRef();

    const wxAnimationDecoder *handler;
    if ( type == wxANIMATION_TYPE_ANY )
    {
        for ( wxAnimationDecoderList::compatibility_iterator node = wxAnimation::GetHandlers().GetFirst();
              node; node = node->GetNext() )
        {
            handler=(const wxAnimationDecoder*)node->GetData();

            if ( handler->CanRead(stream) )
            {
                // do a copy of the handler from the static list which we will own
                // as our reference data
                m_decoder = handler->Clone();
                return m_decoder->Load(stream);
            }
        }

        wxLogWarning( _("No handler found for animation type.") );
        return false;
    }

    handler = wxAnimation::FindHandler(type);

    if (handler == nullptr)
    {
        wxLogWarning( _("No animation handler for type %ld defined."), type );

        return false;
    }


    // do a copy of the handler from the static list which we will own
    // as our reference data
    m_decoder = handler->Clone();

    if (stream.IsSeekable() && !m_decoder->CanRead(stream))
    {
        wxLogError(_("Animation file is not of type %ld."), type);
        return false;
    }
    else
        return m_decoder->Load(stream);
}

void wxAnimationGenericImpl::UnRef()
{
    if ( m_decoder )
    {
        m_decoder->DecRef();
        m_decoder = nullptr;
    }
}

// ----------------------------------------------------------------------------
// wxAnimationCtrl
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(wxGenericAnimationCtrl, wxAnimationCtrlBase)
    EVT_PAINT(wxGenericAnimationCtrl::OnPaint)
    EVT_SIZE(wxGenericAnimationCtrl::OnSize)
    EVT_TIMER(wxID_ANY, wxGenericAnimationCtrl::OnTimer)
wxEND_EVENT_TABLE()

bool wxGenericAnimationCtrl::Create(wxWindow *parent, wxWindowID id,
            const wxAnimation& animation, const wxPoint& pos,
            const wxSize& size, unsigned int style, std::string_view name)
{
    m_timer.SetOwner(this);

    if (!base_type::Create(parent, id, pos, size, style, wxValidator{}, name))
        return false;

    // by default we get the same background colour of our parent
    SetBackgroundColour(parent->GetBackgroundColour());

    SetAnimation(animation);

    return true;
}

wxGenericAnimationCtrl::~wxGenericAnimationCtrl()
{
    if (IsPlaying())
        Stop();
}

bool wxGenericAnimationCtrl::LoadFile(const std::string& filename, wxAnimationType type)
{
    wxFileInputStream fis(filename);
    if (!fis.IsOk())
        return false;
    return Load(fis, type);
}

bool wxGenericAnimationCtrl::Load(wxInputStream& stream, wxAnimationType type)
{
    wxAnimation anim(CreateAnimation());
    if ( !anim.Load(stream, type) || !anim.IsOk() )
        return false;

    SetAnimation(anim);
    return true;
}

wxAnimation wxGenericAnimationCtrl::CreateCompatibleAnimation()
{
    return MakeAnimFromImpl(new wxAnimationGenericImpl());
}

wxAnimationImpl* wxGenericAnimationCtrl::DoCreateAnimationImpl() const
{
    return new wxAnimationGenericImpl();
}

wxSize wxGenericAnimationCtrl::DoGetBestSize() const
{
    if (m_animation.IsOk() && !this->HasFlag(wxAC_NO_AUTORESIZE))
        return m_animation.GetSize();

    return FromDIP(wxSize(100, 100));
}

void wxGenericAnimationCtrl::SetAnimation(const wxAnimation& animation)
{
    if (IsPlaying())
        Stop();

    // set new animation even if it's wxNullAnimation
    m_animation = animation;
    if (!m_animation.IsOk())
    {
        DisplayStaticImage();
        return;
    }

    wxCHECK_RET(animation.IsCompatibleWith(wxGetClassInfo()),
                "incompatible animation" );

    if (AnimationImplGetBackgroundColour() == wxNullColour)
        SetUseWindowBackgroundColour();
    if (!this->HasFlag(wxAC_NO_AUTORESIZE))
        FitToAnimation();

    DisplayStaticImage();
}

void wxGenericAnimationCtrl::SetInactiveBitmap(const wxBitmap &bmp)
{
    // if the bitmap has an associated mask, we need to set our background to
    // the colour of our parent otherwise when calling DrawCurrentFrame()
    // (which uses the bitmap's mask), our background colour would be used for
    // transparent areas - and that's not what we want (at least for
    // consistency with the GTK version)
    if ( bmp.IsOk() && bmp.GetMask() != nullptr && GetParent() != nullptr )
        SetBackgroundColour(GetParent()->GetBackgroundColour());

    wxAnimationCtrlBase::SetInactiveBitmap(bmp);
}

void wxGenericAnimationCtrl::FitToAnimation()
{
    SetSize(m_animation.GetSize());
}

bool wxGenericAnimationCtrl::SetBackgroundColour(const wxColour& colour)
{
    if ( !wxWindow::SetBackgroundColour(colour) )
        return false;

    // if not playing, then this change must be seen immediately (unless
    // there's an inactive bitmap set which has higher priority than bg colour)
    if ( !IsPlaying() )
        DisplayStaticImage();

    return true;
}


// ----------------------------------------------------------------------------
// wxAnimationCtrl - stop/play methods
// ----------------------------------------------------------------------------

void wxGenericAnimationCtrl::Stop()
{
    m_timer.Stop();
    m_isPlaying = false;

    // reset frame counter
    m_currentFrame = 0;

    DisplayStaticImage();
}

bool wxGenericAnimationCtrl::Play(bool looped)
{
    if (!m_animation.IsOk())
        return false;

    m_looped = looped;
    m_currentFrame = 0;

    if (!RebuildBackingStoreUpToFrame(0))
        return false;

    m_isPlaying = true;

    // do a ClearBackground() to avoid that e.g. the custom static bitmap which
    // was eventually shown previously remains partially drawn
    ClearBackground();

    // DrawCurrentFrame() will use our updated backing store
    wxClientDC clientDC(this);
    DrawCurrentFrame(clientDC);

    // start the timer
    auto delay = m_animation.GetDelay(0);
    if (delay == 0ms)
        delay = 1ms;      // 0 is invalid timeout for wxTimer.
    m_timer.Start(delay, true);

    return true;
}



// ----------------------------------------------------------------------------
// wxAnimationCtrl - rendering methods
// ----------------------------------------------------------------------------

bool wxGenericAnimationCtrl::RebuildBackingStoreUpToFrame(unsigned int frame)
{
    // if we've not created the backing store yet or it's too
    // small, then recreate it
    wxSize sz = m_animation.GetSize(),
           winsz = GetClientSize();
    int w = std::min(sz.x, winsz.x);
    int h = std::min(sz.y, winsz.y);

    if ( !m_backingStore.IsOk() ||
            m_backingStore.GetWidth() < w || m_backingStore.GetHeight() < h )
    {
        if (!m_backingStore.Create(wxSize{w, h}))
            return false;
    }

    wxMemoryDC dc;
    dc.SelectObject(m_backingStore);

    // Draw the background
    DisposeToBackground(dc);

    // Draw all intermediate frames that haven't been removed from the animation
    for (unsigned int i = 0; i < frame; i++)
    {
        if (AnimationImplGetDisposalMethod(i) == wxANIM_DONOTREMOVE ||
            AnimationImplGetDisposalMethod(i) == wxANIM_UNSPECIFIED)
        {
            DrawFrame(dc, i);
        }
        else if (AnimationImplGetDisposalMethod(i) == wxANIM_TOBACKGROUND)
            DisposeToBackground(dc, AnimationImplGetFramePosition(i),
                                    AnimationImplGetFrameSize(i));
    }

    // finally draw this frame
    DrawFrame(dc, frame);
    dc.SelectObject(wxNullBitmap);

    return true;
}

void wxGenericAnimationCtrl::IncrementalUpdateBackingStore()
{
    wxMemoryDC dc;
    dc.SelectObject(m_backingStore);

    // OPTIMIZATION:
    // since wxAnimationCtrl can only play animations forward, without skipping
    // frames, we can be sure that m_backingStore contains the m_currentFrame-1
    // frame and thus we just need to dispose the m_currentFrame-1 frame and
    // render the m_currentFrame-th one.

    if (m_currentFrame == 0)
    {
        // before drawing the first frame always dispose to bg colour
        DisposeToBackground(dc);
    }
    else
    {
        switch (AnimationImplGetDisposalMethod(m_currentFrame-1))
        {
        case wxANIM_TOBACKGROUND:
            DisposeToBackground(dc, AnimationImplGetFramePosition(m_currentFrame-1),
                                    AnimationImplGetFrameSize(m_currentFrame-1));
            break;

        case wxANIM_TOPREVIOUS:
            // this disposal should never be used too often.
            // E.g. GIF specification explicitly say to keep the usage of this
            //      disposal limited to the minimum.
            // In fact it may require a lot of time to restore
            if (m_currentFrame == 1)
            {
                // if 0-th frame disposal is to restore to previous frame,
                // the best we can do is to restore to background
                DisposeToBackground(dc);
            }
            else
                if (!RebuildBackingStoreUpToFrame(m_currentFrame-2))
                    Stop();
            break;

        case wxANIM_DONOTREMOVE:
        case wxANIM_UNSPECIFIED:
            break;
        }
    }

    // now just draw the current frame on the top of the backing store
    DrawFrame(dc, m_currentFrame);
    dc.SelectObject(wxNullBitmap);
}

void wxGenericAnimationCtrl::DisplayStaticImage()
{
    wxASSERT(!IsPlaying());

    // m_bmpStaticReal will be updated only if necessary...
    UpdateStaticImage();

    if (m_bmpStaticReal.IsOk())
    {
        // copy the inactive bitmap in the backing store
        // eventually using the mask or the alpha if the static
        // bitmap has one
        if ( m_bmpStaticReal.GetMask() || m_bmpStaticReal.HasAlpha() )
        {
            wxMemoryDC temp;
            temp.SelectObject(m_backingStore);
            DisposeToBackground(temp);
            temp.DrawBitmap(m_bmpStaticReal, 0, 0, true /* use mask */);
        }
        else
            m_backingStore = m_bmpStaticReal;
    }
    else
    {
        // put in the backing store the first frame of the animation
        if (!m_animation.IsOk() ||
            !RebuildBackingStoreUpToFrame(0))
        {
            m_animation = wxNullAnimation;
            DisposeToBackground();
        }
    }

    Refresh();
}

void wxGenericAnimationCtrl::DrawFrame(wxDC &dc, unsigned int frame)
{
    // PERFORMANCE NOTE:
    // this draw stuff is not as fast as possible: the wxAnimationDecoder
    // needs first to convert from its internal format to wxImage RGB24;
    // the wxImage is then converted as a wxBitmap and finally blitted.
    // If wxAnimationDecoder had a function to convert directly from its
    // internal format to a port-specific wxBitmap, it would be somewhat faster.
    wxBitmap bmp(m_animation.GetFrame(frame));
    dc.DrawBitmap(bmp, AnimationImplGetFramePosition(frame),
                  true /* use mask */);
}

void wxGenericAnimationCtrl::DrawCurrentFrame(wxDC& dc)
{
    wxASSERT( m_backingStore.IsOk() );

    // m_backingStore always contains the current frame
    dc.DrawBitmap(m_backingStore, 0, 0, true /* use mask in case it's present */);
}

void wxGenericAnimationCtrl::DisposeToBackground()
{
    // clear the backing store
    wxMemoryDC dc;
    dc.SelectObject(m_backingStore);
    if ( dc.IsOk() )
        DisposeToBackground(dc);
}

void wxGenericAnimationCtrl::DisposeToBackground(wxDC& dc)
{
    wxColour col = IsUsingWindowBackgroundColour()
                    ? GetBackgroundColour()
                    : AnimationImplGetBackgroundColour();

    wxBrush brush(col);
    dc.SetBackground(brush);
    dc.Clear();
}

void wxGenericAnimationCtrl::DisposeToBackground(wxDC& dc, const wxPoint &pos, const wxSize &sz)
{
    wxColour col = IsUsingWindowBackgroundColour()
                    ? GetBackgroundColour()
                    : AnimationImplGetBackgroundColour();
    wxBrush brush(col);
    dc.SetBrush(brush);         // SetBrush and not SetBackground !!
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(pos, sz);
}

// ----------------------------------------------------------------------------
// wxAnimationCtrl - event handlers
// ----------------------------------------------------------------------------

void wxGenericAnimationCtrl::OnPaint([[maybe_unused]] wxPaintEvent& event)
{
    // VERY IMPORTANT: the wxPaintDC *must* be created in any case
    wxPaintDC dc(this);

    if ( m_backingStore.IsOk() )
    {
        // NOTE: we draw the bitmap explicitly ignoring the mask (if any);
        //       i.e. we don't want to combine the backing store with the
        //       possibly wrong preexisting contents of the window!
        dc.DrawBitmap(m_backingStore, 0, 0, false /* no mask */);
    }
    else
    {
        // m_animation is not valid and thus we don't have a valid backing store...
        // clear then our area to the background colour
        DisposeToBackground(dc);
    }
}

void wxGenericAnimationCtrl::OnTimer([[maybe_unused]] wxTimerEvent &event)
{
    m_currentFrame++;
    if (m_currentFrame == m_animation.GetFrameCount())
    {
        // Should a non-looped animation display the last frame?
        if (!m_looped)
        {
            Stop();
            return;
        }
        else
            m_currentFrame = 0;     // let's restart
    }

    IncrementalUpdateBackingStore();

    wxClientDC dc(this);
    DrawCurrentFrame(dc);

#ifdef __WXMAC__
    // without this, the animation currently doesn't redraw under Mac
    Refresh();
#endif // __WXMAC__

    // Set the timer for the next frame
    auto delay = m_animation.GetDelay(m_currentFrame);
    if (delay == 0ms)
        delay = 1ms;      // 0 is invalid timeout for wxTimer.
    m_timer.Start(delay, true);
}

void wxGenericAnimationCtrl::OnSize([[maybe_unused]] wxSizeEvent &event)
{
    // NB: resizing an animation control may take a lot of time
    //     for big animations as the backing store must be
    //     extended and rebuilt. Try to avoid it e.g. using
    //     a null proportion value for your wxAnimationCtrls
    //     when using them inside sizers.
    if (m_animation.IsOk())
    {
        // be careful to change the backing store *only* if we are
        // playing the animation as otherwise we may be displaying
        // the inactive bitmap and overwriting the backing store
        // with the last played frame is wrong in this case
        if (IsPlaying())
        {
            if (!RebuildBackingStoreUpToFrame(m_currentFrame))
                Stop();     // in case we are playing
        }
    }
}

// ----------------------------------------------------------------------------
// helpers to safely access wxAnimationGenericImpl methods
// ----------------------------------------------------------------------------
#define ANIMATION (static_cast<wxAnimationGenericImpl*>(GetAnimImpl()))

wxPoint wxGenericAnimationCtrl::AnimationImplGetFramePosition(unsigned int frame) const
{
    wxCHECK_MSG( m_animation.IsOk(), wxDefaultPosition, "invalid animation" );
    return ANIMATION->GetFramePosition(frame);
}

wxSize wxGenericAnimationCtrl::AnimationImplGetFrameSize(unsigned int frame) const
{
    wxCHECK_MSG( m_animation.IsOk(), wxDefaultSize, "invalid animation" );
    return ANIMATION->GetFrameSize(frame);
}

wxAnimationDisposal wxGenericAnimationCtrl::AnimationImplGetDisposalMethod(unsigned int frame) const
{
    wxCHECK_MSG( m_animation.IsOk(), wxANIM_UNSPECIFIED, "invalid animation" );
    return ANIMATION->GetDisposalMethod(frame);
}

wxColour wxGenericAnimationCtrl::AnimationImplGetTransparentColour(unsigned int frame) const
{
    wxCHECK_MSG( m_animation.IsOk(), wxNullColour, "invalid animation" );
    return ANIMATION->GetTransparentColour(frame);
}

wxColour wxGenericAnimationCtrl::AnimationImplGetBackgroundColour() const
{
    wxCHECK_MSG( m_animation.IsOk(), wxNullColour, "invalid animation" );
    return ANIMATION->GetBackgroundColour();
}

#endif // wxUSE_ANIMATIONCTRL
