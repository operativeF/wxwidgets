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

#include "wx/fontenc.h"         // for wxFontEncoding

#if wxUSE_GUI
    #include "wx/fontutil.h"    // for wxNativeEncodingInfo
#endif // wxUSE_GUI

#include <string_view>

#if wxUSE_CONFIG && wxUSE_FILECONFIG
    class WXDLLIMPEXP_FWD_BASE wxConfigBase;
#endif // wxUSE_CONFIG

class WXDLLIMPEXP_FWD_CORE wxFontMapper;

#if wxUSE_GUI
    class WXDLLIMPEXP_FWD_CORE wxWindow;
#endif // wxUSE_GUI

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

class WXDLLIMPEXP_BASE wxFontMapperBase
{
public:
    // constructor and such
    // ---------------------

    // default ctor
    wxFontMapperBase() = default;

    // virtual dtor for any base class
    virtual ~wxFontMapperBase();

    wxFontMapperBase(const wxFontMapperBase&) = delete;
    wxFontMapperBase& operator=(const wxFontMapperBase&) = delete;
    wxFontMapperBase(wxFontMapperBase&&) = default;
    wxFontMapperBase& operator=(wxFontMapperBase&&) = default;
    
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

    // get the number of font encodings we know about
    static size_t GetSupportedEncodingsCount();

    // get the n-th supported encoding
    static wxFontEncoding GetEncoding(size_t n);

    // return canonical name of this encoding (this is a short string,
    // GetEncodingDescription() returns a longer one)
    static std::string GetEncodingName(wxFontEncoding encoding);

    // return a list of all names of this encoding (see GetEncodingName)
    static std::vector<std::string_view> GetAllEncodingNames(wxFontEncoding encoding);

    // return user-readable string describing the given encoding
    //
    // NB: hard-coded now, but might change later (read it from config?)
    static std::string GetEncodingDescription(wxFontEncoding encoding);

    // find the encoding corresponding to the given name, inverse of
    // GetEncodingName() and less general than CharsetToEncoding()
    //
    // returns wxFONTENCODING_MAX if the name is not a supported encoding
    static wxFontEncoding GetEncodingFromName(const std::string& name);


    // functions which allow to configure the config object used: by default,
    // the global one (from wxConfigBase::Get() will be used) and the default
    // root path for the config settings is the string returned by
    // GetDefaultConfigPath()
    // ----------------------------------------------------------------------

#if wxUSE_CONFIG && wxUSE_FILECONFIG
    // set the root config path to use (should be an absolute path)
    void SetConfigPath(const std::string& prefix);

    // return default config path
    static const std::string& GetDefaultConfigPath();
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
    bool ChangePath(const wxString& pathNew, wxString* pathOld);

    // restore the config path after use
    void RestorePath(const wxString& pathOld);

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

class WXDLLIMPEXP_CORE wxFontMapper : public wxFontMapperBase
{
public:
    // default ctor
    wxFontMapper() = default;

    // virtual dtor for a base class
    ~wxFontMapper() = default;

    wxFontMapper(const wxFontMapper&) = delete;
    wxFontMapper& operator=(const wxFontMapper&) = delete;
    wxFontMapper(wxFontMapper&&) = default;
    wxFontMapper& operator=(wxFontMapper&&) = default;

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

