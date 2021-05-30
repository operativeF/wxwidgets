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
    ~wxFileInputStream() override;

    wxFileOffset GetLength() const override;

    bool Ok() const { return IsOk(); }
    bool IsOk() const override;
    bool IsSeekable() const override { return m_file->GetKind() == wxFILE_KIND_DISK; }

    wxFile* GetFile() const { return m_file; }

protected:
    wxFileInputStream();

    size_t OnSysRead(void *buffer, size_t size) override;
    wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode) override;
    wxFileOffset OnSysTell() const override;

protected:
    wxFile *m_file;
    bool m_file_destroy;

    wxFileInputStream(const wxFileInputStream&) = delete;
	wxFileInputStream& operator=(const wxFileInputStream&) = delete;
};

class WXDLLIMPEXP_BASE wxFileOutputStream : public wxOutputStream
{
public:
    wxFileOutputStream(const wxString& fileName);
    wxFileOutputStream(wxFile& file);
    wxFileOutputStream(int fd);
    ~wxFileOutputStream() override;

    void Sync() override;
    bool Close() override { return m_file_destroy ? m_file->Close() : true; }
    wxFileOffset GetLength() const override;

    bool Ok() const { return IsOk(); }
    bool IsOk() const override;
    bool IsSeekable() const override { return m_file->GetKind() == wxFILE_KIND_DISK; }

    wxFile* GetFile() const { return m_file; }

protected:
    wxFileOutputStream();

    size_t OnSysWrite(const void *buffer, size_t size) override;
    wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode) override;
    wxFileOffset OnSysTell() const override;

protected:
    wxFile *m_file;
    bool m_file_destroy;

    wxFileOutputStream(const wxFileOutputStream&) = delete;
	wxFileOutputStream& operator=(const wxFileOutputStream&) = delete;
};

class WXDLLIMPEXP_BASE wxTempFileOutputStream : public wxOutputStream
{
public:
    wxTempFileOutputStream(const wxString& fileName);
    ~wxTempFileOutputStream() override;

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

    wxTempFileOutputStream(const wxTempFileOutputStream&) = delete;
	wxTempFileOutputStream& operator=(const wxTempFileOutputStream&) = delete;
};

class WXDLLIMPEXP_BASE wxTempFFileOutputStream : public wxOutputStream
{
public:
    wxTempFFileOutputStream(const wxString& fileName);
    ~wxTempFFileOutputStream() override;

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

    wxTempFFileOutputStream(const wxTempFFileOutputStream&) = delete;
	wxTempFFileOutputStream& operator=(const wxTempFFileOutputStream&) = delete;
};

class WXDLLIMPEXP_BASE wxFileStream : public wxFileInputStream,
                                      public wxFileOutputStream
{
public:
    wxFileStream(const wxString& fileName);
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

private:
    wxFileStream(const wxFileStream&) = delete;
	wxFileStream& operator=(const wxFileStream&) = delete;
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
    ~wxFFileInputStream() override;

    wxFileOffset GetLength() const override;

    bool Ok() const { return IsOk(); }
    bool IsOk() const override;
    bool IsSeekable() const override { return m_file->GetKind() == wxFILE_KIND_DISK; }

    wxFFile* GetFile() const { return m_file; }

protected:
    wxFFileInputStream();

    size_t OnSysRead(void *buffer, size_t size) override;
    wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode) override;
    wxFileOffset OnSysTell() const override;

protected:
    wxFFile *m_file;
    bool m_file_destroy;

    wxFFileInputStream(const wxFFileInputStream&) = delete;
	wxFFileInputStream& operator=(const wxFFileInputStream&) = delete;
};

class WXDLLIMPEXP_BASE wxFFileOutputStream : public wxOutputStream
{
public:
    wxFFileOutputStream(const wxString& fileName, const wxString& mode = wxASCII_STR("wb"));
    wxFFileOutputStream(wxFFile& file);
    wxFFileOutputStream(FILE *file);
    ~wxFFileOutputStream() override;

    void Sync() override;
    bool Close() override { return m_file_destroy ? m_file->Close() : true; }
    wxFileOffset GetLength() const override;

    bool Ok() const { return IsOk(); }
    bool IsOk() const override;
    bool IsSeekable() const override { return m_file->GetKind() == wxFILE_KIND_DISK; }

    wxFFile* GetFile() const { return m_file; }

protected:
    wxFFileOutputStream();

    size_t OnSysWrite(const void *buffer, size_t size) override;
    wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode) override;
    wxFileOffset OnSysTell() const override;

protected:
    wxFFile *m_file;
    bool m_file_destroy;

    wxFFileOutputStream(const wxFFileOutputStream&) = delete;
	wxFFileOutputStream& operator=(const wxFFileOutputStream&) = delete;
};

class WXDLLIMPEXP_BASE wxFFileStream : public wxFFileInputStream,
                                       public wxFFileOutputStream
{
public:
    wxFFileStream(const wxString& fileName, const wxString& mode = wxASCII_STR("w+b"));

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

private:
    wxFFileStream(const wxFFileStream&) = delete;
	wxFFileStream& operator=(const wxFFileStream&) = delete;
};

#endif //wxUSE_FFILE

#endif // wxUSE_STREAMS

#endif // _WX_WXFSTREAM_H__
