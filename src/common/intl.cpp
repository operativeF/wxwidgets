/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/intl.cpp
// Purpose:     Internationalization and localisation for wxWidgets
// Author:      Vadim Zeitlin
// Modified by: Michael N. Filippov <michael@idisys.iae.nsk.su>
//              (2003/09/30 - PluralForms support)
// Created:     29/01/98
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_INTL

#include "wx/intl.h"
#include "wx/log.h"
#include "wx/utils.h"
#include "wx/module.h"

#ifdef HAVE_LANGINFO_H
    #include <langinfo.h>
#endif

#ifdef __WIN32__
    #include "wx/dynlib.h"
    #include "wx/msw/private.h"

    #ifndef LOCALE_SNAME
    #define LOCALE_SNAME 0x5c
    #endif
    #ifndef LOCALE_CUSTOM_UI_DEFAULT
    #define LOCALE_CUSTOM_UI_DEFAULT 0x1400
    #endif
#endif

#include "wx/fontmap.h"

#include "wx/apptrait.h"
#include "wx/stdpaths.h"

#if defined(__WXOSX__)
    #include "wx/osx/core/cfref.h"
    #include "wx/osx/core/cfstring.h"
    #include <CoreFoundation/CFLocale.h>
    #include <CoreFoundation/CFDateFormatter.h>
    #include <CoreFoundation/CFString.h>
#endif

import Utils.Strings;
import WX.File.Filename;
import WX.File.File;

import <clocale>;
import <string>;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

constexpr char TRACE_I18N[] = "i18n";

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// global functions
// ----------------------------------------------------------------------------

static wxLocale *wxSetLocale(wxLocale *pLocale);

namespace
{

#if defined(__UNIX__)

// get just the language part ("en" in "en_GB")
inline std::string ExtractLang(const std::string& langFull)
{
    return langFull.BeforeFirst('_');
}

// get everything else (including the leading '_')
inline std::string ExtractNotLang(const std::string& langFull)
{
    size_t pos = langFull.find('_');
    if ( pos != std::string::npos )
        return langFull.substr(pos);
    else
        return std::string();
}

#endif // __UNIX__

} // anonymous namespace

// ----------------------------------------------------------------------------
// wxLanguageInfo
// ----------------------------------------------------------------------------

#ifdef WX_WINDOWS

// helper used by wxLanguageInfo::GetLocaleName() and elsewhere to determine
// whether the locale is Unicode-only (it is if this function returns empty
// string)
static std::string wxGetANSICodePageForLocale(LCID lcid)
{
    std::string cp;

    wchar_t buffer[16];
    if ( ::GetLocaleInfoW(lcid, LOCALE_IDEFAULTANSICODEPAGE,
                        buffer, WXSIZEOF(buffer)) > 0 )
    {
        if ( buffer[0] != wxT('0') || buffer[1] != wxT('\0') )
            cp = boost::nowide::narrow(buffer);
        //else: this locale doesn't use ANSI code page
    }

    return cp;
}

std::uint32_t wxLanguageInfo::GetLCID() const
{
    return MAKELCID(MAKELANGID(WinLang, WinSublang), SORT_DEFAULT);
}

const char* wxLanguageInfo::TrySetLocale() const
{
    std::string locale;

    const LCID lcid = GetLCID();

    wxChar buffer[256];
    buffer[0] = wxT('\0');

    // Prefer to use the new (Vista and later) locale names instead of locale
    // identifiers if supported, both at the OS level (LOCALE_SNAME) and by the
    // CRT (check by calling setlocale()).
    if ( wxGetWinVersion() >= wxWinVersion_Vista )
    {
        if ( ::GetLocaleInfoW(lcid, LOCALE_SNAME, buffer, WXSIZEOF(buffer)) )
        {
            locale = boost::nowide::narrow(buffer);
        }
        else
        {
            wxLogLastError("GetLocaleInfo(LOCALE_SNAME)");
        }

        const char* const retloc = wxSetlocale(LC_ALL, locale);
        if ( retloc )
            return retloc;
        //else: fall back to LOCALE_SENGLANGUAGE
    }

    if ( !::GetLocaleInfoW(lcid, LOCALE_SENGLANGUAGE, buffer, WXSIZEOF(buffer)) )
    {
        wxLogLastError("GetLocaleInfo(LOCALE_SENGLANGUAGE)");
        return nullptr;
    }

    locale = boost::nowide::narrow(buffer);
    if ( ::GetLocaleInfoW(lcid, LOCALE_SENGCOUNTRY,
                        buffer, WXSIZEOF(buffer)) > 0 )
    {
        locale += fmt::format("_{}", boost::nowide::narrow(buffer));
    }

    const std::string cp = wxGetANSICodePageForLocale(lcid);
    if ( !cp.empty() )
    {
        locale += fmt::format(".{}", cp);
    }

    return wxSetlocale(LC_ALL, locale);
}

#else // !WX_WINDOWS

const char* wxLanguageInfo::TrySetLocale() const
{
    return wxSetlocale(LC_ALL, CanonicalName);
}

#endif // WX_WINDOWS/!WX_WINDOWS

std::string wxLanguageInfo::GetLocaleName() const
{
    const char* const orig = wxSetlocale(LC_ALL, nullptr);

    const char* const ret = TrySetLocale();
    std::string retval;
    if ( ret )
    {
        // Note that we must copy the returned value before calling setlocale()
        // again as the string "ret" points to can (and, at least under Linux
        // with glibc, actually always will) be changed by this call.
        retval = ret;
        wxSetlocale(LC_ALL, orig);
    }

    return retval;
}

// ----------------------------------------------------------------------------
// wxLocale
// ----------------------------------------------------------------------------

/*static*/ void wxLocale::CreateLanguagesDB()
{
    if (ms_languagesDB == nullptr)
    {
        ms_languagesDB = new wxLanguageInfoArray;
        InitLanguagesDB();
    }
}

/*static*/ void wxLocale::DestroyLanguagesDB()
{
    wxDELETE(ms_languagesDB);
}


void wxLocale::DoCommonInit()
{
    m_language = wxLANGUAGE_UNKNOWN;

    m_pszOldLocale = nullptr;
    m_pOldLocale = nullptr;

#ifdef __WIN32__
    m_oldLCID = 0;
#endif

    m_initialized = false;
}

