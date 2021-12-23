///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/registry.cpp
// Purpose:     implementation of registry classes and functions
// Author:      Vadim Zeitlin
// Modified by:
// Created:     03.04.98
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
// TODO:        - parsing of registry key names
//              - support of other (than REG_SZ/REG_DWORD) registry types
//              - add high level functions (RegisterOleServer, ...)
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_REGKEY

#include "wx/msw/private.h"

#include "wx/intl.h"
#include "wx/log.h"
#include "wx/crt.h"
#include "wx/utils.h"
#include "wx/dynlib.h"
#include "wx/file.h"

#include <fmt/core.h>

#ifndef _MAX_PATH
    #define _MAX_PATH 512
#endif

// our header
//#define   HKEY_DEFINED    // already defined in windows.h
#include  "wx/msw/registry.h"

import WX.WinDef;
import WX.Cmn.WFStream;

import <string>;

// some registry functions don't like signed chars
using RegString = unsigned char *;
using RegBinary = BYTE *;

#ifndef KEY_WOW64_64KEY
    #define KEY_WOW64_64KEY 0x0100
#endif

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// the standard key names, short names and handles all bundled together for
// convenient access
static struct
{
  HKEY        hkey;
  const char* szName;
  const char* szShortName;
}
aStdKeys[] =
{
  { HKEY_CLASSES_ROOT,      "HKEY_CLASSES_ROOT",      "HKCR" },
  { HKEY_CURRENT_USER,      "HKEY_CURRENT_USER",      "HKCU" },
  { HKEY_LOCAL_MACHINE,     "HKEY_LOCAL_MACHINE",     "HKLM" },
  { HKEY_USERS,             "HKEY_USERS",             "HKU"  }, // short name?
  { HKEY_PERFORMANCE_DATA,  "HKEY_PERFORMANCE_DATA",  "HKPD" }, // (Obsolete under XP and later)
  { HKEY_CURRENT_CONFIG,    "HKEY_CURRENT_CONFIG",    "HKCC" },
  { HKEY_DYN_DATA,          "HKEY_DYN_DATA",          "HKDD" }, // (Obsolete under XP and later)
};

// the registry name separator (perhaps one day MS will change it to '/' ;-)
#define   REG_SEPARATOR     wxT('\\')

// ----------------------------------------------------------------------------
// macros
// ----------------------------------------------------------------------------

#define CONST_CAST const_cast<wxRegKey*>(this)->

// ----------------------------------------------------------------------------
// non member functions
// ----------------------------------------------------------------------------

// removes the trailing backslash from the string if it has one
static inline void RemoveTrailingSeparator(std::string& str);

// returns true if given registry key exists
static bool KeyExists(
    WXHKEY hRootKey,
    const std::string& szKey,
    wxRegKey::WOW64ViewMode viewMode = wxRegKey::WOW64ViewMode_Default);

// return the WOW64 registry view flag which can be used with MSW registry
// functions for opening the key in the specified view
static long GetMSWViewFlags(wxRegKey::WOW64ViewMode viewMode);

// return the access rights which can be used with MSW registry functions for
// opening the key in the specified mode
static long
GetMSWAccessFlags(wxRegKey::AccessMode mode, wxRegKey::WOW64ViewMode viewMode);

// combines value and key name
static std::string GetFullName(const wxRegKey *pKey);
static std::string GetFullName(const wxRegKey *pKey, const std::string& szValue);

// returns "value" argument of wxRegKey methods converted into a value that can
// be passed to win32 registry functions; specifically, converts empty string
// to NULL
static inline const char* RegValueStr(const std::string& szValue);

// Return the user-readable name of the given REG_XXX type constant.
static std::string GetTypeString(WXDWORD dwType)
{
#define REG_TYPE_TO_STR(type) case REG_ ## type: return #type

    switch ( dwType )
    {
        REG_TYPE_TO_STR(NONE);
        REG_TYPE_TO_STR(SZ);
        REG_TYPE_TO_STR(EXPAND_SZ);
        REG_TYPE_TO_STR(BINARY);
        REG_TYPE_TO_STR(DWORD);
        // REG_TYPE_TO_STR(DWORD_LITTLE_ENDIAN); -- same as REG_DWORD
        REG_TYPE_TO_STR(DWORD_BIG_ENDIAN);
        REG_TYPE_TO_STR(LINK);
        REG_TYPE_TO_STR(MULTI_SZ);
        REG_TYPE_TO_STR(RESOURCE_LIST);
        REG_TYPE_TO_STR(FULL_RESOURCE_DESCRIPTOR);
        REG_TYPE_TO_STR(RESOURCE_REQUIREMENTS_LIST);
        REG_TYPE_TO_STR(QWORD);
        // REG_TYPE_TO_STR(QWORD_LITTLE_ENDIAN); -- same as REG_QWORD

        default:  // FIXME: Removed translation for fmt lib
            return fmt::format("unknown (%lu)", dwType);
    }
}

// ============================================================================
// implementation of wxRegKey class
// ============================================================================

// ----------------------------------------------------------------------------
// static functions and variables
// ----------------------------------------------------------------------------

const size_t wxRegKey::nStdKeys = WXSIZEOF(aStdKeys);

// @@ should take a `StdKey key', but as it's often going to be used in loops
//    it would require casts in user code.
const char *wxRegKey::GetStdKeyName(size_t key)
{
  // return empty string if key is invalid
  wxCHECK_MSG( key < nStdKeys, {}, "invalid key in wxRegKey::GetStdKeyName" );

  return aStdKeys[key].szName;
}

const char *wxRegKey::GetStdKeyShortName(size_t key)
{
  // return empty string if key is invalid
  wxCHECK( key < nStdKeys, {} );

  return aStdKeys[key].szShortName;
}

wxRegKey::StdKey wxRegKey::ExtractKeyName(std::string& strKey)
{
  std::string strRoot = wx::utils::BeforeFirst(strKey, REG_SEPARATOR);

  // TODO: Lambda
  size_t ui;
  for ( ui = 0; ui < nStdKeys; ui++ ) {
    if ( wx::utils::CmpNoCase(strRoot, aStdKeys[ui].szName) == 0 ||
         wx::utils::CmpNoCase(strRoot, aStdKeys[ui].szShortName) == 0 ) {
      break;
    }
  }

  if ( ui == nStdKeys ) {
    wxFAIL_MSG("invalid key prefix in wxRegKey::ExtractKeyName.");

    ui = HKCR;
  }
  else {
    strKey = wx::utils::AfterFirst(strKey, REG_SEPARATOR);
    if ( !strKey.empty() && strKey.back() == REG_SEPARATOR )
      strKey.pop_back();
  }

  return (StdKey)ui;
}

