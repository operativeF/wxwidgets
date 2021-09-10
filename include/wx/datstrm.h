/////////////////////////////////////////////////////////////////////////////
// Name:        wx/datstrm.h
// Purpose:     Data stream classes
// Author:      Guilhem Lavaux
// Modified by: Mickael Gilabert
// Created:     28/06/1998
// Copyright:   (c) Guilhem Lavaux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DATSTREAM_H_
#define _WX_DATSTREAM_H_

#include "wx/stream.h"
#include "wx/longlong.h"
#include "wx/convauto.h"

#include <cstdint>

#if wxUSE_STREAMS

// Common wxDataInputStream and wxDataOutputStream parameters.
class WXDLLIMPEXP_BASE wxDataStreamBase
{
public:
    wxDataStreamBase(const wxDataStreamBase&) = delete;
    wxDataStreamBase& operator=(const wxDataStreamBase&) = delete;
    wxDataStreamBase(wxDataStreamBase&&) = default;
    wxDataStreamBase& operator=(wxDataStreamBase&&) = default;

    void BigEndianOrdered(bool be_order) { m_be_order = be_order; }

    // By default we use extended precision (80 bit) format for both float and
    // doubles. Call this function to switch to alternative representation in
    // which IEEE 754 single precision (32 bits) is used for floats and double
    // precision (64 bits) is used for doubles.
    void UseBasicPrecisions()
    {
    }

    void SetConv( const wxMBConv &conv );
    wxMBConv *GetConv() const { return m_conv; }

protected:
    // Ctor and dtor are both protected, this class is never used directly but
    // only by its derived classes.
    wxDataStreamBase(const wxMBConv& conv);
    ~wxDataStreamBase();

    bool m_be_order{false};

    wxMBConv *m_conv;
};


class WXDLLIMPEXP_BASE wxDataInputStream : public wxDataStreamBase
{
public:
    wxDataInputStream(wxInputStream& s, const wxMBConv& conv = wxConvUTF8);

    wxDataInputStream(const wxDataInputStream&) = delete;
    wxDataInputStream& operator=(const wxDataInputStream&) = delete;
    wxDataInputStream(wxDataInputStream&&) = default;
    wxDataInputStream& operator=(wxDataInputStream&&) = default;

    bool IsOk() { return m_input->IsOk(); }

#if wxHAS_INT64
    std::uint64_t Read64();
#endif
#if wxUSE_LONGLONG
    wxLongLong ReadLL();
#endif
    std::uint32_t Read32();
    std::uint16_t Read16();
    std::uint8_t Read8();
    double ReadDouble();
    float ReadFloat();
    wxString ReadString();

#if wxHAS_INT64
    void Read64(std::uint64_t *buffer, size_t size);
    void Read64(std::int64_t *buffer, size_t size);
#endif
#if defined(wxLongLong_t) && wxUSE_LONGLONG
    void Read64(wxULongLong *buffer, size_t size);
    void Read64(wxLongLong *buffer, size_t size);
#endif
#if wxUSE_LONGLONG
    void ReadLL(wxULongLong *buffer, size_t size);
    void ReadLL(wxLongLong *buffer, size_t size);
#endif
    void Read32(std::uint32_t *buffer, size_t size);
    void Read16(std::uint16_t *buffer, size_t size);
    void Read8(std::uint8_t *buffer, size_t size);
    void ReadDouble(double *buffer, size_t size);
    void ReadFloat(float *buffer, size_t size);

    wxDataInputStream& operator>>(wxString& s);
    wxDataInputStream& operator>>(std::int8_t& c);
    wxDataInputStream& operator>>(std::int16_t& i);
    wxDataInputStream& operator>>(std::int32_t& i);
    wxDataInputStream& operator>>(std::uint8_t& c);
    wxDataInputStream& operator>>(std::uint16_t& i);
    wxDataInputStream& operator>>(std::uint32_t& i);
#if wxHAS_INT64
    wxDataInputStream& operator>>(std::uint64_t& i);
    wxDataInputStream& operator>>(std::int64_t& i);
#endif
#if defined(wxLongLong_t) && wxUSE_LONGLONG
    wxDataInputStream& operator>>(wxULongLong& i);
    wxDataInputStream& operator>>(wxLongLong& i);
#endif
    wxDataInputStream& operator>>(double& d);
    wxDataInputStream& operator>>(float& f);

protected:
    wxInputStream *m_input;
};

class WXDLLIMPEXP_BASE wxDataOutputStream : public wxDataStreamBase
{
public:
    wxDataOutputStream(wxOutputStream& s, const wxMBConv& conv = wxConvUTF8);

    wxDataOutputStream(const wxDataOutputStream&) = delete;
    wxDataOutputStream& operator=(const wxDataOutputStream&) = delete;
    wxDataOutputStream(wxDataOutputStream&&) = default;
    wxDataOutputStream& operator=(wxDataOutputStream&&) = default;

    bool IsOk() { return m_output->IsOk(); }

#if wxHAS_INT64
    void Write64(std::uint64_t i);
    void Write64(std::int64_t i);
#endif
#if wxUSE_LONGLONG
    void WriteLL(const wxLongLong &ll);
    void WriteLL(const wxULongLong &ll);
#endif
    void Write32(std::uint32_t i);
    void Write16(std::uint16_t i);
    void Write8(std::uint8_t i);
    void WriteDouble(double d);
    void WriteFloat(float f);
    void WriteString(const wxString& string);

#if wxHAS_INT64
    void Write64(const std::uint64_t *buffer, size_t size);
    void Write64(const std::int64_t *buffer, size_t size);
#endif
#if defined(wxLongLong_t) && wxUSE_LONGLONG
    void Write64(const wxULongLong *buffer, size_t size);
    void Write64(const wxLongLong *buffer, size_t size);
#endif
#if wxUSE_LONGLONG
    void WriteLL(const wxULongLong *buffer, size_t size);
    void WriteLL(const wxLongLong *buffer, size_t size);
#endif
    void Write32(const std::uint32_t *buffer, size_t size);
    void Write16(const std::uint16_t *buffer, size_t size);
    void Write8(const std::uint8_t *buffer, size_t size);
    void WriteDouble(const double *buffer, size_t size);
    void WriteFloat(const float *buffer, size_t size);

    wxDataOutputStream& operator<<(const wxString& string);
    wxDataOutputStream& operator<<(std::int8_t c);
    wxDataOutputStream& operator<<(std::int16_t i);
    wxDataOutputStream& operator<<(std::int32_t i);
    wxDataOutputStream& operator<<(std::uint8_t c);
    wxDataOutputStream& operator<<(std::uint16_t i);
    wxDataOutputStream& operator<<(std::uint32_t i);
#if wxHAS_INT64
    wxDataOutputStream& operator<<(std::uint64_t i);
    wxDataOutputStream& operator<<(std::int64_t i);
#endif
#if defined(wxLongLong_t) && wxUSE_LONGLONG
    wxDataOutputStream& operator<<(const wxULongLong &i);
    wxDataOutputStream& operator<<(const wxLongLong &i);
#endif
    wxDataOutputStream& operator<<(double d);
    wxDataOutputStream& operator<<(float f);

protected:
    wxOutputStream *m_output;
};

#endif
  // wxUSE_STREAMS

#endif
    // _WX_DATSTREAM_H_
