///////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/gridctrl.h
// Purpose:     wxGrid controls
// Author:      Paul Gammans, Roger Gammans
// Modified by:
// Created:     11/04/2001
// Copyright:   (c) The Computer Surgery (paul@compsurg.co.uk)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_GRIDCTRL_H_
#define _WX_GENERIC_GRIDCTRL_H_

#include "wx/grid.h"

#if wxUSE_GRID

inline constexpr wxChar wxGRID_VALUE_CHOICEINT[]    = wxT("choiceint");
inline constexpr wxChar wxGRID_VALUE_DATETIME[]     = wxT("datetime");


// the default renderer for the cells containing string data
class WXDLLIMPEXP_CORE wxGridCellStringRenderer : public wxGridCellRenderer
{
public:
    // draw the string
    void Draw(wxGrid& grid,
                      wxGridCellAttr& attr,
                      wxDC& dc,
                      const wxRect& rect,
                      int row, int col,
                      bool isSelected) override;

    // return the string extent
    wxSize GetBestSize(wxGrid& grid,
                               wxGridCellAttr& attr,
                               wxDC& dc,
                               int row, int col) override;

    wxGridCellRenderer *Clone() const override
        { return new wxGridCellStringRenderer; }

protected:
    // calc the string extent for given string/font
    wxSize DoGetBestSize(const wxGridCellAttr& attr,
                         wxDC& dc,
                         const std::string& text);
};

// the default renderer for the cells containing numeric (long) data
class WXDLLIMPEXP_CORE wxGridCellNumberRenderer : public wxGridCellStringRenderer
{
public:
    explicit wxGridCellNumberRenderer(long minValue = LONG_MIN,
                                      long maxValue = LONG_MAX)
        : m_minValue(minValue),
          m_maxValue(maxValue)
    {
    }

    // draw the string right aligned
    void Draw(wxGrid& grid,
                      wxGridCellAttr& attr,
                      wxDC& dc,
                      const wxRect& rect,
                      int row, int col,
                      bool isSelected) override;

    wxSize GetBestSize(wxGrid& grid,
                               wxGridCellAttr& attr,
                               wxDC& dc,
                               int row, int col) override;

    wxSize GetMaxBestSize(wxGrid& grid,
                                  wxGridCellAttr& attr,
                                  wxDC& dc) override;

    // Optional parameters for this renderer are "<min>,<max>".
    void SetParameters(const wxString& params) override;

    wxGridCellRenderer *Clone() const override
        { return new wxGridCellNumberRenderer(m_minValue, m_maxValue); }

protected:
    std::string GetString(const wxGrid& grid, int row, int col);

    long m_minValue,
         m_maxValue;
};

class WXDLLIMPEXP_CORE wxGridCellFloatRenderer : public wxGridCellStringRenderer
{
public:
    wxGridCellFloatRenderer(int width = -1,
                            int precision = -1,
                            int format = wxGRID_FLOAT_FORMAT_DEFAULT);

    // get/change formatting parameters
    int GetWidth() const { return m_width; }
    void SetWidth(int width) { m_width = width; m_format.clear(); }
    int GetPrecision() const { return m_precision; }
    void SetPrecision(int precision) { m_precision = precision; m_format.clear(); }
    int GetFormat() const { return m_style; }
    void SetFormat(int format) { m_style = format; m_format.clear(); }

    // draw the string right aligned with given width/precision
    void Draw(wxGrid& grid,
                      wxGridCellAttr& attr,
                      wxDC& dc,
                      const wxRect& rect,
                      int row, int col,
                      bool isSelected) override;

    wxSize GetBestSize(wxGrid& grid,
                               wxGridCellAttr& attr,
                               wxDC& dc,
                               int row, int col) override;

    // parameters string format is "width[,precision[,format]]"
    // with format being one of f|e|g|E|F|G
    void SetParameters(const wxString& params) override;

    wxGridCellRenderer *Clone() const override;

protected:
    std::string GetString(const wxGrid& grid, int row, int col);

private:
    // formatting parameters
    int m_width,
        m_precision;

    int m_style;
    wxString m_format;
};

// renderer for boolean fields
class WXDLLIMPEXP_CORE wxGridCellBoolRenderer : public wxGridCellRenderer
{
public:
    // draw a check mark or nothing
    void Draw(wxGrid& grid,
                      wxGridCellAttr& attr,
                      wxDC& dc,
                      const wxRect& rect,
                      int row, int col,
                      bool isSelected) override;

    // return the checkmark size
    wxSize GetBestSize(wxGrid& grid,
                               wxGridCellAttr& attr,
                               wxDC& dc,
                               int row, int col) override;

    wxSize GetMaxBestSize(wxGrid& grid,
                                  wxGridCellAttr& attr,
                                  wxDC& dc) override;

    wxGridCellRenderer *Clone() const override
        { return new wxGridCellBoolRenderer; }
};


#if wxUSE_DATETIME

#include "wx/datetime.h"

namespace wxGridPrivate { class DateParseParams; }

// renderer for the cells containing dates only, without time component
class WXDLLIMPEXP_CORE wxGridCellDateRenderer : public wxGridCellStringRenderer
{
public:
    explicit wxGridCellDateRenderer(const std::string& outformat = {});

    wxGridCellDateRenderer(const wxGridCellDateRenderer& other)
        : m_oformat(other.m_oformat),
          m_tz(other.m_tz)
    {
    }

