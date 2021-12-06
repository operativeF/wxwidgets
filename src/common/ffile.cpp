/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/ffile.cpp
// Purpose:     wxFFile encapsulates "FILE *" IO stream
// Author:      Vadim Zeitlin
// Modified by:
// Created:     14.07.99
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_FFILE

#include "wx/intl.h"
#include "wx/log.h"
#include "wx/crt.h"
#include "wx/filename.h"
#include "wx/ffile.h"

import WX.Utils.Cast;

// ============================================================================
// implementation of wxFFile
// ============================================================================

// ----------------------------------------------------------------------------
// opening the file
// ----------------------------------------------------------------------------

wxFFile::wxFFile(const wxString& filename, const wxString& mode)
{
    std::ignore = Open(filename, mode);
}

bool wxFFile::Open(const wxString& filename, const wxString& mode)
{
    wxASSERT_MSG( !m_fp, "should close or detach the old file first" );

    FILE* const fp = wxFopen(filename, mode);

    if ( !fp )
    {
        wxLogSysError(_("can't open file '%s'"), filename);

        return false;
    }

    Attach(fp, filename);

    return true;
}

bool wxFFile::Close()
{
    if ( IsOpened() )
    {
        if ( fclose(m_fp) != 0 )
        {
            wxLogSysError(_("can't close file '%s'"), m_name.c_str());

            return false;
        }

        m_fp = nullptr;
    }

    return true;
}

// ----------------------------------------------------------------------------
// read/write
// ----------------------------------------------------------------------------

bool wxFFile::ReadAll(wxString *str, const wxMBConv& conv)
{
    wxCHECK_MSG( str, false, "invalid parameter" );
    wxCHECK_MSG( IsOpened(), false, "can't read from closed file" );
    wxCHECK_MSG( Length() >= 0, false, "invalid length" );
    size_t length = wx::narrow_cast<size_t>(Length());
    wxCHECK_MSG( (wxFileOffset)length == Length(), false, "huge file not supported" );

    clearerr(m_fp);

    wxCharBuffer buf(length);

    // note that real length may be less than file length for text files with DOS EOLs
    // ('\r's get dropped by CRT when reading which means that we have
    // realLen = fileLen - numOfLinesInTheFile)
    length = fread(buf.data(), 1, length, m_fp);

    if ( Error() )
    {
        wxLogSysError(_("Read error on file '%s'"), m_name.c_str());

        return false;
    }

    // shrink the buffer to possibly shorter data as explained above:
    buf.shrink(length);

    wxString strTmp(buf, conv);
    str->swap(strTmp);

    return true;
}

size_t wxFFile::Read(void *pBuf, size_t nCount)
{
    if ( !nCount )
        return 0;

    wxCHECK_MSG( pBuf, 0, "invalid parameter" );
    wxCHECK_MSG( IsOpened(), 0, "can't read from closed file" );

    const size_t nRead = fread(pBuf, 1, nCount, m_fp);
    if ( (nRead < nCount) && Error() )
    {
        wxLogSysError(_("Read error on file '%s'"), m_name.c_str());
    }

    return nRead;
}

size_t wxFFile::Write(const void *pBuf, size_t nCount)
{
    if ( !nCount )
        return 0;

    wxCHECK_MSG( pBuf, 0, "invalid parameter" );
    wxCHECK_MSG( IsOpened(), 0, "can't write to closed file" );

    size_t nWritten = fwrite(pBuf, 1, nCount, m_fp);
    if ( nWritten < nCount )
    {
        wxLogSysError(_("Write error on file '%s'"), m_name.c_str());
    }

    return nWritten;
}

bool wxFFile::Write(const wxString& s, const wxMBConv& conv)
{
    // Writing nothing always succeeds -- and simplifies the check for
    // conversion failure below.
    if ( s.empty() )
        return true;

    const wxWX2MBbuf buf = s.mb_str(conv);

    const size_t size = buf.length();

    if ( !size )
    {
        // This means that the conversion failed as the original string wasn't
        // empty (we explicitly checked for this above) and in this case we
        // must fail too to indicate that we can't save the data.
        return false;
    }

    return Write(buf, size) == size;
}

bool wxFFile::Flush()
{
    if ( IsOpened() )
    {
        if ( fflush(m_fp) != 0 )
        {
            wxLogSysError(_("failed to flush the file '%s'"), m_name.c_str());

            return false;
        }
    }

    return true;
}

// ----------------------------------------------------------------------------
// seeking
// ----------------------------------------------------------------------------

