/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/fs_filter.cpp
// Purpose:     wxFilter file system handler
// Author:      Mike Wetherell
// Copyright:   (c) 2006 Mike Wetherell
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_FILESYSTEM

#include "wx/fs_filter.h"
#include "wx/scopedptr.h"
#include "wx/stream.h"

wxDEFINE_SCOPED_PTR_TYPE(wxFSFile)
wxDEFINE_SCOPED_PTR_TYPE(wxInputStream)

//----------------------------------------------------------------------------
// wxFilterFSHandler
//----------------------------------------------------------------------------

bool wxFilterFSHandler::CanOpen(const std::string& location)
{
    return wxFilterClassFactory::Find(GetProtocol(location)) != nullptr;
}

wxFSFile* wxFilterFSHandler::OpenFile(
        wxFileSystem& fs,
        const std::string& location)
{
    std::string right = GetRightLocation(location);
    if (!right.empty())
        return nullptr;

    std::string protocol = GetProtocol(location);
    const wxFilterClassFactory *factory = wxFilterClassFactory::Find(protocol);
    if (!factory)
        return nullptr;

    std::string left = GetLeftLocation(location);
    wxFSFilePtr leftFile(fs.OpenFile(left));
    if (!leftFile.get())
        return nullptr;

    wxInputStreamPtr leftStream(leftFile->DetachStream());
    if (!leftStream.get() || !leftStream->IsOk())
        return nullptr;

    wxInputStreamPtr stream(factory->NewStream(leftStream.release()));

    // The way compressed streams are supposed to be served is e.g.:
    //  Content-type: application/postscript
    //  Content-encoding: gzip
    // So the mime type should be just the mime type of the lhs. However check
    // whether the mime type is that of this compression format (e.g.
    // application/gzip). If so pop any extension and try GetMimeTypeFromExt,
    // e.g. if it were '.ps.gz' pop the '.gz' and try looking up '.ps'
    std::string mime = leftFile->GetMimeType();
    if (factory->CanHandle(mime, wxSTREAM_MIMETYPE))
        mime = GetMimeTypeFromExt(factory->PopExtension(left));

    return new wxFSFile(stream.release(),
                        left + "#" + protocol + ":" + right,
                        mime,
                        GetAnchor(location)
#if wxUSE_DATETIME
                        , leftFile->GetModificationTime()
#endif // wxUSE_DATETIME
                       );
}

std::string wxFilterFSHandler::FindFirst(const std::string& WXUNUSED(spec), unsigned int WXUNUSED(flags))
{
    return {};
}

std::string wxFilterFSHandler::FindNext()
{
    return {};
}

#endif //wxUSE_FILESYSTEM
