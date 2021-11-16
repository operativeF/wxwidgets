///////////////////////////////////////////////////////////////////////////////
// Name:        wx/confbase.h
// Purpose:     declaration of the base class of all config implementations
//              (see also: fileconf.h and msw/regconf.h and iniconf.h)
// Author:      Karsten Ballueder & Vadim Zeitlin
// Modified by:
// Created:     07.04.98 (adapted from appconf.h)
// Copyright:   (c) 1997 Karsten Ballueder   Ballueder@usa.net
//                       Vadim Zeitlin      <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CONFBASE_H_
#define _WX_CONFBASE_H_

#include "wx/defs.h"

#include "wx/object.h"
#include "wx/base64.h"

import <string>;
import <vector>;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

/// shall we be case sensitive in parsing variable names?
#ifndef wxCONFIG_CASE_SENSITIVE
  #define  wxCONFIG_CASE_SENSITIVE       0
#endif

/// separates group and entry names (probably shouldn't be changed)
#ifndef wxCONFIG_PATH_SEPARATOR
  inline constexpr char wxCONFIG_PATH_SEPARATOR = '/';
#endif

/// introduces immutable entries
// (i.e. the ones which can't be changed from the local config file)
#ifndef wxCONFIG_IMMUTABLE_PREFIX
  inline constexpr char wxCONFIG_IMMUTABLE_PREFIX = '!';
#endif

#if wxUSE_CONFIG

// not all compilers can deal with template Read/Write() methods, define this
// symbol if the template functions are available
#if !defined( __VMS ) && \
    !(defined(__HP_aCC) && defined(__hppa))
    #define wxHAS_CONFIG_TEMPLATE_RW
#endif

// Style flags for constructor style parameter
enum
{
    wxCONFIG_USE_LOCAL_FILE = 1,
    wxCONFIG_USE_GLOBAL_FILE = 2,
    wxCONFIG_USE_RELATIVE_PATH = 4,
    wxCONFIG_USE_NO_ESCAPE_CHARACTERS = 8,
    wxCONFIG_USE_SUBDIR = 16
};

// ----------------------------------------------------------------------------
// abstract base class wxConfigBase which defines the interface for derived
// classes
//
// wxConfig organizes the items in a tree-like structure (modelled after the
// Unix/Dos filesystem). There are groups (directories) and keys (files).
// There is always one current group given by the current path.
//
// Keys are pairs "key_name = value" where value may be of string or integer
// (long) type (TODO doubles and other types such as wxDate coming soon).
// ----------------------------------------------------------------------------

class wxConfigBase : public wxObject
{
public:
  // constants
    // the type of an entry
  enum EntryType
  {
    Type_Unknown,
    Type_String,
    Type_Boolean,
    Type_Integer,    // use Read(long *)
    Type_Float       // use Read(double *)
  };

  // static functions
    // sets the config object, returns the previous pointer
  static wxConfigBase *Set(wxConfigBase *pConfig);
    // get the config object, creates it on demand unless DontCreateOnDemand
    // was called
  static wxConfigBase *Get(bool createOnDemand = true)
       { if ( createOnDemand && (!ms_pConfig) ) Create(); return ms_pConfig; }
    // create a new config object: this function will create the "best"
    // implementation of wxConfig available for the current platform, see
    // comments near definition wxUSE_CONFIG_NATIVE for details. It returns
    // the created object and also sets it as ms_pConfig.
  static wxConfigBase *Create();
    // should Get() try to create a new log object if the current one is NULL?
  static void DontCreateOnDemand() { ms_bAutoCreate = false; }

  // ctor & virtual dtor
      // ctor (can be used as default ctor too)
      //
      // Not all args will always be used by derived classes, but including
      // them all in each class ensures compatibility. If appName is empty,
      // uses wxApp name
  wxConfigBase(const std::string& appName = {},
               const std::string& vendorName = {},
               const std::string& localFilename = {},
               const std::string& globalFilename = {},
               unsigned int style = 0);

  // path management
    // set current path: if the first character is '/', it's the absolute path,
    // otherwise it's a relative path. '..' is supported. If the strPath
    // doesn't exist it is created.
  virtual void SetPath(const std::string& strPath) = 0;
    // retrieve the current path (always as absolute path)
  virtual const std::string& GetPath() const = 0;

