///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/textmeasurecmn.cpp
// Purpose:     wxTextMeasureBase implementation
// Author:      Manuel Martin
// Created:     2012-10-05
// Copyright:   (c) 1997-2012 wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// for compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/dc.h"
    #include "wx/window.h"
#endif //WX_PRECOMP

#include "wx/private/textmeasure.h"

// ============================================================================
// wxTextMeasureBase implementation
// ============================================================================

wxTextMeasureBase::wxTextMeasureBase(const wxDC *dc, const wxFont *theFont)
    : m_dc(dc),
      m_win(nullptr),
      m_font(theFont)
{
    wxASSERT_MSG( dc, wxS("wxTextMeasure needs a valid wxDC") );

    // By default, use wxDC version, we'll explicitly reset this to false in
    // the derived classes if the DC is of native variety.
    m_useDCImpl = true;
}

wxTextMeasureBase::wxTextMeasureBase(const wxWindow *win, const wxFont *theFont)
    : m_dc(nullptr),
      m_win(win),
      m_font(theFont)
{
    wxASSERT_MSG( win, wxS("wxTextMeasure needs a valid wxWindow") );

    // We don't have any wxDC so we can't forward to it.
    m_useDCImpl = false;
}

wxFont wxTextMeasureBase::GetFont() const
{
    return m_font ? *m_font
                  : m_win ? m_win->GetFont()
                          : m_dc->GetFont();
}

wxSize wxTextMeasureBase::CallGetTextExtent(const wxString& string,
                                          wxCoord *descent,
                                          wxCoord *externalLeading)
{
    if ( m_useDCImpl )
        return m_dc->GetTextExtent(string, descent, externalLeading);
    else
        return DoGetTextExtent(string, descent, externalLeading);
}

wxSize wxTextMeasureBase::GetTextExtent(const wxString& string,
                                      wxCoord *descent,
                                      wxCoord *externalLeading)
{
    // Avoid even setting up the DC for measuring if we don't actually need to
    // measure anything.
    if ( string.empty() && !descent && !externalLeading )
    {
        return {0, 0};
    }

    MeasuringGuard guard(*this);

    return CallGetTextExtent(string, descent, externalLeading);
}

int wxTextMeasureBase::GetEmptyLineHeight()
{
    return CallGetTextExtent(wxS("W")).y;
}

void wxTextMeasureBase::GetMultiLineTextExtent(const wxString& text,
                                               wxCoord *width,
                                               wxCoord *height,
                                               wxCoord *heightOneLine)
{
    // To make the code simpler, make sure that the width and height pointers
    // are always valid, by making them point to dummy variables if necessary.
    int unusedWidth, unusedHeight;
    if ( !width )
        width = &unusedWidth;
    if ( !height )
        height = &unusedHeight;

    *width = 0;
    *height = 0;

    MeasuringGuard guard(*this);

    // It's noticeably faster to handle the case of a string which isn't
    // actually multiline specially here, to skip iteration above in this case.
    if ( text.find('\n') == wxString::npos )
    {
        // This case needs to be handled specially as we're supposed to return
        // a non-zero height even for empty string.
        if ( text.empty() )
            *height = GetEmptyLineHeight();
        else
        {
            auto textExtent = CallGetTextExtent(text);
            *width = textExtent.x;
            *height = textExtent.y;
        }

        if ( heightOneLine )
            *heightOneLine = *height;
        return;
    }

    wxCoord widthLine = 0;
    wxCoord heightLine = 0;
    wxCoord heightLineDefault = 0;

    wxString::const_iterator lineStart = text.begin();
    for ( wxString::const_iterator pc = text.begin(); ; ++pc )
    {
        if ( pc == text.end() || *pc == wxS('\n') )
        {
            if ( pc == lineStart )
            {
                // we can't use GetTextExtent - it will return 0 for both width
                // and height and an empty line should count in height
                // calculation

                // assume that this line has the same height as the previous
                // one
                if ( !heightLineDefault )
                    heightLineDefault = heightLine;

                // and if we hadn't had any previous one neither, compute it now
                if ( !heightLineDefault )
                    heightLineDefault = GetEmptyLineHeight();

                *height += heightLineDefault;
            }
            else
            {
                auto textExtents = CallGetTextExtent(wxString(lineStart, pc));
                widthLine = textExtents.x;
                heightLine = textExtents.y;

                if ( widthLine > *width )
                    *width = widthLine;
                *height += heightLine;
            }

            if ( pc == text.end() )
            {
               break;
            }
            else // '\n'
            {
               lineStart = pc;
               ++lineStart;
            }
        }
    }

    if ( heightOneLine )
        *heightOneLine = heightLine;
}

wxSize wxTextMeasureBase::GetLargestStringExtent(const std::vector<wxString>& strings)
{
    MeasuringGuard guard(*this);

    wxCoord widthMax = 0, heightMax = 0;

    for ( const auto& str : strings )
    {
        auto textExtent = CallGetTextExtent(str);

        if ( textExtent.x > widthMax )
            widthMax = textExtent.x;
        if ( textExtent.y > heightMax )
            heightMax = textExtent.y;
    }

    return wxSize(widthMax, heightMax);
}

std::vector<int> wxTextMeasureBase::GetPartialTextExtents(const wxString& text, double scaleX)
{
    if ( text.empty() )
        return {};

    MeasuringGuard guard(*this);

    // FIXME: Necessary?
    return DoGetPartialTextExtents(text, scaleX);
}

// ----------------------------------------------------------------------------
// Generic and inefficient DoGetPartialTextExtents() implementation.
// ----------------------------------------------------------------------------

// Each element of the widths array will be the width of the string up to and
// including the corresponding character in text.  This is the generic
// implementation, the port-specific classes should do this with native APIs
// if available and if faster.  Note: pango_layout_index_to_pos is much slower
// than calling GetTextExtent!!

#define FWC_SIZE 256

class FontWidthCache
{
public:
    FontWidthCache()  = default;
    ~FontWidthCache() { delete []m_widths; }

    void Reset()
    {
        if ( !m_widths )
            m_widths = new int[FWC_SIZE];

        memset(m_widths, 0, sizeof(int)*FWC_SIZE);
    }

    wxFont m_font;
    double m_scaleX{1};
    int *m_widths{nullptr};
};

static FontWidthCache s_fontWidthCache;

std::vector<int> wxTextMeasureBase::DoGetPartialTextExtents(const wxString& text, double scaleX)
{
    int totalWidth = 0;

    // reset the cache if font or horizontal scale have changed
    // FIXME: Double equality
    if ( !s_fontWidthCache.m_widths ||
         !(s_fontWidthCache.m_scaleX == scaleX) ||
         (s_fontWidthCache.m_font != *m_font) )
    {
        s_fontWidthCache.Reset();
        s_fontWidthCache.m_font = *m_font;
        s_fontWidthCache.m_scaleX = scaleX;
    }

    std::vector<int> widths;
    // Calculate the position of each character based on the widths of
    // the previous characters. This is inexact for not fixed fonts.
    for ( wxString::const_iterator it = text.begin();
          it != text.end();
          ++it )
    {
        const wxChar c = *it;
        unsigned int c_int = (unsigned int)c;

        int w;
        if ((c_int < FWC_SIZE) && (s_fontWidthCache.m_widths[c_int] != 0))
        {
            w = s_fontWidthCache.m_widths[c_int];
        }
        else
        {
            DoGetTextExtent(c, &w, nullptr);
            if (c_int < FWC_SIZE)
                s_fontWidthCache.m_widths[c_int] = w;
        }

        totalWidth += w;
        widths.push_back(totalWidth);
    }

    return widths;
}

