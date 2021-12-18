/////////////////////////////////////////////////////////////////////////////
// Name:        wx/translation.h
// Purpose:     Internationalization and localisation for wxWidgets
// Author:      Vadim Zeitlin, Vaclav Slavik,
//              Michael N. Filippov <michael@idisys.iae.nsk.su>
// Created:     2010-04-23
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
//              (c) 2010 Vaclav Slavik <vslavik@fastmail.fm>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TRANSLATION_H_
#define _WX_TRANSLATION_H_

#include "wx/string.h"

#if wxUSE_INTL

#include "wx/buffer.h"
#include "wx/language.h"
#include "wx/hashmap.h"
#include "wx/strconv.h"
#include "wx/scopedptr.h"

import WX.WinDef;

import <unordered_map>;
import <vector>;

// ============================================================================
// global decls
// ============================================================================

// ----------------------------------------------------------------------------
// macros
// ----------------------------------------------------------------------------

// gettext() style macros (notice that xgettext should be invoked with
// --keyword="_" --keyword="wxPLURAL:1,2" options
// to extract the strings from the sources)
#ifndef WXINTL_NO_GETTEXT_MACRO
#ifndef wxNO_IMPLICIT_WXSTRING_ENCODING
    #define _(s)                               wxGetTranslation((s))
#else
    #define _(s)                               wxGetTranslation(s)
#endif
    #define wxPLURAL(sing, plur, n)            wxGetTranslation((sing), (plur), n)
#endif

// wx-specific macro for translating strings in the given context: if you use
// them, you need to also add
// --keyword="wxGETTEXT_IN_CONTEXT:1c,2" --keyword="wxGETTEXT_IN_CONTEXT_PLURAL:1c,2,3"
// options to xgettext invocation.
#ifndef wxNO_IMPLICIT_WXSTRING_ENCODING
#define wxGETTEXT_IN_CONTEXT(c, s) \
    wxGetTranslation((s), std::string(), c)
#else
#define wxGETTEXT_IN_CONTEXT(c, s) \
    wxGetTranslation(s, std::string(), c)
#endif
#define wxGETTEXT_IN_CONTEXT_PLURAL(c, sing, plur, n) \
    wxGetTranslation((sing), (plur), n, std::string(), c)

// another one which just marks the strings for extraction, but doesn't
// perform the translation (use -kwxTRANSLATE with xgettext!)
#define wxTRANSLATE(str) str

// ----------------------------------------------------------------------------
// forward decls
// ----------------------------------------------------------------------------

class wxTranslationsLoader;
class wxLocale;

class wxPluralFormsCalculator;
wxDECLARE_SCOPED_PTR(wxPluralFormsCalculator, wxPluralFormsCalculatorPtr)

// ----------------------------------------------------------------------------
// wxMsgCatalog corresponds to one loaded message catalog.
// ----------------------------------------------------------------------------

class wxMsgCatalog
{
public:
    // Ctor is protected, because CreateFromXXX functions must be used,
    // but destruction should be unrestricted

    // load the catalog from disk or from data; caller is responsible for
    // deleting them if not NULL
    static wxMsgCatalog *CreateFromFile(const std::string& filename,
                                        const std::string& domain);

    static wxMsgCatalog *CreateFromData(const wxScopedCharBuffer& data,
                                        const std::string& domain);

    // get name of the catalog
    wxString GetDomain() const { return m_domain; }

    // get the translated string: returns nullptr if not found
    const std::string* GetString(const std::string& sz, unsigned n = UINT_MAX, const std::string& ct = {}) const;

protected:
    wxMsgCatalog(const wxString& domain)
        : m_pNext(nullptr), m_domain(domain)
    {}

private:
    // variable pointing to the next element in a linked list (or NULL)
    wxMsgCatalog *m_pNext;
    friend class wxTranslations;

    wxStringToStringHashMap m_messages; // all messages in the catalog
    std::string             m_domain;   // name of the domain

    wxPluralFormsCalculatorPtr m_pluralFormsCalculator;
};

// ----------------------------------------------------------------------------
// wxTranslations: message catalogs
// ----------------------------------------------------------------------------

// this class allows to get translations for strings
class wxTranslations
{
public:
    wxTranslations();
    ~wxTranslations();

