///////////////////////////////////////////////////////////////////////////////
// Name:        wx/menu.h
// Purpose:     wxMenu and wxMenuBar classes
// Author:      Vadim Zeitlin
// Modified by:
// Created:     26.10.99
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MENU_H_BASE_
#define _WX_MENU_H_BASE_

#if wxUSE_MENUS

#include "wx/list.h"        // for "template" list classes
#include "wx/window.h"      // base class for wxMenuBar

// also include this one to ensure compatibility with old code which only
// included wx/menu.h
#include "wx/menuitem.h"

import <string>;

class wxFrame;
class wxMenu;
class wxMenuBarBase;
class wxMenuBar;
class wxMenuItem;

// pseudo template list classes
WX_DECLARE_LIST(wxMenu, wxMenuList);
WX_DECLARE_LIST(wxMenuItem, wxMenuItemList);

class wxMenuBase : public wxEvtHandler
{
public:
    // create a menu
    static wxMenu *New(const std::string& title = {}, unsigned int style = 0);

    wxMenuBase(const std::string& title, unsigned int style = 0) : m_title(title)
        { Init(style); }
    wxMenuBase(unsigned int style = 0)
        { Init(style); }

    // dtor deletes all the menu items we own
    ~wxMenuBase();

    wxMenuBase& operator=(wxMenuBase&&) = delete;

    // append any kind of item (normal/check/radio/separator)
    wxMenuItem* Append(int itemid,
                       const std::string& text = {},
                       const std::string& help = {},
                       wxItemKind kind = wxITEM_NORMAL)
    {
        return DoAppend(wxMenuItem::New((wxMenu *)this, itemid, text, help, kind));
    }

    // append a separator to the menu
    wxMenuItem* AppendSeparator() { return Append(wxID_SEPARATOR); }

    // append a check item
    wxMenuItem* AppendCheckItem(int itemid,
                                const std::string& text,
                                const std::string& help = {})
    {
        return Append(itemid, text, help, wxITEM_CHECK);
    }

    // append a radio item
    wxMenuItem* AppendRadioItem(int itemid,
                                const std::string& text,
                                const std::string& help = {})
    {
        return Append(itemid, text, help, wxITEM_RADIO);
    }

    // append a submenu
    wxMenuItem* AppendSubMenu(wxMenu *submenu,
                              const std::string& text,
                              const std::string& help = {})
    {
        return DoAppend(wxMenuItem::New((wxMenu *)this, wxID_ANY, text, help,
                                        wxITEM_NORMAL, submenu));
    }

    // the most generic form of Append() - append anything
    wxMenuItem* Append(wxMenuItem *item) { return DoAppend(item); }

    // insert a break in the menu (only works when appending the items, not
    // inserting them)
    virtual void Break() { }

    // insert an item before given position
    wxMenuItem* Insert(size_t pos, wxMenuItem *item);

    // insert an item before given position
    wxMenuItem* Insert(size_t pos,
                       int itemid,
                       const std::string& text = {},
                       const std::string& help = {},
                       wxItemKind kind = wxITEM_NORMAL)
    {
        return Insert(pos, wxMenuItem::New((wxMenu *)this, itemid, text, help, kind));
    }

    // insert a separator
    wxMenuItem* InsertSeparator(size_t pos)
    {
        return Insert(pos, wxMenuItem::New((wxMenu *)this, wxID_SEPARATOR));
    }

    // insert a check item
    wxMenuItem* InsertCheckItem(size_t pos,
                                int itemid,
                                const std::string& text,
                                const std::string& help = {})
    {
        return Insert(pos, itemid, text, help, wxITEM_CHECK);
    }

    // insert a radio item
     wxMenuItem* InsertRadioItem(size_t pos,
                                 int itemid,
                                 const std::string& text,
                                 const std::string& help = {})
    {
        return Insert(pos, itemid, text, help, wxITEM_RADIO);
    }

    // insert a submenu
    wxMenuItem* Insert(size_t pos,
                       int itemid,
                       const std::string& text,
                       wxMenu *submenu,
                       const std::string& help = {})
    {
        return Insert(pos, wxMenuItem::New((wxMenu *)this, itemid, text, help,
                                           wxITEM_NORMAL, submenu));
    }

    // prepend an item to the menu
    wxMenuItem* Prepend(wxMenuItem *item)
    {
        return Insert(0u, item);
    }

    // prepend any item to the menu
    wxMenuItem* Prepend(int itemid,
                        const std::string& text = {},
                        const std::string& help = {},
                        wxItemKind kind = wxITEM_NORMAL)
    {
        return Insert(0u, itemid, text, help, kind);
    }

