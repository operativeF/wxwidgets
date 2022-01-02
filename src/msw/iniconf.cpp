///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/iniconf.cpp
// Purpose:     implementation of wxIniConfig class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     27.07.98
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_INICONF

#include "wx/string.h"
#include "wx/intl.h"
#include "wx/app.h"
#include "wx/utils.h"
#include "wx/log.h"

#include  "wx/config.h"
#include  "wx/file.h"

#include  "wx/msw/iniconf.h"

import <array>;
import <string>;
import <vector>;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// we replace all path separators with this character
constexpr char PATH_SEP_REPLACE =  '_';

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// ctor & dtor
// ----------------------------------------------------------------------------
wxIMPLEMENT_ABSTRACT_CLASS(wxIniConfig, wxConfigBase);

wxIniConfig::wxIniConfig(const wxString& strAppName,
                         const wxString& strVendor,
                         const wxString& localFilename,
                         const wxString& globalFilename,
                         unsigned int style)
           : wxConfigBase(strAppName, strVendor, localFilename, globalFilename, style)

#if 0 // This is too complex for some compilers, e.g. BC++ 5.01
           : wxConfigBase((strAppName.empty() && wxTheApp) ? wxTheApp->GetAppName()
                                               : strAppName,
                          strVendor.empty() ? (wxTheApp ? wxTheApp->GetVendorName()
                                                  : strAppName)
                                      : strVendor,
                          localFilename, globalFilename, style)
#endif
{
    if (strAppName.empty() && wxTheApp)
        SetAppName(wxTheApp->GetAppName());
    if (strVendor.empty() && wxTheApp)
        SetVendorName(wxTheApp->GetVendorName());

    m_strLocalFilename = localFilename;
    if (m_strLocalFilename.empty())
    {
        m_strLocalFilename = GetAppName() + ".ini";
    }

    // append the extension if none given and it's not an absolute file name
    // (otherwise we assume that they know what they're doing)
    if ( !wxIsPathSeparator(m_strLocalFilename[0u]) &&
        m_strLocalFilename.Find(wxT('.')) == wxNOT_FOUND )
    {
        m_strLocalFilename << ".ini";
    }

    // set root path
    SetPath({});
}

// ----------------------------------------------------------------------------
// path management
// ----------------------------------------------------------------------------

void wxIniConfig::SetPath(const wxString& strPath)
{
  std::vector<wxString> aParts;

  // TODO: Use lambda.
  if ( strPath.empty() ) {
    // nothing
  }
  else if ( strPath[0u] == wxCONFIG_PATH_SEPARATOR ) {
    // absolute path
    aParts = wxSplitPath(strPath);
  }
  else {
    // relative path, combine with current one
    wxString strFullPath = GetPath();
    strFullPath << wxCONFIG_PATH_SEPARATOR << strPath;
    aParts = wxSplitPath(strFullPath);
  }

  size_t nPartsCount = aParts.size();
  m_strPath.Empty();
  if ( nPartsCount == 0 ) {
    // go to the root
    m_strGroup = PATH_SEP_REPLACE;
  }
  else {
    // translate
    m_strGroup = aParts[0u];
    for ( size_t nPart = 1; nPart < nPartsCount; nPart++ ) {
      if ( nPart > 1 )
        m_strPath << PATH_SEP_REPLACE;
      m_strPath << aParts[nPart];
    }
  }

  // other functions assume that all this is true, i.e. there are no trailing
  // underscores at the end except if the group is the root one
  wxASSERT( (m_strPath.empty() || m_strPath.Last() != PATH_SEP_REPLACE) &&
            (m_strGroup == wxString(PATH_SEP_REPLACE) ||
             m_strGroup.Last() != PATH_SEP_REPLACE) );
}

const wxString& wxIniConfig::GetPath() const
{
  static wxString s_str;

  // always return abs path
  s_str = wxCONFIG_PATH_SEPARATOR;

  if ( m_strGroup == wxString(PATH_SEP_REPLACE) ) {
    // we're at the root level, nothing to do
  }
  else {
    s_str << m_strGroup;
    if ( !m_strPath.empty() )
      s_str << wxCONFIG_PATH_SEPARATOR;
    for ( const wxStringCharType *p = m_strPath.wx_str(); *p != '\0'; p++ ) {
      s_str << (*p == PATH_SEP_REPLACE ? wxCONFIG_PATH_SEPARATOR : *p);
    }
  }

  return s_str;
}

wxString wxIniConfig::GetPrivateKeyName(const wxString& szKey) const
{
  wxString strKey;

  if ( !m_strPath.empty() )
    strKey << m_strPath << PATH_SEP_REPLACE;

  strKey << szKey;

  return strKey;
}

