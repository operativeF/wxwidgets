/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/menu.cpp
// Purpose:     wxMenu, wxMenuBar, wxMenuItem
// Author:      Julian Smart
// Modified by: Vadim Zeitlin
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_MENUS

#include "wx/menu.h"

#include "wx/msw/private.h"
#include "wx/msw/wrapcctl.h" // include <commctrl.h> "properly"

#include "wx/frame.h"
#include "wx/utils.h"
#include "wx/intl.h"
#include "wx/log.h"

#if wxUSE_OWNER_DRAWN
    #include "wx/ownerdrw.h"
#endif

#include "wx/scopedarray.h"
#include "wx/private/menuradio.h" // for wxMenuRadioItemsData

#include "wx/dynlib.h"

#include <boost/nowide/convert.hpp>
#include <boost/nowide/stackstring.hpp>

import WX.WinDef;

import Utils.Strings;

// ----------------------------------------------------------------------------
// global variables
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// the (popup) menu title has this special id
constexpr int idMenuTitle = wxID_NONE;

// ----------------------------------------------------------------------------
// private helper classes and functions
// ----------------------------------------------------------------------------

namespace
{

// make the given menu item default
void SetDefaultMenuItem(WXHMENU hmenu, WXUINT id)
{
    WinStruct<MENUITEMINFOW> mii;
    mii.fMask = MIIM_STATE;
    mii.fState = MFS_DEFAULT;

    if ( !::SetMenuItemInfoW(hmenu, id, FALSE, &mii) )
    {
        wxLogLastError("SetMenuItemInfo");
    }
}

// make the given menu item owner-drawn
void SetOwnerDrawnMenuItem(WXHMENU hmenu,
                           WXUINT id,
                           ULONG_PTR data,
                           BOOL byPositon = FALSE)
{
    WinStruct<MENUITEMINFOW> mii;
    mii.fMask = MIIM_FTYPE | MIIM_DATA;
    mii.fType = MFT_OWNERDRAW;
    mii.dwItemData = data;

    if ( reinterpret_cast<wxMenuItem*>(data)->IsSeparator() )
        mii.fType |= MFT_SEPARATOR;

    if ( !::SetMenuItemInfoW(hmenu, id, byPositon, &mii) )
    {
        wxLogLastError("SetMenuItemInfo");
    }
}

} // anonymous namespace

// ============================================================================
// implementation
// ============================================================================

// ---------------------------------------------------------------------------
// wxMenu construction, adding and removing menu items
// ---------------------------------------------------------------------------

// Construct a menu with optional title (then use append)
void wxMenu::InitNoCreate()
{
    m_doBreak = false;

#if wxUSE_OWNER_DRAWN
    m_ownerDrawn = false;
    m_maxBitmapWidth = 0;
    m_maxAccelWidth = -1;
#endif // wxUSE_OWNER_DRAWN
}



wxMenu::wxMenu(WXHMENU hMenu)
{
    InitNoCreate();

    m_hMenu = hMenu;

    // Ensure that our internal idea of how many items we have corresponds to
    // the real number of items in the menu.
    //
    // We could also retrieve the real labels of the items here but it doesn't
    // seem to be worth the trouble.
    for ( int n = 0; n < ::GetMenuItemCount(m_hMenu); n++ )
    {
        wxMenuBase::DoAppend(wxMenuItem::New(this, wxID_SEPARATOR));
    }
}

// The wxWindow destructor will take care of deleting the submenus.
wxMenu::~wxMenu()
{
    // we should free Windows resources only if Windows doesn't do it for us
    // which happens if we're attached to a menubar or a submenu of another
    // menu
    if ( m_hMenu && !IsAttached() && !GetParent() )
    {
        if ( !::DestroyMenu(GetHmenu()) )
        {
            wxLogLastError("DestroyMenu");
        }
    }
}

void wxMenu::Break()
{
    // this will take effect during the next call to Append()
    m_doBreak = true;
}

#if wxUSE_ACCEL

std::vector<wxAcceleratorEntry>::const_iterator wxMenu::FindAccel(int id) const
{
    return std::ranges::find_if(m_accels,
        [id](const auto& accel){
            return accel.m_command == id;
        });
}

