module;

#include "wx/colour.h"
#include "wx/gdicmn.h"
#include "wx/window.h"

#include <memory>

module WX.AUI.DockArt;

import <cmath>;
import <string>;

// -- wxAuiDefaultDockArt class implementation --

// wxAuiDefaultDockArt is an art provider class which does all of the drawing for
// wxAuiManager.  This allows the library caller to customize the dock art
// (probably by deriving from this class), or to completely replace all drawing
// with custom dock art (probably by writing a new stand-alone class derived
// from the wxAuiDockArt base class). The active dock art class can be set via
// wxAuiManager::SetDockArt()
wxAuiDefaultDockArt::wxAuiDefaultDockArt()
{
    UpdateColoursFromSystem();

#ifdef __WXMAC__
    m_captionFont = *wxSMALL_FONT;
#else
    m_captionFont = wxFont(8, wxFontFamily::Default, wxFontStyle::Normal, wxFONTWEIGHT_NORMAL);
#endif

    // default metric values
#if defined( __WXMAC__ ) && wxOSX_USE_COCOA_OR_CARBON
    SInt32 height;
    GetThemeMetric( kThemeMetricSmallPaneSplitterHeight , &height );
    m_sashSize     = height;
#elif defined(__WXGTK__)
    m_sashSize     = wxRendererNative::Get().GetSplitterParams(NULL).widthSash;
#else
    m_sashSize     = wxWindow::FromDIP( 4, nullptr);
#endif
    m_captionSize  = wxWindow::FromDIP(17, nullptr);
    m_borderSize   = 1;
    m_buttonSize   = wxWindow::FromDIP(14, nullptr);
    m_gripperSize  = wxWindow::FromDIP( 9, nullptr);
    m_gradientType = wxAUI_GRADIENT_VERTICAL;

    InitBitmaps();
}

std::unique_ptr<wxAuiDockArt> wxAuiDefaultDockArt::Clone()
{
    return std::make_unique<wxAuiDefaultDockArt>(*this);
}

