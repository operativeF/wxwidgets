/////////////////////////////////////////////////////////////////////////////
// Name:        wx/ffile.h
// Purpose:     wxFFile - encapsulates "FILE *" stream
// Author:      Vadim Zeitlin
// Modified by:
// Created:     14.07.99
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include  "wx/string.h"
#include  "wx/filefn.h"

export module WX.Cmn.FFile;

import WX.Cmn.ConvAuto;
import WX.File.Flags;

import <string>;

#if wxUSE_FFILE

export
{

// ----------------------------------------------------------------------------
// class wxFFile: standard C stream library IO
//
// NB: for space efficiency this class has no virtual functions, including
//     dtor which is _not_ virtual, so it shouldn't be used as a base class.
// ----------------------------------------------------------------------------

class wxFFile
{
public:
  // ctors
  // -----
    // def ctor
  wxFFile() { m_fp = nullptr; }
    // open specified file (may fail, use IsOpened())
  wxFFile(const wxString& filename, const wxString& mode = "r");
    // attach to (already opened) file
  wxFFile(FILE *lfp) { m_fp = lfp; }

  // open/close
    // open a file (existing or not - the mode controls what happens)
  bool Open(const wxString& filename, const wxString& mode = "r");
    // closes the opened file (this is a NOP if not opened)
  bool Close();

  // assign an existing file descriptor and get it back from wxFFile object
  void Attach(FILE *lfp, const wxString& name = {})
    { Close(); m_fp = lfp; m_name = name; }
  FILE* Detach() { FILE* fpOld = m_fp; m_fp = nullptr; return fpOld; }
  FILE *fp() const { return m_fp; }

  // read/write (unbuffered)
    // read all data from the file into a string (useful for text files)
  bool ReadAll(wxString *str, const wxMBConv& conv = wxConvAuto());
    // returns number of bytes read - use Eof() and Error() to see if an error
    // occurred or not
  size_t Read(void *pBuf, size_t nCount);
    // returns the number of bytes written
  size_t Write(const void *pBuf, size_t nCount);
    // returns true on success
  bool Write(const wxString& s, const wxMBConv& conv = wxConvAuto());
    // flush data not yet written
  bool Flush();

  // file pointer operations (return ofsInvalid on failure)
    // move ptr ofs bytes related to start/current pos/end of file
  bool Seek(wxFileOffset ofs, wxSeekMode mode = wxSeekMode::FromStart);
    // move ptr to ofs bytes before the end
  bool SeekEnd(wxFileOffset ofs = 0) { return Seek(ofs, wxSeekMode::FromEnd); }
    // get current position in the file
  wxFileOffset Tell() const;
    // get current file length
  wxFileOffset Length() const;

  // simple accessors: note that Eof() and Error() may only be called if
  // IsOpened(). Otherwise they assert and return false.
    // is file opened?
  bool IsOpened() const { return m_fp != nullptr; }
    // is end of file reached?
  bool Eof() const;
    // has an error occurred?
  bool Error() const;
    // get the file name
  const wxString& GetName() const { return m_name; }
    // type such as disk or pipe
  wxFileKind GetKind() const { return wxGetFileKind(m_fp); }

  // dtor closes the file if opened
  ~wxFFile() { Close(); }

private:
  // copy ctor and assignment operator are private because it doesn't make
  // sense to copy files this way: attempt to do it will provoke a compile-time
  // error.
  wxFFile(const wxFFile&);
  wxFFile& operator=(const wxFFile&);

  FILE *m_fp{nullptr};       // IO stream or NULL if not opened

  wxString m_name;  // the name of the file (for diagnostic messages)
};

// ----------------------------------------------------------------------------
// class wxTempFFile: if you want to replace another file, create an instance
// of wxTempFFile passing the name of the file to be replaced to the ctor. Then
// you can write to wxTempFFile and call Commit() function to replace the old
// file (and close this one) or call Discard() to cancel the modification. If
// you call neither of them, dtor will call Discard().
// ----------------------------------------------------------------------------

class wxTempFFile
{
public:
  // ctors
    // default
  wxTempFFile() = default;
    // associates the temp file with the file to be replaced and opens it
  explicit wxTempFFile(const std::string& strName);

  wxTempFFile& operator=(wxTempFFile&&) = delete;

  // open the temp file (strName is the name of file to be replaced)
  bool Open(const std::string& strName);

  // is the file opened?
  bool IsOpened() const { return m_file.IsOpened(); }
    // get current file length
  wxFileOffset Length() const { return m_file.Length(); }
    // move ptr ofs bytes related to start/current pos/end of file
  bool Seek(wxFileOffset ofs, wxSeekMode mode = wxSeekMode::FromStart)
    { return m_file.Seek(ofs, mode); }
    // get current position in the file
  wxFileOffset Tell() const { return m_file.Tell(); }

  // I/O (both functions return true on success, false on failure)
  bool Write(const void *p, size_t n) { return m_file.Write(p, n) == n; }
  bool Write(const wxString& str, const wxMBConv& conv = wxMBConvUTF8())
    { return m_file.Write(str, conv); }

  // flush data: can be called before closing file to ensure that data was
  // correctly written out
  bool Flush() { return m_file.Flush(); }

  // different ways to close the file
    // validate changes and delete the old file of name m_strName
  bool Commit();
    // discard changes
  void Discard();

  // dtor calls Discard() if file is still opened
 ~wxTempFFile();

private:
  std::string  m_strName;  // name of the file to replace in Commit()
  std::string  m_strTemp;  // temporary file name
  
  wxFFile   m_file;     // the temporary file
};

} // export

#endif // wxUSE_FFILE
