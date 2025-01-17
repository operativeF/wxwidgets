/////////////////////////////////////////////////////////////////////////////
// Name:        wx/dcmemory.h
// Purpose:     wxMemoryDC base header
// Author:      Julian Smart
// Modified by:
// Created:
// Copyright:   (c) Julian Smart
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DCMEMORY_H_BASE_
#define _WX_DCMEMORY_H_BASE_

#include "wx/dc.h"

class wxBitmap;


//-----------------------------------------------------------------------------
// wxMemoryDC
//-----------------------------------------------------------------------------

class wxMemoryDC: public wxDC
{
public:
    wxMemoryDC();
    wxMemoryDC( wxBitmap& bitmap );
    wxMemoryDC( wxDC *dc );

    // select the given bitmap to draw on it
    void SelectObject(wxBitmap& bmp);

    // select the given bitmap for read-only
    void SelectObjectAsSource(const wxBitmap& bmp);

    // get selected bitmap
    const wxBitmap& GetSelectedBitmap() const;
    wxBitmap& GetSelectedBitmap();
};


#endif
    // _WX_DCMEMORY_H_BASE_
