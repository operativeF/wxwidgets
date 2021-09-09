///////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/tipwin.cpp
// Purpose:     implementation of wxTipWindow
// Author:      Vadim Zeitlin
// Modified by:
// Created:     10.09.00
// Copyright:   (c) 2000 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"


#if wxUSE_TIPWINDOW

#include "wx/tipwin.h"

#ifndef WX_PRECOMP
    #include <vector>

    #include <gsl/gsl>
#endif // WX_PRECOMP

#include "wx/dcclient.h"
#include "wx/timer.h"
#include "wx/settings.h"

#include "wx/display.h"


// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

constexpr wxCoord TEXT_MARGIN_X = 3;
constexpr wxCoord TEXT_MARGIN_Y = 3;

// ----------------------------------------------------------------------------
// wxTipWindowView
// ----------------------------------------------------------------------------

// Viewer window to put in the frame
class WXDLLEXPORT wxTipWindowView : public wxWindow
{
public:
    explicit wxTipWindowView(wxWindow *parent);

    wxTipWindowView(const wxTipWindowView&) = delete;
	wxTipWindowView& operator=(const wxTipWindowView&) = delete;

    // event handlers
    void OnPaint(wxPaintEvent& event);
    void OnMouseClick(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);

    // calculate the client rect we need to display the text
    void Adjust(std::string_view text, wxCoord maxLength);

private:
    wxTipWindow* m_parent;

    std::vector<std::string> m_textLines;
    wxCoord m_heightLine{0};


    wxDECLARE_EVENT_TABLE();
};

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// event tables
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(wxTipWindow, wxPopupTransientWindow)
    EVT_LEFT_DOWN(wxTipWindow::OnMouseClick)
    EVT_RIGHT_DOWN(wxTipWindow::OnMouseClick)
    EVT_MIDDLE_DOWN(wxTipWindow::OnMouseClick)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(wxTipWindowView, wxWindow)
    EVT_PAINT(wxTipWindowView::OnPaint)

    EVT_LEFT_DOWN(wxTipWindowView::OnMouseClick)
    EVT_RIGHT_DOWN(wxTipWindowView::OnMouseClick)
    EVT_MIDDLE_DOWN(wxTipWindowView::OnMouseClick)

    EVT_MOTION(wxTipWindowView::OnMouseMove)
wxEND_EVENT_TABLE()

// ----------------------------------------------------------------------------
// wxTipWindow
// ----------------------------------------------------------------------------

wxTipWindow::wxTipWindow(wxWindow *parent,
                         const std::string& text,
                         wxCoord maxLength,
                         wxTipWindow** windowPtr,
                         wxRect *rectBounds)
           : wxPopupTransientWindow(parent)
{
    SetTipWindowPtr(windowPtr);
    if ( rectBounds )
    {
        SetBoundingRect(*rectBounds);
    }

    // set colours
    SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));

    int x, y;
    wxGetMousePosition(&x, &y);

    // move to the center of the target display so wxTipWindowView will use the
    // correct DPI
    wxPoint posScreen;
    wxSize sizeScreen;

    const int displayNum = wxDisplay::GetFromPoint(wxPoint(x, y));
    if ( displayNum != wxNOT_FOUND )
    {
        const wxRect rectScreen = wxDisplay(displayNum).GetGeometry();
        posScreen = rectScreen.GetPosition();
        sizeScreen = rectScreen.GetSize();
    }
    else // outside of any display?
    {
        // just use the primary one then
        posScreen = wxPoint(0, 0);
        sizeScreen = wxGetDisplaySize();
    }
    wxPoint center(posScreen.x + sizeScreen.x / 2,
                   posScreen.y + sizeScreen.y / 2);
    Move(center, wxSIZE_NO_ADJUSTMENTS);

    // set size, position and show it
    m_view = new wxTipWindowView(this);
    m_view->Adjust(text, FromDIP(parent->ToDIP(maxLength)) );

    // we want to show the tip below the mouse, not over it, make sure to not
    // overflow into the next display
    //
    // NB: the reason we use "/ 2" here is that we don't know where the current
    //     cursors hot spot is... it would be nice if we could find this out
    //     though
    int cursorOffset = wxSystemSettings::GetMetric(wxSYS_CURSOR_Y, this) / 2;
    if (y + cursorOffset >= posScreen.y + sizeScreen.y)
        cursorOffset = posScreen.y + sizeScreen.y - y - 1;
    y += cursorOffset;

    Position(wxPoint(x, y), wxSize(0,0));
    Popup(m_view);
    #ifdef __WXGTK__
        m_view->CaptureMouse();
    #endif
}