// NB: this function has (desired) side effect of changing current locale
bool wxLocale::Init(const std::string& name,
                    const std::string& shortName,
                    const std::string& locale,
                    bool            bLoadDefault
                    )
{
    // change current locale (default: same as long name)
    std::string szLocale(locale);
    if ( szLocale.empty() )
    {
        // the argument to setlocale()
        szLocale = shortName;

        wxCHECK_MSG( !szLocale.empty(), false,
                    "no locale to set in wxLocale::Init()" );
    }

    if ( const wxLanguageInfo* langInfo = FindLanguageInfo(szLocale) )
    {
        // Prefer to use Init(wxLanguage) overload if possible as it will
        // correctly set our m_language and also set the locale correctly under
        // MSW, where just calling wxSetLocale() as we do below is not enough.
        //
        // However don't do it if the parameters are incompatible with this
        // language, e.g. if we are called with something like ("French", "de")
        // to use French locale but German translations: this seems unlikely to
        // happen but, in principle, it could.
        if ( wx::utils::StartsWith(langInfo->CanonicalName, shortName) )
        {
            return Init(langInfo->Language,
                        bLoadDefault ? wxLOCALE_LOAD_DEFAULT : 0);
        }
    }

    // the short name will be used to look for catalog files as well,
    // so we need something here
    std::string strShort(shortName);
    if ( strShort.empty() ) {
        // FIXME I don't know how these 2 letter abbreviations are formed,
        //       this wild guess is surely wrong
        if ( !szLocale.empty() )
        {
            strShort += (wxChar)wxTolower(szLocale[0]);
            if ( szLocale.length() > 1 )
                strShort += (wxChar)wxTolower(szLocale[1]);
        }
    }

    DoInit(name, strShort, wxLANGUAGE_UNKNOWN);

    const bool ret = wxSetlocale(LC_ALL, szLocale) != nullptr;

    return DoCommonPostInit(ret, szLocale, shortName, bLoadDefault);
}

void wxLocale::DoInit(const std::string& name,
                      const std::string& shortName,
                      int language)
{
    wxASSERT_MSG( !m_initialized,
                  "you can't call wxLocale::Init more than once" );

    m_initialized = true;
    m_strLocale = name;
    m_strShort = shortName;
    m_language = language;

    // Store the current locale in order to be able to restore it in the dtor.
    m_pszOldLocale = wxSetlocale(LC_ALL, nullptr);
    if ( m_pszOldLocale )
        m_pszOldLocale = wxStrdup(m_pszOldLocale);

#ifdef __WIN32__
    m_oldLCID = ::GetThreadLocale();
#endif

    m_pOldLocale = wxSetLocale(this);

    // Set translations object, but only if the user didn't do so yet.
    // This is to preserve compatibility with wx-2.8 where wxLocale was
    // the only API for translations. wxLocale works as a stack, with
    // latest-created one being the active one:
    //     wxLocale loc_fr(wxLANGUAGE_FRENCH);
    //     // _() returns French
    //     {
    //         wxLocale loc_cs(wxLANGUAGE_CZECH);
    //         // _() returns Czech
    //     }
    //     // _() returns French again
    wxTranslations *oldTrans = wxTranslations::Get();
    if ( !oldTrans ||
         (m_pOldLocale && oldTrans == &m_pOldLocale->m_translations) )
    {
        wxTranslations::SetNonOwned(&m_translations);
    }
}

bool wxLocale::DoCommonPostInit(bool success,
                                const std::string& name,
                                const std::string& shortName,
                                bool bLoadDefault)
{
    if ( !success )
    {
        wxLogWarning(_("Cannot set locale to language \"%s\"."), name);

        // As we failed to change locale, there is no need to restore the
        // previous one: it's still valid.
        free(const_cast<char *>(m_pszOldLocale));
        m_pszOldLocale = nullptr;

        // continue nevertheless and try to load at least the translations for
        // this language
    }

    wxTranslations *t = wxTranslations::Get();
    if ( t )
    {
        t->SetLanguage(shortName);

        if ( bLoadDefault )
            t->AddStdCatalog();
    }

    return success;
}

#if defined(__UNIX__)

// Helper of wxSetlocaleTryAll() below which tries setting the given locale
// with and without UTF-8 suffix. Don't use this one directly.
static const char *wxSetlocaleTryUTF8(int c, const std::string& lc)
{
    const char *l = NULL;

    // NB: We prefer to set UTF-8 locale if it's possible and only fall back to
    //     non-UTF-8 locale if it fails, but this is not necessary under the
    //     supported macOS versions where xx_YY locales are just aliases to
    //     xx_YY.UTF-8 anyhow.
#if !defined(__WXMAC__)
    if ( !lc.empty() )
    {
        std::string buf(lc);
        std::string buf2;
        buf2 = buf + ".UTF-8";
        l = wxSetlocale(c, buf2);
        if ( !l )
        {
            buf2 = buf + ".utf-8";
            l = wxSetlocale(c, buf2);
        }
        if ( !l )
        {
            buf2 = buf + ".UTF8";
            l = wxSetlocale(c, buf2);
        }
        if ( !l )
        {
            buf2 = buf + ".utf8";
            l = wxSetlocale(c, buf2);
        }
    }

    // if we can't set UTF-8 locale, try non-UTF-8 one:
    if ( !l )
#endif // !__WXMAC__
        l = wxSetlocale(c, lc);

    return l;
}

// Try setting all possible versions of the given locale, i.e. with and without
// UTF-8 encoding, and with or without the "_territory" part.
static const char *wxSetlocaleTryAll(int c, const std::string& lc)
{
    const char* l = wxSetlocaleTryUTF8(c, lc);
    if ( !l )
    {
        const std::string& lcOnlyLang = ExtractLang(lc);
        if ( lcOnlyLang != lc )
            l = wxSetlocaleTryUTF8(c, lcOnlyLang);
    }

    return l;
}

#endif // __UNIX__

#ifdef __WIN32__

// Trivial wrapper for ::SetThreadUILanguage().
//
// TODO-XP: Drop this when we don't support XP any longer.
static void wxMSWSetThreadUILanguage(LANGID langid)
{
    // SetThreadUILanguage() is available on XP, but with unclear
    // behavior, so avoid calling it there.
    if ( wxGetWinVersion() >= wxWinVersion_Vista )
    {
        wxLoadedDLL dllKernel32("kernel32.dll");
        typedef LANGID(WINAPI *SetThreadUILanguage_t)(LANGID);
        SetThreadUILanguage_t pfnSetThreadUILanguage = nullptr;
        wxDL_INIT_FUNC(pfn, SetThreadUILanguage, dllKernel32);
        if (pfnSetThreadUILanguage)
            pfnSetThreadUILanguage(langid);
    }
}

#endif // __WIN32__

