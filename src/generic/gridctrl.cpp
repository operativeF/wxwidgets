///////////////////////////////////////////////////////////////////////////
// Name:        src/generic/gridctrl.cpp
// Purpose:     wxGrid controls
// Author:      Paul Gammans, Roger Gammans
// Modified by:
// Created:     11/04/2001
// Copyright:   (c) The Computer Surgery (paul@compsurg.co.uk)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/textctrl.h"
#include "wx/dc.h"
#include "wx/log.h"
#include "wx/checkbox.h"

#include "wx/renderer.h"

#include "wx/generic/private/grid.h"
#include "wx/private/window.h"

#include <fmt/core.h>

module WX.Grid.Ctrl;

import WX.Grid.Editors;

import WX.Utils.Settings;
import Utils.Strings;

import <charconv>;

// ----------------------------------------------------------------------------
// wxGridCellRenderer
// ----------------------------------------------------------------------------

void wxGridCellRenderer::Draw(wxGrid& grid,
                              wxGridCellAttr& attr,
                              wxDC& dc,
                              const wxRect& rect,
                              [[maybe_unused]] int row, [[maybe_unused]] int col,
                              bool isSelected)
{
    dc.SetBackgroundMode( wxBrushStyle::Solid );

    wxColour clr;
    if ( grid.IsThisEnabled() )
    {
        if ( isSelected )
        {
            if ( grid.HasFocus() )
                clr = grid.GetSelectionBackground();
            else
                clr = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW);
        }
        else
        {
            clr = attr.GetBackgroundColour();
        }
    }
    else // grey out fields if the grid is disabled
    {
        clr = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
    }

    dc.SetBrush(clr);
    dc.SetPen( *wxTRANSPARENT_PEN );
    dc.DrawRectangle(rect);
}

void wxGridCellRenderer::SetTextColoursAndFont(const wxGrid& grid,
                                               const wxGridCellAttr& attr,
                                               wxDC& dc,
                                               bool isSelected)
{
    dc.SetBackgroundMode( wxBrushStyle::Transparent );

    // TODO some special colours for attr.IsReadOnly() case?

    // different coloured text when the grid is disabled
    if ( grid.IsThisEnabled() )
    {
        if ( isSelected )
        {
            wxColour clr;
            if ( grid.HasFocus() )
                clr = grid.GetSelectionBackground();
            else
                clr = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW);
            dc.SetTextBackground( clr );
            dc.SetTextForeground( grid.GetSelectionForeground() );
        }
        else
        {
            dc.SetTextBackground( attr.GetBackgroundColour() );
            dc.SetTextForeground( attr.GetTextColour() );
        }
    }
    else
    {
        dc.SetTextBackground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
        dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    }

    dc.SetFont( attr.GetFont() );
}

// ----------------------------------------------------------------------------
// wxGridCellDateTimeRenderer
// ----------------------------------------------------------------------------

#if wxUSE_DATETIME

bool
wxGridPrivate::TryGetValueAsDate(wxDateTime& result,
                                 const DateParseParams& params,
                                 const wxGrid& grid,
                                 int row, int col)
{
    wxGridTableBase *table = grid.GetTable();

    if ( table->CanGetValueAs(row, col, wxGRID_VALUE_DATETIME) )
    {
        void * tempval = table->GetValueAsCustom(row, col, wxGRID_VALUE_DATETIME);

        if (tempval)
        {
            result = *((wxDateTime *)tempval);
            delete (wxDateTime *)tempval;

            return true;
        }

    }

    const wxString text = table->GetValue(row, col);

    wxString::const_iterator end;

    if ( result.ParseFormat(wxString{text}, params.format, &end) && end == text.end() )
        return true;

    // Check if we can fall back to free-form parsing, which notably allows us
    // to parse strings such as "today" or "tomorrow" which would be never
    // accepted by ParseFormat().
    if ( params.fallbackParseDate &&
            result.ParseDate(wxString{text}, &end) && end == text.end() )
        return true;

    return false;
}

