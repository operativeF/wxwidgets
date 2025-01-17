////////////////////////////////////////////////////////////////////////////////
// Name:        src/common/list.cpp
// Purpose:     wxList implementation
// Author:      Julian Smart
// Modified by: VZ at 16/11/98: WX_DECLARE_LIST() and typesafe lists added
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
////////////////////////////////////////////////////////////////////////////////

#include "wx/list.h"
#include "wx/crt.h"

#include "wx/listimpl.cpp"
WX_DEFINE_LIST(wxObjectList)

// wxStringList contains wxString objects, not pointers
void _WX_LIST_HELPER_wxStringListBase::DeleteFunction( [[maybe_unused]] wxString X )
{
}

_WX_LIST_HELPER_wxStringListBase::BaseListType _WX_LIST_HELPER_wxStringListBase::EmptyList;
