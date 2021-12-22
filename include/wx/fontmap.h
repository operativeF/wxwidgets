/////////////////////////////////////////////////////////////////////////////
// Name:        wx/fontmap.h
// Purpose:     wxFontMapper class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     04.11.99
// Copyright:   (c) Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FONTMAPPER_H_
#define _WX_FONTMAPPER_H_

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#if wxUSE_FONTMAP

import WX.Base.FontEnc;
#include "wx/translation.h"

#if wxUSE_GUI
    #include "wx/fontutil.h"    // for wxNativeEncodingInfo
#endif // wxUSE_GUI

import Utils.Chars;
import Utils.Strings;

import <array>;
import <string>;
import <string_view>;
import <vector>;

#if wxUSE_CONFIG && wxUSE_FILECONFIG
    class wxConfigBase;
#endif // wxUSE_CONFIG

class wxFontMapper;

#if wxUSE_GUI
    class wxWindow;
#endif // wxUSE_GUI

struct EncodingInfo
{
    wxFontEncoding   encoding;
    std::string_view description;
    std::string_view names;
};

// although koi8-ru is not strictly speaking the same as koi8-r,
// they are similar enough to make mapping it to koi8 better than
// not recognizing it at all

// encodings supported by GetEncodingDescription
inline constexpr std::array<EncodingInfo, 83> gs_encodings =
{{
    { wxFONTENCODING_ISO8859_1,         wxTRANSLATE("Western European (ISO-8859-1)"), "ISO-8859-1,ISO8859-1,iso88591,8859-1,iso_8859_1" },
    { wxFONTENCODING_ISO8859_2,         wxTRANSLATE("Central European (ISO-8859-2)"), "ISO-8859-2,ISO8859-2,iso88592,8859-2" },
    { wxFONTENCODING_ISO8859_3,         wxTRANSLATE("Esperanto (ISO-8859-3)"), "ISO-8859-3,ISO8859-3,iso88593,8859-3" },
    { wxFONTENCODING_ISO8859_4,         wxTRANSLATE("Baltic (old) (ISO-8859-4)"), "ISO-8859-4,ISO8859-4,iso88594,8859-4" },
    { wxFONTENCODING_ISO8859_5,         wxTRANSLATE("Cyrillic (ISO-8859-5)"), "ISO-8859-5,ISO8859-5,iso88595,8859-5" },
    { wxFONTENCODING_ISO8859_6,         wxTRANSLATE("Arabic (ISO-8859-6)"), "ISO-8859-6,ISO8859-6,iso88596,8859-6" },
    { wxFONTENCODING_ISO8859_7,         wxTRANSLATE("Greek (ISO-8859-7)"), "ISO-8859-7,ISO8859-7,iso88597,8859-7" },
    { wxFONTENCODING_ISO8859_8,         wxTRANSLATE("Hebrew (ISO-8859-8)"), "ISO-8859-8,ISO8859-8,iso88598,8859-8" },
    { wxFONTENCODING_ISO8859_9,         wxTRANSLATE("Turkish (ISO-8859-9)"), "ISO-8859-9,ISO8859-9,iso88599,8859-9" },
    { wxFONTENCODING_ISO8859_10,        wxTRANSLATE("Nordic (ISO-8859-10)"), "ISO-8859-10,ISO8859-10,iso885910,8859-10" },
    { wxFONTENCODING_ISO8859_11,        wxTRANSLATE("Thai (ISO-8859-11)"), "ISO-8859-11,ISO8859-11,iso885911,8859-11" },
    { wxFONTENCODING_ISO8859_12,        wxTRANSLATE("Indian (ISO-8859-12)"), "ISO-8859-12,ISO8859-12,iso885912,8859-12" },
    { wxFONTENCODING_ISO8859_13,        wxTRANSLATE("Baltic (ISO-8859-13)"), "ISO-8859-13,ISO8859-13,iso885913,8859-13" },
    { wxFONTENCODING_ISO8859_14,        wxTRANSLATE("Celtic (ISO-8859-14)"), "ISO-8859-14,ISO8859-14,iso885914,8859-14" },
    { wxFONTENCODING_ISO8859_15,        wxTRANSLATE("Western European with Euro (ISO-8859-15)"), "ISO-8859-15,ISO8859-15,iso885915,8859-15" },
    { wxFONTENCODING_KOI8,              wxTRANSLATE("KOI8-R"), "KOI8-R,KOI8-RU" },
    { wxFONTENCODING_KOI8_U,            wxTRANSLATE("KOI8-U"), "KOI8-U" },
    { wxFONTENCODING_CP866,             wxTRANSLATE("Windows/DOS OEM Cyrillic (CP 866)"), "WINDOWS-866,CP866" },
    { wxFONTENCODING_CP874,             wxTRANSLATE("Windows Thai (CP 874)"), "WINDOWS-874,CP874,MS874,IBM-874" },
    { wxFONTENCODING_CP932,             wxTRANSLATE("Windows Japanese (CP 932) or Shift-JIS"), "WINDOWS-932,CP932,MS932,IBM-932,SJIS,SHIFT-JIS,SHIFT_JIS" },
    { wxFONTENCODING_CP936,             wxTRANSLATE("Windows Chinese Simplified (CP 936) or GB-2312"), "WINDOWS-936,CP936,MS936,IBM-936,GB2312,gbk,GBK" },
    { wxFONTENCODING_CP949,             wxTRANSLATE("Windows Korean (CP 949)"), "WINDOWS-949,CP949,MS949,IBM-949,EUC-KR,eucKR,euc_kr" },
    { wxFONTENCODING_CP950,             wxTRANSLATE("Windows Chinese Traditional (CP 950) or Big-5"), "WINDOWS-950,CP950,MS950,IBM-950,BIG5,BIG-5,BIG-FIVE" },
    { wxFONTENCODING_CP1250,            wxTRANSLATE("Windows Central European (CP 1250)"), "WINDOWS-1250,CP1250,MS1250,IBM-1250" },
    { wxFONTENCODING_CP1251,            wxTRANSLATE("Windows Cyrillic (CP 1251)"), "WINDOWS-1251,CP1251,MS1251,IBM-1251" },
    { wxFONTENCODING_CP1252,            wxTRANSLATE("Windows Western European (CP 1252)"), "WINDOWS-1252,CP1252,MS1252,IBM-1252" },
    { wxFONTENCODING_CP1253,            wxTRANSLATE("Windows Greek (CP 1253)"), "WINDOWS-1253,CP1253,MS1253,IBM-1253" },
    { wxFONTENCODING_CP1254,            wxTRANSLATE("Windows Turkish (CP 1254)"), "WINDOWS-1254,CP1254,MS1254,IBM-1254" },
    { wxFONTENCODING_CP1255,            wxTRANSLATE("Windows Hebrew (CP 1255)"), "WINDOWS-1255,CP1255,MS1255,IBM-1255" },
    { wxFONTENCODING_CP1256,            wxTRANSLATE("Windows Arabic (CP 1256)"), "WINDOWS-1256,CP1256,MS1256,IBM-1256" },
    { wxFONTENCODING_CP1257,            wxTRANSLATE("Windows Baltic (CP 1257)"), "WINDOWS-1257,CP1257,MS1257,IBM-1257" },
    { wxFONTENCODING_CP1258,            wxTRANSLATE("Windows Vietnamese (CP 1258)"), "WINDOWS-1258,CP1258,MS1258,IBM-1258" },
    { wxFONTENCODING_CP1361,            wxTRANSLATE("Windows Johab (CP 1361)"), "WINDOWS-1361,CP1361,MS1361,IBM-1361,JOHAB" },
    { wxFONTENCODING_CP437,             wxTRANSLATE("Windows/DOS OEM (CP 437)"), "WINDOWS-437,CP437,MS437,IBM-437" },
    { wxFONTENCODING_UTF7,              wxTRANSLATE("Unicode 7 bit (UTF-7)"), "UTF-7,UTF7" },
    { wxFONTENCODING_UTF8,              wxTRANSLATE("Unicode 8 bit (UTF-8)"), "UTF-8,UTF8" },
#ifdef WORDS_BIGENDIAN
    { wxFONTENCODING_UTF16BE,          wxTRANSLATE("Unicode 16 bit (UTF-16)"),                 "UTF-16BE,UTF16BE,UCS-2BE,UCS2BE,UTF-16,UTF16,UCS-2,UCS2" },
    { wxFONTENCODING_UTF16LE,          wxTRANSLATE("Unicode 16 bit Little Endian (UTF-16LE)"), "UTF-16LE,UTF16LE,UCS-2LE,UCS2LE" },
    { wxFONTENCODING_UTF32BE,          wxTRANSLATE("Unicode 32 bit (UTF-32)"),                 "UTF-32BE,UTF32BE,UCS-4BE,UTF-32,UTF32,UCS-4,UCS4" },
    { wxFONTENCODING_UTF32LE,          wxTRANSLATE("Unicode 32 bit Little Endian (UTF-32LE)"), "UTF-32LE,UTF32LE,UCS-4LE,UCS4LE" },
#else // !WORDS_BIGENDIAN
    { wxFONTENCODING_UTF16BE,          wxTRANSLATE("Unicode 16 bit Big Endian (UTF-16BE)"), "UTF-16BE,UTF16BE,UCS-2BE,UCS2BE" },
    { wxFONTENCODING_UTF16LE,          wxTRANSLATE("Unicode 16 bit (UTF-16)"),              "UTF-16LE,UTF16LE,UCS-2LE,UTF-16,UTF16,UCS-2,UCS2" },
    { wxFONTENCODING_UTF32BE,          wxTRANSLATE("Unicode 32 bit Big Endian (UTF-32BE)"), "UTF-32BE,UTF32BE,UCS-4BE,UCS4BE" },
    { wxFONTENCODING_UTF32LE,          wxTRANSLATE("Unicode 32 bit (UTF-32)"),              "UTF-32LE,UTF32LE,UCS-4LE,UCS4LE,UTF-32,UTF32,UCS-4,UCS4" },
#endif // WORDS_BIGENDIAN
    { wxFONTENCODING_EUC_JP,           wxTRANSLATE("Extended Unix Codepage for Japanese (EUC-JP)"), "EUC-JP,eucJP,euc_jp,IBM-eucJP" },
    { wxFONTENCODING_DEFAULT,           wxTRANSLATE("US-ASCII"), "US-ASCII,ASCII,C,POSIX,ANSI_X3.4-1968,646,roman8" }, // 646 is for Solaris, roman8 -- for HP-UX
    { wxFONTENCODING_ISO2022_JP,           wxTRANSLATE("ISO-2022-JP"), "ISO-2022-JP" },
    { wxFONTENCODING_MACROMAN,           wxTRANSLATE("MacRoman"), "MacRoman" },
    { wxFONTENCODING_MACJAPANESE,           wxTRANSLATE("MacJapanese"), "MacJapanese" },
    { wxFONTENCODING_MACCHINESETRAD,           wxTRANSLATE("MacChineseTrad"), "MacChineseTrad" },
    { wxFONTENCODING_MACKOREAN,           wxTRANSLATE("MacKorean"), "MacKorean" },
    { wxFONTENCODING_MACARABIC,           wxTRANSLATE("MacArabic"), "MacArabic" },
    { wxFONTENCODING_MACHEBREW,           wxTRANSLATE("MacHebrew"), "MacHebrew" },
    { wxFONTENCODING_MACGREEK,           wxTRANSLATE("MacGreek"), "MacGreek" },
    { wxFONTENCODING_MACCYRILLIC,           wxTRANSLATE("MacCyrillic"), "MacCyrillic" },
    { wxFONTENCODING_MACDEVANAGARI,           wxTRANSLATE("MacDevanagari"), "MacDevanagari" },
    { wxFONTENCODING_MACGURMUKHI,           wxTRANSLATE("MacGurmukhi"), "MacGurmukhi" },
    { wxFONTENCODING_MACGUJARATI,           wxTRANSLATE("MacGujarati"), "MacGujarati" },
    { wxFONTENCODING_MACORIYA,           wxTRANSLATE("MacOriya"), "MacOriya" },
    { wxFONTENCODING_MACBENGALI,           wxTRANSLATE("MacBengali"), "MacBengali" },
    { wxFONTENCODING_MACTAMIL,           wxTRANSLATE("MacTamil"), "MacTamil" },
    { wxFONTENCODING_MACTELUGU,           wxTRANSLATE("MacTelugu"), "MacTelugu" },
    { wxFONTENCODING_MACKANNADA,           wxTRANSLATE("MacKannada"), "MacKannada" },
    { wxFONTENCODING_MACMALAJALAM,           wxTRANSLATE("MacMalayalam"), "MacMalayalam" },
    { wxFONTENCODING_MACSINHALESE,           wxTRANSLATE("MacSinhalese"), "MacSinhalese" },
    { wxFONTENCODING_MACBURMESE,           wxTRANSLATE("MacBurmese"), "MacBurmese" },
    { wxFONTENCODING_MACKHMER,           wxTRANSLATE("MacKhmer"), "MacKhmer" },
    { wxFONTENCODING_MACTHAI,           wxTRANSLATE("MacThai"), "MacThai" },
    { wxFONTENCODING_MACLAOTIAN,           wxTRANSLATE("MacLaotian"), "MacLaotian" },
    { wxFONTENCODING_MACGEORGIAN,           wxTRANSLATE("MacGeorgian"), "MacGeorgian" },
    { wxFONTENCODING_MACARMENIAN,           wxTRANSLATE("MacArmenian"), "MacArmenian" },
    { wxFONTENCODING_MACCHINESESIMP,           wxTRANSLATE("MacChineseSimp"), "MacChineseSimp" },
    { wxFONTENCODING_MACTIBETAN,           wxTRANSLATE("MacTibetan"), "MacTibetan" },
    { wxFONTENCODING_MACMONGOLIAN,           wxTRANSLATE("MacMongolian"), "MacMongolian" },
    { wxFONTENCODING_MACETHIOPIC,           wxTRANSLATE("MacEthiopic"), "MacEthiopic" },
    { wxFONTENCODING_MACCENTRALEUR,           wxTRANSLATE("MacCentralEurRoman"), "MacCentralEurRoman" },
    { wxFONTENCODING_MACVIATNAMESE,           wxTRANSLATE("MacVietnamese"), "MacVietnamese" },
    { wxFONTENCODING_MACARABICEXT,           wxTRANSLATE("MacExtArabic"), "MacExtArabic" },
    { wxFONTENCODING_MACSYMBOL,           wxTRANSLATE("MacSymbol"), "MacSymbol" },
    { wxFONTENCODING_MACDINGBATS,           wxTRANSLATE("MacDingbats"), "MacDingbats" },
    { wxFONTENCODING_MACTURKISH,           wxTRANSLATE("MacTurkish"), "MacTurkish" },
    { wxFONTENCODING_MACCROATIAN,           wxTRANSLATE("MacCroatian"), "MacCroatian" },
    { wxFONTENCODING_MACICELANDIC,           wxTRANSLATE("MacIcelandic"), "MacIcelandic" },
    { wxFONTENCODING_MACROMANIAN,           wxTRANSLATE("MacRomanian"), "MacRomanian" },
    { wxFONTENCODING_MACCELTIC,           wxTRANSLATE("MacCeltic"), "MacCeltic" },
    { wxFONTENCODING_MACGAELIC,           wxTRANSLATE("MacGaelic"), "MacGaelic" },
    { wxFONTENCODING_MACKEYBOARD,           wxTRANSLATE("MacKeyboardGlyphs"), "MacKeyboardGlyphs" }
}};