void wxMenu::UpdateAccel(wxMenuItem *item)
{
    if ( item->IsSubMenu() )
    {
        wxMenu *submenu = item->GetSubMenu();
        wxMenuItemList::compatibility_iterator node = submenu->GetMenuItems().GetFirst();
        while ( node )
        {
            UpdateAccel(node->GetData());

            node = node->GetNext();
        }
    }
    else if ( !item->IsSeparator() )
    {
        // recurse upwards: we should only modify m_accels of the top level
        // menus, not of the submenus as wxMenuBar doesn't look at them
        // (alternative and arguable cleaner solution would be to recurse
        // downwards in GetAccelCount() and CopyAccels())
        if ( GetParent() )
        {
            GetParent()->UpdateAccel(item);
            return;
        }

        // find the (new) accel for this item
        auto accel = wxAcceleratorEntry::Create(item->GetItemLabel());
        if ( accel )
            accel->m_command = item->GetId();

        // find the old one
        const auto old_accel = FindAccel(item->GetId());

        if ( old_accel == m_accels.cend() )
        {
            // no old, add new if any
            if ( accel )
                m_accels.push_back(*accel);
            else
                return;     // skipping RebuildAccelTable() below
        }
        else
        {
            // replace old with new or just remove the old one if no new
            if ( accel )
                m_accels.insert(old_accel, *accel);
            else
                m_accels.erase(old_accel);
        }

        if ( IsAttached() )
        {
            GetMenuBar()->RebuildAccelTable();
        }

#if wxUSE_OWNER_DRAWN
        ResetMaxAccelWidth();
#endif
    }
    //else: it is a separator, they can't have accels, nothing to do
}

void wxMenu::RemoveAccel(wxMenuItem *item)
{
    // recurse upwards: we should only modify m_accels of the top level
    // menus, not of the submenus as wxMenuBar doesn't look at them
    // (alternative and arguable cleaner solution would be to recurse
    // downwards in GetAccelCount() and CopyAccels())
    if ( GetParent() )
    {
        GetParent()->RemoveAccel(item);
        return;
    }

    // remove the corresponding accel from the accel table
    auto accelToRemove = FindAccel(item->GetId());
    if ( accelToRemove != m_accels.cend() )
    {
        m_accels.erase(accelToRemove);

#if wxUSE_OWNER_DRAWN
        ResetMaxAccelWidth();
#endif
    }
    //else: this item doesn't have an accel, nothing to do
}

#endif // wxUSE_ACCEL

namespace
{

} // anonymous namespace

bool wxMenu::MSWGetRadioGroupRange(int pos, int *start, int *end) const
{
    return m_radioData && m_radioData->GetGroupRange(pos, start, end);
}

