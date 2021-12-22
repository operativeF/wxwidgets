/////////////////////////////////////////////////////////////////////////////
// Name:        wx/mstream.h
// Purpose:     Memory stream classes
// Author:      Guilhem Lavaux
// Modified by:
// Created:     11/07/98
// Copyright:   (c) Guilhem Lavaux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_WXMMSTREAM_H__
#define _WX_WXMMSTREAM_H__

#if wxUSE_STREAMS

#include "wx/filefn.h"

import WX.Cmn.Stream;

import WX.File.Flags;

class wxMemoryOutputStream;

class wxMemoryInputStream : public wxInputStream
{
public:
    wxMemoryInputStream(const void *data, size_t length);
    wxMemoryInputStream(const wxMemoryOutputStream& stream);
    wxMemoryInputStream(wxInputStream& stream,
                        wxFileOffset lenFile = wxInvalidOffset)
    {
        InitFromStream(stream, lenFile);
    }
    wxMemoryInputStream(wxMemoryInputStream& stream)
         
    {
        InitFromStream(stream, wxInvalidOffset);
    }

    ~wxMemoryInputStream();

    wxMemoryInputStream& operator=(const wxMemoryInputStream&) = delete;

    wxFileOffset GetLength() const override { return m_length; }
    bool IsSeekable() const override { return true; }

    char Peek() override;
    bool CanRead() const override;

    wxStreamBuffer *GetInputStreamBuffer() const { return m_i_streambuf; }

protected:
    wxStreamBuffer *m_i_streambuf;

    size_t OnSysRead(void *buffer, size_t nbytes) override;
    wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode) override;
    wxFileOffset OnSysTell() const override;

private:
    // common part of ctors taking wxInputStream
    void InitFromStream(wxInputStream& stream, wxFileOffset lenFile);

    size_t m_length{};

    // copy ctor is implemented above: it copies the other stream in this one
    wxDECLARE_ABSTRACT_CLASS(wxMemoryInputStream);
};

class wxMemoryOutputStream : public wxOutputStream
{
public:
    // if data is !NULL it must be allocated with malloc()
    wxMemoryOutputStream(void *data = nullptr, size_t length = 0);
    ~wxMemoryOutputStream();

    wxMemoryOutputStream& operator=(wxMemoryOutputStream&&) = delete;

    wxFileOffset GetLength() const override { return m_o_streambuf->GetLastAccess(); }
    bool IsSeekable() const override { return true; }

    size_t CopyTo(void *buffer, size_t len) const;

    wxStreamBuffer *GetOutputStreamBuffer() const { return m_o_streambuf; }

protected:
    wxStreamBuffer *m_o_streambuf;

protected:
    size_t OnSysWrite(const void *buffer, size_t nbytes) override;
    wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode) override;
    wxFileOffset OnSysTell() const override;

    wxDECLARE_DYNAMIC_CLASS(wxMemoryOutputStream);
};

#endif // wxUSE_STREAMS

#endif // _WX_WXMMSTREAM_H__