using namespace wxGridPrivate;

// Enables a grid cell to display a formatted date

wxGridCellDateRenderer::wxGridCellDateRenderer(const std::string& outformat)
{
    if ( outformat.empty() )
    {
        m_oformat = "%x"; // Localized date representation.
    }
    else
    {
        m_oformat = outformat;
    }

    m_tz = wxDateTime::Local;
}

wxGridCellRenderer *wxGridCellDateRenderer::Clone() const
{
    return new wxGridCellDateRenderer(*this);
}

std::string wxGridCellDateRenderer::GetString(const wxGrid& grid, int row, int col)
{
    std::string text;

    DateParseParams params;
    GetDateParseParams(params);

    wxDateTime val;
    if ( TryGetValueAsDate(val, params, grid, row, col) )
        text = val.Format(m_oformat, m_tz );

    // If we failed to parse string just show what we where given?
    return text;
}

void
wxGridCellDateRenderer::GetDateParseParams(DateParseParams& params) const
{
    params = DateParseParams::WithFallback(m_oformat);
}

void wxGridCellDateRenderer::Draw(wxGrid& grid,
                                  wxGridCellAttr& attr,
                                  wxDC& dc,
                                  const wxRect& rectCell,
                                  int row, int col,
                                  bool isSelected)
{
    wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);

    SetTextColoursAndFont(grid, attr, dc, isSelected);

    wxRect rect = rectCell;
    rect.Inflate(-1);

    // draw the text right aligned by default
    grid.DrawTextRectangle(dc, GetString(grid, row, col), rect, attr,
                           wxALIGN_RIGHT);
}

wxSize wxGridCellDateRenderer::GetBestSize(wxGrid& grid,
                                           wxGridCellAttr& attr,
                                           wxDC& dc,
                                           int row, int col)
{
    return DoGetBestSize(attr, dc, GetString(grid, row, col));
}

wxSize wxGridCellDateRenderer::GetMaxBestSize([[maybe_unused]] wxGrid& grid,
                                              wxGridCellAttr& attr,
                                              wxDC& dc)
{
    wxSize size;

    // Try to produce the longest string in the current format: as we don't
    // know which month is the longest, we need to try all of them.
    for ( int m = wxDateTime::Jan; m <= wxDateTime::Dec; ++m )
    {
        const wxDateTime d(28, static_cast<wxDateTime::Month>(m), 9999);

        size.IncTo(DoGetBestSize(attr, dc, d.Format(m_oformat, m_tz).ToStdString()));
    }

    return size;
}

void wxGridCellDateRenderer::SetParameters(const std::string& params)
{
    if (!params.empty())
        m_oformat = params;
}


// Enables a grid cell to display a formatted date and or time

wxGridCellDateTimeRenderer::wxGridCellDateTimeRenderer(const std::string& outformat, const std::string& informat)
    : wxGridCellDateRenderer(outformat)
    , m_iformat(informat)
{
}

wxGridCellRenderer *wxGridCellDateTimeRenderer::Clone() const
{
    return new wxGridCellDateTimeRenderer(*this);
}

// FIXME: Return params.
void
wxGridCellDateTimeRenderer::GetDateParseParams(DateParseParams& params) const
{
    params = DateParseParams::WithoutFallback(m_iformat);
}

#endif // wxUSE_DATETIME

// ----------------------------------------------------------------------------
// wxGridCellChoiceRenderer
// ----------------------------------------------------------------------------

wxSize wxGridCellChoiceRenderer::GetMaxBestSize([[maybe_unused]] wxGrid& grid,
                                                wxGridCellAttr& attr,
                                                wxDC& dc)
{
    wxSize size;

    for ( size_t n = 0; n < m_choices.size(); ++n )
    {
        size.IncTo(DoGetBestSize(attr, dc, m_choices[n]));
    }

    return size;
}

void wxGridCellChoiceRenderer::SetParameters(const std::string& params)
{
    // TODO: Is clearing necessary?
    m_choices.clear();

    if ( params.empty() )
        return;

    m_choices = wx::utils::StrSplit(params, ',');
}