// append a new item or submenu to the menu
bool wxMenu::DoInsertOrAppend(wxMenuItem *pItem, size_t pos)
{
#if wxUSE_ACCEL
    UpdateAccel(pItem);
#endif // wxUSE_ACCEL

    // we should support disabling the item even prior to adding it to the menu
    WXUINT flags = pItem->IsEnabled() ? MF_ENABLED : MF_GRAYED;

    // if "Break" has just been called, insert a menu break before this item
    // (and don't forget to reset the flag)
    if ( m_doBreak ) {
        flags |= MF_MENUBREAK;
        m_doBreak = false;
    }

    if ( pItem->IsSeparator() ) {
        flags |= MF_SEPARATOR;
    }

    // id is the numeric id for normal menu items and WXHMENU for submenus as
    // required by ::AppendMenu() API
    UINT_PTR id;
    wxMenu *submenu = pItem->GetSubMenu();
    if ( submenu != nullptr ) {
        wxASSERT_MSG( submenu->GetHMenu(), "invalid submenu" );

        submenu->SetParent(this);

        id = (UINT_PTR)submenu->GetHMenu();

        flags |= MF_POPUP;
    }
    else {
        id = pItem->GetMSWId();
    }


    // prepare to insert the item in the menu
    std::string itemText = pItem->GetItemLabel();
    LPCWSTR pData = nullptr;
    if ( pos == (size_t)-1 )
    {
        // append at the end (note that the item is already appended to
        // internal data structures)
        pos = GetMenuItemCount() - 1;
    }

    // Update radio groups data if we're inserting a new menu item.
    // Inserting radio and non-radio item has a different impact
    // on radio groups so we have to handle each case separately.
    // (Inserting a radio item in the middle of existing group extends
    // the group, but inserting non-radio item breaks it into two subgroups.)
    //
    bool checkInitially = false;
    if ( pItem->IsRadio() )
    {
        if ( !m_radioData )
            m_radioData = std::make_unique<wxMenuRadioItemsData>();

        if ( m_radioData->UpdateOnInsertRadio(pos) )
            checkInitially = true;
    }
    else if ( m_radioData )
    {
        if ( m_radioData->UpdateOnInsertNonRadio(pos) )
        {
            // One of the existing groups has been split into two subgroups.
            wxFAIL_MSG("Inserting non-radio item inside a radio group?");
        }
    }

    // Also handle the case of check menu items that had been checked before
    // being attached to the menu: we don't need to actually call Check() on
    // them, so we don't use checkInitially in this case, but we do need to
    // make them checked at Windows level too. Notice that we shouldn't ask
    // Windows for the checked state here, as wxMenuItem::IsChecked() does, as
    // the item is not attached yet, so explicitly call the base class version.
    if ( pItem->IsCheck() && pItem->wxMenuItemBase::IsChecked() )
        flags |= MF_CHECKED;

    // adjust position to account for the title of a popup menu, if any
    if ( !GetMenuBar() && !m_title.empty() )
        pos += 2; // for the title itself and its separator

    BOOL ok = false;

#if wxUSE_OWNER_DRAWN
    // Under older systems mixing owner-drawn and non-owner-drawn items results
    // in inconsistent margins, so we force this one to be owner-drawn if any
    // other items already are.
    if ( m_ownerDrawn )
        pItem->SetOwnerDrawn(true);

    // check if we have something more than a simple text item
    bool makeItemOwnerDrawn = false;
#endif // wxUSE_OWNER_DRAWN

    if (
#if wxUSE_OWNER_DRAWN
            !pItem->IsOwnerDrawn() &&
#endif
        !pItem->IsSeparator() )
            {
                WinStruct<MENUITEMINFOW> mii;
                mii.fMask = MIIM_STRING | MIIM_DATA;

                // don't set hbmpItem for the checkable items as it would
                // be used for both checked and unchecked state
                if ( pItem->IsCheckable() )
                {
                    mii.fMask |= MIIM_CHECKMARKS;
                    mii.hbmpChecked = pItem->GetHBitmapForMenu(wxMenuItem::Checked);
                    mii.hbmpUnchecked = pItem->GetHBitmapForMenu(wxMenuItem::Unchecked);
                }
                else if ( pItem->GetBitmap().IsOk() )
                {
                    mii.fMask |= MIIM_BITMAP;
                    mii.hbmpItem = pItem->GetHBitmapForMenu(wxMenuItem::Normal);
                }

                mii.cch = itemText.length();
                boost::nowide::wstackstring stackItem(itemText.c_str());
                mii.dwTypeData = stackItem.get();

                if ( flags & MF_POPUP )
                {
                    mii.fMask |= MIIM_SUBMENU;
                    mii.hSubMenu = GetHmenuOf(pItem->GetSubMenu());
                }
                else
                {
                    mii.fMask |= MIIM_ID;
                    mii.wID = id;
                }

                if ( flags & MF_GRAYED )
                {
                    mii.fMask |= MIIM_STATE;
                    mii.fState = MFS_GRAYED;
                }

                if ( flags & MF_CHECKED )
                {
                    mii.fMask |= MIIM_STATE;
                    mii.fState = MFS_CHECKED;
                }

                if ( flags & MF_MENUBREAK )
                {
                    mii.fMask |= MIIM_FTYPE;
                    mii.fType = MFT_MENUBREAK;
                }

                mii.dwItemData = reinterpret_cast<ULONG_PTR>(pItem);

                ok = ::InsertMenuItemW(GetHmenu(), pos, TRUE /* by pos */, &mii);
                if ( !ok )
                {
                    wxLogLastError("InsertMenuItem()");
#if wxUSE_OWNER_DRAWN
            // In case of failure switch new item to the owner-drawn mode.
            makeItemOwnerDrawn = true;
#endif
                }
                else // InsertMenuItem() ok
                {
                    // we need to remove the extra indent which is reserved for
                    // the checkboxes by default as it looks ugly unless check
                    // boxes are used together with bitmaps and this is not the
                    // case in wx API
                    WinStruct<MENUINFO> mi;
                    mi.fMask = MIM_STYLE;
                    mi.dwStyle = MNS_CHECKORBMP;
                    if ( !::SetMenuInfo(GetHmenu(), &mi) )
                    {
                        wxLogLastError("SetMenuInfo(MNS_NOCHECK)");
                    }
                }
        }

#if wxUSE_OWNER_DRAWN
    if ( pItem->IsOwnerDrawn() || makeItemOwnerDrawn )
        {
            // item draws itself, pass pointer to it in data parameter
            flags |= MF_OWNERDRAW;
            pData = (LPCWSTR)pItem;

            bool updateAllMargins = false;

            // get size of bitmap always return valid value (0 for invalid bitmap),
            // so we don't needed check if bitmap is valid ;)
            int uncheckedW = pItem->GetBitmap(false).GetWidth();
            int checkedW   = pItem->GetBitmap(true).GetWidth();

            if ( m_maxBitmapWidth < uncheckedW )
            {
                m_maxBitmapWidth = uncheckedW;
                updateAllMargins = true;
            }

            if ( m_maxBitmapWidth < checkedW )
            {
                m_maxBitmapWidth = checkedW;
                updateAllMargins = true;
            }

            // make other item ownerdrawn and update margin width for equals alignment
            if ( !m_ownerDrawn || updateAllMargins )
            {
                // we must use position in SetOwnerDrawnMenuItem because
                // all separators have the same id
                int position = 0;
                wxMenuItemList::compatibility_iterator node = GetMenuItems().GetFirst();
                while (node)
                {
                    wxMenuItem* item = node->GetData();

                    // Current item is already added to the list of items
                    // but is not yet physically attached to the menu
                    // so we have to skip setting it as an owner drawn.
                    // It will be done later on when the item will be created.
                    if ( !item->IsOwnerDrawn() && item != pItem )
                    {
                        item->SetOwnerDrawn(true);
                        SetOwnerDrawnMenuItem(GetHmenu(), position,
                                              reinterpret_cast<ULONG_PTR>(item), TRUE);
                    }

                    item->SetMarginWidth(m_maxBitmapWidth);

                    node = node->GetNext();
                    // Current item is already added to the list of items
                    // but is not yet physically attached to the menu
                    // so it cannot be counted while determining position
                    // in the menu.
                    if ( item != pItem )
                        position++;
                }

                // set menu as ownerdrawn
                m_ownerDrawn = true;

                ResetMaxAccelWidth();
            }
            // only update our margin for equals alignment to other item
            else if ( !updateAllMargins )
            {
                pItem->SetMarginWidth(m_maxBitmapWidth);
            }
        }
#endif // wxUSE_OWNER_DRAWN

    // item might have already been inserted by InsertMenuItem() above
    if ( !ok )
    {
        if ( !::InsertMenuW(GetHmenu(), pos, flags | MF_BYPOSITION, id, pData) )
        {
            wxLogLastError("InsertMenu[Item]()");

            return false;
        }

#if wxUSE_OWNER_DRAWN
        if ( makeItemOwnerDrawn )
        {
            pItem->SetOwnerDrawn(true);
            SetOwnerDrawnMenuItem(GetHmenu(), pos,
                                  reinterpret_cast<ULONG_PTR>(pItem), TRUE);
        }
#endif
    }


    // Check the item if it should be initially checked.
    if ( checkInitially )
        pItem->Check(true);

    // if we just appended the title, highlight it
    if ( id == (UINT_PTR)idMenuTitle )
    {
        // visually select the menu title
        SetDefaultMenuItem(GetHmenu(), id);
    }

    // if we're already attached to the menubar, we must update it
    if ( IsAttached() && GetMenuBar()->IsAttached() )
    {
        GetMenuBar()->Refresh();
    }

    return true;
}

