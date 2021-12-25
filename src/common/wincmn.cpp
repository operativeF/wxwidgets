/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/wincmn.cpp
// Purpose:     common (to all ports) wxWindow functions
// Author:      Julian Smart, Vadim Zeitlin
// Modified by:
// Created:     13/07/98
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/app.h"
#include "wx/string.h"
#include "wx/log.h"
#include "wx/intl.h"
#include "wx/window.h"
#include "wx/control.h"
#include "wx/checkbox.h"
#include "wx/radiobut.h"
#include "wx/statbox.h"
#include "wx/textctrl.h"
#include "wx/msgdlg.h"
#include "wx/msgout.h"
#include "wx/statusbr.h"
#include "wx/toolbar.h"
#include "wx/dcclient.h"
#include "wx/dcscreen.h"
#include "wx/scrolbar.h"
#include "wx/menu.h"
#include "wx/button.h"
#include "wx/utils.h"

#if wxUSE_DRAG_AND_DROP
    #include "wx/dnd.h"
#endif // wxUSE_DRAG_AND_DROP

#if wxUSE_ACCESSIBILITY
    #include "wx/access.h"
#endif

#if wxUSE_HELP
    #include "wx/cshelp.h"
#endif // wxUSE_HELP

#if wxUSE_TOOLTIPS
    #include "wx/tooltip.h"
#endif // wxUSE_TOOLTIPS

#if wxUSE_CARET
    #include "wx/caret.h"
#endif // wxUSE_CARET

#if wxUSE_SYSTEM_OPTIONS
import WX.Cmn.SysOpt;
#endif

#include "wx/display.h"
#include "wx/recguard.h"
#include "wx/private/window.h"

import WX.Utils.Settings;
import WX.Cmn.PlatInfo;
import WX.Core.Sizer;

import WX.Cfg.Flags;

namespace wxMouseCapture
{

// Check if the given window is in the capture cap_stack.
bool IsInCaptureStack(wxWindowBase* win);

} // wxMouseCapture

wxIMPLEMENT_ABSTRACT_CLASS(wxWindowBase, wxEvtHandler);

wxBEGIN_EVENT_TABLE(wxWindowBase, wxEvtHandler)
    EVT_SYS_COLOUR_CHANGED(wxWindowBase::OnSysColourChanged)
    EVT_INIT_DIALOG(wxWindowBase::OnInitDialog)
    EVT_MIDDLE_DOWN(wxWindowBase::OnMiddleClick)

#if wxUSE_HELP
    EVT_HELP(wxID_ANY, wxWindowBase::OnHelp)
#endif // wxUSE_HELP

    EVT_SIZE(wxWindowBase::InternalOnSize)
wxEND_EVENT_TABLE()

#ifndef __WXUNIVERSAL__
wxIMPLEMENT_DYNAMIC_CLASS(wxWindow, wxWindowBase);
#endif

// the default initialization
wxWindowBase::wxWindowBase()
    : m_virtualSize(wxDefaultSize)
    , m_bestSizeCache(wxDefaultSize)
{
    // no window yet, no parent nor children
    m_parent = nullptr;
    m_windowId = wxID_ANY;

    // no constraints on the minimal window size
    m_minWidth =
    m_maxWidth = wxDefaultCoord;
    m_minHeight =
    m_maxHeight = wxDefaultCoord;

    // window are created enabled and visible by default
    m_isShown =
    m_isEnabled = true;

    // the default event handler is just this window
    m_eventHandler = this;

#if wxUSE_VALIDATORS
    // no validator
    m_windowValidator = nullptr;
#endif // wxUSE_VALIDATORS

    // the colours/fonts are default for now, so leave m_font,
    // m_backgroundColour and m_foregroundColour uninitialized and set those
    m_hasBgCol =
    m_hasFgCol =
    m_hasFont = false;
    m_inheritBgCol =
    m_inheritFgCol =
    m_inheritFont = false;

    // no style bits
    m_exStyle =
    m_windowStyle = 0;

    m_backgroundStyle = wxBackgroundStyle::Erase;

    m_windowSizer = nullptr;
    m_containingSizer = nullptr;
    m_autoLayout = false;

    m_disableFocusFromKbd = false;

#if wxUSE_DRAG_AND_DROP
    m_dropTarget = nullptr;
#endif // wxUSE_DRAG_AND_DROP

#if wxUSE_TOOLTIPS
    m_tooltip = nullptr;
#endif // wxUSE_TOOLTIPS

#if wxUSE_CARET
    m_caret = nullptr;
#endif // wxUSE_CARET

#if wxUSE_PALETTE
    m_hasCustomPalette = false;
#endif // wxUSE_PALETTE

#if wxUSE_ACCESSIBILITY
    m_accessible = nullptr;
#endif

    m_scrollHelper = nullptr;

    m_windowVariant = wxWindowVariant::Normal;
#if wxUSE_SYSTEM_OPTIONS
    if ( wxSystemOptions::HasOption(wxWINDOW_DEFAULT_VARIANT) )
    {
       m_windowVariant = (wxWindowVariant) wxSystemOptions::GetOptionInt( wxWINDOW_DEFAULT_VARIANT ) ;
    }
#endif

    // Whether we're using the current theme for this window (wxGTK only for now)
    m_themeEnabled = false;

    // This is set to true by SendDestroyEvent() which should be called by the
    // most derived class to ensure that the destruction event is sent as soon
    // as possible to allow its handlers to still see the undestroyed window
    m_isBeingDeleted = false;

    m_freezeCount = 0;
}

// common part of window creation process
bool wxWindowBase::CreateBase(wxWindowBase *parent,
                              wxWindowID id,
                              [[maybe_unused]] const wxPoint& pos,
                              const wxSize& size,
                              unsigned int style,
                              std::string_view name)
{
    // ids are limited to 16 bits under MSW so if you care about portability,
    // it's not a good idea to use ids out of this range (and negative ids are
    // reserved for wxWidgets own usage)
    wxASSERT_MSG( id == wxID_ANY || (id >= 0 && id < 32767) ||
                  (id >= wxID_AUTO_LOWEST && id <= wxID_AUTO_HIGHEST),
                  "invalid id value" );

    // generate a new id if the user doesn't care about it
    if ( id == wxID_ANY )
    {
        m_windowId = NewControlId();
    }
    else // valid id specified
    {
        m_windowId = id;
    }

    // don't use SetWindowStyleFlag() here, this function should only be called
    // to change the flag after creation as it tries to reflect the changes in
    // flags by updating the window dynamically and we don't need this here
    m_windowStyle = style;

    // assume the user doesn't want this window to shrink beneath its initial
    // size, this worked like this in wxWidgets 2.8 and before and generally
    // often makes sense for child windows (for top level ones it definitely
    // does not as the user should be able to resize the window)
    //
    // note that we can't use IsTopLevel() from ctor
    if ( size != wxDefaultSize && !wxTopLevelWindows.Find((wxWindow *)this) )
        SetMinSize(size);

    SetName(name);
    SetParent(parent);

    return true;
}

bool wxWindowBase::CreateBase(wxWindowBase *parent,
                              wxWindowID id,
                              const wxPoint& pos,
                              const wxSize& size,
                              unsigned int style,
                              const wxValidator& wxVALIDATOR_PARAM(validator),
                              std::string_view name)
{
    if ( !CreateBase(parent, id, pos, size, style, name) )
        return false;

#if wxUSE_VALIDATORS
    SetValidator(validator);
#endif // wxUSE_VALIDATORS

    return true;
}

bool wxWindowBase::ToggleWindowStyle(unsigned int flag)
{
    wxASSERT_MSG( flag, "flags with 0 value can't be toggled" );

    bool rc;
    unsigned int style = GetWindowStyleFlag();

    if ( style & flag )
    {
        style &= ~flag;
        rc = false;
    }
    else // currently off
    {
        style |= flag;
        rc = true;
    }

    SetWindowStyleFlag(style);

    return rc;
}

// ----------------------------------------------------------------------------
// destruction
// ----------------------------------------------------------------------------

// common clean up
wxWindowBase::~wxWindowBase()
{
    wxASSERT_MSG( !wxMouseCapture::IsInCaptureStack(this),
                    "Destroying window before releasing mouse capture: this "
                    "will result in a crash later." );

    // FIXME if these 2 cases result from programming errors in the user code
    //       we should probably assert here instead of silently fixing them

    // Just in case the window has been Closed, but we're then deleting
    // immediately: don't leave dangling pointers.
    wxPendingDelete.DeleteObject(this);

    // Just in case we've loaded a top-level window via LoadNativeDialog but
    // we weren't a dialog class
    wxTopLevelWindows.DeleteObject((wxWindow*)this);

    // Any additional event handlers should be popped before the window is
    // deleted as otherwise the last handler will be left with a dangling
    // pointer to this window result in a difficult to diagnose crash later on.
    wxASSERT_MSG( GetEventHandler() == this,
                    "any pushed event handlers must have been removed" );

#if wxUSE_MENUS
    // The associated popup menu can still be alive, disassociate from it in
    // this case
    if ( wxCurrentPopupMenu && wxCurrentPopupMenu->GetInvokingWindow() == this )
        wxCurrentPopupMenu->SetInvokingWindow(nullptr);
#endif // wxUSE_MENUS

    wxASSERT_MSG( GetChildren().GetCount() == 0, "children not destroyed" );

    // notify the parent about this window destruction
    if ( m_parent )
        m_parent->RemoveChild(this);

#if wxUSE_VALIDATORS
    delete m_windowValidator;
#endif // wxUSE_VALIDATORS

    if ( m_containingSizer )
        m_containingSizer->Detach( (wxWindow*)this );

    delete m_windowSizer;

#if wxUSE_TOOLTIPS
    delete m_tooltip;
#endif // wxUSE_TOOLTIPS

#if wxUSE_ACCESSIBILITY
    delete m_accessible;
#endif

#if wxUSE_HELP
    // NB: this has to be called unconditionally, because we don't know
    //     whether this window has associated help text or not
    wxHelpProvider *helpProvider = wxHelpProvider::Get();
    if ( helpProvider )
        helpProvider->RemoveHelp(this);
#endif
}

bool wxWindowBase::IsBeingDeleted() const
{
    return m_isBeingDeleted ||
            (!IsTopLevel() && m_parent && m_parent->IsBeingDeleted());
}

void wxWindowBase::SendDestroyEvent()
{
    if ( m_isBeingDeleted )
    {
        // we could have been already called from a more derived class dtor,
        // e.g. ~wxTLW calls us and so does ~wxWindow and the latter call
        // should be simply ignored
        return;
    }

    m_isBeingDeleted = true;

    wxWindowDestroyEvent event;
    event.SetEventObject(this);
    event.SetId(GetId());
    GetEventHandler()->ProcessEvent(event);
}

bool wxWindowBase::Destroy()
{
    // If our handle is invalid, it means that this window has never been
    // created, either because creating it failed or, more typically, because
    // this wxWindow object was default-constructed and its Create() method had
    // never been called. As we didn't send wxWindowCreateEvent in this case
    // (which is sent after successful creation), don't send the matching
    // wxWindowDestroyEvent neither.
    if ( GetHandle() )
        SendDestroyEvent();

    delete this;

    return true;
}

bool wxWindowBase::Close(bool force)
{
    wxCloseEvent event(wxEVT_CLOSE_WINDOW, m_windowId);
    event.SetEventObject(this);
    event.SetCanVeto(!force);

    // return false if window wasn't closed because the application vetoed the
    // close event
    return HandleWindowEvent(event) && !event.GetVeto();
}

bool wxWindowBase::DestroyChildren()
{
    wxWindowList::compatibility_iterator node;
    for ( ;; )
    {
        // we iterate until the list becomes empty
        node = GetChildren().GetFirst();
        if ( !node )
            break;

        wxWindow *child = node->GetData();

        // note that we really want to delete it immediately so don't call the
        // possible overridden Destroy() version which might not delete the
        // child immediately resulting in problems with our (top level) child
        // outliving its parent
        child->wxWindowBase::Destroy();

        wxASSERT_MSG( !GetChildren().Find(child),
                      "child didn't remove itself using RemoveChild()" );
    }

    return true;
}

// ----------------------------------------------------------------------------
// size/position related methods
// ----------------------------------------------------------------------------

// centre the window with respect to its parent in either (or both) directions
void wxWindowBase::DoCentre(unsigned int dir)
{
    wxCHECK_RET( !(dir & wxCENTRE_ON_SCREEN) && GetParent(),
                 "this method only implements centering child windows" );

    SetSize(GetRect().CentreIn(GetParent()->GetClientSize(), dir));
}

// fits the window around the children
void wxWindowBase::Fit()
{
    SetSize(GetBestSize());
}

// fits virtual size (ie. scrolled area etc.) around children
void wxWindowBase::FitInside()
{
    SetVirtualSize( GetBestVirtualSize() );
}

