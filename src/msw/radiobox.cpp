/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/radiobox.cpp
// Purpose:     wxRadioBox implementation
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_RADIOBOX

#include "wx/radiobox.h"
#include "wx/log.h"

#include "wx/msw/subwin.h"
#include "wx/renderer.h"

#if wxUSE_TOOLTIPS
    #include "wx/tooltip.h"
#endif // wxUSE_TOOLTIPS

#include <boost/nowide/convert.hpp>

import WX.Utils.Settings;

import WX.Win.UniqueHnd;

// ---------------------------------------------------------------------------
// private functions
// ---------------------------------------------------------------------------

// wnd proc for radio buttons
LRESULT APIENTRY
wxRadioBtnWndProc(WXHWND hWnd, WXUINT message, WXWPARAM wParam, WXLPARAM lParam);

// ---------------------------------------------------------------------------
// global vars
// ---------------------------------------------------------------------------

namespace
{

// the pointer to standard radio button wnd proc
WXWNDPROC s_wndprocRadioBtn = nullptr;

// Hash allowing to find wxRadioBox containing the given radio button by its
// WXHWND. This is used by (subclassed) radio button window proc to find the
// radio box it belongs to.
using RadioBoxFromButton = std::unordered_map< WXHWND, wxRadioBox *, wxPointerHash, wxPointerEqual >;


RadioBoxFromButton gs_boxFromButton;

} // anonymous namespace

// ===========================================================================
// implementation
// ===========================================================================

/* static */
wxRadioBox* wxRadioBox::GetFromRadioButtonHWND(WXHWND hwnd)
{
    const RadioBoxFromButton::const_iterator it = gs_boxFromButton.find(hwnd);
    return it == gs_boxFromButton.end() ? NULL : it->second;
}

// ---------------------------------------------------------------------------
// wxRadioBox creation
// ---------------------------------------------------------------------------

bool wxRadioBox::Create(wxWindow *parent,
                        wxWindowID id,
                        const std::string& title,
                        const wxPoint& pos,
                        const wxSize& size,
                        const std::vector<std::string>& choices,
                        int majorDim,
                        unsigned int style,
                        const wxValidator& val,
                        std::string_view name)
{
    // common initialization
    if ( !wxStaticBox::Create(parent, id, title, pos, size, style, name) )
        return false;

    // the code elsewhere in this file supposes that either wxRA_SPECIFY_COLS
    // or wxRA_SPECIFY_ROWS is set, ensure that this is indeed the case
    if ( !(style & (wxRA_SPECIFY_ROWS | wxRA_SPECIFY_COLS)) )
        style |= wxRA_SPECIFY_COLS;

#if wxUSE_VALIDATORS
    SetValidator(val);
#else
    wxUnusedVar(val);
#endif // wxUSE_VALIDATORS/!wxUSE_VALIDATORS

    // We need an extra one to keep track of the 'dummy' item we
    // create to end the radio group, so it will be destroyed and
    // it's id will be released.  But we want it separate from the
    // other buttons since the wxSubwindows will operate on it as
    // well and we just want to ignore it until destroying it.
    // For instance, we don't want the bounding box of the radio
    // buttons to include the dummy button
    m_radioButtons = new wxSubwindows(choices.size());

    for ( std::size_t i{0}; const auto& choice : choices )
    {
        long styleBtn = BS_AUTORADIOBUTTON | WS_TABSTOP | WS_CHILD | WS_VISIBLE;
        if ( i == 0 )
            styleBtn |= WS_GROUP;

        wxWindowIDRef subid = NewControlId();

        WXHWND hwndBtn = ::CreateWindowW(L"BUTTON",
                                      boost::nowide::widen(choice).c_str(),
                                      styleBtn,
                                      0, 0, 0, 0,   // will be set in SetSize()
                                      GetHwndOf(parent),
                                      (WXHMENU)wxUIntToPtr(subid.GetValue()),
                                      wxGetInstance(),
                                      nullptr);

        if ( !hwndBtn )
        {
            wxLogLastError("CreateWindow(radio btn)");

            return false;
        }

        // Keep track of the subwindow
        m_radioButtons->Set(i, hwndBtn, subid);

        SubclassRadioButton((WXHWND)hwndBtn);

        // Also, make it a subcontrol of this control
        m_subControls.push_back(subid);

        ++i;
    }

    // Create a dummy radio control to end the group.
    m_dummyId = NewControlId();

    m_dummyHwnd = (WXHWND)::CreateWindowW(L"BUTTON",
                         L"",
                         WS_GROUP | BS_AUTORADIOBUTTON | WS_CHILD,
                         0, 0, 0, 0, GetHwndOf(parent),
                         (WXHMENU)wxUIntToPtr(m_dummyId.GetValue()),
                         wxGetInstance(), nullptr);


    m_radioButtons->SetFont(GetFont());

    SetMajorDim(majorDim == 0 ? choices.size() : majorDim, style);
    // Select the first radio button if we have any buttons at all.
    if ( !choices.empty() )
        SetSelection(0);
    SetSize(wxRect{pos, size});

    // Now that we have items determine what is the best size and set it.
    SetInitialSize(size);

    // And update all the buttons positions to match it.
    const wxSize actualSize = GetSize();
    PositionAllButtons(wxRect{pos, actualSize});

    // The base wxStaticBox class never accepts focus, but we do because giving
    // focus to a wxRadioBox actually gives it to one of its buttons, which are
    // not visible at wx level and hence are not taken into account by the
    // logic in wxControlContainer code.
    m_container.EnableSelfFocus();

    return true;
}

