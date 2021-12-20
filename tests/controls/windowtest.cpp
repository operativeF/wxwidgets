///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/windowtest.cpp
// Purpose:     wxWindow unit test
// Author:      Steven Lamerton
// Created:     2010-07-10
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "wx/app.h"
#include "wx/window.h"
#include "wx/button.h"

#include "wx/uiaction.h"
#include "wx/caret.h"
#include "wx/cshelp.h"

#include "wx/stopwatch.h"
#include "wx/tooltip.h"

#include "asserthelper.h"
#include "testableframe.h"
#include "testwindow.h"

import WX.Core.Sizer;

import WX.Test.Prec;

class WindowTestCase
{
public:
    WindowTestCase()
        : m_window(new wxWindow(wxTheApp->GetTopWindow(), wxID_ANY))
    {
    #ifdef __WXGTK3__
        // Without this, when running this test suite solo it succeeds,
        // but not when running it together with the other tests !!
        // Not needed when run under Xvfb display.
        for ( wxStopWatch sw; sw.Time() < 50; )
            wxYield();
    #endif
    }

    ~WindowTestCase()
    {
        wxTheApp->GetTopWindow()->DestroyChildren();
    }

protected:
    wxWindow* const m_window;

    WindowTestCase(const WindowTestCase&) = delete;
	WindowTestCase& operator=(const WindowTestCase&) = delete;
};

TEST_CASE_FIXTURE(WindowTestCase, "Window::ShowHideEvent")
{
#if defined(__WXMSW__)
    EventCounter show(m_window, wxEVT_SHOW);

    CHECK(m_window->IsShown());

    m_window->Show(false);

    CHECK(!m_window->IsShown());

    m_window->Show();

    CHECK(m_window->IsShown());

    CHECK( show.GetCount() == 2 );
#endif // __WXMSW__
}

TEST_CASE_FIXTURE(WindowTestCase, "Window::KeyEvent")
{
#if wxUSE_UIACTIONSIMULATOR
    if ( !EnableUITests() )
        return;

    EventCounter keydown(m_window, wxEVT_KEY_DOWN);
    EventCounter keyup(m_window, wxEVT_KEY_UP);
    EventCounter keychar(m_window, wxEVT_CHAR);

    wxUIActionSimulator sim;

    m_window->SetFocus();
    wxYield();

    sim.Text("text");
    sim.Char(WXK_SHIFT);
    wxYield();

    CHECK( keydown.GetCount() == 5 );
    CHECK( keyup.GetCount() == 5 );
    CHECK( keychar.GetCount() == 4 );
#endif
}

TEST_CASE_FIXTURE(WindowTestCase, "Window::FocusEvent")
{
#ifndef __WXOSX__
    if ( IsAutomaticTest() )
    {
        // Skip this test when running under buildbot, it fails there for
        // unknown reason and this failure can't be reproduced locally.
        return;
    }

    EventCounter setfocus(m_window, wxEVT_SET_FOCUS);
    EventCounter killfocus(m_window, wxEVT_KILL_FOCUS);

    m_window->SetFocus();

    CHECK(setfocus.WaitEvent(500));
    CHECK_FOCUS_IS( m_window );

    wxButton* button = new wxButton(wxTheApp->GetTopWindow(), wxID_ANY);

    wxYield();
    button->SetFocus();

    CHECK( killfocus.GetCount() == 1 );
    CHECK(!m_window->HasFocus());
#endif
}

TEST_CASE_FIXTURE(WindowTestCase, "Window::Mouse")
{
    wxCursor cursor(wxCURSOR_CHAR);
    m_window->SetCursor(cursor);

    CHECK(m_window->GetCursor().IsOk());

#if wxUSE_CARET
    CHECK(!m_window->GetCaret());

    std::unique_ptr<wxCaret> caret;

    // Try creating the caret in two different, but normally equivalent, ways.
    SUBCASE("Caret 1-step")
    {
        caret = std::make_unique<wxCaret>(m_window, wxSize{16, 16});
    }

    SUBCASE("Caret 2-step")
    {
        caret = std::make_unique<wxCaret>();
        caret->Create(m_window, wxSize{16, 16});
    }

    m_window->SetCaret(std::move(caret));

    CHECK(m_window->GetCaret()->IsOk());
#endif

    m_window->CaptureMouse();

    CHECK(m_window->HasCapture());

    m_window->ReleaseMouse();

    CHECK(!m_window->HasCapture());
}

TEST_CASE_FIXTURE(WindowTestCase, "Window::Properties")
{
    m_window->SetLabel("label");

    CHECK( m_window->GetLabel() == "label" );

    m_window->SetName("name");

    CHECK( m_window->GetName() == "name" );

    //As we used wxID_ANY we should have a negative id
    CHECK(m_window->GetId() < 0);

    m_window->SetId(wxID_HIGHEST + 10);

    CHECK( m_window->GetId() == wxID_HIGHEST + 10 );
}