    // prepend a separator
    wxMenuItem* PrependSeparator()
    {
        return InsertSeparator(0u);
    }

    // prepend a check item
    wxMenuItem* PrependCheckItem(int itemid,
                                 const std::string& text,
                                 const std::string& help = {})
    {
        return InsertCheckItem(0u, itemid, text, help);
    }

    // prepend a radio item
    wxMenuItem* PrependRadioItem(int itemid,
                                 const std::string& text,
                                 const std::string& help = {})
    {
        return InsertRadioItem(0u, itemid, text, help);
    }

    // prepend a submenu
    wxMenuItem* Prepend(int itemid,
                        const std::string& text,
                        wxMenu *submenu,
                        const std::string& help = {})
    {
        return Insert(0u, itemid, text, submenu, help);
    }

    // detach an item from the menu, but don't delete it so that it can be
    // added back later (but if it's not, the caller is responsible for
    // deleting it!)
    wxMenuItem *Remove(int itemid) { return Remove(FindChildItem(itemid)); }
    wxMenuItem *Remove(wxMenuItem *item);

    // delete an item from the menu (submenus are not destroyed by this
    // function, see Destroy)
    bool Delete(int itemid) { return Delete(FindChildItem(itemid)); }
    bool Delete(wxMenuItem *item);

    // delete the item from menu and destroy it (if it's a submenu)
    bool Destroy(int itemid) { return Destroy(FindChildItem(itemid)); }
    bool Destroy(wxMenuItem *item);

    // menu items access
    // -----------------

    // get the items
    size_t GetMenuItemCount() const { return m_items.GetCount(); }

    const wxMenuItemList& GetMenuItems() const { return m_items; }
    wxMenuItemList& GetMenuItems() { return m_items; }

    // search
    virtual int FindItem(const std::string& item) const;
    wxMenuItem* FindItem(int itemid, wxMenu **menu = nullptr) const;

    // find by position
    wxMenuItem* FindItemByPosition(size_t position) const;

    // get/set items attributes
    void Enable(int itemid, bool enable);
    bool IsEnabled(int itemid) const;

    void Check(int itemid, bool check);
    bool IsChecked(int itemid) const;

    void SetLabel(int itemid, const std::string& label);
    std::string GetLabel(int itemid) const;

    //  Returns the stripped label
    std::string GetLabelText(int itemid) const { return wxMenuItem::GetLabelText(GetLabel(itemid)); }

    virtual void SetHelpString(int itemid, const std::string& helpString);
    virtual std::string GetHelpString(int itemid) const;

    // misc accessors
    // --------------

    // the title
    virtual void SetTitle(const std::string& title) { m_title = title; }
    const std::string& GetTitle() const { return m_title; }

    // event handler
    void SetEventHandler(wxEvtHandler *handler) { m_eventHandler = handler; }
    wxEvtHandler *GetEventHandler() const { return m_eventHandler; }

    // Invoking window: this is set by wxWindow::PopupMenu() before showing a
    // popup menu and reset after it's hidden. Notice that you probably want to
    // use GetWindow() below instead of GetInvokingWindow() as the latter only
    // returns non-NULL for the top level menus
    //
    // NB: avoid calling SetInvokingWindow() directly if possible, use
    //     wxMenuInvokingWindowSetter class below instead
    void SetInvokingWindow(wxWindow *win);
    wxWindow *GetInvokingWindow() const { return m_invokingWindow; }

    // the window associated with this menu: this is the invoking window for
    // popup menus or the top level window to which the menu bar is attached
    // for menus which are part of a menu bar
    wxWindow *GetWindow() const;

    // style
    unsigned int GetStyle() const { return m_style; }

    // implementation helpers
    // ----------------------

    // Updates the UI for a menu and all submenus recursively by generating
    // wxEVT_UPDATE_UI for all the items.
    //
    // Do not use the "source" argument, it allows to override the event
    // handler to use for these events, but this should never be needed.
    void UpdateUI(wxEvtHandler* source = nullptr);

#if wxUSE_MENUBAR
    // get the menu bar this menu is attached to (may be NULL, always NULL for
    // popup menus).  Traverse up the menu hierarchy to find it.
    wxMenuBar *GetMenuBar() const;

    // called when the menu is attached/detached to/from a menu bar
    virtual void Attach(wxMenuBarBase *menubar);
    virtual void Detach();

    // is the menu attached to a menu bar (or is it a popup one)?
    bool IsAttached() const { return GetMenuBar() != nullptr; }
#endif

    // set/get the parent of this menu
    void SetParent(wxMenu *parent) { m_menuParent = parent; }
    wxMenu *GetParent() const { return m_menuParent; }