// On Mac, scrollbars are explicitly children.
#if defined( __WXMAC__ ) && !defined(__WXUNIVERSAL__)
static bool wxHasRealChildren(const wxWindowBase* win)
{
    int realChildCount = 0;

    for ( wxWindowList::compatibility_iterator node = win->GetChildren().GetFirst();
          node;
          node = node->GetNext() )
    {
        wxWindow *win = node->GetData();
        if ( !win->IsTopLevel() && win->IsShown()
#if wxUSE_SCROLLBAR
            && !dynamic_cast<wxScrollBar*>(win)
#endif
            )
            realChildCount ++;
    }
    return (realChildCount > 0);
}
#endif

void wxWindowBase::InvalidateBestSize()
{
    m_bestSizeCache = wxDefaultSize;

    // parent's best size calculation may depend on its children's
    // as long as child window we are in is not top level window itself
    // (because the TLW size is never resized automatically)
    // so let's invalidate it as well to be safe:
    if (m_parent && !IsTopLevel())
        m_parent->InvalidateBestSize();
}

// return the size best suited for the current window
wxSize wxWindowBase::DoGetBestSize() const
{
    wxSize best;

    if ( m_windowSizer )
    {
        best = m_windowSizer->GetMinSize();
    }
    else if ( !GetChildren().empty()
#if defined( __WXMAC__ ) && !defined(__WXUNIVERSAL__)
              && wxHasRealChildren(this)
#endif
              )
    {
        // our minimal acceptable size is such that all our visible child
        // windows fit inside
        int maxX = 0,
            maxY = 0;

        for ( wxWindowList::compatibility_iterator node = GetChildren().GetFirst();
              node;
              node = node->GetNext() )
        {
            wxWindow *win = node->GetData();
            if ( win->IsTopLevel()
                    || !win->IsShown()
#if wxUSE_STATUSBAR
                        || dynamic_cast<wxStatusBar*>(win)
#endif // wxUSE_STATUSBAR
               )
            {
                // dialogs and frames lie in different top level windows -
                // don't deal with them here; as for the status bars, they
                // don't lie in the client area at all
                continue;
            }

            wxPoint wxpt = win->GetPosition();

            // if the window hadn't been positioned yet, assume that it is in
            // the origin
            if ( wxpt.x == wxDefaultCoord )
                wxpt.x = 0;
            if ( wxpt.y == wxDefaultCoord )
                wxpt.y = 0;

            wxSize wxsz = win->GetSize();
            if ( wxpt.x + wxsz.x > maxX )
                maxX = wxsz.x + wxsz.x;
            if ( wxpt.y + wxsz.y > maxY )
                maxY = wxsz.y + wxsz.y;
        }

        best = wxSize(maxX, maxY);
    }
    else // ! has children
    {
        wxSize size = GetMinSize();

        if ( !size.IsFullySpecified() )
        {
            // if the window doesn't define its best size we assume that it can
            // be arbitrarily small -- usually this is not the case, of course,
            // but we have no way to know what the limit is, it should really
            // override DoGetBestClientSize() itself to tell us
            size = {1, 1};
        }

        // return as-is, unadjusted by the client size difference.
        return size;
    }

    // Add any difference between size and client size
    wxSize diff = GetSize() - GetClientSize();
    best.x += std::max(0, diff.x);
    best.y += std::max(0, diff.y);

    return best;
}

double wxWindowBase::GetContentScaleFactor() const
{
    // By default, we assume that there is no mapping between logical and
    // physical pixels and so the content scale factor is just 1. Only the
    // platforms that do perform such mapping (currently ports for Apple
    // platforms and GTK 3) override this function to return something
    // different.
    return 1.0;
}

double wxWindowBase::GetDPIScaleFactor() const
{
    return wxDisplay(dynamic_cast<const wxWindow*>(this)).GetScaleFactor();
}

// helper of GetWindowBorderSize(): as many ports don't implement support for
// wxSYS_BORDER/EDGE_X/Y metrics in their wxSystemSettings, use hard coded
// fallbacks in this case
static int wxGetMetricOrDefault(wxSystemMetric what, const wxWindowBase* win)
{
    int rc = wxSystemSettings::GetMetric(
        what, dynamic_cast<const wxWindow*>(win));
    if ( rc == -1 )
    {
        switch ( what )
        {
            case wxSYS_BORDER_X:
            case wxSYS_BORDER_Y:
                // 2D border is by default 1 pixel wide
                rc = 1;
                break;

            case wxSYS_EDGE_X:
            case wxSYS_EDGE_Y:
                // 3D borders are by default 2 pixels
                rc = 2;
                break;

            default:
                wxFAIL_MSG( "unexpected wxGetMetricOrDefault() argument" );
                rc = 0;
        }
    }

    return rc;
}

wxSize wxWindowBase::GetWindowBorderSize() const
{
    wxSize size;

    switch ( GetBorder() )
    {
        case wxBORDER_NONE:
            // nothing to do, size is already (0, 0)
            break;

        case wxBORDER_SIMPLE:
        case wxBORDER_STATIC:
            size.x = wxGetMetricOrDefault(wxSYS_BORDER_X, this);
            size.y = wxGetMetricOrDefault(wxSYS_BORDER_Y, this);
            break;

        case wxBORDER_SUNKEN:
        case wxBORDER_RAISED:
            size.x = std::max(wxGetMetricOrDefault(wxSYS_EDGE_X, this),
                           wxGetMetricOrDefault(wxSYS_BORDER_X, this));
            size.y = std::max(wxGetMetricOrDefault(wxSYS_EDGE_Y, this),
                           wxGetMetricOrDefault(wxSYS_BORDER_Y, this));
            break;

        case wxBORDER_DOUBLE:
            size.x = wxGetMetricOrDefault(wxSYS_EDGE_X, this) +
                        wxGetMetricOrDefault(wxSYS_BORDER_X, this);
            size.y = wxGetMetricOrDefault(wxSYS_EDGE_Y, this) +
                        wxGetMetricOrDefault(wxSYS_BORDER_Y, this);
            break;

        default:
            wxFAIL_MSG("Unknown border style.");
            break;
    }

    // we have borders on both sides
    return size*2;
}

bool
wxWindowBase::InformFirstDirection(int direction,
                                   int size,
                                   int availableOtherDir)
{
    return GetSizer() && GetSizer()->InformFirstDirection(direction,
                                                          size,
                                                          availableOtherDir);
}

wxSize wxWindowBase::GetEffectiveMinSize() const
{
    // merge the best size with the min size, giving priority to the min size
    wxSize min = GetMinSize();

    if (min.x == wxDefaultCoord || min.y == wxDefaultCoord)
    {
        wxSize best = GetBestSize();
        if (min.x == wxDefaultCoord) min.x =  best.x;
        if (min.y == wxDefaultCoord) min.y =  best.y;
    }

    return min;
}

wxSize wxWindowBase::DoGetBorderSize() const
{
    // there is one case in which we can implement it for all ports easily
    if ( GetBorder() == wxBORDER_NONE )
        return {0, 0};

    // otherwise use the difference between the real size and the client size
    // as a fallback: notice that this is incorrect in general as client size
    // also doesn't take the scrollbars into account
    return GetSize() - GetClientSize();
}

wxSize wxWindowBase::GetBestSize() const
{
    if ( !m_windowSizer && m_bestSizeCache.IsFullySpecified() )
        return m_bestSizeCache;

    // call DoGetBestClientSize() first, if a derived class overrides it wants
    // it to be used
    wxSize size = DoGetBestClientSize();
    if ( size != wxDefaultSize )
        size += DoGetBorderSize();
    else
        size = DoGetBestSize();

    // Ensure that the best size is at least as large as min size.
    size.IncTo(GetMinSize());

    // And not larger than max size.
    size.DecToIfSpecified(GetMaxSize());

    // Finally cache result and return.
    CacheBestSize(size);
    return size;
}

int wxWindowBase::GetBestHeight(int width) const
{
    const int height = DoGetBestClientHeight(width);

    return height == wxDefaultCoord
            ? GetBestSize().y
            : height + DoGetBorderSize().y;
}

int wxWindowBase::GetBestWidth(int height) const
{
    const int width = DoGetBestClientWidth(height);

    return width == wxDefaultCoord
            ? GetBestSize().x
            : width + DoGetBorderSize().x;
}

void wxWindowBase::SetMinSize(const wxSize& minSize)
{
    m_minWidth = minSize.x;
    m_minHeight = minSize.y;

    InvalidateBestSize();
}

void wxWindowBase::SetMaxSize(const wxSize& maxSize)
{
    m_maxWidth = maxSize.x;
    m_maxHeight = maxSize.y;

    InvalidateBestSize();
}

void wxWindowBase::SetInitialSize(const wxSize& size)
{
    // Set the min size to the size passed in.  This will usually either be
    // wxDefaultSize or the size passed to this window's ctor/Create function.
    SetMinSize(size);

    // Merge the size with the best size if needed
    wxSize best = GetEffectiveMinSize();

    // If the current size doesn't match then change it
    if (GetSize() != best)
        SetSize(best);
}


// by default the origin is not shifted
wxPoint wxWindowBase::GetClientAreaOrigin() const
{
    return {0, 0};
}

wxSize wxWindowBase::ClientToWindowSize(const wxSize& size) const
{
    const wxSize diff(GetSize() - GetClientSize());

    return {size.x == -1 ? -1 : size.x + diff.x,
            size.y == -1 ? -1 : size.y + diff.y};
}

wxSize wxWindowBase::WindowToClientSize(const wxSize& size) const
{
    const wxSize diff(GetSize() - GetClientSize());

    return {size.x == -1 ? -1 : size.x - diff.x,
            size.y == -1 ? -1 : size.y - diff.y};
}

void wxWindowBase::WXSetInitialFittingClientSize(unsigned int flags)
{
    wxSizer* const sizer = GetSizer();
    if ( !sizer )
        return;

    const wxSize
        size = sizer->ComputeFittingClientSize(dynamic_cast<wxWindow *>(this));

    // It is important to set the min client size before changing the size
    // itself as the existing size hints could prevent SetClientSize() from
    // working otherwise.
    if ( flags & wxSIZE_SET_MIN )
        SetMinClientSize(size);

    if ( flags & wxSIZE_SET_CURRENT )
        SetClientSize(size);
}

void wxWindowBase::SetWindowVariant( wxWindowVariant variant )
{
    if ( m_windowVariant != variant )
    {
        m_windowVariant = variant;

        DoSetWindowVariant(variant);
    }
}

void wxWindowBase::DoSetWindowVariant( wxWindowVariant variant )
{
    // adjust the font height to correspond to our new variant (notice that
    // we're only called if something really changed)
    wxFont font = GetFont();
    double size = font.GetFractionalPointSize();
    switch ( variant )
    {
        case wxWindowVariant::Normal:
            break;

        case wxWindowVariant::Small:
            size /= 1.2;
            break;

        case wxWindowVariant::Mini:
            size /= 1.2 * 1.2;
            break;

        case wxWindowVariant::Large:
            size *= 1.2;
            break;

        default:
            wxFAIL_MSG("unexpected window variant");
            break;
    }

    font.SetFractionalPointSize(size);
    SetFont(font);
}

void wxWindowBase::DoSetSizeHints( int minW, int minH,
                                   int maxW, int maxH,
                                   [[maybe_unused]] int incW, [[maybe_unused]] int incH )
{
    wxCHECK_RET( (minW == wxDefaultCoord || maxW == wxDefaultCoord || minW <= maxW) &&
                    (minH == wxDefaultCoord || maxH == wxDefaultCoord || minH <= maxH),
                 "min width/height must be less than max width/height!" );

    m_minWidth = minW;
    m_maxWidth = maxW;
    m_minHeight = minH;
    m_maxHeight = maxH;
}

void wxWindowBase::DoSetVirtualSize( wxSize sz )
{
    m_virtualSize = sz;
}

wxSize wxWindowBase::DoGetVirtualSize() const
{
    // we should use the entire client area so if it is greater than our
    // virtual size, expand it to fit (otherwise if the window is big enough we
    // wouldn't be using parts of it)
    wxSize size = GetClientSize();
    if ( m_virtualSize.x > size.x )
        size.x = m_virtualSize.x;

    if ( m_virtualSize.y >= size.y )
        size.y = m_virtualSize.y;

    return size;
}

wxPoint wxWindowBase::DoGetScreenPosition() const
{
    // screen position is the same as (0, 0) in client coords for non TLWs (and
    // TLWs override this method)
    wxPoint pt;

    return ClientToScreen(pt);
}

void wxWindowBase::SendSizeEvent(unsigned int flags)
{
    wxSizeEvent event(GetSize(), GetId());
    event.SetEventObject(this);
    if ( flags & wxSEND_EVENT_POST )
        wxPostEvent(GetEventHandler(), event);
    else
        HandleWindowEvent(event);
}

void wxWindowBase::SendSizeEventToParent(unsigned int flags)
{
    wxWindow * const parent = GetParent();
    if ( parent && !parent->IsBeingDeleted() )
        parent->SendSizeEvent(flags);
}

