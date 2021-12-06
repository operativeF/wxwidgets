/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/fileback.cpp
// Purpose:     Back an input stream with memory or a file
// Author:      Mike Wetherell
// Copyright:   (c) 2006 Mike Wetherell
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_FILESYSTEM

#include "wx/private/fileback.h"
#include "wx/utils.h"
#include "wx/log.h"
#include "wx/private/filename.h"

// Prefer wxFFile unless wxFile has large file support but wxFFile does not.
//
#if wxUSE_FFILE && (defined wxHAS_LARGE_FFILES || !defined wxHAS_LARGE_FILES)
using wxBFFile = wxFFile;
constexpr bool wxBadSeek = false;
#else
using wxBFFile = wxFile;
static constexpe wxFileOffset wxBadSeek = wxInvalidOffset;
#endif

/////////////////////////////////////////////////////////////////////////////
// Backing file implementation

class wxBackingFileImpl
{
public:
    wxBackingFileImpl(wxInputStream *stream,
                      size_t bufsize,
                      const std::string& prefix);
    ~wxBackingFileImpl();

    void Release() { if (--m_refcount == 0) delete this; }
    wxBackingFileImpl *AddRef() { m_refcount++; return this; }

    wxStreamError ReadAt(wxFileOffset pos, void *buffer, size_t *size);
    wxFileOffset GetLength() const;

private:
    std::string m_prefix;
    std::string m_filename;

    wxBFFile m_file;

    wxInputStream *m_stream;
    char *m_buf{nullptr};

    wxFileOffset m_filelen{ 0 };

    size_t m_bufsize;
    size_t m_buflen{0};

    int m_refcount{ 1 };

    wxStreamError m_parenterror{ wxSTREAM_NO_ERROR };
};

wxBackingFileImpl::wxBackingFileImpl(wxInputStream *stream,
                                     size_t bufsize,
                                     const std::string& prefix) :
    m_stream(stream),
    m_bufsize(bufsize),
    m_prefix(prefix)
{
    const wxFileOffset len = m_stream->GetLength();

    if (len >= 0 && len + size_t{1} < m_bufsize)
        m_bufsize = wx::narrow_cast<size_t>(len + 1);

    if (m_bufsize)
        m_buf = new char[m_bufsize];
}

wxBackingFileImpl::~wxBackingFileImpl()
{
    delete m_stream;
    delete [] m_buf;

    if (!m_filename.empty())
        wxRemoveFile(m_filename);
}

wxStreamError wxBackingFileImpl::ReadAt(wxFileOffset pos,
                                        void *buffer,
                                        size_t *size)
{
    const size_t reqestedSize = *size;
    *size = 0;

    // size1 is the number of bytes it will read directly from the backing
    // file. size2 is any remaining bytes not yet backed, these are returned
    // from the buffer or read from the parent stream.
    size_t size1, size2;

    if (pos + reqestedSize <= m_filelen + size_t{0}) {
        size1 = reqestedSize;
        size2 = 0;
    } else if (pos < m_filelen) {
        size1 = size_t(m_filelen - pos);
        size2 = reqestedSize - size1;
    } else {
        size1 = 0;
        size2 = reqestedSize;
    }

    if (pos < 0)
        return wxSTREAM_READ_ERROR;

    // read the backing file
    if (size1) {
        if (m_file.Seek(pos) == wxBadSeek)
            return wxSTREAM_READ_ERROR;

        ssize_t n = m_file.Read(buffer, size1);
        if (n > 0) {
            *size = n;
            pos += n;
        }

        if (*size < size1)
            return wxSTREAM_READ_ERROR;
    }

    // read from the buffer or parent stream
    if (size2)
    {
        while (*size < reqestedSize)
        {
            // if pos is further ahead than the parent has been read so far,
            // then read forward in the parent stream
            while (pos - m_filelen + size_t{0} >= m_buflen)
            {
                // if the parent is small enough, don't use a backing file
                // just the buffer memory
                if (!m_stream && m_filelen == 0)
                    return m_parenterror;

                // before refilling the buffer write out the current buffer
                // to the backing file if there is anything in it
                if (m_buflen)
                {
                    if (!m_file.IsOpened())
                        if (!wxCreateTempFile(m_prefix, &m_file, &m_filename))
                            return wxSTREAM_READ_ERROR;

                    if (m_file.Seek(m_filelen) == wxBadSeek)
                        return wxSTREAM_READ_ERROR;

                    const size_t count = m_file.Write(m_buf, m_buflen);
                    m_filelen += count;

                    if (count < m_buflen) {
                        wxDELETE(m_stream);
                        if (count > 0) {
                            wxDELETEA(m_buf);
                            m_buflen = 0;
                        }
                        m_parenterror = wxSTREAM_READ_ERROR;
                        return m_parenterror;
                    }

                    m_buflen = 0;

                    if (!m_stream) {
                        wxDELETEA(m_buf);
                    }
                }

                if (!m_stream)
                    return m_parenterror;

                // refill buffer
                m_buflen = m_stream->Read(m_buf, m_bufsize).LastRead();

                if (m_buflen < m_bufsize) {
                    m_parenterror = m_stream->GetLastError();
                    if (m_parenterror == wxSTREAM_NO_ERROR)
                        m_parenterror = wxSTREAM_EOF;
                    wxDELETE(m_stream);
                }
            }

            // copy to the user's buffer
            const size_t start = size_t(pos - m_filelen);
            size_t len = std::min(m_buflen - start, reqestedSize - *size);

            memcpy((char*)buffer + *size, m_buf + start, len);
            *size += len;
            pos += len;
        }
    }

    return wxSTREAM_NO_ERROR;
}