// ----------------------------------------------------------------------------
// wxGridCellEnumRenderer
// ----------------------------------------------------------------------------
// Renders a number as a textual equivalent.
// eg data in cell is 0,1,2 ... n the cell could be rendered as "John","Fred"..."Bob"


wxGridCellEnumRenderer::wxGridCellEnumRenderer(const std::string& choices)
{
    if (!choices.empty())
        SetParameters(choices);
}

wxGridCellRenderer *wxGridCellEnumRenderer::Clone() const
{
    wxGridCellEnumRenderer *renderer = new wxGridCellEnumRenderer;
    renderer->m_choices = m_choices;
    return renderer;
}

std::string wxGridCellEnumRenderer::GetString(const wxGrid& grid, int row, int col)
{
    wxGridTableBase *table = grid.GetTable();

    if ( table->CanGetValueAs(row, col, wxGRID_VALUE_NUMBER) )
    {
        const int choiceno = table->GetValueAsLong(row, col);
        return m_choices[ choiceno ];
    }
    else
    {
        // If we faild to parse string just show what we where given?
        return table->GetValue(row, col);
    }
}

void wxGridCellEnumRenderer::Draw(wxGrid& grid,
                                   wxGridCellAttr& attr,
                                   wxDC& dc,
                                   const wxRect& rectCell,
                                   int row, int col,
                                   bool isSelected)
{
    wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);

    SetTextColoursAndFont(grid, attr, dc, isSelected);

    wxRect rect = rectCell;
    rect.Inflate(-1);

    // draw the text right aligned by default
    grid.DrawTextRectangle(dc, GetString(grid, row, col), rect, attr,
                           wxALIGN_RIGHT);
}

wxSize wxGridCellEnumRenderer::GetBestSize(wxGrid& grid,
                                            wxGridCellAttr& attr,
                                            wxDC& dc,
                                            int row, int col)
{
    return DoGetBestSize(attr, dc, GetString(grid, row, col));
}

// ----------------------------------------------------------------------------
// wxGridCellAutoWrapStringRenderer
// ----------------------------------------------------------------------------


void
wxGridCellAutoWrapStringRenderer::Draw(wxGrid& grid,
                      wxGridCellAttr& attr,
                      wxDC& dc,
                      const wxRect& rectCell,
                      int row, int col,
                      bool isSelected) {


    wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);

    // now we only have to draw the text
    SetTextColoursAndFont(grid, attr, dc, isSelected);

    int horizAlign, vertAlign;
    attr.GetAlignment(&horizAlign, &vertAlign);

    wxRect rect = rectCell;
    rect.Inflate(-1);

    // Do not use here the overload taking the attribute, as this would
    // ellipsize the text, which is never necessary with this renderer.
    grid.DrawTextRectangle(dc, GetTextLines(grid,dc,attr,rect,row,col),
                           rect, horizAlign, vertAlign);
}


std::vector<std::string>
wxGridCellAutoWrapStringRenderer::GetTextLines(wxGrid& grid,
                                               wxDC& dc,
                                               const wxGridCellAttr& attr,
                                               const wxRect& rect,
                                               int row, int col)
{
    dc.SetFont(attr.GetFont());
    const wxCoord maxWidth = rect.GetWidth();

    // Transform logical lines into physical ones, wrapping the longer ones.
    const std::vector<std::string>
        logicalLines = wx::utils::StrSplit(grid.GetCellValue(row, col), '\n');

    // Trying to do anything if the column is hidden anyhow doesn't make sense
    // and we run into problems in BreakLine() in this case.
    if ( maxWidth <= 0 )
        return logicalLines;

    std::vector<std::string> physicalLines;
    
    for ( const auto& line : logicalLines )
    {
        if ( dc.GetTextExtent(line).x > maxWidth )
        {
            // Line does not fit, break it up.
            BreakLine(dc, line, maxWidth, physicalLines);
        }
        else // The entire line fits as is
        {
            physicalLines.push_back(line);
        }
    }

    return physicalLines;
}

