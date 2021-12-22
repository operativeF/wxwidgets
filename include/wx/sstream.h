///////////////////////////////////////////////////////////////////////////////
// Name:        wx/sstream.h
// Purpose:     string-based streams
// Author:      Vadim Zeitlin
// Modified by:
// Created:     2004-09-19
// Copyright:   (c) 2004 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SSTREAM_H_
#define _WX_SSTREAM_H_

#if wxUSE_STREAMS

#include "wx/filefn.h"
#include "wx/string.h"

import WX.Cmn.Stream;
import WX.File.Flags;

// ----------------------------------------------------------------------------
// wxStringInputStream is a stream reading from the given (fixed size) string
// ----------------------------------------------------------------------------

class wxStringInputStream : public wxInputStream
{
public:
    // ctor associates the stream with the given string which makes a copy of
    // it
    wxStringInputStream(const wxString& s);

    wxStringInputStream& operator=(wxStringInputStream&&) = delete;

    wxFileOffset GetLength() const override;
    bool IsSeekable() const override { return true; }

protected:
    wxFileOffset OnSysSeek(wxFileOffset ofs, wxSeekMode mode) override;
    wxFileOffset OnSysTell() const override;
    size_t OnSysRead(void *buffer, size_t size) override;

private:
    // the string that was passed in the ctor
    wxString m_str;

    // the buffer we're reading from
    wxCharBuffer m_buf;

    // length of the buffer we're reading from
    size_t m_len;

    // position in the stream in bytes, *not* in chars
    size_t m_pos;
};

// ----------------------------------------------------------------------------
// wxStringOutputStream writes data to the given string, expanding it as needed
// ----------------------------------------------------------------------------

class wxStringOutputStream : public wxOutputStream
{
public:
    // The stream will write data either to the provided string or to an
    // internal string which can be retrieved using GetString()
    //
    // Note that the conversion object should have the life time greater than
    // this stream.
    explicit wxStringOutputStream(wxString *pString = nullptr,
                                  wxMBConv& conv = wxConvUTF8);

    // get the string containing current output
    const wxString& GetString() const { return *m_str; }

    bool IsSeekable() const override { return true; }

protected:
    wxFileOffset OnSysTell() const override;
    size_t OnSysWrite(const void *buffer, size_t size) override;

private:
    // internal string, not used if caller provided his own string
    wxString m_strInternal;

    // pointer given by the caller or just pointer to m_strInternal
    wxString *m_str;

    // position in the stream in bytes, *not* in chars
    size_t m_pos;

    // converter to use: notice that with the default UTF-8 one the input
    // stream must contain valid UTF-8 data, use wxConvISO8859_1 to work with
    // arbitrary 8 bit data
    wxMBConv& m_conv;

    // unconverted data from the last call to OnSysWrite()
    wxMemoryBuffer m_unconv;
};

#endif // wxUSE_STREAMS

#endif // _WX_SSTREAM_H_

