///////////////////////////////////////////////////////////////////////////////
// Name:        tests/events/propagation.cpp
// Purpose:     Test events propagation
// Author:      Vadim Zeitlin
// Created:     2009-01-16
// Copyright:   (c) 2009 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#include "wx/app.h"
#include "wx/event.h"
#include "wx/scrolwin.h"
#include "wx/window.h"

#include "wx/docmdi.h"
#include "wx/frame.h"
#include "wx/menu.h"

#include "wx/scopeguard.h"
#include "wx/toolbar.h"
#include "wx/uiaction.h"

import WX.Cmn.Stopwatch;

// FIXME: Currently under OS X testing paint event doesn't work because neither
//        calling Refresh()+Update() nor even sending wxPaintEvent directly to
//        the window doesn't result in calls to its event handlers, so disable
//        some tests there. But this should be fixed and the tests reenabled
//        because wxPaintEvent propagation in wxScrolledWindow is a perfect
//        example of fragile code that could be broken under OS X.
#if !defined(__WXOSX__)
    #define CAN_TEST_PAINT_EVENTS
#endif

namespace
{

// this string will record the execution of all handlers
wxString g_str;

// a custom event
wxDEFINE_EVENT(TEST_EVT, wxCommandEvent);

// a custom event handler tracing the propagation of the events of the
// specified types
template <class Event>
class TestEvtHandlerBase : public wxEvtHandler
{
public:
    TestEvtHandlerBase(wxEventType evtType, char tag)
        : m_evtType(evtType),
          m_tag(tag)
    {
        Connect(evtType,
                static_cast<wxEventFunction>(&TestEvtHandlerBase::OnTest));
    }

    // override ProcessEvent() to confirm that it is called for all event
    // handlers in the chain
    bool ProcessEvent(wxEvent& event) override
    {
        if ( event.GetEventType() == m_evtType )
            g_str += 'o'; // "o" == "overridden"

        return wxEvtHandler::ProcessEvent(event);
    }

private:
    void OnTest(wxEvent& event)
    {
        g_str += m_tag;

        event.Skip();
    }

    const wxEventType m_evtType;
    const char m_tag;

    TestEvtHandlerBase(const TestEvtHandlerBase<Event>&)  = delete;
	TestEvtHandlerBase& operator=(const TestEvtHandlerBase<Event>&) = delete;
};

struct TestEvtHandler : TestEvtHandlerBase<wxCommandEvent>
{
    TestEvtHandler(char tag)
        : TestEvtHandlerBase<wxCommandEvent>(TEST_EVT, tag)
    {
    }
};

struct TestMenuEvtHandler : TestEvtHandlerBase<wxCommandEvent>
{
    TestMenuEvtHandler(char tag)
        : TestEvtHandlerBase<wxCommandEvent>(wxEVT_MENU, tag)
    {
    }
};

struct TestPaintEvtHandler : TestEvtHandlerBase<wxPaintEvent>
{
    TestPaintEvtHandler(char tag)
        : TestEvtHandlerBase<wxPaintEvent>(wxEVT_PAINT, tag)
    {
    }
};

// Another custom event handler, suitable for use with Connect().
struct TestEvtSink : wxEvtHandler
{
    TestEvtSink(char tag)
        : m_tag(tag)
    {
    }

    void Handle(wxEvent& event)
    {
        g_str += m_tag;

        event.Skip();
    }

    const char m_tag;

    TestEvtSink(const TestEvtSink&) = delete;
	TestEvtSink& operator=(const TestEvtSink&) = delete;
};

// a window handling the test event
class TestWindow : public wxWindow
{
public:
    TestWindow(wxWindow *parent, char tag)
        : wxWindow(parent, wxID_ANY),
          m_tag(tag)
    {
        Connect(TEST_EVT, wxCommandEventHandler(TestWindow::OnTest));
    }

private:
    void OnTest(wxCommandEvent& event)
    {
        g_str += m_tag;

        event.Skip();
    }

    const char m_tag;

    TestWindow(const TestWindow&) = delete;
	TestWindow& operator=(const TestWindow&) = delete;
};

// a scroll window handling paint event: we want to have a special test case
// for this because the event propagation is complicated even further than
// usual here by the presence of wxScrollHelperEvtHandler in the event handlers
// chain and the fact that OnDraw() virtual method must be called if EVT_PAINT
// is not handled
class TestScrollWindow : public wxScrolledWindow
{
public:
    TestScrollWindow(wxWindow *parent)
        : wxScrolledWindow(parent, wxID_ANY)
    {
        Connect(wxEVT_PAINT, wxPaintEventHandler(TestScrollWindow::OnPaint));
    }