bool wxLocale::Init(int lang, unsigned int flags)
{
    wxCHECK_MSG( lang != wxLANGUAGE_UNKNOWN, false,
                 "Initializing unknown locale doesn't make sense, did you "
                 "mean to use wxLANGUAGE_DEFAULT perhaps?" );

    std::string name, shortName;

    const wxLanguageInfo *info = GetLanguageInfo(lang);

    // Unknown language:
    if (info == nullptr)
    {
        // This could have happened because some concrete language has been
        // requested and we just don't know anything about it. In this case, we
        // have no choice but to simply give up.
        if ( lang != wxLANGUAGE_DEFAULT )
        {
            wxLogError("Unknown language %i.", lang);
            return false;
        }

        // However in case we didn't recognize the default system language, we
        // can still try to use it, even though we don't know anything about it
        // because setlocale() still might.
    }
    else
    {
        name = info->Description;
        shortName = info->CanonicalName;
    }

    DoInit(name, shortName, lang);

    // Set the locale:

#if defined(__UNIX__) || defined(__WIN32__)

    // We prefer letting the CRT to set its locale on its own when using
    // default locale, as it does a better job of it than we do. We also have
    // to do this when we didn't recognize the default language at all.
    const char *retloc = lang == wxLANGUAGE_DEFAULT ? wxSetlocale(LC_ALL, "")
                                                    : nullptr;

#if defined(__UNIX__)
    if ( !retloc )
        retloc = wxSetlocaleTryAll(LC_ALL, shortName);

    if ( !retloc )
    {
        // Some C libraries (namely glibc) still use old ISO 639,
        // so will translate the abbrev for them
        std::string localeAlt;
        const std::string& langOnly = ExtractLang(shortName);
        if ( langOnly == "he" )
            localeAlt = "iw" + ExtractNotLang(shortName);
        else if ( langOnly == "id" )
            localeAlt = "in" + ExtractNotLang(shortName);
        else if ( langOnly == "yi" )
            localeAlt = "ji" + ExtractNotLang(shortName);
        else if ( langOnly == "nb" )
            localeAlt = "no_NO";
        else if ( langOnly == "nn" )
            localeAlt = "no_NY";

        if ( !localeAlt.empty() )
            retloc = wxSetlocaleTryAll(LC_ALL, localeAlt);
    }

#ifdef __AIX__
    // at least in AIX 5.2 libc is buggy and the string returned from
    // setlocale(LC_ALL) can't be passed back to it because it returns 6
    // strings (one for each locale category), i.e. for C locale we get back
    // "C C C C C C"
    //
    // this contradicts IBM own docs but this is not of much help, so just work
    // around it in the crudest possible manner
    char* p = const_cast<char*>(wxStrchr(retloc, ' '));
    if ( p )
        *p = '\0';
#endif // __AIX__

#elif defined(__WIN32__)
    if ( lang == wxLANGUAGE_DEFAULT )
    {
        ::SetThreadLocale(LOCALE_USER_DEFAULT);
        wxMSWSetThreadUILanguage(LANG_USER_DEFAULT);

        // CRT locale already set above.
    }
    else if ( info->WinLang == 0 )
    {
        wxLogWarning("Locale '%s' not supported by OS.", name);

        retloc = "C";
    }
    else // language supported by Windows
    {
        const std::uint32_t lcid = info->GetLCID();

        // change locale used by Windows functions
        ::SetThreadLocale(lcid);

        wxMSWSetThreadUILanguage(LANGIDFROMLCID(lcid));

        // and also call setlocale() to change locale used by the CRT
        retloc = info->TrySetLocale();
    }
#if (defined(__VISUALC__) || defined(__MINGW32__))
    // VC++ setlocale() (also used by Mingw) can't set locale to languages that
    // can only be written using Unicode, therefore wxSetlocale() call fails
    // for such languages but we don't want to report it as an error -- so that
    // at least message catalogs can be used.
    if ( !retloc )
    {
        if ( wxGetANSICodePageForLocale(LOCALE_USER_DEFAULT).empty() )
        {
            // we set the locale to a Unicode-only language, don't treat the
            // inability of CRT to use it as an error
            retloc = "C";
        }
    }
#endif // CRT not handling Unicode-only languages
#else
    #error "Unsupported platform"
#endif

    return DoCommonPostInit
           (
                retloc != nullptr,
                name,
                shortName,
                flags & wxLOCALE_LOAD_DEFAULT
           );
#else // !(__UNIX__ || __WIN32__)
    wxUnusedVar(flags);
    return false;
#endif
}

namespace
{

#if defined(__UNIX__) && !defined(__WXOSX__)
// Small helper function: get the value of the given environment variable and
// return true only if the variable was found and has non-empty value.
inline bool wxGetNonEmptyEnvVar(const std::string& name, std::string* value)
{
    return wxGetEnv(name, value) && !value->empty();
}
#endif

} // anonymous namespace

