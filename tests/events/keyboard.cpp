///////////////////////////////////////////////////////////////////////////////
// Name:        tests/events/keyboard.cpp
// Purpose:     Test keyboard events
// Author:      Vadim Zeitlin
// Created:     2010-09-05
// Copyright:   (c) 2010 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#if wxUSE_UIACTIONSIMULATOR

#include "wx/app.h"
#include "wx/event.h"
#include "wx/window.h"

#include "wx/uiaction.h"

#ifdef __WXGTK__
import WX.Cmn.Stopwatch;
#endif

import WX.Test.Prec;

import <ostream>;

namespace
{

// ----------------------------------------------------------------------------
// test window verifying the event generation
// ----------------------------------------------------------------------------

class KeyboardTestWindow : public wxWindow
{
public:
    KeyboardTestWindow(wxWindow *parent)
        : wxWindow(parent, wxID_ANY, wxPoint(0, 0), parent->GetClientSize())
    {
        Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(KeyboardTestWindow::OnKeyDown));
        Connect(wxEVT_CHAR, wxKeyEventHandler(KeyboardTestWindow::OnChar));
        Connect(wxEVT_KEY_UP, wxKeyEventHandler(KeyboardTestWindow::OnKeyUp));
    }

    KeyboardTestWindow(const KeyboardTestWindow&) = delete;
    KeyboardTestWindow& operator=(const KeyboardTestWindow&) = delete;

    size_t GetKeyDownCount() const { return m_keyDownEvents.size(); }
    size_t GetCharCount() const { return m_charEvents.size(); }
    size_t GetKeyUpCount() const { return m_keyUpEvents.size(); }

    const wxKeyEvent& GetKeyDownEvent(unsigned n = 0) const
    {
        return m_keyDownEvents[n];
    }
    const wxKeyEvent& GetCharEvent(unsigned n = 0) const
    {
        return m_charEvents[n];
    }
    const wxKeyEvent& GetKeyUpEvent(unsigned n = 0) const
    {
        return m_keyUpEvents[n];
    }

    void ClearEvents()
    {
        m_keyDownEvents.clear();
        m_charEvents.clear();
        m_keyUpEvents.clear();
    }

private:
    void OnKeyDown(wxKeyEvent& event)
    {
        m_keyDownEvents.push_back(event);
        event.Skip();
    }

    void OnChar(wxKeyEvent& event)
    {
        m_charEvents.push_back(event);
        event.Skip();
    }

    void OnKeyUp(wxKeyEvent& event)
    {
        m_keyUpEvents.push_back(event);
        event.Skip();
    }

    std::vector<wxKeyEvent> m_keyDownEvents;
    std::vector<wxKeyEvent> m_charEvents;
    std::vector<wxKeyEvent> m_keyUpEvents;

};

// Object describing the (main fields of) keyboard event.
struct KeyDesc
{
    KeyDesc(int keycode, int mods = 0)
        : m_keycode(keycode),
          m_mods(mods)
    {
    }

    int m_keycode;
    int m_mods;
};

// Helper for ModKeyDown().
int GetModForKey(int keycode)
{
    switch ( keycode )
    {
        case WXK_CONTROL:   return wxMOD_CONTROL;
        case WXK_SHIFT:     return wxMOD_SHIFT;
        case WXK_ALT:       return wxMOD_ALT;
        default:
            wxFAIL_MSG( "Unknown modifier key" );
    }

    return wxMOD_NONE;
}

// Helper function to allow writing just ModKeyDown(WXK_CONTROL) instead of
// more verbose KeyDesc(WXK_CONTROL, wxMOD_CONTROL).
KeyDesc ModKeyDown(int keycode)
{
    return KeyDesc(keycode, GetModForKey(keycode));
}

// Another helper provided for symmetry with ModKeyDown() only.
KeyDesc ModKeyUp(int keycode)
{
    return KeyDesc(keycode);
}

// Verify that the event object corresponds to our idea of what it should be.
void TestEvent(int line, const wxKeyEvent& ev, const KeyDesc& desc)
{
    // Construct the message we'll display if an assert fails.
    std::string msg = [=](const auto testEvt) {
        if(testEvt == wxEVT_KEY_DOWN)
            return "key down";
        else if(testEvt == wxEVT_KEY_UP)
            return "key up";
        else if(testEvt == wxEVT_CHAR)
            return "char";
        else
        {
            FAIL("unknown event type");
            return ""; // Never actually will reach this.
        }
        
    }(ev.GetEventType());

    msg += " event at line ";
    msg += wxString::Format("%d", line).mb_str();


    CHECK_MESSAGE(desc.m_keycode == ev.GetKeyCode(), ("wrong key code in " + msg) );

    if ( desc.m_keycode < WXK_START )
    {
        // For Latin-1 our key code is the same as Unicode character value.
        CHECK_MESSAGE((char)desc.m_keycode == (char)ev.GetUnicodeKey(),
                      ("wrong Unicode key in " + msg) );
    }
    else // Special key
    {
        // Key codes above WXK_START don't correspond to printable characters.
        CHECK_MESSAGE( 0 == (int)ev.GetUnicodeKey(),
                       ("wrong non-zero Unicode key in " + msg) );
    }

    CHECK_MESSAGE( desc.m_mods == ev.GetModifiers(),
                   ("wrong modifiers in " + msg) );
}

// Call TestEvent() passing it the line number from where it was called: this
// is useful for interpreting the assert failure messages.
#define ASSERT_KEY_EVENT_IS( ev, desc ) TestEvent(__LINE__, ev, desc)

} // anonymous namespace