void
wxGridCellAutoWrapStringRenderer::BreakLine(wxDC& dc,
                                            const std::string& logicalLine,
                                            wxCoord maxWidth,
                                            std::vector<std::string>& lines)
{
    wxCoord lineWidth = 0;
    std::string line;

    // For each word
    wxStringTokenizer wordTokenizer(logicalLine, " \t", wxStringTokenizerMode::RetDelims);
    while ( wordTokenizer.HasMoreTokens() )
    {
        const std::string word = wordTokenizer.GetNextToken();
        const wxCoord wordWidth = dc.GetTextExtent(word).x;
        if ( lineWidth + wordWidth < maxWidth )
        {
            // Word fits, just add it to this line.
            line += word;
            lineWidth += wordWidth;
        }
        else
        {
            // Word does not fit, check whether the word is itself wider that
            // available width
            if ( wordWidth < maxWidth )
            {
                // Word can fit in a new line, put it at the beginning
                // of the new line.
                lines.push_back(line);
                line = word;
                lineWidth = wordWidth;
            }
            else // Word cannot fit in available width at all.
            {
                if ( !line.empty() )
                {
                    lines.push_back(line);
                    line.clear();
                    lineWidth = 0;
                }

                // Break it up in several lines.
                lineWidth = BreakWord(dc, word, maxWidth, lines, line);
            }
        }
    }

    if ( !line.empty() )
        lines.push_back(line);
}


wxCoord
wxGridCellAutoWrapStringRenderer::BreakWord(wxDC& dc,
                                            const std::string& word,
                                            wxCoord maxWidth,
                                            std::vector<std::string>& lines,
                                            std::string& line)
{
    std::vector<int> widths = dc.GetPartialTextExtents(word);

    // TODO: Use binary search to find the first element > maxWidth.
    // TODO: Lambda
    const unsigned count = widths.size();
    unsigned n;
    for ( n = 0; n < count; n++ )
    {
        if ( widths[n] > maxWidth )
            break;
    }

    if ( n == 0 )
    {
        // This is a degenerate case: the first character of the word is
        // already wider than the available space, so we just can't show it
        // completely and have to put the first character in this line.
        n = 1;
    }

    lines.push_back(word.substr(0, n));

    // Check if the remainder of the string fits in one line.
    //
    // Unfortunately we can't use the existing partial text extents as the
    // extent of the remainder may be different when it's rendered in a
    // separate line instead of as part of the same one, so we have to
    // recompute it.
    const std::string rest = word.substr(n);
    const wxCoord restWidth = dc.GetTextExtent(rest).x;
    if ( restWidth <= maxWidth )
    {
        line = rest;
        return restWidth;
    }

    // Break the rest of the word into lines.
    //
    // TODO: Perhaps avoid recursion? The code is simpler like this but using a
    // loop in this function would probably be more efficient.
    return BreakWord(dc, rest, maxWidth, lines, line);
}

wxSize
wxGridCellAutoWrapStringRenderer::GetBestSize(wxGrid& grid,
                                              wxGridCellAttr& attr,
                                              wxDC& dc,
                                              int row, int col)
{
    // We have to make a choice here and fix either width or height because we
    // don't have any naturally best size. This choice is mostly arbitrary, but
    // we need to be consistent about it, otherwise wxGrid auto-sizing code
    // would get confused. For now we decide to use a single line of text and
    // compute the width needed to fully display everything.
    const int height = dc.GetCharHeight();

    return {GetBestWidth(grid, attr, dc, row, col, height), height};
}

constexpr int AUTOWRAP_Y_MARGIN = 4;