bool wxWindowBase::CanScroll(int orient) const
{
    return (m_windowStyle &
            (orient == wxHORIZONTAL ? wxHSCROLL : wxVSCROLL)) != 0;
}

bool wxWindowBase::HasScrollbar(int orient) const
{
    // if scrolling in the given direction is disabled, we can't have the
    // corresponding scrollbar no matter what
    if ( !CanScroll(orient) )
        return false;

    const wxSize sizeVirt = GetVirtualSize();
    const wxSize sizeClient = GetClientSize();

    return orient == wxHORIZONTAL ? sizeVirt.x > sizeClient.x
                                  : sizeVirt.y > sizeClient.y;
}

// ----------------------------------------------------------------------------
// show/hide/enable/disable the window
// ----------------------------------------------------------------------------

bool wxWindowBase::Show(bool show)
{
    if ( show != m_isShown )
    {
        m_isShown = show;

        return true;
    }
    else
    {
        return false;
    }
}

bool wxWindowBase::IsEnabled() const
{
    return IsThisEnabled() && (IsTopLevel() || !GetParent() || GetParent()->IsEnabled());
}

// Define this macro if the corresponding operating system handles the state
// of children windows automatically when the parent is enabled/disabled.
// Otherwise wx itself must ensure that when the parent is disabled its
// children are disabled too, and their initial state is restored when the
// parent is enabled back.
#if defined(__WXMSW__)
    // must do everything ourselves
    #undef wxHAS_NATIVE_ENABLED_MANAGEMENT
#elif defined(__WXOSX__)
    // must do everything ourselves
    #undef wxHAS_NATIVE_ENABLED_MANAGEMENT
#else
    #define wxHAS_NATIVE_ENABLED_MANAGEMENT
#endif

void wxWindowBase::NotifyWindowOnEnableChange(bool enabled)
{
    DoEnable(enabled);

#ifndef wxHAS_NATIVE_ENABLED_MANAGEMENT
    // Disabling a top level window is typically done when showing a modal
    // dialog and we don't need to disable its children in this case, they will
    // be logically disabled anyhow (i.e. their IsEnabled() will return false)
    // and the TLW won't accept any input for them. Moreover, explicitly
    // disabling them would look ugly as the entire TLW would be greyed out
    // whenever a modal dialog is shown and no native applications under any
    // platform behave like this.
    if ( IsTopLevel() && !enabled )
        return;

    // When disabling (or enabling back) a non-TLW window we need to
    // recursively propagate the change of the state to its children, otherwise
    // they would still show as enabled even though they wouldn't actually
    // accept any input (at least under MSW where children don't accept input
    // if any of the windows in their parent chain is enabled).
    for ( wxWindowList::compatibility_iterator node = GetChildren().GetFirst();
          node;
          node = node->GetNext() )
    {
        wxWindowBase * const child = node->GetData();
        if ( !child->IsTopLevel() && child->IsThisEnabled() )
            child->NotifyWindowOnEnableChange(enabled);
    }
#endif // !defined(wxHAS_NATIVE_ENABLED_MANAGEMENT)
}

bool wxWindowBase::Enable(bool enable)
{
    if ( enable == IsThisEnabled() )
        return false;

    m_isEnabled = enable;

    NotifyWindowOnEnableChange(enable);

    return true;
}

bool wxWindowBase::IsShownOnScreen() const
{
    // A window is shown on screen if it itself is shown and so are all its
    // parents. But if a window is toplevel one, then its always visible on
    // screen if IsShown() returns true, even if it has a hidden parent.
    return IsShown() &&
           (IsTopLevel() || GetParent() == nullptr || GetParent()->IsShownOnScreen());
}

// ----------------------------------------------------------------------------
// RTTI
// ----------------------------------------------------------------------------

bool wxWindowBase::IsTopLevel() const
{
    return false;
}

// ----------------------------------------------------------------------------
// Freeze/Thaw
// ----------------------------------------------------------------------------

void wxWindowBase::Freeze()
{
    if ( !m_freezeCount++ )
    {
        // physically freeze this window:
        DoFreeze();

        // and recursively freeze all children:
        for ( wxWindowList::iterator i = GetChildren().begin();
              i != GetChildren().end(); ++i )
        {
            wxWindow *child = *i;
            if ( child->IsTopLevel() )
                continue;

            child->Freeze();
        }
    }
}

void wxWindowBase::Thaw()
{
    wxASSERT_MSG( m_freezeCount, "Thaw() without matching Freeze()" );

    if ( !--m_freezeCount )
    {
        // recursively thaw all children:
        for ( wxWindowList::iterator i = GetChildren().begin();
              i != GetChildren().end(); ++i )
        {
            wxWindow *child = *i;
            if ( child->IsTopLevel() )
                continue;

            child->Thaw();
        }

        // physically thaw this window:
        DoThaw();
    }
}

// ----------------------------------------------------------------------------
// Dealing with parents and children.
// ----------------------------------------------------------------------------

bool wxWindowBase::IsDescendant(wxWindowBase* win) const
{
    // Iterate until we find this window in the parent chain or exhaust it.
    while ( win )
    {
        if ( win == this )
            return true;

        // Stop iterating on reaching the top level window boundary.
        if ( win->IsTopLevel() )
            break;

        win = win->GetParent();
    }

    return false;
}

void wxWindowBase::AddChild(wxWindowBase *child)
{
    wxCHECK_RET( child, "can't add a NULL child" );

    // this should never happen and it will lead to a crash later if it does
    // because RemoveChild() will remove only one node from the children list
    // and the other(s) one(s) will be left with dangling pointers in them
    wxASSERT_MSG( !GetChildren().Find((wxWindow*)child), "AddChild() called twice" );

    GetChildren().Append((wxWindow*)child);
    child->SetParent(this);

    // adding a child while frozen will assert when thawed, so freeze it as if
    // it had been already present when we were frozen
    if ( IsFrozen() && !child->IsTopLevel() )
        child->Freeze();
}

void wxWindowBase::RemoveChild(wxWindowBase *child)
{
    wxCHECK_RET( child, "can't remove a NULL child" );

    // removing a child while frozen may result in permanently frozen window
    // if used e.g. from Reparent(), so thaw it
    //
    // NB: IsTopLevel() doesn't return true any more when a TLW child is being
    //     removed from its ~wxWindowBase, so check for IsBeingDeleted() too
    if ( IsFrozen() && !child->IsBeingDeleted() && !child->IsTopLevel() )
        child->Thaw();

    GetChildren().DeleteObject((wxWindow *)child);
    child->SetParent(nullptr);
}

void wxWindowBase::SetParent(wxWindowBase *parent)
{
    // This assert catches typos which may result in using "this" instead of
    // "parent" when creating the window. This doesn't happen often but when it
    // does the results are unpleasant because the program typically just
    // crashes when due to a stack overflow or something similar and this
    // assert doesn't cost much (OTOH doing a more general check that the
    // parent is not one of our children would be more expensive and probably
    // not worth it).
    wxASSERT_MSG( parent != this, "Can't use window as its own parent" );

    m_parent = (wxWindow *)parent;
}

bool wxWindowBase::Reparent(wxWindowBase *newParent)
{
    wxWindow *oldParent = GetParent();
    if ( newParent == oldParent )
    {
        // nothing done
        return false;
    }

    const bool oldEnabledState = IsEnabled();

    // unlink this window from the existing parent.
    if ( oldParent )
    {
        oldParent->RemoveChild(this);
    }
    else
    {
        wxTopLevelWindows.DeleteObject((wxWindow *)this);
    }

    // add it to the new one
    if ( newParent )
    {
        newParent->AddChild(this);
    }
    else
    {
        wxTopLevelWindows.Append((wxWindow *)this);
    }

    // We need to notify window (and its subwindows) if by changing the parent
    // we also change our enabled/disabled status.
    const bool newEnabledState = IsEnabled();
    if ( newEnabledState != oldEnabledState )
    {
        NotifyWindowOnEnableChange(newEnabledState);
    }

    return true;
}

// ----------------------------------------------------------------------------
// event handler stuff
// ----------------------------------------------------------------------------

void wxWindowBase::SetEventHandler(wxEvtHandler *handler)
{
    wxCHECK_RET(handler != nullptr, "SetEventHandler(NULL) called");

    m_eventHandler = handler;
}

void wxWindowBase::SetNextHandler([[maybe_unused]] wxEvtHandler *handler)
{
    // disable wxEvtHandler chain mechanism for wxWindows:
    // wxWindow uses its own stack mechanism which doesn't mix well with wxEvtHandler's one

    wxFAIL_MSG("wxWindow cannot be part of a wxEvtHandler chain");
}
void wxWindowBase::SetPreviousHandler([[maybe_unused]] wxEvtHandler *handler)
{
    // we can't simply wxFAIL here as in SetNextHandler: in fact the last
    // handler of our stack when is destroyed will be Unlink()ed and thus
    // will call this function to update the pointer of this window...

    //wxFAIL_MSG("wxWindow cannot be part of a wxEvtHandler chain");
}

void wxWindowBase::PushEventHandler(wxEvtHandler *handlerToPush)
{
    wxCHECK_RET( handlerToPush != nullptr, "PushEventHandler(NULL) called" );

    // the new handler is going to be part of the wxWindow stack of event handlers:
    // it can't be part also of an event handler double-linked chain:
    wxASSERT_MSG(handlerToPush->IsUnlinked(),
        "The handler being pushed in the wxWindow stack shouldn't be part of "
        "a wxEvtHandler chain; call Unlink() on it first");

    wxEvtHandler *handlerOld = GetEventHandler();
    wxCHECK_RET( handlerOld, "an old event handler is NULL?" );

    // now use wxEvtHandler double-linked list to implement a stack:
    handlerToPush->SetNextHandler(handlerOld);

    if (handlerOld != this)
        handlerOld->SetPreviousHandler(handlerToPush);

    SetEventHandler(handlerToPush);

#if wxDEBUG_LEVEL
    // final checks of the operations done above:
    wxASSERT_MSG( handlerToPush->GetPreviousHandler() == nullptr,
        "the first handler of the wxWindow stack should "
        "have no previous handlers set" );
    wxASSERT_MSG( handlerToPush->GetNextHandler() != nullptr,
        "the first handler of the wxWindow stack should "
        "have non-NULL next handler" );

    wxEvtHandler* pLast = handlerToPush;
    while ( pLast && pLast != this )
        pLast = pLast->GetNextHandler();
    wxASSERT_MSG( pLast->GetNextHandler() == nullptr,
        "the last handler of the wxWindow stack should "
        "have this window as next handler" );
#endif // wxDEBUG_LEVEL
}

wxEvtHandler *wxWindowBase::PopEventHandler(bool deleteHandler)
{
    // we need to pop the wxWindow stack, i.e. we need to remove the first handler

    wxEvtHandler *firstHandler = GetEventHandler();
    wxCHECK_MSG( firstHandler != nullptr, nullptr, "wxWindow cannot have a NULL event handler" );
    wxCHECK_MSG( firstHandler != this, nullptr, "cannot pop the wxWindow itself" );
    wxCHECK_MSG( firstHandler->GetPreviousHandler() == nullptr, nullptr,
        "the first handler of the wxWindow stack should have no previous handlers set" );

    wxEvtHandler *secondHandler = firstHandler->GetNextHandler();
    wxCHECK_MSG( secondHandler != nullptr, nullptr,
        "the first handler of the wxWindow stack should have non-NULL next handler" );

    firstHandler->SetNextHandler(nullptr);

    // It is harmless but useless to unset the previous handler of the window
    // itself as it's always NULL anyhow, so don't do this.
    if ( secondHandler != this )
        secondHandler->SetPreviousHandler(nullptr);

    // now firstHandler is completely unlinked; set secondHandler as the new window event handler
    SetEventHandler(secondHandler);

    if ( deleteHandler )
    {
        wxDELETE(firstHandler);
    }

    return firstHandler;
}

bool wxWindowBase::RemoveEventHandler(wxEvtHandler *handlerToRemove)
{
    wxCHECK_MSG( handlerToRemove != nullptr, false, "RemoveEventHandler(NULL) called" );
    wxCHECK_MSG( handlerToRemove != this, false, "Cannot remove the window itself" );

    if (handlerToRemove == GetEventHandler())
    {
        // removing the first event handler is equivalent to "popping" the stack
        PopEventHandler(false);
        return true;
    }

    // NOTE: the wxWindow event handler list is always terminated with "this" handler
    wxEvtHandler *handlerCur = GetEventHandler()->GetNextHandler();
    while ( handlerCur != this && handlerCur )
    {
        wxEvtHandler *handlerNext = handlerCur->GetNextHandler();

        if ( handlerCur == handlerToRemove )
        {
            handlerCur->Unlink();

            wxASSERT_MSG( handlerCur != GetEventHandler(),
                        "the case Remove == Pop should was already handled" );
            return true;
        }

        handlerCur = handlerNext;
    }

    wxFAIL_MSG( "where has the event handler gone?" );

    return false;
}

