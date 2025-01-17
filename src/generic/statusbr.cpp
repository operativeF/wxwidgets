/////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/statusbr.cpp
// Purpose:     wxStatusBarGeneric class implementation
// Author:      Julian Smart
// Modified by: Francesco Montorsi
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_STATUSBAR

#include "wx/statusbr.h"
#include "wx/dcclient.h"
#include "wx/toplevel.h"
#include "wx/control.h"

#ifdef __WXGTK20__
    #include "wx/gtk/private.h"
#endif

import WX.Utils.Settings;

// we only have to do it here when we use wxStatusBarGeneric in addition to the
// standard wxStatusBar class, if wxStatusBarGeneric is the same as
// wxStatusBar, then the corresponding wxIMPLEMENT_DYNAMIC_CLASS is already in
// common/statbar.cpp
#if defined(__WXMAC__) || \
    (defined(wxUSE_NATIVE_STATUSBAR) && wxUSE_NATIVE_STATUSBAR)
    #include "wx/generic/statusbr.h"
#endif // wxUSE_NATIVE_STATUSBAR

import <numeric>;

// Default status border dimensions
constexpr int wxTHICK_LINE_BORDER = 2;

// Margin between the field text and the field rect
constexpr int wxFIELD_TEXT_MARGIN = 2;

// ----------------------------------------------------------------------------
// GTK+ signal handler
// ----------------------------------------------------------------------------

#if defined( __WXGTK20__ )
#if GTK_CHECK_VERSION(2,12,0)
extern "C" {
static
gboolean statusbar_query_tooltip([[maybe_unused]] GtkWidget*  widget,
                                 gint        x,
                                 gint        y,
                                 [[maybe_unused]] gboolean    keyboard_mode,
                                 GtkTooltip *tooltip,
                                 wxStatusBar* statbar)
{
    int n = statbar->GetFieldFromPoint(wxPoint(x,y));
    if (n == wxNOT_FOUND)
        return FALSE;

    // should we show the tooltip for the n-th pane of the statusbar?
    if (!statbar->GetField(n).IsEllipsized())
        return FALSE;   // no, it's not useful

    const std::string& str = statbar->GetStatusText(n);
    if (str.empty())
        return FALSE;

    gtk_tooltip_set_text(tooltip, wxGTK_CONV_SYS(str));
    return TRUE;
}
}
#endif
#endif

// ----------------------------------------------------------------------------
// wxStatusBarGeneric
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(wxStatusBarGeneric, wxWindow)
    EVT_PAINT(wxStatusBarGeneric::OnPaint)
    EVT_SIZE(wxStatusBarGeneric::OnSize)
#ifdef __WXGTK20__
    EVT_LEFT_DOWN(wxStatusBarGeneric::OnLeftDown)
    EVT_RIGHT_DOWN(wxStatusBarGeneric::OnRightDown)
#endif
    EVT_SYS_COLOUR_CHANGED(wxStatusBarGeneric::OnSysColourChanged)
wxEND_EVENT_TABLE()

void wxStatusBarGeneric::Init()
{
    m_borderX = wxTHICK_LINE_BORDER;
    m_borderY = wxTHICK_LINE_BORDER;
}

bool wxStatusBarGeneric::Create(wxWindow *parent,
                                wxWindowID id,
                                unsigned int style,
                                std::string_view name)
{
    style |= wxTAB_TRAVERSAL | wxFULL_REPAINT_ON_RESIZE;
    if ( !wxWindow::Create(parent, id,
                           wxDefaultPosition, wxDefaultSize,
                           style, name) )
        return false;

    // The status bar should have a themed background
    SetThemeEnabled( true );

    InitColours();

    int height = (int)((11*GetCharHeight())/10 + 2*GetBorderY());
    SetSize(wxRect{wxDefaultCoord, wxDefaultCoord, wxDefaultCoord, height});

    SetFieldsCount(1);

#if defined( __WXGTK20__ )
#if GTK_CHECK_VERSION(2,12,0)
    if (HasFlag(wxSTB_SHOW_TIPS) && wx_is_at_least_gtk2(12))
    {
        g_object_set(m_widget, "has-tooltip", TRUE, NULL);
        g_signal_connect(m_widget, "query-tooltip",
                         G_CALLBACK(statusbar_query_tooltip), this);
    }
#endif
#endif

    return true;
}

wxSize wxStatusBarGeneric::DoGetBestSize() const
{
    wxSize best_size;

    // best width is the width of the parent
    if (GetParent())
        best_size.x = GetParent()->GetClientSize().x;
    else
        best_size.x = 80;     // a dummy value

    // best height is as calculated above in Create()
    best_size.y = (int)((11*GetCharHeight())/10 + 2*GetBorderY());

    return best_size;
}