wxMenuItem* wxMenu::DoAppend(wxMenuItem *item)
{
    return wxMenuBase::DoAppend(item) && DoInsertOrAppend(item) ? item : nullptr;
}

wxMenuItem* wxMenu::DoInsert(size_t pos, wxMenuItem *item)
{
    if (wxMenuBase::DoInsert(pos, item) && DoInsertOrAppend(item, pos))
        return item;
    else
        return nullptr;
}

wxMenuItem *wxMenu::DoRemove(wxMenuItem *item)
{
    // we need to find the item's position in the child list
    std::size_t pos;
    wxMenuItemList::compatibility_iterator node = GetMenuItems().GetFirst();
    for ( pos = 0; node; pos++ )
    {
        if ( node->GetData() == item )
            break;

        node = node->GetNext();
    }

#if wxUSE_ACCEL
    RemoveAccel(item);
#endif // wxUSE_ACCEL

    // Update indices of radio groups.
    if ( m_radioData )
    {
        if ( m_radioData->UpdateOnRemoveItem(pos) )
        {
            wxASSERT_MSG( item->IsRadio(),
                          "Removing non radio button from radio group?" );
        }
        //else: item being removed is not in a radio group
    }

    // remove the item from the menu
    if ( !::RemoveMenu(GetHmenu(), wx::narrow_cast<WXUINT>(pos), MF_BYPOSITION) )
    {
        wxLogLastError("RemoveMenu");
    }

    if ( IsAttached() && GetMenuBar()->IsAttached() )
    {
        // otherwise, the change won't be visible
        GetMenuBar()->Refresh();
    }

    // and from internal data structures
    return wxMenuBase::DoRemove(item);
}

