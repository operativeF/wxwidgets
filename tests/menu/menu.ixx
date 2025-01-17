///////////////////////////////////////////////////////////////////////////////
// Name:        tests/menu/menu.cpp
// Purpose:     wxMenu unit test
// Author:      wxWidgets team
// Created:     2010-11-10
// Copyright:   (c) 2010 wxWidgets team
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/app.h"
#include "wx/frame.h"
#include "wx/menu.h"
#include "wx/textctrl.h"
#include "wx/translation.h"
#include "wx/uiaction.h"
#include "wx/utils.h"

export module WX.Test.Menu;

import WX.Test.Prec;
import WX.MetaTest;

import <cstdarg>;
import <string>;

// ----------------------------------------------------------------------------
// helper
// ----------------------------------------------------------------------------

#if wxUSE_MENUBAR

namespace ut = boost::ut;

namespace
{

enum
{
    MenuTestCase_Foo = 10000,
    MenuTestCase_SelectAll,
    MenuTestCase_Bar,
    MenuTestCase_First
};

void PopulateMenu(wxMenu* menu, const std::string& name,  size_t& itemcount)
{
    // Start at item 1 to make it human-readable ;)
    for (int n=1; n<6; ++n, ++itemcount)
    {
        std::string label = fmt::format("{}{:d}", name, n);
        menu->Append(MenuTestCase_First + itemcount, label, label + " help string");
    }
}

void RecursivelyCountMenuItems(const wxMenu* menu, size_t& count)
{
    using namespace ut;

    expect( menu );

    count += menu->GetMenuItemCount();
    for (size_t n=0; n < menu->GetMenuItemCount(); ++n)
    {
        wxMenuItem* item = menu->FindItemByPosition(n);
        if (item->IsSubMenu())
        {
            RecursivelyCountMenuItems(item->GetSubMenu(), count);
        }
    }
}

} // anon namespace

#if wxUSE_INTL

static std::string GetTranslatedString(const wxTranslations& trans, const std::string& s)
{
    const std::string* t = trans.GetTranslatedString(s);
    return t ? *t : s;
}

#endif

#if wxUSE_UIACTIONSIMULATOR

// In C++98 this class can't be defined inside Events() method, unfortunately,
// as its OnMenu() method wouldn't be usable with template Bind() then.
class MenuEventHandler : public wxEvtHandler
{
public:
    MenuEventHandler(wxWindow* win)
        : m_win(win)
    {
        m_win->Bind(wxEVT_MENU, &MenuEventHandler::OnMenu, this);
    }

    virtual ~MenuEventHandler()
    {
        m_win->Unbind(wxEVT_MENU, &MenuEventHandler::OnMenu, this);
    }

    std::unique_ptr<wxCommandEvent> GetEvent()
    {
        using namespace ut;

        expect(m_event != nullptr);

        return std::move(m_event);
    }

    bool GotEvent() const
    {
        return m_event != nullptr;
    }

private:
    void OnMenu(wxCommandEvent& event)
    {
        using namespace ut;

        expect(m_event == nullptr);

        m_event.reset(dynamic_cast<wxCommandEvent*>(event.Clone().release()));
    }

    wxWindow* const m_win;
    std::unique_ptr<wxCommandEvent> m_event;
};

#endif // wxUSE_UIACTIONSIMULATOR