wxRegKey::StdKey wxRegKey::GetStdKeyFromHkey(WXHKEY hkey)
{
  for ( size_t ui = 0; ui < nStdKeys; ui++ ) {
    if ( aStdKeys[ui].hkey == (HKEY)hkey )
      return (StdKey)ui;
  }

  wxFAIL_MSG("non root hkey passed to wxRegKey::GetStdKeyFromHkey.");

  return HKCR;
}

// ----------------------------------------------------------------------------
// ctors and dtor
// ----------------------------------------------------------------------------

wxRegKey::wxRegKey(WOW64ViewMode viewMode)
  : m_viewMode(viewMode),
    m_hRootKey((WXHKEY) aStdKeys[HKCR].hkey)
{
}

wxRegKey::wxRegKey(const std::string& strKey, WOW64ViewMode viewMode)
    : m_strKey(strKey),
      m_viewMode(viewMode),
      m_hRootKey((WXHKEY) aStdKeys[ExtractKeyName(m_strKey)].hkey)
{
}

// parent is a predefined (and preopened) key
wxRegKey::wxRegKey(StdKey keyParent,
                   const std::string& strKey,
                   WOW64ViewMode viewMode)
    : m_strKey(strKey), m_viewMode(viewMode),
      m_hRootKey((WXHKEY) aStdKeys[keyParent].hkey)
{
  RemoveTrailingSeparator(m_strKey);
}

// parent is a normal regkey
wxRegKey::wxRegKey(const wxRegKey& keyParent, const std::string& strKey)
    : m_strKey(keyParent.m_strKey), m_viewMode(keyParent.GetView())
{
  // combine our name with parent's to get the full name
  if ( !m_strKey.empty() &&
       (strKey.empty() || strKey[0] != REG_SEPARATOR) ) {
      m_strKey += REG_SEPARATOR;
  }

  m_strKey += strKey;
  RemoveTrailingSeparator(m_strKey);

  m_hRootKey  = keyParent.m_hRootKey;
}

// dtor closes the key releasing system resource
wxRegKey::~wxRegKey()
{
  Close();
}

// ----------------------------------------------------------------------------
// change the key name/hkey
// ----------------------------------------------------------------------------

// set the full key name
void wxRegKey::SetName(const std::string& strKey)
{
  Close();

  m_strKey = strKey;
  m_hRootKey = (WXHKEY) aStdKeys[ExtractKeyName(m_strKey)].hkey;
}

// the name is relative to the parent key
void wxRegKey::SetName(StdKey keyParent, const std::string& strKey)
{
  Close();

  m_strKey = strKey;
  RemoveTrailingSeparator(m_strKey);
  m_hRootKey = (WXHKEY) aStdKeys[keyParent].hkey;
}

// the name is relative to the parent key
void wxRegKey::SetName(const wxRegKey& keyParent, const std::string& strKey)
{
  Close();

  // combine our name with parent's to get the full name

  // NB: this method is called by wxRegConfig::SetPath() which is a performance
  //     critical function and so it preallocates space for our m_strKey to
  //     gain some speed - this is why we only use += here and not = which
  //     would just free the prealloc'd buffer and would have to realloc it the
  //     next line!
  m_strKey.clear();
  m_strKey += keyParent.m_strKey;
  if ( !strKey.empty() && strKey[0] != REG_SEPARATOR )
    m_strKey += REG_SEPARATOR;
  m_strKey += strKey;

  RemoveTrailingSeparator(m_strKey);

  m_hRootKey = keyParent.m_hRootKey;
}

// hKey should be opened and will be closed in wxRegKey dtor
void wxRegKey::SetHkey(WXHKEY hKey)
{
  Close();

  m_hKey = hKey;

  // we don't know the parent of this key, assume HKLM by default
  m_hRootKey = HKEY_LOCAL_MACHINE;

  // we don't know in which mode was this key opened but we can't reopen it
  // anyhow because we don't know its name, so the only thing we can is to hope
  // that it allows all the operations which we're going to perform on it
  m_mode = Write;

  // reset old data
  m_strKey.clear();
  m_dwLastError = 0;
}

// ----------------------------------------------------------------------------
// info about the key
// ----------------------------------------------------------------------------

// returns true if the key exists
bool wxRegKey::Exists() const
{
  // opened key has to exist, try to open it if not done yet
  return IsOpened()
      ? true
      : KeyExists(m_hRootKey, m_strKey, m_viewMode);
}

// returns the full name of the key (prefix is abbreviated if bShortPrefix)
std::string wxRegKey::GetName(bool bShortPrefix) const
{
  StdKey key = GetStdKeyFromHkey((WXHKEY) m_hRootKey);
  std::string str = bShortPrefix ? aStdKeys[key].szShortName
                              : aStdKeys[key].szName;
  if ( !m_strKey.empty() )
    str += "\\" + m_strKey;

  return str;
}

bool wxRegKey::GetKeyInfo(size_t *pnSubKeys,
                          size_t *pnMaxKeyLen,
                          size_t *pnValues,
                          size_t *pnMaxValueLen) const
{
  // it might be unexpected to some that this function doesn't open the key
  wxASSERT_MSG( IsOpened(), "key should be opened in GetKeyInfo" );

  // We need to use intermediate variables in 64 bit build as the function
  // parameters must be 32 bit DWORDs and not 64 bit size_t values.
#ifdef __WIN64__
  WXDWORD dwSubKeys = 0,
        dwMaxKeyLen = 0,
        dwValues = 0,
        dwMaxValueLen = 0;

  #define REG_PARAM(name) &dw##name
#else // Win32
  #define REG_PARAM(name)   (LPDWORD)(pn##name)
#endif


  m_dwLastError = ::RegQueryInfoKeyW
                  (
                    (HKEY) m_hKey,
                    nullptr,                   // class name
                    nullptr,                   // (ptr to) size of class name buffer
                    wxRESERVED_PARAM,
                    REG_PARAM(SubKeys),     // [out] number of subkeys
                    REG_PARAM(MaxKeyLen),   // [out] max length of a subkey name
                    nullptr,                   // longest subkey class name
                    REG_PARAM(Values),      // [out] number of values
                    REG_PARAM(MaxValueLen), // [out] max length of a value name
                    nullptr,                   // longest value data
                    nullptr,                   // security descriptor
                    nullptr                    // time of last modification
                  );

#ifdef __WIN64__
  if ( pnSubKeys )
    *pnSubKeys = dwSubKeys;
  if ( pnMaxKeyLen )
    *pnMaxKeyLen = dwMaxKeyLen;
  if ( pnValues )
    *pnValues = dwValues;
  if ( pnMaxValueLen )
    *pnMaxValueLen = dwMaxValueLen;
#endif // __WIN64__

#undef REG_PARAM

  if ( m_dwLastError != ERROR_SUCCESS ) {
    wxLogSysError(m_dwLastError, _("Can't get info about registry key '%s'"),
                  GetName().c_str());
    return false;
  }

  return true;
}

