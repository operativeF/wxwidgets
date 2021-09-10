/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/datstrm.cpp
// Purpose:     Data stream classes
// Author:      Guilhem Lavaux
// Modified by: Mickael Gilabert
// Created:     28/06/98
// Copyright:   (c) Guilhem Lavaux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_STREAMS

#include "wx/datstrm.h"

namespace
{

// helper unions used to swap bytes of floats and doubles
union Float32Data
{
    float f;
    std::uint32_t i;
};

union Float64Data
{
    double f;
    std::uint32_t i[2];
};

} // anonymous namespace

// ----------------------------------------------------------------------------
// wxDataStreamBase
// ----------------------------------------------------------------------------

wxDataStreamBase::wxDataStreamBase(const wxMBConv& conv)
    : m_conv(conv.Clone())
{
    // It is unused in non-Unicode build, so suppress a warning there.
    wxUnusedVar(conv);

}

void wxDataStreamBase::SetConv( const wxMBConv &conv )
{
    delete m_conv;
    m_conv = conv.Clone();
}

wxDataStreamBase::~wxDataStreamBase()
{
    delete m_conv;
}

// ---------------------------------------------------------------------------
// wxDataInputStream
// ---------------------------------------------------------------------------

wxDataInputStream::wxDataInputStream(wxInputStream& s, const wxMBConv& conv)
  : wxDataStreamBase(conv),
    m_input(&s)
{
}

#if wxHAS_INT64
std::uint64_t wxDataInputStream::Read64()
{
  // TODO: Return value
  std::uint64_t tmp;
  Read64(&tmp, 1);
  return tmp;
}
#endif // wxHAS_INT64

std::uint32_t wxDataInputStream::Read32()
{
  // TODO: Return value
  std::uint32_t i32;

  m_input->Read(&i32, 4);

  if (m_be_order)
    return wxUINT32_SWAP_ON_LE(i32);
  else
    return wxUINT32_SWAP_ON_BE(i32);
}

std::uint16_t wxDataInputStream::Read16()
{
  // TODO: Return value.
  std::uint16_t i16;

  m_input->Read(&i16, 2);

  if (m_be_order)
    return wxUINT16_SWAP_ON_LE(i16);
  else
    return wxUINT16_SWAP_ON_BE(i16);
}

std::uint8_t wxDataInputStream::Read8()
{
  std::uint8_t buf;

  m_input->Read(&buf, 1);
  return (std::uint8_t)buf;
}

double wxDataInputStream::ReadDouble()
{
    Float64Data floatData;

    if ( m_be_order == (wxBYTE_ORDER == wxBIG_ENDIAN) )
    {
        floatData.i[0] = Read32();
        floatData.i[1] = Read32();
    }
    else
    {
        floatData.i[1] = Read32();
        floatData.i[0] = Read32();
    }

    return static_cast<double>(floatData.f);
}

float wxDataInputStream::ReadFloat()
{
  Float32Data floatData;

  floatData.i = Read32();
  return static_cast<float>(floatData.f);
}

wxString wxDataInputStream::ReadString()
{
    wxString ret;

    const size_t len = Read32();
    if ( len > 0 )
    {
        wxCharBuffer tmp(len);
        if ( tmp )
        {
            m_input->Read(tmp.data(), len);
            ret = m_conv->cMB2WC(tmp.data(), len, nullptr);
        }
    }

    return ret;
}

#if wxUSE_LONGLONG

