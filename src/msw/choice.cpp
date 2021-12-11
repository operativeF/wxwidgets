/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/choice.cpp
// Purpose:     wxChoice
// Author:      Julian Smart
// Modified by: Vadim Zeitlin to derive from wxChoiceBase
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_CHOICE

#include "wx/choice.h"
#include "wx/utils.h"
#include "wx/app.h"
#include "wx/log.h"
#include "wx/dynlib.h"

#include "wx/msw/private.h"

#include <boost/nowide/convert.hpp>

import WX.WinDef;

import WX.Utils.Settings;

bool wxChoice::Create(wxWindow *parent,
                      wxWindowID id,
                      const wxPoint& pos,
                      const wxSize& size,
                      const std::vector<std::string>& choices,
                      unsigned int style,
                      const wxValidator& validator,
                      std::string_view name)
{
    // Experience shows that wxChoice vs. wxComboBox distinction confuses
    // quite a few people - try to help them
    wxASSERT_MSG( !(style & wxCB_DROPDOWN) &&
                  !(style & wxCB_READONLY) &&
                  !(style & wxCB_SIMPLE),
                  "this style flag is ignored by wxChoice, you "
                  "probably want to use a wxComboBox" );

    return CreateAndInit(parent, id, pos, size, choices, style,
                         validator, name);
}

bool wxChoice::CreateAndInit(wxWindow *parent,
                             wxWindowID id,
                             const wxPoint& pos,
                             const wxSize& size,
                             const std::vector<std::string>& choices,
                             unsigned int style,
                             const wxValidator& validator,
                             std::string_view name)
{
    // initialize wxControl
    if ( !CreateControl(parent, id, pos, size, style, validator, name) )
        return false;

    // now create the real WXHWND
    if ( !MSWCreateControl("COMBOBOX", "", pos, size) )
        return false;


    // initialize the controls contents
    Append(choices);

    // and now we may finally size the control properly (if needed)
    SetInitialSize(size);

    return true;
}

void wxChoice::SetLabel(std::string_view label)
{
    if ( FindString(label) == wxNOT_FOUND )
    {
        // unless we explicitly do this here, CB_GETCURSEL will continue to
        // return the index of the previously selected item which will result
        // in wrongly replacing the value being set now with the previously
        // value if the user simply opens and closes (without selecting
        // anything) the combobox popup
        SetSelection(-1);
    }

    wxChoiceBase::SetLabel(label);
}

bool wxChoice::MSWShouldPreProcessMessage(WXMSG *pMsg)
{
    MSG *msg = (MSG *) pMsg;

    // if the dropdown list is visible, don't preprocess certain keys
    if ( msg->message == WM_KEYDOWN
        && (msg->wParam == VK_ESCAPE || msg->wParam == VK_RETURN) )
    {
        if (::SendMessageW(GetHwndOf(this), CB_GETDROPPEDSTATE, 0, 0))
        {
            return false;
        }
    }

    return wxControl::MSWShouldPreProcessMessage(pMsg);
}

WXDWORD wxChoice::MSWGetStyle(unsigned int style, WXDWORD *exstyle) const
{
    // we never have an external border
    WXDWORD msStyle = wxControl::MSWGetStyle
                      (
                        (style & ~wxBORDER_MASK) | wxBORDER_NONE, exstyle
                      );

    // WS_CLIPSIBLINGS is useful with wxChoice and doesn't seem to result in
    // any problems
    msStyle |= WS_CLIPSIBLINGS;

    // wxChoice-specific styles
    msStyle |= CBS_DROPDOWNLIST | WS_HSCROLL | WS_VSCROLL;
    if ( style & wxCB_SORT )
        msStyle |= CBS_SORT;

    return msStyle;
}

#ifndef EP_EDITTEXT
    #define EP_EDITTEXT         1
    #define ETS_NORMAL          1
#endif