int
wxGridCellAutoWrapStringRenderer::GetBestHeight(wxGrid& grid,
                                                wxGridCellAttr& attr,
                                                wxDC& dc,
                                                int row, int col,
                                                int width)
{
    const int lineHeight = dc.GetCharHeight();

    // Use as many lines as we need for this width and add a small border to
    // improve the appearance.
    return GetTextLines(grid, dc, attr, wxSize(width, lineHeight),
                        row, col).size() * lineHeight + AUTOWRAP_Y_MARGIN;
}

int
wxGridCellAutoWrapStringRenderer::GetBestWidth(wxGrid& grid,
                                               wxGridCellAttr& attr,
                                               wxDC& dc,
                                               int row, int col,
                                               int height)
{
    const int lineHeight = dc.GetCharHeight();

    // Base the maximal number of lines either on how many fit or how many
    // (new)lines the cell's text contains, whichever results in the most lines.
    //
    // It's important to take the newlines into account as GetTextLines() splits
    // based on them and the number of lines returned can never drop below that,
    // resulting in the while loop below never exiting if there are already more
    // lines in the text than can fit in the available height.
    auto cellVal = grid.GetCellValue(row, col);
    auto nline_ct = std::count(cellVal.begin(), cellVal.end(), '\n');

    const size_t maxLines = std::max(
                              (height - AUTOWRAP_Y_MARGIN) / lineHeight,
                              static_cast<int>(1 + nline_ct));

    // Increase width until all the text fits.
    //
    // TODO: this is not the most efficient to do it for the long strings.
    const int charWidth = dc.wxGetCharWidth();
    int width = 2*charWidth;
    while ( GetTextLines(grid, dc, attr, wxSize(width, height),
                         row, col).size() > maxLines )
        width += charWidth;

    return width;
}

// ----------------------------------------------------------------------------
// wxGridCellStringRenderer
// ----------------------------------------------------------------------------

wxSize wxGridCellStringRenderer::DoGetBestSize(const wxGridCellAttr& attr,
                                               wxDC& dc,
                                               const std::string& text)
{
    dc.SetFont(attr.GetFont());
    return dc.GetMultiLineTextExtent(text);
}

wxSize wxGridCellStringRenderer::GetBestSize(wxGrid& grid,
                                             wxGridCellAttr& attr,
                                             wxDC& dc,
                                             int row, int col)
{
    return DoGetBestSize(attr, dc, grid.GetCellValue(row, col));
}

void wxGridCellStringRenderer::Draw(wxGrid& grid,
                                    wxGridCellAttr& attr,
                                    wxDC& dc,
                                    const wxRect& rectCell,
                                    int row, int col,
                                    bool isSelected)
{
    wxRect rect = rectCell;
    rect.Inflate(-1);

    // erase only this cells background, overflow cells should have been erased
    wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);

    if ( attr.CanOverflow() )
    {
        int hAlign, vAlign;
        attr.GetAlignment(&hAlign, &vAlign);

        int overflowCols = 0;
        int cols = grid.GetNumberCols();
        int best_width = GetBestSize(grid,attr,dc,row,col).x;
        int cell_rows, cell_cols;
        attr.GetSize( &cell_rows, &cell_cols ); // shouldn't get here if <= 0
        if ((best_width > rectCell.width) && (col < cols) && grid.GetTable())
        {
            int i, c_cols, c_rows;
            for (i = col+cell_cols; i < cols; i++)
            {
                bool is_empty = true;
                for (int j=row; j < row + cell_rows; j++)
                {
                    // check w/ anchor cell for multicell block
                    grid.GetCellSize(j, i, &c_rows, &c_cols);
                    if (c_rows > 0)
                        c_rows = 0;
                    if (!grid.GetTable()->IsEmptyCell(j + c_rows, i))
                    {
                        is_empty = false;
                        break;
                    }
                }

                if (is_empty)
                {
                    rect.width += grid.GetColSize(i);
                }
                else
                {
                    i--;
                    break;
                }

                if (rect.width >= best_width)
                    break;
            }

            overflowCols = i - col - cell_cols + 1;
            if (overflowCols >= cols)
                overflowCols = cols - 1;
        }

        if (overflowCols > 0) // redraw overflow cells w/ proper hilight
        {
            hAlign = wxALIGN_LEFT; // if oveflowed then it's left aligned
            wxRect clip = rect;
            clip.x += rectCell.width;
            // draw each overflow cell individually
            int col_end = col + cell_cols + overflowCols;
            if (col_end >= grid.GetNumberCols())
                col_end = grid.GetNumberCols() - 1;
            for (int i = col + cell_cols; i <= col_end; i++)
            {
                // redraw the cell to update the background
                wxGridCellCoords coords(row, i);
                grid.DrawCell(dc, coords);

                clip.width = grid.GetColSize(i) - 1;
                wxDCClipper clipper(dc, clip);

                SetTextColoursAndFont(grid, attr, dc,
                        grid.IsInSelection(row,i));

                grid.DrawTextRectangle(dc, grid.GetCellValue(row, col),
                        rect, hAlign, vAlign);
                clip.x += grid.GetColSize(i) - 1;
            }

            rect = rectCell;
            rect.Inflate(-1);
            rect.width++;
        }
    }

    // now we only have to draw the text
    SetTextColoursAndFont(grid, attr, dc, isSelected);

    grid.DrawTextRectangle(dc, grid.GetCellValue(row, col),
                           rect, attr);
}