template <class T>
static
void DoReadLL(T *buffer, size_t size, wxInputStream *input, bool be_order)
{
    using DataType = T;
    unsigned char *pchBuffer = new unsigned char[size * 8];
    // TODO: Check for overflow when size is of type uint and is > than 512m
    input->Read(pchBuffer, size * 8);
    size_t idx_base = 0;
    if ( be_order )
    {
        for ( size_t uiIndex = 0; uiIndex != size; ++uiIndex )
        {
            buffer[uiIndex] = 0l;
            for ( unsigned ui = 0; ui != 8; ++ui )
            {
                buffer[uiIndex] = buffer[uiIndex] * 256l +
                            DataType((unsigned long) pchBuffer[idx_base + ui]);
            }

            idx_base += 8;
        }
    }
    else // little endian
    {
        for ( size_t uiIndex=0; uiIndex!=size; ++uiIndex )
        {
            buffer[uiIndex] = 0l;
            for ( unsigned ui=0; ui!=8; ++ui )
                buffer[uiIndex] = buffer[uiIndex] * 256l +
                    DataType((unsigned long) pchBuffer[idx_base + 7 - ui]);
            idx_base += 8;
        }
    }
    delete[] pchBuffer;
}

template <class T>
static void DoWriteLL(const T *buffer, size_t size, wxOutputStream *output, bool be_order)
{
    using DataType = T;
    unsigned char *pchBuffer = new unsigned char[size * 8];
    size_t idx_base = 0;
    if ( be_order )
    {
        for ( size_t uiIndex = 0; uiIndex != size; ++uiIndex )
        {
            DataType i64 = buffer[uiIndex];
            for ( unsigned ui = 0; ui != 8; ++ui )
            {
                pchBuffer[idx_base + 7 - ui] =
                    (unsigned char) (i64.GetLo() & 255l);
                i64 >>= 8l;
            }

            idx_base += 8;
        }
    }
    else // little endian
    {
        for ( size_t uiIndex=0; uiIndex != size; ++uiIndex )
        {
            DataType i64 = buffer[uiIndex];
            for (unsigned ui=0; ui!=8; ++ui)
            {
                pchBuffer[idx_base + ui] =
                    (unsigned char) (i64.GetLo() & 255l);
                i64 >>= 8l;
            }

            idx_base += 8;
        }
    }

    // TODO: Check for overflow when size is of type uint and is > than 512m
    output->Write(pchBuffer, size * 8);
    delete[] pchBuffer;
}

#endif // wxUSE_LONGLONG

#ifdef wxLongLong_t

template <class T>
static
void DoReadI64(T *buffer, size_t size, wxInputStream *input, bool be_order)
{
    using DataType = T;
    unsigned char *pchBuffer = (unsigned char*) buffer;
    // TODO: Check for overflow when size is of type uint and is > than 512m
    input->Read(pchBuffer, size * 8);
    if ( be_order )
    {
        for ( std::uint32_t i = 0; i < size; i++ )
        {
            DataType v = wxUINT64_SWAP_ON_LE(*buffer);
            *(buffer++) = v;
        }
    }
    else // little endian
    {
        for ( std::uint32_t i=0; i<size; i++ )
        {
            DataType v = wxUINT64_SWAP_ON_BE(*buffer);
            *(buffer++) = v;
        }
    }
}

template <class T>
static
void DoWriteI64(const T *buffer, size_t size, wxOutputStream *output, bool be_order)
{
  using DataType = T;
  if ( be_order )
  {
    for ( size_t i = 0; i < size; i++ )
    {
      DataType i64 = wxUINT64_SWAP_ON_LE(*buffer);
      buffer++;
      output->Write(&i64, 8);
    }
  }
  else // little endian
  {
    for ( size_t i=0; i < size; i++ )
    {
      DataType i64 = wxUINT64_SWAP_ON_BE(*buffer);
      buffer++;
      output->Write(&i64, 8);
    }
  }
}

#endif // wxLongLong_t


#if wxHAS_INT64
void wxDataInputStream::Read64(std::uint64_t *buffer, size_t size)
{
#ifndef wxLongLong_t
    DoReadLL(buffer, size, m_input, m_be_order);
#else
    DoReadI64(buffer, size, m_input, m_be_order);
#endif
}

void wxDataInputStream::Read64(std::int64_t *buffer, size_t size)
{
#ifndef wxLongLong_t
    DoReadLL(buffer, size, m_input, m_be_order);
#else
    DoReadI64(buffer, size, m_input, m_be_order);
#endif
}
#endif // wxHAS_INT64

