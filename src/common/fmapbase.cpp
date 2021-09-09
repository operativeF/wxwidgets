///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/fmapbase.cpp
// Purpose:     wxFontMapperBase class implementation
// Author:      Vadim Zeitlin
// Modified by:
// Created:     21.06.2003 (extracted from common/fontmap.cpp)
// Copyright:   (c) 1999-2003 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// for compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_FONTMAP

#ifndef WX_PRECOMP
    #include <array>
    #include <string>
    #include <string_view>
#endif //WX_PRECOMP

#if defined(__WINDOWS__)
    #include  "wx/msw/private.h"  // includes windows.h for LOGFONT
    #include  "wx/msw/winundef.h"
#endif

#include "wx/app.h"
#include "wx/log.h"
#include "wx/intl.h"
#include "wx/module.h"
#include "wx/wxcrtvararg.h"
#include "wx/fontmap.h"
#include "wx/fmappriv.h"
#include "wx/stringutils.h"

#include "wx/apptrait.h"

// wxMemoryConfig uses wxFileConfig
#if wxUSE_CONFIG && wxUSE_FILECONFIG
    #include "wx/config.h"
    #include "wx/memconf.h"
#endif

#include <cwctype>

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// encodings supported by GetEncodingDescription
constexpr std::array gs_encodings =
{
    wxFONTENCODING_ISO8859_1,
    wxFONTENCODING_ISO8859_2,
    wxFONTENCODING_ISO8859_3,
    wxFONTENCODING_ISO8859_4,
    wxFONTENCODING_ISO8859_5,
    wxFONTENCODING_ISO8859_6,
    wxFONTENCODING_ISO8859_7,
    wxFONTENCODING_ISO8859_8,
    wxFONTENCODING_ISO8859_9,
    wxFONTENCODING_ISO8859_10,
    wxFONTENCODING_ISO8859_11,
    wxFONTENCODING_ISO8859_12,
    wxFONTENCODING_ISO8859_13,
    wxFONTENCODING_ISO8859_14,
    wxFONTENCODING_ISO8859_15,
    wxFONTENCODING_KOI8,
    wxFONTENCODING_KOI8_U,
    wxFONTENCODING_CP866,
    wxFONTENCODING_CP874,
    wxFONTENCODING_CP932,
    wxFONTENCODING_CP936,
    wxFONTENCODING_CP949,
    wxFONTENCODING_CP950,
    wxFONTENCODING_CP1250,
    wxFONTENCODING_CP1251,
    wxFONTENCODING_CP1252,
    wxFONTENCODING_CP1253,
    wxFONTENCODING_CP1254,
    wxFONTENCODING_CP1255,
    wxFONTENCODING_CP1256,
    wxFONTENCODING_CP1257,
    wxFONTENCODING_CP1258,
    wxFONTENCODING_CP1361,
    wxFONTENCODING_CP437,
    wxFONTENCODING_UTF7,
    wxFONTENCODING_UTF8,
    wxFONTENCODING_UTF16BE,
    wxFONTENCODING_UTF16LE,
    wxFONTENCODING_UTF32BE,
    wxFONTENCODING_UTF32LE,
    wxFONTENCODING_EUC_JP,
    wxFONTENCODING_DEFAULT,
    wxFONTENCODING_ISO2022_JP,

    wxFONTENCODING_MACROMAN,
    wxFONTENCODING_MACJAPANESE,
    wxFONTENCODING_MACCHINESETRAD,
    wxFONTENCODING_MACKOREAN,
    wxFONTENCODING_MACARABIC,
    wxFONTENCODING_MACHEBREW,
    wxFONTENCODING_MACGREEK,
    wxFONTENCODING_MACCYRILLIC,
    wxFONTENCODING_MACDEVANAGARI,
    wxFONTENCODING_MACGURMUKHI,
    wxFONTENCODING_MACGUJARATI,
    wxFONTENCODING_MACORIYA,
    wxFONTENCODING_MACBENGALI,
    wxFONTENCODING_MACTAMIL,
    wxFONTENCODING_MACTELUGU,
    wxFONTENCODING_MACKANNADA,
    wxFONTENCODING_MACMALAJALAM,
    wxFONTENCODING_MACSINHALESE,
    wxFONTENCODING_MACBURMESE,
    wxFONTENCODING_MACKHMER,
    wxFONTENCODING_MACTHAI,
    wxFONTENCODING_MACLAOTIAN,
    wxFONTENCODING_MACGEORGIAN,
    wxFONTENCODING_MACARMENIAN,
    wxFONTENCODING_MACCHINESESIMP,
    wxFONTENCODING_MACTIBETAN,
    wxFONTENCODING_MACMONGOLIAN,
    wxFONTENCODING_MACETHIOPIC,
    wxFONTENCODING_MACCENTRALEUR,
    wxFONTENCODING_MACVIATNAMESE,
    wxFONTENCODING_MACARABICEXT,
    wxFONTENCODING_MACSYMBOL,
    wxFONTENCODING_MACDINGBATS,
    wxFONTENCODING_MACTURKISH,
    wxFONTENCODING_MACCROATIAN,
    wxFONTENCODING_MACICELANDIC,
    wxFONTENCODING_MACROMANIAN,
    wxFONTENCODING_MACCELTIC,
    wxFONTENCODING_MACGAELIC,
    wxFONTENCODING_MACKEYBOARD
};