// ---------------------------------------------------------------------------
// accelerator helpers
// ---------------------------------------------------------------------------

#if wxUSE_ACCEL

// create the wxAcceleratorEntries for our accels and put them into a vector
void wxMenu::CopyAccels(std::vector<wxAcceleratorEntry>& accels) const
{
    std::copy(m_accels.begin(), m_accels.end(), std::back_inserter(accels));
}

wxAcceleratorTable wxMenu::CreateAccelTable() const
{
    std::vector<wxAcceleratorEntry> entries;
    CopyAccels(entries);
    return wxAcceleratorTable{entries};
}

#endif // wxUSE_ACCEL

// ---------------------------------------------------------------------------
// ownerdrawn helpers
// ---------------------------------------------------------------------------

#if wxUSE_OWNER_DRAWN

void wxMenu::CalculateMaxAccelWidth()
{
    wxASSERT_MSG( m_maxAccelWidth == -1, "it's really needed?" );

    wxMenuItemList::compatibility_iterator node = GetMenuItems().GetFirst();
    while (node)
    {
        wxMenuItem* item = node->GetData();

        if ( item->IsOwnerDrawn() )
        {
            int width = item->MeasureAccelWidth();
            if (width > m_maxAccelWidth )
                m_maxAccelWidth = width;
        }

        node = node->GetNext();
    }
}

#endif // wxUSE_OWNER_DRAWN

// ---------------------------------------------------------------------------
// set wxMenu title
// ---------------------------------------------------------------------------

void wxMenu::SetTitle(const std::string& label)
{
    bool hasNoTitle = m_title.empty();
    m_title = label;

    WXHMENU hMenu = GetHmenu();

    if ( hasNoTitle )
    {
        if ( !label.empty() )
        {
            if ( !::InsertMenuW(hMenu, 0u, MF_BYPOSITION | MF_STRING,
                               (UINT_PTR)idMenuTitle, boost::nowide::widen(m_title).c_str()) ||
                 !::InsertMenuW(hMenu, 1u, MF_BYPOSITION, (unsigned)-1, nullptr) )
            {
                wxLogLastError("InsertMenu");
            }
        }
    }
    else
    {
        if ( label.empty() )
        {
            // remove the title and the separator after it
            if ( !::RemoveMenu(hMenu, 0, MF_BYPOSITION) ||
                 !::RemoveMenu(hMenu, 0, MF_BYPOSITION) )
            {
                wxLogLastError("RemoveMenu");
            }
        }
        else
        {
            // modify the title
            if ( !::ModifyMenuW(hMenu, 0u,
                             MF_BYPOSITION | MF_STRING,
                             (UINT_PTR)idMenuTitle, boost::nowide::widen(m_title).c_str()) )
            {
                wxLogLastError("ModifyMenu");
            }
        }
    }

    // put the title string in bold face
    if ( !m_title.empty() )
    {
        SetDefaultMenuItem(GetHmenu(), (WXUINT)idMenuTitle);
    }
}

// ---------------------------------------------------------------------------
// event processing
// ---------------------------------------------------------------------------

bool wxMenu::MSWCommand([[maybe_unused]] WXUINT param, WXWORD id_)
{
    const int id = (signed short)id_;

    // ignore commands from the menu title
    if ( id != idMenuTitle )
    {
        // Default value for uncheckable items.
        int checked = -1;

        // update the check item when it's clicked
        wxMenuItem * const item = FindItem(id);
        if ( item )
        {
            if ( item->IsRadio() && item->IsChecked() )
                return true;

            if ( item->IsCheckable() )
            {
                item->Toggle();

                // Get the status of the menu item: note that it has been just changed
                // by Toggle() above so here we already get the new state of the item.
                //
                // Also notice that we must pass unsigned id_ and not sign-extended id
                // to ::GetMenuState() as this is what it expects.
                WXUINT menuState = ::GetMenuState(GetHmenu(), id_, MF_BYCOMMAND);
                checked = (menuState & MF_CHECKED) != 0;
            }

            item->GetMenu()->SendEvent(id, checked);
        }
        else
        {
            SendEvent(id, checked);
        }
    }

    return true;
}

