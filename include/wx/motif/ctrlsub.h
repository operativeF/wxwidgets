///////////////////////////////////////////////////////////////////////////////
// Name:        wx/motif/ctrlsub.h
// Purpose:     common functionality of wxItemContainer-derived controls
// Author:      Vadim Zeitlin
// Created:     2007-07-25
// Copyright:   (c) 2007 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MOTIF_CTRLSUB_H_
#define _WX_MOTIF_CTRLSUB_H_

#include "wx/dynarray.h"
#include "wx/generic/ctrlsub.h"

// ----------------------------------------------------------------------------
// wxControlWithItems
// ----------------------------------------------------------------------------

class wxControlWithItems : public wxControlWithItemsGeneric
{
public:
    wxControlWithItems() { }

protected:
    // Motif functions inserting items in the control interpret positions
    // differently from wx: they're 1-based and 0 means to append
    unsigned int GetMotifPosition(unsigned int pos) const
    {
        return pos == GetCount() ? 0 : pos + 1;
    }

private:
    wxDECLARE_ABSTRACT_CLASS(wxControlWithItems);
    wxControlWithItems(const wxControlWithItems&) = delete;
	wxControlWithItems& operator=(const wxControlWithItems&) = delete;
};

#endif // _WX_MOTIF_CTRLSUB_H_

