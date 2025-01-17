///////////////////////////////////////////////////////////////////////////////
// Name:        wx/lzmastream.h
// Purpose:     Filters streams using LZMA(2) compression
// Author:      Vadim Zeitlin
// Created:     2018-03-29
// Copyright:   (c) 2018 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_LZMASTREAM_H_
#define _WX_LZMASTREAM_H_

#if wxUSE_LIBLZMA && wxUSE_STREAMS

import WX.Cmn.Stream;

import WX.Utils.VersionInfo;

import <cstdint>;

namespace wxPrivate
{

// Private wrapper for lzma_stream struct.
struct wxLZMAStream;

// Common part of input and output LZMA streams: this is just an implementation
// detail and is not part of the public API.
class wxLZMAData
{
public:
    wxLZMAData& operator=(wxLZMAData&&) = delete;

protected:
    wxLZMAData();
    ~wxLZMAData();

    wxLZMAStream* m_stream;
    std::uint8_t* m_streamBuf;
    wxFileOffset m_pos;
};

} // namespace wxPrivate

// ----------------------------------------------------------------------------
// Filter for decompressing data compressed using LZMA
// ----------------------------------------------------------------------------

class wxLZMAInputStream : public wxFilterInputStream,
                                           private wxPrivate::wxLZMAData
{
public:
    explicit wxLZMAInputStream(wxInputStream& stream)
        : wxFilterInputStream(stream)
    {
        Init();
    }

    explicit wxLZMAInputStream(wxInputStream* stream)
        : wxFilterInputStream(stream)
    {
        Init();
    }

    char Peek() override { return wxInputStream::Peek(); }
    wxFileOffset GetLength() const override { return wxInputStream::GetLength(); }

protected:
    size_t OnSysRead(void *buffer, size_t size) override;
    wxFileOffset OnSysTell() const override { return m_pos; }

private:
    void Init();
};

// ----------------------------------------------------------------------------
// Filter for compressing data using LZMA(2) algorithm
// ----------------------------------------------------------------------------

class wxLZMAOutputStream : public wxFilterOutputStream,
                                            private wxPrivate::wxLZMAData
{
public:
    explicit wxLZMAOutputStream(wxOutputStream& stream, int level = -1)
        : wxFilterOutputStream(stream)
    {
        Init(level);
    }

    explicit wxLZMAOutputStream(wxOutputStream* stream, int level = -1)
        : wxFilterOutputStream(stream)
    {
        Init(level);
    }

    virtual ~wxLZMAOutputStream() { Close(); }

    void Sync() override { DoFlush(false); }
    bool Close() override;
    wxFileOffset GetLength() const override { return m_pos; }

protected:
    size_t OnSysWrite(const void *buffer, size_t size) override;
    wxFileOffset OnSysTell() const override { return m_pos; }

private:
    void Init(int level);

    // Write the contents of the internal buffer to the output stream.
    bool UpdateOutput();

    // Write out the current buffer if necessary, i.e. if no space remains in
    // it, and reinitialize m_stream to point to it. Returns false on success
    // or false on error, in which case m_lasterror is updated.
    bool UpdateOutputIfNecessary();

    // Run LZMA_FINISH (if argument is true) or LZMA_FULL_FLUSH, return true on
    // success or false on error.
    bool DoFlush(bool finish);
};

// ----------------------------------------------------------------------------
// Support for creating LZMA streams from extension/MIME type
// ----------------------------------------------------------------------------

class wxLZMAClassFactory: public wxFilterClassFactory
{
public:
    wxLZMAClassFactory();

    wxFilterInputStream *NewStream(wxInputStream& stream) const override
        { return new wxLZMAInputStream(stream); }
    wxFilterOutputStream *NewStream(wxOutputStream& stream) const override
        { return new wxLZMAOutputStream(stream, -1); }
    wxFilterInputStream *NewStream(wxInputStream *stream) const override
        { return new wxLZMAInputStream(stream); }
    wxFilterOutputStream *NewStream(wxOutputStream *stream) const override
        { return new wxLZMAOutputStream(stream, -1); }

    const wxChar * const *GetProtocols(wxStreamProtocolType type
                                       = wxSTREAM_PROTOCOL) const override;

private:
    wxDECLARE_DYNAMIC_CLASS(wxLZMAClassFactory);
};

wxVersionInfo wxGetLibLZMAVersionInfo();

#endif // wxUSE_LIBLZMA && wxUSE_STREAMS

#endif // _WX_LZMASTREAM_H_
