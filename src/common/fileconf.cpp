///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/fileconf.cpp
// Purpose:     implementation of wxFileConfig derivation of wxConfig
// Author:      Vadim Zeitlin
// Modified by:
// Created:     07.04.98 (adapted from appconf.cpp)
// Copyright:   (c) 1997 Karsten Ballueder  &  Vadim Zeitlin
//                       Ballueder@usa.net     <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_CONFIG && wxUSE_FILECONFIG

#include  "wx/dynarray.h"
#include  "wx/intl.h"
#include  "wx/log.h"
#include  "wx/app.h"
#include  "wx/utils.h"    // for wxGetHomeDir
#if wxUSE_STREAMS
    #include  "wx/stream.h"
#endif // wxUSE_STREAMS

#include  "wx/file.h"
#include  "wx/textfile.h"
#include  "wx/memtext.h"
#include  "wx/config.h"
#include  "wx/fileconf.h"
#include  "wx/filefn.h"

#include "wx/base64.h"

#include  "wx/stdpaths.h"

#if defined(WX_WINDOWS)
    #include "wx/msw/private.h"
#endif  //windows.h

#include <fmt/core.h>

import Utils.Strings;

import <cctype>;
import <cstdlib>;
import <charconv>;
import <string>;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

constexpr char FILECONF_TRACE_MASK[] = "fileconf";

// ----------------------------------------------------------------------------
// global functions declarations
// ----------------------------------------------------------------------------

// compare functions for sorting the arrays
static int CompareEntries(wxFileConfigEntry *p1, wxFileConfigEntry *p2);
static int CompareGroups(wxFileConfigGroup *p1, wxFileConfigGroup *p2);

// filter strings
static std::string FilterInValue(const std::string& str);
static std::string FilterOutValue(const std::string& str);

static std::string FilterInEntryName(const std::string& str);
static std::string FilterOutEntryName(const std::string& str);

// ============================================================================
// private classes
// ============================================================================

// ----------------------------------------------------------------------------
// "template" array types
// ----------------------------------------------------------------------------

#ifdef WXMAKINGDLL_BASE
    WX_DEFINE_SORTED_USER_EXPORTED_ARRAY(wxFileConfigEntry *, ArrayEntries,
                                         WXDLLIMPEXP_BASE);
    WX_DEFINE_SORTED_USER_EXPORTED_ARRAY(wxFileConfigGroup *, ArrayGroups,
                                         WXDLLIMPEXP_BASE);
#else
    WX_DEFINE_SORTED_ARRAY(wxFileConfigEntry *, ArrayEntries);
    WX_DEFINE_SORTED_ARRAY(wxFileConfigGroup *, ArrayGroups);
#endif

// ----------------------------------------------------------------------------
// wxFileConfigLineList
// ----------------------------------------------------------------------------

// we store all lines of the local config file as a linked list in memory
class wxFileConfigLineList
{
public:
  void SetNext(wxFileConfigLineList *pNext)  { m_pNext = pNext; }
  void SetPrev(wxFileConfigLineList *pPrev)  { m_pPrev = pPrev; }

  explicit wxFileConfigLineList(const std::string& str,
                       wxFileConfigLineList *pNext = nullptr) : m_strLine(str)
    { SetNext(pNext); SetPrev(nullptr); }

  wxFileConfigLineList(const wxFileConfigLineList&) = delete;
  wxFileConfigLineList& operator=(const wxFileConfigLineList&) = delete;

  // next/prev nodes in the linked list
  wxFileConfigLineList *Next() const { return m_pNext;  }
  wxFileConfigLineList *Prev() const { return m_pPrev;  }

  // get/change lines text
  void SetText(const std::string& str) { m_strLine = str;  }
  const std::string& Text() const { return m_strLine; }

private:
  std::string  m_strLine;                  // line contents
  wxFileConfigLineList *m_pNext,        // next node
                       *m_pPrev;        // previous one
};

// ----------------------------------------------------------------------------
// wxFileConfigEntry: a name/value pair
// ----------------------------------------------------------------------------

class wxFileConfigEntry
{
private:
  std::string      m_strName,      // entry name
                m_strValue;     // value

  wxFileConfigGroup *m_pParent; // group that contains us

  // pointer to our line in the linked list or NULL if it was found in global
  // file (which we don't modify)
  wxFileConfigLineList* m_pLine;

  int           m_nLine;        // used if m_pLine == NULL only

  bool          m_bImmutable; // can be overridden locally?
  bool          m_bHasValue;  // set after first call to SetValue()

public:
  wxFileConfigEntry(wxFileConfigGroup *pParent,
                    const std::string& strName, int nLine);

  // simple accessors
  const std::string& Name()        const { return m_strName;    }
  const std::string& Value()       const { return m_strValue;   }
  wxFileConfigGroup *Group()    const { return m_pParent;    }
  bool            IsImmutable() const { return m_bImmutable; }
  bool            IsLocal()     const { return m_pLine != nullptr; }
  int             Line()        const { return m_nLine;      }
  wxFileConfigLineList *
                  GetLine()     const { return m_pLine;      }

  // modify entry attributes
  void SetValue(const std::string& strValue, bool bUser = true);
  void SetLine(wxFileConfigLineList *pLine);

    wxFileConfigEntry(const wxFileConfigEntry&) = delete;
	wxFileConfigEntry& operator=(const wxFileConfigEntry&) = delete;
};

// ----------------------------------------------------------------------------
// wxFileConfigGroup: container of entries and other groups
// ----------------------------------------------------------------------------

class wxFileConfigGroup
{
private:
  wxFileConfig *m_pConfig;          // config object we belong to
  wxFileConfigGroup  *m_pParent;    // parent group (NULL for root group)
  ArrayEntries  m_aEntries;         // entries in this group
  ArrayGroups   m_aSubgroups;       // subgroups
  std::string      m_strName;          // group's name
  wxFileConfigLineList *m_pLine{nullptr};    // pointer to our line in the linked list
  wxFileConfigEntry *m_pLastEntry{nullptr};  // last entry/subgroup of this group in the
  wxFileConfigGroup *m_pLastGroup{nullptr};  // local file (we insert new ones after it)

  // DeleteSubgroupByName helper
  bool DeleteSubgroup(wxFileConfigGroup *pGroup);

  // used by Rename()
  void UpdateGroupAndSubgroupsLines();

public:
  // ctor
  wxFileConfigGroup(wxFileConfigGroup *pParent, const std::string& strName, wxFileConfig *);

  // dtor deletes all entries and subgroups also
  ~wxFileConfigGroup();

  // simple accessors
  const std::string& Name()    const { return m_strName; }
  wxFileConfigGroup    *Parent()  const { return m_pParent; }
  wxFileConfig   *Config()  const { return m_pConfig; }

  const ArrayEntries& Entries() const { return m_aEntries;   }
  const ArrayGroups&  Groups()  const { return m_aSubgroups; }
  bool  IsEmpty() const { return Entries().IsEmpty() && Groups().IsEmpty(); }

  // find entry/subgroup (NULL if not found)
  wxFileConfigGroup *FindSubgroup(const std::string& name) const;
  wxFileConfigEntry *FindEntry   (const std::string& name) const;

  // delete entry/subgroup, return false if doesn't exist
  bool DeleteSubgroupByName(const std::string& name);
  bool DeleteEntry(const std::string& name);

  // create new entry/subgroup returning pointer to newly created element
  wxFileConfigGroup *AddSubgroup(const std::string& strName);
  wxFileConfigEntry *AddEntry   (const std::string& strName, int nLine = wxNOT_FOUND);

  void SetLine(wxFileConfigLineList *pLine);

  // rename: no checks are done to ensure that the name is unique!
  void Rename(const std::string& newName);

  //
  std::string GetFullName() const;

  // get the last line belonging to an entry/subgroup of this group
  wxFileConfigLineList *GetGroupLine();     // line which contains [group]
                                            // may be NULL for "/" only
  wxFileConfigLineList *GetLastEntryLine(); // after which our subgroups start
  wxFileConfigLineList *GetLastGroupLine(); // after which the next group starts

