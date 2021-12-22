/////////////////////////////////////////////////////////////////////////////
// Name:        wx/wfstream.h
// Purpose:     File stream classes
// Author:      Guilhem Lavaux
// Modified by:
// Created:     11/07/98
// Copyright:   (c) Guilhem Lavaux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/object.h"
#include "wx/string.h"
#include "wx/file.h"

export module WX.Cmn.WFStream;

import WX.Cmn.Stream;
import WX.Cmn.FFile;

import WX.File.Flags;

#if wxUSE_STREAMS

export
{

#if wxUSE_FILE

// ----------------------------------------------------------------------------
// wxFileStream using wxFile
// ----------------------------------------------------------------------------

class wxFileInputStream : public wxInputStream
{
public:
    wxFileInputStream(const wxString& ifileName);
    wxFileInputStream(wxFile& file);
    wxFileInputStream(int fd);
    ~wxFileInputStream();

    wxFileInputStream& operator=(wxFileInputStream&&) = delete;

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

class wxFileOutputStream : public wxOutputStream
{
public:
    wxFileOutputStream(const wxString& fileName);
    wxFileOutputStream(wxFile& file);
    wxFileOutputStream(int fd);
    ~wxFileOutputStream();

    wxFileOutputStream& operator=(wxFileOutputStream&&) = delete;

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

class wxTempFileOutputStream : public wxOutputStream
{
public:
    wxTempFileOutputStream(const wxString& fileName);
    ~wxTempFileOutputStream();

    wxTempFileOutputStream& operator=(wxTempFileOutputStream&&) = delete;

    bool Close() override { return Commit(); }
    virtual bool Commit() { return m_file->Commit(); }
    virtual void Discard() { m_file->Discard(); }

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

class wxTempFFileOutputStream : public wxOutputStream
{
public:
    wxTempFFileOutputStream(const wxString& fileName);
    ~wxTempFFileOutputStream();

    wxTempFFileOutputStream& operator=(wxTempFFileOutputStream&&) = delete;

    bool Close() override { return Commit(); }
    virtual bool Commit() { return m_file->Commit(); }
    virtual void Discard() { m_file->Discard(); }

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

class wxFileStream : public wxFileInputStream,
                                      public wxFileOutputStream
{
public:
    wxFileStream(const wxString& fileName);

    wxFileStream& operator=(wxFileStream&&) = delete;

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

class wxFFileInputStream : public wxInputStream
{
public:
    wxFFileInputStream(const wxString& fileName, const wxString& mode = "rb");
    wxFFileInputStream(wxFFile& file);
    wxFFileInputStream(FILE *file);
    ~wxFFileInputStream();

    wxFFileInputStream& operator=(wxFFileInputStream&&) = delete;

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

class wxFFileOutputStream : public wxOutputStream
{
public:
    wxFFileOutputStream(const wxString& fileName, const wxString& mode = "wb");
    wxFFileOutputStream(wxFFile& file);
    wxFFileOutputStream(FILE *file);
    ~wxFFileOutputStream();

    wxFFileOutputStream& operator=(wxFFileOutputStream&&) = delete;

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

class wxFFileStream : public wxFFileInputStream,
                                       public wxFFileOutputStream
{
public:
    wxFFileStream(const wxString& fileName, const wxString& mode = "w+b");

    wxFFileStream& operator=(wxFFileStream&&) = delete;

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

} // export

#endif // wxUSE_STREAMS

