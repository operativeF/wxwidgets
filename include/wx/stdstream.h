/////////////////////////////////////////////////////////////////////////////
// Name:        wx/stdstream.h
// Purpose:     Header of std::istream and std::ostream derived wrappers for
//              wxInputStream and wxOutputStream
// Author:      Jonathan Liu <net147@gmail.com>
// Created:     2009-05-02
// Copyright:   (c) 2009 Jonathan Liu
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_STDSTREAM_H_
#define _WX_STDSTREAM_H_

#if wxUSE_STREAMS

#include "wx/stream.h"

import <iostream>;

// ==========================================================================
// wxStdInputStreamBuffer
// ==========================================================================

class wxStdInputStreamBuffer : public std::streambuf
{
public:
    wxStdInputStreamBuffer(wxInputStream& stream);

protected:
    std::streambuf *setbuf(char *s, std::streamsize n) override;
    std::streampos seekoff(std::streamoff off,
                                   std::ios_base::seekdir way,
                                   std::ios_base::openmode which =
                                       std::ios_base::in |
                                       std::ios_base::out) override;
    std::streampos seekpos(std::streampos sp,
                                   std::ios_base::openmode which =
                                       std::ios_base::in |
                                       std::ios_base::out) override;
    std::streamsize showmanyc() override;
    std::streamsize xsgetn(char *s, std::streamsize n) override;
    int underflow() override;
    int uflow() override;
    int pbackfail(int c = EOF) override;

    wxInputStream& m_stream;
    int m_lastChar;
};

// ==========================================================================
// wxStdInputStream
// ==========================================================================

class wxStdInputStream : public std::istream
{
public:
    wxStdInputStream(wxInputStream& stream);

protected:
    wxStdInputStreamBuffer m_streamBuffer;
};

// ==========================================================================
// wxStdOutputStreamBuffer
// ==========================================================================

class wxStdOutputStreamBuffer : public std::streambuf
{
public:
    wxStdOutputStreamBuffer(wxOutputStream& stream);

protected:
    std::streambuf *setbuf(char *s, std::streamsize n) override;
    std::streampos seekoff(std::streamoff off,
                                   std::ios_base::seekdir way,
                                   std::ios_base::openmode which =
                                       std::ios_base::in |
                                       std::ios_base::out) override;
    std::streampos seekpos(std::streampos sp,
                                   std::ios_base::openmode which =
                                       std::ios_base::in |
                                       std::ios_base::out) override;
    std::streamsize xsputn(const char *s, std::streamsize n) override;
    int overflow(int c) override;

    wxOutputStream& m_stream;
};

// ==========================================================================
// wxStdOutputStream
// ==========================================================================

class wxStdOutputStream : public std::ostream
{
public:
    wxStdOutputStream(wxOutputStream& stream);

protected:
    wxStdOutputStreamBuffer m_streamBuffer;
};

#endif // wxUSE_STREAMS

#endif // _WX_STDSTREAM_H_