void
wxAuiDefaultDockArt::InitBitmaps()
{
    // some built in bitmaps
    // TODO: Provide x1.5 and x2.0 versions or migrate to SVG.
#if defined( __WXMAC__ )
     static constexpr unsigned char close_bits[]={
         0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0xFE, 0x03, 0xF8, 0x01, 0xF0, 0x19, 0xF3,
         0xB8, 0xE3, 0xF0, 0xE1, 0xE0, 0xE0, 0xF0, 0xE1, 0xB8, 0xE3, 0x19, 0xF3,
         0x01, 0xF0, 0x03, 0xF8, 0x0F, 0xFE, 0xFF, 0xFF };
#elif defined(__WXGTK__)
     static constexpr unsigned char close_bits[]={
         0xff, 0xff, 0xff, 0xff, 0x07, 0xf0, 0xfb, 0xef, 0xdb, 0xed, 0x8b, 0xe8,
         0x1b, 0xec, 0x3b, 0xee, 0x1b, 0xec, 0x8b, 0xe8, 0xdb, 0xed, 0xfb, 0xef,
         0x07, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
#else
    static constexpr unsigned char close_bits[]={
         // reduced height, symmetric
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xcf, 0xf3, 0x9f, 0xf9,
         0x3f, 0xfc, 0x7f, 0xfe, 0x3f, 0xfc, 0x9f, 0xf9, 0xcf, 0xf3, 0xff, 0xff,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
      /*
         // same height as maximize/restore
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe7, 0xe7, 0xcf, 0xf3, 0x9f, 0xf9,
         0x3f, 0xfc, 0x7f, 0xfe, 0x3f, 0xfc, 0x9f, 0xf9, 0xcf, 0xf3, 0xe7, 0xe7,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
      */
#endif

    static constexpr unsigned char maximize_bits[] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0xf0, 0xf7, 0xf7, 0x07, 0xf0,
        0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0xf7, 0x07, 0xf0,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    static constexpr unsigned char restore_bits[]={
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0xf0, 0x1f, 0xf0, 0xdf, 0xf7,
        0x07, 0xf4, 0x07, 0xf4, 0xf7, 0xf5, 0xf7, 0xf1, 0xf7, 0xfd, 0xf7, 0xfd,
        0x07, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    static constexpr unsigned char pin_bits[]={
        0xff,0xff,0xff,0xff,0xff,0xff,0x1f,0xfc,0xdf,0xfc,0xdf,0xfc,
        0xdf,0xfc,0xdf,0xfc,0xdf,0xfc,0x0f,0xf8,0x7f,0xff,0x7f,0xff,
        0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

#ifdef __WXMAC__
    const wxColour inactive = wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVECAPTION);
    const wxColour active = wxSystemSettings::GetColour(wxSYS_COLOUR_CAPTIONTEXT);
#else
    const wxColor inactive = m_inactiveCaptionTextColour;
    const wxColor active = m_activeCaptionTextColour;
#endif

    m_inactiveCloseBitmap = wxAuiBitmapFromBits(close_bits, wxSize{16, 16}, inactive);
    m_activeCloseBitmap = wxAuiBitmapFromBits(close_bits, wxSize{16, 16}, active);

    m_inactiveMaximizeBitmap = wxAuiBitmapFromBits(maximize_bits, wxSize{16, 16}, inactive);
    m_activeMaximizeBitmap = wxAuiBitmapFromBits(maximize_bits, wxSize{16, 16}, active);

    m_inactiveRestoreBitmap = wxAuiBitmapFromBits(restore_bits, wxSize{16, 16}, inactive);
    m_activeRestoreBitmap = wxAuiBitmapFromBits(restore_bits, wxSize{16, 16}, active);

    m_inactivePinBitmap = wxAuiBitmapFromBits(pin_bits, wxSize{16, 16}, inactive);
    m_activePinBitmap = wxAuiBitmapFromBits(pin_bits, wxSize{16, 16}, active);
}

void wxAuiDefaultDockArt::UpdateColoursFromSystem()
{
    wxColor baseColour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);

    // the baseColour is too pale to use as our base colour,
    // so darken it a bit --
    if ((255-baseColour.Red()) +
        (255-baseColour.Green()) +
        (255-baseColour.Blue()) < 60)
    {
        baseColour = baseColour.ChangeLightness(92);
    }

    m_baseColour = baseColour;
    wxColor darker1Colour = baseColour.ChangeLightness(85);
    wxColor darker2Colour = baseColour.ChangeLightness(75);
    wxColor darker3Colour = baseColour.ChangeLightness(60);
    //wxColor darker4Colour = baseColour.ChangeLightness(50);
    wxColor darker5Colour = baseColour.ChangeLightness(40);

    m_activeCaptionColour = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
    m_activeCaptionGradientColour = wxAuiLightContrastColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
    m_activeCaptionTextColour = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);
    m_inactiveCaptionColour = darker1Colour;
    m_inactiveCaptionGradientColour = baseColour.ChangeLightness(97);
    m_inactiveCaptionTextColour = wxSystemSettings::GetColour(wxSYS_COLOUR_INACTIVECAPTIONTEXT);

    m_sashBrush = wxBrush(baseColour);
    m_backgroundBrush = wxBrush(baseColour);
    m_gripperBrush = wxBrush(baseColour);

    m_borderPen = wxPen(darker2Colour);
    int pen_width = wxWindow::FromDIP(1, nullptr);
    m_gripperPen1 = wxPen(darker5Colour, pen_width);
    m_gripperPen2 = wxPen(darker3Colour, pen_width);
    m_gripperPen3 = wxPen(*wxStockGDI::GetColour(wxStockGDI::COLOUR_WHITE), pen_width);
    InitBitmaps();
}

int wxAuiDefaultDockArt::GetMetric(int id)
{
    switch (id)
    {
        case wxAUI_DOCKART_SASH_SIZE:          return m_sashSize;
        case wxAUI_DOCKART_CAPTION_SIZE:       return m_captionSize;
        case wxAUI_DOCKART_GRIPPER_SIZE:       return m_gripperSize;
        case wxAUI_DOCKART_PANE_BORDER_SIZE:   return m_borderSize;
        case wxAUI_DOCKART_PANE_BUTTON_SIZE:   return m_buttonSize;
        case wxAUI_DOCKART_GRADIENT_TYPE:      return m_gradientType;
        default: wxFAIL_MSG("Invalid Metric Ordinal"); break;
    }

    return 0;
}

void wxAuiDefaultDockArt::SetMetric(int id, int new_val)
{
    switch (id)
    {
        case wxAUI_DOCKART_SASH_SIZE:          m_sashSize = new_val; break;
        case wxAUI_DOCKART_CAPTION_SIZE:       m_captionSize = new_val; break;
        case wxAUI_DOCKART_GRIPPER_SIZE:       m_gripperSize = new_val; break;
        case wxAUI_DOCKART_PANE_BORDER_SIZE:   m_borderSize = new_val; break;
        case wxAUI_DOCKART_PANE_BUTTON_SIZE:   m_buttonSize = new_val; break;
        case wxAUI_DOCKART_GRADIENT_TYPE:      m_gradientType = new_val; break;
        default: wxFAIL_MSG("Invalid Metric Ordinal"); break;
    }
}

wxColour wxAuiDefaultDockArt::GetColour(int id)
{
    switch (id)
    {
        case wxAUI_DOCKART_BACKGROUND_COLOUR:                return m_backgroundBrush.GetColour();
        case wxAUI_DOCKART_SASH_COLOUR:                      return m_sashBrush.GetColour();
        case wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR:          return m_inactiveCaptionColour;
        case wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR: return m_inactiveCaptionGradientColour;
        case wxAUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR:     return m_inactiveCaptionTextColour;
        case wxAUI_DOCKART_ACTIVE_CAPTION_COLOUR:            return m_activeCaptionColour;
        case wxAUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR:   return m_activeCaptionGradientColour;
        case wxAUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR:       return m_activeCaptionTextColour;
        case wxAUI_DOCKART_BORDER_COLOUR:                    return m_borderPen.GetColour();
        case wxAUI_DOCKART_GRIPPER_COLOUR:                   return m_gripperBrush.GetColour();
        default: wxFAIL_MSG("Invalid Metric Ordinal"); break;
    }

    return {};
}

void wxAuiDefaultDockArt::SetColour(int id, const wxColor& colour)
{
    switch (id)
    {
        case wxAUI_DOCKART_BACKGROUND_COLOUR:                m_backgroundBrush.SetColour(colour); break;
        case wxAUI_DOCKART_SASH_COLOUR:                      m_sashBrush.SetColour(colour); break;
        case wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR:          m_inactiveCaptionColour = colour; break;
        case wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR: m_inactiveCaptionGradientColour = colour; break;
        case wxAUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR:     m_inactiveCaptionTextColour = colour; break;
        case wxAUI_DOCKART_ACTIVE_CAPTION_COLOUR:            m_activeCaptionColour = colour; break;
        case wxAUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR:   m_activeCaptionGradientColour = colour; break;
        case wxAUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR:       m_activeCaptionTextColour = colour; break;
        case wxAUI_DOCKART_BORDER_COLOUR:                    m_borderPen.SetColour(colour); break;
        case wxAUI_DOCKART_GRIPPER_COLOUR:
            m_gripperBrush.SetColour(colour);
            m_gripperPen1.SetColour(colour.ChangeLightness(40));
            m_gripperPen2.SetColour(colour.ChangeLightness(60));
            break;
        default: wxFAIL_MSG("Invalid Metric Ordinal"); break;
    }

    InitBitmaps();
}

void wxAuiDefaultDockArt::SetFont(int id, const wxFont& font)
{
    if (id == wxAUI_DOCKART_CAPTION_FONT)
        m_captionFont = font;
}

wxFont wxAuiDefaultDockArt::GetFont(int id)
{
    if (id == wxAUI_DOCKART_CAPTION_FONT)
        return m_captionFont;
    return wxNullFont;
}

void wxAuiDefaultDockArt::DrawSash(wxDC& dc, wxWindow *window, int orientation, const wxRect& rect)
{
#if defined( __WXMAC__ ) && wxOSX_USE_COCOA_OR_CARBON
    wxUnusedVar(window);
    wxUnusedVar(orientation);

    if ( wxPlatformInfo::Get().CheckOSVersion(10, 14) && wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW).Red() < 128 )
    {
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(m_sashBrush);
        dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);
    }
    else
    {
        HIRect splitterRect = CGRectMake( rect.x , rect.y , rect.width , rect.height );
        CGContextRef cgContext ;
        wxGCDCImpl *impl = (wxGCDCImpl*) dc.GetImpl();
        cgContext = (CGContextRef) impl->GetGraphicsContext()->GetNativeContext() ;

        HIThemeSplitterDrawInfo drawInfo ;
        drawInfo.version = 0 ;
        drawInfo.state = kThemeStateActive ;
        drawInfo.adornment = kHIThemeSplitterAdornmentNone ;
        HIThemeDrawPaneSplitter( &splitterRect , &drawInfo , cgContext , kHIThemeOrientationNormal ) ;
    }