// ----------------------------------------------------------------------------
// wxGridCellNumberRenderer
// ----------------------------------------------------------------------------

std::string wxGridCellNumberRenderer::GetString(const wxGrid& grid, int row, int col)
{
    wxGridTableBase *table = grid.GetTable();

    if ( table->CanGetValueAs(row, col, wxGRID_VALUE_NUMBER) )
    {
        return fmt::format("{}", table->GetValueAsLong(row, col));
    }
    else
    {
        return table->GetValue(row, col);
    }
}

void wxGridCellNumberRenderer::Draw(wxGrid& grid,
                                    wxGridCellAttr& attr,
                                    wxDC& dc,
                                    const wxRect& rectCell,
                                    int row, int col,
                                    bool isSelected)
{
    wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);

    SetTextColoursAndFont(grid, attr, dc, isSelected);

    wxRect rect = rectCell;
    rect.Inflate(-1);

    // draw the text right aligned by default
    grid.DrawTextRectangle(dc, GetString(grid, row, col), rect, attr,
                           wxALIGN_RIGHT);
}

wxSize wxGridCellNumberRenderer::GetBestSize(wxGrid& grid,
                                             wxGridCellAttr& attr,
                                             wxDC& dc,
                                             int row, int col)
{
    return DoGetBestSize(attr, dc, GetString(grid, row, col));
}

wxSize wxGridCellNumberRenderer::GetMaxBestSize([[maybe_unused]] wxGrid& grid,
                                                wxGridCellAttr& attr,
                                                wxDC& dc)
{
    // In theory, it's possible that there is a value in min..max range which
    // is longer than both min and max, e.g. we could conceivably have "88" be
    // wider than both "87" and "91" with some fonts, but it seems something
    // too exotic to worry about in practice.
    wxSize size = DoGetBestSize(attr, dc, fmt::format("{}", m_minValue));
    size.IncTo(DoGetBestSize(attr, dc, fmt::format("{}", m_maxValue)));

    return size;
}

void wxGridCellNumberRenderer::SetParameters(const std::string& params)
{
    if ( params.empty() )
        return;

    auto paramSplit = wx::utils::StrSplit(params, ',');

    auto [min_p, min_err] = std::from_chars(paramSplit[0].data(), paramSplit[0].data() + paramSplit[0].size(), m_minValue);
    auto [max_p, max_err] = std::from_chars(paramSplit[1].data(), paramSplit[1].data() + paramSplit[1].size(), m_maxValue);

    if ( min_err != std::errc() || max_err != std::errc() )
    {
        wxLogDebug("Invalid wxGridCellNumberRenderer parameters \"%s\"", params);
    }
}

// ----------------------------------------------------------------------------
// wxGridCellFloatRenderer
// ----------------------------------------------------------------------------

