/////////////////////////////////////////////////////////////////////////////
// Name:        wx/private/fileback.h
// Purpose:     Back an input stream with memory or a file
// Author:      Mike Wetherell
// Copyright:   (c) 2006 Mike Wetherell
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FILEBACK_H__
#define _WX_FILEBACK_H__

#if wxUSE_FILESYSTEM

#include "wx/filefn.h"

import WX.Cmn.Stream;

import WX.File.Flags;

import <cstdint>;
import <string>;

// ----------------------------------------------------------------------------
// Backs an input stream with memory or a file to make it seekable.
//
// One or more wxBackedInputStreams can be used to read it's data. The data is
// reference counted, so stays alive until the last wxBackingFile or
// wxBackedInputStream using it is destroyed.
// ----------------------------------------------------------------------------

class wxBackingFile
{
public:
    static constexpr std::size_t DefaultBufSize = 16384;

    // Takes ownership of stream. If the stream is smaller than bufsize, the
    // backing file is never created and the backing is done with memory.
    wxBackingFile(wxInputStream *stream,
                  size_t bufsize = DefaultBufSize,
                  const std::string& prefix = "wxbf");

    wxBackingFile()  = default;
    ~wxBackingFile();

    wxBackingFile(const wxBackingFile& backer);
    wxBackingFile& operator=(const wxBackingFile& backer);

    operator bool() const { return m_impl != nullptr; }

private:
    class wxBackingFileImpl *m_impl{nullptr};
    friend class wxBackedInputStream;
};

// ----------------------------------------------------------------------------
// An input stream to read from a wxBackingFile.
// ----------------------------------------------------------------------------

class wxBackedInputStream : public wxInputStream
{
public:
    wxBackedInputStream(const wxBackingFile& backer);

    // If the length of the backer's parent stream is unknown then GetLength()
    // returns wxInvalidOffset until the parent has been read to the end.
    wxFileOffset GetLength() const override;

    // Returns the length, reading the parent stream to the end if necessary.
    wxFileOffset FindLength() const;

    bool IsSeekable() const override { return true; }

protected:
    size_t OnSysRead(void *buffer, size_t size) override;
    wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode) override;
    wxFileOffset OnSysTell() const override;

private:
    wxBackingFile m_backer;
    wxFileOffset m_pos;

    wxBackedInputStream(const wxBackedInputStream&) = delete;
	wxBackedInputStream& operator=(const wxBackedInputStream&) = delete;
};

#endif // wxUSE_FILESYSTEM

#endif // _WX_FILEBACK_H__
