/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/uiactioncmn.cpp
// Purpose:     wxUIActionSimulator common implementation
// Author:      Kevin Ollivier, Steven Lamerton, Vadim Zeitlin
// Created:     2010-03-06
// Copyright:   (c) 2010 Kevin Ollivier
//              (c) 2010 Steven Lamerton
//              (c) 2010-2016 Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_UIACTIONSIMULATOR

#include "wx/uiaction.h"

#include "wx/ctrlsub.h"

#ifdef wxNO_RTTI
    #include "wx/choice.h"
    #include "wx/combobox.h"
    #include "wx/listbox.h"
#endif // wxNO_RTTI

#include "wx/private/uiaction.h"
#include "wx/utils.h"

#include <cassert>

// ----------------------------------------------------------------------------
// Methods forwarded to wxUIActionSimulatorImpl
// ----------------------------------------------------------------------------

bool wxUIActionSimulator::MouseMove(long x, long y)
{
    return m_impl->MouseMove(x, y);
}

bool wxUIActionSimulator::MouseDown(int button)
{
    return m_impl->MouseDown(button);
}

bool wxUIActionSimulator::MouseUp(int button)
{
    return m_impl->MouseUp(button);
}

bool wxUIActionSimulator::MouseClick(int button)
{
    return m_impl->MouseClick(button);
}

bool wxUIActionSimulator::MouseDblClick(int button)
{
    return m_impl->MouseDblClick(button);
}

bool wxUIActionSimulator::MouseDragDrop(long x1, long y1, long x2, long y2,
                                        int button)
{
    return m_impl->MouseDragDrop(x1, y1, x2, y2, button);
}

// ----------------------------------------------------------------------------
// Methods implemented in wxUIActionSimulatorImpl itself
// ----------------------------------------------------------------------------

bool wxUIActionSimulatorImpl::MouseClick(int button)
{
    MouseDown(button);
    MouseUp(button);

    return true;
}

bool wxUIActionSimulatorImpl::MouseDblClick(int button)
{
    MouseClick(button);
    MouseClick(button);

    return true;
}

bool wxUIActionSimulatorImpl::MouseDragDrop(long x1, long y1, long x2, long y2,
                                   int button)
{
    MouseMove(x1, y1);
    MouseDown(button);
    MouseMove(x2, y2);
    MouseUp(button);

    return true;
}

bool
wxUIActionSimulator::Key(int keycode, int modifiers, bool isDown)
{
    wxASSERT_MSG( !(modifiers & wxMOD_META ),
        "wxMOD_META is not implemented" );
    wxASSERT_MSG( !(modifiers & wxMOD_WIN ),
        "wxMOD_WIN is not implemented" );

    if ( isDown )
        SimulateModifiers(modifiers, true);

    bool rc = m_impl->DoKey(keycode, modifiers, isDown);

    if ( !isDown )
        SimulateModifiers(modifiers, false);

    return rc;
}

void wxUIActionSimulator::SimulateModifiers(int modifiers, bool isDown)
{
    if ( modifiers & wxMOD_SHIFT )
        m_impl->DoKey(WXK_SHIFT, modifiers, isDown);
    if ( modifiers & wxMOD_ALT )
        m_impl->DoKey(WXK_ALT, modifiers, isDown);
    if ( modifiers & wxMOD_CONTROL )
        m_impl->DoKey(WXK_CONTROL, modifiers, isDown);
}

bool wxUIActionSimulator::Char(int keycode, int modifiers)
{
    Key(keycode, modifiers, true);
    Key(keycode, modifiers, false);

    return true;
}

// Helper function checking if a key must be entered with Shift pressed. If it
// must, returns true and modifies the key to contain the unshifted version.
//
// This currently works only for the standard US keyboard layout which is
// definitely not ideal, but better than nothing...
static bool MapUnshifted(char& ch)
{
    const char* const unshifted =
        "`1234567890-=\\"
        "[]"
        ";'"
        ",./"
        ;

    const char* const shifted =
        "~!@#$%^&*()_+|"
        "{}"
        ":\""
        "<>?"
        ;

    static_assert(sizeof(unshifted) == sizeof(shifted),
                  "Shifted / Unshifted keys mismatch");

    const char* const p = strchr(shifted, ch);
    if ( !p )
        return false;

    ch = *(unshifted + (p - shifted));

    return true;
}

bool wxUIActionSimulator::Text(const std::string& str)
{
    for( auto ch : str )
    {
        int modifiers{0};

        if(std::isupper(ch) || MapUnshifted(ch))
            modifiers |= wxMOD_SHIFT;

        if( !Char(ch, modifiers))
            return false;
    }

    return true;
}

bool wxUIActionSimulator::Select(std::string_view text)
{
    wxWindow* const focus = wxWindow::FindFocus();
    if ( !focus )
        return false;

    // We can only select something in controls inheriting from
    // wxItemContainer, so check that we have it.
#ifdef wxNO_RTTI
    wxItemContainer* container = NULL;

    if ( wxComboBox* combo = dynamic_cast<wxComboBox*>(focus) )
        container = combo;
    else if ( wxChoice* choice = dynamic_cast<wxChoice*>(focus) )
        container = choice;
    else if ( wxListBox* listbox = dynamic_cast<wxListBox*>(focus) )
        container = listbox;
#else // !wxNO_RTTI
    wxItemContainer* const container = dynamic_cast<wxItemContainer*>(focus);
#endif // wxNO_RTTI/!wxNO_RTTI

    if ( !container )
        return false;

    // We prefer to exactly emulate what a (keyboard) user would do, so prefer
    // to emulate selecting the first item of the control if possible (this
    // works with wxChoice, wxListBox and wxComboBox with wxCB_READONLY style
    // under MSW).
    if ( container->GetSelection() != 0 )
    {
        Char(WXK_HOME);
        wxYield();

        // But if this didn't work, set the selection programmatically.
        if ( container->GetSelection() != 0 )
            container->SetSelection(0);
    }

    // And then go down in the control until we reach the item we want.
    for ( ;; )
    {
        if ( container->GetStringSelection() == text )
            return true;

        // We could test if the selection becomes equal to its maximal value
        // (i.e. GetCount() - 1), but if, for some reason, pressing WXK_DOWN
        // doesn't move it, this would still result in an infinite loop, so
        // check that the selection changed for additional safety.
        const int current = container->GetSelection();

        Char(WXK_DOWN);
        wxYield();

        if ( container->GetSelection() == current )
            break;
    }

    return false;
}

#endif // wxUSE_UIACTIONSIMULATOR