wxVisualAttributes
wxChoice::GetClassDefaultAttributes([[maybe_unused]] wxWindowVariant variant)
{
    // it is important to return valid values for all attributes from here,
    // GetXXX() below rely on this
    wxVisualAttributes attrs;

    // FIXME: Use better dummy window?
    wxWindow* wnd = wxTheApp->GetTopWindow();
    if (!wnd)
        return attrs;

    attrs.font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);

    // there doesn't seem to be any way to get the text colour using themes
    // API: TMT_TEXTCOLOR doesn't work neither for EDIT nor COMBOBOX
    attrs.colFg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);

    // NB: use EDIT, not COMBOBOX (the latter works in XP but not Vista)
    attrs.colBg = wnd->MSWGetThemeColour(L"EDIT",
                                         EP_EDITTEXT,
                                         ETS_NORMAL,
                                         ThemeColourBackground,
                                         wxSYS_COLOUR_WINDOW);

    return attrs;
}

wxChoice::~wxChoice()
{
    Clear();
}

// ----------------------------------------------------------------------------
// adding/deleting items to/from the list
// ----------------------------------------------------------------------------

int wxChoice::DoInsertItems(const std::vector<std::string>& items,
                            unsigned int pos,
                            void **clientData, wxClientDataType type)
{
    MSWAllocStorage(items, CB_INITSTORAGE);

    const bool append = pos == GetCount();

    // use CB_ADDSTRING when appending at the end to make sure the control is
    // resorted if it has wxCB_SORT style
    const unsigned msg = append ? CB_ADDSTRING : CB_INSERTSTRING;

    if ( append )
        pos = 0;

    int n = wxNOT_FOUND;
    const unsigned numItems = items.size();
    for ( unsigned i = 0; i < numItems; ++i )
    {
        n = MSWInsertOrAppendItem(pos, items[i], msg);
        if ( n == wxNOT_FOUND )
            return n;

        if ( !append )
            pos++;

        AssignNewItemClientData(n, clientData, i, type);
    }

    // we need to refresh our size in order to have enough space for the
    // newly added items
    if ( !IsFrozen() )
        MSWUpdateDropDownHeight();

    InvalidateBestSize();

    return n;
}

void wxChoice::DoDeleteOneItem(unsigned int n)
{
    wxCHECK_RET( IsValid(n), "invalid item index in wxChoice::Delete" );

    ::SendMessageW(GetHwnd(), CB_DELETESTRING, n, 0);

    if ( !IsFrozen() )
        MSWUpdateDropDownHeight();

    InvalidateBestSize();
}

void wxChoice::DoClear()
{
    ::SendMessageW(GetHwnd(), CB_RESETCONTENT, 0, 0);

    if ( !IsFrozen() )
        MSWUpdateDropDownHeight();

    InvalidateBestSize();
}

// ----------------------------------------------------------------------------
// selection
// ----------------------------------------------------------------------------

int wxChoice::GetSelection() const
{
    // if m_lastAcceptedSelection is set, it means that the dropdown is
    // currently shown and that we want to use the last "permanent" selection
    // instead of whatever is under the mouse pointer currently
    //
    // otherwise, get the selection from the control
    return m_lastAcceptedSelection == wxID_NONE ? GetCurrentSelection()
                                                : m_lastAcceptedSelection;
}

int wxChoice::GetCurrentSelection() const
{
    return (int)::SendMessageW(GetHwnd(), CB_GETCURSEL, 0, 0);
}

void wxChoice::SetSelection(int n)
{
    ::SendMessageW(GetHwnd(), CB_SETCURSEL, n, 0);
}

// ----------------------------------------------------------------------------
// string list functions
// ----------------------------------------------------------------------------

size_t wxChoice::GetCount() const
{
    return ::SendMessageW(GetHwnd(), CB_GETCOUNT, 0, 0);
}

int wxChoice::FindString(std::string_view s, bool bCase) const
{
   //TODO:  Evidently some MSW versions (all?) don't like empty strings
   //passed to SendMessage, so we have to do it ourselves in that case
   if ( s.empty() )
   {
       unsigned int count = GetCount();
       for ( unsigned int i = 0; i < count; i++ )
       {
         if (GetString(i).empty())
             return i;
       }

       return wxNOT_FOUND;
   }
   else if (bCase)
   {
       // back to base class search for not native search type
       return wxItemContainerImmutable::FindString( s, bCase );
   }
   else
   {
       int pos = (int)::SendMessageW(GetHwnd(), CB_FINDSTRINGEXACT,
                                  (WXWPARAM)-1, reinterpret_cast<WXLPARAM>(boost::nowide::widen(s).c_str()));

       return pos == LB_ERR ? wxNOT_FOUND : pos;
   }
}