  // enumeration: all functions here return false when there are no more items.
  // you must pass the same lIndex to GetNext and GetFirst (don't modify it)
    // enumerate subgroups
  virtual bool GetFirstGroup(std::string& str, long& lIndex) const = 0;
  virtual bool GetNextGroup (std::string& str, long& lIndex) const = 0;
    // enumerate entries
  virtual bool GetFirstEntry(std::string& str, long& lIndex) const = 0;
  virtual bool GetNextEntry (std::string& str, long& lIndex) const = 0;
    // get number of entries/subgroups in the current group, with or without
    // it's subgroups
  virtual size_t GetNumberOfEntries(bool bRecursive = false) const = 0;
  virtual size_t GetNumberOfGroups(bool bRecursive = false) const = 0;

  // tests of existence
    // returns true if the group by this name exists
  virtual bool HasGroup(const std::string& strName) const = 0;
    // same as above, but for an entry
  virtual bool HasEntry(const std::string& strName) const = 0;
    // returns true if either a group or an entry with a given name exist
  bool Exists(const std::string& strName) const
    { return HasGroup(strName) || HasEntry(strName); }

    // get the entry type
  virtual EntryType GetEntryType(const std::string& name) const
  {
    // by default all entries are strings
    return HasEntry(name) ? Type_String : Type_Unknown;
  }

  // key access: returns true if value was really read, false if default used
  // (and if the key is not found the default value is returned.)

    // read a string from the key
  bool Read(const std::string& key, std::string *pStr) const;
  bool Read(const std::string& key, std::string *pStr, const std::string& defVal) const;

    // read a number (long)
  bool Read(const std::string& key, long *pl) const;
  bool Read(const std::string& key, long *pl, long defVal) const;

    // read an int (wrapper around `long' version)
  bool Read(const std::string& key, int *pi) const;
  bool Read(const std::string& key, int *pi, int defVal) const;

    // read a double
  bool Read(const std::string& key, double* val) const;
  bool Read(const std::string& key, double* val, double defVal) const;

    // read a float
  bool Read(const std::string& key, float* val) const;
  bool Read(const std::string& key, float* val, float defVal) const;

    // read a bool
  bool Read(const std::string& key, bool* val) const;
  bool Read(const std::string& key, bool* val, bool defVal) const;

    // read a 64-bit number when long is 32 bits
#ifdef wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG
  bool Read(const std::string& key, wxLongLong_t *pl) const;
  bool Read(const std::string& key, wxLongLong_t *pl, wxLongLong_t defVal) const;
#endif // wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG

  bool Read(const std::string& key, size_t* val) const;
  bool Read(const std::string& key, size_t* val, size_t defVal) const;

#if wxUSE_BASE64
    // read a binary data block
  bool Read(const std::string& key, wxMemoryBuffer* data) const
    { return DoReadBinary(key, data); }
   // no default version since it does not make sense for binary data
#endif // wxUSE_BASE64

#ifdef wxHAS_CONFIG_TEMPLATE_RW
  // read other types, for which wxFromString is defined
  template <typename T>
  bool Read(const std::string& key, T* value) const
  {
      std::string s;
      if ( !Read(key, &s) )
          return false;
      return wxFromString(s, value);
  }

  template <typename T>
  bool Read(const std::string& key, T* value, const T& defVal) const
  {
      const bool found = Read(key, value);
      if ( !found )
      {
          if (IsRecordingDefaults())
              const_cast<wxConfigBase*>(this)->Write(key, defVal);
          *value = defVal;
      }
      return found;
  }
#endif // wxHAS_CONFIG_TEMPLATE_RW

  // convenience functions returning directly the value
  std::string Read(const std::string& key,
                const std::string& defVal = {}) const
    { std::string s; Read(key, &s, defVal); return s; }

  // we have to provide a separate version for C strings as otherwise the
  // template Read() would be used
  [[maybe_unused]] std::string Read(const std::string& key, const char* defVal) const
    { return Read(key, std::string(defVal)); }

  long ReadLong(const std::string& key, long defVal) const
    { long l; Read(key, &l, defVal); return l; }

  wxLongLong_t ReadLongLong(const std::string& key, wxLongLong_t defVal) const
    { wxLongLong_t ll; Read(key, &ll, defVal); return ll; }

