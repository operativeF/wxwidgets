/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/sckstrm.cpp
// Purpose:     wxSocket*Stream
// Author:      Guilhem Lavaux
// Modified by:
// Created:     17/07/97
// Copyright:   (c)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_SOCKETS && wxUSE_STREAMS

#include "wx/sckstrm.h"
#include "wx/socket.h"

import WX.Cmn.Stream;

// ---------------------------------------------------------------------------
// wxSocketOutputStream
// ---------------------------------------------------------------------------

wxSocketOutputStream::wxSocketOutputStream(wxSocketBase& s)
  : m_o_socket(&s)
{
}

size_t wxSocketOutputStream::OnSysWrite(const void *buffer, size_t size)
{
    const size_t ret = m_o_socket->Write(buffer, size).LastWriteCount();
    m_lasterror = m_o_socket->Error()
                    ? m_o_socket->IsClosed() ? wxSTREAM_EOF
                                             : wxSTREAM_WRITE_ERROR
                    : wxSTREAM_NO_ERROR;
    return ret;
}

// ---------------------------------------------------------------------------
// wxSocketInputStream
// ---------------------------------------------------------------------------

wxSocketInputStream::wxSocketInputStream(wxSocketBase& s)
 : m_i_socket(&s)
{
}

size_t wxSocketInputStream::OnSysRead(void *buffer, size_t size)
{
    const size_t ret = m_i_socket->Read(buffer, size).LastReadCount();
    m_lasterror = m_i_socket->Error()
                    ? m_i_socket->IsClosed() ? wxSTREAM_EOF
                                             : wxSTREAM_READ_ERROR
                    : wxSTREAM_NO_ERROR;
    return ret;
}

// ---------------------------------------------------------------------------
// wxSocketStream
// ---------------------------------------------------------------------------

wxSocketStream::wxSocketStream(wxSocketBase& s)
  : wxSocketInputStream(s), wxSocketOutputStream(s)
{
}

#endif // wxUSE_STREAMS && wxUSE_SOCKETS
