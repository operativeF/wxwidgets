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


#if wxUSE_SOCKETS && wxUSE_STREAMS

import WX.Cmn.Stream;
import WX.File.Flags;

class wxSocketBase;

class wxSocketOutputStream : public wxOutputStream
{
public:
    wxSocketOutputStream(wxSocketBase& s);
    wxSocketOutputStream& operator=(wxSocketOutputStream&&) = delete;

protected:
    wxSocketBase *m_o_socket;

    size_t OnSysWrite(const void *buffer, size_t bufsize) override;

    // socket streams are both un-seekable and size-less streams:
    wxFileOffset OnSysTell() const override
        { return wxInvalidOffset; }
    wxFileOffset OnSysSeek([[maybe_unused]] wxFileOffset pos, [[maybe_unused]] wxSeekMode mode) override
        { return wxInvalidOffset; }
};

class wxSocketInputStream : public wxInputStream
{
public:
    explicit wxSocketInputStream(wxSocketBase& s);
    wxSocketInputStream& operator=(wxSocketInputStream&&) = delete;

protected:
    wxSocketBase *m_i_socket;

    size_t OnSysRead(void *buffer, size_t bufsize) override;

    // socket streams are both un-seekable and size-less streams:

    wxFileOffset OnSysTell() const override
        { return wxInvalidOffset; }
    wxFileOffset OnSysSeek([[maybe_unused]] wxFileOffset pos, [[maybe_unused]] wxSeekMode mode) override
        { return wxInvalidOffset; }
};

class wxSocketStream : public wxSocketInputStream,
                   public wxSocketOutputStream
{
public:
    wxSocketStream(wxSocketBase& s);
   wxSocketStream& operator=(wxSocketStream&&) = delete;
};

#endif
  // wxUSE_SOCKETS && wxUSE_STREAMS

#endif
  // __SCK_STREAM_H__
