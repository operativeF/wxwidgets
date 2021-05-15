/////////////////////////////////////////////////////////////////////////////
// Name:        wx/listimpl.cpp
// Purpose:     second-part of macro based implementation of template lists
// Author:      Vadim Zeitlin
// Modified by:
// Created:     16/11/98
// Copyright:   (c) 1998 Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#undef  WX_DEFINE_LIST
#define WX_DEFINE_LIST(name)                                                  \
    void _WX_LIST_HELPER_##name::DeleteFunction( _WX_LIST_ITEM_TYPE_##name X )\
    {                                                                         \
        delete X;                                                             \
    }                                                                         \
    _WX_LIST_HELPER_##name::BaseListType _WX_LIST_HELPER_##name::EmptyList;