bool wxWindowBase::HandleWindowEvent(wxEvent& event) const
{
    // SafelyProcessEvent() will handle exceptions nicely
    return GetEventHandler()->SafelyProcessEvent(event);
}

// ----------------------------------------------------------------------------
// colours, fonts &c
// ----------------------------------------------------------------------------

void wxWindowBase::InheritAttributes()
{
    const wxWindowBase * const parent = GetParent();
    if ( !parent )
        return;

    // we only inherit attributes which had been explicitly set for the parent
    // which ensures that this only happens if the user really wants it and
    // not by default which wouldn't make any sense in modern GUIs where the
    // controls don't all use the same fonts (nor colours)
    if ( parent->m_inheritFont && !m_hasFont )
        SetFont(parent->GetFont());

    // in addition, there is a possibility to explicitly forbid inheriting
    // colours at each class level by overriding ShouldInheritColours()
    if ( ShouldInheritColours() )
    {
        if ( parent->m_inheritFgCol && !m_hasFgCol )
            SetForegroundColour(parent->GetForegroundColour());

        // inheriting (solid) background colour is wrong as it totally breaks
        // any kind of themed backgrounds
        //
        // instead, the controls should use the same background as their parent
        // (ideally by not drawing it at all)
#if 0
        if ( parent->m_inheritBgCol && !m_hasBgCol )
            SetBackgroundColour(parent->GetBackgroundColour());
#endif // 0
    }
}

/* static */ wxVisualAttributes
wxWindowBase::GetClassDefaultAttributes([[maybe_unused]] wxWindowVariant variant)
{
    // it is important to return valid values for all attributes from here,
    // GetXXX() below rely on this
    wxVisualAttributes attrs;
    attrs.font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    attrs.colFg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    attrs.colBg = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
    return attrs;
}

wxColour wxWindowBase::GetBackgroundColour() const
{
    if ( !m_backgroundColour.IsOk() )
    {
        wxASSERT_MSG( !m_hasBgCol, "we have invalid explicit bg colour?" );

        // get our default background colour
        wxColour colBg = GetDefaultAttributes().colBg;

        // we must return some valid colour to avoid redoing this every time
        // and also to avoid surprising the applications written for older
        // wxWidgets versions where GetBackgroundColour() always returned
        // something -- so give them something even if it doesn't make sense
        // for this window (e.g. it has a themed background)
        if ( !colBg.IsOk() )
            colBg = GetClassDefaultAttributes().colBg;

        return colBg;
    }
    else
        return m_backgroundColour;
}

wxColour wxWindowBase::GetForegroundColour() const
{
    // logic is the same as above
    if ( !m_hasFgCol && !m_foregroundColour.IsOk() )
    {
        wxColour colFg = GetDefaultAttributes().colFg;

        if ( !colFg.IsOk() )
            colFg = GetClassDefaultAttributes().colFg;

        return colFg;
    }
    else
        return m_foregroundColour;
}

bool wxWindowBase::SetBackgroundStyle(wxBackgroundStyle style)
{
    // The checks below shouldn't be triggered if we're not really changing the
    // style.
    if ( style == m_backgroundStyle )
        return true;

    // Transparent background style can be only set before creation because of
    // wxGTK limitation.
    wxCHECK_MSG( (style != wxBackgroundStyle::Transparent) || !GetHandle(),
                 false,
                 "wxBackgroundStyle::Transparent style can only be set before "
                 "Create()-ing the window." );

    // And once it is set, wxBackgroundStyle::Transparent can't be unset.
    wxCHECK_MSG( (m_backgroundStyle != wxBackgroundStyle::Transparent) ||
                 (style == wxBackgroundStyle::Transparent),
                 false,
                 "wxBackgroundStyle::Transparent can't be unset once it was set." );

    m_backgroundStyle = style;

    return true;
}

bool wxWindowBase::IsTransparentBackgroundSupported(std::string *reason) const
{
    if ( reason )
        *reason = _("This platform does not support background transparency.");

    return false;
}

bool wxWindowBase::SetBackgroundColour( const wxColour &colour )
{
    if ( colour == m_backgroundColour )
        return false;

    m_hasBgCol = colour.IsOk();

    m_inheritBgCol = m_hasBgCol;
    m_backgroundColour = colour;
    SetThemeEnabled( !m_hasBgCol && !m_foregroundColour.IsOk() );
    return true;
}

bool wxWindowBase::SetForegroundColour( const wxColour &colour )
{
    if (colour == m_foregroundColour )
        return false;

    m_hasFgCol = colour.IsOk();
    m_inheritFgCol = m_hasFgCol;
    m_foregroundColour = colour;
    SetThemeEnabled( !m_hasFgCol && !m_backgroundColour.IsOk() );
    return true;
}

bool wxWindowBase::SetCursor(const wxCursor& cursor)
{
    // setting an invalid cursor is ok, it means that we don't have any special
    // cursor
    if ( m_cursor.IsSameAs(cursor) )
    {
        // no change
        return false;
    }

    m_cursor = cursor;

    return true;
}

wxFont wxWindowBase::GetFont() const
{
    // logic is the same as in GetBackgroundColour()
    if ( !m_font.IsOk() )
    {
        wxASSERT_MSG( !m_hasFont, "we have invalid explicit font?" );

        wxFont font = GetDefaultAttributes().font;
        if ( !font.IsOk() )
            font = GetClassDefaultAttributes().font;

        WXAdjustFontToOwnPPI(font);

        return font;
    }
    else
        return m_font;
}

bool wxWindowBase::SetFont(const wxFont& font)
{
    if ( font == m_font )
    {
        // no change
        return false;
    }

    m_font = font;
    m_hasFont = font.IsOk();
    m_inheritFont = m_hasFont;

    if ( m_hasFont )
        WXAdjustFontToOwnPPI(m_font);

    InvalidateBestSize();

    return true;
}

#if wxUSE_PALETTE

void wxWindowBase::SetPalette(const wxPalette& pal)
{
    m_hasCustomPalette = true;
    m_palette = pal;

    // VZ: can anyone explain me what do we do here?
    wxWindowDC d((wxWindow *) this);
    d.SetPalette(pal);
}

wxWindow *wxWindowBase::GetAncestorWithCustomPalette() const
{
    wxWindow* win = const_cast<wxWindow*>(dynamic_cast<const wxWindow*>(this));
    while ( win && !win->HasCustomPalette() )
    {
        win = win->GetParent();
    }

    return win;
}

#endif // wxUSE_PALETTE

#if wxUSE_CARET
void wxWindowBase::SetCaret(std::unique_ptr<wxCaret> caret)
{
    m_caret = std::move(caret);

    if ( m_caret )
    {
        wxASSERT_MSG( m_caret->GetWindow() == this,
                      "caret should be created associated to this window" );
    }
}
#endif // wxUSE_CARET

#if wxUSE_VALIDATORS
// ----------------------------------------------------------------------------
// validators
// ----------------------------------------------------------------------------

void wxWindowBase::SetValidator(const wxValidator& validator)
{
    delete m_windowValidator;

    m_windowValidator = dynamic_cast<wxValidator *>(validator.Clone());

    if ( m_windowValidator )
        m_windowValidator->SetWindow(dynamic_cast<wxWindow*>(this));
}
#endif // wxUSE_VALIDATORS

// ----------------------------------------------------------------------------
// update region stuff
// ----------------------------------------------------------------------------

wxRect wxWindowBase::GetUpdateClientRect() const
{
    wxRegion rgnUpdate = GetUpdateRegion();
    rgnUpdate.Intersect(GetClientRect());
    wxRect rectUpdate = rgnUpdate.GetBox();
    wxPoint ptOrigin = GetClientAreaOrigin();
    rectUpdate.x -= ptOrigin.x;
    rectUpdate.y -= ptOrigin.y;

    return rectUpdate;
}

bool wxWindowBase::DoIsExposed(int x, int y) const
{
    return m_updateRegion.Contains(x, y) != wxRegionContain::Outside;
}

bool wxWindowBase::DoIsExposed(int x, int y, int w, int h) const
{
    return m_updateRegion.Contains(x, y, w, h) != wxRegionContain::Outside;
}

void wxWindowBase::ClearBackground()
{
    // wxGTK uses its own version, no need to add never used code
#ifndef __WXGTK__
    wxClientDC dc((wxWindow *)this);
    wxBrush brush(GetBackgroundColour(), wxBrushStyle::Solid);
    dc.SetBackground(brush);
    dc.Clear();
#endif // __WXGTK__
}

// ----------------------------------------------------------------------------
// find child window by id or name
// ----------------------------------------------------------------------------

wxWindow *wxWindowBase::wxFindWindow(long id) const
{
    if ( id == m_windowId )
        return const_cast<wxWindow*>(dynamic_cast<const wxWindow*>(this));

    wxWindowBase *res = nullptr;
    wxWindowList::compatibility_iterator node;
    for ( node = m_children.GetFirst(); node && !res; node = node->GetNext() )
    {
        wxWindowBase *child = node->GetData();

        // As usual, don't recurse into child dialogs, finding a button in a
        // child dialog when looking in this window would be unexpected.
        if ( child->IsTopLevel() )
            continue;

        res = child->wxFindWindow( id );
    }

    return dynamic_cast<wxWindow*>(res);
}

wxWindow *wxWindowBase::wxFindWindow(const std::string& name) const
{
    if ( name == m_windowName )
        return const_cast<wxWindow*>(dynamic_cast<const wxWindow*>(this));

    wxWindowBase *res = nullptr;
    wxWindowList::compatibility_iterator node;
    for ( node = m_children.GetFirst(); node && !res; node = node->GetNext() )
    {
        wxWindow *child = node->GetData();

        // As in FindWindow() overload above, never recurse into child dialogs.
        if ( child->IsTopLevel() )
            continue;

        res = child->wxFindWindow(name);
    }

    return dynamic_cast<wxWindow*>(res);
}


// find any window by id or name or label: If parent is non-NULL, look through
// children for a label or title matching the specified string. If NULL, look
// through all top-level windows.
//
// to avoid duplicating code we reuse the same helper function but with
// different comparators

using wxFindWindowCmp = bool (*)(const wxWindow *win,
                                 const std::string& label, long id);

static
bool wxFindWindowCmpLabels(const wxWindow *win, const std::string& label,
                           [[maybe_unused]] long id)
{
    return win->GetLabel() == label;
}

static
bool wxFindWindowCmpNames(const wxWindow *win, const std::string& label,
                          [[maybe_unused]] long id)
{
    return win->GetName() == label;
}

static
bool wxFindWindowCmpIds(const wxWindow *win, [[maybe_unused]] const std::string& label,
                        long id)
{
    return win->GetId() == id;
}

// recursive helper for the FindWindowByXXX() functions
static
wxWindow *wxFindWindowRecursively(const wxWindow *parent,
                                  const std::string& label,
                                  long id,
                                  wxFindWindowCmp cmp)
{
    // see if this is the one we're looking for
    if ( (*cmp)(parent, label, id) )
        return const_cast<wxWindow*>(parent);

    // It wasn't, so check all its children
    for ( wxWindowList::compatibility_iterator node = parent->GetChildren().GetFirst();
          node;
          node = node->GetNext() )
    {
        // recursively check each child
        auto* win = dynamic_cast<wxWindow*>(node->GetData());
        wxWindow *retwin = wxFindWindowRecursively(win, label, id, cmp);
        if (retwin)
            return retwin;
    }

    // Not found
    return nullptr;
}

// helper for FindWindowByXXX()
static
wxWindow *wxFindWindowHelper(const wxWindow *parent,
                             const std::string& label,
                             long id,
                             wxFindWindowCmp cmp)
{
    if ( parent )
    {
        // just check parent and all its children
        return wxFindWindowRecursively(parent, label, id, cmp);
    }

    // start at very top of wx's windows
    for ( wxWindowList::compatibility_iterator node = wxTopLevelWindows.GetFirst();
          node;
          node = node->GetNext() )
    {
        // recursively check each window & its children
        wxWindow *win = node->GetData();
        wxWindow *retwin = wxFindWindowRecursively(win, label, id, cmp);
        if (retwin)
            return retwin;
    }

    return nullptr;
}

/* static */
wxWindow *
wxWindowBase::FindWindowByLabel(const std::string& title, const wxWindow *parent)
{
    return wxFindWindowHelper(parent, title, 0, wxFindWindowCmpLabels);
}

/* static */
wxWindow *
wxWindowBase::FindWindowByName(const std::string& title, const wxWindow *parent)
{
    wxWindow *win = wxFindWindowHelper(parent, title, 0, wxFindWindowCmpNames);

    if ( !win )
    {
        // fall back to the label
        win = FindWindowByLabel(title, parent);
    }

    return win;
}