// ============================================================================
// wxFontMapper manages user-definable correspondence between wxWidgets font
// encodings and the fonts present on the machine.
//
// This is a singleton class, font mapper objects can only be accessed using
// wxFontMapper::Get().
// ============================================================================

// ----------------------------------------------------------------------------
// wxFontMapperBase: this is a non-interactive class which just uses its built
//                   in knowledge of the encodings equivalence
// ----------------------------------------------------------------------------

// get the n-th supported encoding
constexpr wxFontEncoding GetFontEncodingFromIndex(size_t n)
{
    wxCHECK_MSG( n < gs_encodings.size(), wxFONTENCODING_SYSTEM,
                    "wxFontMapper::GetEncoding(): invalid index" );

    return gs_encodings[n].encoding;
}

// get the number of font encodings we know about
constexpr std::size_t GetSupportedEncodingsCount()
{
    return gs_encodings.size();
}

// find the encoding corresponding to the given name, inverse of
// GetEncodingName() and less general than CharsetToEncoding()
//
// returns wxFONTENCODING_MAX if the name is not a supported encoding
constexpr wxFontEncoding GetEncodingFromName(std::string_view name)
{
    const auto NameCaps = wx::utils::ToUpperCopy(name);

    for(const auto& enc : gs_encodings)
    {
        const auto name_match = std::search(enc.names.begin(), enc.names.end(), NameCaps.begin(), NameCaps.end(), 
            [](const auto& enc_ch, const auto& name_ch){
                return wx::utils::ToUpperCh(enc_ch) == name_ch;
            });
        
        if(name_match != enc.names.cend())
            return enc.encoding;
    }

    return wxFONTENCODING_MAX;
}

