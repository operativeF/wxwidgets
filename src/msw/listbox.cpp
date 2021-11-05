///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/listbox.cpp
// Purpose:     wxListBox
// Author:      Julian Smart
// Modified by: Vadim Zeitlin (owner drawn stuff)
// Created:
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_LISTBOX

#include "wx/listbox.h"

#include "wx/dynarray.h"
#include "wx/settings.h"
#include "wx/font.h"
#include "wx/dc.h"
#include "wx/dcclient.h"
#include "wx/utils.h"
#include "wx/log.h"
#include "wx/window.h"

#include "wx/msw/dc.h"

#include "wx/msw/private.h"

#include <windowsx.h>

#include <boost/nowide/convert.hpp>
#include <boost/nowide/stackstring.hpp>

#include <gsl/gsl>

#include <memory>

#if wxUSE_OWNER_DRAWN
    #include  "wx/ownerdrw.h"

    namespace
    {
        // space beneath/above each row in pixels
        const int LISTBOX_EXTRA_SPACE = 1;
    } // anonymous namespace
#endif // wxUSE_OWNER_DRAWN

#if wxUSE_OWNER_DRAWN

class wxListBoxItem : public wxOwnerDrawn
{
public:
    explicit wxListBoxItem(wxListBox *parent) : m_parent(parent)
    {
    }

    wxListBox *GetParent() const
        { return m_parent; }

    int GetIndex() const
        { return m_parent->GetItemIndex(const_cast<wxListBoxItem*>(this)); }

    std::string GetName() const override
        { return m_parent->GetString(GetIndex()); }

private:
    wxListBox *m_parent;
};

std::unique_ptr<wxOwnerDrawn> wxListBox::CreateLboxItem(size_t WXUNUSED(n))
{
    return std::make_unique<wxListBoxItem>(this);
}

#endif  //USE_OWNER_DRAWN

bool wxListBox::Create(wxWindow *parent,
                       wxWindowID id,
                       const wxPoint& pos,
                       const wxSize& size,
                       const std::vector<std::string>& choices,
                       unsigned int style,
                       const wxValidator& validator,
                       const std::string& name)
{
    // initialize base class fields
    if ( !CreateControl(parent, id, pos, size, style, validator, name) )
        return false;

    // create the native control
    if ( !MSWCreateControl("LISTBOX", "", pos, size) )
    {
        // control creation failed
        return false;
    }

    // initialize the contents

    std::for_each(choices.begin(), choices.end(), [this](const auto& choice){ Append(choice); });

    // now we can compute our best size correctly, so do it again
    SetInitialSize(size);

    return true;
}

wxListBox::~wxListBox()
{
    Clear();
}

DWORD wxListBox::MSWGetStyle(unsigned int style, DWORD *exstyle) const
{
    DWORD msStyle = wxControl::MSWGetStyle(style, exstyle);

    // we always want to get the notifications
    msStyle |= LBS_NOTIFY;

    // without this style, you get unexpected heights, so e.g. constraint
    // layout doesn't work properly
    msStyle |= LBS_NOINTEGRALHEIGHT;

    wxASSERT_MSG( !(style & wxLB_MULTIPLE) || !(style & wxLB_EXTENDED),
                  "only one of listbox selection modes can be specified" );

    if ( style & wxLB_MULTIPLE )
        msStyle |= LBS_MULTIPLESEL;
    else if ( style & wxLB_EXTENDED )
        msStyle |= LBS_EXTENDEDSEL;

    wxASSERT_MSG( !(style & wxLB_ALWAYS_SB) || !(style & wxLB_NO_SB),
                  "Conflicting styles wxLB_ALWAYS_SB and wxLB_NO_SB." );

    if ( !(style & wxLB_NO_SB) )
    {
        msStyle |= WS_VSCROLL;
        if ( style & wxLB_ALWAYS_SB )
            msStyle |= LBS_DISABLENOSCROLL;
    }

    if ( m_windowStyle & wxLB_HSCROLL )
        msStyle |= WS_HSCROLL;
    if ( m_windowStyle & wxLB_SORT )
        msStyle |= LBS_SORT;

#if wxUSE_OWNER_DRAWN
    if ( m_windowStyle & wxLB_OWNERDRAW )
    {
        // we don't support LBS_OWNERDRAWVARIABLE yet and we also always put
        // the strings in the listbox for simplicity even though we could have
        // avoided it in this case
        msStyle |= LBS_OWNERDRAWFIXED | LBS_HASSTRINGS;
    }
#endif // wxUSE_OWNER_DRAWN

    // tabs stops are expanded by default on linux/GTK and macOS/Cocoa
    msStyle |= LBS_USETABSTOPS;

    return msStyle;
}