wxGridCellFloatRenderer::wxGridCellFloatRenderer(int width,
                                                 int precision,
                                                 int format)
{
    SetWidth(width);
    SetPrecision(precision);
    SetFormat(format);
}

wxGridCellRenderer *wxGridCellFloatRenderer::Clone() const
{
    wxGridCellFloatRenderer *renderer = new wxGridCellFloatRenderer;
    renderer->m_width = m_width;
    renderer->m_precision = m_precision;
    renderer->m_style = m_style;
    renderer->m_format = m_format;

    return renderer;
}

std::string wxGridCellFloatRenderer::GetString(const wxGrid& grid, int row, int col)
{
    wxGridTableBase *table = grid.GetTable();

    bool hasDouble;
    double val;
    std::string text;
    std::from_chars_result fromDoubleResult{nullptr, std::errc{}};
    if ( table->CanGetValueAs(row, col, wxGRID_VALUE_FLOAT) )
    {
        val = table->GetValueAsDouble(row, col);
        hasDouble = true;
    }
    else
    {
        text = table->GetValue(row, col);

        fromDoubleResult = std::from_chars(text.data(), text.data() + text.size(), val);
    }

    if ( fromDoubleResult.ec == std::errc() )
    {
        if ( m_format.empty() )
        {
            if ( m_width == -1 )
            {
                if ( m_precision == -1 )
                {
                    // default width/precision
                    m_format = "%";
                }
                else
                {
                    m_format = fmt::format("%%.%d", m_precision);
                }
            }
            else if ( m_precision == -1 )
            {
                // default precision
                m_format = fmt::format("%%%d.", m_width);
            }
            else
            {
                m_format = fmt::format("%%%d.%d", m_width, m_precision);
            }

            bool isUpper = ( ( m_style & wxGRID_FLOAT_FORMAT_UPPER ) == wxGRID_FLOAT_FORMAT_UPPER);
            if ( ( m_style & wxGRID_FLOAT_FORMAT_SCIENTIFIC ) == wxGRID_FLOAT_FORMAT_SCIENTIFIC)
                m_format += isUpper ? 'E' : 'e';
            else if ( ( m_style & wxGRID_FLOAT_FORMAT_COMPACT ) == wxGRID_FLOAT_FORMAT_COMPACT)
                m_format += isUpper ? 'G' : 'g';
            else
                m_format += 'f';
        }

        text = fmt::format(fmt::runtime(m_format), val);

    }
    //else: text already contains the string

    return text;
}

void wxGridCellFloatRenderer::Draw(wxGrid& grid,
                                   wxGridCellAttr& attr,
                                   wxDC& dc,
                                   const wxRect& rectCell,
                                   int row, int col,
                                   bool isSelected)
{
    wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);

    SetTextColoursAndFont(grid, attr, dc, isSelected);

    wxRect rect = rectCell;
    rect.Inflate(-1);

    // draw the text right aligned by default
    grid.DrawTextRectangle(dc, GetString(grid, row, col), rect, attr,
                           wxALIGN_RIGHT);
}

wxSize wxGridCellFloatRenderer::GetBestSize(wxGrid& grid,
                                            wxGridCellAttr& attr,
                                            wxDC& dc,
                                            int row, int col)
{
    return DoGetBestSize(attr, dc, GetString(grid, row, col));
}

