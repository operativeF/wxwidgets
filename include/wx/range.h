/////////////////////////////////////////////////////////////////////////////
// Name:        wx/range.h
// Purpose:     Range Value Class
// Author:      Stefan Csomor
// Modified by:
// Created:     2011-01-07
// Copyright:   (c) 2011 Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_RANGE_H_
#define _WX_RANGE_H_

#include "wx/defs.h"

class wxRange
{
public :
    wxRange() {}
    wxRange( int minVal, int maxVal) : m_minVal(minVal), m_maxVal(maxVal) {}
    int GetMin() const { return m_minVal; }
    int GetMax() const { return m_maxVal; }
private :
    int m_minVal{0};
    int m_maxVal{0};
};

#endif // _WX_RANGE_H_