void wxListBox::MSWUpdateFontOnDPIChange(const wxSize& newDPI)
{
    wxListBoxBase::MSWUpdateFontOnDPIChange(newDPI);

    if ( m_font.IsOk() )
        SetFont(m_font);
}

void wxListBox::OnInternalIdle()
{
    wxWindow::OnInternalIdle();

    if (m_updateHorizontalExtent)
    {
        SetHorizontalExtent("");
        m_updateHorizontalExtent = false;
    }
}

void wxListBox::MSWOnItemsChanged()
{
    // we need to do two things when items change: update their max horizontal
    // extent so that horizontal scrollbar could be shown or hidden as
    // appropriate and also invlaidate the best size
    //
    // updating the max extent is slow (it's an O(N) operation) and so we defer
    // it until the idle time but the best size should be invalidated
    // immediately doing it in idle time is too late -- layout using incorrect
    // old best size will have been already done by then

    m_updateHorizontalExtent = true;

    InvalidateBestSize();
}

void wxListBox::EnsureVisible(int n)
{
    wxCHECK_RET( IsValid(n),
                 "invalid index in wxListBox::EnsureVisible" );

    // when item is before the first visible item, make the item the first visible item
    const auto firstItem = ::SendMessageW(GetHwnd(), LB_GETTOPINDEX, 0, 0);
    if ( n <= firstItem )
    {
        DoSetFirstItem(n);
        return;
    }

    // retrieve item height in order to compute last visible item and scroll amount
    const auto itemHeight = ::SendMessageW(GetHwnd(), LB_GETITEMHEIGHT, 0, 0);
    if ( itemHeight == LB_ERR || itemHeight == 0)
        return;

    // compute the amount of fully visible items
    int countVisible = GetClientSize().y / itemHeight;
    if ( !countVisible )
        countVisible = 1;

    // when item is before the last fully visible item, it is already visible
    const auto lastItem = firstItem + countVisible - 1;
    if ( n <= lastItem )
        return;

    // make the item the last visible item by setting the first visible item accordingly
    DoSetFirstItem(n - countVisible + 1);
}

int wxListBox::GetTopItem() const
{
    return ::SendMessageW(GetHwnd(), LB_GETTOPINDEX, 0, 0);
}

int wxListBox::GetCountPerPage() const
{
    const auto lineHeight = ::SendMessageW(GetHwnd(), LB_GETITEMHEIGHT, 0, 0);
    if ( lineHeight == LB_ERR || lineHeight == 0 )
        return -1;

    const RECT r = wxGetClientRect(GetHwnd());

    return (r.bottom - r.top) / lineHeight;
}

void wxListBox::DoSetFirstItem(int N)
{
    wxCHECK_RET( IsValid(N),
                 "invalid index in wxListBox::SetFirstItem" );

    ::SendMessageW(GetHwnd(), LB_SETTOPINDEX, (WPARAM)N, (LPARAM)0);
}