constexpr std::vector<std::string_view> GetAllEncodingNames(wxFontEncoding encoding)
{
    auto possibleMatch = std::ranges::find_if(gs_encodings,
        [encoding](const auto& enc){
            return enc.encoding == encoding;
        }
    );

    if(possibleMatch != gs_encodings.end())
    {
        return wx::unsafe::StrViewSplit(possibleMatch->names, ',');
    }

    return {};
}

class wxFontMapperBase
{
public:
    // virtual dtor for any base class
    virtual ~wxFontMapperBase();

    wxFontMapperBase& operator=(wxFontMapperBase&&) = delete;
    
    // return instance of the wxFontMapper singleton
    // wxBase code only cares that it's a wxFontMapperBase
    // In wxBase, wxFontMapper is only forward declared
    // so one cannot implicitly cast from it to wxFontMapperBase.
    static wxFontMapperBase *Get();

    // set the singleton to 'mapper' instance and return previous one
    static wxFontMapper *Set(wxFontMapper *mapper);

    // delete the existing font mapper if any
    static void Reset();


    // translates charset strings to encoding
    // --------------------------------------

    // returns the encoding for the given charset (in the form of RFC 2046) or
    // wxFONTENCODING_SYSTEM if couldn't decode it
    //
    // interactive parameter is ignored in the base class, we behave as if it
    // were always false
    virtual wxFontEncoding CharsetToEncoding(const std::string& charset,
                                             bool interactive = true);

