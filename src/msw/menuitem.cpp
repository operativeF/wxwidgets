///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/menuitem.cpp
// Purpose:     wxMenuItem implementation
// Author:      Vadim Zeitlin
// Modified by:
// Created:     11.11.97
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_MENUS

#include "wx/menuitem.h"
#include "wx/stockitem.h"

#include "wx/msw/private.h"

#include "wx/dcmemory.h"
#include "wx/font.h"
#include "wx/bitmap.h"
#include "wx/window.h"
#include "wx/accel.h"
#include "wx/log.h"
#include "wx/menu.h"

#if wxUSE_ACCEL
    #include "wx/accel.h"
#endif // wxUSE_ACCEL

#include "wx/msw/dc.h"
#include "wx/msw/uxtheme.h"

#include <boost/nowide/convert.hpp>
#include <boost/nowide/stackstring.hpp>

import WX.WinDef;
import WX.Win.UniqueHnd;
import Utils.Strings;
import WX.Utils.Settings;

import <string>;

// ---------------------------------------------------------------------------
// macro
// ---------------------------------------------------------------------------

// hide the ugly cast
#define GetHMenuOf(menu)    ((WXHMENU)menu->GetHMenu())

// ----------------------------------------------------------------------------
// helper classes for temporarily changing WXHDC parameters
// ----------------------------------------------------------------------------

namespace
{

// This class just stores an WXHDC.
class HDCHandler
{
protected:
    explicit HDCHandler(WXHDC hdc) : m_hdc(hdc) { }

    const WXHDC m_hdc;
};

class HDCTextColChanger : HDCHandler
{
public:
    HDCTextColChanger(WXHDC hdc, COLORREF col)
        : HDCHandler(hdc),
          m_colOld(::SetTextColor(hdc, col))
    {
    }

    ~HDCTextColChanger()
    {
        ::SetTextColor(m_hdc, m_colOld);
    }

private:
    COLORREF m_colOld;
};

class HDCBgColChanger : HDCHandler
{
public:
    HDCBgColChanger(WXHDC hdc, COLORREF col)
        : HDCHandler(hdc),
          m_colOld(::SetBkColor(hdc, col))
    {
    }

    ~HDCBgColChanger()
    {
        ::SetBkColor(m_hdc, m_colOld);
    }

private:
    COLORREF m_colOld;
};

class HDCBgModeChanger : HDCHandler
{
public:
    HDCBgModeChanger(WXHDC hdc, int mode)
        : HDCHandler(hdc),
          m_modeOld(::SetBkMode(hdc, mode))
    {
    }

    ~HDCBgModeChanger()
    {
        ::SetBkMode(m_hdc, m_modeOld);
    }

private:
    int m_modeOld;
};

inline bool IsGreaterThanStdSize(const wxBitmap& bmp, const wxWindow* win)
{
    return bmp.GetWidth() > wxGetSystemMetrics(SM_CXMENUCHECK, win) ||
            bmp.GetHeight() > wxGetSystemMetrics(SM_CYMENUCHECK, win);
}

} // anonymous namespace

// ============================================================================
// implementation
// ============================================================================

#if wxUSE_OWNER_DRAWN

#include "wx/fontutil.h"
#include "wx/msw/private/metrics.h"

#endif // wxUSE_OWNER_DRAWN

// ----------------------------------------------------------------------------
// dynamic classes implementation
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// wxMenuItem
// ----------------------------------------------------------------------------

#if wxUSE_OWNER_DRAWN

namespace
{

// helper class to keep information about metrics and other stuff
// needed for measuring and drawing menu item
class MenuDrawData
{
public:
    // Wrapper around standard MARGINS structure providing some helper
    // functions and automatically initializing the margin fields to 0.
    struct Margins
#if wxUSE_UXTHEME
        : MARGINS
#endif // wxUSE_UXTHEME
    {
        Margins()
        {
            cxLeftWidth =
            cxRightWidth =
            cyTopHeight =
            cyBottomHeight = 0;
        }

        int GetTotalX() const { return cxLeftWidth + cxRightWidth; }
        int GetTotalY() const { return cyTopHeight + cyBottomHeight; }

        void ApplyTo(RECT& rect) const
        {
            rect.top += cyTopHeight;
            rect.left += cxLeftWidth;
            rect.right -= cyTopHeight;
            rect.bottom -= cyBottomHeight;
        }

        void UnapplyFrom(RECT& rect) const
        {
            rect.top -= cyTopHeight;
            rect.left -= cxLeftWidth;
            rect.right += cyTopHeight;
            rect.bottom += cyBottomHeight;
        }

#if !wxUSE_UXTHEME
        // When MARGINS struct is not available, we need to define the fields
        // we use ourselves.
        int cxLeftWidth, cxRightWidth, cyTopHeight, cyBottomHeight;
#endif // !wxUSE_UXTHEME
    };

    wxFont Font;                // default menu font

    Margins ItemMargin;         // popup item margins

    Margins CheckMargin;        // popup check margins
    Margins CheckBgMargin;      // popup check background margins

    Margins ArrowMargin;        // popup submenu arrow margins

    Margins SeparatorMargin;    // popup separator margins

    SIZE CheckSize;             // popup check size metric
    SIZE ArrowSize;             // popup submenu arrow size metric
    SIZE SeparatorSize;         // popup separator size metric

    int TextBorder;             // popup border space between
                                // item text and gutter

    int AccelBorder;            // popup border space between
                                // item text and accelerator

    int ArrowBorder;            // popup border space between
                                // item accelerator and submenu arrow

    int Offset;                 // system added space at the end of the menu,
                                // add this offset for remove the extra space

    int dpi;                    // DPI used for calculating sizes

    bool AlwaysShowCues;        // must keyboard cues always be shown?