ut::suite MenuTests = []
{
    using namespace ut;

    auto m_frame = std::make_unique<wxFrame>(wxTheApp->GetTopWindow(), wxID_ANY, "test frame");

    // Holds the number of menuitems contained in all the menus
    size_t m_itemCount;

    // Store here the id of a known submenu item, to be searched for later
    int m_submenuItemId;

    // and a sub-submenu item
    int m_subsubmenuItemId;

    std::vector<std::string> m_menuLabels;

    // The menu containing the item with MenuTestCase_Bar id.
    wxMenu* m_menuWithBar;

    wxMenu* fileMenu = new wxMenu;
    wxMenu* helpMenu = new wxMenu;
    wxMenu* subMenu = new wxMenu;
    wxMenu* subsubMenu = new wxMenu;

    m_itemCount = 0;

    PopulateMenu(subsubMenu, "Subsubmenu item ", m_itemCount);

    // Store one of its IDs for later
    m_subsubmenuItemId = MenuTestCase_First + m_itemCount - 2;

    PopulateMenu(subMenu, "Submenu item ", m_itemCount);

    // Store one of its IDs for later
    m_submenuItemId = MenuTestCase_First + m_itemCount - 2;

    subMenu->AppendSubMenu(subsubMenu, "Subsubmen&u", "Test a subsubmenu");
    m_itemCount++;

    // Check GetTitle() returns the correct string _before_ appending to the bar
    fileMenu->SetTitle("&Foo\tCtrl-F");
    expect("&Foo\tCtrl-F" == fileMenu->GetTitle());

    PopulateMenu(fileMenu, "Filemenu item ", m_itemCount);

    fileMenu->Append(MenuTestCase_Foo, "&Foo\tCtrl-F", "Test item to be found");
    m_itemCount++;
    fileMenu->Append(MenuTestCase_SelectAll, "Select &all\tCtrl-A",
        "Accelerator conflicting with wxTextCtrl");
    m_itemCount++;


    PopulateMenu(helpMenu, "Helpmenu item ", m_itemCount);
    helpMenu->Append(MenuTestCase_Bar, "Bar\tF1");
    m_itemCount++;
    m_menuWithBar = helpMenu;
    helpMenu->AppendSubMenu(subMenu, "Sub&menu", "Test a submenu");
    m_itemCount++;

    // Use an arraystring here, to help with future tests
    m_menuLabels.push_back("&File");
    m_menuLabels.push_back("&Help");

    wxMenuBar* menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, m_menuLabels[0]);
    menuBar->Append(helpMenu, m_menuLabels[1]);
    m_frame->SetMenuBar(menuBar);

    "FindInMenubar"_test = [&]
    {
        wxMenuBar* bar = m_frame->GetMenuBar();

        // Find by name:
        expect( bar->FindMenu("File") != wxNOT_FOUND );
        expect( bar->FindMenu("&File") != wxNOT_FOUND );
        expect( bar->FindMenu("&Fail") == wxNOT_FOUND );

        // Find by menu name plus item name:
        expect( bar->FindMenuItem("File", "Foo") != wxNOT_FOUND );
        expect( bar->FindMenuItem("&File", "&Foo") != wxNOT_FOUND );
        // and using the menu label
        int index = bar->FindMenu("&File");
        expect( index != wxNOT_FOUND );
        std::string menulabel = bar->GetMenuLabel(index);
        expect( bar->FindMenuItem(menulabel, "&Foo") != wxNOT_FOUND );
        // and title
        std::string menutitle = bar->GetMenu(index)->GetTitle();
        expect( bar->FindMenuItem(menutitle, "&Foo") != wxNOT_FOUND );

        // Find by position:
        for (size_t n=0; n < bar->GetMenuCount(); ++n)
        {
            expect( bar->GetMenu(n) );
        }

        // Find by id:
        wxMenu* menu = nullptr;
        wxMenuItem* item = nullptr;
        item = bar->FindItem(MenuTestCase_Foo, &menu);
        expect( item );
        expect( menu );
        // Check that the correct menu was found
        expect( menu->FindChildItem(MenuTestCase_Foo) );

        // Find submenu item:
        item = bar->FindItem(m_submenuItemId, &menu);
        expect( item );
        expect( menu );
        // and, for completeness, a subsubmenu one:
        item = bar->FindItem(m_subsubmenuItemId, &menu);
        expect( item );
        expect( menu );
    };

    "FindInMenu"_test = [&]
    {
        wxMenuBar* bar = m_frame->GetMenuBar();

        // Find by name:
        wxMenu* menuFind = bar->GetMenu(0);
        expect( menuFind->FindItem("Foo") != wxNOT_FOUND );
        expect( menuFind->FindItem("&Foo") != wxNOT_FOUND );
        // and for submenus
        wxMenu* menuHelp = bar->GetMenu(1);
        expect( menuHelp->FindItem("Submenu") != wxNOT_FOUND );
        expect( menuHelp->FindItem("Sub&menu") != wxNOT_FOUND );

        // Find by position:
        size_t n;
        for (n=0; n < menuHelp->GetMenuItemCount(); ++n)
        {
            expect( menuHelp->FindItemByPosition(n) );
        }

        // Find by id:
        expect( menuHelp->FindItem(MenuTestCase_Bar) );
        expect( !menuHelp->FindItem(MenuTestCase_Foo) );

        for (n=0; n < menuHelp->GetMenuItemCount(); ++n)
        {
            size_t locatedAt;
            wxMenuItem* itemByPos = menuHelp->FindItemByPosition(n);
            expect( itemByPos );
            wxMenuItem* itemById = menuHelp->FindChildItem(itemByPos->GetId(), &locatedAt);
            expect( itemByPos == itemById );
            expect( locatedAt == n );
        }

        // Find submenu item:
        for (n=0; n < menuHelp->GetMenuItemCount(); ++n)
        {
            wxMenuItem* item = menuHelp->FindItemByPosition(n);
            if (item->IsSubMenu())
            {
                wxMenu* submenu;
                wxMenuItem* submenuItem = menuHelp->FindItem(m_submenuItemId, &submenu);
                expect( submenuItem );
                expect( item->GetSubMenu() == submenu );
            }
        }
    };

    "EnableTop"_test = [&]
    {
        wxMenuBar* const bar = m_frame->GetMenuBar();
        expect( bar->IsEnabledTop(0) );
        bar->EnableTop( 0, false );
        expect( !bar->IsEnabledTop(0) );
        bar->EnableTop( 0, true );
        expect( bar->IsEnabledTop(0) );
    };

    "Count"_test = [&]
    {
        wxMenuBar* bar = m_frame->GetMenuBar();
        // I suppose you could call this "counting menubars" :)
        expect( bar );

        expect( bar->GetMenuCount() == 2 );

        size_t count = 0;
        for (size_t n=0; n < bar->GetMenuCount(); ++n)
        {
            RecursivelyCountMenuItems(bar->GetMenu(n), count);
        }

        expect( count == m_itemCount );
    };

    "Labels"_test = [&]
    {
        wxMenuBar* bar = m_frame->GetMenuBar();
        expect( bar );
        wxMenu* filemenu;
        wxMenuItem* itemFoo = bar->FindItem(MenuTestCase_Foo, &filemenu);
        expect( itemFoo );
        expect( filemenu );

        // These return labels including mnemonics/accelerators:

        // wxMenuBar
        expect( "&File" == bar->GetMenuLabel(0) );
        expect( "&Foo\tCtrl-F" == bar->GetLabel(MenuTestCase_Foo) );

        // wxMenu
        expect( "&File" == filemenu->GetTitle() );
        expect( "&Foo\tCtrl-F" == filemenu->GetLabel(MenuTestCase_Foo) );

        // wxMenuItem
        expect( "&Foo\tCtrl-F" == itemFoo->GetItemLabel() );

        // These return labels stripped of mnemonics/accelerators:

        // wxMenuBar
        expect( "File" == bar->GetMenuLabelText(0) );

        // wxMenu
        expect( "Foo" == filemenu->GetLabelText(MenuTestCase_Foo) );

        // wxMenuItem
        expect( "Foo" == itemFoo->GetItemLabelText() );
        expect( "Foo" == wxMenuItem::GetLabelText("&Foo\tCtrl-F") );
    };


