///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/ownerdrawnbutton.h
// Purpose:     Common base class for wxCheckBox and wxRadioButton
// Author:      Vadim Zeitlin
// Created:     2014-05-04
// Copyright:   (c) 2014 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_OWNERDRAWNBUTTON_H_
#define _WX_MSW_OWNERDRAWNBUTTON_H_

import WX.WinDef;

import Utils.Geometry;

// ----------------------------------------------------------------------------
// wxMSWOwnerDrawnButton: base class for any kind of Windows buttons
// ----------------------------------------------------------------------------

// This class contains the type-independent part of wxMSWOwnerDrawnButton and
// is implemented in src/msw/control.cpp.
//
// Notice that this class is internal implementation detail only and is
// intentionally not documented. Ideally it wouldn't be even exported from the
// DLL but this somehow breaks building of applications using wxWidgets with
// Intel compiler using LTCG, so we do export it.
class wxMSWOwnerDrawnButtonBase
{
protected:
    // Ctor takes the back pointer to the real window, must be non-NULL.
    wxMSWOwnerDrawnButtonBase(wxWindow* win) :
        m_win(win)
    {
    }

    // Explicitly define the destructor even if it's trivial to make it
    // protected. This avoids compiler warnings about the fact that this class
    // has virtual functions, but no virtual destructor without making the dtor
    // virtual which is not needed here as objects are never deleted via
    // pointers to this class (and protected dtor enforces this).

    wxMSWOwnerDrawnButtonBase& operator=(wxMSWOwnerDrawnButtonBase&&) = delete;

    // Make the control owner drawn if necessary to implement support for the
    // given foreground colour.
    void MSWMakeOwnerDrawnIfNecessary(const wxColour& colFg);

    // Return true if the control is currently owner drawn.
    bool MSWIsOwnerDrawn() const;

    // Draw the button if the message information about which is provided in
    // the given DRAWITEMSTRUCT asks us to do it, otherwise just return false.
    bool MSWDrawButton(WXDRAWITEMSTRUCT *item);


    // Methods which must be overridden in the derived concrete class.

    // Return the style to use for the non-owner-drawn button.
    virtual int MSWGetButtonStyle() const = 0;

    // Called after reverting button to non-owner drawn state, provides a hook
    // for wxCheckBox-specific hack.
    virtual void MSWOnButtonResetOwnerDrawn() { }

    // Return the flags (such as wxCONTROL_CHECKED) to use for the control when
    // drawing it. Notice that this class already takes care of the common
    // logic and sets the other wxCONTROL_XXX flags on its own, this method
    // really only needs to return the flags depending on the checked state.
    virtual unsigned int MSWGetButtonCheckedFlag() const = 0;

    // Actually draw the check or radio bitmap, typically just by using the
    // appropriate wxRendererNative method.
    virtual void
        MSWDrawButtonBitmap(wxDC& dc, const wxRect& rect, unsigned int flags) = 0;


private:
    // Make the control owner drawn or reset it to normal style.
    void MSWMakeOwnerDrawn(bool ownerDrawn);

    // Event handlers used to update the appearance of owner drawn button.
    void OnMouseEnterOrLeave(wxMouseEvent& event);
    void OnMouseLeft(wxMouseEvent& event);
    void OnFocus(wxFocusEvent& event);


    // The real window.
    wxWindow* const m_win;

    // true if the checkbox is currently pressed
    bool m_isPressed{false};

    // true if mouse is currently over the control
    bool m_isHot{false};
};

// This class uses a weak version of CRTP, i.e. it's a template class taking
// the base class that the class deriving from it would normally derive from.
template <class T>
class wxMSWOwnerDrawnButton
    : public T,
      private wxMSWOwnerDrawnButtonBase
{
private:
    using Base = T;

public:
    wxMSWOwnerDrawnButton() : wxMSWOwnerDrawnButtonBase(this)
    {
    }

    bool SetForegroundColour(const wxColour& colour) override
    {
        if ( !Base::SetForegroundColour(colour) )
            return false;

        MSWMakeOwnerDrawnIfNecessary(colour);

        return true;
    }

    bool MSWOnDraw(WXDRAWITEMSTRUCT *item) override
    {
        return MSWDrawButton(item) || Base::MSWOnDraw(item);
    }

protected:
    bool IsOwnerDrawn() const { return MSWIsOwnerDrawn(); }
};

#endif // _WX_MSW_OWNERDRAWNBUTTON_H_