    bool Theme;                 // is data initialized for MenuLayoutType::FullTheme?

    static const MenuDrawData* Get(wxMenu* menu)
    {
        const wxWindow* window = menu->GetWindow();
        // notice that s_menuData can't be created as a global variable because
        // it needs a window to initialize and no windows exist at the time of
        // globals initialization yet
        if ( !ms_instance )
        {
            static MenuDrawData s_menuData(window);
            ms_instance = &s_menuData;
        }

    #if wxUSE_UXTHEME
        const bool theme = MenuLayout() == MenuLayoutType::FullTheme;
        if ( ms_instance->Theme != theme )
        {
            ms_instance->Init(window);
        }
        else
    #endif // wxUSE_UXTHEME
        {
            if ( ms_instance->dpi != window->GetDPI().y )
            {
                ms_instance->Init(window);
                menu->ResetMaxAccelWidth();
            }
        }
        return ms_instance;
    }

    explicit MenuDrawData(const wxWindow* window)
    {
        Init(window);
    }


    // get the theme engine or NULL if themes
    // are not available or not supported on menu
    static bool IsUxThemeActive()
    {
    #if wxUSE_UXTHEME
        if ( MenuLayout() == MenuLayoutType::FullTheme )
            return true;
    #endif // wxUSE_UXTHEME
        return false;
    }


    enum class MenuLayoutType
    {
        FullTheme,      // full menu themes (Vista or new)
        PseudoTheme,    // pseudo menu themes (on XP)
        Classic
    };

    static MenuLayoutType MenuLayout()
    {
        MenuLayoutType menu = MenuLayoutType::Classic;
    #if wxUSE_UXTHEME
        if ( wxUxThemeIsActive() )
        {
            static wxWinVersion ver = wxGetWinVersion();
            if ( ver >= wxWinVersion_Vista )
                menu = MenuLayoutType::FullTheme;
            else if ( ver == wxWinVersion_XP )
                menu = MenuLayoutType::PseudoTheme;
        }
    #endif // wxUSE_UXTHEME
        return menu;
    }

private:
    void Init(wxWindow const* window);

    static MenuDrawData* ms_instance;
};

MenuDrawData* MenuDrawData::ms_instance = nullptr;

void MenuDrawData::Init(wxWindow const* window)
{
#if wxUSE_UXTHEME
    if ( IsUxThemeActive() )
    {
        wxUxThemeHandle hTheme(window, "MENU");

        ::GetThemeMargins(hTheme, nullptr, MENU_POPUPITEM, 0,
                               TMT_CONTENTMARGINS, nullptr,
                               &ItemMargin);

        ::GetThemeMargins(hTheme, nullptr, MENU_POPUPCHECK, 0,
                               TMT_CONTENTMARGINS, nullptr,
                               &CheckMargin);
        ::GetThemeMargins(hTheme, nullptr, MENU_POPUPCHECKBACKGROUND, 0,
                               TMT_CONTENTMARGINS, nullptr,
                               &CheckBgMargin);

        ::GetThemeMargins(hTheme, nullptr, MENU_POPUPSUBMENU, 0,
                               TMT_CONTENTMARGINS, nullptr,
                               &ArrowMargin);

        ::GetThemeMargins(hTheme, nullptr, MENU_POPUPSEPARATOR, 0,
                               TMT_SIZINGMARGINS, nullptr,
                               &SeparatorMargin);

        ::GetThemePartSize(hTheme, nullptr, MENU_POPUPCHECK, 0,
                                nullptr, TS_TRUE, &CheckSize);

        ::GetThemePartSize(hTheme, nullptr, MENU_POPUPSUBMENU, 0,
                                nullptr, TS_TRUE, &ArrowSize);

        ::GetThemePartSize(hTheme, nullptr, MENU_POPUPSEPARATOR, 0,
                                nullptr, TS_TRUE, &SeparatorSize);

        ::GetThemeInt(hTheme, MENU_POPUPBACKGROUND, 0, TMT_BORDERSIZE, &TextBorder);

        AccelBorder = 34;
        ArrowBorder = 0;

        Offset = -14;

        wxUxThemeFont themeFont;
        ::GetThemeSysFont(hTheme, TMT_MENUFONT, themeFont.GetPtr());
        // Use NULL window for wxNativeFontInfo, height it is already at the correct ppi
        Font = wxFont(wxNativeFontInfo(themeFont.GetLOGFONT(), nullptr));

        Theme = true;

        // native menu doesn't uses the vertical margins
        ItemMargin.cyTopHeight =
        ItemMargin.cyBottomHeight = 0;

        // native menu uses small top margin for separator
        if ( SeparatorMargin.cyTopHeight >= 2 )
            SeparatorMargin.cyTopHeight -= 2;
    }
    else
#endif // wxUSE_UXTHEME
    {
        const NONCLIENTMETRICSW& metrics = wxMSWImpl::GetNonClientMetrics(window);

        CheckMargin.cxLeftWidth =
        CheckMargin.cxRightWidth  = wxGetSystemMetrics(SM_CXEDGE, window);
        CheckMargin.cyTopHeight =
        CheckMargin.cyBottomHeight = wxGetSystemMetrics(SM_CYEDGE, window);

        CheckSize.cx = wxGetSystemMetrics(SM_CXMENUCHECK, window);
        CheckSize.cy = wxGetSystemMetrics(SM_CYMENUCHECK, window);

        ArrowSize = CheckSize;

        // separator height with margins
        const int sepFullSize = metrics.iMenuHeight / 2;

        SeparatorMargin.cxLeftWidth =
        SeparatorMargin.cxRightWidth = 1;
        SeparatorMargin.cyTopHeight =
        SeparatorMargin.cyBottomHeight = sepFullSize / 2 - 1;

        SeparatorSize.cx = 1;
        SeparatorSize.cy = sepFullSize - SeparatorMargin.GetTotalY();

        TextBorder = 0;
        AccelBorder = 8;
        ArrowBorder = 6;

        Offset = -12;

        Font = wxFont(wxNativeFontInfo(metrics.lfMenuFont, window));

        Theme = false;
    }

    int value;
    if ( ::SystemParametersInfoW(SPI_GETKEYBOARDCUES, 0, &value, 0) == 0 )
    {
        // if it's not supported, we must be on an old Windows version
        // which always shows them
        value = 1;
    }

    AlwaysShowCues = value == 1;

    dpi = window->GetDPI().y;
}

} // anonymous namespace