void wxStatusBarGeneric::DoUpdateStatusText(int number)
{
    wxRect rect;
    GetFieldRect(number, rect);

    Refresh(true, &rect);

    // it's common to show some text in the status bar before starting a
    // relatively lengthy operation, ensure that the text is shown to the
    // user immediately and not after the lengthy operation end
    Update();
}

void wxStatusBarGeneric::SetStatusWidths(int n, const int widths_field[])
{
    // only set status widths when n == number of statuswindows
    wxCHECK_RET( (size_t)n == m_panes.size(), "status bar field count mismatch" );

    wxStatusBarBase::SetStatusWidths(n, widths_field);

    // update cache
    DoUpdateFieldWidths();
}

void wxStatusBarGeneric::DoUpdateFieldWidths()
{
    m_lastClientSize = GetClientSize();

    int width = m_lastClientSize.x;
    if ( ShowsSizeGrip() )
        width -= GetSizeGripRect().width;

    // recompute the cache of the field widths if the status bar width has changed
    m_widthsAbs = CalculateAbsWidths(width);
}

bool wxStatusBarGeneric::ShowsSizeGrip() const
{
    // Currently drawing size grip is implemented only in wxGTK.
#ifdef __WXGTK20__
    if ( !HasFlag(wxSTB_SIZEGRIP) )
        return false;

    wxTopLevelWindow * const
        tlw = dynamic_cast<wxTopLevelWindow*>(wxGetTopLevelParent(GetParent()));
    return tlw && !tlw->wxIsMaximized() && tlw->HasFlag(wxRESIZE_BORDER);
#else // !__WXGTK20__
    return false;
#endif // __WXGTK20__/!__WXGTK20__
}

void wxStatusBarGeneric::DrawFieldText(wxDC& dc, const wxRect& rect, int i, int textHeight)
{
    std::string text(GetStatusText(i));
    if (text.empty())
        return;     // optimization

    int xpos = rect.x + wxFIELD_TEXT_MARGIN,
        maxWidth = rect.width - 2*wxFIELD_TEXT_MARGIN,
        ypos = (int) (((rect.height - textHeight) / 2) + rect.y + 0.5);

    if (ShowsSizeGrip())
    {
        // don't write text over the size grip:
        // NOTE: overloading DoGetClientSize() and GetClientAreaOrigin() wouldn't
        //       work because the adjustment needs to be done only when drawing
        //       the field text and not also when drawing the background, the
        //       size grip itself, etc
        if ((GetLayoutDirection() == wxLayoutDirection::RightToLeft && i == 0) ||
            (GetLayoutDirection() != wxLayoutDirection::RightToLeft &&
                 i == (int)m_panes.size()-1))
        {
            const wxRect& gripRc = GetSizeGripRect();

            // NOTE: we don't need any special treatment wrt to the layout direction
            //       since wxDrawText() will automatically adjust the origin of the
            //       text accordingly to the layout in use

            maxWidth -= gripRc.width;
        }
    }

    // eventually ellipsize the text so that it fits the field width

    wxEllipsizeMode ellmode = wxEllipsizeMode::None;
    if (HasFlag(wxSTB_ELLIPSIZE_START)) ellmode = wxEllipsizeMode::Start;
    else if (HasFlag(wxSTB_ELLIPSIZE_MIDDLE)) ellmode = wxEllipsizeMode::Middle;
    else if (HasFlag(wxSTB_ELLIPSIZE_END)) ellmode = wxEllipsizeMode::End;

    if (ellmode == wxEllipsizeMode::None)
    {
        // if we have the wxSTB_SHOW_TIPS we must set the ellipsized flag even if
        // we don't ellipsize the text but just truncate it
        if (HasFlag(wxSTB_SHOW_TIPS))
            SetEllipsizedFlag(i, dc.GetTextExtent(text).x > maxWidth);

        dc.SetClippingRegion(rect);
    }
    else
    {
        text = wxControl::Ellipsize(text, dc,
                                    ellmode,
                                    maxWidth,
                                    wxEllipsizeFlags::ExpandTabs);
            // Ellipsize() will do something only if necessary

        // update the ellipsization status for this pane; this is used later to
        // decide whether a tooltip should be shown or not for this pane
        // (if we have wxSTB_SHOW_TIPS)
        SetEllipsizedFlag(i, text != GetStatusText(i));
    }

#if defined( __WXGTK__ )
    xpos++;
    ypos++;
#elif defined(__WXMAC__)
    xpos++;
#endif

    // draw the text
    dc.wxDrawText(text, wxPoint{xpos, ypos});

    if (ellmode == wxEllipsizeMode::None)
        dc.DestroyClippingRegion();
}