  double ReadDouble(const std::string& key, double defVal) const
    { double d; Read(key, &d, defVal); return d; }

  bool ReadBool(const std::string& key, bool defVal) const
    { bool b; Read(key, &b, defVal); return b; }

  template <typename T>
  T ReadObject(const std::string& key, T const& defVal) const
    { T t; Read(key, &t, defVal); return t; }

  // for compatibility with wx 2.8
  long Read(const std::string& key, long defVal) const
    { return ReadLong(key, defVal); }


  // write the value (return true on success)
  bool Write(const std::string& key, const std::string& value)
    { return DoWriteString(key, value); }

  bool Write(const std::string& key, long value)
    { return DoWriteLong(key, value); }

  bool Write(const std::string& key, double value)
    { return DoWriteDouble(key, value); }

  bool Write(const std::string& key, bool value)
    { return DoWriteBool(key, value); }

#if wxUSE_BASE64
  bool Write(const std::string& key, const wxMemoryBuffer& buf)
    { return DoWriteBinary(key, buf); }
#endif // wxUSE_BASE64

  // we have to provide a separate version for C strings as otherwise they
  // would be converted to bool and not to std::string as expected!
  bool Write(const std::string& key, const char *value)
    { return Write(key, std::string(value)); }

  // we also have to provide specializations for other types which we want to
  // handle using the specialized DoWriteXXX() instead of the generic template
  // version below
  bool Write(const std::string& key, char value)
    { return DoWriteLong(key, value); }

  bool Write(const std::string& key, unsigned char value)
    { return DoWriteLong(key, value); }

  bool Write(const std::string& key, short value)
    { return DoWriteLong(key, value); }

  bool Write(const std::string& key, unsigned short value)
    { return DoWriteLong(key, value); }

  bool Write(const std::string& key, unsigned int value)
    { return DoWriteLong(key, value); }

  bool Write(const std::string& key, int value)
    { return DoWriteLong(key, value); }

  bool Write(const std::string& key, unsigned long value)
    { return DoWriteLong(key, value); }

#ifdef wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG
  bool Write(const std::string& key, wxLongLong_t value)
    { return DoWriteLongLong(key, value); }

  bool Write(const std::string& key, wxULongLong_t value)
    { return DoWriteLongLong(key, value); }
#endif // wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG

  bool Write(const std::string& key, float value)
    { return DoWriteDouble(key, double(value)); }

  // Causes ambiguities in under OpenVMS
#if !defined( __VMS )
  // for other types, use wxToString()
  template <typename T>
  bool Write(const std::string& key, T const& value)
    { return Write(key, wxToString(value)); }
#endif

  // permanently writes all changes
  virtual bool Flush(bool bCurrentOnly = false) = 0;

  // renaming, all functions return false on failure (probably because the new
  // name is already taken by an existing entry)
    // rename an entry
  virtual bool RenameEntry(const std::string& oldName,
                           const std::string& newName) = 0;
    // rename a group
  virtual bool RenameGroup(const std::string& oldName,
                           const std::string& newName) = 0;

  // delete entries/groups
    // deletes the specified entry and the group it belongs to if
    // it was the last key in it and the second parameter is true
  virtual bool DeleteEntry(const std::string& key,
                           bool bDeleteGroupIfEmpty = true) = 0;
    // delete the group (with all subgroups)
  virtual bool DeleteGroup(const std::string& key) = 0;
    // delete the whole underlying object (disk file, registry key, ...)
    // primarily for use by uninstallation routine.
  virtual bool DeleteAll() = 0;

  // options
    // we can automatically expand environment variables in the config entries
    // (this option is on by default, you can turn it on/off at any time)
  bool IsExpandingEnvVars() const { return m_bExpandEnvVars; }
  void SetExpandEnvVars(bool bDoIt = true) { m_bExpandEnvVars = bDoIt; }
    // recording of default values
  void SetRecordDefaults(bool bDoIt = true) { m_bRecordDefaults = bDoIt; }
  bool IsRecordingDefaults() const { return m_bRecordDefaults; }
  // does expansion only if needed
  std::string ExpandEnvVars(const std::string& str) const;

    // misc accessors
  std::string GetAppName() const { return m_appName; }
  std::string GetVendorName() const { return m_vendorName; }