// get the menu with given handle (recursively)
wxMenu* wxMenu::MSWGetMenu(WXHMENU hMenu)
{
    // check self
    if ( GetHMenu() == hMenu )
        return this;

    // recursively query submenus
    for ( size_t n = 0 ; n < GetMenuItemCount(); ++n )
    {
        wxMenuItem* item = FindItemByPosition(n);
        wxMenu* submenu = item->GetSubMenu();
        if ( submenu )
        {
            submenu = submenu->MSWGetMenu(hMenu);
            if (submenu)
                return submenu;
        }
    }

    // unknown hMenu
    return nullptr;
}

// ---------------------------------------------------------------------------
// Menu Bar
// ---------------------------------------------------------------------------

wxMenuBar::wxMenuBar()
{
    m_eventHandler = this;
}

wxMenuBar::wxMenuBar( [[maybe_unused]] unsigned int style )
{
    m_eventHandler = this;
}

wxMenuBar::wxMenuBar(size_t count, wxMenu *menus[], const std::string titles[], [[maybe_unused]] unsigned int style)
{
    m_eventHandler = this;

    // FIXME: Indexing into two separate arrays
    for ( size_t i = 0; i < count; i++ )
    {
        // We just want to store the menu title in the menu itself, not to
        // show it as a dummy item in the menu itself as we do with the popup
        // menu titles in overridden wxMenu::SetTitle().
        menus[i]->wxMenuBase::SetTitle(titles[i]);
        m_menus.Append(menus[i]);

        menus[i]->Attach(this);
    }
}

wxMenuBar::~wxMenuBar()
{
    // we should free Windows resources only if Windows doesn't do it for us
    // which happens if we're attached to a frame
    if (m_hMenu && !IsAttached())
    {
        ::DestroyMenu((WXHMENU)m_hMenu);
        m_hMenu = (WXHMENU)nullptr;
    }
}

// ---------------------------------------------------------------------------
// wxMenuBar helpers
// ---------------------------------------------------------------------------

void wxMenuBar::Refresh()
{
    if ( IsFrozen() )
        return;

    wxCHECK_RET( IsAttached(), "can't refresh unattached menubar" );

    DrawMenuBar(GetHwndOf(GetFrame()));
}

WXHMENU wxMenuBar::Create()
{
    if ( m_hMenu != nullptr )
        return m_hMenu;

    m_hMenu = (WXHMENU)::CreateMenu();

    if ( !m_hMenu )
    {
        wxLogLastError("CreateMenu");
    }
    else
    {
        for ( wxMenuList::iterator it = m_menus.begin();
              it != m_menus.end();
              ++it )
        {
            if ( !::AppendMenuW((WXHMENU)m_hMenu, MF_POPUP | MF_STRING,
                               (UINT_PTR)(*it)->GetHMenu(),
                               boost::nowide::widen((*it)->GetTitle()).c_str()) )
            {
                wxLogLastError("AppendMenu");
            }
        }
    }

    return m_hMenu;
}

int wxMenuBar::MSWPositionForWxMenu(wxMenu *menu, int wxpos)
{
    wxASSERT(menu);
    wxASSERT(menu->GetHMenu());
    wxASSERT(m_hMenu);

    int totalMSWItems = GetMenuItemCount((WXHMENU)m_hMenu);

    int i; // For old C++ compatibility
    for(i=wxpos; i<totalMSWItems; i++)
    {
        if(GetSubMenu((WXHMENU)m_hMenu,i)==(WXHMENU)menu->GetHMenu())
            return i;
    }
    for(i=0; i<wxpos; i++)
    {
        if(GetSubMenu((WXHMENU)m_hMenu,i)==(WXHMENU)menu->GetHMenu())
            return i;
    }
    wxFAIL;
    return -1;
}

// ---------------------------------------------------------------------------
// wxMenuBar functions to work with the top level submenus
// ---------------------------------------------------------------------------

// NB: we don't support owner drawn top level items for now, if we do these
//     functions would have to be changed to use wxMenuItem as well

void wxMenuBar::EnableTop(size_t pos, bool enable)
{
    wxCHECK_RET( IsAttached(), "doesn't work with unattached menubars" );
    wxCHECK_RET( pos < GetMenuCount(), "invalid menu index" );

    int flag = enable ? MF_ENABLED : MF_GRAYED;

    ::EnableMenuItem((WXHMENU)m_hMenu, MSWPositionForWxMenu(GetMenu(pos),pos), MF_BYPOSITION | flag);

    Refresh();
}