    // information about supported encodings
    // -------------------------------------

    // return canonical name of this encoding (this is a short string,
    // GetEncodingDescription() returns a longer one)
    static std::string GetEncodingName(wxFontEncoding encoding);

    // return a list of all names of this encoding (see GetEncodingName)
    static std::vector<std::string_view> GetAllEncodingNames(wxFontEncoding encoding);

    // return user-readable string describing the given encoding
    //
    // NB: hard-coded now, but might change later (read it from config?)
    static std::string GetEncodingDescription(wxFontEncoding encoding);

    // functions which allow to configure the config object used: by default,
    // the global one (from wxConfigBase::Get() will be used) and the default
    // root path for the config settings is the string returned by
    // GetDefaultConfigPath()
    // ----------------------------------------------------------------------

#if wxUSE_CONFIG && wxUSE_FILECONFIG
    // set the root config path to use (should be an absolute path)
    void SetConfigPath(const std::string& prefix);

    // return default config path
    static std::string GetDefaultConfigPath();
#endif // wxUSE_CONFIG


    // returns true for the base class and false for a "real" font mapper object
    // (implementation-only)
    virtual bool IsDummy() { return true; }

protected:
#if wxUSE_CONFIG && wxUSE_FILECONFIG
    // get the config object we're using -- either the global config object
    // or a wxMemoryConfig object created by this class otherwise
    wxConfigBase *GetConfig();

