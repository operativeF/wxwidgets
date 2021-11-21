///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/registry.h
// Purpose:     Registry classes and functions
// Author:      Vadim Zeitlin
// Modified by:
// Created:     03.04.1998
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_REGISTRY_H_
#define _WX_MSW_REGISTRY_H_

#if wxUSE_REGKEY

#include "wx/defs.h"

class wxOutputStream;

// ----------------------------------------------------------------------------
// class wxRegKey encapsulates window HKEY handle
// ----------------------------------------------------------------------------

class wxRegKey
{
public:
  // NB: do _not_ change the values of elements in these enumerations!

  // registry value types (with comments from winnt.h)
  enum ValueType
  {
    Type_None,                       // No value type
    Type_String,                     // Unicode nul terminated string
    Type_Expand_String,              // Unicode nul terminated string
                                     // (with environment variable references)
    Type_Binary,                     // Free form binary
    Type_Dword,                      // 32-bit number
    Type_Dword_little_endian         // 32-bit number
        = Type_Dword,                // (same as Type_DWORD)
    Type_Dword_big_endian,           // 32-bit number
    Type_Link,                       // Symbolic Link (unicode)
    Type_Multi_String,               // Multiple Unicode strings
    Type_Resource_list,              // Resource list in the resource map
    Type_Full_resource_descriptor,   // Resource list in the hardware description
    Type_Resource_requirements_list, // ???
    Type_Qword                       // 64-bit number
  };

  // predefined registry keys
  enum StdKey
  {
    HKCR,       // classes root
    HKCU,       // current user
    HKLM,       // local machine
    HKUSR,      // users
    HKCC,       // current config
    HKMAX
  };

  // access mode for the key
  enum AccessMode
  {
      Read,     // read-only
      Write     // read and write
  };

  // Different registry views supported under WOW64.
  enum WOW64ViewMode
  {
      // 32 bit registry for 32 bit applications, 64 bit registry
      // for 64 bit ones.
      WOW64ViewMode_Default,

      // Can be used in 64 bit apps to access 32 bit registry,
      // has no effect (i.e. treated as default) in 32 bit apps.
      WOW64ViewMode_32,

      // Can be used in 32 bit apps to access 64 bit registry,
      // has no effect (i.e. treated as default) in 64 bit apps.
      WOW64ViewMode_64
  };

  // information about standard (predefined) registry keys
    // number of standard keys
  static const size_t nStdKeys;
    // get the name of a standard key
  static const char* GetStdKeyName(size_t key);
    // get the short name of a standard key
  static const char* GetStdKeyShortName(size_t key);
    // get StdKey from root HKEY
  static StdKey GetStdKeyFromHkey(WXHKEY hkey);

  // extracts the std key prefix from the string (return value) and
  // leaves only the part after it (i.e. modifies the string passed!)
  static StdKey ExtractKeyName(std::string& str);

    // root key is set to HKCR (the only root key under Win16)
    wxRegKey(WOW64ViewMode viewMode = WOW64ViewMode_Default);

    // strKey is the full name of the key (i.e. starting with HKEY_xxx...)
    wxRegKey(const std::string& strKey,
        WOW64ViewMode viewMode = WOW64ViewMode_Default);

    // strKey is the name of key under (standard key) keyParent
    wxRegKey(StdKey keyParent,
        const std::string& strKey,
        WOW64ViewMode viewMode = WOW64ViewMode_Default);

    // strKey is the name of key under (previously created) keyParent
  wxRegKey(const wxRegKey& keyParent, const std::string& strKey);
    // dtor closes the key
  ~wxRegKey();

  wxRegKey& operator=(wxRegKey&&) = delete;

  // change key (closes the previously opened key if any)
    // the name is absolute, i.e. should start with HKEY_xxx
  void  SetName(const std::string& strKey);
    // the name is relative to the parent key
  void  SetName(StdKey keyParent, const std::string& strKey);
    // the name is relative to the parent key
  void  SetName(const wxRegKey& keyParent, const std::string& strKey);
    // hKey should be opened and will be closed in wxRegKey dtor
  void  SetHkey(WXHKEY hKey);

  // get information about the key
    // get the (full) key name. Abbreviate std root keys if bShortPrefix.
  std::string GetName(bool bShortPrefix = true) const;
    // Retrieves the registry view used by this key.
    WOW64ViewMode GetView() const { return m_viewMode; }
    // return true if the key exists
  bool  Exists() const;
    // get the info about key (any number of these pointers may be NULL)
  bool  GetKeyInfo(size_t *pnSubKeys,      // number of subkeys
                   size_t *pnMaxKeyLen,    // max length of subkey name
                   size_t *pnValues,       // number of values
                   size_t *pnMaxValueLen) const;
    // return true if the key is opened
  bool  IsOpened() const { return m_hKey != nullptr; }
    // for "if ( !key ) wxLogError(...)" kind of expressions
  operator bool()  const { return m_dwLastError == 0; }

