///////////////////////////////////////////////////////////////////////////////
// Name:        wx/mousestate.h
// Purpose:     Declaration of wxMouseState class
// Author:      Vadim Zeitlin
// Created:     2008-09-19 (extracted from wx/utils.h)
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/debug.h"

export module WX.Cmn.MouseState;

import WX.Cmn.KbdState;
import Utils.Geometry;

export
{

// the symbolic names for the mouse buttons
enum wxMouseButton
{
    wxMOUSE_BTN_ANY     = -1,
    wxMOUSE_BTN_NONE    = 0,
    wxMOUSE_BTN_LEFT    = 1,
    wxMOUSE_BTN_MIDDLE  = 2,
    wxMOUSE_BTN_RIGHT   = 3,
    wxMOUSE_BTN_AUX1    = 4,
    wxMOUSE_BTN_AUX2    = 5,
    wxMOUSE_BTN_MAX
};

// ----------------------------------------------------------------------------
// wxMouseState contains the information about mouse position, buttons and also
// key modifiers
// ----------------------------------------------------------------------------

// wxMouseState is used to hold information about button and modifier state
// and is what is returned from wxGetMouseState.
class wxMouseState : public wxKeyboardState
{
public:
    // accessors for the mouse position
    wxCoord GetX() const { return m_x; }
    wxCoord GetY() const { return m_y; }
    wxPoint GetPosition() const { return wxPoint(m_x, m_y); }

    // accessors for the pressed buttons
    bool LeftIsDown()    const { return m_leftDown; }
    bool MiddleIsDown()  const { return m_middleDown; }
    bool RightIsDown()   const { return m_rightDown; }
    bool Aux1IsDown()    const { return m_aux1Down; }
    bool Aux2IsDown()    const { return m_aux2Down; }

    bool ButtonIsDown(wxMouseButton but) const
    {
        switch ( but )
        {
            case wxMOUSE_BTN_ANY:
                return LeftIsDown() || MiddleIsDown() || RightIsDown() ||
                            Aux1IsDown() || Aux2IsDown();

            case wxMOUSE_BTN_LEFT:
                return LeftIsDown();

            case wxMOUSE_BTN_MIDDLE:
                return MiddleIsDown();

            case wxMOUSE_BTN_RIGHT:
                return RightIsDown();

            case wxMOUSE_BTN_AUX1:
                return Aux1IsDown();

            case wxMOUSE_BTN_AUX2:
                return Aux2IsDown();

            case wxMOUSE_BTN_NONE:
            case wxMOUSE_BTN_MAX:
                break;
        }

        assert("invalid parameter");
        return false;
    }


    // these functions are mostly used by wxWidgets itself
    void SetX(wxCoord x) { m_x = x; }
    void SetY(wxCoord y) { m_y = y; }
    void SetPosition(const wxPoint& pos) { m_x = pos.x; m_y = pos.y; }

    void SetLeftDown(bool down)   { m_leftDown = down; }
    void SetMiddleDown(bool down) { m_middleDown = down; }
    void SetRightDown(bool down)  { m_rightDown = down; }
    void SetAux1Down(bool down)   { m_aux1Down = down; }
    void SetAux2Down(bool down)   { m_aux2Down = down; }

    // this mostly makes sense in the derived classes such as wxMouseEvent
    void SetState(const wxMouseState& state) { *this = state; }

    // for compatibility reasons these variables are public as the code using
    // wxMouseEvent often uses them directly -- however they should not be
    // accessed directly in this class, use the accessors above instead
// private:
    bool m_leftDown{false};
    bool m_middleDown{false};
    bool m_rightDown{false};
    bool m_aux1Down{false};
    bool m_aux2Down{false};

    wxCoord m_x{0};
    wxCoord m_y{0};
};

} // export
