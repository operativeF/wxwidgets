///////////////////////////////////////////////////////////////////////////////
// Name:        wx/fileconf.h
// Purpose:     wxFileConfig derivation of wxConfigBase
// Author:      Vadim Zeitlin
// Modified by:
// Created:     07.04.98 (adapted from appconf.cpp)
// Copyright:   (c) 1997 Karsten Ballueder   &  Vadim Zeitlin
//                       Ballueder@usa.net     <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef   _FILECONF_H
#define   _FILECONF_H

#if wxUSE_CONFIG

#include "wx/confbase.h"

import WX.File.Filename;

import WX.Cmn.TextBuffer;
import WX.Cmn.TextFile;

// ----------------------------------------------------------------------------
// wxFileConfig
// ----------------------------------------------------------------------------

/*
  wxFileConfig derives from base Config and implements file based config class,
  i.e. it uses ASCII disk files to store the information. These files are
  alternatively called INI, .conf or .rc in the documentation. They are
  organized in groups or sections, which can nest (i.e. a group contains
  subgroups, which contain their own subgroups &c). Each group has some
  number of entries, which are "key = value" pairs. More precisely, the format
  is:

  # comments are allowed after either ';' or '#' (Win/UNIX standard)

  # blank lines (as above) are ignored

  # global entries are members of special (no name) top group
  written_for = Windows
  platform    = Linux

  # the start of the group 'Foo'
  [Foo]                           # may put comments like this also
  # following 3 lines are entries
  key = value
  another_key = "  strings with spaces in the beginning should be quoted, \
                   otherwise the spaces are lost"
  last_key = but you don't have to put " normally (nor quote them, like here)

  # subgroup of the group 'Foo'
  # (order is not important, only the name is: separator is '/', as in paths)
  [Foo/Bar]
  # entries prefixed with "!" are immutable, i.e. can't be changed if they are
  # set in the system-wide config file
  !special_key = value
  bar_entry = whatever

  [Foo/Bar/Fubar]   # depth is (theoretically :-) unlimited
  # may have the same name as key in another section
  bar_entry = whatever not

  You have {read/write/delete}Entry functions (guess what they do) and also
  setCurrentPath to select current group. enum{Subgroups/Entries} allow you
  to get all entries in the config file (in the current group). Finally,
  flush() writes immediately all changed entries to disk (otherwise it would
  be done automatically in dtor)

  wxFileConfig manages not less than 2 config files for each program: global
  and local (or system and user if you prefer). Entries are read from both of
  them and the local entries override the global ones unless the latter is
  immutable (prefixed with '!') in which case a warning message is generated
  and local value is ignored. Of course, the changes are always written to local
  file only.

  The names of these files can be specified in a number of ways. First of all,
  you can use the standard convention: using the ctor which takes 'strAppName'
  parameter will probably be sufficient for 90% of cases. If, for whatever
  reason you wish to use the files with some other names, you can always use the
  second ctor.

  wxFileConfig also may automatically expand the values of environment variables
  in the entries it reads: for example, if you have an entry
    score_file = $HOME/.score
  a call to Read(&str, "score_file") will return a complete path to .score file
  unless the expansion was previously disabled with SetExpandEnvVars(false) call
  (it's on by default, the current status can be retrieved with
   IsExpandingEnvVars function).
*/
class wxFileConfigGroup;
class wxFileConfigEntry;
class wxFileConfigLineList;

#if wxUSE_STREAMS
class wxInputStream;
class wxOutputStream;
#endif // wxUSE_STREAMS

class wxFileConfig : public wxConfigBase
{
public:
  // construct the "standard" full name for global (system-wide) and
  // local (user-specific) config files from the base file name.
  //
  // the following are the filenames returned by this functions:
  //            global                local
  // Unix   /etc/file.ext           ~/.file
  // Win    %windir%\file.ext   %USERPROFILE%\file.ext
  //
  // where file is the basename of szFile, ext is its extension
  // or .conf (Unix) or .ini (Win) if it has none
  static wxFileName GetGlobalFile(const std::string& szFile);
  static wxFileName GetLocalFile(const std::string& szFile, unsigned int style = 0);

  static std::string GetGlobalFileName(const std::string& szFile)
  {
      return GetGlobalFile(szFile).GetFullPath();
  }

  static std::string GetLocalFileName(const std::string& szFile, unsigned int style = 0)
  {
      return GetLocalFile(szFile, style).GetFullPath();
  }

    // New constructor: one size fits all. Specify wxCONFIG_USE_LOCAL_FILE or
    // wxCONFIG_USE_GLOBAL_FILE to say which files should be used.
  wxFileConfig(const std::string& appName = {},
               const std::string& vendorName = {},
               const std::string& localFilename = {},
               const std::string& globalFilename = {},
               unsigned int style = wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_GLOBAL_FILE,
               const wxMBConv& conv = wxConvAuto());

#if wxUSE_STREAMS
    // ctor that takes an input stream.
  wxFileConfig(wxInputStream &inStream, const wxMBConv& conv = wxConvAuto());
#endif // wxUSE_STREAMS