wxTipWindow::~wxTipWindow()
{
    if ( m_windowPtr )
    {
        *m_windowPtr = nullptr;
    }
    #ifdef __WXGTK__
        if ( m_view->HasCapture() )
            m_view->ReleaseMouse();
    #endif
}

void wxTipWindow::OnMouseClick(wxMouseEvent& WXUNUSED(event))
{
    Close();
}

void wxTipWindow::OnDismiss()
{
    Close();
}

void wxTipWindow::SetBoundingRect(const wxRect& rectBound)
{
    m_rectBound = rectBound;
}

void wxTipWindow::Close()
{
    if ( m_windowPtr )
    {
        *m_windowPtr = nullptr;
        m_windowPtr = nullptr;
    }

    Show(false);
    #ifdef __WXGTK__
        if ( m_view->HasCapture() )
            m_view->ReleaseMouse();
    #endif
    // Under OS X we get destroyed because of wxEVT_KILL_FOCUS generated by
    // Show(false).
    #ifndef __WXOSX__
        Destroy();
    #endif
}

// ----------------------------------------------------------------------------
// wxTipWindowView
// ----------------------------------------------------------------------------

wxTipWindowView::wxTipWindowView(wxWindow *parent)
    : wxWindow(parent, wxID_ANY,
                wxDefaultPosition, wxDefaultSize,
                wxNO_BORDER),
      m_parent((wxTipWindow*)parent)
{
    // set colours
    SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));
}

void wxTipWindowView::Adjust(std::string_view text, wxCoord maxLength)
{
    wxClientDC dc(this);
    dc.SetFont(GetFont());

    // calculate the length: we want each line be no longer than maxLength
    // pixels and we only break lines at words boundary
    std::string current;

    wxCoord height{0};
    wxCoord width{0};
    wxCoord widthMax{0};

    bool breakLine = false;
    for ( const auto& ch : text )
    {
        if ( ch == '\n' || ch == '\0' )
        {
            dc.GetTextExtent(current, &width, &height);
            if ( width > widthMax )
                widthMax = width;

            if ( height > m_heightLine )
                m_heightLine = height;

            m_textLines.push_back(current);

            current.clear();
            breakLine = false;
        }
        else if ( breakLine && (ch == ' ' || ch == '\t') )
        {
            // word boundary - break the line here
            m_textLines.push_back(current);
            current.clear();
            breakLine = false;
        }
        else // line goes on
        {
            current += ch;
            dc.GetTextExtent(current, &width, &height);
            if ( width > maxLength )
                breakLine = true;

            if ( width > widthMax )
                widthMax = width;

            if ( height > m_heightLine )
                m_heightLine = height;
        }
    }

    // take into account the border size and the margins
    width  = 2*(TEXT_MARGIN_X + 1) + widthMax;
    height = 2*(TEXT_MARGIN_Y + 1) + gsl::narrow<wxCoord>(m_textLines.size()) * m_heightLine;
    m_parent->SetClientSize(width, height);
    SetSize(0, 0, width, height);
}

void wxTipWindowView::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);

    wxRect rect;
    const wxSize size = GetClientSize();
    rect.width = size.x;
    rect.height = size.y;

    // first filll the background
    dc.SetBrush(wxBrush(GetBackgroundColour(), wxBrushStyle::Solid));
    dc.SetPen(wxPen(GetForegroundColour(), 1, wxPenStyle::Solid));
    dc.DrawRectangle(rect);

    // and then draw the text line by line
    dc.SetTextBackground(GetBackgroundColour());
    dc.SetTextForeground(GetForegroundColour());
    dc.SetFont(GetFont());

    wxPoint pt;
    pt.x = TEXT_MARGIN_X;
    pt.y = TEXT_MARGIN_Y;

    for ( std::string_view line : m_textLines )
    {
        dc.DrawText(line, pt);

        pt.y += m_heightLine;
    }
}

void wxTipWindowView::OnMouseClick(wxMouseEvent& WXUNUSED(event))
{
    m_parent->Close();
}

void wxTipWindowView::OnMouseMove(wxMouseEvent& event)
{
    const wxRect& rectBound = m_parent->m_rectBound;

    if ( rectBound.width &&
            !rectBound.Contains(ClientToScreen(event.GetPosition())) )
    {
        // mouse left the bounding rect, disappear
        m_parent->Close();
    }
    else
    {
        event.Skip();
    }
}

#endif // wxUSE_TIPWINDOW