wxString wxIniConfig::GetKeyName(const wxString& szKey) const
{
  wxString strKey;

  if ( m_strGroup != wxString(PATH_SEP_REPLACE) )
    strKey << m_strGroup << PATH_SEP_REPLACE;
  if ( !m_strPath.empty() )
    strKey << m_strPath << PATH_SEP_REPLACE;

  strKey << szKey;

  return strKey;
}

// ----------------------------------------------------------------------------
// enumeration
// ----------------------------------------------------------------------------

// not implemented
bool wxIniConfig::GetFirstGroup([[maybe_unused]] wxString& str, [[maybe_unused]] long& lIndex) const
{
    wxFAIL_MSG("not implemented");

    return false;
}

bool wxIniConfig::GetNextGroup ([[maybe_unused]] wxString& str, [[maybe_unused]] long& lIndex) const
{
    wxFAIL_MSG("not implemented");

    return false;
}

bool wxIniConfig::GetFirstEntry([[maybe_unused]] wxString& str, [[maybe_unused]] long& lIndex) const
{
    wxFAIL_MSG("not implemented");

    return false;
}

bool wxIniConfig::GetNextEntry ([[maybe_unused]] wxString& str, [[maybe_unused]] long& lIndex) const
{
    wxFAIL_MSG("not implemented");

    return false;
}

// ----------------------------------------------------------------------------
// misc info
// ----------------------------------------------------------------------------

// not implemented
size_t wxIniConfig::GetNumberOfEntries([[maybe_unused]] bool bRecursive) const
{
    wxFAIL_MSG("not implemented");

    return (size_t)-1;
}

size_t wxIniConfig::GetNumberOfGroups([[maybe_unused]] bool bRecursive) const
{
    wxFAIL_MSG("not implemented");

    return (size_t)-1;
}

bool wxIniConfig::HasGroup([[maybe_unused]] const wxString& strName) const
{
    wxFAIL_MSG("not implemented");

    return false;
}

bool wxIniConfig::HasEntry([[maybe_unused]] const wxString& strName) const
{
    wxFAIL_MSG("not implemented");

    return false;
}

// is current group empty?
bool wxIniConfig::IsEmpty() const
{
    std::array<wchar_t, 1024> szBuf;

    boost::nowide::wstackstring stackLocalFilename{m_strLocalFilename.c_str()};
    ::GetPrivateProfileStringW(m_strGroup.t_str(), nullptr, L"",
                            szBuf.data(), szBuf.size(),
                            stackLocalFilename.get());
    if ( !szBuf.empty() )
        return false;

    boost::nowide::wstackstring stackStrGroup{m_strGroup.c_str()};
    ::GetProfileStringW(m_strGroup.get(), nullptr, L"", szBuf.data(), szBuf.size());

    return szBuf.empty();
}

// ----------------------------------------------------------------------------
// read/write
// ----------------------------------------------------------------------------

bool wxIniConfig::DoReadString(const wxString& szKey, wxString *pstr) const
{
  wxConfigPathChanger path(this, szKey);
  wxString strKey = GetPrivateKeyName(path.Name());

  std::vector<wchar_t> szBuf;

  szBuf.resize(1024);

  // first look in the private INI file

  // NB: the lpDefault param to GetPrivateProfileString can't be NULL
  boost::nowide::wstackstring stackLocalFilename{m_strLocalFilename.c_str()};
  boost::nowide::wstackstring stackStrGroup{m_strGroup.c_str()};
  boost::nowide::wstackstring stackStrKey{strKey.c_str()};

  ::GetPrivateProfileString(stackStrGroup.get(), stackStrKey.get(), L"",
                          szBuf.data(), szBuf.size(),
                          stackLocalFilename.get());
  if ( szBuf.empty() ) {
    // now look in win.ini
    std::string strWinKey = GetKeyName(path.Name()).ToStdString();

    boost::nowide::stackStrWinKey{strWinKey.c_str()};

    ::GetProfileString(stackStrGroup.get(), stackStrWinKey.get(),
                       L"", szBuf.data(), 1024);
  }

  if ( szBuf )
    return false;

  *pstr = szBuf;
  return true;
}

bool wxIniConfig::DoReadLong(const wxString& szKey, long *pl) const
{
  wxConfigPathChanger path(this, szKey);
  wxString strKey = GetPrivateKeyName(path.Name());

  // hack: we have no mean to know if it really found the default value or
  // didn't find anything, so we call it twice

  static constexpr int nMagic  = 17; // 17 is some "rare" number
  static constexpr int nMagic2 = 28; // arbitrary number != nMagic
  long lVal = GetPrivateProfileInt(m_strGroup.t_str(), strKey.t_str(),
                                   nMagic, m_strLocalFilename.t_str());
  if ( lVal != nMagic ) {
    // the value was read from the file
    *pl = lVal;
    return true;
  }

  // is it really nMagic?
  lVal = GetPrivateProfileInt(m_strGroup.t_str(), strKey.t_str(),
                              nMagic2, m_strLocalFilename.t_str());
  if ( lVal != nMagic2 ) {
    // the nMagic it returned was indeed read from the file
    *pl = lVal;
    return true;
  }

  // CS : I have no idea why they should look up in win.ini
  // and if at all they have to do the same procedure using the two magic numbers
  // otherwise it always returns true, even if the key was not there at all
#if 0
  // no, it was just returning the default value, so now look in win.ini
 *pl = GetProfileInt(GetVendorName(), GetKeyName(szKey), *pl);

  return true;
#endif
  return false ;
}