#if defined(wxLongLong_t) && wxUSE_LONGLONG
void wxDataInputStream::Read64(wxULongLong *buffer, size_t size)
{
    DoReadLL(buffer, size, m_input, m_be_order);
}

void wxDataInputStream::Read64(wxLongLong *buffer, size_t size)
{
    DoReadLL(buffer, size, m_input, m_be_order);
}
#endif // wxLongLong_t

#if wxUSE_LONGLONG
void wxDataInputStream::ReadLL(wxULongLong *buffer, size_t size)
{
    DoReadLL(buffer, size, m_input, m_be_order);
}

void wxDataInputStream::ReadLL(wxLongLong *buffer, size_t size)
{
    DoReadLL(buffer, size, m_input, m_be_order);
}

wxLongLong wxDataInputStream::ReadLL()
{
    wxLongLong ll;
    DoReadLL(&ll, (size_t)1, m_input, m_be_order);
    return ll;
}
#endif // wxUSE_LONGLONG

void wxDataInputStream::Read32(std::uint32_t *buffer, size_t size)
{
    m_input->Read(buffer, size * 4);

    if (m_be_order)
    {
        for (std::uint32_t i=0; i<size; i++)
        {
            std::uint32_t v = wxUINT32_SWAP_ON_LE(*buffer);
            *(buffer++) = v;
        }
    }
    else
    {
        for (std::uint32_t i=0; i<size; i++)
        {
            std::uint32_t v = wxUINT32_SWAP_ON_BE(*buffer);
            *(buffer++) = v;
        }
    }
}

void wxDataInputStream::Read16(std::uint16_t *buffer, size_t size)
{
  m_input->Read(buffer, size * 2);

  if (m_be_order)
  {
    for (std::uint32_t i=0; i<size; i++)
    {
      std::uint16_t v = wxUINT16_SWAP_ON_LE(*buffer);
      *(buffer++) = v;
    }
  }
  else
  {
    for (std::uint32_t i=0; i<size; i++)
    {
      std::uint16_t v = wxUINT16_SWAP_ON_BE(*buffer);
      *(buffer++) = v;
    }
  }
}

void wxDataInputStream::Read8(std::uint8_t *buffer, size_t size)
{
  m_input->Read(buffer, size);
}

void wxDataInputStream::ReadDouble(double *buffer, size_t size)
{
  for (std::uint32_t i=0; i<size; i++)
  {
    *(buffer++) = ReadDouble();
  }
}

void wxDataInputStream::ReadFloat(float *buffer, size_t size)
{
  for (std::uint32_t i=0; i<size; i++)
  {
    *(buffer++) = ReadFloat();
  }
}

wxDataInputStream& wxDataInputStream::operator>>(wxString& s)
{
  s = ReadString();
  return *this;
}

wxDataInputStream& wxDataInputStream::operator>>(std::int8_t& c)
{
  c = (std::int8_t)Read8();
  return *this;
}

wxDataInputStream& wxDataInputStream::operator>>(std::int16_t& i)
{
  i = (std::int16_t)Read16();
  return *this;
}

wxDataInputStream& wxDataInputStream::operator>>(std::int32_t& i)
{
  i = (std::int32_t)Read32();
  return *this;
}

wxDataInputStream& wxDataInputStream::operator>>(std::uint8_t& c)
{
  c = Read8();
  return *this;
}

wxDataInputStream& wxDataInputStream::operator>>(std::uint16_t& i)
{
  i = Read16();
  return *this;
}

wxDataInputStream& wxDataInputStream::operator>>(std::uint32_t& i)
{
  i = Read32();
  return *this;
}

#if wxHAS_INT64
wxDataInputStream& wxDataInputStream::operator>>(std::uint64_t& i)
{
  i = Read64();
  return *this;
}

wxDataInputStream& wxDataInputStream::operator>>(std::int64_t& i)
{
  i = Read64();
  return *this;
}
#endif // wxHAS_INT64

#if defined(wxLongLong_t) && wxUSE_LONGLONG
wxDataInputStream& wxDataInputStream::operator>>(wxULongLong& i)
{
  i = ReadLL();
  return *this;
}