void wxListBox::DoDeleteOneItem(unsigned int n)
{
    wxCHECK_RET( IsValid(n),
                 "invalid index in wxListBox::Delete" );

#if wxUSE_OWNER_DRAWN
    if ( HasFlag(wxLB_OWNERDRAW) )
    {
        m_aItems.erase(m_aItems.begin() + n);
    }
#endif // wxUSE_OWNER_DRAWN

    ::SendMessageW(GetHwnd(), LB_DELETESTRING, n, 0);
    m_noItems--;

    MSWOnItemsChanged();

    UpdateOldSelections();
}

int wxListBox::FindString(std::string_view s, bool bCase) const
{
    // back to base class search for not native search type
    if (bCase)
       return wxItemContainerImmutable::FindString( s, bCase );

    int pos = ListBox_FindStringExact(GetHwnd(), -1, boost::nowide::widen(s).c_str());
    if (pos == LB_ERR)
        return wxNOT_FOUND;
    else
        return pos;
}

void wxListBox::DoClear()
{
#if wxUSE_OWNER_DRAWN
    if ( HasFlag(wxLB_OWNERDRAW) )
    {
        m_aItems.clear();
    }
#endif // wxUSE_OWNER_DRAWN

    ListBox_ResetContent(GetHwnd());

    m_noItems = 0;
    MSWOnItemsChanged();

    UpdateOldSelections();
}

void wxListBox::DoSetSelection(int N, bool select)
{
    wxCHECK_RET( N == wxNOT_FOUND || IsValid(N),
                 "invalid index in wxListBox::SetSelection" );

    if ( HasMultipleSelection() )
    {
        // Setting selection to -1 should deselect everything.
        const bool deselectAll = N == wxNOT_FOUND;
        ::SendMessageW(GetHwnd(), LB_SETSEL,
                    deselectAll ? FALSE : select,
                    deselectAll ? -1 : N);
    }
    else
    {
        ::SendMessageW(GetHwnd(), LB_SETCURSEL, select ? N : -1, 0);
    }

    UpdateOldSelections();
}

bool wxListBox::IsSelected(int N) const
{
    wxCHECK_MSG( IsValid(N), false,
                 "invalid index in wxListBox::Selected" );

    return ::SendMessageW(GetHwnd(), LB_GETSEL, N, 0) != 0;
}

void *wxListBox::DoGetItemClientData(unsigned int n) const
{
    // This is done here for the same reasons as in wxChoice method with the
    // same name.
    SetLastError(ERROR_SUCCESS);

    auto rc = ::SendMessageW(GetHwnd(), LB_GETITEMDATA, n, 0);
    if ( rc == LB_ERR && GetLastError() != ERROR_SUCCESS )
    {
        wxLogLastError("LB_GETITEMDATA");

        return nullptr;
    }

    return (void *)rc;
}

void wxListBox::DoSetItemClientData(unsigned int n, void *clientData)
{
    if ( ListBox_SetItemData(GetHwnd(), n, clientData) == LB_ERR )
    {
        wxLogDebug("LB_SETITEMDATA failed");
    }
}

// Return number of selections and an array of selected integers
int wxListBox::GetSelections(std::vector<int>& aSelections) const
{
    aSelections.clear();

    if ( HasMultipleSelection() )
    {
        int countSel = ListBox_GetSelCount(GetHwnd());
        if ( countSel == LB_ERR )
        {
            wxLogDebug("ListBox_GetSelCount failed");
        }
        else if ( countSel != 0 )
        {
            auto selections{std::make_unique<int[]>(countSel)};

            if ( ListBox_GetSelItems(GetHwnd(),
                                     countSel, selections.get()) == LB_ERR )
            {
                wxLogDebug("ListBox_GetSelItems failed");
                countSel = -1;
            }
            else
            {
                aSelections.reserve(countSel);
                for ( int n = 0; n < countSel; n++ )
                    aSelections.push_back(selections[n]);
            }
        }

        return countSel;
    }
    else  // single-selection listbox
    {
        if (ListBox_GetCurSel(GetHwnd()) > -1)
            aSelections.push_back(ListBox_GetCurSel(GetHwnd()));

        return aSelections.size();
    }
}

