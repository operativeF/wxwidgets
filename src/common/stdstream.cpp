/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/stdstream.cpp
// Purpose:     Implementation of std::istream and std::ostream derived
//              wrappers for wxInputStream and wxOutputStream
// Author:      Jonathan Liu <net147@gmail.com>
// Created:     2009-05-02
// Copyright:   (c) 2009 Jonathan Liu
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ==========================================================================
// Declarations
// ==========================================================================

#if wxUSE_STREAMS

#include "wx/stdstream.h"

import <ios>;
import <istream>;
import <ostream>;
import <streambuf>;

import WX.File.Flags;

// ==========================================================================
// Helpers
// ==========================================================================

namespace
{

bool
IosSeekDirToWxSeekMode(std::ios_base::seekdir way,
                       wxSeekMode& seekMode)
{
    switch ( way )
    {
        case std::ios_base::beg:
            seekMode = wxSeekMode::FromStart;
            break;
        case std::ios_base::cur:
            seekMode = wxSeekMode::FromCurrent;
            break;
        case std::ios_base::end:
            seekMode = wxSeekMode::FromEnd;
            break;
        default:
            return false;
    }

    return true;
}

} // anonymous namespace

// ==========================================================================
// wxStdInputStreamBuffer
// ==========================================================================

wxStdInputStreamBuffer::wxStdInputStreamBuffer(wxInputStream& stream) :
    m_stream(stream), m_lastChar(EOF)
{
}

std::streambuf *
wxStdInputStreamBuffer::setbuf([[maybe_unused]] char *s,
                               [[maybe_unused]] std::streamsize n)
{
    return nullptr;
}

std::streampos
wxStdInputStreamBuffer::seekoff(std::streamoff off,
                                std::ios_base::seekdir way,
                                std::ios_base::openmode which)
{
    wxSeekMode seekMode;

    if ( !IosSeekDirToWxSeekMode(way, seekMode) )
        return -1;
    if ( !(which & std::ios_base::in) )
        return -1;

    const off_t newPos = m_stream.SeekI((off_t) off, seekMode);

    if ( newPos != wxInvalidOffset )
        return (std::streampos) newPos;
    else
        return -1;
}

std::streampos
wxStdInputStreamBuffer::seekpos(std::streampos sp,
                                std::ios_base::openmode which)
{
    if ( !(which & std::ios_base::in) )
        return -1;

    const off_t newPos = m_stream.SeekI((off_t) sp);

    if ( newPos != wxInvalidOffset )
        return (std::streampos) newPos;
    else
        return -1;
}

std::streamsize
wxStdInputStreamBuffer::showmanyc()
{
    if ( m_stream.CanRead() && (off_t) m_stream.GetSize() > m_stream.TellI() )
        return m_stream.GetSize() - m_stream.TellI();
    else
        return 0;
}

std::streamsize
wxStdInputStreamBuffer::xsgetn(char *s, std::streamsize n)
{
    m_stream.Read((void *) s, (size_t) n);

    std::streamsize read = m_stream.LastRead();

    if ( read > 0 )
        m_lastChar = (unsigned char) s[read - 1];

    return read;
}

int
wxStdInputStreamBuffer::underflow()
{
    int ch = m_stream.GetC();

    if ( m_stream.LastRead() == 1 )
    {
        m_stream.Ungetch((char) ch);
        return ch;
    }
    else
    {
        return EOF;
    }
}

int
wxStdInputStreamBuffer::uflow()
{
    const int ch = m_stream.GetC();

    if ( m_stream.LastRead() == 1 )
    {
        m_lastChar = ch;
        return ch;
    }
    else
    {
        return EOF;
    }
}

int
wxStdInputStreamBuffer::pbackfail(int c)
{
    if ( c == EOF )
    {
        if ( m_lastChar == EOF )
            return EOF;

        c = m_lastChar;
        m_lastChar = EOF;
    }

    return m_stream.Ungetch((char) c) ? c : EOF;
}

// ==========================================================================
// wxStdOutputStreamBuffer
// ==========================================================================

wxStdOutputStreamBuffer::wxStdOutputStreamBuffer(wxOutputStream& stream) :
    m_stream(stream)
{
}

std::streambuf *
wxStdOutputStreamBuffer::setbuf([[maybe_unused]] char *s,
                                [[maybe_unused]] std::streamsize n)
{
    return nullptr;
}

std::streampos
wxStdOutputStreamBuffer::seekoff(std::streamoff off,
                                 std::ios_base::seekdir way,
                                 std::ios_base::openmode which)
{
    wxSeekMode seekMode;

    if ( !IosSeekDirToWxSeekMode(way, seekMode) )
        return -1;
    if ( !(which & std::ios_base::out) )
        return -1;

    const off_t newPos = m_stream.SeekO((off_t) off, seekMode);

    if ( newPos != wxInvalidOffset )
        return (std::streampos) newPos;
    else
        return -1;
}

std::streampos
wxStdOutputStreamBuffer::seekpos(std::streampos sp,
                                 std::ios_base::openmode which)
{
    if ( !(which & std::ios_base::out) )
        return -1;

    off_t newPos = m_stream.SeekO((off_t) sp);

    if ( newPos != wxInvalidOffset )
        return (std::streampos) newPos;
    else
        return -1;
}

std::streamsize
wxStdOutputStreamBuffer::xsputn(const char *s,
                                std::streamsize n)
{
    m_stream.Write((const void *) s, (size_t) n);
    return (std::streamsize) m_stream.LastWrite();
}

int
wxStdOutputStreamBuffer::overflow(int c)
{
    m_stream.PutC(c);
    return m_stream.IsOk() ? c : EOF;
}

// ==========================================================================
// wxStdInputStream and wxStdOutputStream
// ==========================================================================

wxStdInputStream::wxStdInputStream(wxInputStream& stream) :
    std::istream(nullptr), m_streamBuffer(stream)
{
    std::ios::init(&m_streamBuffer);
}

wxStdOutputStream::wxStdOutputStream(wxOutputStream& stream) :
    std::ostream(nullptr), m_streamBuffer(stream)
{
    std::ios::init(&m_streamBuffer);
}

#endif // wxUSE_STREAMS