void wxChoice::SetString(unsigned int n, const std::string& s)
{
    wxCHECK_RET( IsValid(n), "invalid item index in wxChoice::SetString" );

    // we have to delete and add back the string as there is no way to change a
    // string in place

    // we need to preserve the client data manually
    void *oldData = nullptr;
    wxClientData *oldObjData = nullptr;
    if ( HasClientUntypedData() )
        oldData = GetClientData(n);
    else if ( HasClientObjectData() )
        oldObjData = GetClientObject(n);

    // and also the selection if we're going to delete the item that was
    // selected
    const bool wasSelected = static_cast<int>(n) == GetSelection();

    // TODO: Correct?
    ::SendMessageW(GetHwnd(), CB_DELETESTRING, n, 0);
    ::SendMessageW(GetHwnd(), CB_INSERTSTRING, n, reinterpret_cast<WXLPARAM>(boost::nowide::widen(s).c_str()) );

    // restore the client data
    if ( oldData )
        SetClientData(n, oldData);
    else if ( oldObjData )
        SetClientObject(n, oldObjData);

    // and the selection
    if ( wasSelected )
        SetSelection(n);

    // the width could have changed so the best size needs to be recomputed
    InvalidateBestSize();
}

std::string wxChoice::GetString(unsigned int n) const
{
    const int len = (int)::SendMessageW(GetHwnd(), CB_GETLBTEXTLEN, n, 0);

    std::wstring str;

    wxCHECK_MSG( len != CB_ERR, boost::nowide::narrow(str), "Invalid index");

    if ( len > 0 )
    {
        str.resize(len);

        // FIXME: Is this correct?
        if ( ::SendMessageW
               (
                GetHwnd(),
                CB_GETLBTEXT,
                n,
                reinterpret_cast<WXLPARAM>(&str[0])
               ) == CB_ERR )
        {
            wxLogLastError("SendMessage(CB_GETLBTEXT)");
        }
    }

    return boost::nowide::narrow(str);
}

// ----------------------------------------------------------------------------
// client data
// ----------------------------------------------------------------------------

void wxChoice::DoSetItemClientData(unsigned int n, void* clientData)
{
    if ( ::SendMessageW(GetHwnd(), CB_SETITEMDATA,
                       n, (WXLPARAM)clientData) == CB_ERR )
    {
        wxLogLastError("CB_SETITEMDATA");
    }
}

void* wxChoice::DoGetItemClientData(unsigned int n) const
{
    // Before using GetLastError() below, ensure that we don't have a stale
    // error code from a previous API call as CB_GETITEMDATA doesn't reset it
    // in case of success, it only sets it if an error occurs.
    SetLastError(ERROR_SUCCESS);

    WXLPARAM rc = SendMessageW(GetHwnd(), CB_GETITEMDATA, n, 0);

    // Notice that we must call GetLastError() to distinguish between a real
    // error and successfully retrieving a previously stored client data value
    // of CB_ERR (-1).
    if ( rc == CB_ERR && GetLastError() != ERROR_SUCCESS )
    {
        wxLogLastError("CB_GETITEMDATA");

        // unfortunately, there is no way to return an error code to the user
        rc = (WXLPARAM) NULL;
    }

    return (void *)rc;
}

// ----------------------------------------------------------------------------
// wxMSW-specific geometry management
// ----------------------------------------------------------------------------

namespace
{

// there is a difference between the height passed to CB_SETITEMHEIGHT and the
// real height of the combobox; it is probably not constant for all Windows
// versions/settings but right now I don't know how to find what it is so it is
// temporarily hardcoded to its value under XP systems with normal fonts sizes
const int COMBO_HEIGHT_ADJ = 6;

} // anonymous namespace