// the descriptions for them
constexpr std::array gs_encodingDescs =
{
    wxTRANSLATE( "Western European (ISO-8859-1)" ),
    wxTRANSLATE( "Central European (ISO-8859-2)" ),
    wxTRANSLATE( "Esperanto (ISO-8859-3)" ),
    wxTRANSLATE( "Baltic (old) (ISO-8859-4)" ),
    wxTRANSLATE( "Cyrillic (ISO-8859-5)" ),
    wxTRANSLATE( "Arabic (ISO-8859-6)" ),
    wxTRANSLATE( "Greek (ISO-8859-7)" ),
    wxTRANSLATE( "Hebrew (ISO-8859-8)" ),
    wxTRANSLATE( "Turkish (ISO-8859-9)" ),
    wxTRANSLATE( "Nordic (ISO-8859-10)" ),
    wxTRANSLATE( "Thai (ISO-8859-11)" ),
    wxTRANSLATE( "Indian (ISO-8859-12)" ),
    wxTRANSLATE( "Baltic (ISO-8859-13)" ),
    wxTRANSLATE( "Celtic (ISO-8859-14)" ),
    wxTRANSLATE( "Western European with Euro (ISO-8859-15)" ),
    wxTRANSLATE( "KOI8-R" ),
    wxTRANSLATE( "KOI8-U" ),
    wxTRANSLATE( "Windows/DOS OEM Cyrillic (CP 866)" ),
    wxTRANSLATE( "Windows Thai (CP 874)" ),
    wxTRANSLATE( "Windows Japanese (CP 932) or Shift-JIS" ),
    wxTRANSLATE( "Windows Chinese Simplified (CP 936) or GB-2312" ),
    wxTRANSLATE( "Windows Korean (CP 949)" ),
    wxTRANSLATE( "Windows Chinese Traditional (CP 950) or Big-5" ),
    wxTRANSLATE( "Windows Central European (CP 1250)" ),
    wxTRANSLATE( "Windows Cyrillic (CP 1251)" ),
    wxTRANSLATE( "Windows Western European (CP 1252)" ),
    wxTRANSLATE( "Windows Greek (CP 1253)" ),
    wxTRANSLATE( "Windows Turkish (CP 1254)" ),
    wxTRANSLATE( "Windows Hebrew (CP 1255)" ),
    wxTRANSLATE( "Windows Arabic (CP 1256)" ),
    wxTRANSLATE( "Windows Baltic (CP 1257)" ),
    wxTRANSLATE( "Windows Vietnamese (CP 1258)" ),
    wxTRANSLATE( "Windows Johab (CP 1361)" ),
    wxTRANSLATE( "Windows/DOS OEM (CP 437)" ),
    wxTRANSLATE( "Unicode 7 bit (UTF-7)" ),
    wxTRANSLATE( "Unicode 8 bit (UTF-8)" ),
#ifdef WORDS_BIGENDIAN
    wxTRANSLATE( "Unicode 16 bit (UTF-16)" ),
    wxTRANSLATE( "Unicode 16 bit Little Endian (UTF-16LE)" ),
    wxTRANSLATE( "Unicode 32 bit (UTF-32)" ),
    wxTRANSLATE( "Unicode 32 bit Little Endian (UTF-32LE)" ),
#else // WORDS_BIGENDIAN
    wxTRANSLATE( "Unicode 16 bit Big Endian (UTF-16BE)" ),
    wxTRANSLATE( "Unicode 16 bit (UTF-16)" ),
    wxTRANSLATE( "Unicode 32 bit Big Endian (UTF-32BE)" ),
    wxTRANSLATE( "Unicode 32 bit (UTF-32)" ),
#endif // WORDS_BIGENDIAN
    wxTRANSLATE( "Extended Unix Codepage for Japanese (EUC-JP)" ),
    wxTRANSLATE( "US-ASCII" ),
    wxTRANSLATE( "ISO-2022-JP" ),

    wxTRANSLATE( "MacRoman" ),
    wxTRANSLATE( "MacJapanese" ),
    wxTRANSLATE( "MacChineseTrad" ),
    wxTRANSLATE( "MacKorean" ),
    wxTRANSLATE( "MacArabic" ),
    wxTRANSLATE( "MacHebrew" ),
    wxTRANSLATE( "MacGreek" ),
    wxTRANSLATE( "MacCyrillic" ),
    wxTRANSLATE( "MacDevanagari" ),
    wxTRANSLATE( "MacGurmukhi" ),
    wxTRANSLATE( "MacGujarati" ),
    wxTRANSLATE( "MacOriya" ),
    wxTRANSLATE( "MacBengali" ),
    wxTRANSLATE( "MacTamil" ),
    wxTRANSLATE( "MacTelugu" ),
    wxTRANSLATE( "MacKannada" ),
    wxTRANSLATE( "MacMalayalam" ),
    wxTRANSLATE( "MacSinhalese" ),
    wxTRANSLATE( "MacBurmese" ),
    wxTRANSLATE( "MacKhmer" ),
    wxTRANSLATE( "MacThai" ),
    wxTRANSLATE( "MacLaotian" ),
    wxTRANSLATE( "MacGeorgian" ),
    wxTRANSLATE( "MacArmenian" ),
    wxTRANSLATE( "MacChineseSimp" ),
    wxTRANSLATE( "MacTibetan" ),
    wxTRANSLATE( "MacMongolian" ),
    wxTRANSLATE( "MacEthiopic" ),
    wxTRANSLATE( "MacCentralEurRoman" ),
    wxTRANSLATE( "MacVietnamese" ),
    wxTRANSLATE( "MacExtArabic" ),
    wxTRANSLATE( "MacSymbol" ),
    wxTRANSLATE( "MacDingbats" ),
    wxTRANSLATE( "MacTurkish" ),
    wxTRANSLATE( "MacCroatian" ),
    wxTRANSLATE( "MacIcelandic" ),
    wxTRANSLATE( "MacRomanian" ),
    wxTRANSLATE( "MacCeltic" ),
    wxTRANSLATE( "MacGaelic" ),
    wxTRANSLATE( "MacKeyboardGlyphs" )
};

