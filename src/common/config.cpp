///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/config.cpp
// Purpose:     implementation of wxConfigBase class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     07.04.98
// Copyright:   (c) 1997 Karsten Ballueder  Ballueder@usa.net
//                       Vadim Zeitlin      <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/config.h"
#include "wx/intl.h"
#include "wx/log.h"
#include "wx/app.h"
#include "wx/utils.h"

import WX.Utils.Cast;

import Utils.Strings;

import <charconv>;
import <limits>;
import <string>;
import <vector>;

#if wxUSE_CONFIG && ((wxUSE_FILE && wxUSE_TEXTFILE))

#include "wx/apptrait.h"
#include "wx/file.h"


// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxAppTraitsBase
// ----------------------------------------------------------------------------

wxConfigBase *wxAppTraitsBase::CreateConfig()
{
    return new wxFileConfig(wxTheApp->GetAppName());
}

// ----------------------------------------------------------------------------
// wxConfigBase
// ----------------------------------------------------------------------------
wxIMPLEMENT_ABSTRACT_CLASS(wxConfigBase, wxObject);

// Not all args will always be used by derived classes, but including them all
// in each class ensures compatibility.
wxConfigBase::wxConfigBase(const std::string& appName,
                           const std::string& vendorName,
                           [[maybe_unused]] const std::string& localFilename,
                           [[maybe_unused]] const std::string& globalFilename,
                           unsigned int style)
            : m_appName(appName), m_vendorName(vendorName), m_style(style)
{
}

wxConfigBase *wxConfigBase::Set(wxConfigBase *pConfig)
{
  wxConfigBase *pOld = ms_pConfig;
  ms_pConfig = pConfig;
  return pOld;
}

wxConfigBase *wxConfigBase::Create()
{
  if ( ms_bAutoCreate && ms_pConfig == nullptr ) {
    wxAppTraits * const traits = wxApp::GetTraitsIfExists();
    wxCHECK_MSG( traits, nullptr, "create wxApp before calling this" );

    ms_pConfig = traits->CreateConfig();
  }

  return ms_pConfig;
}

// ----------------------------------------------------------------------------
// wxConfigBase reading entries
// ----------------------------------------------------------------------------