void wxChoice::MSWUpdateVisibleHeight()
{
    if ( m_heightOwn != wxDefaultCoord )
    {
        ::SendMessageW(GetHwnd(), CB_SETITEMHEIGHT,
                      (WXWPARAM)-1, m_heightOwn - COMBO_HEIGHT_ADJ);
    }
}

#if wxUSE_DEFERRED_SIZING
void wxChoice::MSWEndDeferWindowPos()
{
    // we can only set the height of the choice itself now as it is reset to
    // default every time the control is resized
    MSWUpdateVisibleHeight();

    wxChoiceBase::MSWEndDeferWindowPos();
}
#endif // wxUSE_DEFERRED_SIZING

void wxChoice::MSWUpdateDropDownHeight()
{
    unsigned int flags = wxSIZE_USE_EXISTING;

    // be careful to not change the width here
    DoSetSize(wxRect{wxDefaultCoord, wxDefaultCoord, wxDefaultCoord, GetSize().y}, flags);
}

void wxChoice::DoMoveWindow(wxRect boundary)
{
    // here is why this is necessary: if the width is negative, the combobox
    // window proc makes the window of the size width*height instead of
    // interpreting height in the usual manner (meaning the height of the drop
    // down list - usually the height specified in the call to MoveWindow()
    // will not change the height of combo box per se)
    //
    // this behaviour is not documented anywhere, but this is just how it is
    // here (NT 4.4) and, anyhow, the check shouldn't hurt - however without
    // the check, constraints/sizers using combos may break the height
    // constraint will have not at all the same value as expected
    if ( boundary.width < 0 )
        return;

    // the height which we must pass to Windows should be the total height of
    // the control including the drop down list while the height given to us
    // is, of course, just the height of the permanently visible part of it so
    // add the drop down height to it

    // don't make the drop down list too tall, arbitrarily limit it to 30
    // items max and also don't make it too small if it's currently empty
    size_t nItems = GetCount();
    if (!HasFlag(wxCB_SIMPLE))
    {
        if (!nItems)
            nItems = 9;
        else if (nItems > 30)
            nItems = 30;
    }

    int heightWithItems = 0;
    if (!HasFlag(wxCB_SIMPLE))
    {
        // The extra item (" + 1") is required to prevent a vertical
        // scrollbar from appearing with comctl32.dll versions earlier
        // than 6.0 (such as found in Win2k).
        const auto hItem = ::SendMessageW(GetHwnd(), CB_GETITEMHEIGHT, 0, 0);
        heightWithItems = boundary.height + hItem*(nItems + 1);
    }
    else
    {
        heightWithItems = SetHeightSimpleComboBox(nItems);
    }

    wxControl::DoMoveWindow(wxRect{boundary.x, boundary.y, boundary.width, heightWithItems});
}

wxSize wxChoice::DoGetSize() const
{
    wxSize ctrl_sz = wxControl::DoGetSize();

    // FIXME: Find out why this occurs.
    // this is weird: sometimes, the height returned by Windows is clearly the
    // total height of the control including the drop down list -- but only
    // sometimes, and sometimes it isn't so work around this here by using our
    // own stored value if we have it
    if ( ctrl_sz.y && m_heightOwn != wxDefaultCoord )
        ctrl_sz.y = m_heightOwn;

    return ctrl_sz;
}

void wxChoice::DoSetSize(wxRect boundary, unsigned int sizeFlags)
{
    if ( boundary.height == GetBestSize().y )
    {
        // we don't need to manually manage our height, let the system use the
        // default one
        m_heightOwn = wxDefaultCoord;
    }
    else if ( boundary.height != wxDefaultCoord ) // non-default height specified
    {
        // set our new own height but be careful not to make it too big: the
        // native control apparently stores it as a single byte and so setting
        // own height to 256 pixels results in default height being used (255
        // is still ok)
        m_heightOwn = boundary.height;

        if ( m_heightOwn > UCHAR_MAX )
            m_heightOwn = UCHAR_MAX;
        // nor too small: see MSWUpdateVisibleHeight()
        else if ( m_heightOwn < COMBO_HEIGHT_ADJ )
            m_heightOwn = COMBO_HEIGHT_ADJ;
    }

    // do resize the native control
    wxControl::DoSetSize(boundary, sizeFlags);


    // make the control itself of the requested height: notice that this
    // must be done after changing its size or it has no effect (apparently
    // the height is reset to default during the control layout) and that it's
    // useless to do it when using the deferred sizing -- in this case it
    // will be done from MSWEndDeferWindowPos()
#if wxUSE_DEFERRED_SIZING
    if ( m_pendingSize == wxDefaultSize )
    {
        // not using deferred sizing, update it immediately
        MSWUpdateVisibleHeight();
    }
    else // in the middle of deferred sizing
    {
        // we need to report the size of the visible part of the control back
        // in GetSize() and not height stored by DoSetSize() in m_pendingSize
        m_pendingSize = boundary.GetSize();
    }
#else // !wxUSE_DEFERRED_SIZING
    // always update the visible height immediately
    MSWUpdateVisibleHeight();
#endif // wxUSE_DEFERRED_SIZING
}

