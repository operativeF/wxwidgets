/////////////////////////////////////////////////////////////////////////////
// Name:        wx/wfstream.h
// Purpose:     File stream classes
// Author:      Guilhem Lavaux
// Modified by:
// Created:     11/07/98
// Copyright:   (c) Guilhem Lavaux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_WXFSTREAM_H__
#define _WX_WXFSTREAM_H__

#include "wx/defs.h"

#if wxUSE_STREAMS

#include "wx/object.h"
#include "wx/string.h"
#include "wx/stream.h"
#include "wx/file.h"
#include "wx/ffile.h"

#if wxUSE_FILE

// ----------------------------------------------------------------------------
// wxFileStream using wxFile
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_BASE wxFileInputStream : public wxInputStream
{
public:
    wxFileInputStream(const wxString& ifileName);
    wxFileInputStream(wxFile& file);
    wxFileInputStream(int fd);
    ~wxFileInputStream();

    wxFileInputStream(const wxFileInputStream&) = delete;
    wxFileInputStream& operator=(const wxFileInputStream&) = delete;
    wxFileInputStream(wxFileInputStream&&) = default;
    wxFileInputStream& operator=(wxFileInputStream&&) = default;

    wxFileOffset GetLength() const override;

    bool IsOk() const override;
    bool IsSeekable() const override { return m_file->GetKind() == wxFileKind::Disk; }

    wxFile* GetFile() const { return m_file; }

protected:
    wxFileInputStream() = default;

    size_t OnSysRead(void *buffer, size_t size) override;
    wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode) override;
    wxFileOffset OnSysTell() const override;

protected:
    wxFile *m_file{nullptr};
    bool m_file_destroy{false};
};

class WXDLLIMPEXP_BASE wxFileOutputStream : public wxOutputStream
{
public:
    wxFileOutputStream(const wxString& fileName);
    wxFileOutputStream(wxFile& file);
    wxFileOutputStream(int fd);
    ~wxFileOutputStream();

    wxFileOutputStream(const wxFileOutputStream&) = delete;
    wxFileOutputStream& operator=(const wxFileOutputStream&) = delete;
    wxFileOutputStream(wxFileOutputStream&&) = default;
    wxFileOutputStream& operator=(wxFileOutputStream&&) = default;

    void Sync() override;
    bool Close() override { return m_file_destroy ? m_file->Close() : true; }
    wxFileOffset GetLength() const override;

    bool IsOk() const override;
    bool IsSeekable() const override { return m_file->GetKind() == wxFileKind::Disk; }

    wxFile* GetFile() const { return m_file; }

protected:
    wxFileOutputStream() = default;

    size_t OnSysWrite(const void *buffer, size_t size) override;
    wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode) override;
    wxFileOffset OnSysTell() const override;

protected:
    wxFile *m_file{nullptr};
    bool m_file_destroy{false};
};

class WXDLLIMPEXP_BASE wxTempFileOutputStream : public wxOutputStream
{
public:
    wxTempFileOutputStream(const wxString& fileName);
    ~wxTempFileOutputStream();

    wxTempFileOutputStream(const wxTempFileOutputStream&) = delete;
    wxTempFileOutputStream& operator=(const wxTempFileOutputStream&) = delete;
    wxTempFileOutputStream(wxTempFileOutputStream&&) = default;
    wxTempFileOutputStream& operator=(wxTempFileOutputStream&&) = default;

    bool Close() override { return Commit(); }
    WXDLLIMPEXP_INLINE_BASE virtual bool Commit() { return m_file->Commit(); }
    WXDLLIMPEXP_INLINE_BASE virtual void Discard() { m_file->Discard(); }

    wxFileOffset GetLength() const override { return m_file->Length(); }
    bool IsSeekable() const override { return true; }

protected:
    size_t OnSysWrite(const void *buffer, size_t size) override;
    wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode) override
        { return m_file->Seek(pos, mode); }
    wxFileOffset OnSysTell() const override { return m_file->Tell(); }

private:
    wxTempFile *m_file;
};

class WXDLLIMPEXP_BASE wxTempFFileOutputStream : public wxOutputStream
{
public:
    wxTempFFileOutputStream(const wxString& fileName);
    ~wxTempFFileOutputStream();

    wxTempFFileOutputStream(const wxTempFFileOutputStream&) = delete;
    wxTempFFileOutputStream& operator=(const wxTempFFileOutputStream&) = delete;
    wxTempFFileOutputStream(wxTempFFileOutputStream&&) = default;
    wxTempFFileOutputStream& operator=(wxTempFFileOutputStream&&) = default;

    bool Close() override { return Commit(); }
    WXDLLIMPEXP_INLINE_BASE virtual bool Commit() { return m_file->Commit(); }
    WXDLLIMPEXP_INLINE_BASE virtual void Discard() { m_file->Discard(); }