wxRadioBox::~wxRadioBox()
{
    SendDestroyEvent();

    // Unsubclass all the radio buttons and remove their soon-to-be-invalid
    // HWNDs from the global map. Notice that we need to unsubclass because
    // otherwise we'd need the entries in gs_boxFromButton for the buttons
    // being deleted to handle the messages generated during their destruction.
    for ( size_t item = 0; item < m_radioButtons->GetCount(); item++ )
    {
        WXHWND hwnd = m_radioButtons->Get(item);

        wxSetWindowProc(hwnd, reinterpret_cast<WNDPROC>(s_wndprocRadioBtn));
        gs_boxFromButton.erase(hwnd);
    }

    delete m_radioButtons;

    if ( m_dummyHwnd )
        DestroyWindow((WXHWND)m_dummyHwnd);
}

// NB: if this code is changed, wxGetWindowForHWND() which relies on having the
//     radiobox pointer in GWL_USERDATA for radio buttons must be updated too!
void wxRadioBox::SubclassRadioButton(WXHWND hWndBtn)
{
    WXHWND hwndBtn = (WXHWND)hWndBtn;

    if ( !s_wndprocRadioBtn )
        s_wndprocRadioBtn = wxGetWindowProc(hwndBtn);

    wxSetWindowProc(hwndBtn, wxRadioBtnWndProc);

    gs_boxFromButton[hwndBtn] = this;
}

// ----------------------------------------------------------------------------
// events generation
// ----------------------------------------------------------------------------

bool wxRadioBox::MSWCommand(WXUINT cmd, WXWORD id_)
{
    const int id = (signed short)id_;

    if ( cmd == BN_CLICKED )
    {
        if (id == GetId())
            return true;

        int selectedButton = wxNOT_FOUND;

        for ( unsigned int i = 0; i < GetCount(); i++ )
        {
            const WXHWND hwndBtn = (*m_radioButtons)[i];
            if ( id == wxGetWindowId(hwndBtn) )
            {
                // we can get BN_CLICKED for a button which just became focused
                // but it may not be checked, in which case we shouldn't
                // generate a radiobox selection changed event for it
                if ( ::SendMessageW(hwndBtn, BM_GETCHECK, 0, 0) == BST_CHECKED )
                    selectedButton = i;

                break;
            }
        }

        if ( selectedButton == wxNOT_FOUND )
        {
            // just ignore it - due to a hack with WM_NCHITTEST handling in our
            // wnd proc, we can receive dummy click messages when we click near
            // the radiobox edge (this is ugly but Julian wouldn't let me get
            // rid of this...)
            return false;
        }

        if ( selectedButton != m_selectedButton )
        {
            m_selectedButton = selectedButton;

            SendNotificationEvent();
        }
        //else: don't generate events when the selection doesn't change

        return true;
    }
    else
        return false;
}

void wxRadioBox::Command(wxCommandEvent & event)
{
    SetSelection (event.GetInt());
    SetFocus();
    ProcessCommand(event);
}