#endif // wxUSE_OWNER_DRAWN


// ctor & dtor
// -----------

wxMenuItem::wxMenuItem(wxMenu *pParentMenu,
                       int id,
                       const std::string& text,
                       const std::string& strHelp,
                       wxItemKind kind,
                       wxMenu *pSubMenu)
          : wxMenuItemBase(pParentMenu, id, text, strHelp, kind, pSubMenu)
{
    
#if  wxUSE_OWNER_DRAWN

    // when the color is not valid, wxOwnerDraw takes the default ones.
    // If we set the colors here and they are changed by the user during
    // the execution, then the colors are not updated until the application
    // is restarted and our menus look bad
    SetTextColour(wxNullColour);
    SetBackgroundColour(wxNullColour);

    // setting default colors switched ownerdraw on: switch it off again
    SetOwnerDrawn(false);

    //  switch ownerdraw back on if using a non default margin
    if ( !IsSeparator() )
        SetMarginWidth(GetMarginWidth());

#endif // wxUSE_OWNER_DRAWN

}

// misc
// ----

// return the id for calling Win32 API functions
WXWPARAM wxMenuItem::GetMSWId() const
{
    // we must use ids in unsigned short range with Windows functions, if we
    // pass ids > USHRT_MAX to them they get very confused (e.g. start
    // generating WM_COMMAND messages with negative high word of wParam), so
    // use the cast to ensure the id is in range
    return m_subMenu ? wxPtrToUInt(m_subMenu->GetHMenu())
                     : static_cast<unsigned short>(GetId());
}

// get item state
// --------------

bool wxMenuItem::IsChecked() const
{
    // fix that RTTI is always getting the correct state (separators cannot be
    // checked, but the Windows call below returns true
    if ( IsSeparator() )
        return false;

    // the item might not be attached to a menu yet
    //
    // TODO: shouldn't we just always call the base class version? It seems
    //       like it ought to always be in sync
    if ( !m_parentMenu )
        return wxMenuItemBase::IsChecked();

    WXHMENU hmenu = GetHMenuOf(m_parentMenu);
    int flag = ::GetMenuState(hmenu, GetMSWId(), MF_BYCOMMAND);

    return (flag & MF_CHECKED) != 0;
}

// change item state
// -----------------

void wxMenuItem::Enable(bool enable)
{
    if ( m_isEnabled == enable )
        return;

    const int itemPos = MSGetMenuItemPos();
    if ( itemPos != -1 )
    {
        long rc = ::EnableMenuItem(GetHMenuOf(m_parentMenu),
                                 itemPos,
                                 MF_BYPOSITION |
                                 (enable ? MF_ENABLED : MF_GRAYED));

        if ( rc == -1 )
        {
            wxLogLastError("EnableMenuItem");
        }
    }

    wxMenuItemBase::Enable(enable);
}

void wxMenuItem::Check(bool check)
{
    wxCHECK_RET( IsCheckable(), "only checkable items may be checked" );

    if ( m_isChecked == check )
        return;

    if ( m_parentMenu )
    {
        unsigned int flags = check ? MF_CHECKED : MF_UNCHECKED;
        WXHMENU hmenu = GetHMenuOf(m_parentMenu);

        if ( GetKind() == wxITEM_RADIO )
        {
            // it doesn't make sense to uncheck a radio item -- what would this
            // do?
            if ( !check )
                return;

            // get the index of this item in the menu
            const wxMenuItemList& items = m_parentMenu->GetMenuItems();
            int pos = items.IndexOf(this);
            wxCHECK_RET( pos != wxNOT_FOUND,
                         "menuitem not found in the menu items list?" );

            // get the radio group range
            int start,
                end;

            if ( !m_parentMenu->MSWGetRadioGroupRange(pos, &start, &end) )
            {
                wxFAIL_MSG( "Menu radio item not part of radio group?" );
                return;
            }

            // calling CheckMenuRadioItem() with such parameters hangs my system
            // (NT4 SP6) and I suspect this could happen to the others as well,
            // so don't do it!
            wxCHECK_RET( start != -1 && end != -1,
                         "invalid ::CheckMenuRadioItem() parameter(s)" );

            if ( !::CheckMenuRadioItem(hmenu,
                                       start,   // the first radio group item
                                       end,     // the last one
                                       pos,     // the one to check
                                       MF_BYPOSITION) )
            {
                wxLogLastError("CheckMenuRadioItem");
            }

            // also uncheck all the other items in this radio group
            wxMenuItemList::compatibility_iterator node = items.Item(start);
            for ( int n = start; n <= end && node; n++ )
            {
                if ( n != pos )
                {
                    node->GetData()->m_isChecked = false;
                }

                node = node->GetNext();
            }
        }
        else // check item
        {
            if ( ::CheckMenuItem(hmenu,
                                 GetMSWId(),
                                 MF_BYCOMMAND | flags) == (WXDWORD)-1 )
            {
                wxFAIL_MSG("CheckMenuItem() failed, item not in the menu?");
            }
        }
    }

    wxMenuItemBase::Check(check);
}

