///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/ownerdrw.h
// Purpose:     wxOwnerDrawn class
// Author:      Marcin Malich
// Modified by:
// Created:     2009-09-22
// Copyright:   (c) 2009 Marcin Malich <me@malcom.pl>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_OWNERDRW_H_
#define _WX_OWNERDRW_H_

#if wxUSE_OWNER_DRAWN

import Utils.Geometry;

struct wxOwnerDrawn : public wxOwnerDrawnBase
{
    bool OnDrawItem(wxDC& dc, const wxRect& rc,
                            wxODAction act, wxODStatus stat) override;
};

#endif // wxUSE_OWNER_DRAWN

#endif // _WX_OWNERDRW_H_