// and the internal names (these are not translated on purpose!)
constexpr std::array<std::string_view, 83> gs_encodingNames =
{{
    // names from the columns correspond to these OS:
    //      Linux        Solaris and IRIX       HP-UX             AIX
    "ISO-8859-1,ISO8859-1,iso88591,8859-1,iso_8859_1",
    "ISO-8859-2,ISO8859-2,iso88592,8859-2",
    "ISO-8859-3,ISO8859-3,iso88593,8859-3",
    "ISO-8859-4,ISO8859-4,iso88594,8859-4",
    "ISO-8859-5,ISO8859-5,iso88595,8859-5",
    "ISO-8859-6,ISO8859-6,iso88596,8859-6",
    "ISO-8859-7,ISO8859-7,iso88597,8859-7",
    "ISO-8859-8,ISO8859-8,iso88598,8859-8",
    "ISO-8859-9,ISO8859-9,iso88599,8859-9",
    "ISO-8859-10,ISO8859-10,iso885910,8859-10",
    "ISO-8859-11,ISO8859-11,iso885911,8859-11",
    "ISO-8859-12,ISO8859-12,iso885912,8859-12",
    "ISO-8859-13,ISO8859-13,iso885913,8859-13",
    "ISO-8859-14,ISO8859-14,iso885914,8859-14",
    "ISO-8859-15,ISO8859-15,iso885915,8859-15",

    // although koi8-ru is not strictly speaking the same as koi8-r,
    // they are similar enough to make mapping it to koi8 better than
    // not recognizing it at all
     "KOI8-R,KOI8-RU",
     "KOI8-U",

     "WINDOWS-866,CP866",

     "WINDOWS-874,CP874,MS874,IBM-874",
     "WINDOWS-932,CP932,MS932,IBM-932,SJIS,SHIFT-JIS,SHIFT_JIS",
     "WINDOWS-936,CP936,MS936,IBM-936,GB2312,gbk,GBK",
     "WINDOWS-949,CP949,MS949,IBM-949,EUC-KR,eucKR,euc_kr",
     "WINDOWS-950,CP950,MS950,IBM-950,BIG5,BIG-5,BIG-FIVE",
     "WINDOWS-1250,CP1250,MS1250,IBM-1250",
     "WINDOWS-1251,CP1251,MS1251,IBM-1251",
     "WINDOWS-1252,CP1252,MS1252,IBM-1252",
     "WINDOWS-1253,CP1253,MS1253,IBM-1253",
     "WINDOWS-1254,CP1254,MS1254,IBM-1254",
     "WINDOWS-1255,CP1255,MS1255,IBM-1255",
     "WINDOWS-1256,CP1256,MS1256,IBM-1256",
     "WINDOWS-1257,CP1257,MS1257,IBM-1257",
     "WINDOWS-1258,CP1258,MS1258,IBM-1258",
     "WINDOWS-1361,CP1361,MS1361,IBM-1361,JOHAB",
     "WINDOWS-437,CP437,MS437,IBM-437",

     "UTF-7,UTF7",
     "UTF-8,UTF8",
#ifdef WORDS_BIGENDIAN
    wxT( "UTF-16BE,UTF16BE,UCS-2BE,UCS2BE,UTF-16,UTF16,UCS-2,UCS2"),
    wxT( "UTF-16LE,UTF16LE,UCS-2LE,UCS2LE"),
    wxT( "UTF-32BE,UTF32BE,UCS-4BE,UTF-32,UTF32,UCS-4,UCS4"),
    wxT( "UTF-32LE,UTF32LE,UCS-4LE,UCS4LE"),
#else // WORDS_BIGENDIAN
    "UTF-16BE,UTF16BE,UCS-2BE,UCS2BE",
    "UTF-16LE,UTF16LE,UCS-2LE,UTF-16,UTF16,UCS-2,UCS2",
    "UTF-32BE,UTF32BE,UCS-4BE,UCS4BE",
    "UTF-32LE,UTF32LE,UCS-4LE,UCS4LE,UTF-32,UTF32,UCS-4,UCS4",
#endif // WORDS_BIGENDIAN

     "EUC-JP,eucJP,euc_jp,IBM-eucJP",

    // 646 is for Solaris, roman8 -- for HP-UX
     "US-ASCII,ASCII,C,POSIX,ANSI_X3.4-1968,646,roman8",

     "ISO-2022-JP",

     "MacRoman",
     "MacJapanese",
     "MacChineseTrad",
     "MacKorean",
     "MacArabic",
     "MacHebrew",
     "MacGreek",
     "MacCyrillic",
     "MacDevanagari",
     "MacGurmukhi",
     "MacGujarati",
     "MacOriya",
     "MacBengali",
     "MacTamil",
     "MacTelugu",
     "MacKannada",
     "MacMalayalam",
     "MacSinhalese",
     "MacBurmese",
     "MacKhmer",
     "MacThai",
     "MacLaotian",
     "MacGeorgian",
     "MacArmenian",
     "MacChineseSimp",
     "MacTibetan",
     "MacMongolian",
     "MacEthiopic",
     "MacCentralEurRoman",
     "MacVietnamese",
     "MacExtArabic",
     "MacSymbol",
     "MacDingbats",
     "MacTurkish",
     "MacCroatian",
     "MacIcelandic",
     "MacRomanian",
     "MacCeltic",
     "MacGaelic",
     "MacKeyboardGlyphs"
}};

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// clean up the font mapper object
class wxFontMapperModule : public wxModule
{
public:
    bool OnInit() override
    {
        // a dummy wxFontMapperBase object could have been created during the
        // program startup before wxApp was created, we have to delete it to
        // allow creating the real font mapper next time it is needed now that
        // we can create it (when the modules are initialized, wxApp object
        // already exists)
        wxFontMapperBase *fm = wxFontMapperBase::Get();
        if ( fm && fm->IsDummy() )
            wxFontMapperBase::Reset();

        return true;
    }