// ----------------------------------------------------------------------------
// operations
// ----------------------------------------------------------------------------

// opens key (it's not an error to call Open() on an already opened key)
bool wxRegKey::Open(AccessMode mode)
{
    if ( IsOpened() )
    {
        if ( mode <= m_mode )
            return true;

        // we had been opened in read mode but now must be reopened in write
        Close();
    }

    HKEY tmpKey;
    boost::nowide::wstackstring stackStrKey{m_strKey.c_str()};
    m_dwLastError = ::RegOpenKeyExW
                    (
                        (HKEY) m_hRootKey,
                        stackStrKey.get(),
                        wxRESERVED_PARAM,
                        GetMSWAccessFlags(mode, m_viewMode),
                        &tmpKey
                    );

    if ( m_dwLastError != ERROR_SUCCESS )
    {
        wxLogSysError(m_dwLastError, _("Can't open registry key '%s'"),
                      GetName().c_str());
        return false;
    }

    m_hKey = (WXHKEY) tmpKey;
    m_mode = mode;

    return true;
}

// creates key, failing if it exists and !bOkIfExists
bool wxRegKey::Create(bool bOkIfExists)
{
  // check for existence only if asked (i.e. order is important!)
  if ( !bOkIfExists && Exists() )
    return false;

  if ( IsOpened() )
    return true;

  HKEY tmpKey;
  WXDWORD disposition;
  boost::nowide::wstackstring stackStrKey{m_strKey.c_str()};
  m_dwLastError = ::RegCreateKeyExW((HKEY) m_hRootKey, stackStrKey.get(),
      wxRESERVED_PARAM,
      nullptr, // The user-defined class type of this key.
      REG_OPTION_NON_VOLATILE, // supports other values as well; see MS docs
      GetMSWAccessFlags(wxRegKey::Write, m_viewMode),
      nullptr, // pointer to a SECURITY_ATTRIBUTES structure
      &tmpKey,
      &disposition);

  if ( m_dwLastError != ERROR_SUCCESS ) {
    wxLogSysError(m_dwLastError, _("Can't create registry key '%s'"),
                  GetName().c_str());
    return false;
  }
  else
  {
    m_hKey = (WXHKEY) tmpKey;
    return true;
  }
}

// close the key, it's not an error to call it when not opened
bool wxRegKey::Close()
{
  if ( IsOpened() ) {
    m_dwLastError = RegCloseKey((HKEY) m_hKey);
    m_hKey = nullptr;

    if ( m_dwLastError != ERROR_SUCCESS ) {
      wxLogSysError(m_dwLastError, _("Can't close registry key '%s'"),
                    GetName().c_str());

      return false;
    }
  }

  return true;
}

bool
wxRegKey::RenameValue(const std::string& szValueOld, const std::string& szValueNew)
{
    bool ok = true;
    if ( HasValue(szValueNew) ) {
        wxLogError(_("Registry value '%s' already exists."), szValueNew);

        ok = false;
    }

    if ( !ok ||
         !CopyValue(szValueOld, *this, szValueNew) ||
         !DeleteValue(szValueOld) ) {
        wxLogError(_("Failed to rename registry value '%s' to '%s'."),
                   szValueOld, szValueNew);

        return false;
    }

    return true;
}

bool wxRegKey::CopyValue(const std::string& szValue,
                         wxRegKey& keyDst,
                         const std::string& szValueNew)
{
    std::string valueNew(szValueNew);
    if ( valueNew.empty() ) {
        // by default, use the same name
        valueNew = szValue;
    }

    switch ( GetValueType(szValue) ) {
        case Type_String:
        case Type_Expand_String:
            {
                std::string strVal;
                return QueryRawValue(szValue, strVal) &&
                       keyDst.SetValue(valueNew, strVal);
            }

        case Type_Dword:
        /* case Type_Dword_little_endian: == Type_Dword */
            {
                long dwVal;
                return QueryValue(szValue, &dwVal) &&
                       keyDst.SetValue(valueNew, dwVal);
            }

        case Type_Binary:
        {
            wxMemoryBuffer buf;
            return QueryValue(szValue,buf) &&
                   keyDst.SetValue(valueNew,buf);
        }

        // these types are unsupported because I am not sure about how
        // exactly they should be copied and because they shouldn't
        // occur among the application keys (supposedly created with
        // this class)
        case Type_None:
        case Type_Dword_big_endian:
        case Type_Link:
        case Type_Multi_String:
        case Type_Resource_list:
        case Type_Full_resource_descriptor:
        case Type_Resource_requirements_list:
        default:
            wxLogError(_("Can't copy values of unsupported type %d."),
                       GetValueType(szValue));
            return false;
    }
}

