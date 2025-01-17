///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/private/dc.h
// Purpose:     private wxMSW helpers for working with HDCs
// Author:      Vadim Zeitlin
// Created:     2009-06-16 (extracted from src/msw/dc.cpp)
// Copyright:   (c) 2009 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _MSW_PRIVATE_DC_H_
#define _MSW_PRIVATE_DC_H_

#include "wx/msw/dc.h"

#include "wx/log.h"

namespace wxMSWImpl
{

// various classes to change some DC property temporarily

// text background and foreground colours
class wxTextColoursChanger
{
public:
    wxTextColoursChanger(WXHDC hdc, const wxMSWDCImpl& dc)
        : m_hdc(hdc)
    {
        Change(dc.GetTextForeground(), dc.GetTextBackground());
    }

    wxTextColoursChanger(WXHDC hdc, const wxColour& colFg, const wxColour& colBg)
        : m_hdc(hdc)
    {
        Change(colFg, colBg);
    }

    wxTextColoursChanger(WXHDC hdc, COLORREF colFg, COLORREF colBg)
        : m_hdc(hdc)
    {
        Change(colFg, colBg);
    }

    ~wxTextColoursChanger()
    {
        if ( m_oldColFg != CLR_INVALID )
            ::SetTextColor(m_hdc, m_oldColFg);
        if ( m_oldColBg != CLR_INVALID )
            ::SetBkColor(m_hdc, m_oldColBg);
    }

protected:
    // this ctor doesn't change mode immediately, call Change() later to do it
    // only if needed
    wxTextColoursChanger(WXHDC hdc)
        : m_hdc(hdc)
    {
        m_oldColFg =
        m_oldColBg = CLR_INVALID;
    }

    void Change(const wxColour& colFg, const wxColour& colBg)
    {
        Change(colFg.IsOk() ? colFg.GetPixel() : CLR_INVALID,
               colBg.IsOk() ? colBg.GetPixel() : CLR_INVALID);
    }

    void Change(COLORREF colFg, COLORREF colBg)
    {
        if ( colFg != CLR_INVALID )
        {
            m_oldColFg = ::SetTextColor(m_hdc, colFg);
            if ( m_oldColFg == CLR_INVALID )
            {
                wxLogLastError("SetTextColor");
            }
        }
        else
        {
            m_oldColFg = CLR_INVALID;
        }

        if ( colBg != CLR_INVALID )
        {
            m_oldColBg = ::SetBkColor(m_hdc, colBg);
            if ( m_oldColBg == CLR_INVALID )
            {
                wxLogLastError("SetBkColor");
            }
        }
        else
        {
            m_oldColBg = CLR_INVALID;
        }
    }

private:
    const WXHDC m_hdc;
    COLORREF m_oldColFg,
             m_oldColBg;
};

// background mode
class wxBkModeChanger
{
public:
    // set background mode to opaque if mode != wxBrushStyle::Transparent
    wxBkModeChanger(WXHDC hdc, wxBrushStyle mode)
        : m_hdc(hdc)
    {
        Change(mode);
    }

    ~wxBkModeChanger()
    {
        if ( m_oldMode )
            ::SetBkMode(m_hdc, m_oldMode);
    }

protected:
    // this ctor doesn't change mode immediately, call Change() later to do it
    // only if needed
    wxBkModeChanger(WXHDC hdc) : m_hdc(hdc) { m_oldMode = 0; }

    void Change(wxBrushStyle mode)
    {
        m_oldMode = ::SetBkMode(m_hdc, mode == wxBrushStyle::Transparent
                                        ? TRANSPARENT
                                        : OPAQUE);
        if ( !m_oldMode )
        {
            wxLogLastError("SetBkMode");
        }
    }

private:
    const WXHDC m_hdc;
    int m_oldMode;
};

} // namespace wxMSWImpl

#endif // _MSW_PRIVATE_DC_H_