wxDataInputStream& wxDataInputStream::operator>>(wxLongLong& i)
{
  i = ReadLL();
  return *this;
}
#endif // wxLongLong_t

wxDataInputStream& wxDataInputStream::operator>>(double& d)
{
  d = ReadDouble();
  return *this;
}

wxDataInputStream& wxDataInputStream::operator>>(float& f)
{
  f = ReadFloat();
  return *this;
}

// ---------------------------------------------------------------------------
// wxDataOutputStream
// ---------------------------------------------------------------------------

wxDataOutputStream::wxDataOutputStream(wxOutputStream& s, const wxMBConv& conv)
  : wxDataStreamBase(conv),
    m_output(&s)
{
}

#if wxHAS_INT64
void wxDataOutputStream::Write64(std::uint64_t i)
{
  Write64(&i, 1);
}

void wxDataOutputStream::Write64(std::int64_t i)
{
  Write64(&i, 1);
}
#endif // wxHAS_INT64

void wxDataOutputStream::Write32(std::uint32_t i)
{
  std::uint32_t i32;

  if (m_be_order)
    i32 = wxUINT32_SWAP_ON_LE(i);
  else
    i32 = wxUINT32_SWAP_ON_BE(i);
  m_output->Write(&i32, 4);
}

void wxDataOutputStream::Write16(std::uint16_t i)
{
  std::uint16_t i16;

  if (m_be_order)
    i16 = wxUINT16_SWAP_ON_LE(i);
  else
    i16 = wxUINT16_SWAP_ON_BE(i);

  m_output->Write(&i16, 2);
}

void wxDataOutputStream::Write8(std::uint8_t i)
{
  m_output->Write(&i, 1);
}

void wxDataOutputStream::WriteString(const wxString& string)
{
  const wxWX2MBbuf buf = string.mb_str(*m_conv);
  const size_t len = buf.length();

  Write32(len);
  if (len > 0)
      m_output->Write(buf, len);
}

void wxDataOutputStream::WriteDouble(double d)
{

    Float64Data floatData;

    floatData.f = d;

    if ( m_be_order == (wxBYTE_ORDER == wxBIG_ENDIAN) )
    {
        Write32(floatData.i[0]);
        Write32(floatData.i[1]);
    }
    else
    {
        Write32(floatData.i[1]);
        Write32(floatData.i[0]);
    }
}

void wxDataOutputStream::WriteFloat(float f)
{
    Float32Data floatData;

    floatData.f = f;
    Write32(floatData.i);
}

#if wxHAS_INT64
void wxDataOutputStream::Write64(const std::uint64_t *buffer, size_t size)
{
#ifndef wxLongLong_t
    DoWriteLL(buffer, size, m_output, m_be_order);
#else
    DoWriteI64(buffer, size, m_output, m_be_order);
#endif
}

void wxDataOutputStream::Write64(const std::int64_t *buffer, size_t size)
{
#ifndef wxLongLong_t
    DoWriteLL(buffer, size, m_output, m_be_order);
#else
    DoWriteI64(buffer, size, m_output, m_be_order);
#endif
}
#endif // wxHAS_INT64

#if defined(wxLongLong_t) && wxUSE_LONGLONG
void wxDataOutputStream::Write64(const wxULongLong *buffer, size_t size)
{
    DoWriteLL(buffer, size, m_output, m_be_order);
}

void wxDataOutputStream::Write64(const wxLongLong *buffer, size_t size)
{
    DoWriteLL(buffer, size, m_output, m_be_order);
}
#endif // wxLongLong_t

#if wxUSE_LONGLONG
void wxDataOutputStream::WriteLL(const wxULongLong *buffer, size_t size)
{
    DoWriteLL(buffer, size, m_output, m_be_order);
}

void wxDataOutputStream::WriteLL(const wxLongLong *buffer, size_t size)
{
    DoWriteLL(buffer, size, m_output, m_be_order);
}