void wxRadioBox::SendNotificationEvent()
{
    wxCommandEvent event(wxEVT_RADIOBOX, m_windowId);
    event.SetInt( m_selectedButton );
    event.SetString(GetString(m_selectedButton));
    event.SetEventObject( this );
    ProcessCommand(event);
}

// ----------------------------------------------------------------------------
// simple accessors
// ----------------------------------------------------------------------------

size_t wxRadioBox::GetCount() const
{
    return m_radioButtons ? m_radioButtons->GetCount() : 0;
}

void wxRadioBox::SetString(unsigned int item, const std::string& label)
{
    wxCHECK_RET( IsValid(item), "invalid radiobox index" );

    ::SetWindowTextW((*m_radioButtons)[item], boost::nowide::widen(label).c_str());

    InvalidateBestSize();
}

void wxRadioBox::SetSelection(int N)
{
    wxCHECK_RET( IsValid(N), "invalid radiobox index" );

    // unselect the old button
    if ( m_selectedButton != wxNOT_FOUND )
        ::SendMessageW((*m_radioButtons)[m_selectedButton], BM_SETCHECK, 0, 0L);

    // and select the new one
    ::SendMessageW((*m_radioButtons)[N], BM_SETCHECK, 1, 0L);

    m_selectedButton = N;
}

// Find string for position
std::string wxRadioBox::GetString(unsigned int item) const
{
    wxCHECK_MSG( IsValid(item), "",
                 "invalid radiobox index" );

    return wxGetWindowText((*m_radioButtons)[item]);
}

void wxRadioBox::SetFocus()
{
    if ( GetCount() > 0 )
    {
        ::SetFocus((*m_radioButtons)[m_selectedButton == wxNOT_FOUND
                                        ? 0
                                        : m_selectedButton]);
    }
}

bool wxRadioBox::CanBeFocused() const
{
    // If the control itself is hidden or disabled, no need to check anything
    // else.
    if ( !wxStaticBox::CanBeFocused() )
        return false;

    // Otherwise, check if we have any buttons that can be focused.
    for ( size_t item = 0; item < m_radioButtons->GetCount(); item++ )
    {
        if ( IsItemEnabled(item) && IsItemShown(item) )
            return true;
    }

    // We didn't find any items that can accept focus, so neither can we as a
    // whole accept it.
    return false;
}

// Enable a specific button
bool wxRadioBox::Enable(unsigned int item, bool enable)
{
    wxCHECK_MSG( IsValid(item), false,
                 "invalid item in wxRadioBox::Enable()" );

    BOOL ret = MSWEnableHWND((*m_radioButtons)[item], enable);

    return (ret == 0) != enable;
}

bool wxRadioBox::IsItemEnabled(unsigned int item) const
{
    wxCHECK_MSG( IsValid(item), false,
                 "invalid item in wxRadioBox::IsItemEnabled()" );

    return ::IsWindowEnabled((*m_radioButtons)[item]) != 0;
}

// Show a specific button
bool wxRadioBox::Show(unsigned int item, bool show)
{
    wxCHECK_MSG( IsValid(item), false,
                 "invalid item in wxRadioBox::Show()" );

    BOOL ret = ::ShowWindow((*m_radioButtons)[item], show ? SW_SHOW : SW_HIDE);

    bool changed = (ret != 0) != show;
    if ( changed )
    {
        InvalidateBestSize();
    }

    return changed;
}

bool wxRadioBox::IsItemShown(unsigned int item) const
{
    wxCHECK_MSG( IsValid(item), false,
                 "invalid item in wxRadioBox::IsItemShown()" );

    // don't use IsWindowVisible() here because it would return false if the
    // radiobox itself is hidden while we want to only return false if this
    // button specifically is hidden
    return (::GetWindowLongPtrW((*m_radioButtons)[item],
                            GWL_STYLE) & WS_VISIBLE) != 0;
}

#if wxUSE_TOOLTIPS

bool wxRadioBox::HasToolTips() const
{
    return wxStaticBox::HasToolTips() || wxRadioBoxBase::HasItemToolTips();
}

void wxRadioBox::DoSetItemToolTip(unsigned int item, wxToolTip *tooltip)
{
    // we have already checked for the item to be valid in wxRadioBoxBase
    const WXHWND hwndRbtn = (*m_radioButtons)[item];
    if ( tooltip != nullptr )
        tooltip->AddOtherWindow(hwndRbtn);
    else // unset the tooltip
        wxToolTip::Remove(hwndRbtn, 0, wxRect(0,0,0,0));
        // the second parameter can be zero since it's ignored by Remove()
        // as we pass a rect for which wxRect::IsEmpty()==true...
}