bool wxRegKey::Rename(const std::string& szNewName)
{
    wxCHECK_MSG( !m_strKey.empty(), false, "registry hives can't be renamed" );

    if ( !Exists() ) {
        wxLogError(_("Registry key '%s' does not exist, cannot rename it."),
                   GetFullName(this));

        return false;
    }

    // do we stay in the same hive?
    const bool inSameHive = !wxStrchr(szNewName, REG_SEPARATOR);

    // construct the full new name of the key
    wxRegKey keyDst;

    if ( inSameHive ) {
        // rename the key to the new name under the same parent
        std::string strKey = wx::utils::BeforeLast(m_strKey, REG_SEPARATOR);
        if ( !strKey.empty() ) {
            // don't add '\\' in the start if strFullNewName is empty
            strKey += REG_SEPARATOR;
        }

        strKey += szNewName;

        keyDst.SetName(GetStdKeyFromHkey(m_hRootKey), strKey);
    }
    else {
        // this is the full name already
        keyDst.SetName(szNewName);
    }

    bool ok = keyDst.Create(false /* fail if alredy exists */);
    if ( !ok ) {
        wxLogError(_("Registry key '%s' already exists."),
                   GetFullName(&keyDst));
    }
    else {
        ok = Copy(keyDst) && DeleteSelf();
    }

    if ( !ok ) {
        wxLogError(_("Failed to rename the registry key '%s' to '%s'."),
                   GetFullName(this), GetFullName(&keyDst));
    }
    else {
        m_hRootKey = keyDst.m_hRootKey;
        m_strKey = keyDst.m_strKey;
    }

    return ok;
}

bool wxRegKey::Copy(const std::string& szNewName)
{
    // create the new key first
    wxRegKey keyDst(szNewName);
    bool ok = keyDst.Create(false /* fail if alredy exists */);
    if ( ok ) {
        ok = Copy(keyDst);

        // we created the dest key but copying to it failed - delete it
        if ( !ok ) {
            std::ignore = keyDst.DeleteSelf();
        }
    }

    return ok;
}

bool wxRegKey::Copy(wxRegKey& keyDst)
{
    bool ok = true;

    // copy all sub keys to the new location
    std::string strKey;
    long lIndex;
    bool bCont = GetFirstKey(strKey, lIndex);
    while ( ok && bCont ) {
        wxRegKey key(*this, strKey);
        std::string keyName = fmt::format("{}{}{}", GetFullName(&keyDst), REG_SEPARATOR, strKey);
        ok = key.Copy(keyName);

        if ( ok )
            bCont = GetNextKey(strKey, lIndex);
        else
            wxLogError(_("Failed to copy the registry subkey '%s' to '%s'."),
                   GetFullName(&key), keyName.c_str());

    }

    // copy all values
    std::string strVal;
    bCont = GetFirstValue(strVal, lIndex);
    while ( ok && bCont ) {
        ok = CopyValue(strVal, keyDst);

        if ( !ok ) {
            wxLogSysError(m_dwLastError,
                          _("Failed to copy registry value '%s'"),
                          strVal.c_str());
        }
        else {
            bCont = GetNextValue(strVal, lIndex);
        }
    }

    if ( !ok ) {
        wxLogError(_("Failed to copy the contents of registry key '%s' to '%s'."),
                   GetFullName(this), GetFullName(&keyDst));
    }

    return ok;
}

// ----------------------------------------------------------------------------
// delete keys/values
// ----------------------------------------------------------------------------
bool wxRegKey::DeleteSelf()
{
  {
    wxLogNull nolog;
    if ( !Open() ) {
      // it already doesn't exist - ok!
      return true;
    }
  }

  // prevent a buggy program from erasing one of the root registry keys or an
  // immediate subkey (i.e. one which doesn't have '\\' inside) of any other
  // key except HKCR (HKCR has some "deleteable" subkeys)
  if ( m_strKey.empty() ||
       ((m_hRootKey != (WXHKEY) aStdKeys[HKCR].hkey) &&
        (m_strKey.find(REG_SEPARATOR) == std::string::npos)) ) {
      wxLogError(_("Registry key '%s' is needed for normal system operation,\ndeleting it will leave your system in unusable state:\noperation aborted."),
                 GetFullName(this));

      return false;
  }

  // we can't delete keys while enumerating because it confuses GetNextKey, so
  // we first save the key names and then delete them all
  std::vector<std::string> astrSubkeys;

  std::string strKey;
  long lIndex;
  bool bCont = GetFirstKey(strKey, lIndex);
  while ( bCont ) {
    astrSubkeys.push_back(strKey);

    bCont = GetNextKey(strKey, lIndex);
  }

  size_t nKeyCount = astrSubkeys.size();
  for ( size_t nKey = 0; nKey < nKeyCount; nKey++ ) {
    wxRegKey key(*this, astrSubkeys[nKey]);
    if ( !key.DeleteSelf() )
      return false;
  }

  // now delete this key itself
  Close();

  // deleting a key which doesn't exist is not considered an error
#if wxUSE_DYNLIB_CLASS
  wxDynamicLibrary dllAdvapi32("advapi32");
  // Minimum supported OS for RegDeleteKeyEx: Vista, XP Pro x64, Win Server 2008, Win Server 2003 SP1
  using RegDeleteKeyEx_t = LONG (WINAPI*)(HKEY, LPCWSTR, REGSAM, WXDWORD);
  RegDeleteKeyEx_t wxDL_INIT_FUNC_AW(pfn, RegDeleteKeyEx, dllAdvapi32);
  if (pfnRegDeleteKeyEx)
  {
    boost::nowide::wstackstring stackStrKey{m_strKey.c_str()};
    m_dwLastError = (*pfnRegDeleteKeyEx)((HKEY) m_hRootKey, stackStrKey.get(),
        GetMSWViewFlags(m_viewMode),
        wxRESERVED_PARAM);
  }
  else
#endif // wxUSE_DYNLIB_CLASS
  {
    boost::nowide::wstackstring stackStrKey{m_strKey.c_str()};
    m_dwLastError = ::RegDeleteKeyW((HKEY) m_hRootKey, stackStrKey.get());
  }

  if ( m_dwLastError != ERROR_SUCCESS &&
          m_dwLastError != ERROR_FILE_NOT_FOUND ) {
    wxLogSysError(m_dwLastError, _("Can't delete key '%s'"),
                  GetName().c_str());
    return false;
  }

  return true;
}

bool wxRegKey::DeleteKey(const std::string& szKey)
{
  if ( !Open() )
    return false;

  wxRegKey key(*this, szKey);
  return key.DeleteSelf();
}