/* static */
wxWindow *
wxWindowBase::FindWindowById( long id, const wxWindow* parent )
{
    return wxFindWindowHelper(parent, {}, id, wxFindWindowCmpIds);
}

// ----------------------------------------------------------------------------
// dialog oriented functions
// ----------------------------------------------------------------------------

#if wxUSE_VALIDATORS

namespace
{

// This class encapsulates possibly recursive iteration on window children done
// by Validate() and TransferData{To,From}Window() and allows to avoid code
// duplication in all three functions.
class ValidationTraverserBase
{
public:
    explicit ValidationTraverserBase(wxWindowBase* win)
        : m_win(dynamic_cast<wxWindow*>(win))
    {
    }

    ValidationTraverserBase(const ValidationTraverserBase&) = delete;
	ValidationTraverserBase& operator=(const ValidationTraverserBase&) = delete;

    // Give it a virtual dtor just to suppress gcc warnings about a class with
    // virtual methods but non-virtual dtor -- even if this is completely safe
    // here as we never use the objects of this class polymorphically.
    virtual ~ValidationTraverserBase() = default;

    // Traverse all the direct children calling OnDo() on them and also all
    // grandchildren, calling OnRecurse() for them.
    bool DoForSelfAndChildren()
    {
        wxValidator* const validator = m_win->GetValidator();
        if ( validator && !OnDo(validator) )
        {
            return false;
        }

        wxWindowList& children = m_win->GetChildren();
        for ( wxWindowList::iterator i = children.begin();
              i != children.end();
              ++i )
        {
            auto* const child = dynamic_cast<wxWindow*>(*i);

            // Notice that validation should never recurse into top level
            // children, e.g. some other dialog which might happen to be
            // currently shown.
            if ( !child->IsTopLevel() && !OnRecurse(child) )
            {
                return false;
            }
        }

        return true;
    }

protected:
    // Called for each child, validator is guaranteed to be non-NULL.
    virtual bool OnDo(wxValidator* validator) = 0;

    // Called for each child if we need to recurse into its children.
    virtual bool OnRecurse(wxWindow* child) = 0;


    // The window whose children we're traversing.
    wxWindow* const m_win;
};

} // anonymous namespace

#endif // wxUSE_VALIDATORS

bool wxWindowBase::Validate()
{
#if wxUSE_VALIDATORS
    class ValidateTraverser : public ValidationTraverserBase
    {
    public:
        explicit ValidateTraverser(wxWindowBase* win)
            : ValidationTraverserBase(win)
        {
        }

        bool OnDo(wxValidator* validator) override
        {
            return validator->Validate(m_win);
        }

        bool OnRecurse(wxWindow* child) override
        {
            return child->Validate();
        }
    };

    return ValidateTraverser(this).DoForSelfAndChildren();
#else // !wxUSE_VALIDATORS
    return true;
#endif // wxUSE_VALIDATORS/!wxUSE_VALIDATORS
}

bool wxWindowBase::TransferDataToWindow()
{
#if wxUSE_VALIDATORS
    class DataToWindowTraverser : public ValidationTraverserBase
    {
    public:
        explicit DataToWindowTraverser(wxWindowBase* win)
            : ValidationTraverserBase(win)
        {
        }

        bool OnDo(wxValidator* validator) override
        {
            if ( !validator->TransferToWindow() )
            {
                wxLogWarning(_("Could not transfer data to window"));
#if wxUSE_LOG
                wxLog::FlushActive();
#endif // wxUSE_LOG

                return false;
            }

            return true;
        }

        bool OnRecurse(wxWindow* child) override
        {
            return child->TransferDataToWindow();
        }
    };

    return DataToWindowTraverser(this).DoForSelfAndChildren();
#else // !wxUSE_VALIDATORS
    return true;
#endif // wxUSE_VALIDATORS/!wxUSE_VALIDATORS
}

bool wxWindowBase::TransferDataFromWindow()
{
#if wxUSE_VALIDATORS
    class DataFromWindowTraverser : public ValidationTraverserBase
    {
    public:
        explicit DataFromWindowTraverser(wxWindowBase* win)
            : ValidationTraverserBase(win)
        {
        }

        bool OnDo(wxValidator* validator) override
        {
            return validator->TransferFromWindow();
        }

        bool OnRecurse(wxWindow* child) override
        {
            return child->TransferDataFromWindow();
        }
    };

    return DataFromWindowTraverser(this).DoForSelfAndChildren();
#else // !wxUSE_VALIDATORS
    return true;
#endif // wxUSE_VALIDATORS/!wxUSE_VALIDATORS
}

void wxWindowBase::InitDialog()
{
    wxInitDialogEvent event(GetId());
    event.SetEventObject( this );
    GetEventHandler()->ProcessEvent(event);
}

// ----------------------------------------------------------------------------
// context-sensitive help support
// ----------------------------------------------------------------------------

#if wxUSE_HELP

// associate this help text with this window
void wxWindowBase::SetHelpText(const std::string& text)
{
    wxHelpProvider *helpProvider = wxHelpProvider::Get();
    if ( helpProvider )
    {
        helpProvider->AddHelp(this, text);
    }
}

// get the help string associated with this window (may be empty)
// default implementation forwards calls to the help provider
std::string
wxWindowBase::GetHelpTextAtPoint(const [[maybe_unused]] wxPoint & pt,
                                 [[maybe_unused]] wxHelpEvent::Origin origin) const
{
    wxHelpProvider *helpProvider = wxHelpProvider::Get();
    if ( helpProvider )
    {
        return helpProvider->GetHelp(this);
    }

    return {};
}

// show help for this window
void wxWindowBase::OnHelp(wxHelpEvent& event)
{
    wxHelpProvider *helpProvider = wxHelpProvider::Get();
    if ( helpProvider )
    {
        wxPoint pos = event.GetPosition();
        const wxHelpEvent::Origin origin = event.GetOrigin();
        if ( origin == wxHelpEvent::Origin::Keyboard )
        {
            // if the help event was generated from keyboard it shouldn't
            // appear at the mouse position (which is still the only position
            // associated with help event) if the mouse is far away, although
            // we still do use the mouse position if it's over the window
            // because we suppose the user looks approximately at the mouse
            // already and so it would be more convenient than showing tooltip
            // at some arbitrary position which can be quite far from it
            const wxRect rectClient = GetClientRect();
            if ( !rectClient.Contains(ScreenToClient(pos)) )
            {
                // position help slightly under and to the right of this window
                pos = ClientToScreen(wxPoint(
                        2*wxGetCharWidth(),
                        rectClient.height + GetCharHeight()
                      ));
            }
        }

        if ( helpProvider->ShowHelpAtPoint(this, pos, origin) )
        {
            // skip the event.Skip() below
            return;
        }
    }

    event.Skip();
}

#endif // wxUSE_HELP

// ----------------------------------------------------------------------------
// tooltips
// ----------------------------------------------------------------------------

#if wxUSE_TOOLTIPS

std::string wxWindowBase::GetToolTipText() const
{
    return m_tooltip ? m_tooltip->GetTip() : "";
}

void wxWindowBase::DoSetToolTipText( const std::string& tip )
{
    // don't create the new tooltip if we already have one
    if ( m_tooltip )
    {
        m_tooltip->SetTip( tip );
    }
    else
    {
        SetToolTip( new wxToolTip( tip ) );
    }

    // setting empty tooltip text does not remove the tooltip any more - use
    // SetToolTip(NULL) for this
}

void wxWindowBase::DoSetToolTip(wxToolTip *tooltip)
{
    if ( m_tooltip != tooltip )
    {
        delete m_tooltip;
        m_tooltip = tooltip;
    }
}

bool wxWindowBase::CopyToolTip(wxToolTip *tip)
{
    SetToolTip(tip ? new wxToolTip(tip->GetTip()) : nullptr);

    return tip != nullptr;
}

#endif // wxUSE_TOOLTIPS

// ----------------------------------------------------------------------------
// constraints and sizers
// ----------------------------------------------------------------------------

void wxWindowBase::SetSizer(wxSizer *sizer, bool deleteOld)
{
    if ( sizer == m_windowSizer)
        return;

    if ( m_windowSizer )
    {
        m_windowSizer->SetContainingWindow(nullptr);

        if ( deleteOld )
            delete m_windowSizer;
    }

    m_windowSizer = sizer;
    if ( m_windowSizer )
    {
        m_windowSizer->SetContainingWindow((wxWindow *)this);
    }

    SetAutoLayout(m_windowSizer != nullptr);
}

void wxWindowBase::SetSizerAndFit(wxSizer *sizer, bool deleteOld)
{
    SetSizer( sizer, deleteOld );
    sizer->SetSizeHints( (wxWindow*) this );
}


void wxWindowBase::SetContainingSizer(wxSizer* sizer)
{
    // Adding a window to another sizer if it's already managed by one would
    // result in crashes later because one of the two sizers won't be notified
    // about the window destruction and so will use a dangling pointer when it
    // is destroyed itself. As such problems are hard to debug, don't allow
    // them to happen in the first place.
    if ( sizer )
    {
        // This would be caught by the check below too, but give a more clear
        // error message in this case.
        wxASSERT_MSG( m_containingSizer != sizer,
                      "Adding a window to the same sizer twice?" );

        wxCHECK_RET( !m_containingSizer,
                     "Adding a window already in a sizer, detach it first!" );
    }

    m_containingSizer = sizer;
}

bool wxWindowBase::Layout()
{
    // If there is a sizer, use it instead of the constraints
    if ( GetSizer() )
    {
        int w = 0, h = 0;
        GetVirtualSize(&w, &h);
        GetSizer()->SetDimension( 0, 0, w, h );
    }

    return true;
}

void wxWindowBase::InternalOnSize(wxSizeEvent& event)
{
    if ( GetAutoLayout() )
        Layout();

    event.Skip();
}

void wxWindowBase::AdjustForParentClientOrigin(int& x, int& y, unsigned int sizeFlags) const
{
    wxWindow *parent = GetParent();
    if ( !(sizeFlags & wxSIZE_NO_ADJUSTMENTS) && parent )
    {
        wxPoint pt(parent->GetClientAreaOrigin());
        x += pt.x;
        y += pt.y;
    }
}

// ----------------------------------------------------------------------------
// Update UI processing
// ----------------------------------------------------------------------------

void wxWindowBase::UpdateWindowUI(unsigned int flags)
{
    wxUpdateUIEvent event(GetId());
    event.SetEventObject(this);

    if ( GetEventHandler()->ProcessEvent(event) )
    {
        DoUpdateWindowUI(event);
    }

    if (flags & wxUPDATE_UI_RECURSE)
    {
        wxWindowList::compatibility_iterator node = GetChildren().GetFirst();
        while (node)
        {
            wxWindow* child = (wxWindow*) node->GetData();
            child->UpdateWindowUI(flags);
            node = node->GetNext();
        }
    }
}

// do the window-specific processing after processing the update event
void wxWindowBase::DoUpdateWindowUI(wxUpdateUIEvent& event)
{
    if ( event.GetSetEnabled() )
        Enable(event.GetEnabled());

    if ( event.GetSetShown() )
        Show(event.GetShown());
}

// ----------------------------------------------------------------------------
// Idle processing
// ----------------------------------------------------------------------------

// Send idle event to window and all subwindows
bool wxWindowBase::SendIdleEvents(wxIdleEvent& event)
{
    bool needMore = false;

    OnInternalIdle();

    // should we send idle event to this window?
    if (wxIdleEvent::GetMode() == wxIDLE_PROCESS_ALL ||
        HasExtraStyle(wxWS_EX_PROCESS_IDLE))
    {
        event.SetEventObject(this);
        HandleWindowEvent(event);

        if (event.MoreRequested())
            needMore = true;
    }
    wxWindowList::compatibility_iterator node = GetChildren().GetFirst();
    for (; node; node = node->GetNext())
    {
        wxWindow* child = node->GetData();
        if (child->SendIdleEvents(event))
            needMore = true;
    }

    return needMore;
}

void wxWindowBase::OnInternalIdle()
{
    if ( wxUpdateUIEvent::CanUpdate(this) )
        UpdateWindowUI(wxUPDATE_UI_FROMIDLE);
}

// ----------------------------------------------------------------------------
// DPI-independent pixels and dialog units translations
// ----------------------------------------------------------------------------

wxSize wxWindowBase::GetDPI() const
{
    return wxDisplay(dynamic_cast<const wxWindow*>(this)).GetPPI();
}

#ifndef wxHAVE_DPI_INDEPENDENT_PIXELS

namespace
{

static wxSize GetDPIHelper(const wxWindowBase* w)
{
    wxSize dpi;

    if ( w )
        dpi = w->GetDPI();
    if ( !dpi.x || !dpi.y )
        dpi = wxScreenDC().GetPPI();
    if ( !dpi.x || !dpi.y )
        dpi = wxDisplay::GetStdPPI();

    return dpi;
}

}

