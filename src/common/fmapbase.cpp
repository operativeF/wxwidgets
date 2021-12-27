///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/fmapbase.cpp
// Purpose:     wxFontMapperBase class implementation
// Author:      Vadim Zeitlin
// Modified by:
// Created:     21.06.2003 (extracted from common/fontmap.cpp)
// Copyright:   (c) 1999-2003 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_FONTMAP

#if defined(WX_WINDOWS)
    #include  "wx/msw/private.h"  // includes windows.h for LOGFONTW
#endif

#include "wx/app.h"
#include "wx/log.h"
#include "wx/intl.h"
#include "wx/module.h"
#include "wx/wxcrtvararg.h"
#include "wx/fontmap.h"
#include "wx/fmappriv.h"

#include "wx/apptrait.h"

// wxMemoryConfig uses wxFileConfig
#if wxUSE_CONFIG && wxUSE_FILECONFIG
    #include "wx/config.h"
    #include "wx/memconf.h"
#endif

#include <cwctype>

import Utils.Strings;

import <array>;
import <string>;
import <string_view>;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

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
                            "wxAppTraits::CreateFontMapper() failed" );
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
std::string wxFontMapperBase::GetDefaultConfigPath()
{
    return gs_defaultConfigPath;
}

void wxFontMapperBase::SetConfigPath(const std::string& prefix)
{
    wxCHECK_RET( !prefix.empty() && prefix[0] == wxCONFIG_PATH_SEPARATOR,
                 "an absolute path should be given to wxFontMapper::SetConfigPath()" );

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

bool wxFontMapperBase::ChangePath(const std::string& pathNew, std::string* pathOld)
{
    wxConfigBase *config = GetConfig();
    if ( !config )
        return false;

    *pathOld = config->GetPath();

    std::string path = GetConfigPath();
    if ( path.empty() || path.back() != wxCONFIG_PATH_SEPARATOR )
    {
        path += wxCONFIG_PATH_SEPARATOR;
    }

    // FIXME: Stupid.
    //wxASSERT_MSG( pathNew.empty() || (pathNew.Front() != wxCONFIG_PATH_SEPARATOR),
    //              "should be a relative path" );

    path += pathNew;

    config->SetPath(path);

    return true;
}

void wxFontMapperBase::RestorePath(const std::string& pathOld)
{
    GetConfig()->SetPath(pathOld);
}

#endif

// ----------------------------------------------------------------------------
// charset/encoding correspondence
// ----------------------------------------------------------------------------

wxFontEncoding
wxFontMapperBase::CharsetToEncoding(const std::string& charset,
                                    [[maybe_unused]] bool interactive)
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
    if(charset.empty())
        return wxFONTENCODING_DEFAULT;

    wxFontEncoding encoding = wxFONTENCODING_SYSTEM;

    // we're going to modify it, make a copy
    std::string cs = charset;

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
                return wxFONTENCODING_UNKNOWN;
            }

            if ( value >= 0 && value <= wxFONTENCODING_MAX )
            {
                encoding = (wxFontEncoding)value;
            }
            else
            {
                wxLogDebug("corrupted config data: invalid encoding %ld for charset '%s' ignored",
                           value, charset.c_str());
            }
        }

        if ( encoding == wxFONTENCODING_SYSTEM )
        {
            // may be we have an alias?
            config->SetPath(FONTMAPPER_CHARSET_ALIAS_PATH);

            std::string alias = config->Read(charset);
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
        wx::utils::TrimAllSpace(cs);

        // discard the optional quotes
        if ( !cs.empty() )
        {
            if ( cs.front() == '"' && cs.back() == '"' )
            {
                cs = cs.substr(1, cs.length() - 1);
            }
        }

        wx::utils::ToUpper(cs);

        for ( const auto& encFamily : gs_encodings )
        {
            if (encFamily.names.find(cs) != std::string_view::npos)
                return encFamily.encoding;
        }

        if ( cs.substr(0, 3) == "ISO" )
        {
            // the dash is optional (or, to be exact, it is not, but
            // several broken programs "forget" it)
            const char* p = cs.data() + 3;
            if ( *p == '-' )
                p++;

            unsigned int value;

            if ( wxSscanf(p, "8859-%u", &value) == 1 )
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
        else if ( cs.substr(0, 4) == "8859" )
        {
            const char* p = cs.data();

            unsigned int value;
            if ( wxSscanf(p, "8859-%u", &value) == 1 )
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
            size_t len{};
            if ( cs.substr(0, 7) == "WINDOWS" )
            {
                len = 7;
            }
            else if ( cs.substr(0, 2) == "CP" )
            {
                len = 2;
            }
            //else not a Windows encoding


            if ( len )
            {
                const char* p = cs.data() + len;
                if ( *p == '-' )
                    p++;

                unsigned int value;
                if ( wxSscanf(p, "%u", &value) == 1 )
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
std::string wxFontMapperBase::GetEncodingDescription(wxFontEncoding encoding)
{
    if ( encoding == wxFONTENCODING_DEFAULT )
    {
        return _("Default encoding");
    }

    auto possibleMatch = std::ranges::find_if(gs_encodings,
        [encoding](const auto& enc){
            return enc.encoding == encoding;
        }
    );

    if(possibleMatch != gs_encodings.end())
    {
        return wxGetTranslation(std::string{possibleMatch->description});
    }

    return fmt::format(_("Unknown encoding (%d)"), encoding);
}

/* static */
std::string wxFontMapperBase::GetEncodingName(wxFontEncoding encoding)
{
    if ( encoding == wxFONTENCODING_DEFAULT )
    {
        return _("default");
    }

    auto possibleMatch = std::ranges::find_if(gs_encodings,
        [encoding](const auto& enc){
            return enc.encoding == encoding;
        }
    );

    if(possibleMatch != gs_encodings.end())
    {
        return std::string{possibleMatch->names.substr(0, possibleMatch->names.find_first_of(','))};
    }

    return fmt::format(_("unknown-%d"), encoding);
}

#endif // wxUSE_FONTMAP