void wxStatusBarGeneric::DrawField(wxDC& dc, int i, int textHeight)
{
    wxRect rect;
    GetFieldRect(i, rect);

    if (rect.GetWidth() <= 0)
        return;     // happens when the status bar is shrunk in a very small area!

    int style = GetEffectiveFieldStyle(i);
    if (style == wxSB_RAISED || style == wxSB_SUNKEN)
    {
        // Draw border
        // For wxSB_SUNKEN: paint a grey background, plus 3-d border (one black rectangle)
        // Inside this, left and top sides (dark grey). Bottom and right (white).
        // Reverse it for wxSB_RAISED

        dc.SetPen((style == wxSB_RAISED) ? m_mediumShadowPen : m_hilightPen);

        // Right and bottom lines
        dc.DrawLine(rect.x + rect.width, rect.y,
                    rect.x + rect.width, rect.y + rect.height);
        dc.DrawLine(rect.x + rect.width, rect.y + rect.height,
                    rect.x, rect.y + rect.height);

        dc.SetPen((style == wxSB_RAISED) ? m_hilightPen : m_mediumShadowPen);

        // Left and top lines
        dc.DrawLine(rect.x, rect.y + rect.height,
               rect.x, rect.y);
        dc.DrawLine(rect.x, rect.y,
            rect.x + rect.width, rect.y);
    }

    DrawFieldText(dc, rect, i, textHeight);
}

bool wxStatusBarGeneric::GetFieldRect(int n, wxRect& rect) const
{
    wxCHECK_MSG( (n >= 0) && ((size_t)n < m_panes.size()), false,
                 "invalid status bar field index" );

    // We can be called from the user-defined EVT_SIZE handler in which case
    // the widths haven't been updated yet and we need to do it now. This is
    // not very efficient as we keep testing the size but there is no other way
    // to make the code needing the up-to-date fields sizes in its EVT_SIZE to
    // work.
    if ( GetClientSize().x != m_lastClientSize.x )
    {
        const_cast<wxStatusBarGeneric*>(this)->DoUpdateFieldWidths();
    }

    if (m_widthsAbs.empty())
        return false;

    rect.x = std::reduce(m_widthsAbs.begin(), m_widthsAbs.end());

    rect.x += m_borderX;

    rect.y = m_borderY;
    rect.width = m_widthsAbs[n] - 2*m_borderX;
    rect.height = m_lastClientSize.y - 2*m_borderY;

    return true;
}

int wxStatusBarGeneric::GetFieldFromPoint(const wxPoint& pt) const
{
    if (m_widthsAbs.empty())
        return wxNOT_FOUND;

    // NOTE: we explicitly don't take in count the borders since they are only
    //       useful when rendering the status text, not for hit-test computations

    if (pt.y <= 0 || pt.y >= m_lastClientSize.y)
        return wxNOT_FOUND;

    int x = 0;
    for ( size_t i = 0; i < m_panes.size(); i++ )
    {
        if (pt.x > x && pt.x < x+m_widthsAbs[i])
            return i;

        x += m_widthsAbs[i];
    }

    return wxNOT_FOUND;
}

void wxStatusBarGeneric::InitColours()
{
    m_mediumShadowPen = wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW));
    m_hilightPen = wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DHILIGHT));
}

void wxStatusBarGeneric::SetMinHeight(int height)
{
    // check that this min height is not less than minimal height for the
    // current font (min height is as calculated above in Create() except for border)
    int minHeight = (int)((11*GetCharHeight())/10);

    if ( height > minHeight )
        SetSize(wxRect{wxDefaultCoord, wxDefaultCoord, wxDefaultCoord, height + 2*m_borderY});
}

wxRect wxStatusBarGeneric::GetSizeGripRect() const
{
    wxSize client_size = wxWindow::DoGetClientSize();

#ifndef __WXGTK3__
    if (GetLayoutDirection() == wxLayoutDirection::RightToLeft)
        return {2, 2, client_size.y - 2, client_size.y - 4};
#endif

        return {client_size.x - client_size.y - 2, 2, client_size.y - 2, client_size.y - 4};
}

// ----------------------------------------------------------------------------
// wxStatusBarGeneric - event handlers
// ----------------------------------------------------------------------------