void wxMenuItem::SetItemLabel(const std::string& txt)
{
    // don't do anything if label didn't change
    if ( m_text == txt )
        return;

    // wxMenuItemBase will do stock ID checks
    wxMenuItemBase::SetItemLabel(txt);

    // the item can be not attached to any menu yet and SetItemLabel() is still
    // valid to call in this case and should do nothing else
    if ( !m_parentMenu )
        return;

#if wxUSE_ACCEL
    m_parentMenu->UpdateAccel(this);
#endif // wxUSE_ACCEL

    const int itemPos = MSGetMenuItemPos();
    if ( itemPos == -1 )
        return;

    WXHMENU hMenu = GetHMenuOf(m_parentMenu);

    // update the text of the native menu item
    WinStruct<MENUITEMINFOW> info;

    // surprisingly, calling SetMenuItemInfo() with just MIIM_STRING doesn't
    // work as it resets the menu bitmap, so we need to first get the old item
    // state and then modify it
    info.fMask = MIIM_STATE |
                 MIIM_ID |
                 MIIM_SUBMENU |
                 MIIM_CHECKMARKS |
                 MIIM_DATA |
                 MIIM_BITMAP |
                 MIIM_FTYPE;
    if ( !::GetMenuItemInfoW(hMenu, itemPos, TRUE, &info) )
    {
        wxLogLastError("GetMenuItemInfo");
        return;
    }

#if wxUSE_OWNER_DRAWN
    // Don't set the text for the owner drawn items, they don't use it and even
    // though setting it doesn't seem to actually do any harm under Windows 7,
    // avoid doing this relatively nonsensical operation just in case it does
    // break something on other, past or future, Windows versions.
    //
    // Notice that we do need to call SetMenuItemInfo() even for the ownerdrawn
    // items however as otherwise their size wouldn't be recalculated as
    // WM_MEASUREITEM wouldn't be sent and this could result in display
    // problems if the length of the menu item changed significantly.
    //
    // Also notice that we shouldn't use our IsOwnerDrawn() because it can be
    // true because it was set by e.g. SetBitmap(), even if the item wasn't
    // made owner drawn at Windows level.
    if ( !(info.fState & MF_OWNERDRAW) )
#endif // wxUSE_OWNER_DRAWN
    {
        info.fMask |= MIIM_STRING;
        boost::nowide::wstackstring stackText(m_text.c_str());
        info.dwTypeData = stackText.get();
        info.cch = m_text.length();
    }

    if ( !::SetMenuItemInfoW(hMenu, itemPos, TRUE, &info) )
    {
        wxLogLastError("SetMenuItemInfo");
    }
}

void wxMenuItem::DoSetBitmap(const wxBitmap& bmpNew, bool bChecked)
{
    wxBitmap& bmp = bChecked ? m_bmpChecked : m_bmpUnchecked;
    if ( bmp.IsSameAs(bmpNew) )
        return;

#if wxUSE_IMAGE
    if ( !bmpNew.HasAlpha() && wxGetWinVersion() >= wxWinVersion_Vista)
    {
        // we must use PARGB DIB for the menu bitmaps so ensure that we do
        wxImage img(bmpNew.ConvertToImage());
        img.InitAlpha();
        bmp = wxBitmap(img);
    }
    else
#endif // wxUSE_IMAGE
    {
        bmp = bmpNew;
    }

#if wxUSE_OWNER_DRAWN
    // already marked as owner-drawn, cannot be reverted
    if ( IsOwnerDrawn() )
        return;

    if ( MSWMustUseOwnerDrawn() )
    {
        SetOwnerDrawn(true);

        // Parent menu has to be rearranged/recalculated in this case
        // (all other menu items have to be also set to owner-drawn mode).
        if ( m_parentMenu )
        {
            size_t pos;
            wxMenuItem *item = m_parentMenu->FindChildItem(GetId(), &pos);
            if ( item )
            {
                wxCHECK_RET( item == this, "Non unique menu item ID?" );

                // Use a copied value of m_parentMenu because it is
                // nullified by Remove.
                wxMenu *menu = m_parentMenu;
                menu->Remove(this);
                menu->Insert(pos, this);
            }
            //else: the item hasn't been inserted into the parent menu yet
        }
        return;
    }
#endif // wxUSE_OWNER_DRAWN

    const int itemPos = MSGetMenuItemPos();
    if ( itemPos == -1 )
    {
        // The item is probably not attached to any menu yet. SetBitmap() is
        // still valid to call in this case, just do nothing else here.
        return;
    }

    // update the bitmap of the native menu item
    // don't set hbmpItem for the checkable items as it would
    // be used for both checked and unchecked state
    WinStruct<MENUITEMINFOW> mii;
    if ( IsCheckable() )
    {
        mii.fMask = MIIM_CHECKMARKS;
        mii.hbmpChecked = GetHBitmapForMenu(Checked);
        mii.hbmpUnchecked = GetHBitmapForMenu(Unchecked);
    }
    else
    {
        mii.fMask = MIIM_BITMAP;
        mii.hbmpItem = GetHBitmapForMenu(Normal);
    }

    if ( !::SetMenuItemInfoW(GetHMenuOf(m_parentMenu), itemPos, TRUE, &mii) )
    {
        wxLogLastError("SetMenuItemInfo");
    }
}

#if wxUSE_OWNER_DRAWN

int wxMenuItem::MeasureAccelWidth() const
{
    std::string accel = wx::utils::AfterFirst(GetItemLabel(), '\t');

    wxMemoryDC dc;
    wxFont font;
    GetFontToUse(font);
    dc.SetFont(font);

    return dc.GetTextExtent(accel).x;
}

std::string wxMenuItem::GetName() const
{
    return GetItemLabelText();
}