wxSize wxChoice::DoGetBestSize() const
{
    // The base version returns the size of the largest string
    return GetSizeFromTextSize(wxChoiceBase::DoGetBestSize().x);
}

int wxChoice::SetHeightSimpleComboBox(int nItems) const
{
    wxSize ch_size = wxGetCharSize( GetHWND(), GetFont() );
    auto hItem = ::SendMessageW(GetHwnd(), CB_GETITEMHEIGHT, (WXWPARAM)-1, 0);
    return EDIT_HEIGHT_FROM_CHAR_HEIGHT( ch_size.y ) * std::min( std::max( nItems, 3 ), 6 ) + hItem - 1;
}

wxSize wxChoice::DoGetSizeFromTextSize(int xlen, int ylen) const
{
    int cHeight = GetCharHeight();

    // We are interested in the difference of sizes between the whole control
    // and its child part. I.e. arrow, separators, etc.
    wxSize tsize(xlen, 0);

    WinStruct<COMBOBOXINFO> info;
    if ( ::GetComboBoxInfo(GetHwnd(), &info) )
    {
        tsize.x += info.rcItem.left + info.rcButton.right - info.rcItem.right
                    + info.rcItem.left + 3; // right and extra margins
    }
    else // Just use some rough approximation.
    {
        tsize.x += 4*cHeight;
    }

    // set height on our own
    if( HasFlag( wxCB_SIMPLE ) )
        tsize.y = SetHeightSimpleComboBox(GetCount());
    else
        tsize.y = EDIT_HEIGHT_FROM_CHAR_HEIGHT(cHeight);

    // Perhaps the user wants something different from CharHeight
    if ( ylen > 0 )
        tsize.IncBy(0, ylen - cHeight);

    return tsize;
}

// ----------------------------------------------------------------------------
// Popup operations
// ----------------------------------------------------------------------------

void wxChoice::MSWDoPopupOrDismiss(bool show)
{
    wxASSERT_MSG( !HasFlag(wxCB_SIMPLE),
                  "can't popup/dismiss the list for simple combo box" );

    // we *must* set focus to the combobox before showing or hiding the drop
    // down as without this we get WM_LBUTTONDOWN messages with invalid WXHWND
    // when hiding it (whether programmatically or manually) resulting in a
    // crash when we pass them to IsDialogMessage()
    //
    // this can be seen in the combo page of the widgets sample under Windows 7
    SetFocus();

    ::SendMessageW(GetHwnd(), CB_SHOWDROPDOWN, show, 0);
}

bool wxChoice::Show(bool show)
{
    if ( !wxChoiceBase::Show(show) )
        return false;

    // When hiding the combobox, we also need to hide its popup part as it
    // doesn't happen automatically.
    if ( !show && ::SendMessageW(GetHwnd(), CB_GETDROPPEDSTATE, 0, 0) )
        MSWDoPopupOrDismiss(false);

    return true;
}

// ----------------------------------------------------------------------------
// MSW message handlers
// ----------------------------------------------------------------------------