  // called by entries/subgroups when they're created/deleted
  void SetLastEntry(wxFileConfigEntry *pEntry);
  void SetLastGroup(wxFileConfigGroup *pGroup)
    { m_pLastGroup = pGroup; }

  wxFileConfigGroup(const wxFileConfigGroup&) = delete;
	wxFileConfigGroup& operator=(const wxFileConfigGroup&) = delete;
};

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// static functions
// ----------------------------------------------------------------------------

std::string wxFileConfig::GetGlobalDir()
{
    return wxStandardPaths::Get().GetConfigDir();
}

std::string wxFileConfig::GetLocalDir(unsigned int style)
{
    wxUnusedVar(style);

    const wxStandardPathsBase& stdp = wxStandardPaths::Get();

    // it so happens that user data directory is a subdirectory of user config
    // directory on all supported platforms, which explains why we use it here
    return style & wxCONFIG_USE_SUBDIR ? stdp.GetUserDataDir()
                                       : stdp.GetUserConfigDir();
}

wxFileName wxFileConfig::GetGlobalFile(const std::string& szFile)
{
    const wxStandardPathsBase& stdp = wxStandardPaths::Get();

    return {GetGlobalDir(), stdp.MakeConfigFileName(szFile)};
}

wxFileName wxFileConfig::GetLocalFile(const std::string& szFile, unsigned int style)
{
    const wxStandardPathsBase& stdp = wxStandardPaths::Get();

    // If the config file is located in a subdirectory, we always use an
    // extension for it, but we use just the leading dot if it is located
    // directly in the home directory. Note that if wxStandardPaths is
    // configured to follow XDG specification, all config files go to a
    // subdirectory of XDG_CONFIG_HOME anyhow, so in this case we'll still end
    // up using the extension even if wxCONFIG_USE_SUBDIR is not set, but this
    // is the correct and expected (if a little confusing) behaviour.
    const wxStandardPaths::ConfigFileConv
        conv = style & wxCONFIG_USE_SUBDIR
                ? wxStandardPaths::ConfigFileConv_Ext
                : wxStandardPaths::ConfigFileConv_Dot;

    return {GetLocalDir(style), stdp.MakeConfigFileName(szFile, conv)};
}

// ----------------------------------------------------------------------------
// ctor
// ----------------------------------------------------------------------------
wxIMPLEMENT_ABSTRACT_CLASS(wxFileConfig, wxConfigBase);

// constructor supports creation of wxFileConfig objects of any type
wxFileConfig::wxFileConfig(const std::string& appName, const std::string& vendorName,
                           const std::string& strLocal, const std::string& strGlobal,
                           unsigned int style,
                           const wxMBConv& conv)
            : wxConfigBase(( appName.empty() && wxTheApp ) ? wxTheApp->GetAppName() : appName,
                           vendorName,
                           strLocal, strGlobal,
                           style),
              m_fnLocalFile(strLocal),
              m_fnGlobalFile(strGlobal),
              m_conv(conv.Clone())
{
    // Make up names for files if empty
    if ( !m_fnLocalFile.IsOk() && (style & wxCONFIG_USE_LOCAL_FILE) )
        m_fnLocalFile = GetLocalFile(GetAppName(), style);

    if ( !m_fnGlobalFile.IsOk() && (style & wxCONFIG_USE_GLOBAL_FILE) )
        m_fnGlobalFile = GetGlobalFile(GetAppName());

    // Check if styles are not supplied, but filenames are, in which case
    // add the correct styles.
    if ( m_fnLocalFile.IsOk() )
        SetStyle(GetStyle() | wxCONFIG_USE_LOCAL_FILE);

    if ( m_fnGlobalFile.IsOk() )
        SetStyle(GetStyle() | wxCONFIG_USE_GLOBAL_FILE);

    // if the path is not absolute, prepend the standard directory to it
    // unless explicitly asked not to
    if ( !(style & wxCONFIG_USE_RELATIVE_PATH) )
    {
        if ( m_fnLocalFile.IsOk() )
            m_fnLocalFile.MakeAbsolute(GetLocalDir(style));

        if ( m_fnGlobalFile.IsOk() )
            m_fnGlobalFile.MakeAbsolute(GetGlobalDir());
    }

    SetUmask(-1);

    
    m_pCurrentGroup =
    m_pRootGroup    = new wxFileConfigGroup(nullptr, {}, this);

    m_linesHead =
    m_linesTail = nullptr;

    // It's not an error if (one of the) file(s) doesn't exist.

    // parse the global file
    if ( m_fnGlobalFile.IsOk() && m_fnGlobalFile.FileExists() )
    {
        wxTextFile fileGlobal(m_fnGlobalFile.GetFullPath());

        if ( fileGlobal.Open(*m_conv/*ignored in ANSI build*/) )
        {
            Parse(fileGlobal, false /* global */);
            SetRootPath();
        }
        else
        {
            wxLogWarning(_("can't open global configuration file '%s'."), m_fnGlobalFile.GetFullPath().c_str());
        }
    }

    // parse the local file
    if ( m_fnLocalFile.IsOk() && m_fnLocalFile.FileExists() )
    {
        wxTextFile fileLocal(m_fnLocalFile.GetFullPath());
        if ( fileLocal.Open(*m_conv/*ignored in ANSI build*/) )
        {
            Parse(fileLocal, true /* local */);
            SetRootPath();
        }
        else
        {
            const std::string path = m_fnLocalFile.GetFullPath();
            wxLogWarning(_("can't open user configuration file '%s'."),
                         path.c_str());

            if ( m_fnLocalFile.FileExists() )
            {
                wxLogWarning(_("Changes won't be saved to avoid overwriting the existing file \"%s\""),
                             path.c_str());
                m_fnLocalFile.Clear();
            }
        }
    }

    m_isDirty = false;
    m_autosave = true;

}

#if wxUSE_STREAMS

wxFileConfig::wxFileConfig(wxInputStream &inStream, const wxMBConv& conv)
            : m_conv(conv.Clone())
{
    // always local_file when this constructor is called (?)
    SetStyle(GetStyle() | wxCONFIG_USE_LOCAL_FILE);

    m_pCurrentGroup =
    m_pRootGroup    = new wxFileConfigGroup(nullptr, {}, this);

    m_linesHead =
    m_linesTail = nullptr;

    // read the entire stream contents in memory
    wxWxCharBuffer cbuf;
    static constexpr size_t chunkLen = 1024;

    wxMemoryBuffer buf(chunkLen);
    do
    {
        inStream.Read(buf.GetAppendBuf(chunkLen), chunkLen);
        buf.UngetAppendBuf(inStream.LastRead());

        const wxStreamError err = inStream.GetLastError();

        if ( err != wxSTREAM_NO_ERROR && err != wxSTREAM_EOF )
        {
            wxLogError(_("Error reading config options."));
            break;
        }
    }
    while ( !inStream.Eof() );

    size_t len;
    cbuf = conv.cMB2WC((char *)buf.GetData(), buf.GetDataLen() + 1, &len);
    if ( !len && buf.GetDataLen() )
    {
        wxLogError(_("Failed to read config options."));
    }

    // parse the input contents if there is anything to parse
    if ( cbuf )
    {
        // now break it into lines
        wxMemoryText memText;
        for ( const wxChar *s = cbuf; ; ++s )
        {
            const wxChar *e = s;
            while ( *e != '\0' && *e != '\n' && *e != '\r' )
                ++e;

            // notice that we throw away the original EOL kind here, maybe we
            // should preserve it?
            if ( e != s )
                memText.AddLine(std::string(s, e));

            if ( *e == '\0' )
                break;

            // skip the second EOL byte if it's a DOS one
            if ( *e == '\r' && e[1] == '\n' )
                ++e;

            s = e;
        }

        // Finally we can parse it all.
        Parse(memText, true /* local */);
    }

    SetRootPath();
    ResetDirty();
}

#endif // wxUSE_STREAMS