bool wxMenuItem::OnMeasureItem(size_t *width, size_t *height)
{
    const MenuDrawData* data = MenuDrawData::Get(GetMenu());

    if ( IsOwnerDrawn() )
    {
        if ( IsSeparator() )
        {
            *width  = data->ItemMargin.GetTotalX();
            *height = data->ItemMargin.GetTotalY();

            *width  += data->SeparatorSize.cx
                     + data->SeparatorMargin.GetTotalX();
            *height += data->SeparatorSize.cy
                     + data->SeparatorMargin.GetTotalY();
            return true;
        }

        std::string str = GetName();

        wxMemoryDC dc;
        wxFont font;
        GetFontToUse(font);
        dc.SetFont(font);

        auto textExtents = dc.GetTextExtent(str);

        *width = data->TextBorder + textExtents.x + data->AccelBorder;
        *height = textExtents.y;

        textExtents.x = m_parentMenu->GetMaxAccelWidth();
        if ( textExtents.x > 0 )
            *width += textExtents.x + data->ArrowBorder;

        *width += data->Offset;
        *width += data->ArrowMargin.GetTotalX() + data->ArrowSize.cx;
    }
    else // don't draw the text, just the bitmap (if any)
    {
        *width = 0;
        *height = 0;
    }

    // bitmap

    if ( IsOwnerDrawn() )
    {
        // width of menu icon with margins in ownerdrawn menu
        // if any bitmap is not set, the width of space reserved for icon
        // image is equal to the width of std check mark,
        // if bitmap is set, then the width is set to the width of the widest
        // bitmap in menu (GetMarginWidth()) unless std check mark is wider,
        // then it's is set to std mark's width
        int imgWidth = std::max(GetMarginWidth(), static_cast<int>(data->CheckSize.cx))
                     + data->CheckMargin.GetTotalX();

        *width += imgWidth + data->CheckBgMargin.GetTotalX();
    }

    if ( m_bmpChecked.IsOk() || m_bmpUnchecked.IsOk() )
    {
        // get size of bitmap always return valid value (0 for invalid bitmap),
        // so we don't needed check if bitmap is valid ;)
        size_t heightBmp = std::max(m_bmpChecked.GetHeight(), m_bmpUnchecked.GetHeight());
        size_t widthBmp = std::max(m_bmpChecked.GetWidth(),  m_bmpUnchecked.GetWidth());

        if ( IsOwnerDrawn() )
        {
            heightBmp += data->CheckMargin.GetTotalY();
        }
        else
        {
            // we must allocate enough space for the bitmap
            *width += widthBmp;
        }

        // Is BMP height larger than text height?
        if ( *height < heightBmp )
            *height = heightBmp;
    }

    // make sure that this item is at least as tall as the system menu height
    const size_t menuHeight = data->CheckMargin.GetTotalY()
                            + data->CheckSize.cy;
    if (*height < menuHeight)
        *height = menuHeight;

    return true;
}