/* static */
wxSize
wxWindowBase::FromDIP(const wxSize& sz, const wxWindowBase* w)
{
    const wxSize dpi = GetDPIHelper(w);

    static constexpr int baseline = wxDisplay::GetStdPPIValue();

    // Take care to not scale -1 because it has a special meaning of
    // "unspecified" which should be preserved.
    return {sz.x == -1 ? -1 : wxMulDivInt32(sz.x, dpi.x, baseline),
            sz.y == -1 ? -1 : wxMulDivInt32(sz.y, dpi.y, baseline)};
}

/* static */
wxSize
wxWindowBase::ToDIP(const wxSize& sz, const wxWindowBase* w)
{
    const wxSize dpi = GetDPIHelper(w);

    constexpr int baseline = wxDisplay::GetStdPPIValue();

    // Take care to not scale -1 because it has a special meaning of
    // "unspecified" which should be preserved.
    return {sz.x == -1 ? -1 : wxMulDivInt32(sz.x, baseline, dpi.x),
            sz.y == -1 ? -1 : wxMulDivInt32(sz.y, baseline, dpi.y)};
}

#endif // !wxHAVE_DPI_INDEPENDENT_PIXELS

// Windows' computes dialog units using average character width over upper-
// and lower-case ASCII alphabet and not using the average character width
// metadata stored in the font; see
// http://support.microsoft.com/default.aspx/kb/145994 for detailed discussion.
// It's important that we perform the conversion in identical way, because
// dialog units natively exist only on Windows and Windows HIG is expressed
// using them.
wxSize wxWindowBase::GetDlgUnitBase() const
{
    const wxWindowBase* const parent =
        wxGetTopLevelParent(const_cast<wxWindow*>(dynamic_cast<const wxWindow*>(this)));

    wxCHECK_MSG( parent, wxDefaultSize, "Must have TLW parent" );

    if ( !parent->m_font.IsOk() )
    {
        // Default GUI font is used. This is the most common case, so
        // cache the results.
        static wxPrivate::DpiDependentValue<wxSize> s_defFontSize;
        if ( s_defFontSize.HasChanged(parent) )
            s_defFontSize.SetAtNewDPI(wxPrivate::GetAverageASCIILetterSize(*parent));
        return s_defFontSize.Get();
    }
    else
    {
        // Custom font, we always need to compute the result
        return wxPrivate::GetAverageASCIILetterSize(*parent);
    }
}

wxPoint wxWindowBase::ConvertPixelsToDialog(const wxPoint& pt) const
{
    const wxSize base = GetDlgUnitBase();

    // NB: wxMulDivInt32() is used, because it correctly rounds the result

    wxPoint pt2 = wxDefaultPosition;
    if (pt.x != wxDefaultCoord)
        pt2.x = wxMulDivInt32(pt.x, 4, base.x);
    if (pt.y != wxDefaultCoord)
        pt2.y = wxMulDivInt32(pt.y, 8, base.y);

    return pt2;
}

wxPoint wxWindowBase::ConvertDialogToPixels(const wxPoint& pt) const
{
    const wxSize base = GetDlgUnitBase();

    wxPoint pt2 = wxDefaultPosition;
    if (pt.x != wxDefaultCoord)
        pt2.x = wxMulDivInt32(pt.x, base.x, 4);
    if (pt.y != wxDefaultCoord)
        pt2.y = wxMulDivInt32(pt.y, base.y, 8);

    return pt2;
}

// ----------------------------------------------------------------------------
// event handlers
// ----------------------------------------------------------------------------

// propagate the colour change event to the subwindows
void wxWindowBase::OnSysColourChanged([[maybe_unused]] wxSysColourChangedEvent& event)
{
    wxWindowList::compatibility_iterator node = GetChildren().GetFirst();
    while ( node )
    {
        // Only propagate to non-top-level windows
        wxWindow *win = node->GetData();
        if ( !win->IsTopLevel() )
        {
            wxSysColourChangedEvent event2;
            event2.SetEventObject(win);
            win->GetEventHandler()->ProcessEvent(event2);
        }

        node = node->GetNext();
    }

    Refresh();
}