/*static*/ int wxLocale::GetSystemLanguage()
{
    CreateLanguagesDB();

    size_t i = 0;
    const size_t count = ms_languagesDB->size();

#if defined(__UNIX__)
    // first get the string identifying the language from the environment
    std::string langFull;
#ifdef __WXOSX__
    wxCFRef<CFLocaleRef> userLocaleRef(CFLocaleCopyCurrent());

    // because the locale identifier (kCFLocaleIdentifier) is formatted a little bit differently, eg
    // az_Cyrl_AZ@calendar=buddhist;currency=JPY we just recreate the base info as expected by wx here

    wxCFStringRef str(wxCFRetain((CFStringRef)CFLocaleGetValue(userLocaleRef, kCFLocaleLanguageCode)));
    langFull = str.AsString()+"_";
    str.reset(wxCFRetain((CFStringRef)CFLocaleGetValue(userLocaleRef, kCFLocaleCountryCode)));
    langFull += str.AsString();
#else
    if (!wxGetNonEmptyEnvVar("LC_ALL", &langFull) &&
        !wxGetNonEmptyEnvVar("LC_MESSAGES", &langFull) &&
        !wxGetNonEmptyEnvVar("LANG", &langFull))
    {
        // no language specified, treat it as English
        return wxLANGUAGE_ENGLISH_US;
    }

#endif

    // the language string has the following form
    //
    //      lang[_LANG][.encoding][@modifier]
    //
    // (see environ(5) in the Open Unix specification)
    //
    // where lang is the primary language, LANG is a sublang/territory,
    // encoding is the charset to use and modifier "allows the user to select
    // a specific instance of localization data within a single category"
    //
    // for example, the following strings are valid:
    //      fr
    //      fr_FR
    //      de_DE.iso88591
    //      de_DE@euro
    //      de_DE.iso88591@euro

    // for now we don't use the encoding, although we probably should (doing
    // translations of the msg catalogs on the fly as required) (TODO)
    //
    // we need the modified for languages like Valencian: ca_ES@valencia
    // though, remember it
    std::string modifier;
    size_t posModifier = langFull.find_first_of("@");
    if ( posModifier != std::string::npos )
        modifier = langFull.Mid(posModifier);

    size_t posEndLang = langFull.find_first_of("@.");
    if ( posEndLang != std::string::npos )
    {
        langFull.Truncate(posEndLang);
    }

    if ( langFull == "C") || langFull == wxS("POSIX" )
    {
        // default C locale is English too
        return wxLANGUAGE_ENGLISH_US;
    }

    // do we have just the language (or sublang too)?
    const bool justLang = langFull.find('_') == std::string::npos;

    // 0. Make sure the lang is according to latest ISO 639
    //    (this is necessary because glibc uses iw and in instead
    //    of he and id respectively).

    // the language itself (second part is the dialect/sublang)
    std::string langOrig = ExtractLang(langFull);

    std::string lang;
    if ( langOrig == "iw")
        lang = "he";
    else if (langOrig == "in")
        lang = "id";
    else if (langOrig == "ji")
        lang = "yi";
    else if (langOrig == "no_NO")
        lang = "nb_NO";
    else if (langOrig == "no_NY")
        lang = "nn_NO";
    else if (langOrig == "no")
        lang = "nb_NO";
    else
        lang = langOrig;

    // did we change it?
    if ( lang != langOrig )
    {
        langFull = lang + ExtractNotLang(langFull);
    }

    // 1. Try to find the language either as is:
    // a) With modifier if set
    if ( !modifier.empty() )
    {
        std::string langFullWithModifier = langFull + modifier;
        for ( i = 0; i < count; i++ )
        {
            if ( (*ms_languagesDB)[i].CanonicalName == langFullWithModifier )
                break;
        }
    }

    // b) Without modifier
    if ( modifier.empty() || i == count )
    {
        for ( i = 0; i < count; i++ )
        {
            if ( (*ms_languagesDB)[i].CanonicalName == langFull )
                break;
        }
    }

    // 2. If langFull is of the form xx_YY, try to find xx:
    if ( i == count && !justLang )
    {
        for ( i = 0; i < count; i++ )
        {
            if ( ExtractLang((*ms_languagesDB)[i].CanonicalName) == lang )
            {
                break;
            }
        }
    }

    // 3. If langFull is of the form xx, try to find any xx_YY record:
    if ( i == count && justLang )
    {
        for ( i = 0; i < count; i++ )
        {
            if ( ExtractLang((*ms_languagesDB)[i].CanonicalName)
                    == langFull )
            {
                break;
            }
        }
    }


    if ( i == count )
    {
        // In addition to the format above, we also can have full language
        // names in LANG env var - for example, SuSE is known to use
        // LANG="german" - so check for use of non-standard format and try to
        // find the name in verbose description.
        for ( i = 0; i < count; i++ )
        {
            if ((*ms_languagesDB)[i].Description.CmpNoCase(langFull) == 0)
            {
                break;
            }
        }
    }
#elif defined(__WIN32__)
    const LANGID langid = ::GetUserDefaultUILanguage();
    if ( langid != LOCALE_CUSTOM_UI_DEFAULT )
    {
        const std::uint32_t lang = PRIMARYLANGID(langid);
        const std::uint32_t sublang = SUBLANGID(langid);

        for ( i = 0; i < count; i++ )
        {
            if ((*ms_languagesDB)[i].WinLang == lang &&
                (*ms_languagesDB)[i].WinSublang == sublang)
            {
                break;
            }
        }
    }
    //else: leave wxlang == wxLANGUAGE_UNKNOWN
#endif // Unix/Win32

    if ( i < count )
    {
        // we did find a matching entry, use it
        return (*ms_languagesDB)[i].Language;
    }

    // no info about this language in the database
    return wxLANGUAGE_UNKNOWN;
}

// ----------------------------------------------------------------------------
// encoding stuff
// ----------------------------------------------------------------------------

// this is a bit strange as under Windows we get the encoding name using its
// numeric value and under Unix we do it the other way round, but this just
// reflects the way different systems provide the encoding info

/* static */
std::string wxLocale::GetSystemEncodingName()
{
    std::string encname;

#if defined(__WIN32__)
    // FIXME: what is the error return value for GetACP()?
    const WXUINT codepage = ::GetACP();
    switch ( codepage )
    {
        case 65001:
            encname = "UTF-8";
            break;

        default:
            encname = fmt::format("windows-%u", codepage);
    }
#elif defined(__WXMAC__)
    encname = wxCFStringRef::AsString(
                CFStringGetNameOfEncoding(CFStringGetSystemEncoding())
              );
#elif defined(__UNIX_LIKE__)

#if defined(HAVE_LANGINFO_H) && defined(CODESET)
    // GNU libc provides current character set this way (this conforms
    // to Unix98)
    char *oldLocale = strdup(setlocale(LC_CTYPE, NULL));
    setlocale(LC_CTYPE, "");
    encname = nl_langinfo(CODESET);
    setlocale(LC_CTYPE, oldLocale);
    free(oldLocale);

    if (encname.empty())
#endif // HAVE_LANGINFO_H
    {
        // if we can't get at the character set directly, try to see if it's in
        // the environment variables (in most cases this won't work, but I was
        // out of ideas)
        char *lang = getenv( "LC_ALL");
        char *dot = lang ? strchr(lang, '.') : NULL;
        if (!dot)
        {
            lang = getenv( "LC_CTYPE" );
            if ( lang )
                dot = strchr(lang, '.' );
        }
        if (!dot)
        {
            lang = getenv( "LANG");
            if ( lang )
                dot = strchr(lang, '.');
        }

        if ( dot )
        {
            encname = dot + 1;
        }
    }
#endif // Win32/Unix

    return encname;
}

/* static */
wxFontEncoding wxLocale::GetSystemEncoding()
{
#if defined(__WIN32__)
    const WXUINT codepage = ::GetACP();

    switch ( codepage )
    {
        case 1250:
        case 1251:
        case 1252:
        case 1253:
        case 1254:
        case 1255:
        case 1256:
        case 1257:
        case 1258:
            return (wxFontEncoding)(wxFONTENCODING_CP1250 + codepage - 1250);

        case 1361:
            return wxFONTENCODING_CP1361;

        case 874:
            return wxFONTENCODING_CP874;

        case 932:
            return wxFONTENCODING_CP932;

        case 936:
            return wxFONTENCODING_CP936;

        case 949:
            return wxFONTENCODING_CP949;

        case 950:
            return wxFONTENCODING_CP950;

        case 65001:
            return wxFONTENCODING_UTF8;
    }
#elif defined(__WXMAC__)
    CFStringEncoding encoding = 0 ;
    encoding = CFStringGetSystemEncoding() ;
    return wxMacGetFontEncFromSystemEnc( encoding ) ;
#elif defined(__UNIX_LIKE__) && wxUSE_FONTMAP
    const std::string encname = GetSystemEncodingName();
    if ( !encname.empty() )
    {
        wxFontEncoding enc = wxFontMapperBase::GetEncodingFromName(encname);

        // GetEncodingFromName() returns wxFONTENCODING_DEFAULT for C locale
        // (a.k.a. US-ASCII) which is arguably a bug but keep it like this for
        // backwards compatibility and just take care to not return
        // wxFONTENCODING_DEFAULT from here as this surely doesn't make sense
        if ( enc == wxFONTENCODING_DEFAULT )
        {
            // we don't have wxFONTENCODING_ASCII, so use the closest one
            return wxFONTENCODING_ISO8859_1;
        }

        if ( enc != wxFONTENCODING_MAX )
        {
            return enc;
        }
        //else: return wxFONTENCODING_SYSTEM below
    }
#endif // Win32/Unix

    return wxFONTENCODING_SYSTEM;
}