    // draw the string right aligned
    void Draw(wxGrid& grid,
                      wxGridCellAttr& attr,
                      wxDC& dc,
                      const wxRect& rect,
                      int row, int col,
                      bool isSelected) override;

    wxSize GetBestSize(wxGrid& grid,
                               wxGridCellAttr& attr,
                               wxDC& dc,
                               int row, int col) override;

    wxSize GetMaxBestSize(wxGrid& grid,
                                  wxGridCellAttr& attr,
                                  wxDC& dc) override;

    wxGridCellRenderer *Clone() const override;

    // output strptime()-like format string
    void SetParameters(const wxString& params) override;

protected:
    std::string GetString(const wxGrid& grid, int row, int col);

    // This is overridden in wxGridCellDateTimeRenderer which uses a separate
    // input format and forbids fallback to ParseDate().
    virtual void
    GetDateParseParams(wxGridPrivate::DateParseParams& params) const;

    wxString m_oformat;
    wxDateTime::TimeZone m_tz;
};

// the default renderer for the cells containing times and dates
class WXDLLIMPEXP_CORE wxGridCellDateTimeRenderer : public wxGridCellDateRenderer
{
public:
    wxGridCellDateTimeRenderer(const std::string& outformat = wxDefaultDateTimeFormat,
                               const std::string& informat = wxDefaultDateTimeFormat);

    wxGridCellDateTimeRenderer(const wxGridCellDateTimeRenderer& other) = default;

    wxGridCellRenderer *Clone() const override;

protected:
    void
    GetDateParseParams(wxGridPrivate::DateParseParams& params) const override;

    wxString m_iformat;
};

#endif // wxUSE_DATETIME

// Renderer for fields taking one of a limited set of values: this is the same
// as the renderer for strings, except that it can implement GetMaxBestSize().
class WXDLLIMPEXP_CORE wxGridCellChoiceRenderer : public wxGridCellStringRenderer
{
public:
    wxGridCellChoiceRenderer() = default;

    wxSize GetMaxBestSize(wxGrid& grid,
                                  wxGridCellAttr& attr,
                                  wxDC& dc) override;

    // Parameters string is a comma-separated list of values.
    void SetParameters(const wxString& params) override;

    wxGridCellRenderer *Clone() const override
    {
        return new wxGridCellChoiceRenderer(*this);
    }

protected:
    wxGridCellChoiceRenderer(const wxGridCellChoiceRenderer& other)
        : m_choices(other.m_choices)
    {
    }

    std::vector<wxString> m_choices;
};


// renders a number using the corresponding text string
class WXDLLIMPEXP_CORE wxGridCellEnumRenderer : public wxGridCellChoiceRenderer
{
public:
    wxGridCellEnumRenderer( const std::string& choices = {} );

    // draw the string right aligned
    void Draw(wxGrid& grid,
                      wxGridCellAttr& attr,
                      wxDC& dc,
                      const wxRect& rect,
                      int row, int col,
                      bool isSelected) override;

    wxSize GetBestSize(wxGrid& grid,
                               wxGridCellAttr& attr,
                               wxDC& dc,
                               int row, int col) override;

    wxGridCellRenderer *Clone() const override;

protected:
    std::string GetString(const wxGrid& grid, int row, int col);
};


class WXDLLIMPEXP_CORE wxGridCellAutoWrapStringRenderer : public wxGridCellStringRenderer
{
public:
    wxGridCellAutoWrapStringRenderer() : wxGridCellStringRenderer() { }

    void Draw(wxGrid& grid,
                      wxGridCellAttr& attr,
                      wxDC& dc,
                      const wxRect& rect,
                      int row, int col,
                      bool isSelected) override;

    wxSize GetBestSize(wxGrid& grid,
                               wxGridCellAttr& attr,
                               wxDC& dc,
                               int row, int col) override;

    int GetBestHeight(wxGrid& grid,
                              wxGridCellAttr& attr,
                              wxDC& dc,
                              int row, int col,
                              int width) override;

    int GetBestWidth(wxGrid& grid,
                              wxGridCellAttr& attr,
                              wxDC& dc,
                              int row, int col,
                              int height) override;

    wxGridCellRenderer *Clone() const override
        { return new wxGridCellAutoWrapStringRenderer; }

private:
    std::vector<wxString> GetTextLines( wxGrid& grid,
                                wxDC& dc,
                                const wxGridCellAttr& attr,
                                const wxRect& rect,
                                int row, int col);

    // Helper methods of GetTextLines()

    // Break a single logical line of text into several physical lines, all of
    // which are added to the lines array. The lines are broken at maxWidth and
    // the dc is used for measuring text extent only.
    void BreakLine(wxDC& dc,
                   const wxString& logicalLine,
                   wxCoord maxWidth,
                   std::vector<wxString>& lines);

    // Break a word, which is supposed to be wider than maxWidth, into several
    // lines, which are added to lines array and the last, incomplete, of which
    // is returned in line output parameter.
    //
    // Returns the width of the last line.
    wxCoord BreakWord(wxDC& dc,
                      const std::string& word,
                      wxCoord maxWidth,
                      std::vector<wxString>& lines,
                      wxString& line);


};

#endif  // wxUSE_GRID
#endif // _WX_GENERIC_GRIDCTRL_H_