void wxFileConfig::CleanUp()
{
    delete m_pRootGroup;

    wxFileConfigLineList *pCur = m_linesHead;
    while ( pCur != nullptr ) {
        wxFileConfigLineList *pNext = pCur->Next();
        delete pCur;
        pCur = pNext;
    }
}

wxFileConfig::~wxFileConfig()
{
    if ( m_autosave )
        Flush();

    CleanUp();

    delete m_conv;
}

// ----------------------------------------------------------------------------
// parse a config file
// ----------------------------------------------------------------------------

// FIXME: This is all pretty bogus.
void wxFileConfig::Parse(const wxTextBuffer& buffer, bool bLocal)
{

  for ( size_t n = 0; n < buffer.GetLineCount(); n++ )
  {
    std::string strLine = buffer[n];

    // add the line to linked list
    if ( bLocal )
      LineListAppend(strLine);

    // FIXME: where should this begin, really?
    auto pEnd = strLine.begin();
    auto pStart = strLine.begin();
    // skip leading spaces
    for ( ; wxIsspace(*pStart); pStart = std::next(pStart) )
      ;

    // skip blank/comment lines
    if ( *pStart == '\0'|| *pStart == ';' || *pStart == '#' )
      continue;

    if ( *pStart == '[' ) {          // a new group
      pEnd = pStart;

      while ( *++pEnd != ']' ) {
        if ( *pEnd == '\\' ) {
            // the next char is escaped, so skip it even if it is ']'
            pEnd++;
        }

        if ( *pEnd == '\n' || *pEnd == '\0' ) {
            // we reached the end of line, break out of the loop
            break;
        }
      }

      if ( *pEnd != ']' ) {
        wxLogError(_("file '%s': unexpected character %c at line %zu."),
                   buffer.GetName(), *pEnd, n + 1);
        continue; // skip this line
      }

      // group name here is always considered as abs path
      pStart++;
      std::string strGroup = fmt::format("{}{}", wxCONFIG_PATH_SEPARATOR,
                                                 FilterInEntryName(std::string{pStart, pEnd}));

      // will create it if doesn't yet exist
      SetPath(strGroup);

      if ( bLocal )
      {
        if ( m_pCurrentGroup->Parent() )
          m_pCurrentGroup->Parent()->SetLastGroup(m_pCurrentGroup);
        m_pCurrentGroup->SetLine(m_linesTail);
      }

      // check that there is nothing except comments left on this line
      bool bCont = true;
      while ( *++pEnd != '\0' && bCont ) {
        switch ( *pEnd ) {
          case '#':
          case ';':
            bCont = false;
            break;

          case ' ':
          case '\t':
            // ignore whitespace ('\n' impossible here)
            break;

          default:
            // FIXME: SPD log
            //wxLogWarning(_("file '%s', line %zu: '%s' ignored after group header."),
            //             buffer.GetName().ToStdString(), n + 1, pEnd);
            bCont = false;
        }
      }
    }
    else {                        // a key
      pEnd = pStart;
      while ( *pEnd && *pEnd != '=' /* && !wxIsspace(*pEnd)*/ ) {
        if ( *pEnd == '\\' ) {
          // next character may be space or not - still take it because it's
          // quoted (unless there is nothing)
          pEnd++;
          if ( !*pEnd ) {
            // the error message will be given below anyhow
            break;
          }
        }

        pEnd++;
      }

      std::string strKey = wx::utils::StripTrailingSpace(FilterInEntryName(std::string{pStart, pEnd}));

      // skip whitespace
      while ( wxIsspace(*pEnd) )
        pEnd++;

      if ( *pEnd++ != '=' ) {
        wxLogError(_("file '%s', line %zu: '=' expected."),
                   buffer.GetName(), n + 1);
      }
      else {
        wxFileConfigEntry *pEntry = m_pCurrentGroup->FindEntry(strKey);

        if ( pEntry == nullptr ) {
          // new entry
          pEntry = m_pCurrentGroup->AddEntry(strKey, n);
        }
        else {
          if ( bLocal && pEntry->IsImmutable() ) {
            // immutable keys can't be changed by user
            wxLogWarning(_("file '%s', line %zu: value for immutable key '%s' ignored."),
                         buffer.GetName(), n + 1, strKey.c_str());
            continue;
          }
          // the condition below catches the cases (a) and (b) but not (c):
          //  (a) global key found second time in global file
          //  (b) key found second (or more) time in local file
          //  (c) key from global file now found in local one
          // which is exactly what we want.
          else if ( !bLocal || pEntry->IsLocal() ) {
            wxLogWarning(_("file '%s', line %zu: key '%s' was first found at line %d."),
                         buffer.GetName(), n + 1, strKey.c_str(), pEntry->Line());

          }
        }

        if ( bLocal )
          pEntry->SetLine(m_linesTail);

        // skip whitespace
        while ( wxIsspace(*pEnd) )
          pEnd++;

        std::string value{pEnd, strLine.end()}; // FIXME: Just remove all of this. This probably isn't right.
        if ( !(GetStyle() & wxCONFIG_USE_NO_ESCAPE_CHARACTERS) )
            value = FilterInValue(value);

        pEntry->SetValue(value, false);
      }
    }
  }
}

// ----------------------------------------------------------------------------
// set/retrieve path
// ----------------------------------------------------------------------------

void wxFileConfig::SetRootPath()
{
    m_strPath.clear();
    m_pCurrentGroup = m_pRootGroup;
}

bool
wxFileConfig::DoSetPath(const std::string& strPath, bool createMissingComponents)
{
    std::vector<std::string> aParts;

    if ( strPath.empty() ) {
        SetRootPath();
        return true;
    }

    // TODO: Use lambda.
    if ( strPath[0] == wxCONFIG_PATH_SEPARATOR ) {
        // absolute path
        aParts = wxSplitPath(strPath);
    }
    else {
        // relative path, combine with current one
        std::string strFullPath = fmt::format("{}{}{}", m_strPath, wxCONFIG_PATH_SEPARATOR, strPath);
        aParts = wxSplitPath(strFullPath);
    }

    // change current group
    m_pCurrentGroup = m_pRootGroup;
    for ( size_t n = 0; n < aParts.size(); n++ ) {
        wxFileConfigGroup *pNextGroup = m_pCurrentGroup->FindSubgroup(aParts[n]);
        if ( pNextGroup == nullptr )
        {
            if ( !createMissingComponents )
                return false;

            pNextGroup = m_pCurrentGroup->AddSubgroup(aParts[n]);
        }

        m_pCurrentGroup = pNextGroup;
    }

    // recombine path parts in one variable
    m_strPath.clear();
    for ( size_t n = 0; n < aParts.size(); n++ )
    {
        m_strPath += fmt::format("{}{}", wxCONFIG_PATH_SEPARATOR, aParts[n]);
    }

    return true;
}

void wxFileConfig::SetPath(const std::string& strPath)
{
    DoSetPath(strPath, true /* create missing path components */);
}

const std::string& wxFileConfig::GetPath() const
{
    return m_strPath;
}

// ----------------------------------------------------------------------------
// enumeration
// ----------------------------------------------------------------------------

bool wxFileConfig::GetFirstGroup(std::string& str, long& lIndex) const
{
    lIndex = 0;
    return GetNextGroup(str, lIndex);
}

bool wxFileConfig::GetNextGroup (std::string& str, long& lIndex) const
{
    if ( size_t(lIndex) < m_pCurrentGroup->Groups().GetCount() ) {
        str = m_pCurrentGroup->Groups()[(size_t)lIndex++]->Name();
        return true;
    }
    else
        return false;
}

bool wxFileConfig::GetFirstEntry(std::string& str, long& lIndex) const
{
    lIndex = 0;
    return GetNextEntry(str, lIndex);
}

bool wxFileConfig::GetNextEntry (std::string& str, long& lIndex) const
{
    if ( size_t(lIndex) < m_pCurrentGroup->Entries().GetCount() ) {
        str = m_pCurrentGroup->Entries()[(size_t)lIndex++]->Name();
        return true;
    }
    else
        return false;
}