#if wxUSE_INTL
    "TranslatedMnemonics"_test = [&]
    {
        // Check that appended mnemonics are correctly stripped;
        // see https://trac.wxwidgets.org/ticket/16736
        wxTranslations trans;
        trans.SetLanguage(wxLANGUAGE_JAPANESE);
        wxFileTranslationsLoader::AddCatalogLookupPathPrefix("./intl");
        expect(trans.AddCatalog("internat"));

        // Check the translation is being used:
        expect(std::string("&File") != GetTranslatedString(trans, "&File"));

        std::string filemenu = m_frame->GetMenuBar()->GetMenuLabel(0);
        expect
        (
            wxStripMenuCodes(GetTranslatedString(trans, "&File"), wxStrip_Menu) ==
            wxStripMenuCodes(GetTranslatedString(trans, filemenu), wxStrip_Menu)
        );

        // Test strings that have shortcuts. Duplicate non-mnemonic translations
        // exist for both "Edit" and "View", for ease of comparison
        expect
        (
            GetTranslatedString(trans, "Edit") ==
            wxStripMenuCodes(GetTranslatedString(trans, "E&dit\tCtrl+E"), wxStrip_Menu)
        );

        // "Vie&w" also has a space before the (&W)
        expect
        (
            GetTranslatedString(trans, "View") ==
            wxStripMenuCodes(GetTranslatedString(trans, "Vie&w\tCtrl+V"), wxStrip_Menu)
        );

        // Test a 'normal' mnemonic too: the translation is "Preten&d"
        expect
        (
            "Pretend" ==
            wxStripMenuCodes(GetTranslatedString(trans, "B&ogus"), wxStrip_Menu)
        );
    };