bool wxMenuBar::IsEnabledTop(size_t pos) const
{
    wxCHECK_MSG( pos < GetMenuCount(), false, "invalid menu index" );
    WinStruct<MENUITEMINFOW> mii;
    mii.fMask = MIIM_STATE;
    if ( !::GetMenuItemInfoW(GetHmenu(), pos, TRUE, &mii) )
    {
        wxLogLastError("GetMenuItemInfo(menubar)");
    }

    return !(mii.fState & MFS_GRAYED);
}

void wxMenuBar::SetMenuLabel(size_t pos, const std::string& label)
{
    wxCHECK_RET( pos < GetMenuCount(), "invalid menu index" );

    m_menus[pos]->wxMenuBase::SetTitle(label);

    if ( !IsAttached() )
    {
        return;
    }
    //else: have to modify the existing menu

    int mswpos = MSWPositionForWxMenu(GetMenu(pos),pos);

    UINT_PTR id;
    WXUINT flagsOld = ::GetMenuState((WXHMENU)m_hMenu, mswpos, MF_BYPOSITION);
    if ( flagsOld == 0xFFFFFFFF )
    {
        wxLogLastError("GetMenuState");

        return;
    }

    if ( flagsOld & MF_POPUP )
    {
        // HIBYTE contains the number of items in the submenu in this case
        flagsOld &= 0xff;
        id = (UINT_PTR)::GetSubMenu((WXHMENU)m_hMenu, mswpos);
    }
    else
    {
        id = pos;
    }

    if ( ::ModifyMenuW(GetHmenu(), mswpos, MF_BYPOSITION | MF_STRING | flagsOld,
                      id, boost::nowide::widen(label).c_str()) == (int)0xFFFFFFFF )
    {
        wxLogLastError("ModifyMenu");
    }

    Refresh();
}

std::string wxMenuBar::GetMenuLabel(size_t pos) const
{
    wxCHECK_MSG( pos < GetMenuCount(), "",
                 "invalid menu index in wxMenuBar::GetMenuLabel" );

    return m_menus[pos]->GetTitle();
}

// ---------------------------------------------------------------------------
// wxMenuBar construction
// ---------------------------------------------------------------------------

wxMenu *wxMenuBar::Replace(size_t pos, wxMenu *menu, const std::string& title)
{
    wxMenu *menuOld = wxMenuBarBase::Replace(pos, menu, title);
    if ( !menuOld )
        return nullptr;

    menu->wxMenuBase::SetTitle(title);

    if (GetHmenu())
    {
        int mswpos = MSWPositionForWxMenu(menuOld,pos);

        // can't use ModifyMenu() because it deletes the submenu it replaces
        if ( !::RemoveMenu(GetHmenu(), (WXUINT)mswpos, MF_BYPOSITION) )
        {
            wxLogLastError("RemoveMenu");
        }

        if ( !::InsertMenuW(GetHmenu(), (WXUINT)mswpos,
                           MF_BYPOSITION | MF_POPUP | MF_STRING,
                           (UINT_PTR)GetHmenuOf(menu),
                           boost::nowide::widen(title).c_str()) )
        {
            wxLogLastError("InsertMenu");
        }

#if wxUSE_ACCEL
        if ( menuOld->HasAccels() || menu->HasAccels() )
        {
            // need to rebuild accell table
            RebuildAccelTable();
        }
#endif // wxUSE_ACCEL

        if (IsAttached())
            Refresh();
    }

    return menuOld;
}