size_t wxFileConfig::GetNumberOfEntries(bool bRecursive) const
{
    size_t n = m_pCurrentGroup->Entries().GetCount();
    if ( bRecursive ) {
        wxFileConfig * const self = const_cast<wxFileConfig *>(this);

        wxFileConfigGroup *pOldCurrentGroup = m_pCurrentGroup;
        const size_t nSubgroups = m_pCurrentGroup->Groups().GetCount();
        for ( size_t nGroup = 0; nGroup < nSubgroups; nGroup++ ) {
            self->m_pCurrentGroup = m_pCurrentGroup->Groups()[nGroup];
            n += GetNumberOfEntries(true);
            self->m_pCurrentGroup = pOldCurrentGroup;
        }
    }

    return n;
}

size_t wxFileConfig::GetNumberOfGroups(bool bRecursive) const
{
    size_t n = m_pCurrentGroup->Groups().GetCount();
    if ( bRecursive ) {
        wxFileConfig * const self = const_cast<wxFileConfig *>(this);

        wxFileConfigGroup *pOldCurrentGroup = m_pCurrentGroup;
        const size_t nSubgroups = m_pCurrentGroup->Groups().GetCount();
        for ( size_t nGroup = 0; nGroup < nSubgroups; nGroup++ ) {
            self->m_pCurrentGroup = m_pCurrentGroup->Groups()[nGroup];
            n += GetNumberOfGroups(true);
            self->m_pCurrentGroup = pOldCurrentGroup;
        }
    }

    return n;
}

// ----------------------------------------------------------------------------
// tests for existence
// ----------------------------------------------------------------------------

bool wxFileConfig::HasGroup(const std::string& strName) const
{
    // special case: DoSetPath("") does work as it's equivalent to DoSetPath("/")
    // but there is no group with empty name so treat this separately
    if ( strName.empty() )
        return false;

    const std::string pathOld = GetPath();

    wxFileConfig *self = const_cast<wxFileConfig *>(this);
    const bool
        rc = self->DoSetPath(strName, false /* don't create missing components */);

    self->SetPath(pathOld);

    return rc;
}

bool wxFileConfig::HasEntry(const std::string& entry) const
{
    // path is the part before the last "/"
    std::string path = wx::utils::BeforeLast(entry, wxCONFIG_PATH_SEPARATOR);

    // except in the special case of "/keyname" when there is nothing before "/"
    if ( path.empty() && *entry.c_str() == wxCONFIG_PATH_SEPARATOR )
    {
        path = wxCONFIG_PATH_SEPARATOR;
    }

    // change to the path of the entry if necessary and remember the old path
    // to restore it later
    std::string pathOld;
    wxFileConfig * const self = const_cast<wxFileConfig *>(this);
    if ( !path.empty() )
    {
        pathOld = GetPath();
        if ( pathOld.empty() )
            pathOld = wxCONFIG_PATH_SEPARATOR;

        if ( !self->DoSetPath(path, false /* don't create if doesn't exist */) )
        {
            return false;
        }
    }

    // check if the entry exists in this group
    const bool exists = m_pCurrentGroup->FindEntry(
                            wx::utils::AfterLast(entry, wxCONFIG_PATH_SEPARATOR)) != nullptr;

    // restore the old path if we changed it above
    if ( !pathOld.empty() )
    {
        self->SetPath(pathOld);
    }

    return exists;
}

// ----------------------------------------------------------------------------
// read/write values
// ----------------------------------------------------------------------------

bool wxFileConfig::DoReadString(const std::string& key, std::string* pStr) const
{
    wxConfigPathChanger path(this, key);

    wxFileConfigEntry *pEntry = m_pCurrentGroup->FindEntry(path.Name());
    if (pEntry == nullptr) {
        return false;
    }

    *pStr = pEntry->Value();

    return true;
}

bool wxFileConfig::DoReadLong(const std::string& key, long *pl) const
{
    std::string str;
    if ( !Read(key, &str) )
        return false;

    // extra spaces shouldn't prevent us from reading numeric values
    wx::utils::TrimTrailingSpace(str);

    auto [p, ec] = std::from_chars(str.data(), str.data() + str.size(), *pl);

    return ec == std::errc();
}

#if wxUSE_BASE64

bool wxFileConfig::DoReadBinary(const std::string& key, wxMemoryBuffer* buf) const
{
    wxCHECK_MSG( buf, false, "NULL buffer" );

    std::string str;
    if ( !Read(key, &str) )
        return false;

    *buf = wxBase64Decode(str);
    return true;
}

#endif // wxUSE_BASE64

bool wxFileConfig::DoWriteString(const std::string& key, const std::string& szValue)
{
    wxConfigPathChanger     path(this, key);
    std::string                strName = path.Name();

    wxLogTrace( FILECONF_TRACE_MASK,
                "  Writing String '%s' = '%s' to Group '%s'",
                strName.c_str(),
                szValue.c_str(),
                GetPath().c_str() );

    if ( strName.empty() )
    {
            // setting the value of a group is an error

        wxASSERT_MSG( szValue.empty(), "can't set value of a group!" );

            // ... except if it's empty in which case it's a way to force it's creation

        wxLogTrace( FILECONF_TRACE_MASK,
                    "  Creating group %s",
                    m_pCurrentGroup->Name().c_str() );

        SetDirty();

        // this will add a line for this group if it didn't have it before (or
        // do nothing for the root but it's ok as it always exists anyhow)
        std::ignore = m_pCurrentGroup->GetGroupLine();
    }
    else
    {
        // writing an entry check that the name is reasonable
        if ( strName[0u] == wxCONFIG_IMMUTABLE_PREFIX )
        {
            wxLogError( _("Config entry name cannot start with '%c'."),
                        wxCONFIG_IMMUTABLE_PREFIX);
            return false;
        }

        wxFileConfigEntry   *pEntry = m_pCurrentGroup->FindEntry(strName);

        if ( pEntry == nullptr )
        {
            wxLogTrace( FILECONF_TRACE_MASK,
                        "  Adding Entry %s",
                        strName.c_str() );
            pEntry = m_pCurrentGroup->AddEntry(strName);
        }

        wxLogTrace( FILECONF_TRACE_MASK,
                    "  Setting value %s",
                    szValue.c_str() );
        pEntry->SetValue(szValue);

        SetDirty();
    }

    return true;
}

bool wxFileConfig::DoWriteLong(const std::string& key, long lValue)
{
  return Write(key, fmt::format("%ld", lValue));
}

#if wxUSE_BASE64

bool wxFileConfig::DoWriteBinary(const std::string& key, const wxMemoryBuffer& buf)
{
  return Write(key, wxBase64Encode(buf).ToStdString());
}

#endif // wxUSE_BASE64

bool wxFileConfig::Flush(bool /* bCurrentOnly */)
{
  if ( !IsDirty() || m_fnLocalFile.GetFullPath().empty() )
    return true;

  // set the umask if needed
  wxCHANGE_UMASK(m_umask);

  wxTempFile file(m_fnLocalFile.GetFullPath());

  if ( !file.IsOpened() )
  {
    wxLogError(_("can't open user configuration file."));
    return false;
  }

  // write all strings to file
  std::string filetext;
  filetext.reserve(4096);
  for ( wxFileConfigLineList *p = m_linesHead; p != nullptr; p = p->Next() )
  {
    filetext += p->Text() + wxTextFile::GetEOL();
  }

  if ( !file.Write(filetext, *m_conv) )
  {
    wxLogError(_("can't write user configuration file."));
    return false;
  }

  if ( !file.Commit() )
  {
      wxLogError(_("Failed to update user configuration file."));

      return false;
  }

  ResetDirty();

  return true;
}

#if wxUSE_STREAMS