    void GeneratePaintEvent()
    {
#ifdef __WXGTK__
        // We need to map the window, otherwise we're not going to get any
        // paint events for it.
        for ( wxStopWatch sw; sw.Time() < 50; )
            wxYield();

        // Ignore events generated during the initial mapping.
        g_str.clear();
#endif // __WXGTK__

        Refresh();
        Update();
    }

    void OnDraw([[maybe_unused]] wxDC& dc) override
    {
        g_str += 'D';   // draw
    }

private:
    void OnPaint(wxPaintEvent& event)
    {
        g_str += 'P';   // paint
        event.Skip();
    }

    TestScrollWindow(const TestScrollWindow&) = delete;
	TestScrollWindow& operator=(const TestScrollWindow&) = delete;
};

int DoFilterEvent(wxEvent& event)
{
    if ( event.GetEventType() == TEST_EVT ||
            event.GetEventType() == wxEVT_MENU )
        g_str += 'a';

    return -1;
}

bool DoProcessEvent(wxEvent& event)
{
    if ( event.GetEventType() == TEST_EVT ||
            event.GetEventType() == wxEVT_MENU )
        g_str += 'A';

    return false;
}

} // anonymous namespace

#if wxUSE_MENUS

// Create a menu bar with a single menu containing wxID_APPLY menu item and
// attach it to the specified frame.
wxMenu* CreateTestMenu(wxFrame* frame)
{
    wxMenu* const menu = new wxMenu;
    menu->Append(wxID_APPLY);
#if wxUSE_MENUBAR
    wxMenuBar* const mb = new wxMenuBar;
    mb->Append(menu, "&Menu");
    frame->SetMenuBar(mb);
#endif
    return menu;
}

// Helper for checking that the menu event processing resulted in the expected
// output from the handlers.
//
// Note that we trigger the menu event by sending it directly as this is more
// reliable than using wxUIActionSimulator and currently works in all ports as
// they all call wxMenuBase::SendEvent() from their respective menu event
// handlers.
#define ASSERT_MENU_EVENT_RESULT_FOR(cmd, menu, result) \
    g_str.clear();                                      \
    menu->SendEvent(cmd);                               \
    CHECK( g_str == result )

#define ASSERT_MENU_EVENT_RESULT(menu, result) \
    ASSERT_MENU_EVENT_RESULT_FOR(wxID_APPLY, menu, result)

#endif


// Helper function: get the event propagation level.
int GetPropagationLevel(wxEvent& e)
{
    const int level = e.StopPropagation();
    e.ResumePropagation(level);
    return level;
}

#if wxUSE_DOC_VIEW_ARCHITECTURE

// Minimal viable implementations of wxDocument and wxView.
class EventTestDocument : public wxDocument
{
public:
    EventTestDocument() { }

    wxDECLARE_DYNAMIC_CLASS(EventTestDocument);
};

class EventTestView : public wxView
{
public:
    EventTestView() { }

    void OnDraw(wxDC*) override { }

    wxDECLARE_DYNAMIC_CLASS(EventTestView);
};

wxIMPLEMENT_DYNAMIC_CLASS(EventTestDocument, wxDocument);
wxIMPLEMENT_DYNAMIC_CLASS(EventTestView, wxView);

#endif // wxUSE_DOC_VIEW_ARCHITECTURE