bool wxMenuBar::Insert(size_t pos, wxMenu *menu, const std::string& title)
{
    // Find out which MSW item before which we'll be inserting before
    // wxMenuBarBase::Insert is called and GetMenu(pos) is the new menu.
    // If IsAttached() is false this won't be used anyway
    bool isAttached =
        (GetHmenu() != nullptr);

    if ( !wxMenuBarBase::Insert(pos, menu, title) )
        return false;

    menu->wxMenuBase::SetTitle(title);

    if ( isAttached )
    {
        // We have a problem with the index if there is an extra "Window" menu
        // in this menu bar, which is added by wxMDIParentFrame to it directly
        // using Windows API (so that it remains invisible to the user code),
        // but which does affect the indices of the items we insert after it.
        // So we check if any of the menus before the insertion position is a
        // foreign one and adjust the insertion index accordingly.
        int mswExtra = 0;

        // Skip all this if the total number of menus matches (notice that the
        // internal menu count has already been incremented by wxMenuBarBase::
        // Insert() call above, hence -1).
        int mswCount = ::GetMenuItemCount(GetHmenu());
        if ( mswCount != -1 &&
                static_cast<unsigned>(mswCount) != GetMenuCount() - 1 )
        {
            wxMenuList::compatibility_iterator node = m_menus.GetFirst();
            for ( size_t n = 0; n < pos; n++ )
            {
                if ( ::GetSubMenu(GetHmenu(), n) != GetHmenuOf(node->GetData()) )
                    mswExtra++;
                else
                    node = node->GetNext();
            }
        }

        if ( !::InsertMenuW(GetHmenu(), pos + mswExtra,
                           MF_BYPOSITION | MF_POPUP | MF_STRING,
                           (UINT_PTR)GetHmenuOf(menu),
                           boost::nowide::widen(title).c_str()) )
        {
            wxLogLastError("InsertMenu");
        }
#if wxUSE_ACCEL
        if ( menu->HasAccels() )
        {
            // need to rebuild accell table
            RebuildAccelTable();
        }
#endif // wxUSE_ACCEL

        if (IsAttached())
            Refresh();
    }

    return true;
}

bool wxMenuBar::Append(wxMenu *menu, const std::string& title)
{
    WXHMENU submenu = menu ? menu->GetHMenu() : nullptr;
    wxCHECK_MSG( submenu, false, "can't append invalid menu to menubar" );

    if ( !wxMenuBarBase::Append(menu, title) )
        return false;

    menu->wxMenuBase::SetTitle(title);

    if (GetHmenu())
    {
        if ( !::AppendMenuW(GetHmenu(),
                            MF_POPUP | MF_STRING,
                           (UINT_PTR)submenu,
                           boost::nowide::widen(title).c_str()) )
        {
            wxLogLastError("AppendMenu");
        }

#if wxUSE_ACCEL
        if ( menu->HasAccels() )
        {
            // need to rebuild accelerator table
            RebuildAccelTable();
        }
#endif // wxUSE_ACCEL

        if (IsAttached())
            Refresh();
    }

    return true;
}

wxMenu *wxMenuBar::Remove(size_t pos)
{
    wxMenu *menu = wxMenuBarBase::Remove(pos);
    if ( !menu )
        return nullptr;

    if (GetHmenu())
    {
        if ( !::RemoveMenu(GetHmenu(), (WXUINT)MSWPositionForWxMenu(menu,pos), MF_BYPOSITION) )
        {
            wxLogLastError("RemoveMenu");
        }

#if wxUSE_ACCEL
        if ( menu->HasAccels() )
        {
            // need to rebuild accell table
            RebuildAccelTable();
        }
#endif // wxUSE_ACCEL

        if (IsAttached())
            Refresh();
    }

    return menu;
}

#if wxUSE_ACCEL

void wxMenuBar::RebuildAccelTable()
{
    std::vector<wxAcceleratorEntry> accelEntries;

    std::ranges::for_each(m_menus,
        [&accelEntries](const auto& aMenu){
            aMenu->CopyAccels(accelEntries);
    });

    SetAcceleratorTable(wxAcceleratorTable(accelEntries));
}

#endif // wxUSE_ACCEL

void wxMenuBar::Attach(wxFrame *frame)
{
    wxMenuBarBase::Attach(frame);

#if wxUSE_ACCEL
    RebuildAccelTable();
#endif // wxUSE_ACCEL
}

void wxMenuBar::Detach()
{
    wxMenuBarBase::Detach();
}

int wxMenuBar::MSWGetTopMenuPos(WXHMENU hMenu) const
{
    for ( size_t n = 0 ; n < GetMenuCount(); ++n )
    {
        wxMenu* menu = GetMenu(n)->MSWGetMenu(hMenu);
        if ( menu )
            return n;
    }

    return wxNOT_FOUND;
}

wxMenu* wxMenuBar::MSWGetMenu(WXHMENU hMenu) const
{
    // If we're called with the handle of the menu bar itself, we can return
    // immediately as it certainly can't be the handle of one of our menus.
    if ( hMenu == GetHMenu() )
        return nullptr;

    // query all menus
    for ( size_t n = 0 ; n < GetMenuCount(); ++n )
    {
        wxMenu* menu = GetMenu(n)->MSWGetMenu(hMenu);
        if ( menu )
            return menu;
    }

    // unknown hMenu
    return nullptr;
}

#endif // wxUSE_MENUS