    void OnExit() override
    {
        wxFontMapperBase::Reset();
    }

    wxDECLARE_DYNAMIC_CLASS(wxFontMapperModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxFontMapperModule, wxModule);

// ----------------------------------------------------------------------------
// ctor and dtor
// ----------------------------------------------------------------------------

wxFontMapperBase::~wxFontMapperBase()
{
#if wxUSE_CONFIG && wxUSE_FILECONFIG
    delete m_configDummy;
#endif // wxUSE_CONFIG
}

/* static */
wxFontMapperBase *wxFontMapperBase::Get()
{
    if ( !sm_instance )
    {
        wxAppTraits *traits = wxApp::GetTraitsIfExists();
        if ( traits )
        {
            sm_instance = traits->CreateFontMapper();

            wxASSERT_MSG( sm_instance,
                            wxT("wxAppTraits::CreateFontMapper() failed") );
        }

        if ( !sm_instance )
        {
            // last resort: we must create something because the existing code
            // relies on always having a valid font mapper object
            sm_instance = (wxFontMapper *)new wxFontMapperBase;
        }
    }

    return (wxFontMapperBase*)sm_instance;
}

/* static */
wxFontMapper *wxFontMapperBase::Set(wxFontMapper *mapper)
{
    wxFontMapper *old = sm_instance;
    sm_instance = mapper;
    return old;
}

/* static */
void wxFontMapperBase::Reset()
{
    if ( sm_instance )
    {
        // we need a cast as wxFontMapper is not fully declared here and so the
        // compiler can't know that it derives from wxFontMapperBase (but
        // run-time behaviour will be correct because the dtor is virtual)
        delete (wxFontMapperBase *)sm_instance;
        sm_instance = nullptr;
    }
}

#if wxUSE_CONFIG && wxUSE_FILECONFIG

// ----------------------------------------------------------------------------
// config usage customisation
// ----------------------------------------------------------------------------


static std::string gs_defaultConfigPath(FONTMAPPER_ROOT_PATH);

/* static */
const std::string& wxFontMapperBase::GetDefaultConfigPath()
{
    // NB: we return const wxString& and not wxString for compatibility
    //     with 2.8 that returned const wxChar*
    return gs_defaultConfigPath;
}

void wxFontMapperBase::SetConfigPath(const std::string& prefix)
{
    wxCHECK_RET( !prefix.empty() && prefix[0] == wxCONFIG_PATH_SEPARATOR,
                 wxT("an absolute path should be given to wxFontMapper::SetConfigPath()") );

    m_configRootPath = prefix;
}

// ----------------------------------------------------------------------------
// get config object and path for it
// ----------------------------------------------------------------------------

wxConfigBase *wxFontMapperBase::GetConfig()
{
    wxConfigBase *config = wxConfig::Get(false);

    // If there is no global configuration, use an internal memory configuration
    if ( !config )
    {
        if ( !m_configDummy )
            m_configDummy = new wxMemoryConfig;
        config = m_configDummy;

        // FIXME: ideally, we should add keys from dummy config to a real one later,
        //        but it is a low-priority task because typical wxWin application
        //        either doesn't use wxConfig at all or creates wxConfig object in
        //        wxApp::OnInit(), before any real interaction with the user takes
        //        place...
    }

    return config;
}

const std::string& wxFontMapperBase::GetConfigPath()
{
    if ( m_configRootPath.empty() )
    {
        // use the default
        m_configRootPath = GetDefaultConfigPath();
    }

    return m_configRootPath;
}

// ----------------------------------------------------------------------------
// config helpers
// ----------------------------------------------------------------------------

bool wxFontMapperBase::ChangePath(const wxString& pathNew, wxString* pathOld)
{
    wxConfigBase *config = GetConfig();
    if ( !config )
        return false;

    *pathOld = config->GetPath();

    wxString path = GetConfigPath();
    if ( path.empty() || path.Last() != wxCONFIG_PATH_SEPARATOR )
    {
        path += wxCONFIG_PATH_SEPARATOR;
    }

    // FIXME: Stupid.
    //wxASSERT_MSG( pathNew.empty() || (pathNew.Front() != wxCONFIG_PATH_SEPARATOR),
    //              wxT("should be a relative path") );

    path += pathNew;

    config->SetPath(path);

    return true;
}

void wxFontMapperBase::RestorePath(const wxString& pathOld)
{
    GetConfig()->SetPath(pathOld);
}

#endif

// ----------------------------------------------------------------------------
// charset/encoding correspondence
// ----------------------------------------------------------------------------

wxFontEncoding
wxFontMapperBase::CharsetToEncoding(const std::string& charset,
                                    bool WXUNUSED(interactive))
{
    int enc = NonInteractiveCharsetToEncoding(charset);
    if ( enc == wxFONTENCODING_UNKNOWN )
    {
        // we should return wxFONTENCODING_SYSTEM from here for unknown
        // encodings
        enc = wxFONTENCODING_SYSTEM;
    }

    return (wxFontEncoding)enc;
}

int
wxFontMapperBase::NonInteractiveCharsetToEncoding(const std::string& charset)
{
    // TODO: Implement heuristics for determining / inferring encoding
    // with string_view.

    // TODO: Make sure that this is correct; returning default with empty string.
    if(charset == "")
        return wxFONTENCODING_DEFAULT;

    wxFontEncoding encoding = wxFONTENCODING_SYSTEM;

    // we're going to modify it, make a copy
    wxString cs = charset;

#if wxUSE_CONFIG && wxUSE_FILECONFIG
    // first try the user-defined settings
    wxFontMapperPathChanger path(this, FONTMAPPER_CHARSET_PATH);
    if ( path.IsOk() )
    {
        wxConfigBase *config = GetConfig();

        // do we have an encoding for this charset?
        const long value = config->Read(charset, -1l);
        if ( value != -1 )
        {
            if ( value == wxFONTENCODING_UNKNOWN )
            {
                // don't try to find it, in particular don't ask the user
                return value;
            }

            if ( value >= 0 && value <= wxFONTENCODING_MAX )
            {
                encoding = (wxFontEncoding)value;
            }
            else
            {
                wxLogDebug(wxT("corrupted config data: invalid encoding %ld for charset '%s' ignored"),
                           value, charset.c_str());
            }
        }

        if ( encoding == wxFONTENCODING_SYSTEM )
        {
            // may be we have an alias?
            config->SetPath(FONTMAPPER_CHARSET_ALIAS_PATH);

            wxString alias = config->Read(charset);
            if ( !alias.empty() )
            {
                // yes, we do - use it instead
                cs = alias;
            }
        }
    }
#endif // wxUSE_CONFIG

    // if didn't find it there, try to recognize it ourselves
    if ( encoding == wxFONTENCODING_SYSTEM )
    {
        // trim any spaces
        cs.Trim(true);
        cs.Trim(false);

        // discard the optional quotes
        if ( !cs.empty() )
        {
            if ( cs[0u] == wxT('"') && cs.Last() == wxT('"') )
            {
                cs = wxString(cs.c_str(), cs.length() - 1);
            }
        }

        int enc_index = 0;

        // FIXME: Add MakeUpper function
        for ( const auto encFamily : gs_encodingNames )
        {
            if (encFamily.find(cs.MakeUpper().ToStdString()) != std::string_view::npos)
                return gs_encodings[enc_index];

            ++enc_index;
        }

        cs.MakeUpper();

        if ( cs.Left(3) == wxT("ISO") )
        {
            // the dash is optional (or, to be exact, it is not, but
            // several broken programs "forget" it)
            const wxChar *p = cs.c_str() + 3;
            if ( *p == wxT('-') )
                p++;

            unsigned int value;
            if ( wxSscanf(p, wxT("8859-%u"), &value) == 1 )
            {
                // make it 0 based and check that it is strictly positive in
                // the process (no such thing as iso8859-0 encoding)
                if ( (value-- > 0) &&
                     (value < wxFONTENCODING_ISO8859_MAX -
                              wxFONTENCODING_ISO8859_1) )
                {
                    // it's a valid ISO8859 encoding
                    value += wxFONTENCODING_ISO8859_1;
                    encoding = (wxFontEncoding)value;
                }
            }
        }
        else if ( cs.Left(4) == wxT("8859") )
        {
            const wxChar *p = cs.c_str();

            unsigned int value;
            if ( wxSscanf(p, wxT("8859-%u"), &value) == 1 )
            {
                // make it 0 based and check that it is strictly positive in
                // the process (no such thing as iso8859-0 encoding)
                if ( (value-- > 0) &&
                     (value < wxFONTENCODING_ISO8859_MAX -
                              wxFONTENCODING_ISO8859_1) )
                {
                    // it's a valid ISO8859 encoding
                    value += wxFONTENCODING_ISO8859_1;
                    encoding = (wxFontEncoding)value;
                }
            }
        }
        else // check for Windows charsets
        {
            size_t len;
            if ( cs.Left(7) == wxT("WINDOWS") )
            {
                len = 7;
            }
            else if ( cs.Left(2) == wxT("CP") )
            {
                len = 2;
            }
            else // not a Windows encoding
            {
                len = 0;
            }

            if ( len )
            {
                const wxChar *p = cs.c_str() + len;
                if ( *p == wxT('-') )
                    p++;

                unsigned int value;
                if ( wxSscanf(p, wxT("%u"), &value) == 1 )
                {
                    if ( value >= 1250 )
                    {
                        value -= 1250;
                        if ( value < wxFONTENCODING_CP12_MAX -
                                     wxFONTENCODING_CP1250 )
                        {
                            // a valid Windows code page
                            value += wxFONTENCODING_CP1250;
                            encoding = (wxFontEncoding)value;
                        }
                    }

                    switch ( value )
                    {
                        case 866:
                            encoding = wxFONTENCODING_CP866;
                            break;

                        case 874:
                            encoding = wxFONTENCODING_CP874;
                            break;

                        case 932:
                            encoding = wxFONTENCODING_CP932;
                            break;

                        case 936:
                            encoding = wxFONTENCODING_CP936;
                            break;

                        case 949:
                            encoding = wxFONTENCODING_CP949;
                            break;

                        case 950:
                            encoding = wxFONTENCODING_CP950;
                            break;

                        case 1258:
                            encoding = wxFONTENCODING_CP1258;
                            break;

                        case 1361:
                            encoding = wxFONTENCODING_CP1361;
                            break;
                    }
                }
            }
        }
        //else: unknown
    }

    return encoding;
}

/* static */
std::size_t wxFontMapperBase::GetSupportedEncodingsCount()
{
    return gs_encodings.size();
}

/* static */
wxFontEncoding wxFontMapperBase::GetEncoding(size_t n)
{
    wxCHECK_MSG( n < gs_encodings.size(), wxFONTENCODING_SYSTEM,
                    wxT("wxFontMapper::GetEncoding(): invalid index") );

    return gs_encodings[n];
}

/* static */
std::string wxFontMapperBase::GetEncodingDescription(wxFontEncoding encoding)
{
    if ( encoding == wxFONTENCODING_DEFAULT )
    {
        return _("Default encoding").ToStdString();
    }

    for ( size_t i = 0; i != gs_encodingDescs.size(); i++ )
    {
        if ( gs_encodings[i] == encoding )
        {
            return wxGetTranslation(gs_encodingDescs[i]).ToStdString();
        }
    }

    wxString str;
    str.Printf(_("Unknown encoding (%d)"), encoding);

    return str.ToStdString();
}

/* static */
std::string wxFontMapperBase::GetEncodingName(wxFontEncoding encoding)
{
    if ( encoding == wxFONTENCODING_DEFAULT )
    {
        return _("default").ToStdString();
    }

    for ( size_t i = 0; i < gs_encodingNames.size(); i++ )
    {
        if ( gs_encodings[i] == encoding )
        {
            return std::string(gs_encodingNames[i].substr(0, gs_encodingNames[i].find_first_of(',')));
        }
    }

    wxString str;
    str.Printf(_("unknown-%d"), encoding);

    return str.ToStdString();
}

/* static */
std::vector<std::string_view> wxFontMapperBase::GetAllEncodingNames(wxFontEncoding encoding)
{
    for ( size_t i = 0; i < gs_encodingNames.size(); i++ )
    {
        if ( gs_encodings[i] == encoding )
        {
            return wx::unsafe::StrViewSplit(gs_encodingNames[i], ',');
        }
    }

    return {};
}

/* static */
wxFontEncoding wxFontMapperBase::GetEncodingFromName(const std::string& name)
{
    int i{0};

    for(const auto encName : gs_encodingNames)
    {
        const auto name_match = std::search(encName.cbegin(), encName.cend(), name.cbegin(), name.cend(), 
            [](const auto& enc_ch, const auto& name_ch){
                return std::towupper(enc_ch) == std::towupper(name_ch);
            });
        
        if(name_match != encName.cend())
            return gs_encodings[i];

        ++i;
    }

    return wxFONTENCODING_MAX;
}

#endif // wxUSE_FONTMAP