#endif // wxUSE_INTL

    "RadioItems"_test = [&]
    {
        wxMenuBar* const bar = m_frame->GetMenuBar();
        wxMenu* const menu = new wxMenu;
        bar->Append(menu, "&Radio");

        // Adding consecutive radio items creates a radio group.
        menu->AppendRadioItem(MenuTestCase_First, "Radio 0");
        menu->AppendRadioItem(MenuTestCase_First + 1, "Radio 1");

        // First item of a radio group is checked by default.
        expect(menu->IsChecked(MenuTestCase_First));

        // Subsequent items in a group are not checked.
        expect(!menu->IsChecked(MenuTestCase_First + 1));

#ifdef __WXQT__
        WARN("Radio check test does not work under Qt");
#else
        // Checking the second one make the first one unchecked however.
        menu->Check(MenuTestCase_First + 1, true);
        expect(!menu->IsChecked(MenuTestCase_First));
        expect(menu->IsChecked(MenuTestCase_First + 1));
        menu->Check(MenuTestCase_First, true);
#endif

        // Adding more radio items after a separator creates another radio group...
        menu->AppendSeparator();
        menu->AppendRadioItem(MenuTestCase_First + 2, "Radio 2");
        menu->AppendRadioItem(MenuTestCase_First + 3, "Radio 3");
        menu->AppendRadioItem(MenuTestCase_First + 4, "Radio 4");

        // ... which is independent from the first one.
        expect(menu->IsChecked(MenuTestCase_First));
        expect(menu->IsChecked(MenuTestCase_First + 2));

#ifdef __WXQT__
        WARN("Radio check test does not work under Qt");
#else
        menu->Check(MenuTestCase_First + 3, true);
        expect(menu->IsChecked(MenuTestCase_First + 3));
        expect(!menu->IsChecked(MenuTestCase_First + 2));

        expect(menu->IsChecked(MenuTestCase_First));
        menu->Check(MenuTestCase_First + 2, true);
#endif

        // Insert an item in the middle of an existing radio group.
        menu->InsertRadioItem(4, MenuTestCase_First + 5, "Radio 5");
        expect(menu->IsChecked(MenuTestCase_First + 2));
        expect(!menu->IsChecked(MenuTestCase_First + 5));

#ifdef __WXQT__
        WARN("Radio check test does not work under Qt");
#else
        menu->Check(MenuTestCase_First + 5, true);
        expect(!menu->IsChecked(MenuTestCase_First + 3));

        menu->Check(MenuTestCase_First + 3, true);
#endif

        // Prepend a couple of items before the first group.
        menu->PrependRadioItem(MenuTestCase_First + 6, "Radio 6");
        menu->PrependRadioItem(MenuTestCase_First + 7, "Radio 7");
        expect(!menu->IsChecked(MenuTestCase_First + 6));
        expect(!menu->IsChecked(MenuTestCase_First + 7));

#ifdef __WXQT__
        WARN("Radio check test does not work under Qt");
#else
        menu->Check(MenuTestCase_First + 7, true);
        expect(!menu->IsChecked(MenuTestCase_First + 1));


        // Check that the last radio group still works as expected.
        menu->Check(MenuTestCase_First + 4, true);
        expect(!menu->IsChecked(MenuTestCase_First + 5));
#endif
    };

    "RemoveAdd"_test = [&]
    {
        wxMenuBar* bar = m_frame->GetMenuBar();

        wxMenu* menu0 = bar->GetMenu(0);
        wxMenu* menu1 = bar->GetMenu(1);
        wxMenuItem* item = new wxMenuItem(menu0, MenuTestCase_Foo + 100, "t&ext\tCtrl-E");

        menu0->Insert(0, item);

        expect(menu0->FindItemByPosition(0) == item);
        menu0->Remove(item);

        expect(menu0->FindItemByPosition(0) != item);
        menu1->Insert(0, item);

        expect(menu1->FindItemByPosition(0) == item);
        menu1->Remove(item);

        expect(menu1->FindItemByPosition(0) != item);
        menu0->Insert(0, item);

        expect(menu0->FindItemByPosition(0) == item);
        menu0->Delete(item);
    };

    // FIXME: Add actual expectations
    "ChangeBitmap"_test = [&]
    {
        wxMenu* menu = new wxMenu;

        wxMenuItem* item = new wxMenuItem(menu, wxID_ANY, "Item");
        menu->Append(item);

        // On Windows Vista (and later) calling SetBitmap, *after* the menu
        // item has already been added, used to result in a stack overflow:
        // [Do]SetBitmap can call GetHBitmapForMenu which will call SetBitmap
        // again etc...
        item->SetBitmap(wxBitmap(wxSize{1, 1}));


        // Force owner drawn usage by having a bitmap that's wider than the
        // standard size. This results in rearranging the parent menu which
        // hasn't always worked properly and lead to a nullptr pointer exception.
        item->SetBitmap(wxBitmap(wxSize{512, 1}));

        wxDELETE(menu);
    };