    // returns current translations object, may return NULL
    static wxTranslations *Get();
    // sets current translations object (takes ownership; may be NULL)
    static void Set(wxTranslations *t);

    // changes loader to non-default one; takes ownership of 'loader'
    void SetLoader(wxTranslationsLoader *loader);

    void SetLanguage(wxLanguage lang);
    void SetLanguage(const std::string& lang);

    // get languages available for this app
    std::vector<std::string> GetAvailableTranslations(const std::string& domain) const;

    // find best translation language for given domain
    std::string GetBestTranslation(const std::string& domain, wxLanguage msgIdLanguage);
    std::string GetBestTranslation(const std::string& domain,
                                   const std::string& msgIdLanguage = wxASCII_STR("en"));

    // add standard wxWidgets catalog ("wxstd")
    bool AddStdCatalog();

    // add catalog with given domain name and language, looking it up via
    // wxTranslationsLoader
    bool AddCatalog(const std::string& domain,
                    wxLanguage msgIdLanguage = wxLANGUAGE_ENGLISH_US);

    // check if the given catalog is loaded
    bool IsLoaded(const std::string& domain) const;

    // access to translations
    const std::string *GetTranslatedString(const std::string& origString,
                                        const std::string& domain = {},
                                        const std::string& context = {}) const;
    const std::string *GetTranslatedString(const std::string& origString,
                                        unsigned n,
                                        const std::string& domain = {},
                                        const std::string& context = {}) const;

    std::string GetHeaderValue(const std::string& header,
                            const std::string& domain = {}) const;

    // this is hack to work around a problem with wxGetTranslation() which
    // returns const std::string& and not std::string, so when it returns untranslated
    // string, it needs to have a copy of it somewhere
    static const std::string& GetUntranslatedString(const std::string& str);

private:
    // perform loading of the catalog via m_loader
    bool LoadCatalog(const std::string& domain, const std::string& lang, const std::string& msgIdLang);

    // find catalog by name in a linked list, return NULL if !found
    wxMsgCatalog *FindCatalog(const std::string& domain) const;

    // same as Set(), without taking ownership; only for wxLocale
    static void SetNonOwned(wxTranslations *t);
    friend class wxLocale;

private:
    std::string m_lang;
    wxTranslationsLoader *m_loader;

    wxMsgCatalog *m_pMsgCat{nullptr}; // pointer to linked list of catalogs

    // In addition to keeping all the catalogs in the linked list, we also
    // store them in a hash map indexed by the domain name to allow finding
    // them by name efficiently.
    using wxMsgCatalogMap = std::unordered_map< std::string, wxMsgCatalog *, wxStringHash, wxStringEqual >;
    wxMsgCatalogMap m_catalogMap;
};


// abstraction of translations discovery and loading
class wxTranslationsLoader
{
public:
    virtual ~wxTranslationsLoader() = default;

    virtual wxMsgCatalog *LoadCatalog(const std::string& domain,
                                      const std::string& lang) = 0;

    virtual std::vector<std::string> GetAvailableTranslations(const std::string& domain) const = 0;
};


// standard wxTranslationsLoader implementation, using filesystem
class wxFileTranslationsLoader
    : public wxTranslationsLoader
{
public:
    static void AddCatalogLookupPathPrefix(const std::string& prefix);

    wxMsgCatalog *LoadCatalog(const std::string& domain,
                              const std::string& lang) override;

    std::vector<std::string> GetAvailableTranslations(const std::string& domain) const override;
};


#ifdef WX_WINDOWS
// loads translations from win32 resources
class wxResourceTranslationsLoader
    : public wxTranslationsLoader
{
public:
    wxMsgCatalog *LoadCatalog(const std::string& domain,
                              const std::string& lang) override;

    std::vector<std::string> GetAvailableTranslations(const std::string& domain) const override;

protected:
    // returns resource type to use for translations
    virtual std::string GetResourceType() const { return "MOFILE"; }

    // returns module to load resources from
    virtual WXHINSTANCE GetModule() const { return nullptr; }
};
#endif // WX_WINDOWS


// ----------------------------------------------------------------------------
// global functions
// ----------------------------------------------------------------------------