void wxStatusBarGeneric::OnPaint([[maybe_unused]] wxPaintEvent& event )
{
    wxPaintDC dc(this);

#ifdef __WXGTK20__
    // Draw grip first
    if ( ShowsSizeGrip() )
    {
        const wxRect& rc = GetSizeGripRect();
#ifdef __WXGTK3__
        GtkWidget* toplevel = gtk_widget_get_toplevel(m_widget);
        GdkRectangle rect;
        if (toplevel && (!gtk_window_get_resize_grip_area(GTK_WINDOW(toplevel), &rect) ||
            rect.width == 0 || rect.height == 0))
        {
            GtkStyleContext* sc = gtk_widget_get_style_context(toplevel);
            gtk_style_context_save(sc);
            gtk_style_context_add_class(sc, GTK_STYLE_CLASS_GRIP);
            gtk_render_handle(sc,
                static_cast<cairo_t*>(dc.GetImpl()->GetCairoContext()),
                rc.x, rc.y, rc.width, rc.height);
            gtk_style_context_restore(sc);
        }
#else
        GdkWindowEdge edge =
            GetLayoutDirection() == wxLayoutDirection::RightToLeft ? GDK_WINDOW_EDGE_SOUTH_WEST :
                                                           GDK_WINDOW_EDGE_SOUTH_EAST;
        gtk_paint_resize_grip(gtk_widget_get_style(m_widget),
                            GTKGetDrawingWindow(),
                            gtk_widget_get_state(m_widget),
                            NULL,
                            m_widget,
                            "statusbar",
                            edge,
                            rc.x, rc.y, rc.width, rc.height );
#endif
    }
#endif // __WXGTK20__

    if (GetFont().IsOk())
        dc.SetFont(GetFont());

    // compute char height only once for all panes:
    int textHeight = dc.GetCharHeight();

    dc.SetBackgroundMode(wxBrushStyle::Transparent);
    for (size_t i = 0; i < m_panes.size(); i ++)
        DrawField(dc, i, textHeight);
}

void wxStatusBarGeneric::OnSysColourChanged(wxSysColourChangedEvent& event)
{
    InitColours();

    // Propagate the event to the non-top-level children
    wxWindow::OnSysColourChanged(event);
}

#ifdef __WXGTK20__
void wxStatusBarGeneric::OnLeftDown(wxMouseEvent& event)
{
    wxSize cli_size = GetClientSize();

    GtkWidget* ancestor = gtk_widget_get_toplevel(m_widget);
#ifdef __WXGTK3__
    GdkRectangle rect;
    if (ancestor && gtk_window_get_resize_grip_area(GTK_WINDOW(ancestor), &rect) &&
        rect.width && rect.height)
    {
        ancestor = NULL;
    }
#endif

    if (ancestor && ShowsSizeGrip() && event.GetX() > cli_size.x - cli_size.y)
    {
        GdkWindow *source = GTKGetDrawingWindow();

        int org_x = 0;
        int org_y = 0;
        gdk_window_get_origin( source, &org_x, &org_y );

        if (GetLayoutDirection() == wxLayoutDirection::RightToLeft)
        {
            gtk_window_begin_resize_drag (GTK_WINDOW (ancestor),
                                  GDK_WINDOW_EDGE_SOUTH_WEST,
                                  1,
                                  org_x - event.GetX() + GetSize().x ,
                                  org_y + event.GetY(),
                                  0);
        }
        else
        {
            gtk_window_begin_resize_drag (GTK_WINDOW (ancestor),
                                  GDK_WINDOW_EDGE_SOUTH_EAST,
                                  1,
                                  org_x + event.GetX(),
                                  org_y + event.GetY(),
                                  0);
        }
    }
    else
    {
        event.Skip( true );
    }
}

void wxStatusBarGeneric::OnRightDown(wxMouseEvent& event)
{
    wxSize cli_size = GetClientSize();

    GtkWidget* ancestor = gtk_widget_get_toplevel(m_widget);
#ifdef __WXGTK3__
    GdkRectangle rect;
    if (ancestor && gtk_window_get_resize_grip_area(GTK_WINDOW(ancestor), &rect) &&
        rect.width && rect.height)
    {
        ancestor = NULL;
    }
#endif

    if (ancestor && ShowsSizeGrip() && event.GetX() > cli_size.x - cli_size.y)
    {
        GdkWindow *source = GTKGetDrawingWindow();

        int org_x = 0;
        int org_y = 0;
        gdk_window_get_origin( source, &org_x, &org_y );

        gtk_window_begin_move_drag (GTK_WINDOW (ancestor),
                                2,
                                org_x + event.GetX(),
                                org_y + event.GetY(),
                                0);
    }
    else
    {
        event.Skip( true );
    }
}
#endif // __WXGTK20__

void wxStatusBarGeneric::OnSize(wxSizeEvent& event)
{
    DoUpdateFieldWidths();

    event.Skip();
}

#endif // wxUSE_STATUSBAR