    // gets the root path for our settings -- if it wasn't set explicitly, use
    // GetDefaultConfigPath()
    const std::string& GetConfigPath();

    // change to the given (relative) path in the config, return true if ok
    // (then GetConfig() will return something !NULL), false if no config
    // object
    //
    // caller should provide a pointer to the string variable which should be
    // later passed to RestorePath()
    bool ChangePath(const std::string& pathNew, std::string* pathOld);

    // restore the config path after use
    void RestorePath(const std::string& pathOld);

    // config object and path (in it) to use
    wxConfigBase *m_configDummy{nullptr};

    std::string m_configRootPath;
#endif // wxUSE_CONFIG

    // the real implementation of the base class version of CharsetToEncoding()
    //
    // returns wxFONTENCODING_UNKNOWN if encoding is unknown and we shouldn't
    // ask the user about it, wxFONTENCODING_SYSTEM if it is unknown but we
    // should/could ask the user
    int NonInteractiveCharsetToEncoding(const std::string& charset);

private:
    // the global fontmapper object or nullptr
    inline static wxFontMapper *sm_instance{nullptr};

    friend class wxFontMapperPathChanger;
};

// ----------------------------------------------------------------------------
// wxFontMapper: interactive extension of wxFontMapperBase
//
// The default implementations of all functions will ask the user if they are
// not capable of finding the answer themselves and store the answer in a
// config file (configurable via SetConfigXXX functions). This behaviour may
// be disabled by giving the value of false to "interactive" parameter.
// However, the functions will always consult the config file to allow the
// user-defined values override the default logic and there is no way to
// disable this -- which shouldn't be ever needed because if "interactive" was
// never true, the config file is never created anyhow.
// ----------------------------------------------------------------------------