  // operations on the key itself
    // explicitly open the key (will be automatically done by all functions
    // which need the key to be opened if the key is not opened yet)
  bool  Open(AccessMode mode = Write);
    // create the key: will fail if the key already exists and !bOkIfExists
  bool  Create(bool bOkIfExists = true);
    // rename a value from old name to new one
  bool  RenameValue(const std::string& szValueOld, const std::string& szValueNew);
    // rename the key
  bool  Rename(const std::string& szNewName);
    // copy value to another key possibly changing its name (by default it will
    // remain the same)
  bool  CopyValue(const std::string& szValue, wxRegKey& keyDst,
                  const std::string& szNewName = {});

    // copy the entire contents of the key recursively to another location
  bool  Copy(const std::string& szNewName);
    // same as Copy() but using a key and not the name
  bool  Copy(wxRegKey& keyDst);
    // close the key (will be automatically done in dtor)
  bool  Close();

  // deleting keys/values
    // deletes this key and all of it's subkeys/values
  bool  DeleteSelf();
    // deletes the subkey with all of it's subkeys/values recursively
  bool  DeleteKey(const std::string& szKey);
    // deletes the named value (may be empty string to remove the default value)
  bool DeleteValue(const std::string& szValue);

  // access to values and subkeys
    // get value type
  ValueType GetValueType(const std::string& szValue) const;
    // returns true if the value contains a number (else it's some string)
  bool IsNumericValue(const std::string& szValue) const;

    // assignment operators set the default value of the key
  wxRegKey& operator=(const std::string& strValue)
    { SetValue({}, strValue); return *this; }

    // query the default value of the key: implicitly or explicitly
  std::string QueryDefaultValue() const;
  operator std::string() const { return QueryDefaultValue(); }

    // named values

    // set the string value
  bool  SetValue(const std::string& szValue, const std::string& strValue);
    // retrieve the string value
  bool  QueryValue(const std::string& szValue, std::string& strValue) const
    { return QueryValue(szValue, strValue, false); }
    // retrieve raw string value
  bool  QueryRawValue(const std::string& szValue, std::string& strValue) const
    { return QueryValue(szValue, strValue, true); }
    // retrieve either raw or expanded string value
  bool  QueryValue(const std::string& szValue, std::string& strValue, bool raw) const;

    // set the 32-bit numeric value
  bool  SetValue(const std::string& szValue, long lValue);
    // return the 32-bit numeric value
  bool  QueryValue(const std::string& szValue, long *plValue) const;
    // set the 64-bit numeric value
  bool  SetValue64(const std::string& szValue, wxLongLong_t llValue);
    // return the 64-bit numeric value
  bool  QueryValue64(const std::string& szValue, wxLongLong_t *pllValue) const;
    // set the binary value
  bool  SetValue(const std::string& szValue, const wxMemoryBuffer& buf);
    // return the binary value
  bool  QueryValue(const std::string& szValue, wxMemoryBuffer& buf) const;

  // query existence of a key/value
    // return true if value exists
  bool HasValue(const std::string& szKey) const;
    // return true if given subkey exists
  bool HasSubKey(const std::string& szKey) const;
    // return true if any subkeys exist
  bool HasSubkeys() const;
    // return true if any values exist
  bool HasValues() const;
    // return true if the key is empty (nothing under this key)
  bool IsEmpty() const { return !HasSubkeys() && !HasValues(); }

  // enumerate values and subkeys
  bool  GetFirstValue(std::string& strValueName, long& lIndex);
  bool  GetNextValue (std::string& strValueName, long& lIndex) const;

  bool  GetFirstKey  (std::string& strKeyName  , long& lIndex);
  bool  GetNextKey   (std::string& strKeyName  , long& lIndex) const;

  // export the contents of this key and all its subkeys to the given file
  // (which won't be overwritten, it's an error if it already exists)
  //
  // note that we export the key in REGEDIT4 format, not RegSaveKey() binary
  // format nor newer REGEDIT5 one
  bool Export(const std::string& filename) const;

  // same as above but write to the given (opened) stream
  bool Export(wxOutputStream& ostr) const;


  // for wxRegConfig usage only: preallocate some memory for the name
  void ReserveMemoryForName(size_t bytes) { m_strKey.reserve(bytes); }

private:
  // recursive helper for Export()
  bool DoExport(wxOutputStream& ostr) const;

  // export a single value
  bool DoExportValue(wxOutputStream& ostr, const std::string& name) const;

  // return the text representation (in REGEDIT4 format) of the value with the
  // given name
  std::string FormatValue(const std::string& name) const;


  WXHKEY        m_hKey{nullptr};    // our handle
  WXHKEY        m_hRootKey;         // handle of the top key (i.e. StdKey)
  std::string      m_strKey;           // key name (relative to m_hRootKey)
  WOW64ViewMode m_viewMode;         // which view to select under WOW64
  AccessMode    m_mode;             // FIXME: Default value? valid only if key is opened
  mutable long  m_dwLastError{0};   // last error (0 if none)
};

#endif // wxUSE_REGKEY

#endif // _WX_MSW_REGISTRY_H_

