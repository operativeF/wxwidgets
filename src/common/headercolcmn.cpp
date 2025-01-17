///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/headercolcmn.cpp
// Purpose:     wxHeaderColumn implementation
// Author:      Vadim Zeitlin
// Created:     2008-12-02
// Copyright:   (c) 2008 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_HEADERCTRL

#include "wx/headercol.h"

// ============================================================================
// wxHeaderColumn implementation
// ============================================================================

unsigned int wxHeaderColumn::GetFromIndividualFlags() const
{
    unsigned int flags = 0;

    if ( IsResizeable() )
        flags |= wxCOL_RESIZABLE;
    if ( IsSortable() )
        flags |= wxCOL_SORTABLE;
    if ( IsReorderable() )
        flags |= wxCOL_REORDERABLE;
    if ( IsHidden() )
        flags |= wxCOL_HIDDEN;

    return flags;
}

// ============================================================================
// wxSettableHeaderColumn implementation
// ============================================================================

void wxSettableHeaderColumn::SetIndividualFlags(unsigned int flags)
{
    SetResizeable((flags & wxCOL_RESIZABLE) != 0);
    SetSortable((flags & wxCOL_SORTABLE) != 0);
    SetReorderable((flags & wxCOL_REORDERABLE) != 0);
    SetHidden((flags & wxCOL_HIDDEN) != 0);
}

void wxSettableHeaderColumn::ChangeFlag(unsigned int flag, bool set)
{
    if ( HasFlag(flag) != set )
        ToggleFlag(flag);
}

void wxSettableHeaderColumn::SetFlag(unsigned int flag)
{
    const int flags = GetFlags();
    if ( !(flags & flag) )
        SetFlags(flags | flag);
}

void wxSettableHeaderColumn::ClearFlag(unsigned int flag)
{
    const int flags = GetFlags();
    if ( flags & flag )
        SetFlags(flags & ~flag);
}

void wxSettableHeaderColumn::ToggleFlag(unsigned int flag)
{
    int flags = GetFlags();
    if ( flags & flag )
        flags &= ~flag;
    else
        flags |= flag;

    SetFlags(flags);
}

#endif // wxUSE_HEADERCTRL

