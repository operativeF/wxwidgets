/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/filtall.cpp
// Purpose:     Link all filter streams
// Author:      Mike Wetherell
// Copyright:   (c) 2006 Mike Wetherell
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_STREAMS

#if wxUSE_ZLIB
import WX.Cmn.ZStream;
#endif

// Reference filter classes to ensure they are linked into a statically
// linked program that uses Find or GetFirst to look for an filter handler.
// It is in its own file so that the user can override this behaviour by
// providing their own implementation.

void wxUseFilterClasses()
{
#if wxUSE_ZLIB
    wxZlibClassFactory();
    wxGzipClassFactory();
#endif
}

#endif // wxUSE_STREAMS