bool wxFileConfig::Save(wxOutputStream& os, const wxMBConv& conv)
{
    // save unconditionally, even if not dirty
    for ( wxFileConfigLineList *p = m_linesHead; p != nullptr; p = p->Next() )
    {
        std::string line = p->Text();
        line += wxTextFile::GetEOL();
        
        // FIXME: Use boost::nowide to write here?
        if ( !os.Write(line.data(), line.length()) )
        {
            wxLogError(_("Error saving user configuration data."));

            return false;
        }
    }

    ResetDirty();

    return true;
}

#endif // wxUSE_STREAMS

// ----------------------------------------------------------------------------
// renaming groups/entries
// ----------------------------------------------------------------------------

bool wxFileConfig::RenameEntry(const std::string& oldName,
                               const std::string& newName)
{
    wxASSERT_MSG( oldName.find(wxCONFIG_PATH_SEPARATOR) == std::string::npos,
                   "RenameEntry(): paths are not supported" );

    // check that the entry exists
    wxFileConfigEntry *oldEntry = m_pCurrentGroup->FindEntry(oldName);
    if ( !oldEntry )
        return false;

    // check that the new entry doesn't already exist
    if ( m_pCurrentGroup->FindEntry(newName) )
        return false;

    // delete the old entry, create the new one
    std::string value = oldEntry->Value();
    if ( !m_pCurrentGroup->DeleteEntry(oldName) )
        return false;

    SetDirty();

    wxFileConfigEntry *newEntry = m_pCurrentGroup->AddEntry(newName);
    newEntry->SetValue(value);

    return true;
}

bool wxFileConfig::RenameGroup(const std::string& oldName,
                               const std::string& newName)
{
    // check that the group exists
    wxFileConfigGroup *group = m_pCurrentGroup->FindSubgroup(oldName);
    if ( !group )
        return false;

    // check that the new group doesn't already exist
    if ( m_pCurrentGroup->FindSubgroup(newName) )
        return false;

    group->Rename(newName);

    SetDirty();

    return true;
}

// ----------------------------------------------------------------------------
// delete groups/entries
// ----------------------------------------------------------------------------

bool wxFileConfig::DeleteEntry(const std::string& key, bool bGroupIfEmptyAlso)
{
  wxConfigPathChanger path(this, key);

  if ( !m_pCurrentGroup->DeleteEntry(path.Name()) )
    return false;

  SetDirty();

  if ( bGroupIfEmptyAlso && m_pCurrentGroup->IsEmpty() ) {
    if ( m_pCurrentGroup != m_pRootGroup ) {
      wxFileConfigGroup *pGroup = m_pCurrentGroup;
      SetPath("..");  // changes m_pCurrentGroup!
      m_pCurrentGroup->DeleteSubgroupByName(pGroup->Name());
    }
    //else: never delete the root group
  }

  return true;
}

bool wxFileConfig::DeleteGroup(const std::string& key)
{
  wxConfigPathChanger path(this, RemoveTrailingSeparator(key));

  if ( !m_pCurrentGroup->DeleteSubgroupByName(path.Name()) )
      return false;

  path.UpdateIfDeleted();

  SetDirty();

  return true;
}

bool wxFileConfig::DeleteAll()
{
  CleanUp();

  if ( m_fnLocalFile.IsOk() )
  {
      if ( m_fnLocalFile.FileExists() &&
           !wxRemoveFile(m_fnLocalFile.GetFullPath()) )
      {
          wxLogSysError(_("can't delete user configuration file '%s'"),
                        m_fnLocalFile.GetFullPath().c_str());
          return false;
      }
  }

  
    m_pCurrentGroup =
    m_pRootGroup    = new wxFileConfigGroup(nullptr, "", this);

    m_linesHead =
    m_linesTail = nullptr;

    // It's not an error if (one of the) file(s) doesn't exist.

    // parse the global file
    if ( m_fnGlobalFile.IsOk() && m_fnGlobalFile.FileExists() )
    {
        wxTextFile fileGlobal(m_fnGlobalFile.GetFullPath());

        if ( fileGlobal.Open(*m_conv/*ignored in ANSI build*/) )
        {
            Parse(fileGlobal, false /* global */);
            SetRootPath();
        }
        else
        {
            wxLogWarning(_("can't open global configuration file '%s'."), m_fnGlobalFile.GetFullPath().c_str());
        }
    }

    // parse the local file
    if ( m_fnLocalFile.IsOk() && m_fnLocalFile.FileExists() )
    {
        wxTextFile fileLocal(m_fnLocalFile.GetFullPath());
        if ( fileLocal.Open(*m_conv/*ignored in ANSI build*/) )
        {
            Parse(fileLocal, true /* local */);
            SetRootPath();
        }
        else
        {
            const std::string path = m_fnLocalFile.GetFullPath();
            wxLogWarning(_("can't open user configuration file '%s'."),
                         path.c_str());

            if ( m_fnLocalFile.FileExists() )
            {
                wxLogWarning(_("Changes won't be saved to avoid overwriting the existing file \"%s\""),
                             path.c_str());
                m_fnLocalFile.Clear();
            }
        }
    }

    m_isDirty = false;
    m_autosave = true;


  return true;
}

// ----------------------------------------------------------------------------
// linked list functions
// ----------------------------------------------------------------------------

    // append a new line to the end of the list

wxFileConfigLineList *wxFileConfig::LineListAppend(const std::string& str)
{
    wxLogTrace( FILECONF_TRACE_MASK,
                "    ** Adding Line '%s'",
                str.c_str() );
    wxLogTrace( FILECONF_TRACE_MASK,
                "        head: %s",
                ((m_linesHead) ? m_linesHead->Text()
                               : std::string{}) );
    wxLogTrace( FILECONF_TRACE_MASK,
                "        tail: %s",
                ((m_linesTail) ? m_linesTail->Text()
                               : std::string{}) );

    wxFileConfigLineList *pLine = new wxFileConfigLineList(str);

    if ( m_linesTail == nullptr )
    {
        // list is empty
        m_linesHead = pLine;
    }
    else
    {
        // adjust pointers
        m_linesTail->SetNext(pLine);
        pLine->SetPrev(m_linesTail);
    }

    m_linesTail = pLine;

    wxLogTrace( FILECONF_TRACE_MASK,
                "        head: %s",
                ((m_linesHead) ? m_linesHead->Text()
                               : std::string{}) );
    wxLogTrace( FILECONF_TRACE_MASK,
                "        tail: %s",
                ((m_linesTail) ? m_linesTail->Text()
                               : std::string{}) );

    return m_linesTail;
}

// insert a new line after the given one or in the very beginning if !pLine
wxFileConfigLineList *wxFileConfig::LineListInsert(const std::string& str,
                                                   wxFileConfigLineList *pLine)
{
    wxLogTrace( FILECONF_TRACE_MASK,
                "    ** Inserting Line '%s' after '%s'",
                str.c_str(),
                ((pLine) ? pLine->Text()
                         : std::string{}) );
    wxLogTrace( FILECONF_TRACE_MASK,
                "        head: %s",
                ((m_linesHead) ? m_linesHead->Text()
                               : std::string{}) );
    wxLogTrace( FILECONF_TRACE_MASK,
                "        tail: %s",
                ((m_linesTail) ? m_linesTail->Text()
                               : std::string{}) );

    if ( pLine == m_linesTail )
        return LineListAppend(str);

    wxFileConfigLineList *pNewLine = new wxFileConfigLineList(str);
    if ( pLine == nullptr )
    {
        // prepend to the list
        pNewLine->SetNext(m_linesHead);
        m_linesHead->SetPrev(pNewLine);
        m_linesHead = pNewLine;
    }
    else
    {
        // insert before pLine
        wxFileConfigLineList *pNext = pLine->Next();
        pNewLine->SetNext(pNext);
        pNewLine->SetPrev(pLine);
        pNext->SetPrev(pNewLine);
        pLine->SetNext(pNewLine);
    }

    wxLogTrace( FILECONF_TRACE_MASK,
                "        head: %s",
                ((m_linesHead) ? m_linesHead->Text()
                               : std::string{}) );
    wxLogTrace( FILECONF_TRACE_MASK,
                "        tail: %s",
                ((m_linesTail) ? m_linesTail->Text()
                               : std::string{}) );

    return pNewLine;
}