// Get single selection, for single choice list items
int wxListBox::GetSelection() const
{
    wxCHECK_MSG( !HasMultipleSelection(),
                 -1,
                 "GetSelection() can't be used with multiple-selection listboxes, use GetSelections() instead." );

    return ListBox_GetCurSel(GetHwnd());
}

// Find string for position
std::string wxListBox::GetString(unsigned int n) const
{
    wxCHECK_MSG( IsValid(n), "",
                 "invalid index in wxListBox::GetString" );

    const auto len = ListBox_GetTextLen(GetHwnd(), n);

    // +1 for terminating NUL
    std::wstring result;

    result.resize(len);

    ListBox_GetText(GetHwnd(), n, &result[0]);

    return boost::nowide::narrow(result);
}

int wxListBox::DoInsertItems(const std::vector<std::string>& items,
                             unsigned int pos,
                             void **clientData,
                             wxClientDataType type)
{
    MSWAllocStorage(items, LB_INITSTORAGE);

    const bool append = pos == GetCount();

    // we must use CB_ADDSTRING when appending as only it works correctly for
    // the sorted controls
    const unsigned msg = append ? LB_ADDSTRING : LB_INSERTSTRING;

    if ( append )
        pos = 0;

    int n = wxNOT_FOUND;

    const unsigned int numItems = items.size();
    for ( unsigned int i = 0; i < numItems; i++ )
    {
        n = MSWInsertOrAppendItem(pos, items[i], msg);
        if ( n == wxNOT_FOUND )
            return n;

        if ( !append )
            pos++;

        ++m_noItems;

#if wxUSE_OWNER_DRAWN
        if ( HasFlag(wxLB_OWNERDRAW) )
        {
            std::unique_ptr<wxOwnerDrawn> pNewItem = CreateLboxItem(n);
            pNewItem->SetFont(GetFont());
            m_aItems.insert(m_aItems.begin() + n, std::move(pNewItem));
        }
#endif // wxUSE_OWNER_DRAWN
        AssignNewItemClientData(n, clientData, i, type);
    }

    MSWOnItemsChanged();

    UpdateOldSelections();

    return n;
}

int wxListBox::DoHitTestList(const wxPoint& point) const
{
    const auto lRes = ::SendMessageW(GetHwnd(), LB_ITEMFROMPOINT,
                                 0, MAKELPARAM(point.x, point.y));

    // non zero high-order word means that this item is outside of the client
    // area, IOW the point is outside of the listbox
    return HIWORD(lRes) ? wxNOT_FOUND : LOWORD(lRes);
}

void wxListBox::SetString(unsigned int n, const std::string& s)
{
    wxCHECK_RET( IsValid(n),
                 "invalid index in wxListBox::SetString" );

    // remember the state of the item
    const bool wasSelected = IsSelected(n);

    void *oldData = nullptr;
    wxClientData *oldObjData = nullptr;
    if ( HasClientUntypedData() )
        oldData = GetClientData(n);
    else if ( HasClientObjectData() )
        oldObjData = GetClientObject(n);

    // delete and recreate it
    ::SendMessageW(GetHwnd(), LB_DELETESTRING, n, 0);

    int newN = n;
    if ( n == (m_noItems - 1) )
        newN = -1;

    ListBox_InsertString(GetHwnd(), newN, boost::nowide::widen(s).c_str());

    // restore the client data
    if ( oldData )
        SetClientData(n, oldData);
    else if ( oldObjData )
        SetClientObject(n, oldObjData);

    // we may have lost the selection
    if ( wasSelected )
        Select(n);

    MSWOnItemsChanged();
}

size_t wxListBox::GetCount() const
{
    return m_noItems;
}