WXLRESULT wxChoice::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
    switch ( nMsg )
    {
        case WM_LBUTTONUP:
            {
                int x = (int)LOWORD(lParam);
                int y = (int)HIWORD(lParam);

                // Ok, this is truly weird, but if a panel with a wxChoice
                // loses the focus, then you get a *fake* WM_LBUTTONUP message
                // with x = 65535 and y = 65535. Filter out this nonsense.
                //
                // VZ: I'd like to know how to reproduce this please...
                if ( x == 65535 && y == 65535 )
                    return 0;
            }
            break;

            // we have to handle both: one for the normal case and the other
            // for readonly
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORSTATIC:
            {
                WXHDC hdc;
                WXHWND hwnd;
                UnpackCtlColor(wParam, lParam, &hdc, &hwnd);

                WXHBRUSH hbr = MSWControlColor((WXHDC)hdc, hwnd);
                if ( hbr )
                    return (WXLRESULT)hbr;
                //else: fall through to default window proc
            }
    }

    return wxWindow::MSWWindowProc(nMsg, wParam, lParam);
}

bool wxChoice::MSWCommand(WXUINT param, [[maybe_unused]] WXWORD id)
{
    /*
        The native control provides a great variety in the events it sends in
        the different selection scenarios (undoubtedly for greater amusement of
        the programmers using it). Here are the different cases:

        A. Selecting with just the arrows without opening the dropdown:
            1. CBN_SELENDOK
            2. CBN_SELCHANGE

        B. Opening dropdown with F4 and selecting with arrows:
            1. CBN_DROPDOWN
            2. many CBN_SELCHANGE while changing selection in the list
            3. CBN_SELENDOK
            4. CBN_CLOSEUP

        C. Selecting with the mouse:
            1. CBN_DROPDOWN
            -- no intermediate CBN_SELCHANGEs --
            2. CBN_SELENDOK
            3. CBN_CLOSEUP
            4. CBN_SELCHANGE

        Admire the different order of messages in all of those cases, it must
        surely have taken a lot of effort to Microsoft developers to achieve
        such originality.

        Additionally, notice that CBN_SELENDCANCEL doesn't seem to actually
        cancel anything, if we get CBN_SELCHANGE before it, as it happens in
        the case (B), the selection is still accepted. This doesn't make much
        sense and directly contradicts MSDN documentation but is how the native
        comboboxes behave and so we do the same thing.
     */
    switch ( param )
    {
        case CBN_DROPDOWN:
            // we use this value both because we don't want to track selection
            // using CB_GETCURSEL while the dropdown is opened and because we
            // need to reset the selection back to it if it's eventually
            // cancelled by user
            m_lastAcceptedSelection = GetCurrentSelection();
            break;

        case CBN_CLOSEUP:
            if ( m_pendingSelection != wxID_NONE )
            {
                // This can only happen in the case (B), so set the item
                // selected in the drop down as our real selection.
                SendSelectionChangedEvent(wxEVT_CHOICE);
                m_pendingSelection = wxID_NONE;
            }
            break;

        case CBN_SELENDOK:
            // Reset the variables to prevent CBN_CLOSEUP from doing anything,
            // it's not needed if we do get CBN_SELENDOK.
            m_lastAcceptedSelection =
            m_pendingSelection = wxID_NONE;

            SendSelectionChangedEvent(wxEVT_CHOICE);
            break;

        case CBN_SELCHANGE:
            // If we get this event after CBN_SELENDOK, i.e. cases (A) or (C)
            // above, we don't have anything to do. But in the case (B) we need
            // to remember that the selection should really change once the
            // drop down is closed.
            if ( m_lastAcceptedSelection != wxID_NONE )
                m_pendingSelection = GetCurrentSelection();
            break;

        case CBN_SELENDCANCEL:
            // Do not reset m_pendingSelection here -- it would make sense but,
            // as described above, native controls keep the selection even when
            // closing the drop down by pressing Escape or TAB, so conform to
            // their behaviour.
            m_lastAcceptedSelection = wxID_NONE;
            break;

        default:
            return false;
    }

    return true;
}

WXHBRUSH wxChoice::MSWControlColor(WXHDC hDC, WXHWND hWnd)
{
    if ( !IsThisEnabled() )
        return MSWControlColorDisabled(hDC);

    return wxChoiceBase::MSWControlColor(hDC, hWnd);
}

#endif // wxUSE_CHOICE