// implement both Read() overloads for the given type in terms of DoRead()
#define IMPLEMENT_READ_FOR_TYPE(name, type, deftype, extra)                 \
    bool wxConfigBase::Read(const std::string& key, type *val) const           \
    {                                                                       \
        wxCHECK_MSG( val, false, "wxConfig::Read(): NULL parameter" );  \
                                                                            \
        if ( !DoRead##name(key, val) )                                      \
            return false;                                                   \
                                                                            \
        *val = (extra)(*val);                                               \
                                                                            \
        return true;                                                        \
    }                                                                       \
                                                                            \
    bool wxConfigBase::Read(const std::string& key,                            \
                            type *val,                                      \
                            deftype defVal) const                           \
    {                                                                       \
        wxCHECK_MSG( val, false, "wxConfig::Read(): NULL parameter" );  \
                                                                            \
        bool read = DoRead##name(key, val);                                 \
        if ( !read )                                                        \
        {                                                                   \
            if ( IsRecordingDefaults() )                                    \
            {                                                               \
                const_cast<wxConfigBase*>(this)->DoWrite##name(key, defVal);\
            }                                                               \
                                                                            \
            *val = defVal;                                                  \
        }                                                                   \
                                                                            \
        *val = (extra)(*val);                                               \
                                                                            \
        return read;                                                        \
    }


IMPLEMENT_READ_FOR_TYPE(String, std::string, const std::string&, ExpandEnvVars)
IMPLEMENT_READ_FOR_TYPE(Long, long, long, long)
#ifdef wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG
IMPLEMENT_READ_FOR_TYPE(LongLong, wxLongLong_t, wxLongLong_t, wxLongLong_t)
#endif // wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG
IMPLEMENT_READ_FOR_TYPE(Double, double, double, double)
IMPLEMENT_READ_FOR_TYPE(Bool, bool, bool, bool)

#undef IMPLEMENT_READ_FOR_TYPE

// int is stored as long
bool wxConfigBase::Read(const std::string& key, int *pi) const
{
    long l = *pi;
    const bool r = Read(key, &l);
    wxASSERT_MSG( l < std::numeric_limits<int>::max(), "int overflow in wxConfig::Read" );
    *pi = (int)l;
    return r;
}

bool wxConfigBase::Read(const std::string& key, int *pi, int defVal) const
{
    long l = *pi;
    const bool r = Read(key, &l, defVal);
    wxASSERT_MSG( l < std::numeric_limits<int>::max(), "int overflow in wxConfig::Read" );
    *pi = (int)l;
    return r;
}

// size_t is stored either as long or long long (Win64)
#if SIZEOF_SIZE_T == SIZEOF_LONG
    typedef long SizeSameSizeAsSizeT;
#elif SIZEOF_SIZE_T == SIZEOF_LONG_LONG
    using SizeSameSizeAsSizeT = long long;
#else
    #error Unexpected sizeof(size_t)
#endif

bool wxConfigBase::Read(const std::string& key, size_t* val) const
{
    wxCHECK_MSG( val, false, "wxConfig::Read(): NULL parameter" );

    // TODO: Lambda
    SizeSameSizeAsSizeT tmp;
    if ( !Read(key, &tmp) )
        return false;

    *val = wx::narrow_cast<size_t>(tmp);
    return true;
}

bool wxConfigBase::Read(const std::string& key, size_t* val, size_t defVal) const
{
    wxCHECK_MSG( val, false, "wxConfig::Read(): NULL parameter" );

    if ( !Read(key, val) )
    {
        *val = defVal;
        return false;
    }

    return true;
}

// Read floats as doubles then just type cast it down.
bool wxConfigBase::Read(const std::string& key, float* val) const
{
    wxCHECK_MSG( val, false, "wxConfig::Read(): NULL parameter" );

    // TODO: Lambda
    double temp;
    if ( !Read(key, &temp) )
        return false;

    wxCHECK_MSG( std::fabs(temp) <= double(FLT_MAX), false,
                     "float overflow in wxConfig::Read" );
    wxCHECK_MSG( temp == 0.0 || std::fabs(temp) >= double(FLT_MIN), false,
                     "float underflow in wxConfig::Read" );

    *val = static_cast<float>(temp);

    return true;
}

bool wxConfigBase::Read(const std::string& key, float* val, float defVal) const
{
    wxCHECK_MSG( val, false, "wxConfig::Read(): NULL parameter" );

    if ( Read(key, val) )
        return true;

    *val = defVal;
    return false;
}

// the DoReadXXX() for the other types have implementation in the base class
// but can be overridden in the derived ones
bool wxConfigBase::DoReadBool(const std::string& key, bool* val) const
{
    wxCHECK_MSG( val, false, "wxConfig::Read(): NULL parameter" );

    long l;
    if ( !DoReadLong(key, &l) )
        return false;

    if ( l != 0 && l != 1 )
    {
        // Don't assert here as this could happen in the result of user editing
        // the file directly and this not indicate a bug in the program but
        // still complain that something is wrong.
        wxLogWarning(_("Invalid value %ld for a boolean key \"%s\" in "
                       "config file."),
                     l, key);
    }

    *val = l != 0;

    return true;
}

#ifdef wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG

bool wxConfigBase::DoReadLongLong(const std::string& key, wxLongLong_t *pll) const
{
    std::string str;
    if ( !Read(key, &str) )
        return false;

    wx::utils::TrimTrailingSpace(str);

    auto [p, ec] = std::from_chars(str.data(), str.data() + str.size(), *pll);
    return ec == std::errc();
}

#endif // wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG

bool wxConfigBase::DoReadDouble(const std::string& key, double* val) const
{
    std::string str;
    if ( Read(key, &str) )
    {
        auto [p, ec] = std::from_chars(key.data(), key.data() + key.size(), *val);
        if ( ec == std::errc() )
            return true;

        // Previous versions of wxFileConfig wrote the numbers out using the
        // current locale and not the C one as now, so attempt to parse the
        // string as a number in the current locale too, for compatibility.
        // if ( str.ToDouble(val) )
        //     return true;
    }

    return false;
}

// string reading helper
std::string wxConfigBase::ExpandEnvVars(const std::string& str) const
{
    std::string tmp; // Required for BC++
    if (IsExpandingEnvVars())
        tmp = wxExpandEnvVars(str);
    else
        tmp = str;
    return tmp;
}

// ----------------------------------------------------------------------------
// wxConfigBase writing
// ----------------------------------------------------------------------------

#ifdef wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG

bool wxConfigBase::DoWriteLongLong(const std::string& key, wxLongLong_t llValue)
{
  return Write(key, fmt::format("%" wxLongLongFmtSpec "d", llValue));
}

#endif // wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG

bool wxConfigBase::DoWriteDouble(const std::string& key, double val)
{
    // Notice that we always write out the numbers in C locale and not the
    // current one. This makes the config files portable between machines using
    // different locales.
    std::size_t currentSize{10};
    std::string valAsString;
    valAsString.resize(currentSize); // FIXME: What size to start with?

    std::to_chars_result strValResult = std::to_chars(valAsString.data(), valAsString.data() + valAsString.size(), val);

    while(strValResult.ec != std::errc())
    {
      currentSize += 10; // FIXME: What size to increment with?
      valAsString.resize(currentSize);
      strValResult = std::to_chars(valAsString.data(), valAsString.data() + valAsString.size(), val);
    }

    return DoWriteString(key, valAsString);
}

bool wxConfigBase::DoWriteBool(const std::string& key, bool value)
{
    return DoWriteLong(key, value ? 1l : 0l);
}

// ----------------------------------------------------------------------------
// wxConfigPathChanger
// ----------------------------------------------------------------------------

wxConfigPathChanger::wxConfigPathChanger(const wxConfigBase *pContainer,
                                         const std::string& strEntry)
{
  // FIXME: const_cast
  m_pContainer = const_cast<wxConfigBase *>(pContainer);

  // the path is everything which precedes the last slash and the name is
  // everything after it -- and this works correctly if there is no slash too
  std::string strPath = wx::utils::BeforeLast(strEntry, wxCONFIG_PATH_SEPARATOR);
  m_strName = wx::utils::AfterLast(strEntry, wxCONFIG_PATH_SEPARATOR);

  // except in the special case of "/keyname" when there is nothing before "/"
  if ( strPath.empty() &&
       ((!strEntry.empty()) && strEntry[0] == wxCONFIG_PATH_SEPARATOR) )
  {
    strPath = wxCONFIG_PATH_SEPARATOR;
  }

  if ( !strPath.empty() )
  {
    if ( m_pContainer->GetPath() != strPath )
    {
        // we do change the path so restore it later
        m_bChanged = true;

        /* JACS: work around a memory bug that causes an assert
           when using wxRegConfig, related to reference-counting.
           Can be reproduced by removing .wc_str() below and
           adding the following code to the config sample OnInit under
           Windows:

           pConfig->SetPath("MySettings");
           pConfig->SetPath("..");
           int value;
           pConfig->Read("MainWindowX", & value);
        */
        m_strOldPath = m_pContainer->GetPath();
        if ( *m_strOldPath.c_str() != wxCONFIG_PATH_SEPARATOR )
          m_strOldPath += wxCONFIG_PATH_SEPARATOR;
        m_pContainer->SetPath(strPath);
    }
  }
}

void wxConfigPathChanger::UpdateIfDeleted()
{
    // we don't have to do anything at all if we didn't change the path
    if ( !m_bChanged )
        return;

    // find the deepest still existing parent path of the original path
    while ( !m_pContainer->HasGroup(m_strOldPath) )
    {
        m_strOldPath = wx::utils::BeforeLast(m_strOldPath, wxCONFIG_PATH_SEPARATOR);
        if ( m_strOldPath.empty() )
            m_strOldPath = wxCONFIG_PATH_SEPARATOR;
    }
}

wxConfigPathChanger::~wxConfigPathChanger()
{
  // only restore path if it was changed
  if ( m_bChanged ) {
    m_pContainer->SetPath(m_strOldPath);
  }
}

// this is a wxConfig method but it's mainly used with wxConfigPathChanger
/* static */
std::string wxConfigBase::RemoveTrailingSeparator(const std::string& key)
{
    std::string path(key);

    // don't remove the only separator from a root group path!
    while ( path.length() > 1 )
    {
        if ( *path.rbegin() != wxCONFIG_PATH_SEPARATOR )
            break;

        path.erase(path.end() - 1);
    }

    return path;
}

#endif // wxUSE_CONFIG

// ----------------------------------------------------------------------------
// static & global functions
// ----------------------------------------------------------------------------

// understands both Unix and Windows (but only under Windows) environment
// variables expansion: i.e. $var, $(var) and ${var} are always understood
// and in addition under Windows %var% is also.

// don't change the values the enum elements: they must be equal
// to the matching [closing] delimiter.
enum Bracket
{
  Bracket_None,
  Bracket_Normal  = ')',
  Bracket_Curly   = '}',
#ifdef  WX_WINDOWS
  Bracket_Windows = '%',    // yeah, Windows people are a bit strange ;-)
#endif
  Bracket_Max
};

std::string wxExpandEnvVars(const std::string& str)
{
  std::string strResult;
  strResult.reserve(str.length());

  size_t m;
  for ( size_t n = 0; n < str.length(); n++ ) {
    switch ( str[n] ) {
#ifdef WX_WINDOWS
      case '%':
#endif // WX_WINDOWS
      case '$':
        {
          Bracket bracket;
          #ifdef WX_WINDOWS
            if ( str[n] == '%' )
              bracket = Bracket_Windows;
            else
          #endif // WX_WINDOWS
          if ( n == str.length() - 1 ) {
            bracket = Bracket_None;
          }
          else {
            switch ( str[n + 1] ) {
              case '(':
                bracket = Bracket_Normal;
                n++;                   // skip the bracket
                break;

              case '{':
                bracket = Bracket_Curly;
                n++;                   // skip the bracket
                break;

              default:
                bracket = Bracket_None;
            }
          }

          m = n + 1;

          while ( m < str.length() && (wxIsalnum(str[m]) || str[m] == '_') )
            m++;

          std::string strVarName(str.c_str() + n + 1, m - n - 1);

          // NB: use wxGetEnv instead of wxGetenv as otherwise variables
          //     set through wxSetEnv may not be read correctly!
          bool expanded = false;
          std::string tmp;
          if (wxGetEnv(strVarName, &tmp))
          {
              strResult += tmp;
              expanded = true;
          }
          else
          {
            // variable doesn't exist => don't change anything
            #ifdef  WX_WINDOWS
              if ( bracket != Bracket_Windows )
            #endif
                if ( bracket != Bracket_None )
                  strResult += str[n - 1];
            strResult += str[n] + strVarName;
          }

          // check the closing bracket
          if ( bracket != Bracket_None ) {
            if ( m == str.length() || str[m] != (wxChar)bracket ) {
              // under MSW it's common to have '%' characters in the registry
              // and it's annoying to have warnings about them each time, so
              // ignroe them silently if they are not used for env vars
              //
              // under Unix, OTOH, this warning could be useful for the user to
              // understand why isn't the variable expanded as intended
              #ifndef WX_WINDOWS
                wxLogWarning(_("Environment variables expansion failed: missing '%c' at position %u in '%s'."),
                             (char)bracket, (unsigned int) (m + 1), str.c_str());
              #endif // WX_WINDOWS
            }
            else {
              // skip closing bracket unless the variables wasn't expanded
              if ( !expanded )
                strResult += bracket;
              m++;
            }
          }

          n = m - 1;  // skip variable name
        }
        break;

      case wxT('\\'):
        // backslash can be used to suppress special meaning of % and $
        if ( n != str.length() - 1 &&
                (str[n + 1] == '%' || str[n + 1] == '$') ) {
          strResult += str[++n];

          break;
        }
        [[fallthrough]];

      default:
        strResult += str[n];
    }
  }

  return strResult;
}

// TODO: Return std::vector instead.
// this function is used to properly interpret '..' in path
std::vector<std::string> wxSplitPath(const std::string& path)
{
  std::vector<std::string> aParts;

  std::string strCurrent;
  std::string::const_iterator pc = path.begin();
  for ( ;; ) {
    if ( pc == path.end() || *pc == wxCONFIG_PATH_SEPARATOR ) {
      if ( strCurrent == "." ) {
        // ignore
      }
      else if ( strCurrent == ".." ) {
        // go up one level
        if ( aParts.size() == 0 )
        {
          wxLogWarning(_("'%s' has extra '..', ignored."), path);
        }
        else
        {
          aParts.erase(aParts.end() - 1);
        }

        strCurrent.clear();
      }
      else if ( !strCurrent.empty() ) {
        aParts.push_back(strCurrent);
        strCurrent.clear();
      }
      //else:
        // could log an error here, but we prefer to ignore extra '/'

      if ( pc == path.end() )
        break;
    }
    else
      strCurrent += *pc;

    ++pc;
  }

  return aParts;
}
