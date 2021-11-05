///////////////////////////////////////////////////////////////////////////////
// Name:        wx/bannerwindow.h
// Purpose:     wxBannerWindow class declaration
// Author:      Vadim Zeitlin
// Created:     2011-08-16
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_BANNERWINDOW_H_
#define _WX_BANNERWINDOW_H_

#if wxUSE_BANNERWINDOW

#include "wx/defs.h"

#include "wx/bitmap.h"
#include "wx/event.h"
#include "wx/window.h"

#include <string>

class wxBitmap;
class wxColour;
class wxDC;

constexpr char wxBannerWindowNameStr[] = "bannerwindow";

// ----------------------------------------------------------------------------
// A simple banner window showing either a bitmap or text.
// ----------------------------------------------------------------------------

class wxBannerWindow : public wxWindow
{
public:
    // Default constructor, use Create() later.
    wxBannerWindow() { 
    m_direction = wxLEFT;

    m_colStart = *wxWHITE;
    m_colEnd = *wxBLUE;
 }

    // Convenient constructor that should be used in the majority of cases.
    //
    // The banner orientation changes how the text in it is displayed and also
    // defines where is the bitmap truncated if it's too big to fit but doesn't
    // do anything for the banner position, this is supposed to be taken care
    // of in the usual way, e.g. using sizers.
    wxBannerWindow(wxWindow* parent, wxDirection dir = wxLEFT)
    {
        
    m_direction = wxLEFT;

    m_colStart = *wxWHITE;
    m_colEnd = *wxBLUE;


        Create(parent, wxID_ANY, dir);
    }

    // Full constructor provided for consistency with the other classes only.
    wxBannerWindow(wxWindow* parent,
                   wxWindowID winid,
                   wxDirection dir = wxLEFT,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   unsigned int style = 0,
                   const std::string& name = wxBannerWindowNameStr)
    {
        
    m_direction = wxLEFT;

    m_colStart = *wxWHITE;
    m_colEnd = *wxBLUE;


        Create(parent, winid, dir, pos, size, style, name);
    }

    wxBannerWindow& operator=(wxBannerWindow&&) = delete;

    // Can be only called on objects created with the default constructor.
    [[maybe_unused]] bool Create(wxWindow* parent,
                wxWindowID winid,
                wxDirection dir = wxLEFT,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const std::string& name = wxBannerWindowNameStr);


    // Provide an existing bitmap to show. For wxLEFT orientation the bitmap is
    // truncated from the top, for wxTOP and wxBOTTOM -- from the right and for
    // wxRIGHT -- from the bottom, so put the most important part of the bitmap
    // information in the opposite direction.
    void SetBitmap(const wxBitmap& bmp);

    // Set the text to display. This is mutually exclusive with SetBitmap().
    // Title is rendered in bold and should be single line, message can have
    // multiple lines but is not wrapped automatically.
    void SetText(const std::string& title, const std::string& message);

    // Set the colours between which the gradient runs. This can be combined
    // with SetText() but not SetBitmap().
    void SetGradient(const wxColour& start, const wxColour& end);

protected:
    wxSize DoGetBestClientSize() const override;

private:    
    // Fully invalidates the window.
    void OnSize(wxSizeEvent& event);

    // Redraws the window using either m_bitmap or m_title/m_message.
    void OnPaint(wxPaintEvent& event);

    // Helper of OnPaint(): draw the bitmap at the correct position depending
    // on our orientation.
    void DrawBitmapBackground(wxDC& dc);

    // Helper of OnPaint(): draw the text in the appropriate direction.
    void DrawBannerTextLine(wxDC& dc, std::string_view str, const wxPoint& pos);

    // Return the font to use for the title. Currently this is hardcoded as a
    // larger bold version of the standard window font but could be made
    // configurable in the future.
    wxFont GetTitleFont() const;

    // Start and stop gradient colours, only used when drawing text.
    wxColour m_colStart,
             m_colEnd;

    // Return the colour to use for extending the bitmap. Non-const as it
    // updates m_colBitmapBg if needed.
    wxColour GetBitmapBg();

    // If bitmap is valid, this is the colour we use to extend it if the bitmap
    // is smaller than this window. It is computed on demand by GetBitmapBg().
    wxColour m_colBitmapBg;

    // The title and main message to draw, used if m_bitmap is invalid.
    std::string m_title;
    std::string m_message;
    
    // If valid, this bitmap is drawn as is.
    wxBitmap m_bitmap;

    // The window side along which the banner is laid out.
    wxDirection m_direction;

    wxDECLARE_EVENT_TABLE();
};

#endif // wxUSE_BANNERWINDOW

#endif // _WX_BANNERWINDOW_H_