bool wxMenuItem::OnDrawItem(wxDC& dc, const wxRect& rc,
                            [[maybe_unused]] wxODAction act, wxODStatus stat)
{
    const MenuDrawData* data = MenuDrawData::Get(GetMenu());

    wxMSWDCImpl *impl = (wxMSWDCImpl*) dc.GetImpl();
    WXHDC hdc = GetHdcOf(*impl);

    RECT rect;
    wxCopyRectToRECT(rc, rect);

    int imgWidth = std::max(GetMarginWidth(), static_cast<int>(data->CheckSize.cx));

    if ( IsOwnerDrawn() )
    {
        // font and colors to use
        wxFont font;
        GetFontToUse(font);

        wxColour colText, colBack;
        GetColourToUse(stat, colText, colBack);

        // calculate metrics of item parts
        RECT rcSelection = rect;
        data->ItemMargin.ApplyTo(rcSelection);

        RECT rcSeparator = rcSelection;
        data->SeparatorMargin.ApplyTo(rcSeparator);

        RECT rcGutter = rcSelection;
        rcGutter.right = rcGutter.left
                       + data->ItemMargin.cxLeftWidth
                       + data->CheckBgMargin.cxLeftWidth
                       + data->CheckMargin.cxLeftWidth
                       + imgWidth
                       + data->CheckMargin.cxRightWidth
                       + data->CheckBgMargin.cxRightWidth;

        RECT rcText = rcSelection;
        rcText.left = rcGutter.right + data->TextBorder;

        // we draw the text label vertically centered, but this results in it
        // being 1px too low compared to native menus for some reason, fix it
        if ( MenuDrawData::MenuLayout() != MenuDrawData::MenuLayoutType::FullTheme )
            rcText.top--;

#if wxUSE_UXTHEME
        // If a custom background colour is explicitly specified, we should use
        // it instead of the default theme background.
        if ( !GetBackgroundColour().IsOk() && MenuDrawData::IsUxThemeActive() )
        {
            POPUPITEMSTATES state;
            if ( stat & wxODDisabled )
            {
                state = (stat & wxODSelected) ? MPI_DISABLEDHOT
                                              : MPI_DISABLED;
            }
            else if ( stat & wxODSelected )
            {
                state = MPI_HOT;
            }
            else
            {
                state = MPI_NORMAL;
            }

            wxUxThemeHandle hTheme(GetMenu()->GetWindow(), "MENU");

            if ( ::IsThemeBackgroundPartiallyTransparent(hTheme,
                    MENU_POPUPITEM, state) )
            {
                ::DrawThemeBackground(hTheme, hdc,
                                           MENU_POPUPBACKGROUND,
                                           0, &rect, nullptr);
            }

            ::DrawThemeBackground(hTheme, hdc, MENU_POPUPGUTTER,
                                       0, &rcGutter, nullptr);

            if ( IsSeparator() )
            {
                rcSeparator.left = rcGutter.right;
                ::DrawThemeBackground(hTheme, hdc, MENU_POPUPSEPARATOR,
                                           0, &rcSeparator, nullptr);
                return true;
            }

            ::DrawThemeBackground(hTheme, hdc, MENU_POPUPITEM,
                                       state, &rcSelection, nullptr);

        }
        else
#endif // wxUSE_UXTHEME
        {
            if ( IsSeparator() )
            {
                DrawEdge(hdc, &rcSeparator, EDGE_ETCHED, BF_TOP);
                return true;
            }

            using msw::utils::unique_brush;

            auto hbr = unique_brush(::CreateSolidBrush(colBack.GetPixel()));
            SelectInHDC selBrush(hdc, hbr.get());
            ::FillRect(hdc, &rcSelection, hbr.get());
        }


        // draw text label
        // using native API because it recognizes '&'

        HDCTextColChanger changeTextCol(hdc, colText.GetPixel());
        HDCBgColChanger changeBgCol(hdc, colBack.GetPixel());
        HDCBgModeChanger changeBgMode(hdc, TRANSPARENT);

        SelectInHDC selFont(hdc, GetHfontOf(font));


        // item text name without mnemonic for calculating size
        boost::nowide::wstackstring stackName(GetName().c_str());

        SIZE textSize;
        ::GetTextExtentPoint32W(hdc, stackName.get(), stackName.buffer_size, &textSize);

        // item text name with mnemonic
        auto text = wx::utils::BeforeFirst(GetItemLabel(), '\t');

        unsigned int flags = DST_PREFIXTEXT;
        // themes menu is using specified color for disabled labels
        if ( MenuDrawData::MenuLayout() == MenuDrawData::MenuLayoutType::Classic &&
             (stat & wxODDisabled) && !(stat & wxODSelected) )
            flags |= DSS_DISABLED;

        if ( (stat & wxODHidePrefix) && !data->AlwaysShowCues )
            flags |= DSS_HIDEPREFIX;

        int x = rcText.left;
        int y = rcText.top + (rcText.bottom - rcText.top - textSize.cy) / 2;

        ::DrawStateW(hdc, nullptr, nullptr, reinterpret_cast<WXLPARAM>(boost::nowide::widen(text).c_str()),
                    text.length(), x, y, 0, 0, flags);

        // ::SetTextAlign(hdc, TA_RIGHT) doesn't work with DSS_DISABLED or DSS_MONO
        // as the last parameter in DrawState() (at least with Windows98). So we have
        // to take care of right alignment ourselves.
        std::string accel = wx::utils::AfterFirst(GetItemLabel(), '\t');

        if ( !accel.empty() )
        {
            SIZE accelSize;
            boost::nowide::wstackstring stackAccelExt(accel.c_str());
            ::GetTextExtentPoint32W(hdc, stackAccelExt.get(), stackAccelExt.buffer_size, &accelSize);

            flags = DST_TEXT;
            // themes menu is using specified color for disabled labels
            if ( MenuDrawData::MenuLayout() == MenuDrawData::MenuLayoutType::Classic &&
                 (stat & wxODDisabled) && !(stat & wxODSelected) )
                flags |= DSS_DISABLED;

            x = rcText.right - data->ArrowMargin.GetTotalX()
                                 - data->ArrowSize.cx
                                 - data->ArrowBorder;

            // right align accel on MenuLayoutType::FullTheme menu, left otherwise
            if ( MenuDrawData::MenuLayout() == MenuDrawData::MenuLayoutType::FullTheme)
                x -= accelSize.cx;
            else
                x -= m_parentMenu->GetMaxAccelWidth();

            y = rcText.top + (rcText.bottom - rcText.top - accelSize.cy) / 2;

            boost::nowide::wstackstring stackAccel(accel.c_str());
            ::DrawStateW(hdc, nullptr, nullptr, reinterpret_cast<WXLPARAM>(stackAccel.get()),
                        stackAccel.buffer_size, x, y, 0, 0, flags);
        }
    }


    // draw the bitmap

    RECT rcImg;
    SetRect(&rcImg,
            rect.left   + data->ItemMargin.cxLeftWidth
                        + data->CheckBgMargin.cxLeftWidth
                        + data->CheckMargin.cxLeftWidth,
            rect.top    + data->ItemMargin.cyTopHeight
                        + data->CheckBgMargin.cyTopHeight
                        + data->CheckMargin.cyTopHeight,
            rect.left   + data->ItemMargin.cxLeftWidth
                        + data->CheckBgMargin.cxLeftWidth
                        + data->CheckMargin.cxLeftWidth
                        + imgWidth,
            rect.bottom - data->ItemMargin.cyBottomHeight
                        - data->CheckBgMargin.cyBottomHeight
                        - data->CheckMargin.cyBottomHeight);

    if ( IsCheckable() && !m_bmpChecked.IsOk() )
    {
        if ( stat & wxODChecked )
        {
            DrawStdCheckMark((WXHDC)hdc, &rcImg, stat);
        }
    }
    else
    {
        wxBitmap bmp;

        if ( stat & wxODDisabled )
        {
            bmp = GetDisabledBitmap();
        }

        if ( !bmp.IsOk() )
        {
            // for not checkable bitmaps we should always use unchecked one
            // because their checked bitmap is not set
            bmp = GetBitmap(!IsCheckable() || (stat & wxODChecked));

#if wxUSE_IMAGE
            if ( bmp.IsOk() && stat & wxODDisabled )
            {
                // we need to grey out the bitmap as we don't have any specific
                // disabled bitmap
                wxImage imgGrey = bmp.ConvertToImage().ConvertToGreyscale();
                if ( imgGrey.IsOk() )
                    bmp = wxBitmap(imgGrey);
            }
#endif // wxUSE_IMAGE
        }

        if ( bmp.IsOk() )
        {
            wxMemoryDC dcMem(&dc);
            dcMem.SelectObjectAsSource(bmp);

            // center bitmap
            int nBmpWidth  = bmp.GetWidth(),
                nBmpHeight = bmp.GetHeight();

            int x = rcImg.left + (imgWidth - nBmpWidth) / 2;
            int y = rcImg.top  + (rcImg.bottom - rcImg.top - nBmpHeight) / 2;
            dc.Blit(wxPoint{x, y}, wxSize{nBmpWidth, nBmpHeight}, &dcMem, wxPoint{0, 0}, wxRasterOperationMode::Copy, true);
        }
    }

    return true;

}