#elif defined(__WXGTK__)
    // clear out the rectangle first
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(m_sashBrush);
    dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);

#if 0
    GdkRectangle gdk_rect;
    if (orientation == wxVERTICAL )
    {
        gdk_rect.x = rect.x;
        gdk_rect.y = rect.y;
        gdk_rect.width = m_sashSize;
        gdk_rect.height = rect.height;
    }
    else
    {
        gdk_rect.x = rect.x;
        gdk_rect.y = rect.y;
        gdk_rect.width = rect.width;
        gdk_rect.height = m_sashSize;
    }
#endif

    if (!window) return;
    if (!window->m_wxwindow) return;
    if (!gtk_widget_is_drawable(window->m_wxwindow)) return;

#ifdef __WXGTK3__
    cairo_t* cr = static_cast<cairo_t*>(dc.GetGraphicsContext()->GetNativeContext());
    // invert orientation for widget (horizontal GtkPaned has a vertical splitter)
    wxOrientation orient = orientation == wxVERTICAL ? wxHORIZONTAL : wxVERTICAL;
    GtkWidget* widget = wxGTKPrivate::GetSplitterWidget(orient);
    GtkStyleContext* sc = gtk_widget_get_style_context(widget);
    gtk_style_context_save(sc);

    gtk_style_context_add_class(sc, GTK_STYLE_CLASS_PANE_SEPARATOR);
    gtk_render_handle(sc, cr, rect.x, rect.y, rect.width, rect.height);

    gtk_style_context_restore(sc);
