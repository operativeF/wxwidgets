/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/wfstream.cpp
// Purpose:     "File stream" classes
// Author:      Julian Smart
// Modified by:
// Created:     11/07/98
// Copyright:   (c) Guilhem Lavaux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/filefn.h"

module WX.Cmn.WFStream;

import WX.Cmn.Stream;
import WX.File.Flags;

import <cstdint>;

#if wxUSE_STREAMS

#if wxUSE_FILE

// ----------------------------------------------------------------------------
// wxFileInputStream
// ----------------------------------------------------------------------------

wxFileInputStream::wxFileInputStream(const wxString& fileName)
   : m_file(new wxFile(fileName, wxFile::read)),
     m_file_destroy(true)
{
    if ( !m_file->IsOpened() )
        m_lasterror = wxSTREAM_READ_ERROR;
}

wxFileInputStream::wxFileInputStream(wxFile& file)
    : m_file(&file)
{
}

wxFileInputStream::wxFileInputStream(int fd)
    : m_file(new wxFile(fd)),
      m_file_destroy(true)
{
}

wxFileInputStream::~wxFileInputStream()
{
    if (m_file_destroy)
        delete m_file;
}

wxFileOffset wxFileInputStream::GetLength() const
{
    return m_file->Length();
}

size_t wxFileInputStream::OnSysRead(void *buffer, size_t size)
{
    ssize_t ret = m_file->Read(buffer, size);

    // NB: we can't use a switch here because HP-UX CC doesn't allow
    //     switching over long long (which size_t is in 64bit mode)

    if ( !ret )
    {
        // nothing read, so nothing more to read
        m_lasterror = wxSTREAM_EOF;
    }
    else if ( ret == wxInvalidOffset )
    {
        m_lasterror = wxSTREAM_READ_ERROR;
        ret = 0;
    }
    else
    {
        // normal case
        m_lasterror = wxSTREAM_NO_ERROR;
    }

    return ret;
}

wxFileOffset wxFileInputStream::OnSysSeek(wxFileOffset pos, wxSeekMode mode)
{
    return m_file->Seek(pos, mode);
}

wxFileOffset wxFileInputStream::OnSysTell() const
{
    return m_file->Tell();
}

bool wxFileInputStream::IsOk() const
{
    return wxInputStream::IsOk() && m_file->IsOpened();
}

// ----------------------------------------------------------------------------
// wxFileOutputStream
// ----------------------------------------------------------------------------

wxFileOutputStream::wxFileOutputStream(const wxString& fileName)
    : m_file(new wxFile(fileName, wxFile::write)),
      m_file_destroy(true)
{
    if (!m_file->IsOpened())
        m_lasterror = wxSTREAM_WRITE_ERROR;
}

wxFileOutputStream::wxFileOutputStream(wxFile& file)
    : m_file(&file)
{
}

wxFileOutputStream::wxFileOutputStream(int fd)
    : m_file(new wxFile(fd)),
      m_file_destroy(true)
{
}

wxFileOutputStream::~wxFileOutputStream()
{
    if (m_file_destroy)
    {
        Sync();
        delete m_file;
    }
}

size_t wxFileOutputStream::OnSysWrite(const void *buffer, size_t size)
{
    size_t ret = m_file->Write(buffer, size);

    m_lasterror = m_file->Error() ? wxSTREAM_WRITE_ERROR : wxSTREAM_NO_ERROR;

    return ret;
}

wxFileOffset wxFileOutputStream::OnSysTell() const
{
    return m_file->Tell();
}

wxFileOffset wxFileOutputStream::OnSysSeek(wxFileOffset pos, wxSeekMode mode)
{
    return m_file->Seek(pos, mode);
}

void wxFileOutputStream::Sync()
{
    wxOutputStream::Sync();
    m_file->Flush();
}

wxFileOffset wxFileOutputStream::GetLength() const
{
    return m_file->Length();
}

bool wxFileOutputStream::IsOk() const
{
    return wxOutputStream::IsOk() && m_file->IsOpened();
}

// ----------------------------------------------------------------------------
// wxTempFileOutputStream
// ----------------------------------------------------------------------------

wxTempFileOutputStream::wxTempFileOutputStream(const wxString& fileName)
    : m_file(new wxTempFile(fileName))
{
    if (!m_file->IsOpened())
        m_lasterror = wxSTREAM_WRITE_ERROR;
}

wxTempFileOutputStream::~wxTempFileOutputStream()
{
    if (m_file->IsOpened())
        Discard();
    delete m_file;
}

size_t wxTempFileOutputStream::OnSysWrite(const void *buffer, size_t size)
{
    if (IsOk() && m_file->Write(buffer, size))
        return size;
    m_lasterror = wxSTREAM_WRITE_ERROR;
    return 0;
}

// ----------------------------------------------------------------------------
// wxTempFFileOutputStream
// ----------------------------------------------------------------------------

wxTempFFileOutputStream::wxTempFFileOutputStream(const wxString& fileName)
    : m_file(new wxTempFFile(fileName))
{
    if (!m_file->IsOpened())
        m_lasterror = wxSTREAM_WRITE_ERROR;
}

wxTempFFileOutputStream::~wxTempFFileOutputStream()
{
    if (m_file->IsOpened())
        Discard();
    delete m_file;
}

size_t wxTempFFileOutputStream::OnSysWrite(const void *buffer, size_t size)
{
    if (IsOk() && m_file->Write(buffer, size))
        return size;
    m_lasterror = wxSTREAM_WRITE_ERROR;
    return 0;
}