bool wxIniConfig::DoWriteString(const wxString& szKey, const wxString& szValue)
{
  wxConfigPathChanger path(this, szKey);
  wxString strKey = GetPrivateKeyName(path.Name());

  bool bOk = WritePrivateProfileString(m_strGroup.t_str(), strKey.t_str(),
                                       szValue.t_str(),
                                       m_strLocalFilename.t_str()) != 0;

  if ( !bOk )
  {
    wxLogLastError("WritePrivateProfileString");
  }

  return bOk;
}

bool wxIniConfig::DoWriteLong(const wxString& szKey, long lValue)
{
  return Write(szKey, wxString::Format("%ld", lValue));
}

bool wxIniConfig::DoReadBinary([[maybe_unused]] const wxString& key,
                               [[maybe_unused]] wxMemoryBuffer * buf) const
{
    wxFAIL_MSG("not implemented");

    return false;
}

bool wxIniConfig::DoWriteBinary([[maybe_unused]] const wxString& key,
                                [[maybe_unused]] const wxMemoryBuffer& buf)
{
    wxFAIL_MSG("not implemented");

    return false;
}

bool wxIniConfig::Flush(bool /* bCurrentOnly */)
{
  // this is just the way it works
  return WritePrivateProfileString(nullptr, nullptr, nullptr,
                                   m_strLocalFilename.t_str()) != 0;
}

// ----------------------------------------------------------------------------
// delete
// ----------------------------------------------------------------------------

bool wxIniConfig::DeleteEntry(const wxString& szKey, bool bGroupIfEmptyAlso)
{
  // passing NULL as value to WritePrivateProfileString deletes the key
  wxConfigPathChanger path(this, szKey);
  wxString strKey = GetPrivateKeyName(path.Name());

  if (WritePrivateProfileString(m_strGroup.t_str(), strKey.t_str(),
                                nullptr, m_strLocalFilename.t_str()) == 0)
    return false;

  if ( !bGroupIfEmptyAlso || !IsEmpty() )
    return true;

  // delete the current group too
  bool bOk = WritePrivateProfileString(m_strGroup.t_str(), nullptr,
                                       nullptr, m_strLocalFilename.t_str()) != 0;

  if ( !bOk )
  {
    wxLogLastError("WritePrivateProfileString");
  }

  return bOk;
}

bool wxIniConfig::DeleteGroup(const wxString& szKey)
{
  wxConfigPathChanger path(this, szKey);

  // passing NULL as section name to WritePrivateProfileString deletes the
  // whole section according to the docs
  bool bOk = WritePrivateProfileString(path.Name().t_str(), nullptr,
                                       nullptr, m_strLocalFilename.t_str()) != 0;

  if ( !bOk )
  {
    wxLogLastError("WritePrivateProfileString");
  }

  return bOk;
}

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

bool wxIniConfig::DeleteAll()
{
  // first delete our group in win.ini
  WriteProfileString(GetVendorName().t_str(), nullptr, nullptr);

  // then delete our own ini file
  wxChar szBuf[MAX_PATH];
  size_t nRc = GetWindowsDirectory(szBuf, WXSIZEOF(szBuf));
  if ( nRc == 0 )
  {
    wxLogLastError("GetWindowsDirectory");
  }
  else if ( nRc > WXSIZEOF(szBuf) )
  {
    wxFAIL_MSG("buffer is too small for Windows directory.");
  }

  wxString strFile = szBuf;
  strFile << '\\' << m_strLocalFilename;

  if ( wxFile::Exists(strFile) && !wxRemoveFile(strFile) ) {
    wxLogSysError(_("Can't delete the INI file '%s'"), strFile.c_str());
    return false;
  }

  return true;
}

bool wxIniConfig::RenameEntry([[maybe_unused]] const wxString& oldName,
                              [[maybe_unused]] const wxString& newName)
{
    // Not implemented
    return false;
}

bool wxIniConfig::RenameGroup([[maybe_unused]] const wxString& oldName,
                              [[maybe_unused]] const wxString& newName)
{
    // Not implemented
    return false;
}

#endif // wxUSE_INICONF