#endif // wxUSE_TOOLTIPS

bool wxRadioBox::Reparent(wxWindowBase *newParent)
{
    if ( !wxStaticBox::Reparent(newParent) )
    {
        return false;
    }

    WXHWND hwndParent = GetHwndOf(GetParent());
    for ( size_t item = 0; item < m_radioButtons->GetCount(); item++ )
    {
        ::SetParent((*m_radioButtons)[item], hwndParent);
    }
    return true;
}

WX_FORWARD_STD_METHODS_TO_SUBWINDOWS(wxRadioBox, wxStaticBox, m_radioButtons)

// ----------------------------------------------------------------------------
// size calculations
// ----------------------------------------------------------------------------

wxSize wxRadioBox::GetMaxButtonSize() const
{
    // We use GetCheckBox() because there is no dedicated GetRadioBox() method
    // in wxRendererNative, but check and radio boxes are usually of the same
    // size anyhow. We also add half a character of width to account for the
    // extra space after the radio box itself.
    const int radioWidth =
        wxRendererNative::Get().GetCheckBoxSize(
            static_cast<wxWindow*>(const_cast<wxRadioBox*>(this))).x
        + wxGetCharWidth() / 2;

    // calculate the max button size
    int widthMax{0};
    int heightMax{0};

    for ( unsigned int i = 0 ; i < GetCount(); i++ )
    {
        auto button_extents = GetTextExtent(wxGetWindowText((*m_radioButtons)[i]));

        // adjust the size to take into account the radio box itself
        button_extents.x += radioWidth;
        button_extents.y *= 3;
        button_extents.y  /= 2;

        if ( widthMax < button_extents.x  )
            widthMax = button_extents.x ;
        if ( heightMax < button_extents.y  )
            heightMax = button_extents.y ;
    }

    return {widthMax, heightMax};
}

wxSize wxRadioBox::GetTotalButtonSize(const wxSize& sizeBtn) const
{
    // the radiobox should be big enough for its buttons
    wxSize ch_size = wxGetCharSize(m_hWnd, GetFont());

    int height = GetRowCount() * sizeBtn.y + ch_size.y / 2 + ch_size.y;
    int width  = GetColumnCount() * (sizeBtn.x + ch_size.x) + ch_size.x;

    // Add extra space under the label, if it exists.
    if (!wxControl::GetLabel().empty())
        height += wxRendererNative::Get().GetCheckBoxSize(
            static_cast<wxWindow*>(const_cast<wxRadioBox*>(this))).y / 2;

    // and also wide enough for its label
    int widthBox = wxStaticBox::DoGetBestSize().x;
    if ( widthBox > width )
        width = widthBox;

    return {width, height};
}

wxSize wxRadioBox::DoGetBestSize() const
{
    if ( !m_radioButtons )
    {
        // if we're not fully initialized yet, we can't meaningfully compute
        // our best size, we'll do it later
        return {1, 1};
    }

    return GetTotalButtonSize(GetMaxButtonSize());
}

void wxRadioBox::DoSetSize(wxRect boundary, unsigned int sizeFlags)
{
    if ( (boundary.width == wxDefaultCoord && (sizeFlags & wxSIZE_AUTO_WIDTH)) ||
            (boundary.height == wxDefaultCoord && (sizeFlags & wxSIZE_AUTO_HEIGHT)) )
    {
        // Attempt to have a look coherent with other platforms: We compute the
        // biggest toggle dim, then we align all items according this value.
        const wxSize totSize = GetTotalButtonSize(GetMaxButtonSize());

        // only change our width/height if asked for
        if ( boundary.width == wxDefaultCoord && (sizeFlags & wxSIZE_AUTO_WIDTH) )
            boundary.width = totSize.x;

        if ( boundary.height == wxDefaultCoord && (sizeFlags & wxSIZE_AUTO_HEIGHT) )
            boundary.height = totSize.y;
    }

    wxStaticBox::DoSetSize(boundary);
}

void wxRadioBox::DoMoveWindow(wxRect boundary)
{
    wxStaticBox::DoMoveWindow(boundary);

    PositionAllButtons(boundary);
}

