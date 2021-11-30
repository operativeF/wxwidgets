///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/private/customdraw.h
// Purpose:     Helper for implementing custom drawing support in wxMSW
// Author:      Vadim Zeitlin
// Created:     2016-04-16
// Copyright:   (c) 2016 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_CUSTOMDRAW_H_
#define _WX_MSW_CUSTOMDRAW_H_

#include "wx/itemattr.h"

import WX.WinDef;

namespace wxMSWImpl
{

// ----------------------------------------------------------------------------
// CustomDraw: inherit from this class and forward NM_CUSTOMDRAW to it
// ----------------------------------------------------------------------------

class CustomDraw
{
public:
    // Virtual dtor for the base class.
    virtual ~CustomDraw() = default;

    CustomDraw& operator=(CustomDraw&&) = delete;

    // Implementation of NM_CUSTOMDRAW handler, returns one of CDRF_XXX
    // constants, possibly CDRF_DODEFAULT if custom drawing is not necessary.
    WXLPARAM HandleCustomDraw(WXLPARAM lParam);

private:
    // Return true if we need custom drawing at all.
    virtual bool HasCustomDrawnItems() const = 0;

    // Return the attribute to use for the given item, can return NULL if this
    // item doesn't need to be custom-drawn.
    virtual const wxItemAttr* GetItemAttr(WXDWORD_PTR dwItemSpec) const = 0;


    // Set the colours and font for the specified WXHDC, return CDRF_NEWFONT if
    // the font was changed.
    WXLPARAM HandleItemPrepaint(const wxItemAttr& attr, WXHDC hdc);
};

} // namespace wxMSWImpl

#endif // _WX_MSW_CUSTOMDRAW_H_