  // Used wxIniConfig to set members in constructor
  void SetAppName(const std::string& appName) { m_appName = appName; }
  void SetVendorName(const std::string& vendorName) { m_vendorName = vendorName; }

  void SetStyle(unsigned int style) { m_style = style; }
  unsigned int GetStyle() const { return m_style; }

protected:
  static bool IsImmutable(const std::string& key)
    { return !key.empty() && key[0] == wxCONFIG_IMMUTABLE_PREFIX; }

  // return the path without trailing separator, if any: this should be called
  // to sanitize paths referring to the group names before passing them to
  // wxConfigPathChanger as "/foo/bar/" should be the same as "/foo/bar" and it
  // isn't interpreted in the same way by it (and this can't be changed there
  // as it's not the same for the entries names)
  static std::string RemoveTrailingSeparator(const std::string& key);

  // do read/write the values of different types
  virtual bool DoReadString(const std::string& key, std::string *pStr) const = 0;
  virtual bool DoReadLong(const std::string& key, long *pl) const = 0;
#ifdef wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG
  virtual bool DoReadLongLong(const std::string& key, wxLongLong_t *pll) const;
#endif // wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG
  virtual bool DoReadDouble(const std::string& key, double* val) const;
  virtual bool DoReadBool(const std::string& key, bool* val) const;
#if wxUSE_BASE64
  virtual bool DoReadBinary(const std::string& key, wxMemoryBuffer* buf) const = 0;
#endif // wxUSE_BASE64

  virtual bool DoWriteString(const std::string& key, const std::string& value) = 0;
  virtual bool DoWriteLong(const std::string& key, long value) = 0;
#ifdef wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG
  virtual bool DoWriteLongLong(const std::string& key, wxLongLong_t value);
#endif // wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG
  virtual bool DoWriteDouble(const std::string& key, double value);
  virtual bool DoWriteBool(const std::string& key, bool value);
#if wxUSE_BASE64
  virtual bool DoWriteBinary(const std::string& key, const wxMemoryBuffer& buf) = 0;
#endif // wxUSE_BASE64

private:
  // are we doing automatic environment variable expansion?
  bool m_bExpandEnvVars{true};
  // do we record default values?
  bool m_bRecordDefaults{false};

  // static variables
  inline static wxConfigBase *ms_pConfig{nullptr};
  inline static bool          ms_bAutoCreate{true};

  // Application name and organisation name
  std::string          m_appName;
  std::string          m_vendorName;

  // Style flag
  unsigned int      m_style;

  wxDECLARE_ABSTRACT_CLASS(wxConfigBase);
};

// a handy little class which changes current path to the path of given entry
// and restores it in dtor: so if you declare a local variable of this type,
// you work in the entry directory and the path is automatically restored
// when the function returns
// Taken out of wxConfig since not all compilers can cope with nested classes.
class wxConfigPathChanger
{
public:
  // ctor/dtor do path changing/restoring of the path
  wxConfigPathChanger(const wxConfigBase *pContainer, const std::string& strEntry);
  ~wxConfigPathChanger();

  wxConfigPathChanger& operator=(wxConfigPathChanger&&) = delete;

  // get the key name
  const std::string& Name() const { return m_strName; }

  // this method must be called if the original path (i.e. the current path at
  // the moment of creation of this object) could have been deleted to prevent
  // us from restoring the not existing (any more) path
  //
  // if the original path doesn't exist any more, the path will be restored to
  // the deepest still existing component of the old path
  void UpdateIfDeleted();

private:
  wxConfigBase *m_pContainer;   // object we live in
  std::string      m_strName,      // name of entry (i.e. name only)
                m_strOldPath;   // saved path
  bool          m_bChanged{false};     // was the path changed?
};


#endif // wxUSE_CONFIG

/*
  Replace environment variables ($SOMETHING) with their values. The format is
  $VARNAME or ${VARNAME} where VARNAME contains alphanumeric characters and
  '_' only. '$' must be escaped ('\$') in order to be taken literally.
*/

std::string wxExpandEnvVars(const std::string &sz);

/*
  Split path into parts removing '..' in progress
 */
std::vector<std::string> wxSplitPath(const std::string& path);

#endif // _WX_CONFBASE_H_