    // implementation only from now on
    // -------------------------------

    // unlike FindItem(), this function doesn't recurse but only looks through
    // our direct children and also may return the index of the found child if
    // pos != NULL
    wxMenuItem *FindChildItem(int itemid, size_t *pos = nullptr) const;

    // called to generate a wxCommandEvent, return true if it was processed,
    // false otherwise
    //
    // the checked parameter may have boolean value or -1 for uncheckable items
    bool SendEvent(int itemid, int checked = -1);

    // called to dispatch a wxMenuEvent to the right recipients, menu pointer
    // can be NULL if we failed to find the associated menu (this happens at
    // least in wxMSW for the events from the system menus)
    static
    bool ProcessMenuEvent(wxMenu* menu, wxMenuEvent& event, wxWindow* win);


    // compatibility: these functions are deprecated, use the new ones instead
    // -----------------------------------------------------------------------

    // use the versions taking wxItem_XXX now instead, they're more readable
    // and allow adding the radio items as well
    void Append(int itemid,
                const std::string& text,
                const std::string& help,
                bool isCheckable)
    {
        Append(itemid, text, help, isCheckable ? wxITEM_CHECK : wxITEM_NORMAL);
    }

    // use more readable and not requiring unused itemid AppendSubMenu() instead
    wxMenuItem* Append(int itemid,
                       const std::string& text,
                       wxMenu *submenu,
                       const std::string& help = {})
    {
        return DoAppend(wxMenuItem::New((wxMenu *)this, itemid, text, help,
                                        wxITEM_NORMAL, submenu));
    }

    void Insert(size_t pos,
                int itemid,
                const std::string& text,
                const std::string& help,
                bool isCheckable)
    {
        Insert(pos, itemid, text, help, isCheckable ? wxITEM_CHECK : wxITEM_NORMAL);
    }

    void Prepend(int itemid,
                 const std::string& text,
                 const std::string& help,
                 bool isCheckable)
    {
        Insert(0u, itemid, text, help, isCheckable);
    }

    static void LockAccels(bool locked)
    {
        ms_locked = locked;
    }

protected:
    // virtuals to override in derived classes
    // ---------------------------------------

    virtual wxMenuItem* DoAppend(wxMenuItem *item);
    virtual wxMenuItem* DoInsert(size_t pos, wxMenuItem *item);

    virtual wxMenuItem *DoRemove(wxMenuItem *item);
    virtual bool DoDelete(wxMenuItem *item);
    virtual bool DoDestroy(wxMenuItem *item);

    // helpers
    // -------

    // common part of all ctors
    void Init(unsigned int style);

    // associate the submenu with this menu
    void AddSubMenu(wxMenu *submenu);
    
    std::string       m_title;          // the menu title or label
    
    wxMenuItemList    m_items;          // the list of menu items

    wxMenuBar     *m_menuBar;           // menubar we belong to or NULL
    wxMenu        *m_menuParent;        // parent menu or NULL

    wxWindow      *m_invokingWindow;    // for popup menus

    wxEvtHandler  *m_eventHandler;      // a pluggable in event handler

    unsigned int   m_style;             // combination of wxMENU_XXX flags

    inline static bool      ms_locked{true};


protected:
    // Common part of SendEvent() and ProcessMenuEvent(): sends the event to
    // its intended recipients, returns true if it was processed.
    static bool DoProcessEvent(wxMenuBase* menu, wxEvent& event, wxWindow* win);
};

// ----------------------------------------------------------------------------
// wxMenuBar
// ----------------------------------------------------------------------------

#if wxUSE_MENUBAR

class wxMenuBarBase : public wxWindow
{
public:
    // dtor will delete all menus we own
    ~wxMenuBarBase();

    wxMenuBarBase& operator=(wxMenuBarBase&&) = delete;

    // menu bar construction
    // ---------------------

    // append a menu to the end of menubar, return true if ok
    virtual bool Append(wxMenu *menu, const std::string& title);

    // insert a menu before the given position into the menubar, return true
    // if inserted ok
    virtual bool Insert(size_t pos, wxMenu *menu, const std::string& title);

    // menu bar items access
    // ---------------------

    // get the number of menus in the menu bar
    size_t GetMenuCount() const { return m_menus.GetCount(); }

    // get the menu at given position
    wxMenu *GetMenu(size_t pos) const;

    // replace the menu at given position with another one, returns the
    // previous menu (which should be deleted by the caller)
    virtual wxMenu *Replace(size_t pos, wxMenu *menu, const std::string& title);