namespace
{

// helper function for draw coloured check mark
void DrawColorCheckMark(WXHDC hdc, int x, int y, int cx, int cy, WXHDC hdcCheckMask, int idxColor)
{
    const COLORREF colBlack = RGB(0, 0, 0);
    const COLORREF colWhite = RGB(255, 255, 255);

    HDCTextColChanger changeTextCol(hdc, colBlack);
    HDCBgColChanger changeBgCol(hdc, colWhite);
    HDCBgModeChanger changeBgMode(hdc, TRANSPARENT);

    // memory DC for color bitmap
    MemoryHDC hdcMem(hdc);

    using msw::utils::unique_bitmap;

    auto hbmpMem = unique_bitmap(::CreateCompatibleBitmap(hdc, cx, cy));

    SelectInHDC selMem(hdcMem, hbmpMem.get());

    RECT rect = { 0, 0, cx, cy };
    ::FillRect(hdcMem, &rect, ::GetSysColorBrush(idxColor));

    const COLORREF colCheck = ::GetSysColor(idxColor);
    if ( colCheck == colWhite )
    {
        ::BitBlt(hdc, x, y, cx, cy, hdcCheckMask, 0, 0, MERGEPAINT);
        ::BitBlt(hdc, x, y, cx, cy, hdcMem, 0, 0, SRCAND);
    }
    else
    {
        if ( colCheck != colBlack )
        {
            static constexpr WXDWORD ROP_DSna = 0x00220326;  // dest = (NOT src) AND dest
            ::BitBlt(hdcMem, 0, 0, cx, cy, hdcCheckMask, 0, 0, ROP_DSna);
        }

        ::BitBlt(hdc, x, y, cx, cy, hdcCheckMask, 0, 0, SRCAND);
        ::BitBlt(hdc, x, y, cx, cy, hdcMem, 0, 0, SRCPAINT);
    }
}

} // anonymous namespace

void wxMenuItem::DrawStdCheckMark(WXHDC hdc_, const RECT* rc, wxODStatus stat)
{
    WXHDC hdc = (WXHDC)hdc_;

#if wxUSE_UXTHEME
    if ( MenuDrawData::IsUxThemeActive() )
    {
        wxUxThemeHandle hTheme(GetMenu()->GetWindow(), "MENU");

        const MenuDrawData* data = MenuDrawData::Get(GetMenu());

        // rect for background must be without check margins
        RECT rcBg = *rc;
        data->CheckMargin.UnapplyFrom(rcBg);

        POPUPCHECKBACKGROUNDSTATES stateCheckBg = (stat & wxODDisabled)
                                                    ? MCB_DISABLED
                                                    : MCB_NORMAL;

        ::DrawThemeBackground(hTheme, hdc, MENU_POPUPCHECKBACKGROUND,
                                   stateCheckBg, &rcBg, nullptr);

        POPUPCHECKSTATES stateCheck;
        if ( GetKind() == wxITEM_CHECK )
        {
            stateCheck = (stat & wxODDisabled) ? MC_CHECKMARKDISABLED
                                               : MC_CHECKMARKNORMAL;
        }
        else
        {
            stateCheck = (stat & wxODDisabled) ? MC_BULLETDISABLED
                                               : MC_BULLETNORMAL;
        }

        ::DrawThemeBackground(hTheme, hdc, MENU_POPUPCHECK,
                                   stateCheck, rc, nullptr);
    }
    else
#endif // wxUSE_UXTHEME
    {
        const int cx = rc->right - rc->left;
        const int cy = rc->bottom - rc->top;

        // first create mask of check mark
        MemoryHDC hdcMask(hdc);

        using msw::utils::unique_bitmap;

        auto hbmpMask = unique_bitmap(::CreateBitmap(cx, cy, 1, 1, nullptr));

        SelectInHDC selMask(hdcMask, hbmpMask.get());

        // then draw a check mark into it
        const WXUINT stateCheck = (GetKind() == wxITEM_CHECK) ? DFCS_MENUCHECK
                                                      : DFCS_MENUBULLET;
        RECT rect = { 0, 0, cx, cy };
        ::DrawFrameControl(hdcMask, &rect, DFC_MENU, stateCheck);

        // first draw shadow if disabled
        if ( (stat & wxODDisabled) && !(stat & wxODSelected) )
        {
            DrawColorCheckMark(hdc, rc->left + 1, rc->top + 1,
                               cx, cy, hdcMask, COLOR_3DHILIGHT);
        }

        // then draw a check mark
        int color = COLOR_MENUTEXT;
        if ( stat & wxODDisabled )
            color = COLOR_BTNSHADOW;
        else if ( stat & wxODSelected )
            color = COLOR_HIGHLIGHTTEXT;

        DrawColorCheckMark(hdc, rc->left, rc->top, cx, cy, hdcMask, color);
    }
}

void wxMenuItem::GetFontToUse(wxFont& font) const
{
    font = GetFont();
    if ( !font.IsOk() )
        font = MenuDrawData::Get(GetMenu())->Font;
    font.WXAdjustToPPI(GetMenu()->GetWindow()->GetDPI());
}