#if wxUSE_GUI

class wxFontMapper : public wxFontMapperBase
{
public:
    wxFontMapper& operator=(wxFontMapper&&) = delete;

    // working with the encodings
    // --------------------------

    // returns the encoding for the given charset (in the form of RFC 2046) or
    // wxFONTENCODING_SYSTEM if couldn't decode it
    wxFontEncoding CharsetToEncoding(const std::string& charset,
                                             bool interactive = true) override;

    // find an alternative for the given encoding (which is supposed to not be
    // available on this system). If successful, return true and fill info
    // structure with the parameters required to create the font, otherwise
    // return false
    virtual bool GetAltForEncoding(wxFontEncoding encoding,
                                   wxNativeEncodingInfo *info,
                                   const std::string& facename = {},
                                   bool interactive = true);

    // version better suitable for 'public' use. Returns wxFontEcoding
    // that can be used it wxFont ctor
    bool GetAltForEncoding(wxFontEncoding encoding,
                           wxFontEncoding *alt_encoding,
                           const std::string& facename = {},
                           bool interactive = true);

    // checks whether given encoding is available in given face or not.
    //
    // if no facename is given (default), return true if it's available in any
    // facename at alll.
    virtual bool IsEncodingAvailable(wxFontEncoding encoding,
                                     const std::string& facename = {});


    // configure the appearance of the dialogs we may popup
    // ----------------------------------------------------

    // the parent window for modal dialogs
    void SetDialogParent(wxWindow *parent) { m_windowParent = parent; }

    // the title for the dialogs (note that default is quite reasonable)
    void SetDialogTitle(const std::string& title) { m_titleDialog = title; }

    // GUI code needs to know it's a wxFontMapper because there
    // are additional methods in the subclass.
    static wxFontMapper *Get();

    // pseudo-RTTI since we aren't a wxObject.
    bool IsDummy() override { return false; }

protected:
    // GetAltForEncoding() helper: tests for the existence of the given
    // encoding and saves the result in config if ok - this results in the
    // following (desired) behaviour: when an unknown/unavailable encoding is
    // requested for the first time, the user is asked about a replacement,
    // but if he doesn't choose any and the default logic finds one, it will
    // be saved in the config so that the user won't be asked about it any
    // more
    bool TestAltEncoding(const std::string& configEntry,
                         wxFontEncoding encReplacement,
                         wxNativeEncodingInfo *info);

    // the title for our dialogs
    std::string m_titleDialog;

    // the parent window for our dialogs
    wxWindow *m_windowParent{nullptr};
};

#endif // wxUSE_GUI

#else // !wxUSE_FONTMAP

#if wxUSE_GUI
    // wxEncodingToCodepage (utils.cpp) needs wxGetNativeFontEncoding
    #include "wx/fontutil.h"
#endif

#endif // wxUSE_FONTMAP/!wxUSE_FONTMAP

#endif // _WX_FONTMAPPER_H_