void wxDataOutputStream::WriteLL(const wxLongLong &ll)
{
    WriteLL(&ll, 1);
}

void wxDataOutputStream::WriteLL(const wxULongLong &ll)
{
    WriteLL(&ll, 1);
}
#endif // wxUSE_LONGLONG

void wxDataOutputStream::Write32(const std::uint32_t *buffer, size_t size)
{
  if (m_be_order)
  {
    for (std::uint32_t i=0; i<size ;i++)
    {
      const std::uint32_t i32 = wxUINT32_SWAP_ON_LE(*buffer);
      buffer++;
      m_output->Write(&i32, 4);
    }
  }
  else
  {
    for (std::uint32_t i=0; i<size ;i++)
    {
      const std::uint32_t i32 = wxUINT32_SWAP_ON_BE(*buffer);
      buffer++;
      m_output->Write(&i32, 4);
    }
  }
}

void wxDataOutputStream::Write16(const std::uint16_t *buffer, size_t size)
{
  if (m_be_order)
  {
    for (std::uint32_t i=0; i<size ;i++)
    {
      const std::uint16_t i16 = wxUINT16_SWAP_ON_LE(*buffer);
      buffer++;
      m_output->Write(&i16, 2);
    }
  }
  else
  {
    for (std::uint32_t i=0; i<size ;i++)
    {
      const std::uint16_t i16 = wxUINT16_SWAP_ON_BE(*buffer);
      buffer++;
      m_output->Write(&i16, 2);
    }
  }
}

void wxDataOutputStream::Write8(const std::uint8_t *buffer, size_t size)
{
  m_output->Write(buffer, size);
}

void wxDataOutputStream::WriteDouble(const double *buffer, size_t size)
{
  for (std::uint32_t i=0; i<size; i++)
  {
    WriteDouble(*(buffer++));
  }
}

void wxDataOutputStream::WriteFloat(const float *buffer, size_t size)
{
  for (std::uint32_t i=0; i<size; i++)
  {
    WriteFloat(*(buffer++));
  }
}

wxDataOutputStream& wxDataOutputStream::operator<<(const wxString& string)
{
  WriteString(string);
  return *this;
}

wxDataOutputStream& wxDataOutputStream::operator<<(std::int8_t c)
{
  Write8((std::uint8_t)c);
  return *this;
}

wxDataOutputStream& wxDataOutputStream::operator<<(std::int16_t i)
{
  Write16((std::uint16_t)i);
  return *this;
}

wxDataOutputStream& wxDataOutputStream::operator<<(std::int32_t i)
{
  Write32((std::uint32_t)i);
  return *this;
}

wxDataOutputStream& wxDataOutputStream::operator<<(std::uint8_t c)
{
  Write8(c);
  return *this;
}

wxDataOutputStream& wxDataOutputStream::operator<<(std::uint16_t i)
{
  Write16(i);
  return *this;
}

wxDataOutputStream& wxDataOutputStream::operator<<(std::uint32_t i)
{
  Write32(i);
  return *this;
}

#if wxHAS_INT64
wxDataOutputStream& wxDataOutputStream::operator<<(std::uint64_t i)
{
  Write64(i);
  return *this;
}

wxDataOutputStream& wxDataOutputStream::operator<<(std::int64_t i)
{
  Write64(i);
  return *this;
}
#endif // wxHAS_INT64

#if defined(wxLongLong_t) && wxUSE_LONGLONG
wxDataOutputStream& wxDataOutputStream::operator<<(const wxULongLong &i)
{
  WriteLL(i);
  return *this;
}

wxDataOutputStream& wxDataOutputStream::operator<<(const wxLongLong &i)
{
  WriteLL(i);
  return *this;
}
#endif // wxLongLong_t

wxDataOutputStream& wxDataOutputStream::operator<<(double d)
{
  WriteDouble(d);
  return *this;
}

wxDataOutputStream& wxDataOutputStream::operator<<(float f)
{
  WriteFloat(f);
  return *this;
}

#endif
  // wxUSE_STREAMS