void wxFileConfig::LineListRemove(wxFileConfigLineList *pLine)
{
    wxLogTrace( FILECONF_TRACE_MASK,
                "    ** Removing Line '%s'",
                pLine->Text().c_str() );
    wxLogTrace( FILECONF_TRACE_MASK,
                "        head: %s",
                ((m_linesHead) ? m_linesHead->Text()
                               : std::string{}) );
    wxLogTrace( FILECONF_TRACE_MASK,
                "        tail: %s",
                ((m_linesTail) ? m_linesTail->Text()
                               : std::string{}) );

    wxFileConfigLineList    *pPrev = pLine->Prev(),
                            *pNext = pLine->Next();

        // first entry?

    if ( pPrev == nullptr )
        m_linesHead = pNext;
    else
        pPrev->SetNext(pNext);

        // last entry?

    if ( pNext == nullptr )
        m_linesTail = pPrev;
    else
        pNext->SetPrev(pPrev);

    wxLogTrace( FILECONF_TRACE_MASK,
                "        head: %s",
                ((m_linesHead) ? m_linesHead->Text()
                               : std::string{}) );
    wxLogTrace( FILECONF_TRACE_MASK,
                "        tail: %s",
                ((m_linesTail) ? m_linesTail->Text()
                               : std::string{}) );

    delete pLine;
}

bool wxFileConfig::LineListIsEmpty()
{
    return m_linesHead == nullptr;
}

// ============================================================================
// wxFileConfig::wxFileConfigGroup
// ============================================================================

// ----------------------------------------------------------------------------
// ctor/dtor
// ----------------------------------------------------------------------------

// ctor
wxFileConfigGroup::wxFileConfigGroup(wxFileConfigGroup *pParent,
                                       const std::string& strName,
                                       wxFileConfig *pConfig)
                         : m_aEntries(CompareEntries),
                           m_aSubgroups(CompareGroups),
                           m_strName(strName),
                           m_pConfig(pConfig),
                           m_pParent(pParent)
{
}

// dtor deletes all children
wxFileConfigGroup::~wxFileConfigGroup()
{
  // entries
  for ( size_t n = 0; n < m_aEntries.GetCount(); n++ )
    delete m_aEntries[n];

  // subgroups
  for ( size_t n = 0; n < m_aSubgroups.GetCount(); n++ )
    delete m_aSubgroups[n];
}

// ----------------------------------------------------------------------------
// line
// ----------------------------------------------------------------------------

void wxFileConfigGroup::SetLine(wxFileConfigLineList *pLine)
{
    // for a normal (i.e. not root) group this method shouldn't be called twice
    // unless we are resetting the line
    wxASSERT_MSG( !m_pParent || !m_pLine || !pLine,
                   "changing line for a non-root group?" );

    m_pLine = pLine;
}

/*
  This is a bit complicated, so let me explain it in details. All lines that
  were read from the local file (the only one we will ever modify) are stored
  in a (doubly) linked list. Our problem is to know at which position in this
  list should we insert the new entries/subgroups. To solve it we keep three
  variables for each group: m_pLine, m_pLastEntry and m_pLastGroup.

  m_pLine points to the line containing "[group_name]"
  m_pLastEntry points to the last entry of this group in the local file.
  m_pLastGroup                   subgroup

  Initially, they're NULL all three. When the group (an entry/subgroup) is read
  from the local file, the corresponding variable is set. However, if the group
  was read from the global file and then modified or created by the application
  these variables are still NULL and we need to create the corresponding lines.
  See the following functions (and comments preceding them) for the details of
  how we do it.

  Also, when our last entry/group are deleted we need to find the new last
  element - the code in DeleteEntry/Subgroup does this by backtracking the list
  of lines until it either founds an entry/subgroup (and this is the new last
  element) or the m_pLine of the group, in which case there are no more entries
  (or subgroups) left and m_pLast<element> becomes NULL.

  NB: This last problem could be avoided for entries if we added new entries
      immediately after m_pLine, but in this case the entries would appear
      backwards in the config file (OTOH, it's not that important) and as we
      would still need to do it for the subgroups the code wouldn't have been
      significantly less complicated.
*/

// Return the line which contains "[our name]". If we're still not in the list,
// add our line to it immediately after the last line of our parent group if we
// have it or in the very beginning if we're the root group.
wxFileConfigLineList *wxFileConfigGroup::GetGroupLine()
{
    wxLogTrace( FILECONF_TRACE_MASK,
                "  GetGroupLine() for Group '%s'",
                Name().c_str() );

    if ( !m_pLine )
    {
        wxLogTrace( FILECONF_TRACE_MASK,
                    "    Getting Line item pointer" );

        wxFileConfigGroup   *pParent = Parent();

        // this group wasn't present in local config file, add it now
        if ( pParent )
        {
            wxLogTrace( FILECONF_TRACE_MASK,
                        "    checking parent '%s'",
                        pParent->Name().c_str() );

            // Take substr(1) because we don't want to start with '/'
            std::string    strFullName = fmt::format("[{}]", FilterOutEntryName(GetFullName().substr(1)));

            m_pLine = m_pConfig->LineListInsert(strFullName,
                                                pParent->GetLastGroupLine());
            pParent->SetLastGroup(this);  // we're surely after all the others
        }
        //else: this is the root group and so we return NULL because we don't
        //      have any group line
    }

    return m_pLine;
}

// Return the last line belonging to the subgroups of this group (after which
// we can add a new subgroup), if we don't have any subgroups or entries our
// last line is the group line (m_pLine) itself.
wxFileConfigLineList *wxFileConfigGroup::GetLastGroupLine()
{
    // if we have any subgroups, our last line is the last line of the last
    // subgroup
    if ( m_pLastGroup )
    {
        wxFileConfigLineList *pLine = m_pLastGroup->GetLastGroupLine();

        wxASSERT_MSG( pLine, "last group must have !NULL associated line" );

        return pLine;
    }

    // no subgroups, so the last line is the line of thelast entry (if any)
    return GetLastEntryLine();
}

// return the last line belonging to the entries of this group (after which
// we can add a new entry), if we don't have any entries we will add the new
// one immediately after the group line itself.
wxFileConfigLineList *wxFileConfigGroup::GetLastEntryLine()
{
    wxLogTrace( FILECONF_TRACE_MASK,
                "  GetLastEntryLine() for Group '%s'",
                Name().c_str() );

    if ( m_pLastEntry )
    {
        wxFileConfigLineList    *pLine = m_pLastEntry->GetLine();

        wxASSERT_MSG( pLine, "last entry must have !NULL associated line" );

        return pLine;
    }

    // no entries: insert after the group header, if any
    return GetGroupLine();
}

void wxFileConfigGroup::SetLastEntry(wxFileConfigEntry *pEntry)
{
    m_pLastEntry = pEntry;

    if ( !m_pLine )
    {
        // the only situation in which a group without its own line can have
        // an entry is when the first entry is added to the initially empty
        // root pseudo-group
        wxASSERT_MSG( !m_pParent, "unexpected for non root group" );

        // let the group know that it does have a line in the file now
        m_pLine = pEntry->GetLine();
    }
}

// ----------------------------------------------------------------------------
// group name
// ----------------------------------------------------------------------------

void wxFileConfigGroup::UpdateGroupAndSubgroupsLines()
{
    // update the line of this group
    wxFileConfigLineList *line = GetGroupLine();
    wxCHECK_RET( line, "a non root group must have a corresponding line!" );

    // +1: skip the leading '/'
    line->SetText(fmt::format("[%s]", GetFullName().substr(1)));


    // also update all subgroups as they have this groups name in their lines
    const size_t nCount = m_aSubgroups.GetCount();
    for ( size_t n = 0; n < nCount; n++ )
    {
        m_aSubgroups[n]->UpdateGroupAndSubgroupsLines();
    }
}