/*
#if wxUSE_UIACTIONSIMULATOR
    "Events"_test = [&]
    {
        MenuEventHandler handler(m_frame.get());

        // Invoke the accelerator.
        m_frame->Show();
        m_frame->SetFocus();
        wxYield();

        wxUIActionSimulator sim;
        sim.KeyDown(WXK_F1);
        sim.KeyUp(WXK_F1);
        wxYield();

        const auto ev = handler.GetEvent();
        expect(static_cast<int>(MenuTestCase_Bar) == ev->GetId());

        wxObject* const src = ev->GetEventObject();
        expect(src);

        // FIXME: Use typeinfo
        //CHECK_EQ("wxMenu",
        //    wxString(src->wxGetClassInfo()->wxGetClassName()));
        expect(static_cast<wxObject*>(m_menuWithBar) == src);

        // Invoke another accelerator, it should also work.
        sim.Char('A', wxMOD_CONTROL);
        wxYield();

        const auto ev2 = handler.GetEvent();
        expect(ev2->GetId() == static_cast<int>(MenuTestCase_SelectAll));

        // Now create a text control which uses the same accelerator for itself and
        // check that when the text control has focus, the accelerator does _not_
        // work.
        auto text = std::make_unique<wxTextCtrl>(m_frame.get(), wxID_ANY, "Testing");
        text->SetFocus();

        sim.Char('A', wxMOD_CONTROL);
        wxYield();

        expect(!handler.GotEvent());
    };
#endif // wxUSE_UIACTIONSIMULATOR
*/

};