void wxMenuItem::GetColourToUse(wxODStatus stat, wxColour& colText, wxColour& colBack) const
{
#if wxUSE_UXTHEME
    if ( MenuDrawData::IsUxThemeActive() )
    {
        wxUxThemeHandle hTheme(GetMenu()->GetWindow(), "MENU");

        if ( stat & wxODDisabled)
        {
            wxRGBToColour(colText, ::GetThemeSysColor(hTheme, COLOR_GRAYTEXT));
        }
        else
        {
            colText = GetTextColour();
            if ( !colText.IsOk() )
                wxRGBToColour(colText, ::GetThemeSysColor(hTheme, COLOR_MENUTEXT));
        }

        if ( stat & wxODSelected )
        {
            wxRGBToColour(colBack, ::GetThemeSysColor(hTheme, COLOR_HIGHLIGHT));
        }
        else
        {
            colBack = GetBackgroundColour();
            if ( !colBack.IsOk() )
                wxRGBToColour(colBack, ::GetThemeSysColor(hTheme, COLOR_MENU));
        }
    }
    else
#endif // wxUSE_UXTHEME
    {
        wxOwnerDrawn::GetColourToUse(stat, colText, colBack);
    }
}

bool wxMenuItem::MSWMustUseOwnerDrawn()
{
    // we have to use owner drawn item if it has custom colours or font
    bool mustUseOwnerDrawn = GetTextColour().IsOk() ||
                             GetBackgroundColour().IsOk() ||
                             GetFont().IsOk();

    // Windows XP or earlier don't display menu bitmaps bigger than
    // standard size correctly (they're truncated) nor can
    // checked bitmaps use HBMMENU_CALLBACK, so we must use
    // owner-drawn items to show them correctly there. OTOH Win7
    // doesn't seem to have any problems with even very large bitmaps
    // so don't use owner-drawn items unnecessarily there (Vista wasn't
    // actually tested but I assume it works as 7 rather than as XP).
    static const wxWinVersion winver = wxGetWinVersion();
    if ( !mustUseOwnerDrawn && winver < wxWinVersion_Vista )
    {
        const wxBitmap& bmpUnchecked = GetBitmap(false),
                        bmpChecked   = GetBitmap(true);

        const wxWindow* win = m_parentMenu ? m_parentMenu->GetWindow() : nullptr;
        if ( (bmpUnchecked.IsOk() && IsGreaterThanStdSize(bmpUnchecked, win)) ||
                (bmpChecked.IsOk()   && IsGreaterThanStdSize(bmpChecked, win)) ||
                (bmpChecked.IsOk() && IsCheckable()) )
        {
            mustUseOwnerDrawn = true;
        }
    }

    return mustUseOwnerDrawn;
}

#endif // wxUSE_OWNER_DRAWN

// returns the WXHBITMAP to use in MENUITEMINFOW
WXHBITMAP wxMenuItem::GetHBitmapForMenu(BitmapKind kind) const
{
    // Under versions of Windows older than Vista we can't pass WXHBITMAP
    // directly as hbmpItem for 2 reasons:
    //  1. We can't draw it with transparency then (this is not
    //     very important now but would be with themed menu bg)
    //  2. Worse, Windows inverts the bitmap for the selected
    //     item and this looks downright ugly
    //
    // So we prefer to instead draw it ourselves in MSWOnDrawItem() by using
    // HBMMENU_CALLBACK for normal menu items when inserting it. And use
    // NULL for checkable menu items as hbmpChecked/hBmpUnchecked does not
    // support HBMMENU_CALLBACK.
    //
    // However under Vista using HBMMENU_CALLBACK causes the entire menu to be
    // drawn using the MenuLayoutType::Classic theme instead of the current one and it does
    // handle transparency just fine so do use the real bitmap there
#if wxUSE_IMAGE
    if ( wxGetWinVersion() >= wxWinVersion_Vista )
    {
        wxBitmap bmp = GetBitmap(kind != Unchecked);
        if ( bmp.IsOk() )
        {
            return GetHbitmapOf(bmp);
        }
        //else: bitmap is not set
        return nullptr;
    }
#endif // wxUSE_IMAGE

    return (kind == Normal) ? HBMMENU_CALLBACK : nullptr;
}

int wxMenuItem::MSGetMenuItemPos() const
{
    if ( !m_parentMenu )
        return -1;

    const WXHMENU hMenu = GetHMenuOf(m_parentMenu);
    if ( !hMenu )
        return -1;

    const WXWPARAM id = GetMSWId();

    for ( int i = 0; i < ::GetMenuItemCount(hMenu); i++ )
    {
        const WXUINT state = ::GetMenuState(hMenu, i, MF_BYPOSITION);
        if ( state == (WXUINT)-1 )
        {
            // This indicates that the item at this position and is not
            // supposed to happen here, but test for it just in case.
            continue;
        }

        if ( state & MF_POPUP )
        {
            if ( ::GetSubMenu(hMenu, i) == (WXHMENU)id )
                return i;
        }
        else if ( !(state & MF_SEPARATOR) )
        {
            if ( ::GetMenuItemID(hMenu, i) == id )
                return i;
        }
    }

    return -1;
}

// ----------------------------------------------------------------------------
// wxMenuItemBase
// ----------------------------------------------------------------------------

wxMenuItem *wxMenuItemBase::New(wxMenu *parentMenu,
                                int id,
                                const std::string& name,
                                const std::string& help,
                                wxItemKind kind,
                                wxMenu *subMenu)
{
    return new wxMenuItem(parentMenu, id, name, help, kind, subMenu);
}

#endif // wxUSE_MENUS