void
wxRadioBox::PositionAllButtons(wxRect boundary)
{
    wxSize maxSize = GetMaxButtonSize();
    int maxWidth = maxSize.x,
        maxHeight = maxSize.y;

    // Now position all the buttons: the current button will be put at
    // wxPoint(x_offset, y_offset) and the new row/column will start at
    // startX/startY. The size of all buttons will be the same wxSize(maxWidth,
    // maxHeight) except for the buttons in the last column which should extend
    // to the right border of radiobox and thus can be wider than this.

    // Also, remember that wxRA_SPECIFY_COLS means that we arrange buttons in
    // left to right order and GetMajorDim() is the number of columns while
    // wxRA_SPECIFY_ROWS means that the buttons are arranged top to bottom and
    // GetMajorDim() is the number of rows.

    wxSize ch_size = wxGetCharSize(m_hWnd, GetFont());

    int x_offset = boundary.x + ch_size.x;
    int y_offset = boundary.y + ch_size.y;

    // Add extra space under the label, if it exists.
    if (!wxControl::GetLabel().empty())
        y_offset += wxRendererNative::Get().GetCheckBoxSize(
            static_cast<wxWindow*>(this)).y / 2;

    int startX = x_offset;
    int startY = y_offset;

    const unsigned int count = GetCount();
    for (unsigned int i = 0; i < count; i++)
    {
        // the last button in the row may be wider than the other ones as the
        // radiobox may be wider than the sum of the button widths (as it
        // happens, for example, when the radiobox label is very long)
        bool isLastInTheRow{};
        if ( m_windowStyle & wxRA_SPECIFY_COLS )
        {
            // item is the last in its row if it is a multiple of the number of
            // columns or if it is just the last item
            unsigned int n = i + 1;
            isLastInTheRow = ((n % GetMajorDim()) == 0) || (n == count);
        }
        else // wxRA_SPECIFY_ROWS
        {
            // item is the last in the row if it is in the last columns
            isLastInTheRow = i >= (count/GetMajorDim())*GetMajorDim();
        }

        // is this the start of new row/column?
        if ( i && (i % GetMajorDim() == 0) )
        {
            if ( m_windowStyle & wxRA_SPECIFY_ROWS )
            {
                // start of new column
                y_offset = startY;
                x_offset += maxWidth + ch_size.x;
            }
            else // start of new row
            {
                x_offset = startX;
                y_offset += maxHeight;
            }
        }

        int widthBtn{};
        if ( isLastInTheRow )
        {
            // make the button go to the end of radio box
            widthBtn = startX + boundary.width - x_offset - 2*ch_size.x;
            if ( widthBtn < maxWidth )
                widthBtn = maxWidth;
        }
        else
        {
            // normal button, always of the same size
            widthBtn = maxWidth;
        }

        // make all buttons of the same, maximal size - like this they cover
        // the radiobox entirely and the radiobox tooltips are always shown
        // (otherwise they are not when the mouse pointer is in the radiobox
        // part not belonging to any radiobutton)
        DoMoveSibling((*m_radioButtons)[i], wxRect{x_offset, y_offset, widthBtn, maxHeight});

        // where do we put the next button?
        if ( m_windowStyle & wxRA_SPECIFY_ROWS )
        {
            // below this one
            y_offset += maxHeight;
        }
        else
        {
            // to the right of this one
            x_offset += widthBtn + ch_size.x;
        }
    }
}

int wxRadioBox::GetItemFromPoint(const wxPoint& pt) const
{
    const std::size_t count = GetCount();
    for ( std::size_t i{0}; i < count; i++ )
    {
        RECT rect = wxGetWindowRect((*m_radioButtons)[i]);

        if ( rect.left <= pt.x && pt.x < rect.right &&
                rect.top  <= pt.y && pt.y < rect.bottom )
        {
            return i;
        }
    }

    return wxNOT_FOUND;
}

void wxRadioBox::MSWUpdateFontOnDPIChange(const wxSize& newDPI)
{
    wxStaticBox::MSWUpdateFontOnDPIChange(newDPI);

    if ( m_font.IsOk() )
        m_radioButtons->SetFont(m_font);
}

// ----------------------------------------------------------------------------
// radio box drawing
// ----------------------------------------------------------------------------