void wxListBox::SetHorizontalExtent(const std::string& s)
{
    // the rest is only necessary if we want a horizontal scrollbar
    if ( !HasFlag(wxHSCROLL) )
        return;


    WindowHDC dc(GetHwnd());
    SelectInHDC selFont(dc, GetHfontOf(GetFont()));

    TEXTMETRICW lpTextMetric;
    ::GetTextMetricsW(dc, &lpTextMetric);

    int largestExtent = 0;
    SIZE extentXY{};

    if ( s.empty() )
    {
        // set extent to the max length of all strings
        for ( unsigned int i = 0; i < m_noItems; i++ )
        {
            boost::nowide::wstackstring stackText(GetString(i).c_str());
            ::GetTextExtentPoint32W(dc, stackText.get(), stackText.buffer_size, &extentXY);

            int extentX = (int)(extentXY.cx + lpTextMetric.tmAveCharWidth);
            if ( extentX > largestExtent )
                largestExtent = extentX;
        }
    }
    else // just increase the extent to the length of this string
    {
        int existingExtent = (int)::SendMessageW(GetHwnd(),
                                              LB_GETHORIZONTALEXTENT, 0, 0L);

        boost::nowide::wstackstring stackText(s.c_str());
        ::GetTextExtentPoint32W(dc, stackText.get(), stackText.buffer_size, &extentXY);

        int extentX = (int)(extentXY.cx + lpTextMetric.tmAveCharWidth);
        if ( extentX > existingExtent )
            largestExtent = extentX;
    }

    if ( largestExtent )
    {
        largestExtent = MSWGetFullItemSize(largestExtent, 0 /* height */).x;
        ::SendMessageW(GetHwnd(), LB_SETHORIZONTALEXTENT, LOWORD(largestExtent), 0L);
    }
    //else: it shouldn't change
}

bool wxListBox::MSWSetTabStops(const std::vector<int>& tabStops)
{
    return ::SendMessageW(GetHwnd(), LB_SETTABSTOPS, (WPARAM)tabStops.size(),
                       (LPARAM)(tabStops.empty() ? nullptr : &tabStops[0])) == TRUE;
}

wxSize wxListBox::DoGetBestClientSize() const
{
    // find the widest string
    int wListbox = 0;
    for (unsigned int i = 0; i < m_noItems; i++)
    {
        std::string str(GetString(i));
        auto wLine = GetTextExtent(str).x;
        if ( wLine > wListbox )
            wListbox = wLine;
    }

    // give it some reasonable default value if there are no strings in the
    // list
    if ( wListbox == 0 )
        wListbox = 6*wxGetCharWidth();

    // the listbox should be slightly larger than the widest string
    wListbox += 3*wxGetCharWidth();

    // add room for the scrollbar
    wListbox += wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, m_parent);

    // don't make the listbox too tall (limit height to 10 items) but don't
    // make it too small neither
    auto hListbox = ::SendMessageW(GetHwnd(), LB_GETITEMHEIGHT, 0, 0)*
        std::min(std::max(m_noItems, std::size_t{3}), std::size_t{10});

    return {wListbox, gsl::narrow_cast<int>(hListbox)};
}

bool wxListBox::MSWCommand(WXUINT param, WXWORD WXUNUSED(id))
{
    wxEventType evtType{};
    if ( param == LBN_SELCHANGE )
    {
        if ( HasMultipleSelection() )
            return CalcAndSendEvent();

        evtType = wxEVT_LISTBOX;
    }
    else if ( param == LBN_DBLCLK )
    {
        // Clicking under the last item in the listbox generates double click
        // event for the currently selected item which is rather surprising.
        // Avoid the surprise by checking that we do have an item under mouse.
        const DWORD pos = ::GetMessagePos();
        const wxPoint pt(GET_X_LPARAM(pos), GET_Y_LPARAM(pos));
        if ( HitTest(ScreenToClient(pt)) == wxNOT_FOUND )
            return false;

        evtType = wxEVT_LISTBOX_DCLICK;
    }
    else
    {
        // some event we're not interested in
        return false;
    }

    const int n = ListBox_GetCurSel(GetHwnd());

    // We get events even when mouse is clicked outside of any valid item from
    // Windows, just ignore them.
    if ( n == wxNOT_FOUND )
       return false;

    if ( param == LBN_SELCHANGE )
    {
        if ( !DoChangeSingleSelection(n) )
            return false;
    }

    // Do generate an event otherwise.
    return SendEvent(evtType, n, true /* selection */);
}