bool wxFFile::Seek(wxFileOffset ofs, wxSeekMode mode)
{
    wxCHECK_MSG( IsOpened(), false, "can't seek on closed file" );

    const int origin = [mode]() {
        switch ( mode )
        {
            default:
                wxFAIL_MSG("unknown seek mode");
                [[fallthrough]];

            case wxSeekMode::FromStart:
                return SEEK_SET;

            case wxSeekMode::FromCurrent:
                return SEEK_CUR;

            case wxSeekMode::FromEnd:
                return SEEK_END;
        }
    }();

#ifndef wxHAS_LARGE_FFILES
    if ((long)ofs != ofs)
    {
        wxLogError(_("Seek error on file '%s' (large files not supported by stdio)"), m_name.c_str());

        return false;
    }

    if ( wxFseek(m_fp, (long)ofs, origin) != 0 )
#else
    if ( wxFseek(m_fp, ofs, origin) != 0 )
#endif
    {
        wxLogSysError(_("Seek error on file '%s'"), m_name.c_str());

        return false;
    }

    return true;
}

wxFileOffset wxFFile::Tell() const
{
    wxCHECK_MSG( IsOpened(), wxInvalidOffset,
                 "wxFFile::Tell(): file is closed!" );

    const wxFileOffset rc = wxFtell(m_fp);
    if ( rc == wxInvalidOffset )
    {
        wxLogSysError(_("Can't find current position in file '%s'"),
                      m_name.c_str());
    }

    return rc;
}

wxFileOffset wxFFile::Length() const
{
    wxCHECK_MSG( IsOpened(), wxInvalidOffset,
                 "wxFFile::Length(): file is closed!" );

    const wxFileOffset posOld = Tell();
    if ( posOld != wxInvalidOffset )
    {
        wxFFile& self = *const_cast<wxFFile*>(this);

        if ( self.SeekEnd() )
        {
            const wxFileOffset len = Tell();

            std::ignore = self.Seek(posOld);

            return len;
        }
    }

    return wxInvalidOffset;
}

bool wxFFile::Eof() const
{
    wxCHECK_MSG( IsOpened(), false,
                 "wxFFile::Eof(): file is closed!" );
    return feof(m_fp) != 0;
}

bool wxFFile::Error() const
{
    wxCHECK_MSG( IsOpened(), false,
                 "wxFFile::Error(): file is closed!" );
    return ferror(m_fp) != 0;
}

// ============================================================================
// implementation of wxTempFFile
// ============================================================================

// ----------------------------------------------------------------------------
// construction
// ----------------------------------------------------------------------------

wxTempFFile::wxTempFFile(const wxString& strName)
{
    Open(strName);
}

bool wxTempFFile::Open(const wxString& strName)
{
    // we must have an absolute filename because otherwise CreateTempFileName()
    // would create the temp file in $TMP (i.e. the system standard location
    // for the temp files) which might be on another volume/drive/mount and
    // wxRename()ing it later to m_strName from Commit() would then fail
    //
    // with the absolute filename, the temp file is created in the same
    // directory as this one which ensures that wxRename() may work later
    wxFileName fn(strName);
    if ( !fn.IsAbsolute() )
    {
        fn.Normalize(wxPATH_NORM_ABSOLUTE);
    }

    m_strName = fn.GetFullPath();

    m_strTemp = wxFileName::CreateTempFileName(m_strName, &m_file);

    if ( m_strTemp.empty() )
    {
        // CreateTempFileName() failed
        return false;
    }

#ifdef __UNIX__
    // the temp file should have the same permissions as the original one
    mode_t mode;

    wxStructStat st;
    if ( stat( (const char*) m_strName.fn_str(), &st) == 0 )
    {
        mode = st.st_mode;
    }
    else
    {
        // file probably didn't exist, just give it the default mode _using_
        // user's umask (new files creation should respect umask)
        mode_t mask = umask(0777);
        mode = 0666 & ~mask;
        umask(mask);
    }

    if ( chmod( (const char*) m_strTemp.fn_str(), mode) == -1 )
    {
        wxLogSysError(_("Failed to set temporary file permissions"));
    }
#endif // Unix

    return true;
}

// ----------------------------------------------------------------------------
// destruction
// ----------------------------------------------------------------------------

wxTempFFile::~wxTempFFile()
{
    if ( IsOpened() )
        Discard();
}

bool wxTempFFile::Commit()
{
    m_file.Close();

    if ( wxFile::Exists(m_strName) && wxRemove(m_strName) != 0 ) {
        wxLogSysError(_("can't remove file '%s'"), m_strName.c_str());
        return false;
    }

    if ( !wxRenameFile(m_strTemp, m_strName)  ) {
        wxLogSysError(_("can't commit changes to file '%s'"), m_strName.c_str());
        return false;
    }

    return true;
}

void wxTempFFile::Discard()
{
    m_file.Close();
    if ( wxRemove(m_strTemp) != 0 )
    {
        wxLogSysError(_("can't remove temporary file '%s'"), m_strTemp.c_str());
    }
}

#endif // wxUSE_FFILE