/* static */
void wxLocale::AddLanguage(const wxLanguageInfo& info)
{
    CreateLanguagesDB();
    ms_languagesDB->push_back(info);
}

/* static */
const wxLanguageInfo *wxLocale::GetLanguageInfo(int lang)
{
    CreateLanguagesDB();

    // calling GetLanguageInfo(wxLANGUAGE_DEFAULT) is a natural thing to do, so
    // make it work
    if ( lang == wxLANGUAGE_DEFAULT )
        lang = GetSystemLanguage();

    if ( lang == wxLANGUAGE_UNKNOWN )
        return nullptr;

    const size_t count = ms_languagesDB->size();
    for ( size_t i = 0; i < count; i++ )
    {
        if ( (*ms_languagesDB)[i].Language == lang )
            return &(*ms_languagesDB)[i];
    }

    return nullptr;
}

/* static */
std::string wxLocale::GetLanguageName(int lang)
{
    std::string string;

    if ( lang == wxLANGUAGE_DEFAULT || lang == wxLANGUAGE_UNKNOWN )
        return string;

    const wxLanguageInfo *info = GetLanguageInfo(lang);
    if (info)
        string = info->Description;

    return string;
}

/* static */
std::string wxLocale::GetLanguageCanonicalName(int lang)
{
    if ( lang == wxLANGUAGE_DEFAULT || lang == wxLANGUAGE_UNKNOWN )
        return {};

    const wxLanguageInfo *info = GetLanguageInfo(lang);
    if (info)
        return info->CanonicalName;

    return {};
}

/* static */
const wxLanguageInfo *wxLocale::FindLanguageInfo(const std::string& locale)
{
    CreateLanguagesDB();

    const wxLanguageInfo *infoRet = nullptr;

    const size_t count = ms_languagesDB->size();
    for ( size_t i = 0; i < count; i++ )
    {
        const wxLanguageInfo *info = &(*ms_languagesDB)[i];

        if ( wxStricmp(locale, info->CanonicalName) == 0 ||
                wxStricmp(locale, info->Description) == 0 )
        {
            // exact match, stop searching
            infoRet = info;
            break;
        }

        if ( wxStricmp(locale, wx::utils::BeforeFirst(info->CanonicalName, '_')) == 0 )
        {
            // a match -- but maybe we'll find an exact one later, so continue
            // looking
            //
            // OTOH, maybe we had already found a language match and in this
            // case don't overwrite it because the entry for the default
            // country always appears first in ms_languagesDB
            if ( !infoRet )
                infoRet = info;
        }
    }

    return infoRet;
}

std::string wxLocale::GetSysName() const
{
    return wxSetlocale(LC_ALL, nullptr);
}

// clean up
wxLocale::~wxLocale()
{
    // Nothing here needs to be done if the object had never been initialized
    // successfully.
    if ( !m_initialized )
        return;

    // Restore old translations object.
    // See DoCommonInit() for explanation of why this is needed for backward
    // compatibility.
    if ( wxTranslations::Get() == &m_translations )
    {
        if ( m_pOldLocale )
            wxTranslations::SetNonOwned(&m_pOldLocale->m_translations);
        else
            wxTranslations::Set(nullptr);
    }

    // restore old locale pointer
    wxSetLocale(m_pOldLocale);

    if ( m_pszOldLocale )
    {
        wxSetlocale(LC_ALL, m_pszOldLocale);
        free(const_cast<char *>(m_pszOldLocale));
    }

#ifdef __WIN32__
    ::SetThreadLocale(m_oldLCID);
    wxMSWSetThreadUILanguage(LANGIDFROMLCID(m_oldLCID));
#endif
}


// check if the given locale is provided by OS and C run time
/* static */
bool wxLocale::IsAvailable(int lang)
{
    const wxLanguageInfo *info = wxLocale::GetLanguageInfo(lang);
    if ( !info )
    {
        // The language is unknown (this normally only happens when we're
        // passed wxLANGUAGE_DEFAULT), so we can't support it.
        wxASSERT_MSG( lang == wxLANGUAGE_DEFAULT,
                      "No info for a valid language?" );
        return false;
    }

#if defined(__WIN32__)
    if ( !info->WinLang )
        return false;

    if ( !::IsValidLocale(info->GetLCID(), LCID_INSTALLED) )
        return false;

#elif defined(__UNIX__)

    // Test if setting the locale works, then set it back.
    char * const oldLocale = wxStrdupA(setlocale(LC_ALL, NULL));

    const bool available = wxSetlocaleTryAll(LC_ALL, info->CanonicalName);

    // restore the original locale
    wxSetlocale(LC_ALL, oldLocale);

    free(oldLocale);

    if ( !available )
        return false;
#endif

    return true;
}


bool wxLocale::AddCatalog(const std::string& domain)
{
    wxTranslations *t = wxTranslations::Get();
    if ( !t )
        return false;
    return t->AddCatalog(domain);
}

bool wxLocale::AddCatalog(const std::string& domain, wxLanguage msgIdLanguage)
{
    wxTranslations *t = wxTranslations::Get();
    if ( !t )
        return false;
    return t->AddCatalog(domain, msgIdLanguage);
}

// add a catalog to our linked list
bool wxLocale::AddCatalog(const std::string& szDomain,
                        wxLanguage      msgIdLanguage,
                        const std::string& msgIdCharset)
{
    wxTranslations *t = wxTranslations::Get();
    if ( !t )
        return false;

    wxUnusedVar(msgIdCharset);
    return t->AddCatalog(szDomain, msgIdLanguage);
}

bool wxLocale::IsLoaded(const std::string& domain) const
{
    wxTranslations *t = wxTranslations::Get();
    if ( !t )
        return false;
    return t->IsLoaded(domain);
}

std::string wxLocale::GetHeaderValue(const std::string& header,
                                  const std::string& domain) const
{
    wxTranslations *t = wxTranslations::Get();
    if ( !t )
        return {};
    return t->GetHeaderValue(header, domain);
}

// ----------------------------------------------------------------------------
// accessors for locale-dependent data
// ----------------------------------------------------------------------------

#if defined(WX_WINDOWS) || defined(__WXOSX__)

namespace
{

bool IsAtTwoSingleQuotes(const std::string& fmt, std::string::const_iterator p)
{
    if ( p != fmt.end() && *p == '\'')
    {
        ++p;
        if ( p != fmt.end() && *p == '\'')
        {
            return true;
        }
    }

    return false;
}

} // anonymous namespace

// This function translates from Unicode date formats described at
//
//      http://unicode.org/reports/tr35/tr35-6.html#Date_Format_Patterns
//
// to strftime()-like syntax. This translation is not lossless but we try to do
// our best.