namespace
{

void VerifyAccelAssigned( std::string labelText, int keycode )
{
    using namespace ut;
    
    auto entry = wxAcceleratorEntry::Create( labelText );

    expect( entry.has_value() );
    expect( entry->GetKeyCode() == keycode );
}

struct key
{
    int      keycode;
    std::string name;
    bool     skip;
};
key modKeys[] =
{
    { wxACCEL_NORMAL, "Normal", false },
    { wxACCEL_CTRL,   "Ctrl",   false },
    { wxACCEL_SHIFT,  "Shift",  false },
    { wxACCEL_ALT,    "Alt",    false }
};
/*
 The keys marked as skip below are not supported as accelerator
 keys on GTK.
 */
key specialKeys[] =
{
    { WXK_F1,               "WXK_F1",               false },
    { WXK_F2,               "WXK_F2",               false },
    { WXK_F3,               "WXK_F3",               false },
    { WXK_F4,               "WXK_F4",               false },
    { WXK_F5,               "WXK_F5",               false },
    { WXK_F6,               "WXK_F6",               false },
    { WXK_F7,               "WXK_F7",               false },
    { WXK_F8,               "WXK_F8",               false },
    { WXK_F9,               "WXK_F9",               false },
    { WXK_F10,              "WXK_F10",              false },
    { WXK_F11,              "WXK_F11",              false },
    { WXK_F12,              "WXK_F12",              false },
    { WXK_F13,              "WXK_F13",              false },
    { WXK_F14,              "WXK_F14",              false },
    { WXK_F15,              "WXK_F15",              false },
    { WXK_F16,              "WXK_F16",              false },
    { WXK_F17,              "WXK_F17",              false },
    { WXK_F18,              "WXK_F18",              false },
    { WXK_F19,              "WXK_F19",              false },
    { WXK_F20,              "WXK_F20",              false },
    { WXK_F21,              "WXK_F21",              false },
    { WXK_F22,              "WXK_F22",              false },
    { WXK_F23,              "WXK_F23",              false },
    { WXK_F24,              "WXK_F24",              false },
    { WXK_INSERT,           "WXK_INSERT",           false },
    { WXK_DELETE,           "WXK_DELETE",           false },
    { WXK_UP,               "WXK_UP",               false },
    { WXK_DOWN,             "WXK_DOWN",             false },
    { WXK_PAGEUP,           "WXK_PAGEUP",           false },
    { WXK_PAGEDOWN,         "WXK_PAGEDOWN",         false },
    { WXK_LEFT,             "WXK_LEFT",             false },
    { WXK_RIGHT,            "WXK_RIGHT",            false },
    { WXK_HOME,             "WXK_HOME",             false },
    { WXK_END,              "WXK_END",              false },
    { WXK_RETURN,           "WXK_RETURN",           false },
    { WXK_BACK,             "WXK_BACK",             false },
    { WXK_TAB,              "WXK_TAB",              true },
    { WXK_ESCAPE,           "WXK_ESCAPE",           false },
    { WXK_SPACE,            "WXK_SPACE",            false },
    { WXK_MULTIPLY,         "WXK_MULTIPLY",         false },
    { WXK_ADD,              "WXK_ADD",              true },
    { WXK_SEPARATOR,        "WXK_SEPARATOR",        true },
    { WXK_SUBTRACT,         "WXK_SUBTRACT",         true },
    { WXK_DECIMAL,          "WXK_DECIMAL",          true },
    { WXK_DIVIDE,           "WXK_DIVIDE",           true },
    { WXK_CANCEL,           "WXK_CANCEL",           false },
    { WXK_CLEAR,            "WXK_CLEAR",            false },
    { WXK_MENU,             "WXK_MENU",             false },
    { WXK_PAUSE,            "WXK_PAUSE",            false },
    { WXK_CAPITAL,          "WXK_CAPITAL",          true },
    { WXK_SELECT,           "WXK_SELECT",           false },
    { WXK_PRINT,            "WXK_PRINT",            false },
    { WXK_EXECUTE,          "WXK_EXECUTE",          false },
    { WXK_SNAPSHOT,         "WXK_SNAPSHOT",         true },
    { WXK_HELP,             "WXK_HELP",             false },
    { WXK_NUMLOCK,          "WXK_NUMLOCK",          true },
    { WXK_SCROLL,           "WXK_SCROLL",           true },
    { WXK_NUMPAD_INSERT,    "WXK_NUMPAD_INSERT",    false },
    { WXK_NUMPAD_DELETE,    "WXK_NUMPAD_DELETE",    false },
    { WXK_NUMPAD_SPACE,     "WXK_NUMPAD_SPACE",     false },
    { WXK_NUMPAD_TAB,       "WXK_NUMPAD_TAB",       true },
    { WXK_NUMPAD_ENTER,     "WXK_NUMPAD_ENTER",     false },
    { WXK_NUMPAD_F1,        "WXK_NUMPAD_F1",        false },
    { WXK_NUMPAD_F2,        "WXK_NUMPAD_F2",        false },
    { WXK_NUMPAD_F3,        "WXK_NUMPAD_F3",        false },
    { WXK_NUMPAD_F4,        "WXK_NUMPAD_F4",        false },
    { WXK_NUMPAD_HOME,      "WXK_NUMPAD_HOME",      false },
    { WXK_NUMPAD_LEFT,      "WXK_NUMPAD_LEFT",      false },
    { WXK_NUMPAD_UP,        "WXK_NUMPAD_UP",        false },
    { WXK_NUMPAD_RIGHT,     "WXK_NUMPAD_RIGHT",     false },
    { WXK_NUMPAD_DOWN,      "WXK_NUMPAD_DOWN",      false },
    { WXK_NUMPAD_PAGEUP,    "WXK_NUMPAD_PAGEUP",    false },
    { WXK_NUMPAD_PAGEDOWN,  "WXK_NUMPAD_PAGEDOWN",  false },
    { WXK_NUMPAD_END,       "WXK_NUMPAD_END",       false },
    { WXK_NUMPAD_BEGIN,     "WXK_NUMPAD_BEGIN",     false },
    { WXK_NUMPAD_EQUAL,     "WXK_NUMPAD_EQUAL",     false },
    { WXK_NUMPAD_MULTIPLY,  "WXK_NUMPAD_MULTIPLY",  false },
    { WXK_NUMPAD_ADD,       "WXK_NUMPAD_ADD",       false },
    { WXK_NUMPAD_SEPARATOR, "WXK_NUMPAD_SEPARATOR", false },
    { WXK_NUMPAD_SUBTRACT,  "WXK_NUMPAD_SUBTRACT",  false },
    { WXK_NUMPAD_DECIMAL,   "WXK_NUMPAD_DECIMAL",   false },
    { WXK_NUMPAD_DIVIDE,    "WXK_NUMPAD_DIVIDE",    false },
    { WXK_NUMPAD0,          "WXK_NUMPAD0",          false },
    { WXK_NUMPAD1,          "WXK_NUMPAD1",          false },
    { WXK_NUMPAD2,          "WXK_NUMPAD2",          false },
    { WXK_NUMPAD3,          "WXK_NUMPAD3",          false },
    { WXK_NUMPAD4,          "WXK_NUMPAD4",          false },
    { WXK_NUMPAD5,          "WXK_NUMPAD5",          false },
    { WXK_NUMPAD6,          "WXK_NUMPAD6",          false },
    { WXK_NUMPAD7,          "WXK_NUMPAD7",          false },
    { WXK_NUMPAD8,          "WXK_NUMPAD8",          false },
    { WXK_NUMPAD9,          "WXK_NUMPAD9",          false },
    { WXK_WINDOWS_LEFT,     "WXK_WINDOWS_LEFT",     true },
    { WXK_WINDOWS_RIGHT,    "WXK_WINDOWS_RIGHT",    true },
    { WXK_WINDOWS_MENU,     "WXK_WINDOWS_MENU",     false },
    { WXK_COMMAND,          "WXK_COMMAND",          true }
};

}

