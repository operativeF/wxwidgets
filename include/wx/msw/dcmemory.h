/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/dcmemory.h
// Purpose:     wxMemoryDC class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DCMEMORY_H_
#define _WX_DCMEMORY_H_

#include "wx/dcmemory.h"
#include "wx/msw/dc.h"
#include "wx/geometry/rect.h"

class wxMemoryDCImpl: public wxMSWDCImpl
{
public:
    // A non-null wxDC will attempt to create a compatible DC
    wxMemoryDCImpl( wxMemoryDC *owner, wxDC* dc = nullptr);
    wxMemoryDCImpl( wxMemoryDC *owner, wxBitmap& bitmap );

    wxMemoryDCImpl& operator=(wxMemoryDCImpl&&) = delete;

    // override some base class virtuals
    void DoDrawRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height) override;
    wxSize DoGetSize() const override;
    void DoSelect(const wxBitmap& bitmap) override;

    wxBitmap DoGetAsBitmap(const wxRect* subrect) const override
    { return subrect == nullptr ? GetSelectedBitmap() : GetSelectedBitmap().GetSubBitmapOfHDC(*subrect, GetHDC() );}

protected:
    // create DC compatible with the given one or screen if dc == NULL
    bool CreateCompatible(wxDC *dc);

    wxDECLARE_CLASS(wxMemoryDCImpl);
};

#endif
    // _WX_DCMEMORY_H_