// The function is only exported because it is used in the unit test, it is not
// part of the public API.
std::string wxTranslateFromUnicodeFormat(const std::string& fmt)
{
    std::string fmtWX;
    fmtWX.reserve(fmt.length());

    char chLast = '\0';
    size_t lastCount = 0;

    const char* formatchars =
        "dghHmMsSy"
#ifdef WX_WINDOWS
        "t"
#else
        "EcLawD"
#endif
        ;
    for ( std::string::const_iterator p = fmt.begin(); /* end handled inside */; ++p )
    {
        if ( p != fmt.end() )
        {
            if ( *p == chLast )
            {
                lastCount++;
                continue;
            }

            const wxUniChar ch = *p;
            if ( ch.IsAscii() && strchr(formatchars, ch) )
            {
                // these characters come in groups, start counting them
                chLast = ch;
                lastCount = 1;
                continue;
            }
        }

        // interpret any special characters we collected so far
        if ( lastCount )
        {
            switch ( chLast )
            {
                case 'd':
                    switch ( lastCount )
                    {
                        case 1: // d
                        case 2: // dd
                            // these two are the same as we don't distinguish
                            // between 1 and 2 digits for days
                            fmtWX += "%d";
                            break;
#ifdef WX_WINDOWS
                        case 3: // ddd
                            fmtWX += "%a";
                            break;

                        case 4: // dddd
                            fmtWX += "%A";
                            break;
#endif
                        default:
                            wxFAIL_MSG( "too many 'd's" );
                    }
                    break;
#ifndef WX_WINDOWS
                case 'D':
                    switch ( lastCount )
                    {
                        case 1: // D
                        case 2: // DD
                        case 3: // DDD
                            fmtWX += "%j";
                            break;

                        default:
                            wxFAIL_MSG( "wrong number of 'D's" );
                    }
                    break;
               case 'w':
                    switch ( lastCount )
                    {
                        case 1: // w
                        case 2: // ww
                            fmtWX += "%W";
                            break;

                        default:
                            wxFAIL_MSG( "wrong number of 'w's" );
                    }
                    break;
                case 'E':
                   switch ( lastCount )
                    {
                        case 1: // E
                        case 2: // EE
                        case 3: // EEE
                            fmtWX += "%a";
                            break;
                        case 4: // EEEE
                            fmtWX += "%A";
                            break;
                        case 5: // EEEEE
                        case 6: // EEEEEE
                            // no "narrow form" in strftime(), use abbrev.
                            fmtWX += "%a";
                            break;

                        default:
                            wxFAIL_MSG( "wrong number of 'E's" );
                    }
                    break;
                case 'c':
                    switch ( lastCount )
                {
                    case 1: // c
                        // TODO: unsupported: first day of week as numeric value
                        fmtWX += "1";
                        break;
                    case 3: // ccc
                        fmtWX += "%a";
                        break;
                    case 4: // cccc
                        fmtWX += "%A";
                        break;
                    case 5: // ccccc
                        // no "narrow form" in strftime(), use abbrev.
                        fmtWX += "%a";
                        break;

                    default:
                        wxFAIL_MSG( "wrong number of 'c's" );
                }
                    break;
                case 'L':
                    switch ( lastCount )
                {
                    case 1: // L
                    case 2: // LL
                        fmtWX += "%m";
                        break;

                    case 3: // LLL
                        fmtWX += "%b";
                        break;

                    case 4: // LLLL
                        fmtWX += "%B";
                        break;

                    case 5: // LLLLL
                        // no "narrow form" in strftime(), use abbrev.
                        fmtWX += "%b";
                        break;

                    default:
                        wxFAIL_MSG( "too many 'L's" );
                }
                    break;
#endif
                case 'M':
                    switch ( lastCount )
                    {
                        case 1: // M
                        case 2: // MM
                            // as for 'd' and 'dd' above
                            fmtWX += "%m";
                            break;

                        case 3:
                            fmtWX += "%b";
                            break;

                        case 4:
                            fmtWX += "%B";
                            break;

                        case 5:
                            // no "narrow form" in strftime(), use abbrev.
                            fmtWX += "%b";
                            break;

                        default:
                            wxFAIL_MSG( "too many 'M's" );
                    }
                    break;

                case 'y':
                    switch ( lastCount )
                    {
                        case 1: // y
                        case 2: // yy
                            fmtWX += "%y";
                            break;

                        case 4: // yyyy
                            fmtWX += "%Y";
                            break;

                        default:
                            wxFAIL_MSG( "wrong number of 'y's" );
                    }
                    break;

                case 'H':
                    switch ( lastCount )
                    {
                        case 1: // H
                        case 2: // HH
                            fmtWX += "%H";
                            break;

                        default:
                            wxFAIL_MSG( "wrong number of 'H's" );
                    }
                    break;

               case 'h':
                    switch ( lastCount )
                    {
                        case 1: // h
                        case 2: // hh
                            fmtWX += "%I";
                            break;

                        default:
                            wxFAIL_MSG( "wrong number of 'h's" );
                    }
                    break;

               case 'm':
                    switch ( lastCount )
                    {
                        case 1: // m
                        case 2: // mm
                            fmtWX += "%M";
                            break;

                        default:
                            wxFAIL_MSG( "wrong number of 'm's" );
                    }
                    break;

               case 's':
                    switch ( lastCount )
                    {
                        case 1: // s
                        case 2: // ss
                            fmtWX += "%S";
                            break;

                        default:
                            wxFAIL_MSG( "wrong number of 's's" );
                    }
                    break;

                case 'g':
                    // strftime() doesn't have era string,
                    // ignore this format
                    wxASSERT_MSG( lastCount <= 2, "too many 'g's" );

                    break;
#ifndef WX_WINDOWS
                case 'a':
                    fmtWX += "%p";
                    break;
#endif
#ifdef WX_WINDOWS
                case 't':
                    switch ( lastCount )
                    {
                        case 1: // t
                        case 2: // tt
                            fmtWX += "%p";
                            break;

                        default:
                            wxFAIL_MSG( "too many 't's" );
                    }
                    break;
#endif
                default:
                    wxFAIL_MSG( "unreachable" );
            }

            chLast = '\0';
            lastCount = 0;
        }

        if ( p == fmt.end() )
            break;

        /*
        Handle single quotes:
        "Two single quotes represents [sic] a literal single quote, either
        inside or outside single quotes. Text within single quotes is not
        interpreted in any way (except for two adjacent single quotes)."
        */

        if ( IsAtTwoSingleQuotes(fmt, p) )
        {
            fmtWX += '\'';
            ++p; // the 2nd single quote is skipped by the for loop's increment
            continue;
        }

        bool isEndQuote = false;
        if ( *p == '\'' )
        {
            ++p;
            while ( p != fmt.end() )
            {
                if ( IsAtTwoSingleQuotes(fmt, p) )
                {
                    fmtWX += '\'';
                    p += 2;
                    continue;
                }

                if ( *p == '\'' )
                {
                    isEndQuote = true;
                    break;
                }

                fmtWX += *p;
                ++p;
            }
        }

        if ( p == fmt.end() )
            break;

        if ( !isEndQuote )
        {
            // not a special character so must be just a separator, treat as is
            if ( *p == wxT('%') )
            {
                // this one needs to be escaped
                fmtWX += wxT('%');
            }

            fmtWX += *p;
        }
    }

    return fmtWX;
}


