/////////////////////////////////////////////////////////////////////////////
// Name:        wx/txtstrm.h
// Purpose:     Text stream classes
// Author:      Guilhem Lavaux
// Modified by:
// Created:     28/06/1998
// Copyright:   (c) Guilhem Lavaux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TXTSTREAM_H_
#define _WX_TXTSTREAM_H_

#include "wx/stream.h"
#include "wx/convauto.h"

#if wxUSE_STREAMS

#include <cstdint>

class WXDLLIMPEXP_FWD_BASE wxTextInputStream;
class WXDLLIMPEXP_FWD_BASE wxTextOutputStream;

typedef wxTextInputStream& (*__wxTextInputManip)(wxTextInputStream&);
typedef wxTextOutputStream& (*__wxTextOutputManip)(wxTextOutputStream&);

WXDLLIMPEXP_BASE wxTextOutputStream &endl( wxTextOutputStream &stream );

// If you're scanning through a file using wxTextInputStream, you should check for EOF _before_
// reading the next item (word / number), because otherwise the last item may get lost.
// You should however be prepared to receive an empty item (empty string / zero number) at the
// end of file, especially on Windows systems. This is unavoidable because most (but not all) files end
// with whitespace (i.e. usually a newline).
class WXDLLIMPEXP_BASE wxTextInputStream
{
public:
    wxTextInputStream(wxInputStream& s,
                      const wxString &sep=wxT(" \t"),
                      const wxMBConv& conv = wxConvAuto());

    ~wxTextInputStream();

    wxTextInputStream& operator=(wxTextInputStream&&) = delete;

    const wxInputStream& GetInputStream() const { return m_input; }

    // base may be between 2 and 36, inclusive, or the special 0 (= C format)
    std::uint64_t Read64(int base = 10);
    std::uint32_t Read32(int base = 10);
    std::uint16_t Read16(int base = 10);
    std::uint8_t  Read8(int base = 10);
    std::int64_t  Read64S(int base = 10);
    std::int32_t  Read32S(int base = 10);
    std::int16_t  Read16S(int base = 10);
    std::int8_t   Read8S(int base = 10);
    double   ReadDouble();
    wxString ReadLine();
    wxString ReadWord();
    wxChar   GetChar();

    wxString GetStringSeparators() const { return m_separators; }
    void SetStringSeparators(const wxString &c) { m_separators = c; }

    // Operators
    wxTextInputStream& operator>>(wxString& word);
    wxTextInputStream& operator>>(char& c);
#if wxWCHAR_T_IS_REAL_TYPE
    wxTextInputStream& operator>>(wchar_t& wc);
#endif // wxWCHAR_T_IS_REAL_TYPE
    wxTextInputStream& operator>>(std::int16_t& i);
    wxTextInputStream& operator>>(std::int32_t& i);
    wxTextInputStream& operator>>(std::int64_t& i);
    wxTextInputStream& operator>>(std::uint16_t& i);
    wxTextInputStream& operator>>(std::uint32_t& i);
    wxTextInputStream& operator>>(std::uint64_t& i);
    wxTextInputStream& operator>>(double& i);
    wxTextInputStream& operator>>(float& f);

    wxTextInputStream& operator>>( __wxTextInputManip func) { return func(*this); }

protected:
    wxInputStream &m_input;
    wxString m_separators;

    // Data possibly (see m_validXXX) read from the stream but not decoded yet.
    // This is necessary because GetChar() may only return a single character
    // but we may get more than one character when decoding raw input bytes.
    char m_lastBytes[10];

    // The bytes [0, m_validEnd) of m_lastBytes contain the bytes read by the
    // last GetChar() call (this interval may be empty if GetChar() hasn't been
    // called yet). The bytes [0, m_validBegin) have been already decoded and
    // returned to caller or stored in m_lastWChar in the particularly
    // egregious case of decoding a non-BMP character when using UTF-16 for
    // wchar_t. Finally, the bytes [m_validBegin, m_validEnd) remain to be
    // decoded and returned during the next call (again, this interval can, and
    // usually will, be empty too if m_validBegin == m_validEnd).
    size_t m_validBegin{0};
    size_t m_validEnd{0};

    wxMBConv *m_conv;

    // The second half of a surrogate character when using UTF-16 for wchar_t:
    // we can't return it immediately from GetChar() when we read a Unicode
    // code point outside of the BMP, but we can't keep it in m_lastBytes
    // neither because it can't separately decoded, so we have a separate 1
    // wchar_t buffer just for this case.
#if SIZEOF_WCHAR_T == 2
    wchar_t m_lastWChar{0};
#endif // SIZEOF_WCHAR_T == 2

    bool   EatEOL(const wxChar &c);
    void   UngetLast(); // should be used instead of wxInputStream::Ungetch() because of Unicode issues
    wxChar NextNonSeparators();
};

enum class wxEOL
{
  Native,
  Unix,
  Mac,
  Dos
};

class WXDLLIMPEXP_BASE wxTextOutputStream
{
public:
    wxTextOutputStream(wxOutputStream& s,
                       wxEOL mode = wxEOL::Native,
                       const wxMBConv& conv = wxConvAuto());

    virtual ~wxTextOutputStream();

    wxTextOutputStream& operator=(wxTextOutputStream&&) = delete;

    const wxOutputStream& GetOutputStream() const { return m_output; }

    void SetMode( wxEOL mode = wxEOL::Native );
    wxEOL GetMode() { return m_mode; }

    template<typename T>
    void Write(const T& i)
    {
        wxString str;
        str << i;

        WriteString(str);
    }

    void Write64(std::uint64_t i);
    void Write32(std::uint32_t i);
    void Write16(std::uint16_t i);
    void Write8(std::uint8_t i);
    virtual void WriteDouble(double d);
    virtual void WriteString(const wxString& string);

    wxTextOutputStream& PutChar(wxChar c);

    void Flush();

    wxTextOutputStream& operator<<(const wxString& string);
    wxTextOutputStream& operator<<(char c);
#if wxWCHAR_T_IS_REAL_TYPE
    wxTextOutputStream& operator<<(wchar_t wc);
#endif // wxWCHAR_T_IS_REAL_TYPE
    wxTextOutputStream& operator<<(std::int16_t c);
    wxTextOutputStream& operator<<(std::int32_t c);
    wxTextOutputStream& operator<<(std::int64_t c);
    wxTextOutputStream& operator<<(std::uint16_t c);
    wxTextOutputStream& operator<<(std::uint32_t c);
    wxTextOutputStream& operator<<(std::uint64_t c);
    wxTextOutputStream& operator<<(double f);
    wxTextOutputStream& operator<<(float f);

    wxTextOutputStream& operator<<( __wxTextOutputManip func) { return func(*this); }

protected:
    wxOutputStream &m_output;
    wxEOL           m_mode;

    wxMBConv *m_conv;

#if SIZEOF_WCHAR_T == 2
    // The first half of a surrogate character if one was passed to PutChar()
    // and couldn't be output when it was called the last time.
    wchar_t m_lastWChar;
#endif // SIZEOF_WCHAR_T == 2
};

#endif
  // wxUSE_STREAMS

#endif
    // _WX_DATSTREAM_H_
