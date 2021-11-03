/////////////////////////////////////////////////////////////////////////////
// Name:        wx/position.h
// Purpose:     Common structure and methods for positional information.
// Author:      Vadim Zeitlin, Robin Dunn, Brad Anderson, Bryan Petty
// Created:     2007-03-13
// Copyright:   (c) 2007 The wxWidgets Team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_POSITION_H_
#define _WX_POSITION_H_

class wxPosition
{
public:
    constexpr wxPosition() noexcept = default;
    constexpr wxPosition(int row, int col) noexcept : m_row(row), m_column(col) {}

    // default copy ctor and assignment operator are okay.

    constexpr int GetRow() const noexcept          { return m_row; }
    constexpr int GetColumn() const noexcept       { return m_column; }
    constexpr int GetCol() const noexcept          { return GetColumn(); }
    constexpr void SetRow(int row) noexcept        { m_row = row; }
    constexpr void SetColumn(int column) noexcept  { m_column = column; }
    constexpr void SetCol(int column) noexcept     { SetColumn(column); }

    constexpr bool operator==(const wxPosition& p) const noexcept
        { return m_row == p.m_row && m_column == p.m_column; }
    constexpr bool operator!=(const wxPosition& p) const noexcept
        { return !(*this == p); }

    constexpr wxPosition& operator+=(const wxPosition& p) noexcept
        { m_row += p.m_row; m_column += p.m_column; return *this; }
    constexpr wxPosition& operator-=(const wxPosition& p) noexcept
        { m_row -= p.m_row; m_column -= p.m_column; return *this; }
    constexpr wxPosition& operator+=(const wxSize& s) noexcept
        { m_row += s.y; m_column += s.x; return *this; }
    constexpr wxPosition& operator-=(const wxSize& s) noexcept
        { m_row -= s.y; m_column -= s.x; return *this; }

    constexpr wxPosition operator+(const wxPosition& p) const noexcept
        { return wxPosition(m_row + p.m_row, m_column + p.m_column); }
    constexpr wxPosition operator-(const wxPosition& p) const noexcept
        { return wxPosition(m_row - p.m_row, m_column - p.m_column); }
    constexpr wxPosition operator+(const wxSize& s) const noexcept
        { return wxPosition(m_row + s.y, m_column + s.x); }
    constexpr wxPosition operator-(const wxSize& s) const noexcept
        { return wxPosition(m_row - s.y, m_column - s.x); }

private:
    int m_row{0};
    int m_column{0};
};

#endif // _WX_POSITION_H_