void wxGridCellFloatRenderer::SetParameters(const std::string& params)
{
    if ( params.empty() )
    {
        // reset to defaults
        SetWidth(-1);
        SetPrecision(-1);
        SetFormat(wxGRID_FLOAT_FORMAT_DEFAULT);
    }
    else
    {
        auto paramSplit = wx::utils::StrSplit(params, ',');

        auto [pWidth, errW] = std::from_chars(paramSplit[0].data(), paramSplit[0].data() + paramSplit[0].size(), m_width);
        auto [pPrec, errP] = std::from_chars(paramSplit[1].data(), paramSplit[1].data() + paramSplit[1].size(), m_precision);

        if (!paramSplit[0].empty() && errW != std::errc())
        {
            wxLogDebug("Invalid wxGridCellFloatRenderer width parameter string '%s ignored", params.c_str());
        }

        if (!paramSplit[1].empty() && errP != std::errc())
        {
            wxLogDebug("Invalid wxGridCellFloatRenderer precision parameter string '%s ignored", params.c_str());
        }

        if (!paramSplit[2].empty())
        {
            if (paramSplit[2] == 'f')
            {
                m_style = wxGRID_FLOAT_FORMAT_FIXED;
            }
            else if (paramSplit[2] == 'e')
            {
                m_style = wxGRID_FLOAT_FORMAT_SCIENTIFIC;
            }
            else if (paramSplit[2] == 'g')
            {
                m_style = wxGRID_FLOAT_FORMAT_COMPACT;
            }
            else if (paramSplit[2] == 'E')
            {
                m_style = wxGRID_FLOAT_FORMAT_SCIENTIFIC |
                    wxGRID_FLOAT_FORMAT_UPPER;
            }
            else if (paramSplit[2] == 'F')
            {
                m_style = wxGRID_FLOAT_FORMAT_FIXED |
                    wxGRID_FLOAT_FORMAT_UPPER;
            }
            else if (paramSplit[2] == 'G')
            {
                m_style = wxGRID_FLOAT_FORMAT_COMPACT |
                    wxGRID_FLOAT_FORMAT_UPPER;
            }
            else
            {
                wxLogDebug("Invalid wxGridCellFloatRenderer format "
                    "parameter string '%s ignored", params);
            }
        }
    }
}

// ----------------------------------------------------------------------------
// wxGridCellBoolRenderer
// ----------------------------------------------------------------------------

wxSize wxGridCellBoolRenderer::GetBestSize(wxGrid& grid,
                                           wxGridCellAttr& attr,
                                           wxDC& dc,
                                           [[maybe_unused]] int row,
                                           [[maybe_unused]] int col)
{
    return GetMaxBestSize(grid, attr, dc);
}

wxSize wxGridCellBoolRenderer::GetMaxBestSize(wxGrid& grid,
                                              [[maybe_unused]] wxGridCellAttr& attr,
                                              [[maybe_unused]] wxDC& dc)
{
    static wxPrivate::DpiDependentValue<wxSize> s_sizeCheckMark;

    // Get the check mark size in pixels if it hadn't been done yet or if the
    // DPI has changed.
    if ( s_sizeCheckMark.HasChanged(&grid) )
    {
        s_sizeCheckMark.SetAtNewDPI
            (
                wxRendererNative::Get().GetCheckBoxSize(&grid, wxCONTROL_CELL)
            );
    }

    return s_sizeCheckMark.Get();
}

void wxGridCellBoolRenderer::Draw(wxGrid& grid,
                                  wxGridCellAttr& attr,
                                  wxDC& dc,
                                  const wxRect& rect,
                                  int row, int col,
                                  bool isSelected)
{
    wxGridCellRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);

    int hAlign = wxALIGN_LEFT;
    int vAlign = wxALIGN_CENTRE_VERTICAL;
    attr.GetNonDefaultAlignment(&hAlign, &vAlign);

    const wxRect checkBoxRect =
        wxGetContentRect(GetBestSize(grid, attr, dc, row, col),
                         rect, hAlign, vAlign);

    bool value;
    if ( grid.GetTable()->CanGetValueAs(row, col, wxGRID_VALUE_BOOL) )
    {
        value = grid.GetTable()->GetValueAsBool(row, col);
    }
    else
    {
        value = wxGridCellBoolEditor::IsTrueValue(grid.GetTable()->GetValue(row, col));
    }

    int flags = wxCONTROL_CELL;
    if (value)
        flags |= wxCONTROL_CHECKED;

    wxRendererNative::Get().DrawCheckBox( &grid, dc, checkBoxRect, flags );
}