wxFileOffset wxBackingFileImpl::GetLength() const
{
    if (m_parenterror != wxSTREAM_EOF) {
        wxLogNull nolog;
        return m_stream->GetLength();
    }
    return m_filelen + m_buflen;
}


/////////////////////////////////////////////////////////////////////////////
// Backing File, the handle part

wxBackingFile::wxBackingFile(wxInputStream *stream,
                             size_t bufsize,
                             const std::string& prefix)
  : m_impl(new wxBackingFileImpl(stream, bufsize, prefix))
{
}

wxBackingFile::wxBackingFile(const wxBackingFile& backer)
  : m_impl(backer.m_impl ? backer.m_impl->AddRef() : nullptr)
{
}

wxBackingFile& wxBackingFile::operator=(const wxBackingFile& backer)
{
    if (backer.m_impl != m_impl) {
        if (m_impl)
            m_impl->Release();

        m_impl = backer.m_impl;

        if (m_impl)
            m_impl->AddRef();
    }

    return *this;
}

wxBackingFile::~wxBackingFile()
{
    if (m_impl)
        m_impl->Release();
}


/////////////////////////////////////////////////////////////////////////////
// Input stream

wxBackedInputStream::wxBackedInputStream(const wxBackingFile& backer)
  : m_backer(backer),
    m_pos(0)
{
}

wxFileOffset wxBackedInputStream::GetLength() const
{
    return m_backer.m_impl->GetLength();
}

wxFileOffset wxBackedInputStream::FindLength() const
{
    wxFileOffset len = GetLength();

    if (len == wxInvalidOffset && IsOk()) {
        // read a byte at 7ff...ffe
        wxFileOffset pos = 1;
        pos <<= sizeof(pos) * 8 - 1;
        pos = ~pos - 1;
        char ch;
        size_t size = 1;
        m_backer.m_impl->ReadAt(pos, &ch, &size);
        len = GetLength();
    }

    return len;
}

size_t wxBackedInputStream::OnSysRead(void *buffer, size_t size)
{
    if (!IsOk())
        return 0;

    m_lasterror = m_backer.m_impl->ReadAt(m_pos, buffer, &size);
    m_pos += size;
    return size;
}

wxFileOffset wxBackedInputStream::OnSysSeek(wxFileOffset pos, wxSeekMode mode)
{
    switch (mode) {
        case wxSeekMode::FromCurrent:
        {
            m_pos += pos;
            break;
        }
        case wxSeekMode::FromEnd:
        {
            const wxFileOffset len = GetLength();
            if (len == wxInvalidOffset)
                return wxInvalidOffset;
            m_pos = len + pos;
            break;
        }
        default:
        {
            m_pos = pos;
            break;
        }
    }

    return m_pos;
}

wxFileOffset wxBackedInputStream::OnSysTell() const
{
    return m_pos;
}

#endif // wxUSE_FILESYSTEM