// get the translation of the string in the current locale
inline const std::string& wxGetTranslation(const std::string& str,
                                           const std::string& domain = {},
                                           const std::string& context = {})
{
    wxTranslations* trans = wxTranslations::Get();
    const std::string* transStr = trans ? trans->GetTranslatedString(str, domain, context)
                                     : nullptr;
    if ( transStr )
        return *transStr;
    else
        // NB: this function returns reference to a string, so we have to keep
        //     a copy of it somewhere
        return wxTranslations::GetUntranslatedString(str);
}

inline const std::string& wxGetTranslation(const std::string& str1,
                                        const std::string& str2,
                                        unsigned n,
                                        const std::string& domain = {},
                                        const std::string& context = {})
{
    wxTranslations *trans = wxTranslations::Get();
    const std::string* transStr = trans ? trans->GetTranslatedString(str1, n, domain, context)
                                     : nullptr;
    if ( transStr )
        return *transStr;
    else
        // NB: this function returns reference to a string, so we have to keep
        //     a copy of it somewhere
        return n == 1
               ? wxTranslations::GetUntranslatedString(str1)
               : wxTranslations::GetUntranslatedString(str2);
}

#ifdef wxNO_IMPLICIT_WXSTRING_ENCODING

/*
 * It must always be possible to call wxGetTranslation() with const
 * char* arguments.
 */
inline const wxString& wxGetTranslation(const char *str,
                                        const char *domain = {},
                                        const char *context = {}) {
    const wxMBConv &conv = wxConvWhateverWorks;
    return wxGetTranslation(wxString(str, conv), wxString(domain, conv),
                            wxString(context, conv));
}

inline const wxString& wxGetTranslation(const char *str1,
                                        const char *str2,
                                        unsigned n,
                                        const char *domain = {},
                                        const char *context = {}) {
    const wxMBConv &conv = wxConvWhateverWorks;
    return wxGetTranslation(wxString(str1, conv), wxString(str2, conv), n,
                            wxString(domain, conv),
                            wxString(context, conv));
}

#endif // wxNO_IMPLICIT_WXSTRING_ENCODING

#else // !wxUSE_INTL

// the macros should still be defined - otherwise compilation would fail

#if !defined(WXINTL_NO_GETTEXT_MACRO)
    #if !defined(_)
#ifndef wxNO_IMPLICIT_WXSTRING_ENCODING
        #define _(s)                 (s)
#else
        #define _(s)                 wxASCII_STR(s)
#endif
    #endif
    #define wxPLURAL(sing, plur, n)  ((n) == 1 ? (sing) : (plur))
    #define wxGETTEXT_IN_CONTEXT(c, s)                     (s)
    #define wxGETTEXT_IN_CONTEXT_PLURAL(c, sing, plur, n)  wxPLURAL(sing, plur, n)
#endif

#define wxTRANSLATE(str) str

// NB: we use a template here in order to avoid using
//     wxLocale::GetUntranslatedString() above, which would be required if
//     we returned const wxString&; this way, the compiler should be able to
//     optimize wxGetTranslation() away

template<typename TString>
inline TString wxGetTranslation(TString str)
    { return str; }

template<typename TString, typename TDomain>
inline TString wxGetTranslation(TString str, [[maybe_unused]] TDomain domain)
    { return str; }

template<typename TString, typename TDomain, typename TContext>
inline TString wxGetTranslation(TString str, [[maybe_unused]] TDomain domain, [[maybe_unused]] TContext context)
    { return str; }

template<typename TString, typename TDomain>
inline TString wxGetTranslation(TString str1, TString str2, size_t n)
    { return n == 1 ? str1 : str2; }

template<typename TString, typename TDomain>
inline TString wxGetTranslation(TString str1, TString str2, size_t n,
                                [[maybe_unused]] TDomain domain)
    { return n == 1 ? str1 : str2; }

template<typename TString, typename TDomain, typename TContext>
inline TString wxGetTranslation(TString str1, TString str2, size_t n,
                                [[maybe_unused]] TDomain domain,
                                [[maybe_unused]] TContext context)
    { return n == 1 ? str1 : str2; }

#endif // wxUSE_INTL/!wxUSE_INTL

// define this one just in case it occurs somewhere (instead of preferred
// wxTRANSLATE) too
#if !defined(WXINTL_NO_GETTEXT_MACRO)
    #if !defined(gettext_noop)
        #define gettext_noop(str) (str)
    #endif
    #if !defined(N_)
        #define N_(s)             (s)
    #endif
#endif

#endif // _WX_TRANSLATION_H_