    wxFileOffset GetLength() const override { return m_file->Length(); }
    bool IsSeekable() const override { return true; }

protected:
    size_t OnSysWrite(const void *buffer, size_t size) override;
    wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode) override
        { return m_file->Seek(pos, mode); }
    wxFileOffset OnSysTell() const override { return m_file->Tell(); }

private:
    wxTempFFile *m_file;
};

class WXDLLIMPEXP_BASE wxFileStream : public wxFileInputStream,
                                      public wxFileOutputStream
{
public:
    wxFileStream(const wxString& fileName);

    wxFileStream(const wxFileStream&) = delete;
    wxFileStream& operator=(const wxFileStream&) = delete;
    wxFileStream(wxFileStream&&) = default;
    wxFileStream& operator=(wxFileStream&&) = default;

    bool IsOk() const override;

    // override (some) virtual functions inherited from both classes to resolve
    // ambiguities (this wouldn't be necessary if wxStreamBase were a virtual
    // base class but it isn't)

    bool IsSeekable() const override
    {
        return wxFileInputStream::IsSeekable();
    }

    wxFileOffset GetLength() const override
    {
        return wxFileInputStream::GetLength();
    }

protected:
    wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode) override
    {
        return wxFileInputStream::OnSysSeek(pos, mode);
    }

    wxFileOffset OnSysTell() const override
    {
        return wxFileInputStream::OnSysTell();
    }
};

#endif //wxUSE_FILE

#if wxUSE_FFILE

// ----------------------------------------------------------------------------
// wxFFileStream using wxFFile
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_BASE wxFFileInputStream : public wxInputStream
{
public:
    wxFFileInputStream(const wxString& fileName, const wxString& mode = wxASCII_STR("rb"));
    wxFFileInputStream(wxFFile& file);
    wxFFileInputStream(FILE *file);
    ~wxFFileInputStream();

    wxFFileInputStream(const wxFFileInputStream&) = delete;
    wxFFileInputStream& operator=(const wxFFileInputStream&) = delete;
    wxFFileInputStream(wxFFileInputStream&&) = default;
    wxFFileInputStream& operator=(wxFFileInputStream&&) = default;

    wxFileOffset GetLength() const override;

    bool IsOk() const override;
    bool IsSeekable() const override { return m_file->GetKind() == wxFileKind::Disk; }

    wxFFile* GetFile() const { return m_file; }

protected:
    wxFFileInputStream() = default;

    size_t OnSysRead(void *buffer, size_t size) override;
    wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode) override;
    wxFileOffset OnSysTell() const override;

protected:
    wxFFile *m_file{nullptr};
    bool m_file_destroy{false};
};

class WXDLLIMPEXP_BASE wxFFileOutputStream : public wxOutputStream
{
public:
    wxFFileOutputStream(const wxString& fileName, const wxString& mode = wxASCII_STR("wb"));
    wxFFileOutputStream(wxFFile& file);
    wxFFileOutputStream(FILE *file);
    ~wxFFileOutputStream();

    wxFFileOutputStream(const wxFFileOutputStream&) = delete;
    wxFFileOutputStream& operator=(const wxFFileOutputStream&) = delete;
    wxFFileOutputStream(wxFFileOutputStream&&) = default;
    wxFFileOutputStream& operator=(wxFFileOutputStream&&) = default;

    void Sync() override;
    bool Close() override { return m_file_destroy ? m_file->Close() : true; }
    wxFileOffset GetLength() const override;

    bool IsOk() const override;
    bool IsSeekable() const override { return m_file->GetKind() == wxFileKind::Disk; }

    wxFFile* GetFile() const { return m_file; }

protected:
    wxFFileOutputStream() = default;

    size_t OnSysWrite(const void *buffer, size_t size) override;
    wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode) override;
    wxFileOffset OnSysTell() const override;

protected:
    wxFFile *m_file{nullptr};
    bool m_file_destroy{false};
};

class WXDLLIMPEXP_BASE wxFFileStream : public wxFFileInputStream,
                                       public wxFFileOutputStream
{
public:
    wxFFileStream(const wxString& fileName, const wxString& mode = wxASCII_STR("w+b"));

    wxFFileStream(const wxFFileStream&) = delete;
    wxFFileStream& operator=(const wxFFileStream&) = delete;
    wxFFileStream(wxFFileStream&&) = default;
    wxFFileStream& operator=(wxFFileStream&&) = default;

    // override some virtual functions to resolve ambiguities, just as in
    // wxFileStream

    bool IsOk() const override;

    bool IsSeekable() const override
    {
        return wxFFileInputStream::IsSeekable();
    }

    wxFileOffset GetLength() const override
    {
        return wxFFileInputStream::GetLength();
    }

protected:
    wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode) override
    {
        return wxFFileInputStream::OnSysSeek(pos, mode);
    }

    wxFileOffset OnSysTell() const override
    {
        return wxFFileInputStream::OnSysTell();
    }
};

#endif //wxUSE_FFILE

#endif // wxUSE_STREAMS

#endif // _WX_WXFSTREAM_H__