#if wxUSE_OWNER_DRAWN

bool wxListBox::SetFont(const wxFont &font)
{
    wxListBoxBase::SetFont(font);

    if ( HasFlag(wxLB_OWNERDRAW) )
    {
        std::for_each(m_aItems.begin(), m_aItems.end(), [this](auto& item){ item->SetFont(m_font); });

        // Non owner drawn list boxes update the item height on their own, but
        // we need to do it manually in the owner drawn case.
        wxClientDC dc(this);
        dc.SetFont(m_font);
        ::SendMessageW(GetHwnd(), LB_SETITEMHEIGHT, 0,
                    dc.GetCharHeight() + 2 * LISTBOX_EXTRA_SPACE);
    }

    return true;
}

bool wxListBox::GetItemRect(size_t n, wxRect& rect) const
{
    wxCHECK_MSG( IsValid(n), false,
                 "invalid index in wxListBox::GetItemRect" );

    RECT rc;

    if ( ListBox_GetItemRect(GetHwnd(), n, &rc) != LB_ERR )
    {
        rect = wxRectFromRECT(rc);
        return true;
    }
    else
    {
        // couldn't retrieve rect: for example, item isn't visible
        return false;
    }
}

bool wxListBox::RefreshItem(size_t n)
{
    wxRect rect;
    if ( !GetItemRect(n, rect) )
        return false;

    RECT rc;
    wxCopyRectToRECT(rect, rc);

    return ::InvalidateRect((HWND)GetHWND(), &rc, FALSE) == TRUE;
}

// the height is the same for all items
// TODO should be changed for LBS_OWNERDRAWVARIABLE style listboxes

// NB: can't forward this to wxListBoxItem because LB_SETITEMDATA
//     message is not yet sent when we get here!
bool wxListBox::MSWOnMeasure(WXMEASUREITEMSTRUCT *item)
{
    // only owner-drawn control should receive this message
    wxCHECK( HasFlag(wxLB_OWNERDRAW), false );

    MEASUREITEMSTRUCT *pStruct = (MEASUREITEMSTRUCT *)item;

    HDC hdc = ::CreateIC(L"DISPLAY", nullptr, nullptr, nullptr);

    {
        wxDCTemp dc((WXHDC)hdc);
        dc.SetFont(GetFont());

        pStruct->itemHeight = dc.GetCharHeight() + 2 * LISTBOX_EXTRA_SPACE;
        pStruct->itemWidth  = dc.wxGetCharWidth();
    }

    DeleteDC(hdc);

    return true;
}

// forward the message to the appropriate item
bool wxListBox::MSWOnDraw(WXDRAWITEMSTRUCT *item)
{
    // only owner-drawn control should receive this message
    wxCHECK( HasFlag(wxLB_OWNERDRAW), false );

    DRAWITEMSTRUCT *pStruct = (DRAWITEMSTRUCT *)item;

    // the item may be -1 for an empty listbox
    if ( pStruct->itemID == (UINT)-1 )
        return false;

    wxOwnerDrawn *pItem = m_aItems[pStruct->itemID].get();

    wxDCTemp dc((WXHDC)pStruct->hDC);

    return pItem->OnDrawItem(dc, wxRectFromRECT(pStruct->rcItem),
                             (wxOwnerDrawn::wxODAction)pStruct->itemAction,
                             (wxOwnerDrawn::wxODStatus)(pStruct->itemState | wxOwnerDrawn::wxODHidePrefix));
}

#endif // wxUSE_OWNER_DRAWN

#endif // wxUSE_LISTBOX