TEST_CASE("Event propagation tests.")
{
    SetFilterEventFunc(DoFilterEvent);
    SetProcessEventFunc(DoProcessEvent);

    g_str.clear();

    SUBCASE("One handler")
    {
        wxCommandEvent event(TEST_EVT);
        TestEvtHandler h1('1');
        h1.ProcessEvent(event);
        CHECK_EQ( "oa1A", g_str );
    }

    SUBCASE("Two handlers")
    {
        wxCommandEvent event(TEST_EVT);
        TestEvtHandler h1('1');
        TestEvtHandler h2('2');
        h1.SetNextHandler(&h2);
        h2.SetPreviousHandler(&h1);
        h1.ProcessEvent(event);
        CHECK_EQ( "oa1o2A", g_str );
    }

    SUBCASE("WindowWithoutHandler")
    {
        wxCommandEvent event(TEST_EVT);
        TestWindow * const parent = new TestWindow(wxTheApp->GetTopWindow(), 'p');
        wxON_BLOCK_EXIT_OBJ0( *parent, wxWindow::Destroy );

        TestWindow * const child = new TestWindow(parent, 'c');

        child->GetEventHandler()->ProcessEvent(event);
        CHECK_EQ( "acpA", g_str );
    }

    SUBCASE("WindowWithHandler")
    {
        wxCommandEvent event(TEST_EVT);
        TestWindow * const parent = new TestWindow(wxTheApp->GetTopWindow(), 'p');
        wxON_BLOCK_EXIT_OBJ0( *parent, wxWindow::Destroy );

        TestWindow * const child = new TestWindow(parent, 'c');

        TestEvtHandler h1('1');
        child->PushEventHandler(&h1);
        wxON_BLOCK_EXIT_OBJ1( *child, wxWindow::PopEventHandler, false );
        TestEvtHandler h2('2');
        child->PushEventHandler(&h2);
        wxON_BLOCK_EXIT_OBJ1( *child, wxWindow::PopEventHandler, false );

        child->HandleWindowEvent(event);
        CHECK_EQ( "oa2o1cpA", g_str );
    }

    SUBCASE("ForwardEvent")
    {
        // The idea of this test is to check that the events explicitly forwarded
        // to another event handler still get pre/post-processed as usual as this
        // used to be broken by the fixes trying to avoid duplicate processing.
        TestWindow * const win = new TestWindow(wxTheApp->GetTopWindow(), 'w');
        wxON_BLOCK_EXIT_OBJ0( *win, wxWindow::Destroy );

        TestEvtHandler h1('1');
        win->PushEventHandler(&h1);
        wxON_BLOCK_EXIT_OBJ1( *win, wxWindow::PopEventHandler, false );

        class ForwardEvtHandler : public wxEvtHandler
        {
        public:
            ForwardEvtHandler(wxEvtHandler& h) : m_h(&h) { }

            bool ProcessEvent(wxEvent& event) override
            {
                g_str += 'f';

                return m_h->ProcessEvent(event);
            }

        private:
            wxEvtHandler *m_h;
        } f(h1);

        // First send the event directly to f.
        wxCommandEvent event1(TEST_EVT);
        f.ProcessEvent(event1);
        CHECK_EQ( "foa1wA", g_str );
        g_str.clear();

        // And then also test sending it to f indirectly.
        wxCommandEvent event2(TEST_EVT);
        TestEvtHandler h2('2');
        h2.SetNextHandler(&f);
        h2.ProcessEvent(event2);
        CHECK_EQ( "oa2fo1wAA", g_str );
    }

    SUBCASE("ScrollWindowWithoutHandler")
    {
        TestWindow * const parent = new TestWindow(wxTheApp->GetTopWindow(), 'p');
        wxON_BLOCK_EXIT_OBJ0( *parent, wxWindow::Destroy );

        TestScrollWindow * const win = new TestScrollWindow(parent);

    #ifdef CAN_TEST_PAINT_EVENTS
        win->GeneratePaintEvent();
        CHECK_EQ( "PD", g_str );
    #endif

        g_str.clear();
        wxCommandEvent eventCmd(TEST_EVT);
        win->HandleWindowEvent(eventCmd);
        CHECK_EQ( "apA", g_str );
    }

    SUBCASE("ScrollWindowWithHandler")
    {
        TestWindow * const parent = new TestWindow(wxTheApp->GetTopWindow(), 'p');
        wxON_BLOCK_EXIT_OBJ0( *parent, wxWindow::Destroy );

        TestScrollWindow * const win = new TestScrollWindow(parent);

    #ifdef CAN_TEST_PAINT_EVENTS
        TestPaintEvtHandler h('h');
        win->PushEventHandler(&h);
        wxON_BLOCK_EXIT_OBJ1( *win, wxWindow::PopEventHandler, false );

        win->GeneratePaintEvent();
        CHECK_EQ( "ohPD", g_str );
    #endif

        g_str.clear();
        wxCommandEvent eventCmd(TEST_EVT);
        win->HandleWindowEvent(eventCmd);
        CHECK_EQ( "apA", g_str );
    }

#if wxUSE_MENUS
    SUBCASE("MenuEvent")
    {
        wxFrame* const frame = static_cast<wxFrame*>(wxTheApp->GetTopWindow());

        // Create a minimal menu bar.
        wxMenu* const menu = CreateTestMenu(frame);
#if wxUSE_MENUBAR
        wxMenuBar* const mb = menu->GetMenuBar();
        std::unique_ptr<wxMenuBar> ensureMenuBarDestruction(mb);
        wxON_BLOCK_EXIT_OBJ1(*frame, wxFrame::SetMenuBar, (wxMenuBar*)nullptr);
#endif
        // Check that wxApp gets the event exactly once.
        ASSERT_MENU_EVENT_RESULT(menu, "aA");


        // Check that the menu event handler is called.
        TestMenuEvtHandler hm('m'); // 'm' for "menu"
        menu->SetNextHandler(&hm);
        wxON_BLOCK_EXIT_OBJ1(*menu,
            wxEvtHandler::SetNextHandler, (wxEvtHandler*)nullptr);
        ASSERT_MENU_EVENT_RESULT(menu, "aomA");


        // Check that a handler can also be attached to a submenu.
        wxMenu* const submenu = new wxMenu;
        submenu->Append(wxID_ABOUT);
        menu->Append(wxID_ANY, "Submenu", submenu);

        TestMenuEvtHandler hs('s'); // 's' for "submenu"
        submenu->SetNextHandler(&hs);
        wxON_BLOCK_EXIT_OBJ1(*submenu,
            wxEvtHandler::SetNextHandler, (wxEvtHandler*)nullptr);
        ASSERT_MENU_EVENT_RESULT_FOR(wxID_ABOUT, submenu, "aosomA");

#if wxUSE_MENUBAR
        // Test that the event handler associated with the menu bar gets the event.
        TestMenuEvtHandler hb('b'); // 'b' for "menu Bar"
        mb->PushEventHandler(&hb);
        wxON_BLOCK_EXIT_OBJ1(*mb, wxWindow::PopEventHandler, false);

        ASSERT_MENU_EVENT_RESULT(menu, "aomobA");
#endif

        // Also test that the window to which the menu belongs gets the event.
        TestMenuEvtHandler hw('w'); // 'w' for "Window"
        frame->PushEventHandler(&hw);
        wxON_BLOCK_EXIT_OBJ1(*frame, wxWindow::PopEventHandler, false);

        ASSERT_MENU_EVENT_RESULT(menu, "aomobowA");
    }
#endif

#if wxUSE_UIACTIONSIMULATOR

    class ContextMenuTestWindow : public wxWindow
    {
    public:
        ContextMenuTestWindow(wxWindow* parent, char tag)
            : wxWindow(parent, wxID_ANY),
            m_tag(tag)
        {
            Connect(wxEVT_CONTEXT_MENU,
                wxContextMenuEventHandler(ContextMenuTestWindow::OnMenu));
        }

    private:
        void OnMenu(wxContextMenuEvent& event)
        {
            g_str += m_tag;

            event.Skip();
        }

        const char m_tag;

        ContextMenuTestWindow(const ContextMenuTestWindow&) = delete;
        ContextMenuTestWindow& operator=(const ContextMenuTestWindow&) = delete;
    };

    SUBCASE("ContextMenuEvent")
    {
        ContextMenuTestWindow* const
            parent = new ContextMenuTestWindow(wxTheApp->GetTopWindow(), 'p');
        wxON_BLOCK_EXIT_OBJ0(*parent, wxWindow::Destroy);

        ContextMenuTestWindow* const
            child = new ContextMenuTestWindow(parent, 'c');
        parent->SetSize(100, 100);
        child->SetSize(0, 0, 50, 50);
        child->SetFocus();

        wxUIActionSimulator sim;
        const wxPoint origin = parent->ClientToScreen(wxPoint(0, 0));

        // Right clicking in the child should generate an event for it and the
        // parent.
        g_str.clear();
        sim.MouseMove(origin + wxPoint(10, 10));
        sim.MouseClick(wxMOUSE_BTN_RIGHT);

        // At least with MSW, for WM_CONTEXTMENU to be synthesized by the system
        // from the right mouse click event, we must dispatch the mouse messages.
        wxYield();

        CHECK_EQ("cp", g_str);

        // For some unfathomable reason the test below sporadically fails in wxGTK
        // buildbot builds, so disable it there to avoid spurious failure reports.
#ifdef __WXGTK__
        if (IsAutomaticTest())
            return;
#endif // __WXGTK__

        // Right clicking outside the child should generate the event just in the
        // parent.
        g_str.clear();
        sim.MouseMove(origin + wxPoint(60, 60));
        sim.MouseClick(wxMOUSE_BTN_RIGHT);
        wxYield();
        CHECK_EQ("p", g_str);
    }

    SUBCASE("PropagationLevel")
    {
        wxSizeEvent se;
        CHECK_EQ(GetPropagationLevel(se), (int)wxEVENT_PROPAGATE_NONE);

        wxCommandEvent ce;
        CHECK_EQ(GetPropagationLevel(ce), (int)wxEVENT_PROPAGATE_MAX);

        wxCommandEvent ce2(ce);
        CHECK_EQ(GetPropagationLevel(ce2), (int)wxEVENT_PROPAGATE_MAX);

        wxCommandEvent ce3;
        ce3.ResumePropagation(17);
        CHECK_EQ(GetPropagationLevel(ce3), 17);

        wxCommandEvent ce4(ce3);
        CHECK_EQ(GetPropagationLevel(ce4), 17);
    }

#endif // wxUSE_UIACTIONSIMULATOR

#if wxUSE_DOC_VIEW_ARCHITECTURE
    SUBCASE("DocView")
    {
        // Set up the parent frame and its menu bar.
        wxDocManager docManager;

        std::unique_ptr<wxDocMDIParentFrame>
            parent(new wxDocMDIParentFrame(&docManager, nullptr, wxID_ANY, "Parent"));

        wxMenu* const menu = CreateTestMenu(parent.get());


        // Set up the event handlers.
        TestEvtSink sinkDM('m');
        docManager.Connect(wxEVT_MENU,
            wxEventHandler(TestEvtSink::Handle), nullptr, &sinkDM);

        TestEvtSink sinkParent('p');
        parent->Connect(wxEVT_MENU,
            wxEventHandler(TestEvtSink::Handle), nullptr, &sinkParent);


        // Check that wxDocManager and wxFrame get the event in order.
        ASSERT_MENU_EVENT_RESULT(menu, "ampA");


        // Now check what happens if we have an active document.
        wxDocTemplate docTemplate(&docManager, "Test", "", "", "",
            "Test Document", "Test View",
            wxCLASSINFO(EventTestDocument),
            wxCLASSINFO(EventTestView));
        wxDocument* const doc = docTemplate.CreateDocument("");
        wxView* const view = doc->GetFirstView();

        std::unique_ptr<wxMDIChildFrame>
            child(new wxDocMDIChildFrame(doc, view, parent.get(), wxID_ANY, "Child"));

        wxMenu* const menuChild = CreateTestMenu(child.get());

        // Ensure that the child that we've just created is the active one.
        child->Activate();

#ifdef __WXGTK__
        // There are a lot of hacks related to child frame menu bar handling in
        // wxGTK and, in particular, the code in src/gtk/mdi.cpp relies on getting
        // idle events to really put everything in place. Moreover, as wxGTK uses
        // GtkNotebook as its MDI pages container, the frame must be shown for all
        // this to work as gtk_notebook_set_current_page() doesn't do anything if
        // called for a hidden window (this incredible fact cost me quite some time
        // to find empirically -- only to notice its confirmation in GTK+
        // documentation immediately afterwards). So just do whatever it takes to
        // make things work "as usual".
        child->Show();
        parent->Show();
        wxYield();
#endif // __WXGTK__

        TestEvtSink sinkDoc('d');
        doc->Connect(wxEVT_MENU,
            wxEventHandler(TestEvtSink::Handle), nullptr, &sinkDoc);

        TestEvtSink sinkView('v');
        view->Connect(wxEVT_MENU,
            wxEventHandler(TestEvtSink::Handle), nullptr, &sinkView);

        TestEvtSink sinkChild('c');
        child->Connect(wxEVT_MENU,
            wxEventHandler(TestEvtSink::Handle), nullptr, &sinkChild);

        // Check that wxDocument, wxView, wxDocManager, child frame and the parent
        // get the event in order.
        ASSERT_MENU_EVENT_RESULT(menuChild, "advmcpA");


#if wxUSE_TOOLBAR
        // Also check that toolbar events get forwarded to the active child.
        wxToolBar* const tb = parent->CreateToolBar(wxTB_NOICONS);
        tb->AddTool(wxID_APPLY, "Apply", wxNullBitmap);
        tb->Realize();

        // As in CheckMenuEvent(), use toolbar method actually sending the event
        // instead of bothering with wxUIActionSimulator which would have been
        // trickier.
        g_str.clear();
        tb->OnLeftClick(wxID_APPLY, true /* doesn't matter */);

        CHECK_EQ("advmcpA", g_str);
#endif // wxUSE_TOOLBAR
    }

#endif // wxUSE_DOC_VIEW_ARCHITECTURE

    SetFilterEventFunc(nullptr);
    SetProcessEventFunc(nullptr);
}