#else
    gtk_paint_handle
    (
        gtk_widget_get_style(window->m_wxwindow),
        window->GTKGetDrawingWindow(),
        // flags & wxCONTROL_CURRENT ? GTK_STATE_PRELIGHT : GTK_STATE_NORMAL,
        GTK_STATE_NORMAL,
        GTK_SHADOW_NONE,
        NULL /* no clipping */,
        window->m_wxwindow,
        "paned",
        rect.x,
        rect.y,
        rect.width,
        rect.height,
        (orientation == wxVERTICAL) ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL
    );
#endif // !__WXGTK3__

#else
    wxUnusedVar(window);
    wxUnusedVar(orientation);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(m_sashBrush);
    dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);
#endif
}


void wxAuiDefaultDockArt::DrawBackground(wxDC& dc, [[maybe_unused]] wxWindow *window, int, const wxRect& rect)
{
    dc.SetPen(*wxTRANSPARENT_PEN);
#ifdef __WXMAC__
    // we have to clear first, otherwise we are drawing a light striped pattern
    // over an already darker striped background
    dc.SetBrush(*wxWHITE_BRUSH) ;
    dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);
#endif
    dc.SetBrush(m_backgroundBrush);
    dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);
}

void wxAuiDefaultDockArt::DrawBorder(wxDC& dc, wxWindow* window, const wxRect& _rect,
                                  wxAuiPaneInfo& pane)
{
    dc.SetPen(m_borderPen);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);

    wxRect rect = _rect;
    int border_width = GetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE);

    if (pane.IsToolbar())
    {
        for (int i = 0; i < border_width; ++i)
        {
            dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)));
            dc.DrawLine(rect.x, rect.y, rect.x+rect.width, rect.y);
            dc.DrawLine(rect.x, rect.y, rect.x, rect.y+rect.height);
            dc.SetPen(m_borderPen);
            dc.DrawLine(rect.x, rect.y+rect.height-1,
                        rect.x+rect.width, rect.y+rect.height-1);
            dc.DrawLine(rect.x+rect.width-1, rect.y,
                        rect.x+rect.width-1, rect.y+rect.height);
            rect.Deflate(1);
        }
    }
    else
    {
        // notebooks draw the border themselves, so they can use native rendering (e.g. tabartgtk)
        wxAuiTabArt* art = nullptr;
        wxAuiNotebook* nb = dynamic_cast<wxAuiNotebook*>(window);
        if (nb)
            art = nb->GetArtProvider();

        if (art)
            art->DrawBorder(dc, window, rect);
        else
        {
            for (int i = 0; i < border_width; ++i)
            {
                dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);
                rect.Deflate(1);
            }
        }
    }
}


