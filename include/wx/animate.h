/////////////////////////////////////////////////////////////////////////////
// Name:        wx/animate.h
// Purpose:     wxAnimation and wxAnimationCtrl
// Author:      Julian Smart and Guillermo Rodriguez Garcia
// Modified by: Francesco Montorsi
// Created:     13/8/99
// Copyright:   (c) Julian Smart and Guillermo Rodriguez Garcia
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_ANIMATE_H_
#define _WX_ANIMATE_H_

#if wxUSE_ANIMATIONCTRL

#include "wx/animdecod.h"
#include "wx/control.h"
#include "wx/bitmap.h"

import Utils.Geometry;

#include <chrono>
import <string>;

class wxAnimation;
class wxAnimationImpl;

inline constexpr char wxAnimationCtrlNameStr[] = "animationctrl";

WX_DECLARE_LIST_WITH_DECL(wxAnimationDecoder, wxAnimationDecoderList, class);

// ----------------------------------------------------------------------------
// wxAnimation
// ----------------------------------------------------------------------------

class wxAnimation : public wxObject
{
public:
    wxAnimation();
    explicit wxAnimation(const std::string &name, wxAnimationType type = wxANIMATION_TYPE_ANY);

    wxAnimation(const wxAnimation&) = default;
    wxAnimation& operator=(const wxAnimation&) = default;

    bool IsOk() const;
    bool IsCompatibleWith(wxClassInfo* ci) const;

    std::chrono::milliseconds GetDelay(unsigned int frame) const;
    unsigned int GetFrameCount() const;
    wxImage GetFrame(unsigned int frame) const;
    wxSize GetSize() const;

    bool LoadFile(const std::string& name, wxAnimationType type = wxANIMATION_TYPE_ANY);
    bool Load(wxInputStream& stream, wxAnimationType type = wxANIMATION_TYPE_ANY);

    // Methods for managing the list of decoders
    static inline wxAnimationDecoderList& GetHandlers() { return sm_handlers; }
    static void AddHandler(wxAnimationDecoder *handler);
    static void InsertHandler(wxAnimationDecoder *handler);
    static const wxAnimationDecoder *FindHandler( wxAnimationType animType );

    static void CleanUpHandlers();
    static void InitStandardHandlers();

protected:
    wxAnimationImpl* GetImpl() const;

private:
    inline static wxAnimationDecoderList sm_handlers;

    // Ctor used by wxAnimationCtrl::CreateAnimation() only.
    explicit wxAnimation(wxAnimationImpl* impl);

    // Give it permission to create objects of this class using specific impl
    // and access our GetImpl().
    friend class wxAnimationCtrlBase;

    wxDECLARE_DYNAMIC_CLASS(wxAnimation);
};

inline const wxAnimation wxNullAnimation;

// ----------------------------------------------------------------------------
// wxAnimationCtrlBase
// ----------------------------------------------------------------------------

// do not autoresize to the animation's size when SetAnimation() is called
inline constexpr unsigned int wxAC_NO_AUTORESIZE       = 0x0010;

// default style does not include wxAC_NO_AUTORESIZE, that is, the control
// auto-resizes by default to fit the new animation when SetAnimation() is called
inline constexpr unsigned int wxAC_DEFAULT_STYLE       = wxBORDER_NONE;

class wxAnimationCtrlBase : public wxControl
{
public:
    // public API
    virtual bool LoadFile(const std::string& filename,
                          wxAnimationType type = wxANIMATION_TYPE_ANY) = 0;
    virtual bool Load(wxInputStream& stream,
                      wxAnimationType type = wxANIMATION_TYPE_ANY) = 0;

    virtual void SetAnimation(const wxAnimation &anim) = 0;
    wxAnimation GetAnimation() const { return m_animation; }

    virtual bool Play() = 0;
    virtual void Stop() = 0;

    virtual bool IsPlaying() const = 0;

    virtual void SetInactiveBitmap(const wxBitmap &bmp);

    // always return the original bitmap set in this control
    wxBitmap GetInactiveBitmap() const
        { return m_bmpStatic; }

    wxAnimation CreateAnimation() const
        { return MakeAnimFromImpl(DoCreateAnimationImpl()); }

protected:
    virtual wxAnimationImpl* DoCreateAnimationImpl() const = 0;

    // These methods allow derived classes access to private wxAnimation ctor
    // and wxAnimation::GetImpl(), respectively.
    static wxAnimation MakeAnimFromImpl(wxAnimationImpl* impl)
        { return wxAnimation(impl); }

    wxAnimationImpl* GetAnimImpl() const
        { return m_animation.GetImpl(); }

    // The associated animation, possibly invalid/empty.
    wxAnimation m_animation;

    // the inactive bitmap as it was set by the user
    wxBitmap m_bmpStatic;

    // the inactive bitmap currently shown in the control
    // (may differ in the size from m_bmpStatic)
    wxBitmap m_bmpStaticReal;

    // updates m_bmpStaticReal from m_bmpStatic if needed
    virtual void UpdateStaticImage();

    // called by SetInactiveBitmap
    virtual void DisplayStaticImage() = 0;
};


// ----------------------------------------------------------------------------
// include the platform-specific version of the wxAnimationCtrl class
// ----------------------------------------------------------------------------

#if defined(__WXGTK20__) && !defined(__WXUNIVERSAL__)
    #include "wx/gtk/animate.h"

    #define wxHAS_NATIVE_ANIMATIONCTRL
#else
    #include "wx/generic/animate.h"

    class wxAnimationCtrl : public wxGenericAnimationCtrl
    {
    public:
        wxAnimationCtrl() = default;
        wxAnimationCtrl(wxWindow *parent,
                        wxWindowID id,
                        const wxAnimation& anim = wxNullAnimation,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        unsigned int style = wxAC_DEFAULT_STYLE,
                        const std::string& name = wxAnimationCtrlNameStr)
            : wxGenericAnimationCtrl(parent, id, anim, pos, size, style, name)
            {}

    private:
        wxDECLARE_DYNAMIC_CLASS(wxAnimationCtrl);
    };
#endif // defined(__WXGTK20__)

#endif // wxUSE_ANIMATIONCTRL

#endif // _WX_ANIMATE_H_