ut::suite wxMenuItemAccelEntryTests = []
{
    using namespace ut;

    wxMenu* menu = new wxMenu;

    menu->Append( wxID_ANY, "Test" );
    wxMenuItem* item = menu->FindItemByPosition( 0 );

    "ModifierKeys"_test = [&]
    {
        for ( unsigned i = 0; i < WXSIZEOF(modKeys); i++ )
        {
            const key& k = modKeys[i];

            // FIXME: Info in new framework?
            // INFO( fmt::format( "Modifier: %s",  k.name ) );
            wxAcceleratorEntry accelEntry( k.keycode, 'A' , wxID_ANY, item );
            item->SetAccel( &accelEntry );

            std::string labelText = item->GetItemLabel();
            // INFO( fmt::format( "Label text: %s", labelText ) );

            VerifyAccelAssigned( labelText, 'A' );
        }
    };

    "SpecialKeys"_test = [&]
    {
        for ( unsigned i = 0; i < WXSIZEOF(specialKeys); i++ )
        {
            const key& k = specialKeys[i];

            if( k.skip )
                continue;

            // FIXME: New framework info?
            // INFO( fmt::format( "Keycode: %s",  k.name ) );
            wxAcceleratorEntry accelEntry( wxACCEL_CTRL, k.keycode, wxID_ANY, item );
            item->SetAccel( &accelEntry );

            std::string labelText = item->GetItemLabel();
            // INFO( fmt::format( "Label text: %s", labelText ) );

            VerifyAccelAssigned( labelText, k.keycode );
        }
    };
};

#endif