// ----------------------------------------------------------------------------
// wxFileStream
// ----------------------------------------------------------------------------

wxFileStream::wxFileStream(const wxString& fileName)
{
    wxFileOutputStream::m_file =
    wxFileInputStream::m_file = new wxFile(fileName, wxFile::read_write);

    // this is a bit ugly as streams are symmetric but we still have to delete
    // the file we created above exactly once so we decide to (arbitrarily) do
    // it in wxFileInputStream
    wxFileInputStream::m_file_destroy = true;
}

bool wxFileStream::IsOk() const
{
    return wxFileOutputStream::IsOk() && wxFileInputStream::IsOk();
}

#endif // wxUSE_FILE

#if wxUSE_FFILE

// ----------------------------------------------------------------------------
// wxFFileInputStream
// ----------------------------------------------------------------------------

wxFFileInputStream::wxFFileInputStream(const wxString& fileName,
                                       const wxString& mode)
    : m_file(new wxFFile(fileName, mode)),
      m_file_destroy(true)
{
    if (!m_file->IsOpened())
        m_lasterror = wxSTREAM_WRITE_ERROR;
}

wxFFileInputStream::wxFFileInputStream(wxFFile& file)
    : m_file(&file)
{
}

wxFFileInputStream::wxFFileInputStream(FILE *file)
    : m_file(new wxFFile(file)),
      m_file_destroy(true)
{
}

wxFFileInputStream::~wxFFileInputStream()
{
    if (m_file_destroy)
        delete m_file;
}

wxFileOffset wxFFileInputStream::GetLength() const
{
    return m_file->Length();
}

size_t wxFFileInputStream::OnSysRead(void *buffer, size_t size)
{
    ssize_t ret = m_file->Read(buffer, size);

    // It is not safe to call Eof() if the file is not opened.
    if (!m_file->IsOpened() || m_file->Eof())
        m_lasterror = wxSTREAM_EOF;
    if (ret == wxInvalidOffset)
    {
        m_lasterror = wxSTREAM_READ_ERROR;
        ret = 0;
    }

    return ret;
}

wxFileOffset wxFFileInputStream::OnSysSeek(wxFileOffset pos, wxSeekMode mode)
{
    return m_file->Seek(pos, mode) ? m_file->Tell() : wxInvalidOffset;
}

wxFileOffset wxFFileInputStream::OnSysTell() const
{
    return m_file->Tell();
}

bool wxFFileInputStream::IsOk() const
{
    return wxStreamBase::IsOk() && m_file->IsOpened();
}

// ----------------------------------------------------------------------------
// wxFFileOutputStream
// ----------------------------------------------------------------------------

wxFFileOutputStream::wxFFileOutputStream(const wxString& fileName,
                                         const wxString& mode)
    : m_file(new wxFFile(fileName, mode)),
      m_file_destroy(true)
{
    if (!m_file->IsOpened())
    {
        m_lasterror = wxSTREAM_WRITE_ERROR;
    }
    else
    {
        if (m_file->Error())
            m_lasterror = wxSTREAM_WRITE_ERROR;
    }
}

wxFFileOutputStream::wxFFileOutputStream(wxFFile& file)
    : m_file(&file)
{
}

wxFFileOutputStream::wxFFileOutputStream(FILE *file)
    : m_file(new wxFFile(file)),
      m_file_destroy(true)
{
}

wxFFileOutputStream::~wxFFileOutputStream()
{
    if (m_file_destroy)
    {
        Sync();
        delete m_file;
    }
}

size_t wxFFileOutputStream::OnSysWrite(const void *buffer, size_t size)
{
    const size_t ret = m_file->Write(buffer, size);
    // It is not safe to call Error() if the file is not opened.
    if (!m_file->IsOpened() || m_file->Error())
        m_lasterror = wxSTREAM_WRITE_ERROR;
    else
        m_lasterror = wxSTREAM_NO_ERROR;
    return ret;
}

wxFileOffset wxFFileOutputStream::OnSysTell() const
{
    return m_file->Tell();
}

wxFileOffset wxFFileOutputStream::OnSysSeek(wxFileOffset pos, wxSeekMode mode)
{
    return m_file->Seek(pos, mode) ? m_file->Tell() : wxInvalidOffset;
}

void wxFFileOutputStream::Sync()
{
    wxOutputStream::Sync();
    m_file->Flush();
}

wxFileOffset wxFFileOutputStream::GetLength() const
{
    return m_file->Length();
}

bool wxFFileOutputStream::IsOk() const
{
    return wxStreamBase::IsOk() && m_file->IsOpened();
}

// ----------------------------------------------------------------------------
// wxFFileStream
// ----------------------------------------------------------------------------

wxFFileStream::wxFFileStream(const wxString& fileName, const wxString& mode)
{
    wxASSERT_MSG( mode.find_first_of('+') != wxString::npos,
                  "must be opened in read-write mode for this class to work" );

    wxFFileOutputStream::m_file =
    wxFFileInputStream::m_file = new wxFFile(fileName, mode);

    // see comment in wxFileStream ctor
    wxFFileInputStream::m_file_destroy = true;
}

bool wxFFileStream::IsOk() const
{
    return wxFFileOutputStream::IsOk() && wxFFileInputStream::IsOk();
}

#endif //wxUSE_FFILE

#endif // wxUSE_STREAMS
