/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/filtfind.cpp
// Purpose:     Streams for filter formats
// Author:      Mike Wetherell
// Copyright:   (c) Mike Wetherell
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_STREAMS

import WX.Cmn.Stream;

// These functions are in a separate file so that statically linked apps
// that do not call them to search for filter handlers will only link in
// the filter classes they use.

const wxFilterClassFactory *
wxFilterClassFactory::Find(const wxString& protocol, wxStreamProtocolType type)
{
    for (const wxFilterClassFactory *f = GetFirst(); f; f = f->GetNext())
        if (f->CanHandle(protocol, type))
            return f;

    return nullptr;
}

// static
const wxFilterClassFactory *wxFilterClassFactory::GetFirst()
{
    if (!sm_first)
        wxUseFilterClasses();
    return sm_first;
}

#endif // wxUSE_STREAMS
