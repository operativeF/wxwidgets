/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/gridcoord.h
// Purpose:     wxGrid coordinate structure
// Author:      Michael Bedward (based on code by Julian Smart, Robin Dunn)
// Modified by: Santiago Palacios
// Created:     1/08/1999
// Copyright:   (c) Michael Bedward
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GRID_COORD
#define _WX_GRID_COORD

// ----------------------------------------------------------------------------
// wxGridCellCoords: location of a cell in the grid
// ----------------------------------------------------------------------------

struct WXDLLIMPEXP_CORE wxGridCellCoords
{
    constexpr wxGridCellCoords() {};
    constexpr wxGridCellCoords( int r, int c ) : m_row{r}, m_col{c} {}

    constexpr int GetRow() const { return m_row; }
    constexpr void SetRow( int n ) { m_row = n; }
    constexpr int GetCol() const { return m_col; }
    constexpr void SetCol( int n ) { m_col = n; }
    constexpr void Set( int row, int col ) { m_row = row; m_col = col; }

    constexpr bool operator==( const wxGridCellCoords& other ) const
    {
        return (m_row == other.m_row  &&  m_col == other.m_col);
    }

    constexpr bool operator!=( const wxGridCellCoords& other ) const
    {
        return !(*this == other);
    }

    constexpr bool operator!() const
    {
        return (m_row == -1 && m_col == -1 );
    }

    int m_row{-1};
    int m_col{-1};
};

constexpr wxGridCellCoords wxGridNoCellCoords{-1, -1};


#endif // _WX_GRID_COORD