void wxFileConfigGroup::Rename(const std::string& newName)
{
    wxCHECK_RET( m_pParent, "the root group can't be renamed" );

    if ( newName == m_strName )
        return;

    // we need to remove the group from the parent and it back under the new
    // name to keep the parents array of subgroups alphabetically sorted
    m_pParent->m_aSubgroups.Remove(this);

    m_strName = newName;

    m_pParent->m_aSubgroups.Add(this);

    // update the group lines recursively
    UpdateGroupAndSubgroupsLines();
}

std::string wxFileConfigGroup::GetFullName() const
{
    std::string fullname;
    if ( Parent() )
        fullname = Parent()->GetFullName() + wxCONFIG_PATH_SEPARATOR + Name();

    return fullname;
}

// ----------------------------------------------------------------------------
// find an item
// ----------------------------------------------------------------------------

// use binary search because the array is sorted
wxFileConfigEntry *
wxFileConfigGroup::FindEntry(const std::string& name) const
{
  size_t
       lo = 0,
       hi = m_aEntries.GetCount();
  wxFileConfigEntry *pEntry;

  while ( lo < hi ) {
    size_t i = (lo + hi)/2;
    pEntry = m_aEntries[i];

    #if wxCONFIG_CASE_SENSITIVE
      int res = pEntry->Name().compare(name);
    #else
      int res = wx::utils::CmpNoCase(pEntry->Name(), name);
    #endif

    if ( res > 0 )
      hi = i;
    else if ( res < 0 )
      lo = i + 1;
    else
      return pEntry;
  }

  return nullptr;
}

wxFileConfigGroup *
wxFileConfigGroup::FindSubgroup(const std::string& name) const
{
  size_t
       lo = 0,
       hi = m_aSubgroups.GetCount();
  wxFileConfigGroup *pGroup;

  while ( lo < hi ) {
    size_t i = (lo + hi)/2;
    pGroup = m_aSubgroups[i];

    #if wxCONFIG_CASE_SENSITIVE
    int res = pGroup->Name().compare(name);
    #else
    int res = wx::utils::CmpNoCase(pGroup->Name(), name);
    #endif

    if ( res > 0 )
      hi = i;
    else if ( res < 0 )
      lo = i + 1;
    else
      return pGroup;
  }

  return nullptr;
}

// ----------------------------------------------------------------------------
// create a new item
// ----------------------------------------------------------------------------

// create a new entry and add it to the current group
wxFileConfigEntry *wxFileConfigGroup::AddEntry(const std::string& strName, int nLine)
{
    wxASSERT( FindEntry(strName) == nullptr );

    wxFileConfigEntry   *pEntry = new wxFileConfigEntry(this, strName, nLine);

    m_aEntries.Add(pEntry);
    return pEntry;
}

// create a new group and add it to the current group
wxFileConfigGroup *wxFileConfigGroup::AddSubgroup(const std::string& strName)
{
    wxASSERT( FindSubgroup(strName) == nullptr );

    wxFileConfigGroup   *pGroup = new wxFileConfigGroup(this, strName, m_pConfig);

    m_aSubgroups.Add(pGroup);
    return pGroup;
}

// ----------------------------------------------------------------------------
// delete an item
// ----------------------------------------------------------------------------

/*
  The delete operations are _very_ slow if we delete the last item of this
  group (see comments before GetXXXLineXXX functions for more details),
  so it's much better to start with the first entry/group if we want to
  delete several of them.
 */

bool wxFileConfigGroup::DeleteSubgroupByName(const std::string& name)
{
    wxFileConfigGroup * const pGroup = FindSubgroup(name);

    return pGroup ? DeleteSubgroup(pGroup) : false;
}

// Delete the subgroup and remove all references to it from
// other data structures.
bool wxFileConfigGroup::DeleteSubgroup(wxFileConfigGroup *pGroup)
{
    wxCHECK_MSG( pGroup, false, "deleting non existing group?" );

    wxLogTrace( FILECONF_TRACE_MASK,
                "Deleting group '%s' from '%s'",
                pGroup->Name().c_str(),
                Name().c_str() );

    wxLogTrace( FILECONF_TRACE_MASK,
                "  (m_pLine) = prev: %p, this %p, next %p",
                m_pLine ? static_cast<void*>(m_pLine->Prev()) : nullptr,
                m_pLine,
                m_pLine ? static_cast<void*>(m_pLine->Next()) : nullptr );
    wxLogTrace( FILECONF_TRACE_MASK,
                "  text: '%s'",
                m_pLine ? m_pLine->Text()
                        : std::string{} );

    // delete all entries...
    size_t nCount = pGroup->m_aEntries.GetCount();

    wxLogTrace(FILECONF_TRACE_MASK,
               "Removing %lu entries", (unsigned long)nCount );

    for ( size_t nEntry = 0; nEntry < nCount; nEntry++ )
    {
        wxFileConfigLineList *pLine = pGroup->m_aEntries[nEntry]->GetLine();

        if ( pLine )
        {
            wxLogTrace( FILECONF_TRACE_MASK,
                        "    '%s'",
                        pLine->Text().c_str() );
            m_pConfig->LineListRemove(pLine);
        }
    }

    // ...and subgroups of this subgroup
    nCount = pGroup->m_aSubgroups.GetCount();

    wxLogTrace( FILECONF_TRACE_MASK,
                "Removing %lu subgroups", (unsigned long)nCount );

    for ( size_t nGroup = 0; nGroup < nCount; nGroup++ )
    {
        pGroup->DeleteSubgroup(pGroup->m_aSubgroups[0]);
    }

    // and then finally the group itself
    wxFileConfigLineList *pLine = pGroup->m_pLine;
    if ( pLine )
    {
        wxLogTrace( FILECONF_TRACE_MASK,
                    "  Removing line for group '%s' : '%s'",
                    pGroup->Name().c_str(),
                    pLine->Text().c_str() );
        wxLogTrace( FILECONF_TRACE_MASK,
                    "  Removing from group '%s' : '%s'",
                    Name().c_str(),
                    ((m_pLine) ? m_pLine->Text()
                               : std::string{}) );

        // notice that we may do this test inside the previous "if"
        // because the last entry's line is surely !NULL
        if ( pGroup == m_pLastGroup )
        {
            wxLogTrace( FILECONF_TRACE_MASK,
                        "  Removing last group" );

            // our last entry is being deleted, so find the last one which
            // stays by going back until we find a subgroup or reach the
            // group line
            const size_t nSubgroups = m_aSubgroups.GetCount();

            m_pLastGroup = nullptr;
            for ( wxFileConfigLineList *pl = pLine->Prev();
                  pl && !m_pLastGroup;
                  pl = pl->Prev() )
            {
                // does this line belong to our subgroup?
                for ( size_t n = 0; n < nSubgroups; n++ )
                {
                    // do _not_ call GetGroupLine! we don't want to add it to
                    // the local file if it's not already there
                    if ( m_aSubgroups[n]->m_pLine == pl )
                    {
                        m_pLastGroup = m_aSubgroups[n];
                        break;
                    }
                }

                if ( pl == m_pLine )
                    break;
            }
        }

        m_pConfig->LineListRemove(pLine);
    }
    else
    {
        wxLogTrace( FILECONF_TRACE_MASK,
                    "  No line entry for Group '%s'?",
                    pGroup->Name().c_str() );
    }

    m_aSubgroups.Remove(pGroup);
    delete pGroup;

    return true;
}

