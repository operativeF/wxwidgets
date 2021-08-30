/////////////////////////////////////////////////////////////////////////////
// Name:        wx/sckstrm.h
// Purpose:     wxSocket*Stream
// Author:      Guilhem Lavaux
// Modified by:
// Created:     17/07/97
// Copyright:   (c)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////
#ifndef __SCK_STREAM_H__
#define __SCK_STREAM_H__

#include "wx/stream.h"

#if wxUSE_SOCKETS && wxUSE_STREAMS

#include "wx/socket.h"

class WXDLLIMPEXP_NET wxSocketOutputStream : public wxOutputStream
{
public:
    wxSocketOutputStream(wxSocketBase& s);
    ~wxSocketOutputStream() = default;

    wxSocketOutputStream(const wxSocketOutputStream&) = delete;
    wxSocketOutputStream& operator=(const wxSocketOutputStream&) = delete;
    wxSocketOutputStream(wxSocketOutputStream&&) = default;
    wxSocketOutputStream& operator=(wxSocketOutputStream&&) = default;

protected:
    wxSocketBase *m_o_socket;

    size_t OnSysWrite(const void *buffer, size_t bufsize) override;

    // socket streams are both un-seekable and size-less streams:
    wxFileOffset OnSysTell() const override
        { return wxInvalidOffset; }
    wxFileOffset OnSysSeek(wxFileOffset WXUNUSED(pos), wxSeekMode WXUNUSED(mode)) override
        { return wxInvalidOffset; }
};

class WXDLLIMPEXP_NET wxSocketInputStream : public wxInputStream
{
public:
    explicit wxSocketInputStream(wxSocketBase& s);
    ~wxSocketInputStream() = default;

    wxSocketInputStream(const wxSocketInputStream&) = delete;
    wxSocketInputStream& operator=(const wxSocketInputStream&) = delete;
    wxSocketInputStream(wxSocketInputStream&&) = default;
    wxSocketInputStream& operator=(wxSocketInputStream&&) = default;

protected:
    wxSocketBase *m_i_socket;

    size_t OnSysRead(void *buffer, size_t bufsize) override;

    // socket streams are both un-seekable and size-less streams:

    wxFileOffset OnSysTell() const override
        { return wxInvalidOffset; }
    wxFileOffset OnSysSeek(wxFileOffset WXUNUSED(pos), wxSeekMode WXUNUSED(mode)) override
        { return wxInvalidOffset; }
};

class WXDLLIMPEXP_NET wxSocketStream : public wxSocketInputStream,
                   public wxSocketOutputStream
{
public:
    wxSocketStream(wxSocketBase& s);
    ~wxSocketStream() = default;

   wxSocketStream(const wxSocketStream&) = delete;
   wxSocketStream& operator=(const wxSocketStream&) = delete;
   wxSocketStream(wxSocketStream&&) = default;
   wxSocketStream& operator=(wxSocketStream&&) = default;
};

#endif
  // wxUSE_SOCKETS && wxUSE_STREAMS

#endif
  // __SCK_STREAM_H__