void wxAuiDefaultDockArt::DrawCaptionBackground(wxDC& dc, const wxRect& rect, bool active)
{
    if (m_gradientType == wxAUI_GRADIENT_NONE)
    {
        if (active)
            dc.SetBrush(wxBrush(m_activeCaptionColour));
        else
            dc.SetBrush(wxBrush(m_inactiveCaptionColour));

        dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);
    }
    else
    {
        if (active)
        {
            // on mac the gradients are expected to become darker from the top
#ifdef __WXMAC__
            DrawGradientRectangle(dc, rect,
                                 m_activeCaptionColour,
                                 m_activeCaptionGradientColour,
                                 m_gradientType);
#else
            // on other platforms, active gradients become lighter at the top
            DrawGradientRectangle(dc, rect,
                                 m_activeCaptionGradientColour,
                                 m_activeCaptionColour,
                                 m_gradientType);
#endif
        }
        else
        {
#ifdef __WXMAC__
            // on mac the gradients are expected to become darker from the top
            DrawGradientRectangle(dc, rect,
                                 m_inactiveCaptionGradientColour,
                                 m_inactiveCaptionColour,
                                 m_gradientType);
#else
            // on other platforms, inactive gradients become lighter at the bottom
            DrawGradientRectangle(dc, rect,
                                 m_inactiveCaptionColour,
                                 m_inactiveCaptionGradientColour,
                                 m_gradientType);
#endif
        }
    }
}


void wxAuiDefaultDockArt::DrawCaption(wxDC& dc,
                                   wxWindow* window,
                                   const std::string& text,
                                   const wxRect& rect,
                                   wxAuiPaneInfo& pane)
{
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetFont(m_captionFont);

    DrawCaptionBackground(dc, rect,
                          (pane.state & wxAuiPaneInfo::optionActive) != 0);

    int caption_offset = 0;
    if ( pane.icon.IsOk() )
    {
        DrawIcon(dc, window, rect, pane);

        caption_offset += std::lround(pane.icon.GetScaledWidth()) + window->FromDIP(3);
    }

    if (pane.state & wxAuiPaneInfo::optionActive)
        dc.SetTextForeground(m_activeCaptionTextColour);
    else
        dc.SetTextForeground(m_inactiveCaptionTextColour);


    wxCoord w,h;
    dc.GetTextExtent("ABCDEFHXfgkj", &w, &h);

    wxRect clip_rect = rect;
    clip_rect.width -= window->FromDIP(3); // text offset
    clip_rect.width -= window->FromDIP(2); // button padding
    if (pane.HasCloseButton())
        clip_rect.width -= m_buttonSize;
    if (pane.HasPinButton())
        clip_rect.width -= m_buttonSize;
    if (pane.HasMaximizeButton())
        clip_rect.width -= m_buttonSize;

    std::string draw_text = wxAuiChopText(dc, text, clip_rect.width);

    dc.SetClippingRegion(clip_rect);
    dc.wxDrawText(draw_text, wxPoint{rect.x+window->FromDIP(3) + caption_offset, rect.y+(rect.height/2)-(h/2)-1});
    dc.DestroyClippingRegion();
}

void
wxAuiDefaultDockArt::DrawIcon(wxDC& dc, wxWindow *window, const wxRect& rect, wxAuiPaneInfo& pane)
{
    // Draw the icon centered vertically
    int xOffset = window ? window->FromDIP(2) : 2;
    dc.DrawBitmap(pane.icon,
                  rect.x+xOffset, rect.y+(rect.height-pane.icon.GetScaledHeight())/2,
                  true);
}

