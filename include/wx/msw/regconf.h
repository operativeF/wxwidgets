///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/regconf.h
// Purpose:     Registry based implementation of wxConfigBase
// Author:      Vadim Zeitlin
// Modified by:
// Created:     27.04.98
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_REGCONF_H_
#define _WX_MSW_REGCONF_H_

#if wxUSE_CONFIG && wxUSE_REGKEY

#include "wx/msw/registry.h"
#include "wx/object.h"
#include "wx/confbase.h"
#include "wx/buffer.h"

// ----------------------------------------------------------------------------
// wxRegConfig
// ----------------------------------------------------------------------------

class wxRegConfig : public wxConfigBase
{
public:
  // ctor & dtor
    // will store data in HKLM\appName and HKCU\appName
  wxRegConfig(const std::string& appName = {},
              const std::string& vendorName = {},
              const std::string& localFilename = {},
              const std::string& globalFilename = {},
              unsigned int style = wxCONFIG_USE_GLOBAL_FILE);

	wxRegConfig& operator=(wxRegConfig&&) = delete;

  // implement inherited pure virtual functions
  // ------------------------------------------

  // path management
  void SetPath(const std::string& strPath) override;
  const std::string& GetPath() const override { return m_strPath; }

  // entry/subgroup info
    // enumerate all of them
  bool GetFirstGroup(std::string& str, long& lIndex) const override;
  bool GetNextGroup (std::string& str, long& lIndex) const override;
  bool GetFirstEntry(std::string& str, long& lIndex) const override;
  bool GetNextEntry (std::string& str, long& lIndex) const override;

    // tests for existence
  bool HasGroup(const std::string& strName) const override;
  bool HasEntry(const std::string& strName) const override;
  EntryType GetEntryType(const std::string& name) const override;

    // get number of entries/subgroups in the current group, with or without
    // it's subgroups
  size_t GetNumberOfEntries(bool bRecursive = false) const override;
  size_t GetNumberOfGroups(bool bRecursive = false) const override;

  bool Flush([[maybe_unused]] bool bCurrentOnly = false) override { return true; }

  // rename
  bool RenameEntry(const std::string& oldName, const std::string& newName) override;
  bool RenameGroup(const std::string& oldName, const std::string& newName) override;

  // delete
  bool DeleteEntry(const std::string& key, bool bGroupIfEmptyAlso = true) override;
  bool DeleteGroup(const std::string& key) override;
  bool DeleteAll() override;

protected:
  // opens the local key creating it if necessary and returns it
  wxRegKey& LocalKey() const // must be const to be callable from const funcs
  {
      wxRegConfig* self = const_cast<wxRegConfig *>(this);

      if ( !m_keyLocal.IsOpened() )
      {
          // create on demand
          self->m_keyLocal.Create();
      }

      return self->m_keyLocal;
  }

  // Type-independent implementation of Do{Read,Write}Foo().
  template <typename T>
  bool DoReadValue(const std::string& key, T* pValue) const;
  template <typename T>
  bool DoWriteValue(const std::string& key, const T& value);

  // implement read/write methods
  bool DoReadString(const std::string& key, std::string *pStr) const override;
  bool DoReadLong(const std::string& key, long *plResult) const override;
  bool DoReadLongLong(const std::string& key, wxLongLong_t *pll) const override;
#if wxUSE_BASE64
  bool DoReadBinary(const std::string& key, wxMemoryBuffer* buf) const override;
#endif // wxUSE_BASE64

  bool DoWriteString(const std::string& key, const std::string& szValue) override;
  bool DoWriteLong(const std::string& key, long lValue) override;
  bool DoWriteLongLong(const std::string& key, wxLongLong_t llValue) override;
#if wxUSE_BASE64
  bool DoWriteBinary(const std::string& key, const wxMemoryBuffer& buf) override;
#endif // wxUSE_BASE64

private:
  // these keys are opened during all lifetime of wxRegConfig object
  wxRegKey  m_keyLocalRoot,  m_keyLocal,
            m_keyGlobalRoot, m_keyGlobal;

  // current path (not '/' terminated)
  std::string  m_strPath;

  wxDECLARE_ABSTRACT_CLASS(wxRegConfig);
};

#endif // wxUSE_CONFIG && wxUSE_REGKEY

#endif // _WX_MSW_REGCONF_H_