#if wxUSE_TOOLTIPS
TEST_CASE_FIXTURE(WindowTestCase, "Window::ToolTip")
{
    CHECK(!m_window->GetToolTip());
    CHECK( m_window->GetToolTipText().empty() );

    m_window->SetToolTip("text tip");

    CHECK( m_window->GetToolTipText() == "text tip" );

    m_window->UnsetToolTip();

    CHECK(!m_window->GetToolTip());
    CHECK( m_window->GetToolTipText().empty() );

    wxToolTip* tip = new wxToolTip("other tip");

    m_window->SetToolTip(tip);

    CHECK( m_window->GetToolTip() == tip );
    CHECK( m_window->GetToolTipText() == "other tip" );
}
#endif // wxUSE_TOOLTIPS

TEST_CASE_FIXTURE(WindowTestCase, "Window::Help")
{
#if wxUSE_HELP
    wxHelpProvider::Set(new wxSimpleHelpProvider());

    CHECK( m_window->GetHelpText().empty() );

    m_window->SetHelpText("helptext");

    CHECK( m_window->GetHelpText() == "helptext" );
#endif
}

TEST_CASE_FIXTURE(WindowTestCase, "Window::Parent")
{
    CHECK( m_window->GetGrandParent() == static_cast<wxWindow*>(nullptr) );
    CHECK( m_window->GetParent() == wxTheApp->GetTopWindow() );
}

TEST_CASE_FIXTURE(WindowTestCase, "Window::Siblings")
{
    CHECK( m_window->wxGetNextSibling() == static_cast<wxWindow*>(nullptr) );
    CHECK( m_window->wxGetPrevSibling() == static_cast<wxWindow*>(nullptr) );

    wxWindow* newwin = new wxWindow(wxTheApp->GetTopWindow(), wxID_ANY);

    CHECK( m_window->wxGetNextSibling() == newwin );
    CHECK( m_window->wxGetPrevSibling() == static_cast<wxWindow*>(nullptr) );

    CHECK( newwin->wxGetNextSibling() == static_cast<wxWindow*>(nullptr) );
    CHECK( newwin->wxGetPrevSibling() == m_window );

    wxDELETE(newwin);
}

TEST_CASE_FIXTURE(WindowTestCase, "Window::Children")
{
    CHECK( m_window->GetChildren().GetCount() == 0 );

    wxWindow* child1 = new wxWindow(m_window, wxID_ANY);

    CHECK( m_window->GetChildren().GetCount() == 1 );

    m_window->RemoveChild(child1);

    CHECK( m_window->GetChildren().GetCount() == 0 );

    child1->SetId(wxID_HIGHEST + 1);
    child1->SetName("child1");

    m_window->AddChild(child1);

    CHECK( m_window->GetChildren().GetCount() == 1 );
    CHECK( m_window->wxFindWindow(wxID_HIGHEST + 1) == child1 );
    CHECK( m_window->wxFindWindow("child1") == child1 );

    m_window->DestroyChildren();

    CHECK( m_window->GetChildren().GetCount() == 0 );
}

TEST_CASE_FIXTURE(WindowTestCase, "Window::Focus")
{
#ifndef __WXOSX__
    CHECK(!m_window->HasFocus());

    if ( m_window->AcceptsFocus() )
    {
        m_window->SetFocus();
        CHECK_FOCUS_IS(m_window);
    }

    //Set the focus back to the main window
    wxTheApp->GetTopWindow()->SetFocus();

    if ( m_window->AcceptsFocusFromKeyboard() )
    {
        m_window->SetFocusFromKbd();
        CHECK_FOCUS_IS(m_window);
    }
#endif
}

TEST_CASE_FIXTURE(WindowTestCase, "Window::Positioning")
{
    //Some basic tests for consistency
    wxPoint winPos = m_window->GetPosition();

    CHECK( m_window->GetPosition().x == winPos.x );
    CHECK( m_window->GetPosition().y == winPos.y );
    CHECK( m_window->GetRect().GetTopLeft() == m_window->GetPosition() );

    wxPoint scrPos = m_window->GetScreenPosition();

    CHECK( m_window->GetScreenPosition().x == scrPos.x );
    CHECK( m_window->GetScreenPosition().y == scrPos.y );
    CHECK( m_window->GetScreenRect().GetTopLeft() == m_window->GetScreenPosition() );
}