void wxAuiDefaultDockArt::DrawGripper(wxDC& dc,
                                   wxWindow* window,
                                   const wxRect& rect,
                                   wxAuiPaneInfo& pane)
{
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(m_gripperBrush);

    dc.DrawRectangle(rect.x, rect.y, rect.width,rect.height);

    if (!pane.HasGripperTop())
    {
        int y = window->FromDIP(5);
        while (true)
        {
            dc.SetPen(m_gripperPen1);
            dc.DrawPoint(rect.x+window->FromDIP(3), rect.y+y);
            dc.SetPen(m_gripperPen2);
            dc.DrawPoint(rect.x+window->FromDIP(3), rect.y+y+window->FromDIP(1));
            dc.DrawPoint(rect.x+window->FromDIP(4), rect.y+y                   );
            dc.SetPen(m_gripperPen3);
            dc.DrawPoint(rect.x+window->FromDIP(5), rect.y+y+window->FromDIP(1));
            dc.DrawPoint(rect.x+window->FromDIP(5), rect.y+y+window->FromDIP(2));
            dc.DrawPoint(rect.x+window->FromDIP(4), rect.y+y+window->FromDIP(2));

            y += window->FromDIP(4);
            if (y > rect.GetHeight()-window->FromDIP(5))
                break;
        }
    }
    else
    {
        int x = window->FromDIP(5);
        while (true)
        {
            dc.SetPen(m_gripperPen1);
            dc.DrawPoint(rect.x+x, rect.y+window->FromDIP(3));
            dc.SetPen(m_gripperPen2);
            dc.DrawPoint(rect.x+x+window->FromDIP(1), rect.y+window->FromDIP(3));
            dc.DrawPoint(rect.x+x                   , rect.y+window->FromDIP(4));
            dc.SetPen(m_gripperPen3);
            dc.DrawPoint(rect.x+x+window->FromDIP(1), rect.y+window->FromDIP(5));
            dc.DrawPoint(rect.x+x+window->FromDIP(2), rect.y+window->FromDIP(5));
            dc.DrawPoint(rect.x+x+window->FromDIP(2), rect.y+window->FromDIP(4));

            x += window->FromDIP(4);
            if (x > rect.GetWidth()-window->FromDIP(5))
                break;
        }
    }
}

void wxAuiDefaultDockArt::DrawPaneButton(wxDC& dc,
                                      wxWindow* window,
                                      int button,
                                      int button_state,
                                      const wxRect& _rect,
                                      wxAuiPaneInfo& pane)
{
    wxBitmap bmp;
    switch (button)
    {
        default:
        case wxAUI_BUTTON_CLOSE:
            if (pane.state & wxAuiPaneInfo::optionActive)
                bmp = m_activeCloseBitmap;
            else
                bmp = m_inactiveCloseBitmap;
            break;
        case wxAUI_BUTTON_PIN:
            if (pane.state & wxAuiPaneInfo::optionActive)
                bmp = m_activePinBitmap;
            else
                bmp = m_inactivePinBitmap;
            break;
        case wxAUI_BUTTON_MAXIMIZE_RESTORE:
            if (pane.wxIsMaximized())
            {
                if (pane.state & wxAuiPaneInfo::optionActive)
                    bmp = m_activeRestoreBitmap;
                else
                    bmp = m_inactiveRestoreBitmap;
            }
            else
            {
                if (pane.state & wxAuiPaneInfo::optionActive)
                    bmp = m_activeMaximizeBitmap;
                else
                    bmp = m_inactiveMaximizeBitmap;
            }
            break;
    }

    wxAuiScaleBitmap(bmp, window->GetDPIScaleFactor());

    wxRect rect = _rect;

    rect.y = rect.y + (rect.height/2) - std::lround(bmp.GetScaledHeight()/2);

    if (button_state == wxAUI_BUTTON_STATE_PRESSED)
    {
        rect.x += window->FromDIP(1);
        rect.y += window->FromDIP(1);
    }

    if (button_state == wxAUI_BUTTON_STATE_HOVER ||
        button_state == wxAUI_BUTTON_STATE_PRESSED)
    {
        if (pane.state & wxAuiPaneInfo::optionActive)
        {
            dc.SetBrush(wxBrush(m_activeCaptionColour.ChangeLightness(120)));
            dc.SetPen(wxPen(m_activeCaptionColour.ChangeLightness(70)));
        }
        else
        {
            dc.SetBrush(wxBrush(m_inactiveCaptionColour.ChangeLightness(120)));
            dc.SetPen(wxPen(m_inactiveCaptionColour.ChangeLightness(70)));
        }

        // draw the background behind the button
        dc.DrawRectangle(rect.x, rect.y,
            std::lround(bmp.GetScaledWidth()) - window->FromDIP(1),
            std::lround(bmp.GetScaledHeight()) - window->FromDIP(1));
    }

    // draw the button itself
    dc.DrawBitmap(bmp, rect.x, rect.y, true);
}
