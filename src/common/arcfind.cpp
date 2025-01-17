/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/arcfind.cpp
// Purpose:     Streams for archive formats
// Author:      Mike Wetherell
// Copyright:   (c) Mike Wetherell
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_ARCHIVE_STREAMS

import WX.Cmn.ArchStream;

// These functions are in a separate file so that statically linked apps
// that do not call them to search for archive handlers will only link in
// the archive classes they use.

const wxArchiveClassFactory *
wxArchiveClassFactory::Find(const std::string& protocol, wxStreamProtocolType type)
{
    for (const wxArchiveClassFactory *f = GetFirst(); f; f = f->GetNext())
        if (f->CanHandle(protocol, type))
            return f;

    return nullptr;
}

// static
const wxArchiveClassFactory *wxArchiveClassFactory::GetFirst()
{
    if (!sm_first)
        wxUseArchiveClasses();
    return sm_first;
}

#endif // wxUSE_ARCHIVE_STREAMS