#endif // WX_WINDOWS || __WXOSX__

#if defined(WX_WINDOWS)

namespace
{

LCTYPE GetLCTYPEFormatFromLocalInfo(wxLocaleInfo index)
{
    switch ( index )
    {
        case wxLOCALE_SHORT_DATE_FMT:
            return LOCALE_SSHORTDATE;

        case wxLOCALE_LONG_DATE_FMT:
            return LOCALE_SLONGDATE;

        case wxLOCALE_TIME_FMT:
            return LOCALE_STIMEFORMAT;

        default:
            wxFAIL_MSG( "no matching LCTYPE" );
    }

    return 0;
}

std::string
GetInfoFromLCID(LCID lcid,
                wxLocaleInfo index,
                wxLocaleCategory cat = wxLOCALE_CAT_DEFAULT)
{
    std::string str;

    wxChar buf[256];
    buf[0] = L'\0';

    switch ( index )
    {
        case wxLOCALE_THOUSANDS_SEP:
            if ( ::GetLocaleInfoW(lcid, LOCALE_STHOUSAND, buf, WXSIZEOF(buf)) )
                str = boost::nowide::narrow(buf);
            break;

        case wxLOCALE_DECIMAL_POINT:
            if ( ::GetLocaleInfoW(lcid,
                                 cat == wxLOCALE_CAT_MONEY
                                     ? LOCALE_SMONDECIMALSEP
                                     : LOCALE_SDECIMAL,
                                 buf,
                                 WXSIZEOF(buf)) )
            {
                str = boost::nowide::narrow(buf);

                // As we get our decimal point separator from Win32 and not the
                // CRT there is a possibility of mismatch between them and this
                // can easily happen if the user code called setlocale()
                // instead of using wxLocale to change the locale. And this can
                // result in very strange bugs elsewhere in the code as the
                // assumptions that formatted strings do use the decimal
                // separator actually fail, so check for it here.
                wxASSERT_MSG
                (
                    fmt::format("%.3f", 1.23).find(str) != std::string::npos,
                    "Decimal separator mismatch -- did you use setlocale()?"
                    "If so, use wxLocale to change the locale instead."
                );
            }
            break;

        case wxLOCALE_SHORT_DATE_FMT:
        case wxLOCALE_LONG_DATE_FMT:
        case wxLOCALE_TIME_FMT:
            if (::GetLocaleInfoW(lcid, GetLCTYPEFormatFromLocalInfo(index),
                                 buf, WXSIZEOF(buf)) )
            {
                return wxTranslateFromUnicodeFormat(boost::nowide::narrow(buf));
            }
            break;

        case wxLOCALE_DATE_TIME_FMT:
            // there doesn't seem to be any specific setting for this, so just
            // combine date and time ones
            //
            // we use the short date because this is what "%c" uses by default
            // ("%#c" uses long date but we have no way to specify the
            // alternate representation here)
            {
                const auto
                    datefmt = GetInfoFromLCID(lcid, wxLOCALE_SHORT_DATE_FMT);
                if ( datefmt.empty() )
                    break;

                const auto
                    timefmt = GetInfoFromLCID(lcid, wxLOCALE_TIME_FMT);
                if ( timefmt.empty() )
                    break;

                str += fmt::format("{} {}", datefmt, timefmt);
            }
            break;

        default:
            wxFAIL_MSG( "unknown wxLocaleInfo" );
    }

    return str;
}

} // anonymous namespace

/* static */
std::string wxLocale::GetInfo(wxLocaleInfo index, wxLocaleCategory cat)
{
    if ( !wxGetLocale() )
    {
        // wxSetLocale() hadn't been called yet of failed, hence CRT must be
        // using "C" locale -- but check it to detect bugs that would happen if
        // this were not the case.
        wxASSERT_MSG( strcmp(setlocale(LC_ALL, nullptr), "C") == 0,
                      "You probably called setlocale() directly instead "
                      "of using wxLocale and now there is a "
                      "mismatch between C/C++ and Windows locale.\n"
                      "Things are going to break, please only change "
                      "locale by creating wxLocale objects to avoid this!" );


        // Return the hard coded values for C locale. This is really the right
        // thing to do as there is no LCID we can use in the code below in this
        // case, even LOCALE_INVARIANT is not quite the same as C locale (the
        // only difference is that it uses %Y instead of %y in the date format
        // but this difference is significant enough).
        switch ( index )
        {
            case wxLOCALE_THOUSANDS_SEP:
                return {};

            case wxLOCALE_DECIMAL_POINT:
                return ".";

            case wxLOCALE_SHORT_DATE_FMT:
                return "%m/%d/%y";

            case wxLOCALE_LONG_DATE_FMT:
                return "%A, %B %d, %Y";

            case wxLOCALE_TIME_FMT:
                return "%H:%M:%S";

            case wxLOCALE_DATE_TIME_FMT:
                return "%m/%d/%y %H:%M:%S";

            default:
                wxFAIL_MSG( "unknown wxLocaleInfo" );
        }
    }

    // wxSetLocale() succeeded and so thread locale was set together with CRT one.
    return GetInfoFromLCID(::GetThreadLocale(), index, cat);
}

/* static */
std::string wxLocale::GetOSInfo(wxLocaleInfo index, wxLocaleCategory cat)
{
    return GetInfoFromLCID(::GetThreadLocale(), index, cat);
}

#elif defined(__WXOSX__)