bool wxFileConfigGroup::DeleteEntry(const std::string& name)
{
  wxFileConfigEntry *pEntry = FindEntry(name);
  if ( !pEntry )
  {
      // entry doesn't exist, nothing to do
      return false;
  }

  wxFileConfigLineList *pLine = pEntry->GetLine();
  if ( pLine != nullptr ) {
    // notice that we may do this test inside the previous "if" because the
    // last entry's line is surely !NULL
    if ( pEntry == m_pLastEntry ) {
      // our last entry is being deleted - find the last one which stays
      wxASSERT( m_pLine != nullptr );  // if we have an entry with !NULL pLine...

      // find the previous entry (if any)
      wxFileConfigEntry *pNewLast = nullptr;
      const wxFileConfigLineList * const
        pNewLastLine = m_pLastEntry->GetLine()->Prev();
      const size_t nEntries = m_aEntries.GetCount();
      for ( size_t n = 0; n < nEntries; n++ ) {
        if ( m_aEntries[n]->GetLine() == pNewLastLine ) {
          pNewLast = m_aEntries[n];
          break;
        }
      }

      // pNewLast can be NULL here -- it's ok and can happen if we have no
      // entries left
      m_pLastEntry = pNewLast;

      // For the root group only, we could be removing the first group line
      // here, so update m_pLine to avoid keeping a dangling pointer.
      if ( pLine == m_pLine )
          SetLine(nullptr);
    }

    m_pConfig->LineListRemove(pLine);
  }

  m_aEntries.Remove(pEntry);
  delete pEntry;

  return true;
}

// ============================================================================
// wxFileConfig::wxFileConfigEntry
// ============================================================================

// ----------------------------------------------------------------------------
// ctor
// ----------------------------------------------------------------------------
wxFileConfigEntry::wxFileConfigEntry(wxFileConfigGroup *pParent,
                                       const std::string& strName,
                                       int nLine)
                         : m_strName(strName)
{
  wxASSERT( !strName.empty() );

  m_pParent = pParent;
  m_nLine   = nLine;
  m_pLine   = nullptr;

  m_bHasValue = false;

  m_bImmutable = strName[0] == wxCONFIG_IMMUTABLE_PREFIX;
  if ( m_bImmutable )
    m_strName.erase(0, 1);  // remove first character
}

// ----------------------------------------------------------------------------
// set value
// ----------------------------------------------------------------------------

void wxFileConfigEntry::SetLine(wxFileConfigLineList *pLine)
{
  if ( m_pLine != nullptr ) {
    wxLogWarning(_("entry '%s' appears more than once in group '%s'"),
                 Name().c_str(), m_pParent->GetFullName().c_str());
  }

  m_pLine = pLine;
  Group()->SetLastEntry(this);
}

// second parameter is false if we read the value from file and prevents the
// entry from being marked as 'dirty'
void wxFileConfigEntry::SetValue(const std::string& strValue, bool bUser)
{
    if ( bUser && IsImmutable() )
    {
        wxLogWarning( _("attempt to change immutable key '%s' ignored."),
                      Name().c_str());
        return;
    }

    // do nothing if it's the same value: but don't test for it if m_bHasValue
    // hadn't been set yet or we'd never write empty values to the file
    if ( m_bHasValue && strValue == m_strValue )
        return;

    m_bHasValue = true;
    m_strValue = strValue;

    if ( bUser )
    {
        std::string strValFiltered;

        if ( Group()->Config()->GetStyle() & wxCONFIG_USE_NO_ESCAPE_CHARACTERS )
        {
            strValFiltered = strValue;
        }
        else {
            strValFiltered = FilterOutValue(strValue);
        }

        std::string    strLine = fmt::format("{}={}", FilterOutEntryName(m_strName), strValFiltered);

        if ( m_pLine )
        {
            // entry was read from the local config file, just modify the line
            m_pLine->SetText(strLine);
        }
        else // this entry didn't exist in the local file
        {
            // add a new line to the file: note that line returned by
            // GetLastEntryLine() may be NULL if we're in the root group and it
            // doesn't have any entries yet, but this is ok as passing NULL
            // line to LineListInsert() means to prepend new line to the list
            wxFileConfigLineList *line = Group()->GetLastEntryLine();
            m_pLine = Group()->Config()->LineListInsert(strLine, line);

            Group()->SetLastEntry(this);
        }
    }
}

// ============================================================================
// global functions
// ============================================================================

// ----------------------------------------------------------------------------
// compare functions for array sorting
// ----------------------------------------------------------------------------

int CompareEntries(wxFileConfigEntry *p1, wxFileConfigEntry *p2)
{
#if wxCONFIG_CASE_SENSITIVE
    return p1->Name().compare(p2->Name());
#else
    return wx::utils::CmpNoCase(p1->Name(), p2->Name());
#endif
}

int CompareGroups(wxFileConfigGroup *p1, wxFileConfigGroup *p2)
{
#if wxCONFIG_CASE_SENSITIVE
    return p1->Name().compare(p2->Name());
#else
    return wx::utils::CmpNoCase(p1->Name(), p2->Name());
#endif
}

// ----------------------------------------------------------------------------
// filter functions
// ----------------------------------------------------------------------------

// undo FilterOutValue
static std::string FilterInValue(const std::string& str)
{
    std::string strResult;
    if ( str.empty() )
        return strResult;

    strResult.reserve(str.length());

    std::string::const_iterator i = str.begin();
    const bool bQuoted = *i == '"';
    if ( bQuoted )
        ++i;

    for ( const std::string::const_iterator end = str.end(); i != end; ++i )
    {
        if ( *i == '\\' )
        {
            if ( ++i == end )
            {
                wxLogWarning(_("trailing backslash ignored in '%s'"), str.c_str());
                break;
            }

            switch (*i)
            {
                case 'n':
                    strResult += '\n';
                    break;

                case 'r':
                    strResult += '\r';
                    break;

                case 't':
                    strResult += '\t';
                    break;

                case '\\':
                    strResult += '\\';
                    break;

                case '"':
                    strResult += '"';
                    break;
            }
        }
        else // not a backslash
        {
            if ( *i != '"' || !bQuoted )
            {
                strResult += *i;
            }
            else if ( i != end - 1 )
            {
                wxLogWarning(_("unexpected \" at position %d in '%s'."),
                             i - str.begin(), str.c_str());
            }
            //else: it's the last quote of a quoted string, ok
        }
    }

    return strResult;
}

// quote the string before writing it to file
static std::string FilterOutValue(const std::string& str)
{
   if ( str.empty() )
      return str;

  std::string strResult;
  strResult.reserve(str.length());

  // quoting is necessary to preserve spaces in the beginning of the string
  const bool bQuote = wxIsspace(str[0]) || str[0] == '"';

  if ( bQuote )
    strResult += '"';

  wxChar c;
  for ( size_t n = 0; n != str.length(); n++ ) {
    switch ( str[n] ) {
      case '\n':
        c = 'n';
        break;

      case '\r':
        c = 'r';
        break;

      case '\t':
        c = 't';
        break;

      case '\\':
        c = '\\';
        break;

      case '"':
        if ( bQuote ) {
          c = '"';
          break;
        }
        [[fallthrough]];

      default:
        strResult += str[n];
        continue;   // nothing special to do
    }

    // we get here only for special characters
    strResult += fmt::format("\\{}", c);
  }

  if ( bQuote )
    strResult += '"';

  return strResult;
}

// undo FilterOutEntryName
static std::string FilterInEntryName(const std::string& str)
{
  std::string strResult;
  strResult.resize(str.length());

  for ( const char *pc = str.c_str(); *pc != '\0'; pc++ ) {
    if ( *pc == '\\' ) {
      // we need to test it here or we'd skip past the NUL in the loop line
      if ( *++pc == '\0' )
        break;
    }

    strResult += *pc;
  }

  return strResult;
}

// sanitize entry or group name: insert '\\' before any special characters
static std::string FilterOutEntryName(const std::string& str)
{
  std::string strResult;
  strResult.resize(str.length());

  for ( const char *pc = str.c_str(); *pc != '\0'; pc++ ) {
    const char c = *pc;

    // we explicitly allow some of "safe" chars and 8bit ASCII characters
    // which will probably never have special meaning and with which we can't
    // use isalnum() anyhow (in ASCII built, in Unicode it's just fine)
    //
    // NB: note that wxCONFIG_IMMUTABLE_PREFIX and wxCONFIG_PATH_SEPARATOR
    //     should *not* be quoted
    if ( !wxIsalnum(c) && !wxStrchr("@_/-!.*%()", c ) )
    {
      strResult += '\\';
    }

    strResult += c;
  }

  return strResult;
}

#endif // wxUSE_CONFIG