    // delete the menu at given position from the menu bar, return the pointer
    // to the menu (which should be  deleted by the caller)
    virtual wxMenu *Remove(size_t pos);

    // enable or disable a submenu
    virtual void EnableTop(size_t pos, bool enable) = 0;

    // is the menu enabled?
    virtual bool IsEnabledTop([[maybe_unused]] size_t pos) const { return true; }

    // get or change the label of the menu at given position
    virtual void SetMenuLabel(size_t pos, const std::string& label) = 0;
    virtual std::string GetMenuLabel(size_t pos) const = 0;

    // get the stripped label of the menu at given position
    virtual std::string GetMenuLabelText(size_t pos) const { return wxMenuItem::GetLabelText(GetMenuLabel(pos)); }

    // item search
    // -----------

    // by menu and item names, returns wxNOT_FOUND if not found or id of the
    // found item
    virtual int FindMenuItem(const std::string& menu, const std::string& item) const;

    // find item by id (in any menu), returns NULL if not found
    //
    // if menu is !NULL, it will be filled with wxMenu this item belongs to
    virtual wxMenuItem* FindItem(int itemid, wxMenu **menu = nullptr) const;

    // find menu by its caption, return wxNOT_FOUND on failure
    int FindMenu(const std::string& title) const;

    // item access
    // -----------

    // all these functions just use FindItem() and then call an appropriate
    // method on it
    //
    // NB: under MSW, these methods can only be used after the menubar had
    //     been attached to the frame

    void Enable(int itemid, bool enable);
    void Check(int itemid, bool check);
    bool IsChecked(int itemid) const;
    bool IsEnabled(int itemid) const;
    virtual bool IsEnabled() const { return wxWindow::IsEnabled(); }

    void SetLabel(int itemid, const std::string &label);
    std::string GetLabel(int itemid) const;

    void SetHelpString(int itemid, const std::string& helpString);
    std::string GetHelpString(int itemid) const;

    // implementation helpers

    // get the frame we are attached to (may return NULL)
    wxFrame *GetFrame() const { return m_menuBarFrame; }

    // returns true if we're attached to a frame
    bool IsAttached() const { return GetFrame() != nullptr; }

    // associate the menubar with the frame
    virtual void Attach(wxFrame *frame);

    // called before deleting the menubar normally
    virtual void Detach();

    // need to override these ones to avoid virtual function hiding
    bool Enable(bool enable = true) override { return wxWindow::Enable(enable); }
    void SetLabel(std::string_view s) override { wxWindow::SetLabel(s); }
    std::string GetLabel() const override { return wxWindow::GetLabel(); }

    // don't want menu bars to accept the focus by tabbing to them
    bool AcceptsFocusFromKeyboard() const override { return false; }

    // update all menu item states in all menus
    virtual void UpdateMenus();

    bool CanBeOutsideClientArea() const override { return true; }

protected:
    // the list of all our menus
    wxMenuList m_menus;

    // the frame we are attached to (may be nullptr)
    wxFrame *m_menuBarFrame{nullptr};
};
#endif

// ----------------------------------------------------------------------------
// include the real class declaration
// ----------------------------------------------------------------------------

#ifdef wxUSE_BASE_CLASSES_ONLY
    using wxMenuItem = wxMenuItemBase;
#else // !wxUSE_BASE_CLASSES_ONLY
#if defined(__WXUNIVERSAL__)
    #include "wx/univ/menu.h"
#elif defined(__WXMSW__)
    #include "wx/msw/menu.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/menu.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/menu.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/menu.h"
#elif defined(__WXMAC__)
    #include "wx/osx/menu.h"
#elif defined(__WXQT__)
    #include "wx/qt/menu.h"
#endif
#endif // wxUSE_BASE_CLASSES_ONLY/!wxUSE_BASE_CLASSES_ONLY

inline wxMenu* wxCurrentPopupMenu{nullptr};

// ----------------------------------------------------------------------------
// Helper class used in the implementation only: sets the invoking window of
// the given menu in its ctor and resets it in dtor.
// ----------------------------------------------------------------------------

class wxMenuInvokingWindowSetter
{
public:
    // Ctor sets the invoking window for the given menu.
    //
    // The menu lifetime must be greater than that of this class.
    wxMenuInvokingWindowSetter(wxMenu& menu, wxWindow *win)
        : m_menu(menu)
    {
        menu.SetInvokingWindow(win);
    }

    // Dtor resets the invoking window.
    ~wxMenuInvokingWindowSetter()
    {
        m_menu.SetInvokingWindow(nullptr);
    }

private:
    wxMenu& m_menu;
};

#endif // wxUSE_MENUS

#endif // _WX_MENU_H_BASE_