/* static */
std::string wxLocale::GetInfo(wxLocaleInfo index, [[maybe_unused]] wxLocaleCategory cat)
{
    CFLocaleRef userLocaleRefRaw;
    if ( wxGetLocale() )
    {
        userLocaleRefRaw = CFLocaleCreate
                        (
                                kCFAllocatorDefault,
                                wxCFStringRef(wxGetLocale()->GetCanonicalName())
                        );
    }
    else // no current locale, use the default one
    {
        userLocaleRefRaw = CFLocaleCopyCurrent();
    }

    wxCFRef<CFLocaleRef> userLocaleRef(userLocaleRefRaw);

    CFStringRef cfstr = 0;
    switch ( index )
    {
        case wxLOCALE_THOUSANDS_SEP:
            cfstr = (CFStringRef) CFLocaleGetValue(userLocaleRef, kCFLocaleGroupingSeparator);
            break;

        case wxLOCALE_DECIMAL_POINT:
            cfstr = (CFStringRef) CFLocaleGetValue(userLocaleRef, kCFLocaleDecimalSeparator);
            break;

        case wxLOCALE_SHORT_DATE_FMT:
        case wxLOCALE_LONG_DATE_FMT:
        case wxLOCALE_DATE_TIME_FMT:
        case wxLOCALE_TIME_FMT:
            {
                CFDateFormatterStyle dateStyle = kCFDateFormatterNoStyle;
                CFDateFormatterStyle timeStyle = kCFDateFormatterNoStyle;
                switch (index )
                {
                    case wxLOCALE_SHORT_DATE_FMT:
                        dateStyle = kCFDateFormatterShortStyle;
                        break;
                    case wxLOCALE_LONG_DATE_FMT:
                        dateStyle = kCFDateFormatterFullStyle;
                        break;
                    case wxLOCALE_DATE_TIME_FMT:
                        dateStyle = kCFDateFormatterFullStyle;
                        timeStyle = kCFDateFormatterMediumStyle;
                        break;
                    case wxLOCALE_TIME_FMT:
                        timeStyle = kCFDateFormatterMediumStyle;
                        break;
                    default:
                        wxFAIL_MSG( "unexpected time locale" );
                        return std::string();
                }
                wxCFRef<CFDateFormatterRef> dateFormatter( CFDateFormatterCreate
                    (NULL, userLocaleRef, dateStyle, timeStyle));
                wxCFStringRef cfs = wxCFRetain( CFDateFormatterGetFormat(dateFormatter ));
                std::string format = wxTranslateFromUnicodeFormat(cfs.AsString());
                // we always want full years
                format.Replace("%y","%Y");
                return format;
            }

        default:
            wxFAIL_MSG( "Unknown locale info" );
            return std::string();
    }

    wxCFStringRef str(wxCFRetain(cfstr));
    return str.AsString();
}

#else // !WX_WINDOWS && !__WXOSX__, assume generic POSIX

namespace
{

std::string GetDateFormatFromLangInfo(wxLocaleInfo index)
{
#ifdef HAVE_LANGINFO_H
    // array containing parameters for nl_langinfo() indexes by offset of index
    // from wxLOCALE_SHORT_DATE_FMT
    static constexpr nl_item items[] =
    {
        D_FMT, D_T_FMT, D_T_FMT, T_FMT,
    };

    const int nlidx = index - wxLOCALE_SHORT_DATE_FMT;
    if ( nlidx < 0 || nlidx >= (int)WXSIZEOF(items) )
    {
        wxFAIL_MSG( "logic error in GetInfo() code" );
        return std::string();
    }

    const std::string fmt(nl_langinfo(items[nlidx]));

    // just return the format returned by nl_langinfo() except for long date
    // format which we need to recover from date/time format ourselves (but not
    // if we failed completely)
    if ( fmt.empty() || index != wxLOCALE_LONG_DATE_FMT )
        return fmt;

    // this is not 100% precise but the idea is that a typical date/time format
    // under POSIX systems is a combination of a long date format with time one
    // so we should be able to get just the long date format by removing all
    // time-specific format specifiers
    static constexpr char timeFmtSpecs[] = "HIklMpPrRsSTXzZ";
    static constexpr char timeSep[] = " :./-";

    std::string fmtDateOnly;
    const std::string::const_iterator end = fmt.end();
    std::string::const_iterator lastSep = end;
    for ( std::string::const_iterator p = fmt.begin(); p != end; ++p )
    {
        if ( strchr(timeSep, *p) )
        {
            if ( lastSep == end )
                lastSep = p;

            // skip it for now, we'll discard it if it's followed by a time
            // specifier later or add it to fmtDateOnly if it is not
            continue;
        }

        if ( *p == '%' &&
                (p + 1 != end) && strchr(timeFmtSpecs, p[1]) )
        {
            // time specified found: skip it and any preceding separators
            ++p;
            lastSep = end;
            continue;
        }

        if ( lastSep != end )
        {
            fmtDateOnly += std::string(lastSep, p);
            lastSep = end;
        }

        fmtDateOnly += *p;
    }

    return fmtDateOnly;
#else // !HAVE_LANGINFO_H
    wxUnusedVar(index);

    // no fallback, let the application deal with unavailability of
    // nl_langinfo() itself as there is no good way for us to do it (well, we
    // could try to reverse engineer the format from strftime() output but this
    // looks like too much trouble considering the relatively small number of
    // systems without nl_langinfo() still in use)
    return std::string();
#endif // HAVE_LANGINFO_H/!HAVE_LANGINFO_H
}

} // anonymous namespace

/* static */
std::string wxLocale::GetInfo(wxLocaleInfo index, wxLocaleCategory cat)
{
    lconv * const lc = localeconv();
    if ( !lc )
        return std::string();

    switch ( index )
    {
        case wxLOCALE_THOUSANDS_SEP:
            if ( cat == wxLOCALE_CAT_NUMBER )
                return lc->thousands_sep;
            else if ( cat == wxLOCALE_CAT_MONEY )
                return lc->mon_thousands_sep;

            wxFAIL_MSG( "invalid wxLocaleCategory" );
            break;


        case wxLOCALE_DECIMAL_POINT:
            if ( cat == wxLOCALE_CAT_NUMBER )
                return lc->decimal_point;
            else if ( cat == wxLOCALE_CAT_MONEY )
                return lc->mon_decimal_point;

            wxFAIL_MSG( "invalid wxLocaleCategory" );
            break;

        case wxLOCALE_SHORT_DATE_FMT:
        case wxLOCALE_LONG_DATE_FMT:
        case wxLOCALE_DATE_TIME_FMT:
        case wxLOCALE_TIME_FMT:
            if ( cat != wxLOCALE_CAT_DATE && cat != wxLOCALE_CAT_DEFAULT )
            {
                wxFAIL_MSG( "invalid wxLocaleCategory" );
                break;
            }

            return GetDateFormatFromLangInfo(index);


        default:
            wxFAIL_MSG( "unknown wxLocaleInfo value" );
    }

    return std::string();
}

#endif // platform

#ifndef WX_WINDOWS

/* static */
std::string wxLocale::GetOSInfo(wxLocaleInfo index, wxLocaleCategory cat)
{
    return GetInfo(index, cat);
}

#endif // !WX_WINDOWS

// ----------------------------------------------------------------------------
// global functions and variables
// ----------------------------------------------------------------------------

// retrieve/change current locale
// ------------------------------

// the current locale object
static wxLocale *g_pLocale = nullptr;

wxLocale *wxGetLocale()
{
    return g_pLocale;
}

wxLocale *wxSetLocale(wxLocale *pLocale)
{
    wxLocale *pOld = g_pLocale;
    g_pLocale = pLocale;
    return pOld;
}



// ----------------------------------------------------------------------------
// wxLocale module (for lazy destruction of languagesDB)
// ----------------------------------------------------------------------------

class wxLocaleModule: public wxModule
{
    wxDECLARE_DYNAMIC_CLASS(wxLocaleModule);
public:
        bool OnInit() override
        {
            return true;
        }

        void OnExit() override
        {
            wxLocale::DestroyLanguagesDB();
        }
};

wxIMPLEMENT_DYNAMIC_CLASS(wxLocaleModule, wxModule);

#endif // wxUSE_INTL