TEST_CASE_FIXTURE(WindowTestCase, "Window::PositioningBeyondShortLimit")
{
#ifdef __WXMSW__
    //Positioning under MSW is limited to short relative coordinates

    //
    //Test window creation beyond SHRT_MAX
    int commonDim = 10;
    wxWindow* w = new wxWindow(m_window, wxID_ANY,
                               wxPoint(0, SHRT_MAX + commonDim),
                               wxSize(commonDim, commonDim));
    CHECK( w->GetPosition().y == SHRT_MAX + commonDim );

    w->Move(wxPoint{0, 0});

    //
    //Test window moving beyond SHRT_MAX
    w->Move(wxPoint{0, SHRT_MAX + commonDim});
    CHECK( w->GetPosition().y == SHRT_MAX + commonDim );

    //
    //Test window moving below SHRT_MIN
    w->Move(wxPoint{0, SHRT_MIN - commonDim});
    CHECK( w->GetPosition().y == SHRT_MIN - commonDim );

    //
    //Test deferred move beyond SHRT_MAX
    m_window->SetVirtualSize(wxSize{-1, SHRT_MAX + 2 * commonDim});
    wxWindow* bigWin = new wxWindow(m_window, wxID_ANY, wxDefaultPosition,
                                    //size is also limited by SHRT_MAX
                                    wxSize(commonDim, SHRT_MAX));
    wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(bigWin);
    sizer->AddSpacer(commonDim); //add some space to go beyond SHRT_MAX
    sizer->Add(w);
    m_window->SetSizer(sizer);
    m_window->Layout();
    CHECK( w->GetPosition().y == SHRT_MAX + commonDim );
#endif
}

TEST_CASE_FIXTURE(WindowTestCase, "Window::Show")
{
    CHECK(m_window->IsShown());

    m_window->Hide();

    CHECK_FALSE(m_window->IsShown());

    m_window->Show();

    CHECK(m_window->IsShown());

    m_window->Show(false);

    CHECK_FALSE(m_window->IsShown());

    m_window->ShowWithEffect(wxShowEffect::Blend);

    CHECK(m_window->IsShown());

    m_window->HideWithEffect(wxShowEffect::Blend);

    CHECK_FALSE(m_window->IsShown());
}

TEST_CASE_FIXTURE(WindowTestCase, "Window::Enable")
{
    CHECK(m_window->IsEnabled());

    m_window->Disable();

    CHECK_FALSE(m_window->IsEnabled());

    m_window->Enable();

    CHECK(m_window->IsEnabled());

    m_window->Enable(false);

    CHECK_FALSE(m_window->IsEnabled());
    m_window->Enable();


    wxWindow* const child = new wxWindow(m_window, wxID_ANY);
    CHECK(child->IsEnabled());
    CHECK(child->IsThisEnabled());

    m_window->Disable();
    CHECK_FALSE(child->IsEnabled());
    CHECK(child->IsThisEnabled());

    child->Disable();
    CHECK_FALSE(child->IsEnabled());
    CHECK_FALSE(child->IsThisEnabled());

    m_window->Enable();
    CHECK_FALSE(child->IsEnabled());
    CHECK_FALSE(child->IsThisEnabled());

    child->Enable();
    CHECK(child->IsEnabled());
    CHECK(child->IsThisEnabled());
}

TEST_CASE_FIXTURE(WindowTestCase, "Window::FindWindowBy")
{
    m_window->SetId(wxID_HIGHEST + 1);
    m_window->SetName("name");
    m_window->SetLabel("label");

    CHECK( wxWindow::FindWindowById(wxID_HIGHEST + 1) == m_window );
    CHECK( wxWindow::FindWindowByName("name") == m_window );
    CHECK( wxWindow::FindWindowByLabel("label") == m_window );

    CHECK( wxWindow::FindWindowById(wxID_HIGHEST + 3) == nullptr );
    CHECK( wxWindow::FindWindowByName("noname") == nullptr );
    CHECK( wxWindow::FindWindowByLabel("nolabel") == nullptr );
}

TEST_CASE_FIXTURE(WindowTestCase, "Window::SizerErrors")
{
    wxWindow* const child = new wxWindow(m_window, wxID_ANY);
    const auto sizer1 = std::make_unique<wxBoxSizer>(wxHORIZONTAL);
    const auto sizer2 = std::make_unique<wxBoxSizer>(wxHORIZONTAL);

    REQUIRE_NOTHROW( sizer1->Add(child) );
    CHECK_THROWS_AS( sizer1->Add(child), TestAssertFailure );
    CHECK_THROWS_AS( sizer2->Add(child), TestAssertFailure );

    CHECK_NOTHROW( sizer1->Detach(child) );
    CHECK_NOTHROW( sizer2->Add(child) );

    REQUIRE_NOTHROW( delete child );
}