WXHRGN wxRadioBox::MSWGetRegionWithoutChildren()
{
    RECT rc;
    ::GetWindowRect(GetHwnd(), &rc);
    WXHRGN hrgn = ::CreateRectRgn(rc.left, rc.top, rc.right + 1, rc.bottom + 1);

    using msw::utils::unique_region;

    for ( unsigned int i = 0; i < GetCount(); ++i )
    {
        // don't clip out hidden children
        if ( !IsItemShown(i) )
            continue;

        ::GetWindowRect((*m_radioButtons)[i], &rc);
        auto hrgnchild = unique_region(::CreateRectRgnIndirect(&rc));
        ::CombineRgn(hrgn, hrgn, hrgnchild.get(), RGN_DIFF);
    }

    return (WXHRGN)hrgn;
}

// ---------------------------------------------------------------------------
// window proc for radio buttons
// ---------------------------------------------------------------------------

LRESULT APIENTRY
wxRadioBtnWndProc(WXHWND hwnd, WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{

    wxRadioBox * const radiobox = wxRadioBox::GetFromRadioButtonHWND(hwnd);
    wxCHECK_MSG( radiobox, 0, "Should have the associated radio box" );

    switch ( message )
    {
        case WM_GETDLGCODE:
            // we must tell IsDialogMessage()/our kbd processing code that we
            // want to process arrows ourselves because neither of them is
            // smart enough to handle arrows properly for us
            {
                const auto lDlgCode = ::CallWindowProcW(CASTWNDPROC s_wndprocRadioBtn, hwnd,
                                                 message, wParam, lParam);

                return lDlgCode | DLGC_WANTARROWS;
            }

        case WM_KEYDOWN:
            {
                bool processed = true;

                wxDirection dir;
                switch ( wParam )
                {
                    case VK_UP:
                        dir = wxUP;
                        break;

                    case VK_LEFT:
                        dir = wxLEFT;
                        break;

                    case VK_DOWN:
                        dir = wxDOWN;
                        break;

                    case VK_RIGHT:
                        dir = wxRIGHT;
                        break;

                    default:
                        processed = false;

                        // just to suppress the compiler warning
                        dir = wxALL;
                }

                if ( processed )
                {
                    int selOld = radiobox->GetSelection();
                    int selNew = radiobox->GetNextItem
                                 (
                                  selOld,
                                  dir,
                                  radiobox->wxGetWindowStyle()
                                 );

                    if ( selNew != selOld )
                    {
                        radiobox->SetSelection(selNew);
                        radiobox->SetFocus();

                        // emulate the button click
                        radiobox->SendNotificationEvent();

                        return 0;
                    }
                }
            }
            break;

        case WM_SETFOCUS:
        case WM_KILLFOCUS:
            {
                // if we don't do this, no focus events are generated for the
                // radiobox and, besides, we need to notify the parent about
                // the focus change, otherwise the focus handling logic in
                // wxControlContainer doesn't work
                if ( message == WM_SETFOCUS )
                    radiobox->HandleSetFocus((WXHWND)wParam);
                else
                    radiobox->HandleKillFocus((WXHWND)wParam);
            }
            break;

        case WM_HELP:
            {
                bool processed = false;

                wxEvtHandler * const handler = radiobox->GetEventHandler();

                HELPINFO* info = (HELPINFO*) lParam;
                if ( info->iContextType == HELPINFO_WINDOW )
                {
                    for ( wxWindow* subjectOfHelp = radiobox;
                          subjectOfHelp;
                          subjectOfHelp = subjectOfHelp->GetParent() )
                    {
                        wxHelpEvent helpEvent(wxEVT_HELP,
                                              subjectOfHelp->GetId(),
                                              wxPoint(info->MousePos.x,
                                                      info->MousePos.y));
                        helpEvent.SetEventObject(radiobox);
                        if ( handler->ProcessEvent(helpEvent) )
                        {
                            processed = true;
                            break;
                        }
                    }
                }
                else if (info->iContextType == HELPINFO_MENUITEM)
                {
                    wxHelpEvent helpEvent(wxEVT_HELP, info->iCtrlId);
                    helpEvent.SetEventObject(radiobox);
                    processed = handler->ProcessEvent(helpEvent);
                }

                if ( processed )
                    return 0;
            }
            break;
    }

    return ::CallWindowProcW(CASTWNDPROC s_wndprocRadioBtn, hwnd, message, wParam, lParam);
}

#endif // wxUSE_RADIOBOX