bool wxRegKey::DeleteValue(const std::string& szValue)
{
    if ( !Open() )
        return false;

    boost::nowide::wstackstring stackSzValue{szValue.c_str()};
    m_dwLastError = ::RegDeleteValueW((HKEY) m_hKey, stackSzValue.get());

    // deleting a value which doesn't exist is not considered an error
    if ( (m_dwLastError != ERROR_SUCCESS) &&
         (m_dwLastError != ERROR_FILE_NOT_FOUND) )
    {
        wxLogSysError(m_dwLastError, _("Can't delete value '%s' from key '%s'"),
                      szValue, GetName().c_str());
        return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
// access to values and subkeys
// ----------------------------------------------------------------------------

// return true if value exists
bool wxRegKey::HasValue(const std::string& szValue) const
{
    // this function should be silent, so suppress possible messages from Open()
    wxLogNull nolog;

    if ( !CONST_CAST Open(Read) )
        return false;

    boost::nowide::wstackstring stackSzValue{szValue.c_str()};
    const LONG dwRet = ::RegQueryValueExW((HKEY) m_hKey,
                                   stackSzValue.get(),
                                   wxRESERVED_PARAM,
                                   nullptr, nullptr, nullptr);
    return dwRet == ERROR_SUCCESS;
}

// returns true if this key has any values
bool wxRegKey::HasValues() const
{
  // suppress possible messages from GetFirstValue()
  wxLogNull nolog;

  // just call GetFirstValue with dummy parameters
  std::string str;
  long     l;
  return CONST_CAST GetFirstValue(str, l);
}

// returns true if this key has any subkeys
bool wxRegKey::HasSubkeys() const
{
  // suppress possible messages from GetFirstKey()
  wxLogNull nolog;

  // just call GetFirstKey with dummy parameters
  std::string str;
  long     l;
  return CONST_CAST GetFirstKey(str, l);
}

// returns true if given subkey exists
bool wxRegKey::HasSubKey(const std::string& szKey) const
{
  // this function should be silent, so suppress possible messages from Open()
  wxLogNull nolog;

  if ( !CONST_CAST Open(Read) )
    return false;

  return KeyExists(m_hKey, szKey, m_viewMode);
}

wxRegKey::ValueType wxRegKey::GetValueType(const std::string& szValue) const
{
    if ( ! CONST_CAST Open(Read) )
      return Type_None;

    WXDWORD dwType;
    boost::nowide::wstackstring stackSzValue{szValue.c_str()};
    m_dwLastError = ::RegQueryValueExW((HKEY) m_hKey, stackSzValue.get(), wxRESERVED_PARAM,
                                    &dwType, nullptr, nullptr);
    if ( m_dwLastError != ERROR_SUCCESS ) {
      wxLogSysError(m_dwLastError, _("Can't read value of key '%s'"),
                    GetName().c_str());
      return Type_None;
    }

    return (ValueType)dwType;
}

bool wxRegKey::SetValue(const std::string& szValue, long lValue)
{
  if ( CONST_CAST Open() ) {
    boost::nowide::wstackstring stackSzValue{szValue.c_str()};
    m_dwLastError = ::RegSetValueExW((HKEY) m_hKey, stackSzValue.get(),
                                  wxRESERVED_PARAM, REG_DWORD,
                                  (RegString)&lValue, sizeof(lValue));
    if ( m_dwLastError == ERROR_SUCCESS )
      return true;
  }

  wxLogSysError(m_dwLastError, _("Can't set value of '%s'"),
                GetFullName(this, szValue));
  return false;
}

bool wxRegKey::QueryValue(const std::string& szValue, long *plValue) const
{
  if ( CONST_CAST Open(Read) ) {
    WXDWORD dwType, dwSize = sizeof(WXDWORD);
    RegString pBuf = (RegString)plValue;
    boost::nowide::wstackstring stackSzValue{szValue.c_str()};
    m_dwLastError = ::RegQueryValueExW((HKEY) m_hKey, stackSzValue.get(),
                                    wxRESERVED_PARAM,
                                    &dwType, pBuf, &dwSize);
    if ( m_dwLastError != ERROR_SUCCESS ) {
      wxLogSysError(m_dwLastError, _("Can't read value of key '%s'"),
                    GetName().c_str());
      return false;
    }

    // check that we read the value of right type
    if ( dwType != REG_DWORD_LITTLE_ENDIAN && dwType != REG_DWORD_BIG_ENDIAN ) {
      wxLogError(_("Registry value \"%s\" is not numeric (but of type %s)"),
                 GetFullName(this, szValue), GetTypeString(dwType));
      return false;
    }

    return true;
  }
  else
    return false;
}

bool wxRegKey::SetValue64(const std::string& szValue, std::int64_t llValue)
{
  if ( CONST_CAST Open() ) {
    boost::nowide::wstackstring stackSzValue{szValue.c_str()};
    m_dwLastError = ::RegSetValueExW((HKEY) m_hKey, stackSzValue.get(),
                                  wxRESERVED_PARAM, REG_QWORD,
                                  (RegString)&llValue, sizeof(llValue));
    if ( m_dwLastError == ERROR_SUCCESS )
      return true;
  }

  wxLogSysError(m_dwLastError, _("Can't set value of '%s'"),
                GetFullName(this, szValue));
  return false;
}

bool wxRegKey::QueryValue64(const std::string& szValue, std::int64_t *pllValue) const
{
  if ( CONST_CAST Open(Read) ) {
    WXDWORD dwType, dwSize = sizeof(std::int64_t); // QWORD doesn't exist.
    RegString pBuf = (RegString)pllValue;
    boost::nowide::wstackstring stackSzValue{szValue.c_str()};
    m_dwLastError = ::RegQueryValueExW((HKEY) m_hKey, stackSzValue.get(),
                                    wxRESERVED_PARAM,
                                    &dwType, pBuf, &dwSize);
    if ( m_dwLastError != ERROR_SUCCESS ) {
      wxLogSysError(m_dwLastError, _("Can't read value of key '%s'"),
                    GetName().c_str());
      return false;
    }

    // check that we read the value of right type
    switch ( dwType )
    {
      case REG_DWORD_LITTLE_ENDIAN:
      case REG_DWORD_BIG_ENDIAN:
      case REG_QWORD:
        break;

      default:
        wxLogError(_("Registry value \"%s\" is not numeric (but of type %s)"),
          GetFullName(this, szValue), GetTypeString(dwType));
        return false;
    }

    return true;
  }
  else
    return false;
}
bool wxRegKey::SetValue(const std::string& szValue, const wxMemoryBuffer& buffer)
{
  if ( CONST_CAST Open() ) {
    boost::nowide::wstackstring stackSzValue{szValue.c_str()};
    m_dwLastError = ::RegSetValueExW((HKEY) m_hKey, stackSzValue.get(),
                                  wxRESERVED_PARAM, REG_BINARY,
                                  (RegBinary)buffer.GetData(),buffer.GetDataLen());
    if ( m_dwLastError == ERROR_SUCCESS )
      return true;
  }

  wxLogSysError(m_dwLastError, _("Can't set value of '%s'"),
                GetFullName(this, szValue));
  return false;
}

bool wxRegKey::QueryValue(const std::string& szValue, wxMemoryBuffer& buffer) const
{
  if ( CONST_CAST Open(Read) ) {
    // first get the type and size of the data
    WXDWORD dwType, dwSize;
    boost::nowide::wstackstring stackRegValue{szValue.c_str()};
    m_dwLastError = ::RegQueryValueExW((HKEY) m_hKey, stackRegValue.get(),
                                    wxRESERVED_PARAM,
                                    &dwType, nullptr, &dwSize);

    if ( m_dwLastError == ERROR_SUCCESS ) {
        if ( dwType != REG_BINARY ) {
          wxLogError(_("Registry value \"%s\" is not binary (but of type %s)"),
                     GetFullName(this, szValue), GetTypeString(dwType));
          return false;
        }

        if ( dwSize ) {
            const RegBinary pBuf = (RegBinary)buffer.GetWriteBuf(dwSize);
            m_dwLastError = ::RegQueryValueExW((HKEY) m_hKey,
                                            stackRegValue.get(),
                                            wxRESERVED_PARAM,
                                            &dwType,
                                            pBuf,
                                            &dwSize);
            buffer.UngetWriteBuf(dwSize);
        } else {
            buffer.SetDataLen(0);
        }
    }


    if ( m_dwLastError != ERROR_SUCCESS ) {
      wxLogSysError(m_dwLastError, _("Can't read value of key '%s'"),
                    GetName().c_str());
      return false;
    }
    return true;
  }
  return false;
}



bool wxRegKey::QueryValue(const std::string& szValue,
                          std::string& strValue,
                          bool raw) const
{
    if ( CONST_CAST Open(Read) )
    {

        // first get the type and size of the data
        WXDWORD dwType=REG_NONE, dwSize=0;
        boost::nowide::wstackstring stackSzValue{szValue.c_str()};
        m_dwLastError = ::RegQueryValueExW((HKEY) m_hKey,
                                        stackSzValue.get(),
                                        wxRESERVED_PARAM,
                                        &dwType, nullptr, &dwSize);
        if ( m_dwLastError == ERROR_SUCCESS )
        {
            if ( dwType != REG_SZ && dwType != REG_EXPAND_SZ )
            {
                wxLogError(_("Registry value \"%s\" is not text (but of type %s)"),
                             GetFullName(this, szValue), GetTypeString(dwType));
                return false;
            }

            // We need length in characters, not bytes.
            WXDWORD chars = dwSize / sizeof(wxChar);
            if ( !chars )
            {
                // must treat this case specially as GetWriteBuf() doesn't like
                // being called with 0 size
                strValue.clear();
            }
            else
            {
                // extra scope for wxStringBufferLength
                {
                    // FIXME: Okay to reuse stackSzValue buffer?
                    // boost::nowide::wstackstring stackSzValue{szValue.c_str()};
                    boost::nowide::wstackstring stackStrBuf;

                    m_dwLastError = ::RegQueryValueExW((HKEY) m_hKey,
                                                    stackSzValue.get(),
                                                    wxRESERVED_PARAM,
                                                    &dwType,
                                                    reinterpret_cast<LPBYTE>(stackStrBuf.get()),
                                                    &dwSize);

                    strValue = boost::nowide::narrow(stackStrBuf.get());
                    // The returned string may or not be NUL-terminated,
                    // exclude the trailing NUL if it's there (which is
                    // typically the case but is not guaranteed to always be).
                    // if ( strBuf[chars - 1] == '\0' )
                    //     chars--;

                    // strBuf.SetLength(chars);
                }

                // expand the var expansions in the string unless disabled
                if ( (dwType == REG_EXPAND_SZ) && !raw )
                {   // FIXME: Okay to reuse stack buffer?
                    boost::nowide::wstackstring stackStrValue{strValue.c_str()};
                    const WXDWORD dwExpSize = ::ExpandEnvironmentStringsW(stackStrValue.get(), nullptr, 0);
                    bool ok = dwExpSize != 0;
                    if ( ok )
                    {
                        std::string strExpValue;
                        strExpValue.resize(dwExpSize);
                        boost::nowide::wstackstring stackExpValue{strExpValue.c_str()};
                        ok = ::ExpandEnvironmentStringsW(stackStrValue.get(),
                                                         stackExpValue.get(),
                                                        dwExpSize
                                                        ) != 0;
                        strValue = boost::nowide::narrow(stackExpValue.get());
                    }

                    if ( !ok )
                    {
                        wxLogLastError("ExpandEnvironmentStrings");
                    }
                }
            }

            if ( m_dwLastError == ERROR_SUCCESS )
              return true;
        }
    }

    wxLogSysError(m_dwLastError, _("Can't read value of '%s'"),
                  GetFullName(this, szValue));
    return false;
}

bool wxRegKey::SetValue(const std::string& szValue, const std::string& strValue)
{
  if ( CONST_CAST Open() ) {
      boost::nowide::wstackstring stackStrValue{strValue.c_str()};
      boost::nowide::wstackstring stackSzValue{szValue.c_str()};
      m_dwLastError = ::RegSetValueExW((HKEY) m_hKey,
                                    stackSzValue.get(),
                                    wxRESERVED_PARAM, REG_SZ,
                                    reinterpret_cast<const BYTE*>(stackStrValue.get()),
                                    (stackStrValue.buffer_size + 1) * sizeof(wchar_t));
      if ( m_dwLastError == ERROR_SUCCESS )
        return true;
  }

  wxLogSysError(m_dwLastError, _("Can't set value of '%s'"),
                GetFullName(this, szValue));
  return false;
}

std::string wxRegKey::QueryDefaultValue() const
{
  std::string str;
  QueryValue({}, str, false);
  return str;
}

// ----------------------------------------------------------------------------
// enumeration
// NB: all these functions require an index variable which allows to have
//     several concurrently running indexations on the same key
// ----------------------------------------------------------------------------

bool wxRegKey::GetFirstValue(std::string& strValueName, long& lIndex)
{
  if ( !Open(Read) )
    return false;

  lIndex = 0;
  return GetNextValue(strValueName, lIndex);
}

bool wxRegKey::GetNextValue(std::string& strValueName, long& lIndex) const
{
    wxASSERT( IsOpened() );

    // are we already at the end of enumeration?
    if ( lIndex == -1 )
        return false;

    wxChar  szValueName[1024];                  // @@ use RegQueryInfoKey...
    WXDWORD dwValueLen = WXSIZEOF(szValueName);

    m_dwLastError = ::RegEnumValueW((HKEY) m_hKey, lIndex++,
                                 szValueName, &dwValueLen,
                                 wxRESERVED_PARAM,
                                 nullptr,            // [out] type
                                 nullptr,            // [out] buffer for value
                                 nullptr);           // [i/o]  it's length

    if ( m_dwLastError != ERROR_SUCCESS ) {
      if ( m_dwLastError == ERROR_NO_MORE_ITEMS ) {
        m_dwLastError = ERROR_SUCCESS;
        lIndex = -1;
      }
      else {
        wxLogSysError(m_dwLastError, _("Can't enumerate values of key '%s'"),
                      GetName().c_str());
      }

      return false;
    }

    strValueName = boost::nowide::narrow(szValueName);

  return true;
}

bool wxRegKey::GetFirstKey(std::string& strKeyName, long& lIndex)
{
  if ( !Open(Read) )
    return false;

  lIndex = 0;
  return GetNextKey(strKeyName, lIndex);
}

bool wxRegKey::GetNextKey(std::string& strKeyName, long& lIndex) const
{
  wxASSERT( IsOpened() );

  // are we already at the end of enumeration?
  if ( lIndex == -1 )
    return false;

  wxChar szKeyName[_MAX_PATH + 1];

  m_dwLastError = ::RegEnumKeyW((HKEY) m_hKey, lIndex++, szKeyName, WXSIZEOF(szKeyName));

  if ( m_dwLastError != ERROR_SUCCESS ) {
    if ( m_dwLastError == ERROR_NO_MORE_ITEMS ) {
      m_dwLastError = ERROR_SUCCESS;
      lIndex = -1;
    }
    else {
      wxLogSysError(m_dwLastError, _("Can't enumerate subkeys of key '%s'"),
                    GetName().c_str());
    }

    return false;
  }

  strKeyName = boost::nowide::narrow(szKeyName);
  return true;
}

// returns true if the value contains a number (else it's some string)
bool wxRegKey::IsNumericValue(const std::string& szValue) const
{
    ValueType type = GetValueType(szValue);
    switch ( type ) {
        case Type_Dword:
        /* case Type_Dword_little_endian: == Type_Dword */
        case Type_Dword_big_endian:
        case Type_Qword:
            return true;

        default:
            return false;
    }
}

// ----------------------------------------------------------------------------
// exporting registry keys to file
// ----------------------------------------------------------------------------

#if wxUSE_STREAMS

// helper functions for writing ASCII strings (even in Unicode build)
static inline bool WriteAsciiChar(wxOutputStream& ostr, char ch)
{
    ostr.PutC(ch);
    return ostr.IsOk();
}

static inline bool WriteAsciiEOL(wxOutputStream& ostr)
{
    // as we open the file in text mode, it is enough to write LF without CR
    return WriteAsciiChar(ostr, '\n');
}

static inline bool WriteAsciiString(wxOutputStream& ostr, const char *p)
{
    return ostr.Write(p, strlen(p)).IsOk();
}

static inline bool WriteAsciiString(wxOutputStream& ostr, const std::string& s)
{
    wxCharBuffer name(s.c_str());
    ostr.Write(name, strlen(name));

    return ostr.IsOk();
}

#endif // wxUSE_STREAMS

bool wxRegKey::Export(const std::string& filename) const
{
#if wxUSE_FILE && wxUSE_FFILE && wxUSE_STREAMS
    if ( wxFile::Exists(filename) )
    {
        wxLogError(_("Exporting registry key: file \"%s\" already exists and won't be overwritten."),
                   filename.c_str());
        return false;
    }

    wxFFileOutputStream ostr(filename, "w");

    return ostr.IsOk() && Export(ostr);
#else
    wxUnusedVar(filename);
    return false;
#endif
}

#if wxUSE_STREAMS
bool wxRegKey::Export(wxOutputStream& ostr) const
{
    // write out the header
    if ( !WriteAsciiString(ostr, "REGEDIT4\n\n") )
        return false;

    return DoExport(ostr);
}
#endif // wxUSE_STREAMS

static
std::string
FormatAsHex(const void *data,
            size_t size,
            wxRegKey::ValueType type = wxRegKey::Type_Binary)
{
    std::string value("hex");

    // binary values use just "hex:" prefix while the other ones must indicate
    // the real type
    if ( type != wxRegKey::Type_Binary )
        value += fmt::format("({})", type);
    value += ':';

    // write all the rest as comma-separated bytes
    value.reserve(3*size + 10);
    const char * const p = static_cast<const char *>(data);
    for ( size_t n = 0; n < size; n++ )
    {
        // TODO: line wrapping: although not required by regedit, this makes
        //       the generated files easier to read and compare with the files
        //       produced by regedit
        if ( n )
            value += ',';

        value += fmt::format("{:02x}", (unsigned char)p[n]);
    }

    return value;
}

static inline
std::string FormatAsHex(const std::string& value, wxRegKey::ValueType type)
{
    return FormatAsHex(value.c_str(), value.length() + 1, type);
}

std::string wxRegKey::FormatValue(const std::string& name) const
{
    std::string rhs;
    const ValueType type = GetValueType(name);
    switch ( type )
    {
        case Type_String:
        case Type_Expand_String:
            {
                std::string value;
                if ( !QueryRawValue(name, value) )
                    break;

                // quotes and backslashes must be quoted, linefeeds are not
                // allowed in string values
                rhs.reserve(value.length() + 2);
                rhs = wxT('"');

                // there can be no NULs here
                bool useHex = false;
                for ( std::string::const_iterator p = value.begin();
                      p != value.end() && !useHex; ++p )
                {
                    switch ( *p )
                    {
                        case wxT('\n'):
                            // we can only represent this string in hex
                            useHex = true;
                            break;

                        case wxT('"'):
                        case wxT('\\'):
                            // escape special symbol
                            rhs += wxT('\\');
                            [[fallthrough]];

                        default:
                            rhs += *p;
                    }
                }

                if ( useHex )
                    rhs = FormatAsHex(value, Type_String);
                else
                    rhs += wxT('"');
            }
            break;

        case Type_Dword:
        /* case Type_Dword_little_endian: == Type_Dword */
            {
                long value;
                if ( !QueryValue(name, &value) )
                    break;

                rhs = fmt::format("dword:%08x", (unsigned int)value);
            }
            break;

        case Type_Multi_String:
            {
                std::string value;
                if ( !QueryRawValue(name, value) )
                    break;

                rhs = FormatAsHex(value, type);
            }
            break;

        case Type_Binary:
            {
                wxMemoryBuffer buf;
                if ( !QueryValue(name, buf) )
                    break;

                rhs = FormatAsHex(buf.GetData(), buf.GetDataLen());
            }
            break;

        // no idea how those appear in REGEDIT4 files
        case Type_None:
        case Type_Dword_big_endian:
        case Type_Link:
        case Type_Resource_list:
        case Type_Full_resource_descriptor:
        case Type_Resource_requirements_list:
        default:
            wxLogWarning(_("Can't export value of unsupported type %d."), type);
    }

    return rhs;
}

#if wxUSE_STREAMS

bool wxRegKey::DoExportValue(wxOutputStream& ostr, const std::string& name) const
{
    // first examine the value type: if it's unsupported, simply skip it
    // instead of aborting the entire export process because we failed to
    // export a single value
    std::string value = FormatValue(name);
    if ( value.empty() )
    {
        wxLogWarning(_("Ignoring value \"%s\" of the key \"%s\"."),
                     name.c_str(), GetName().c_str());
        return true;
    }

    // we do have the text representation of the value, now write everything
    // out

    // special case: unnamed/default value is represented as just "@"
    if ( name.empty() )
    {
        if ( !WriteAsciiChar(ostr, '@') )
            return false;
    }
    else // normal, named, value
    {
        if ( !WriteAsciiChar(ostr, '"') ||
                !WriteAsciiString(ostr, name) ||
                    !WriteAsciiChar(ostr, '"') )
            return false;
    }

    if ( !WriteAsciiChar(ostr, '=') )
        return false;

    return WriteAsciiString(ostr, value) && WriteAsciiEOL(ostr);
}

bool wxRegKey::DoExport(wxOutputStream& ostr) const
{
    // write out this key name
    if ( !WriteAsciiChar(ostr, '[') )
        return false;

    if ( !WriteAsciiString(ostr, GetName(false /* no short prefix */)) )
        return false;

    if ( !WriteAsciiChar(ostr, ']') || !WriteAsciiEOL(ostr) )
        return false;

    // dump all our values
    long dummy;
    std::string name;
    wxRegKey& self = const_cast<wxRegKey&>(*this);
    bool cont = self.GetFirstValue(name, dummy);
    while ( cont )
    {
        if ( !DoExportValue(ostr, name) )
            return false;

        cont = GetNextValue(name, dummy);
    }

    // always terminate values by blank line, even if there were no values
    if ( !WriteAsciiEOL(ostr) )
        return false;

    // recurse to subkeys
    cont = self.GetFirstKey(name, dummy);
    while ( cont )
    {
        wxRegKey subkey(*this, name);
        if ( !subkey.DoExport(ostr) )
            return false;

        cont = GetNextKey(name, dummy);
    }

    return true;
}

#endif // wxUSE_STREAMS

// ============================================================================
// implementation of global private functions
// ============================================================================

bool KeyExists(WXHKEY hRootKey,
               const std::string& szKey,
               wxRegKey::WOW64ViewMode viewMode)
{
    // don't close this key itself for the case of empty szKey!
    if ( szKey.empty() )
        return true;

    HKEY hkeyDummy;

    boost::nowide::wstackstring stackSzKey{szKey.c_str()};
    if ( ::RegOpenKeyExW
         (
            (HKEY)hRootKey,
            stackSzKey.get(),
            wxRESERVED_PARAM,
            // we might not have enough rights for rw access
            GetMSWAccessFlags(wxRegKey::Read, viewMode),
            &hkeyDummy
         ) == ERROR_SUCCESS )
    {
        ::RegCloseKey(hkeyDummy);

        return true;
    }

    return false;
}

long GetMSWViewFlags(wxRegKey::WOW64ViewMode viewMode)
{
    long samWOW64ViewMode = 0;

    switch ( viewMode )
    {
        case wxRegKey::WOW64ViewMode_32:
#ifdef __WIN64__    // the flag is only needed by 64 bit apps
            samWOW64ViewMode = KEY_WOW64_32KEY;
#endif // Win64
            break;

        case wxRegKey::WOW64ViewMode_64:
#ifndef __WIN64__   // the flag is only needed by 32 bit apps
            // 64 bit registry can only be accessed under 64 bit platforms
            if ( wxIsPlatform64Bit() )
                samWOW64ViewMode = KEY_WOW64_64KEY;
#endif // Win32
            break;

        default:
            wxFAIL_MSG("Unknown registry view.");
            [[fallthrough]];

        case wxRegKey::WOW64ViewMode_Default:
            // Use default registry view for the current application,
            // i.e. 32 bits for 32 bit ones and 64 bits for 64 bit apps
            break;
    }

    return samWOW64ViewMode;
}

long GetMSWAccessFlags(wxRegKey::AccessMode mode,
    wxRegKey::WOW64ViewMode viewMode)
{
    long sam = mode == wxRegKey::Read ? KEY_READ : KEY_ALL_ACCESS;

    sam |= GetMSWViewFlags(viewMode);

    return sam;
}

std::string GetFullName(const wxRegKey *pKey, const std::string& szValue)
{
  std::string str(pKey->GetName());
  if ( !szValue.empty() )
    str += "\\" + szValue;

  return str;
}

std::string GetFullName(const wxRegKey *pKey)
{
  return pKey->GetName();
}

inline void RemoveTrailingSeparator(std::string& str)
{
  if ( !str.empty() && str.back() == REG_SEPARATOR )
    str.pop_back();
}

inline const char* RegValueStr(const std::string& szValue)
{
    return szValue.empty() ? (const char*)nullptr : szValue.c_str();
}

#endif // wxUSE_REGKEY