    // dtor will save unsaved data
  ~wxFileConfig();

  wxFileConfig& operator=(wxFileConfig&&) = delete;

  // under Unix, set the umask to be used for the file creation, do nothing
  // under other systems
#ifdef __UNIX__
  void SetUmask(int mode) { m_umask = mode; }
#else // !__UNIX__
  void SetUmask([[maybe_unused]] int mode) { }
#endif // __UNIX__/!__UNIX__

  // implement inherited pure virtual functions
  void SetPath(const std::string& strPath) override;
  const std::string& GetPath() const override;

  bool GetFirstGroup(std::string& str, long& lIndex) const override;
  bool GetNextGroup (std::string& str, long& lIndex) const override;
  bool GetFirstEntry(std::string& str, long& lIndex) const override;
  bool GetNextEntry (std::string& str, long& lIndex) const override;

  size_t GetNumberOfEntries(bool bRecursive = false) const override;
  size_t GetNumberOfGroups(bool bRecursive = false) const override;

  bool HasGroup(const std::string& strName) const override;
  bool HasEntry(const std::string& strName) const override;

  bool Flush(bool bCurrentOnly = false) override;

  bool RenameEntry(const std::string& oldName, const std::string& newName) override;
  bool RenameGroup(const std::string& oldName, const std::string& newName) override;

  bool DeleteEntry(const std::string& key, bool bGroupIfEmptyAlso = true) override;
  bool DeleteGroup(const std::string& szKey) override;
  bool DeleteAll() override;

  // additional, wxFileConfig-specific, functionality
#if wxUSE_STREAMS
  // save the entire config file text to the given stream, note that the text
  // won't be saved again in dtor when Flush() is called if you use this method
  // as it won't be "changed" any more
  virtual bool Save(wxOutputStream& os, const wxMBConv& conv = wxConvAuto());
#endif // wxUSE_STREAMS

  void EnableAutoSave() { m_autosave = true; }
  void DisableAutoSave() { m_autosave = false; }

public:
  // functions to work with this list
  wxFileConfigLineList *LineListAppend(const std::string& str);
  wxFileConfigLineList *LineListInsert(const std::string& str,
                           wxFileConfigLineList *pLine);    // NULL => Prepend()
  void      LineListRemove(wxFileConfigLineList *pLine);
  bool      LineListIsEmpty();

protected:
  bool DoReadString(const std::string& key, std::string *pStr) const override;
  bool DoReadLong(const std::string& key, long *pl) const override;
#if wxUSE_BASE64
  bool DoReadBinary(const std::string& key, wxMemoryBuffer* buf) const override;
#endif // wxUSE_BASE64

  bool DoWriteString(const std::string& key, const std::string& szValue) override;
  bool DoWriteLong(const std::string& key, long lValue) override;
#if wxUSE_BASE64
  bool DoWriteBinary(const std::string& key, const wxMemoryBuffer& buf) override;
#endif // wxUSE_BASE64

private:
  // GetXXXFileName helpers: return ('/' terminated) directory names
  static std::string GetGlobalDir();
  static std::string GetLocalDir(unsigned int style = 0);

  // common part of from dtor and DeleteAll
  void CleanUp();

  // parse the whole file
  void Parse(const wxTextBuffer& buffer, bool bLocal);

  // the same as SetPath("/")
  void SetRootPath();

  // real SetPath() implementation, returns true if path could be set or false
  // if path doesn't exist and createMissingComponents == false
  bool DoSetPath(const std::string& strPath, bool createMissingComponents);

  // set/test the dirty flag
  void SetDirty() { m_isDirty = true; }
  void ResetDirty() { m_isDirty = false; }
  bool IsDirty() const { return m_isDirty; }


  // member variables
  // ----------------
  wxFileConfigLineList *m_linesHead{nullptr};    // head of the linked list
  wxFileConfigLineList *m_linesTail{nullptr};    // tail

  wxFileName  m_fnLocalFile,            // local  file name passed to ctor
              m_fnGlobalFile;           // global
  std::string    m_strPath;                // current path (not '/' terminated)

  wxFileConfigGroup *m_pRootGroup,      // the top (unnamed) group
                    *m_pCurrentGroup;   // the current group

  wxMBConv    *m_conv;

#ifdef __UNIX__
  int m_umask;                          // the umask to use for file creation
#endif // __UNIX__

  bool m_isDirty{false};                       // if true, we have unsaved changes
  bool m_autosave{true};                      // if true, save changes on destruction

  wxDECLARE_ABSTRACT_CLASS(wxFileConfig);
};

#endif
  // wxUSE_CONFIG

#endif
  //_FILECONF_H