// FIXME: A lot of yielding going on here.
// Is this really necessary?
TEST_CASE("Keyboard Event Tests")
{
    auto m_win = std::make_unique<KeyboardTestWindow>(wxTheApp->GetTopWindow());
    m_win->SetFocus();

#ifdef __WXGTK__
    for (wxStopWatch sw; sw.Time() < 10; )
#endif
    wxYield(); // needed to show the new window

    // The window might get some key up events when it's being shown if the key
    // was pressed when the program was started and released after the window
    // was shown, e.g. this does happen in practice when launching the test
    // from command line. Simply discard all the spurious events so far.
    m_win->ClearEvents();

    SUBCASE("NormalLetter")
    {
        wxUIActionSimulator sim;
        sim.Char('a');
        wxYield();

        CHECK_EQ( 1, m_win->GetKeyDownCount() );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyDownEvent(), 'A' );

        CHECK_EQ( 1, m_win->GetCharCount() );
        ASSERT_KEY_EVENT_IS( m_win->GetCharEvent(), 'a' );

        CHECK_EQ( 1, m_win->GetKeyUpCount() );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyUpEvent(), 'A' );
    }

    SUBCASE("NormalSpecial")
    {
        wxUIActionSimulator sim;
        sim.Char(WXK_END);
        wxYield();

        CHECK_EQ( 1, m_win->GetKeyDownCount() );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyDownEvent(), WXK_END );

        CHECK_EQ( 1, m_win->GetCharCount() );
        ASSERT_KEY_EVENT_IS( m_win->GetCharEvent(), WXK_END );

        CHECK_EQ( 1, m_win->GetKeyUpCount() );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyUpEvent(), WXK_END );
    }

    SUBCASE("CtrlLetter")
    {
        wxUIActionSimulator sim;
        sim.Char('z', wxMOD_CONTROL);
        wxYield();

        CHECK_EQ( 2, m_win->GetKeyDownCount() );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyDownEvent(0),
                             ModKeyDown(WXK_CONTROL) );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyDownEvent(1),
                             KeyDesc('Z', wxMOD_CONTROL) );

        CHECK_EQ( 1, m_win->GetCharCount() );
        ASSERT_KEY_EVENT_IS( m_win->GetCharEvent(),
                             KeyDesc('\x1a', wxMOD_CONTROL) );

        CHECK_EQ( 2, m_win->GetKeyUpCount() );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyUpEvent(0),
                             KeyDesc('Z', wxMOD_CONTROL) );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyUpEvent(1),
                             ModKeyUp(WXK_CONTROL) );
    }

    SUBCASE("CtrlSpecial")
    {
        wxUIActionSimulator sim;
        sim.Char(WXK_PAGEUP, wxMOD_CONTROL);
        wxYield();

        CHECK_EQ( 2, m_win->GetKeyDownCount() );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyDownEvent(0),
                             ModKeyDown(WXK_CONTROL) );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyDownEvent(1),
                             KeyDesc(WXK_PAGEUP, wxMOD_CONTROL) );

        CHECK_EQ( 1, m_win->GetCharCount() );
        ASSERT_KEY_EVENT_IS( m_win->GetCharEvent(),
                             KeyDesc(WXK_PAGEUP, wxMOD_CONTROL) );

        CHECK_EQ( 2, m_win->GetKeyUpCount() );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyUpEvent(0),
                             KeyDesc(WXK_PAGEUP, wxMOD_CONTROL) );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyUpEvent(1),
                             ModKeyUp(WXK_CONTROL) );
    }

    SUBCASE("ShiftLetter")
    {
        wxUIActionSimulator sim;
        sim.Char('Q', wxMOD_SHIFT);
        wxYield();

        CHECK_EQ( 2, m_win->GetKeyDownCount() );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyDownEvent(0),
                             ModKeyDown(WXK_SHIFT) );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyDownEvent(1),
                             KeyDesc('Q', wxMOD_SHIFT) );

        CHECK_EQ( 1, m_win->GetCharCount() );
        ASSERT_KEY_EVENT_IS( m_win->GetCharEvent(),
                             KeyDesc('Q', wxMOD_SHIFT) );

        CHECK_EQ( 2, m_win->GetKeyUpCount() );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyUpEvent(0),
                             KeyDesc('Q', wxMOD_SHIFT) );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyUpEvent(1),
                             ModKeyUp(WXK_SHIFT) );
    }

    SUBCASE("ShiftSpecial")
    {
        wxUIActionSimulator sim;
        sim.Char(WXK_F3, wxMOD_SHIFT);
        wxYield();

        CHECK_EQ( 2, m_win->GetKeyDownCount() );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyDownEvent(0),
                             ModKeyDown(WXK_SHIFT) );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyDownEvent(1),
                             KeyDesc(WXK_F3, wxMOD_SHIFT) );

        CHECK_EQ( 1, m_win->GetCharCount() );
        ASSERT_KEY_EVENT_IS( m_win->GetCharEvent(),
                             KeyDesc(WXK_F3, wxMOD_SHIFT) );

        CHECK_EQ( 2, m_win->GetKeyUpCount() );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyUpEvent(0),
                             KeyDesc(WXK_F3, wxMOD_SHIFT) );
        ASSERT_KEY_EVENT_IS( m_win->GetKeyUpEvent(1),
                             ModKeyUp(WXK_SHIFT) );
    }

    // TODO: Can we bypass this entirely without issue?
    //m_win->Destroy();
}

#endif // wxUSE_UIACTIONSIMULATOR