// the default action is to populate dialog with data when it's created,
// and nudge the UI into displaying itself correctly in case
// we've turned the wxUpdateUIEvents frequency down low.
void wxWindowBase::OnInitDialog( [[maybe_unused]] wxInitDialogEvent &event )
{
    TransferDataToWindow();

    // Update the UI at this point
    UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

// ----------------------------------------------------------------------------
// menu-related functions
// ----------------------------------------------------------------------------

#if wxUSE_MENUS

bool wxWindowBase::PopupMenu(wxMenu *menu, int x, int y)
{
    wxCHECK_MSG( menu, false, "can't popup NULL menu" );

    wxMenuInvokingWindowSetter
        setInvokingWin(*menu, dynamic_cast<wxWindow *>(this));

    wxCurrentPopupMenu = menu;
    const bool rc = DoPopupMenu(menu, x, y);
    wxCurrentPopupMenu = nullptr;

    return rc;
}

// this is used to pass the id of the selected item from the menu event handler
// to the main function itself
//
// it's ok to use a global here as there can be at most one popup menu shown at
// any time
static int gs_popupMenuSelection = wxID_NONE;

void wxWindowBase::InternalOnPopupMenu(wxCommandEvent& event)
{
    // store the id in a global variable where we'll retrieve it from later
    gs_popupMenuSelection = event.GetId();
}

void wxWindowBase::InternalOnPopupMenuUpdate([[maybe_unused]] wxUpdateUIEvent& event)
{
    // nothing to do but do not skip it
}

int
wxWindowBase::DoGetPopupMenuSelectionFromUser(wxMenu& menu, int x, int y)
{
    gs_popupMenuSelection = wxID_NONE;

    Bind(wxEVT_MENU, &wxWindowBase::InternalOnPopupMenu, this);

    // it is common to construct the menu passed to this function dynamically
    // using some fixed range of ids which could clash with the ids used
    // elsewhere in the program, which could result in some menu items being
    // unintentionally disabled or otherwise modified by update UI handlers
    // elsewhere in the program code and this is difficult to avoid in the
    // program itself, so instead we just temporarily suspend UI updating while
    // this menu is shown
    Bind(wxEVT_UPDATE_UI, &wxWindowBase::InternalOnPopupMenuUpdate, this);

    PopupMenu(&menu, x, y);

    Unbind(wxEVT_UPDATE_UI, &wxWindowBase::InternalOnPopupMenuUpdate, this);
    Unbind(wxEVT_MENU, &wxWindowBase::InternalOnPopupMenu, this);

    return gs_popupMenuSelection;
}

#endif // wxUSE_MENUS

bool wxWindowBase::WXSendContextMenuEvent(const wxPoint& posInScreenCoords)
{
    // When the mouse click happens in a subwindow of a composite control,
    // the user-visible event should seem to originate from the main window
    // and, notably, use its ID and not the (usually auto-generated and so
    // not very useful) ID of the subwindow.
    wxWindow* const mainWin = GetMainWindowOfCompositeControl();

    wxContextMenuEvent
        evtCtx(wxEVT_CONTEXT_MENU, mainWin->GetId(), posInScreenCoords);
    evtCtx.SetEventObject(mainWin);
    return mainWin->HandleWindowEvent(evtCtx);
}

// methods for drawing the sizers in a visible way: this is currently only
// enabled for "full debug" builds with wxDEBUG_LEVEL==2 as it doesn't work
// that well and also because we don't want to leave it enabled in default
// builds used for production
#if wxDEBUG_LEVEL > 1

static void DrawSizers(wxWindowBase *win);

static void DrawBorder(wxWindowBase *win, const wxRect& rect, bool fill, const wxPen* pen)
{
    wxClientDC dc((wxWindow *)win);
    dc.SetPen(*pen);
    dc.SetBrush(fill ? wxBrush(pen->GetColour(), wxBrushStyle::CrossDiagHatch) :
                       *wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(rect.Deflate(1, 1));
}

static void DrawSizer(wxWindowBase *win, wxSizer *sizer)
{
    const wxSizerItemList& items = sizer->GetChildren();
    for ( wxSizerItemList::const_iterator i = items.begin(),
                                        end = items.end();
          i != end;
          ++i )
    {
        wxSizerItem *item = *i;
        if ( item->IsSizer() )
        {
            DrawBorder(win, item->GetRect().Deflate(2), false, wxRED_PEN);
            DrawSizer(win, item->GetSizer());
        }
        else if ( item->IsSpacer() )
        {
            DrawBorder(win, item->GetRect().Deflate(2), true, wxBLUE_PEN);
        }
        else if ( item->IsWindow() )
        {
            DrawSizers(item->GetWindow());
        }
        else
            wxFAIL_MSG("inconsistent wxSizerItem status!");
    }
}

static void DrawSizers(wxWindowBase *win)
{
    DrawBorder(win, win->GetClientSize(), false, wxGREEN_PEN);

    wxSizer *sizer = win->GetSizer();
    if ( sizer )
    {
        DrawSizer(win, sizer);
    }
    else // no sizer, still recurse into the children
    {
        const wxWindowList& children = win->GetChildren();
        for ( wxWindowList::const_iterator i = children.begin(),
                                         end = children.end();
              i != end;
              ++i )
        {
            DrawSizers(*i);
        }

        // show all kind of sizes of this window; see the "window sizing" topic
        // overview for more info about the various differences:
        wxSize fullSz = win->GetSize();
        wxSize clientSz = win->GetClientSize();
        wxSize bestSz = win->GetBestSize();
        wxSize minSz = win->GetMinSize();
        wxSize maxSz = win->GetMaxSize();
        wxSize virtualSz = win->GetVirtualSize();

        wxMessageOutputDebug dbgout;
        dbgout.Printf(
            "%-10s => fullsz=%4d;%-4d  clientsz=%4d;%-4d  bestsz=%4d;%-4d  minsz=%4d;%-4d  maxsz=%4d;%-4d virtualsz=%4d;%-4d\n",
            win->GetName(),
            fullSz.x, fullSz.y,
            clientSz.x, clientSz.y,
            bestSz.x, bestSz.y,
            minSz.x, minSz.y,
            maxSz.x, maxSz.y,
            virtualSz.x, virtualSz.y);
    }
}

#endif // wxDEBUG_LEVEL

// process special middle clicks
void wxWindowBase::OnMiddleClick( wxMouseEvent& event )
{
    if ( event.ControlDown() && event.AltDown() )
    {
#if wxDEBUG_LEVEL > 1
        // Ctrl-Alt-Shift-mclick makes the sizers visible in debug builds
        if ( event.ShiftDown() )
        {
            DrawSizers(this);
        }
        else
#endif // __WXDEBUG__
        {
#if wxUSE_MSGDLG
            // just Ctrl-Alt-middle click shows information about wx version
            ::wxInfoMessageBox((wxWindow*)this);
#endif // wxUSE_MSGDLG
        }
    }
    else
    {
        event.Skip();
    }
}

// ----------------------------------------------------------------------------
// accessibility
// ----------------------------------------------------------------------------

#if wxUSE_ACCESSIBILITY
void wxWindowBase::SetAccessible(wxAccessible* accessible)
{
    if (m_accessible && (accessible != m_accessible))
        delete m_accessible;
    m_accessible = accessible;
    if (m_accessible)
        m_accessible->SetWindow((wxWindow*) this);
}

// Returns the accessible object, creating if necessary.
wxAccessible* wxWindowBase::GetOrCreateAccessible()
{
    if (!m_accessible)
        m_accessible = CreateAccessible();
    return m_accessible;
}

#endif

// ----------------------------------------------------------------------------
// list classes implementation
// ----------------------------------------------------------------------------

#include "wx/listimpl.cpp"
WX_DEFINE_LIST(wxWindowList)

// ----------------------------------------------------------------------------
// borders
// ----------------------------------------------------------------------------

wxBorder wxWindowBase::GetBorder(long flags) const
{
    wxBorder border = (wxBorder)(flags & wxBORDER_MASK);
    if ( border == wxBORDER_DEFAULT )
    {
        border = GetDefaultBorder();
    }
    else if ( border == wxBORDER_THEME )
    {
        border = GetDefaultBorderForControl();
    }

    return border;
}

wxBorder wxWindowBase::GetDefaultBorder() const
{
    return wxBORDER_NONE;
}

// ----------------------------------------------------------------------------
// hit testing
// ----------------------------------------------------------------------------

wxHitTest wxWindowBase::DoHitTest(wxCoord x, wxCoord y) const
{
    // here we just check if the point is inside the window or not

    // check the top and left border first
    bool outside = x < 0 || y < 0;
    if ( !outside )
    {
        // check the right and bottom borders too
        wxSize size = GetSize();
        outside = x >= size.x || y >= size.y;
    }

    return outside ? wxHT_WINDOW_OUTSIDE : wxHT_WINDOW_INSIDE;
}

// ----------------------------------------------------------------------------
// mouse capture
// ----------------------------------------------------------------------------

// Private data used for mouse capture tracking.
namespace wxMouseCapture
{

// Stack of the windows which previously had the capture, the top most element
// is the window that has the mouse capture now.
//
// NB: We use vector and not wxStack to be able to examine all of the stack
//     elements for debug checks, but only the stack operations should be
//     performed with this vector.
std::vector<wxWindow*> cap_stack;

// Flag preventing reentrancy in {Capture,Release}Mouse().
wxRecursionGuardFlag changing;

bool IsInCaptureStack(wxWindowBase* win)
{
    return cap_stack.end() != std::ranges::find_if(cap_stack,
        [win](const auto window){ return window == win; });
}

} // wxMouseCapture

void wxWindowBase::CaptureMouse()
{
    wxLogTrace("mousecapture", "CaptureMouse(%p)", this);

    wxRecursionGuard guard(wxMouseCapture::changing);
    wxASSERT_MSG( !guard.IsInside(), "recursive CaptureMouse call?" );

    wxASSERT_MSG( !wxMouseCapture::IsInCaptureStack(this),
                    "Recapturing the mouse in the same window?" );

    wxWindow *winOld = GetCapture();
    if ( winOld )
        ((wxWindowBase*) winOld)->DoReleaseMouse();

    DoCaptureMouse();

    wxMouseCapture::cap_stack.push_back(dynamic_cast<wxWindow*>(this));
}

void wxWindowBase::ReleaseMouse()
{
    wxLogTrace("mousecapture", "ReleaseMouse(%p)", this);

    wxRecursionGuard guard(wxMouseCapture::changing);
    wxASSERT_MSG( !guard.IsInside(), "recursive ReleaseMouse call?" );

#if wxDEBUG_LEVEL
    wxWindow* const winCapture = GetCapture();
    if ( !winCapture )
    {
        wxFAIL_MSG
        (
          fmt::format
          (
            "Releasing mouse in (%s) but it is not captured",
            std::string{wxGetClassInfo()->wxGetClassName()}
            // FIXME: Non-void pointer used for formatting.
            // "Releasing mouse in %p(%s) but it is not captured",
            // static_cast<void*>(this),
            // wxGetClassInfo()->wxGetClassName()
          )
        );
    }
    else if ( winCapture != this )
    {
        wxFAIL_MSG
        (
          fmt::format
          (
            "Releasing mouse in (%s) but it is captured by (%s)",
            std::string{wxGetClassInfo()->wxGetClassName()},
            std::string{winCapture->wxGetClassInfo()->wxGetClassName()}
          )
        // FIXME: non-void pointers used for formatting.
        //   fmt::format
        //   (
        //     "Releasing mouse in %p(%s) but it is captured by %p(%s)",
        //     this,
        //     wxGetClassInfo()->wxGetClassName(),
        //     winCapture,
        //     winCapture->wxGetClassInfo()->wxGetClassName()
        //   )
        );
    }
#endif // wxDEBUG_LEVEL

    DoReleaseMouse();

    wxCHECK_RET( !wxMouseCapture::cap_stack.empty(),
                    "Releasing mouse capture but cap_stack empty?" );
    wxCHECK_RET( wxMouseCapture::cap_stack.back() == this,
                    "Window releasing mouse capture not top of cap_stack?" );

    wxMouseCapture::cap_stack.pop_back();

    // Restore the capture to the previous window, if any.
    if ( !wxMouseCapture::cap_stack.empty() )
    {
        ((wxWindowBase*)wxMouseCapture::cap_stack.back())->DoCaptureMouse();
    }

    wxLogTrace("mousecapture",
        "After ReleaseMouse() mouse is captured by %p",
        static_cast<void*>(GetCapture()));
}

static void DoNotifyWindowAboutCaptureLost(wxWindow *win)
{
    wxMouseCaptureLostEvent event(win->GetId());
    event.SetEventObject(win);
    if ( !win->GetEventHandler()->ProcessEvent(event) )
    {
        // windows must handle this event, otherwise the app wouldn't behave
        // correctly if it loses capture unexpectedly; see the discussion here:
        // https://trac.wxwidgets.org/ticket/2277
        // http://article.gmane.org/gmane.comp.lib.wxwidgets.devel/82376
        wxFAIL_MSG( "window that captured the mouse didn't process wxEVT_MOUSE_CAPTURE_LOST" );
    }
}

/* static */
void wxWindowBase::NotifyCaptureLost()
{
    // don't do anything if capture lost was expected, i.e. resulted from
    // a wx call to ReleaseMouse or CaptureMouse:
    wxRecursionGuard guard(wxMouseCapture::changing);
    if ( guard.IsInside() )
        return;

    // if the capture was lost unexpectedly, notify every window that has
    // capture (on cap_stack or current) about it and clear the cap_stack:
    while ( !wxMouseCapture::cap_stack.empty() )
    {
        DoNotifyWindowAboutCaptureLost(wxMouseCapture::cap_stack.back());

        wxMouseCapture::cap_stack.pop_back();
    }
}

#if wxUSE_HOTKEY

bool
wxWindowBase::RegisterHotKey([[maybe_unused]] int hotkeyId,
                             [[maybe_unused]] int modifiers,
                             [[maybe_unused]] int keycode)
{
    // not implemented
    return false;
}

bool wxWindowBase::UnregisterHotKey([[maybe_unused]] int hotkeyId)
{
    // not implemented
    return false;
}

#endif // wxUSE_HOTKEY

// ----------------------------------------------------------------------------
// event processing
// ----------------------------------------------------------------------------

bool wxWindowBase::TryBefore(wxEvent& event)
{
#if wxUSE_VALIDATORS
    // Can only use the validator of the window which
    // is receiving the event
    if ( event.GetEventObject() == this )
    {
        wxValidator * const validator = GetValidator();
        if ( validator && validator->ProcessEventLocally(event) )
        {
            return true;
        }
    }
#endif // wxUSE_VALIDATORS

    return wxEvtHandler::TryBefore(event);
}

bool wxWindowBase::TryAfter(wxEvent& event)
{
    // carry on up the parent-child hierarchy if the propagation count hasn't
    // reached zero yet
    if ( event.ShouldPropagate() )
    {
        // honour the requests to stop propagation at this window: this is
        // used by the dialogs, for example, to prevent processing the events
        // from the dialog controls in the parent frame which rarely, if ever,
        // makes sense
        if ( !(GetExtraStyle() & wxWS_EX_BLOCK_EVENTS) )
        {
            wxWindow *parent = GetParent();
            if ( parent && !parent->IsBeingDeleted() )
            {
                wxPropagateOnce propagateOnce(event, this);

                return parent->GetEventHandler()->ProcessEvent(event);
            }
        }
    }

    return wxEvtHandler::TryAfter(event);
}

// ----------------------------------------------------------------------------
// window relationships
// ----------------------------------------------------------------------------

wxWindow *wxWindowBase::DoGetSibling(WindowOrder order) const
{
    wxCHECK_MSG( GetParent(), nullptr,
                    "GetPrev/NextSibling() don't work for TLWs!" );

    wxWindowList& siblings = GetParent()->GetChildren();
    wxWindowList::compatibility_iterator i = siblings.Find(dynamic_cast<const wxWindow*>(this));
    wxCHECK_MSG( i, nullptr, "window not a child of its parent?" );

    if ( order == WindowOrder::Before )
        i = i->GetPrevious();
    else // WindowOrder::After
        i = i->GetNext();

    return i ? i->GetData() : nullptr;
}

// ----------------------------------------------------------------------------
// keyboard navigation
// ----------------------------------------------------------------------------

// Navigates in the specified direction inside this window
bool wxWindowBase::DoNavigateIn(unsigned int flags)
{
#ifdef wxHAS_NATIVE_TAB_TRAVERSAL
    // native code doesn't process our wxNavigationKeyEvents anyhow
    wxUnusedVar(flags);
    return false;
#else // !wxHAS_NATIVE_TAB_TRAVERSAL
    wxNavigationKeyEvent eventNav;
    wxWindow *focused = FindFocus();
    eventNav.SetCurrentFocus(focused);
    eventNav.SetEventObject(focused);
    eventNav.SetFlags(flags);
    return GetEventHandler()->ProcessEvent(eventNav);
#endif // wxHAS_NATIVE_TAB_TRAVERSAL/!wxHAS_NATIVE_TAB_TRAVERSAL
}

bool wxWindowBase::HandleAsNavigationKey(const wxKeyEvent& event)
{
    if ( event.GetKeyCode() != WXK_TAB )
        return false;

    int flags = wxNavigationKeyEvent::FromTab;

    if ( event.ShiftDown() )
        flags |= wxNavigationKeyEvent::IsBackward;
    else
        flags |= wxNavigationKeyEvent::IsForward;

    if ( event.ControlDown() )
        flags |= wxNavigationKeyEvent::WinChange;

    Navigate(flags);
    return true;
}

void wxWindowBase::DoMoveInTabOrder(wxWindow *win, WindowOrder move)
{
    // check that we're not a top level window
    wxCHECK_RET( GetParent(),
                    "MoveBefore/AfterInTabOrder() don't work for TLWs!" );

    // detect the special case when we have nothing to do anyhow and when the
    // code below wouldn't work
    if ( win == this )
        return;

    // find the target window in the siblings list
    wxWindowList& siblings = GetParent()->GetChildren();
    wxWindowList::compatibility_iterator i = siblings.Find(win);
    wxCHECK_RET( i, "MoveBefore/AfterInTabOrder(): win is not a sibling" );

    // unfortunately DetachNode() is not
    // implemented so we can't just move the node around
    wxWindow *self = (wxWindow *)this;
    siblings.DeleteObject(self);
    if ( move == WindowOrder::After )
    {
        i = i->GetNext();
    }

    if ( i )
    {
        siblings.Insert(i, self);
    }
    else // WindowOrder::After and win was the last sibling
    {
        siblings.Append(self);
    }
}

// ----------------------------------------------------------------------------
// focus handling
// ----------------------------------------------------------------------------

/*static*/ wxWindow* wxWindowBase::FindFocus()
{
    wxWindowBase *win = DoFindFocus();
    return win ? win->GetMainWindowOfCompositeControl() : nullptr;
}

bool wxWindowBase::HasFocus() const
{
    wxWindowBase* const win = DoFindFocus();
    return win &&
            (this == win || this == win->GetMainWindowOfCompositeControl());
}

// ----------------------------------------------------------------------------
// drag and drop
// ----------------------------------------------------------------------------

#if wxUSE_DRAG_AND_DROP && !defined(__WXMSW__)

namespace
{

class DragAcceptFilesTarget : public wxFileDropTarget
{
public:
    DragAcceptFilesTarget(wxWindowBase *win) : m_win(win) {}

    virtual bool OnDropFiles(wxCoord x, wxCoord y,
                             const std::vector<std::string>& filenames) override
    {
        wxDropFilesEvent event(wxEVT_DROP_FILES,
                               filenames.size(),
                               wxCArrayString(filenames).Release());
        event.SetEventObject(m_win);
        event.m_pos.x = x;
        event.m_pos.y = y;

        return m_win->HandleWindowEvent(event);
    }

private:
    wxWindowBase * const m_win;

    DragAcceptFilesTarget(const DragAcceptFilesTarget&) = delete;
	DragAcceptFilesTarget& operator=(const DragAcceptFilesTarget&) = delete;
};


} // anonymous namespace

// Generic version of DragAcceptFiles(). It works by installing a simple
// wxFileDropTarget-to-EVT_DROP_FILES adaptor and therefore cannot be used
// together with explicit SetDropTarget() calls.
void wxWindowBase::DragAcceptFiles(bool accept)
{
    if ( accept )
    {
        wxASSERT_MSG( !GetDropTarget(),
                      "cannot use DragAcceptFiles() and SetDropTarget() together" );
        SetDropTarget(new DragAcceptFilesTarget(this));
    }
    else
    {
        SetDropTarget(NULL);
    }
}

#endif // wxUSE_DRAG_AND_DROP && !defined(__WXMSW__)

// ----------------------------------------------------------------------------
// global functions
// ----------------------------------------------------------------------------

wxWindow* wxGetTopLevelParent(wxWindowBase *win_)
{
    auto* win = dynamic_cast<wxWindow *>(win_);
    while ( win && !win->IsTopLevel() )
         win = win->GetParent();

    return win;
}

#if wxUSE_ACCESSIBILITY
// ----------------------------------------------------------------------------
// accessible object for windows
// ----------------------------------------------------------------------------

// Can return either a child object, or an integer
// representing the child element, starting from 1.
wxAccStatus wxWindowAccessible::HitTest([[maybe_unused]] const wxPoint& pt, [[maybe_unused]] int* childId, [[maybe_unused]] wxAccessible** childObject)
{
    wxCHECK( GetWindow() != nullptr, wxAccStatus::Fail );

    return wxAccStatus::NotImplemented;
}

// Returns the rectangle for this object (id = 0) or a child element (id > 0).
wxAccStatus wxWindowAccessible::GetLocation(wxRect& rect, int elementId)
{
    wxCHECK( GetWindow() != nullptr, wxAccStatus::Fail );

    wxWindow* win = nullptr;
    if (elementId == wxACC_SELF)
    {
        win = GetWindow();
    }
    else
    {
        if (elementId <= (int) GetWindow()->GetChildren().GetCount())
        {
            win = GetWindow()->GetChildren().Item(elementId-1)->GetData();
        }
        else
            return wxAccStatus::Fail;
    }
    if (win)
    {
        rect = win->GetRect();
        if (win->GetParent() && !dynamic_cast<wxTopLevelWindow*>(win))
            rect.SetPosition(win->GetParent()->ClientToScreen(rect.GetPosition()));
        return wxAccStatus::Ok;
    }

    return wxAccStatus::NotImplemented;
}

// Navigates from fromId to toId/toObject.
wxAccStatus wxWindowAccessible::Navigate(wxNavDir navDir, int fromId,
                             [[maybe_unused]] int* toId, wxAccessible** toObject)
{
    wxCHECK( GetWindow() != nullptr, wxAccStatus::Fail );

    switch (navDir)
    {
    case wxNavDir::FirstChild:
        {
            if (GetWindow()->GetChildren().GetCount() == 0)
                return wxAccStatus::False;
            wxWindow* childWindow = (wxWindow*) GetWindow()->GetChildren().GetFirst()->GetData();
            *toObject = childWindow->GetOrCreateAccessible();

            return wxAccStatus::Ok;
        }
    case wxNavDir::LastChild:
        {
            if (GetWindow()->GetChildren().GetCount() == 0)
                return wxAccStatus::False;
            wxWindow* childWindow = (wxWindow*) GetWindow()->GetChildren().GetLast()->GetData();
            *toObject = childWindow->GetOrCreateAccessible();

            return wxAccStatus::Ok;
        }
    case wxNavDir::Right:
    case wxNavDir::Down:
    case wxNavDir::Next:
        {
            wxWindowList::compatibility_iterator node =
                wxWindowList::compatibility_iterator();
            if (fromId == wxACC_SELF)
            {
                // Can't navigate to sibling of this window
                // if we're a top-level window.
                if (!GetWindow()->GetParent())
                    return wxAccStatus::NotImplemented;

                node = GetWindow()->GetParent()->GetChildren().Find(GetWindow());
            }
            else
            {
                if (fromId <= (int) GetWindow()->GetChildren().GetCount())
                    node = GetWindow()->GetChildren().Item(fromId-1);
            }

            if (node && node->GetNext())
            {
                wxWindow* nextWindow = node->GetNext()->GetData();
                *toObject = nextWindow->GetOrCreateAccessible();
                return wxAccStatus::Ok;
            }
            else
                return wxAccStatus::False;
        }
    case wxNavDir::Left:
    case wxNavDir::Up:
    case wxNavDir::Previous:
        {
            wxWindowList::compatibility_iterator node =
                wxWindowList::compatibility_iterator();
            if (fromId == wxACC_SELF)
            {
                // Can't navigate to sibling of this window
                // if we're a top-level window.
                if (!GetWindow()->GetParent())
                    return wxAccStatus::NotImplemented;

                node = GetWindow()->GetParent()->GetChildren().Find(GetWindow());
            }
            else
            {
                if (fromId <= (int) GetWindow()->GetChildren().GetCount())
                    node = GetWindow()->GetChildren().Item(fromId-1);
            }

            if (node && node->GetPrevious())
            {
                wxWindow* previousWindow = node->GetPrevious()->GetData();
                *toObject = previousWindow->GetOrCreateAccessible();
                return wxAccStatus::Ok;
            }
            else
                return wxAccStatus::False;
        }
    }

    return wxAccStatus::NotImplemented;
}

// Gets the name of the specified object.
wxAccStatus wxWindowAccessible::GetName(int childId, std::string* name)
{
    wxCHECK( GetWindow() != nullptr, wxAccStatus::Fail );

    std::string title;

    // If a child, leave wxWidgets to call the function on the actual
    // child object.
    if (childId > 0)
        return wxAccStatus::NotImplemented;

    // This will eventually be replaced by specialised
    // accessible classes, one for each kind of wxWidgets
    // control or window.
#if wxUSE_BUTTON
    if (dynamic_cast<wxButton*>(GetWindow()))
        title = ((wxButton*) GetWindow())->GetLabel();
    else
#endif
        title = GetWindow()->GetName();

    if (!title.empty())
    {
        *name = title;
        return wxAccStatus::Ok;
    }
    else
        return wxAccStatus::NotImplemented;
}

// Gets the number of children.
wxAccStatus wxWindowAccessible::GetChildCount(int* childId)
{
    wxCHECK( GetWindow() != nullptr, wxAccStatus::Fail );

    *childId = (int) GetWindow()->GetChildren().GetCount();
    return wxAccStatus::Ok;
}

// Gets the specified child (starting from 1).
// If *child is NULL and return value is wxAccStatus::Ok,
// this means that the child is a simple element and
// not an accessible object.
wxAccStatus wxWindowAccessible::GetChild(int childId, wxAccessible** child)
{
    wxCHECK( GetWindow() != nullptr, wxAccStatus::Fail );

    if (childId == wxACC_SELF)
    {
        *child = this;
        return wxAccStatus::Ok;
    }

    if (childId > (int) GetWindow()->GetChildren().GetCount())
        return wxAccStatus::Fail;

    wxWindow* childWindow = GetWindow()->GetChildren().Item(childId-1)->GetData();
    *child = childWindow->GetOrCreateAccessible();
    if (*child)
        return wxAccStatus::Ok;
    else
        return wxAccStatus::Fail;
}

// Gets the parent, or NULL.
wxAccStatus wxWindowAccessible::GetParent(wxAccessible** parent)
{
    wxCHECK( GetWindow() != nullptr, wxAccStatus::Fail );

    wxWindow* parentWindow = GetWindow()->GetParent();
    if (!parentWindow)
    {
        *parent = nullptr;
        return wxAccStatus::Ok;
    }
    else
    {
        *parent = parentWindow->GetOrCreateAccessible();
        if (*parent)
            return wxAccStatus::Ok;
        else
            return wxAccStatus::Fail;
    }
}

// Performs the default action. childId is 0 (the action for this object)
// or > 0 (the action for a child).
// Return wxAccStatus::NotSupported if there is no default action for this
// window (e.g. an edit control).
wxAccStatus wxWindowAccessible::DoDefaultAction([[maybe_unused]] int childId)
{
    wxCHECK( GetWindow() != nullptr, wxAccStatus::Fail );

    return wxAccStatus::NotImplemented;
}

// Gets the default action for this object (0) or > 0 (the action for a child).
// Return wxAccStatus::Ok even if there is no action. actionName is the action, or the empty
// string if there is no action.
// The retrieved string describes the action that is performed on an object,
// not what the object does as a result. For example, a toolbar button that prints
// a document has a default action of "Press" rather than "Prints the current document."
wxAccStatus wxWindowAccessible::GetDefaultAction([[maybe_unused]] int childId, [[maybe_unused]] std::string* actionName)
{
    wxCHECK( GetWindow() != nullptr, wxAccStatus::Fail );

    return wxAccStatus::NotImplemented;
}

// Returns the description for this object or a child.
wxAccStatus wxWindowAccessible::GetDescription([[maybe_unused]] int childId, std::string* description)
{
    wxCHECK( GetWindow() != nullptr, wxAccStatus::Fail );

    // FIXME: Make this more optional.
    std::ignore = description;
#if wxUSE_HELP
    std::string ht(GetWindow()->GetHelpTextAtPoint(wxDefaultPosition, wxHelpEvent::Origin::Keyboard));
    if (!ht.empty())
    {
        *description = ht;
        return wxAccStatus::Ok;
    }
#endif // wxUSE_HELP

    return wxAccStatus::NotImplemented;
}

// Returns help text for this object or a child, similar to tooltip text.
// FIXME: Change this to be more optional.
wxAccStatus wxWindowAccessible::GetHelpText([[maybe_unused]] int childId, std::string* helpText)
{
    wxCHECK( GetWindow() != nullptr, wxAccStatus::Fail );

    std::ignore = helpText;
#if wxUSE_HELP
    std::string ht(GetWindow()->GetHelpTextAtPoint(wxDefaultPosition, wxHelpEvent::Origin::Keyboard));
    if (!ht.empty())
    {
        *helpText = ht;
        return wxAccStatus::Ok;
    }
#endif // wxUSE_HELP

    return wxAccStatus::NotImplemented;
}

// Returns the keyboard shortcut for this object or child.
// Return e.g. ALT+K
wxAccStatus wxWindowAccessible::GetKeyboardShortcut([[maybe_unused]] int childId, [[maybe_unused]] std::string* shortcut)
{
    wxCHECK( GetWindow() != nullptr, wxAccStatus::Fail );

    return wxAccStatus::NotImplemented;
}

// Returns a role constant.
wxAccStatus wxWindowAccessible::GetRole(int childId, wxAccSystemRole* role)
{
    wxCHECK( GetWindow() != nullptr, wxAccStatus::Fail );

    // If a child, leave wxWidgets to call the function on the actual
    // child object.
    if (childId > 0)
        return wxAccStatus::NotImplemented;

    if (dynamic_cast<wxControl*>(GetWindow()))
        return wxAccStatus::NotImplemented;
#if wxUSE_STATUSBAR
    if (dynamic_cast<wxStatusBar*>(GetWindow()))
        return wxAccStatus::NotImplemented;
#endif
#if wxUSE_TOOLBAR
    if (dynamic_cast<wxToolBar*>(GetWindow()))
        return wxAccStatus::NotImplemented;
#endif

    //*role = wxAccSystemRole::Client;
    *role = wxAccSystemRole::Client;
    return wxAccStatus::Ok;

    #if 0
    return wxAccStatus::NotImplemented;
    #endif
}

// Returns a state constant.
wxAccStatus wxWindowAccessible::GetState(int childId, unsigned int* state)
{
    wxCHECK( GetWindow() != nullptr, wxAccStatus::Fail );

    // If a child, leave wxWidgets to call the function on the actual
    // child object.
    if (childId > 0)
        return wxAccStatus::NotImplemented;

    if (dynamic_cast<wxControl*>(GetWindow()))
        return wxAccStatus::NotImplemented;

#if wxUSE_STATUSBAR
    if (dynamic_cast<wxStatusBar*>(GetWindow()))
        return wxAccStatus::NotImplemented;
#endif
#if wxUSE_TOOLBAR
    if (dynamic_cast<wxToolBar*>(GetWindow()))
        return wxAccStatus::NotImplemented;
#endif

    *state = 0;
    return wxAccStatus::Ok;

    #if 0
    return wxAccStatus::NotImplemented;
    #endif
}

// Returns a localized string representing the value for the object
// or child.
wxAccStatus wxWindowAccessible::GetValue([[maybe_unused]] int childId, [[maybe_unused]] std::string* strValue)
{
    wxCHECK( GetWindow() != nullptr, wxAccStatus::Fail );

    return wxAccStatus::NotImplemented;
}

// Selects the object or child.
wxAccStatus wxWindowAccessible::Select([[maybe_unused]] int childId, [[maybe_unused]] wxAccSelectionFlags selectFlags)
{
    wxCHECK( GetWindow() != nullptr, wxAccStatus::Fail );

    return wxAccStatus::NotImplemented;
}

// Gets the window with the keyboard focus.
// If childId is 0 and child is NULL, no object in
// this subhierarchy has the focus.
// If this object has the focus, child should be 'this'.
wxAccStatus wxWindowAccessible::GetFocus([[maybe_unused]] int* childId, [[maybe_unused]] wxAccessible** child)
{
    wxCHECK( GetWindow() != nullptr, wxAccStatus::Fail );

    return wxAccStatus::NotImplemented;
}

#if wxUSE_VARIANT
// Gets a variant representing the selected children
// of this object.
// Acceptable values:
// - a null variant (IsNull() returns true)
// - a list variant (GetType() == "list"
// - an integer representing the selected child element,
//   or 0 if this object is selected (GetType() == "long"
// - a "void*" pointer to a wxAccessible child object
wxAccStatus wxWindowAccessible::GetSelections([[maybe_unused]] wxVariant* selections)
{
    wxCHECK( GetWindow() != nullptr, wxAccStatus::Fail );

    return wxAccStatus::NotImplemented;
}
#endif // wxUSE_VARIANT

#endif // wxUSE_ACCESSIBILITY

// ----------------------------------------------------------------------------
// RTL support
// ----------------------------------------------------------------------------

wxCoord
wxWindowBase::AdjustForLayoutDirection(wxCoord x,
                                       wxCoord width,
                                       wxCoord widthTotal) const
{
    if ( GetLayoutDirection() == wxLayoutDirection::RightToLeft )
    {
        x = widthTotal - x - width;
    }

    return x;
}


