/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/translation.cpp
// Purpose:     Internationalization and localisation for wxWidgets
// Author:      Vadim Zeitlin, Vaclav Slavik,
//              Michael N. Filippov <michael@idisys.iae.nsk.su>
//              (2003/09/30 - PluralForms support)
// Created:     2010-04-23
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declaration
// ============================================================================

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_INTL

#ifndef WX_PRECOMP
    #include "wx/dynarray.h"
    #include "wx/string.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/utils.h"
    #include "wx/hashmap.h"
    #include "wx/module.h"
#endif // WX_PRECOMP

#include "wx/dir.h"
#include "wx/file.h"
#include "wx/filename.h"
#include "wx/tokenzr.h"
#include "wx/fontmap.h"
#include "wx/scopedptr.h"
#include "wx/stdpaths.h"
#include "wx/version.h"
#include "wx/private/threadinfo.h"

#ifdef __WINDOWS__
    #include "wx/dynlib.h"
    #include "wx/scopedarray.h"
    #include "wx/msw/wrapwin.h"
    #include "wx/msw/missing.h"
#endif
#ifdef __WXOSX__
    #include "wx/osx/core/cfstring.h"
    #include <CoreFoundation/CFBundle.h>
    #include <CoreFoundation/CFLocale.h>
#endif

#include <algorithm>
#include <memory>
#include <vector>

// ----------------------------------------------------------------------------
// simple types
// ----------------------------------------------------------------------------

using size_t32 = wxUint32;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// magic number identifying the .mo format file
constexpr size_t32 MSGCATALOG_MAGIC    = 0x950412de;
constexpr size_t32 MSGCATALOG_MAGIC_SW = 0xde120495;

constexpr wxChar TRACE_I18N[] = wxS("i18n");

// ============================================================================
// implementation
// ============================================================================

namespace
{

// ----------------------------------------------------------------------------
// Platform specific helpers
// ----------------------------------------------------------------------------

#if wxUSE_LOG_TRACE

// TODO: Use span / string_view
void LogTraceArray(const char *prefix, const std::vector<wxString>& arr)
{
    wxLogTrace(TRACE_I18N, "%s: [%s]", prefix, wxJoin(arr, ','));
}

// TODO: Use span / string_view
void LogTraceLargeArray(const wxString& prefix, const std::vector<wxString>& arr)
{
    wxLogTrace(TRACE_I18N, "%s:", prefix);
    for ( const auto& i : arr )
        wxLogTrace(TRACE_I18N, "    %s", i);
}

#else // !wxUSE_LOG_TRACE

#define LogTraceArray(prefix, arr)
#define LogTraceLargeArray(prefix, arr)

#endif // wxUSE_LOG_TRACE/!wxUSE_LOG_TRACE

// Use locale-based detection as a fallback
// TODO: Lambda
wxString GetPreferredUILanguageFallback(const std::vector<wxString>& WXUNUSED(available))
{
    wxString lang = wxLocale::GetLanguageCanonicalName(wxLocale::GetSystemLanguage());
    wxLogTrace(TRACE_I18N, " - obtained best language from locale: %s", lang);
    return lang;
}

#ifdef __WINDOWS__

wxString GetPreferredUILanguage(const std::vector<wxString>& available)
{
    typedef BOOL (WINAPI *GetUserPreferredUILanguages_t)(DWORD, PULONG, PWSTR, PULONG);
    static GetUserPreferredUILanguages_t s_pfnGetUserPreferredUILanguages = nullptr;
    static bool s_initDone = false;
    if ( !s_initDone )
    {
        wxLoadedDLL dllKernel32("kernel32.dll");
        wxDL_INIT_FUNC(s_pfn, GetUserPreferredUILanguages, dllKernel32);
        s_initDone = true;
    }

    if ( s_pfnGetUserPreferredUILanguages )
    {
        ULONG numLangs;
        ULONG bufferSize = 0;
        if ( (*s_pfnGetUserPreferredUILanguages)(MUI_LANGUAGE_NAME,
                                                 &numLangs,
                                                 nullptr,
                                                 &bufferSize) )
        {
            auto langs = std::make_unique<WCHAR[]>(bufferSize);
            
            if ( (*s_pfnGetUserPreferredUILanguages)(MUI_LANGUAGE_NAME,
                                                     &numLangs,
                                                     langs.get(),
                                                     &bufferSize) )
            {
                std::vector<wxString> preferred;

                WCHAR *buf = langs.get();
                for ( unsigned i = 0; i < numLangs; i++ )
                {
                    const wxString lang(buf);
                    preferred.push_back(lang);
                    buf += lang.length() + 1;
                }
                LogTraceArray(" - system preferred languages", preferred);

                for ( const auto& j : preferred )
                {
                    wxString lang(j);
                    lang.Replace("-", "_");
                    const auto isLang = std::find_if(available.cbegin(), available.cend(),
                        [lang](const auto& aLang){
                            return lang.IsSameAs(aLang, false);
                        });
                    if ( isLang != std::cend(available) )
                        return lang;

                    size_t pos = lang.find('_');

                    if ( pos != wxString::npos )
                    {
                        lang = lang.substr(0, pos);
                        const auto isSubLang = std::find_if(available.cbegin(), available.cend(),
                            [lang](const auto& aLang){
                                return lang.IsSameAs(aLang, false);
                            });
                        if ( isSubLang != std::cend(available) )
                            return lang;
                    }
                }
            }
        }
    }

    return GetPreferredUILanguageFallback(available);
}

#elif defined(__WXOSX__)

#if wxUSE_LOG_TRACE

void LogTraceArray(const char *prefix, CFArrayRef arr)
{
    wxString s;
    const unsigned count = CFArrayGetCount(arr);
    if ( count )
    {
        s += wxCFStringRef::AsString((CFStringRef)CFArrayGetValueAtIndex(arr, 0));
        for ( unsigned i = 1 ; i < count; i++ )
            s += "," + wxCFStringRef::AsString((CFStringRef)CFArrayGetValueAtIndex(arr, i));
    }
    wxLogTrace(TRACE_I18N, "%s: [%s]", prefix, s);
}

#endif // wxUSE_LOG_TRACE

wxString GetPreferredUILanguage(const std::vector<wxString>& available)
{
    wxStringToStringHashMap availableNormalized;
    wxCFRef<CFMutableArrayRef> availableArr(
        CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks));

    for ( auto i = available.begin();
          i != available.end();
          ++i )
    {
        wxString lang(*i);
        wxCFStringRef code_wx(*i);
        wxCFStringRef code_norm(
            CFLocaleCreateCanonicalLanguageIdentifierFromString(kCFAllocatorDefault, code_wx));
        CFArrayAppendValue(availableArr, code_norm);
        availableNormalized[code_norm.AsString()] = *i;
    }
    LogTraceArray(" - normalized available list", availableArr);

    wxCFRef<CFArrayRef> prefArr(
        CFBundleCopyLocalizationsForPreferences(availableArr, NULL));
    LogTraceArray(" - system preferred languages", prefArr);

    unsigned prefArrLength = CFArrayGetCount(prefArr);
    if ( prefArrLength > 0 )
    {
        // Lookup the name in 'available' by index -- we need to get the
        // original value corresponding to the normalized one chosen.
        wxString lang(wxCFStringRef::AsString((CFStringRef)CFArrayGetValueAtIndex(prefArr, 0)));
        wxStringToStringHashMap::const_iterator i = availableNormalized.find(lang);
        if ( i == availableNormalized.end() )
            return lang;
        else
            return i->second;
    }

    return GetPreferredUILanguageFallback(available);
}

#else

// When the preferred UI language is determined, the LANGUAGE environment
// variable is the primary source of preference.
// http://www.gnu.org/software/gettext/manual/html_node/Locale-Environment-Variables.html
//
// The LANGUAGE variable may contain a colon separated list of language
// codes in the order of preference.
// http://www.gnu.org/software/gettext/manual/html_node/The-LANGUAGE-variable.html
wxString GetPreferredUILanguage(const std::vector<wxString>& available)
{
    wxString languageFromEnv;
    std::vector<wxString> preferred;
    if ( wxGetEnv("LANGUAGE", &languageFromEnv) )
    {
        wxStringTokenizer tknzr(languageFromEnv, ":");
        while ( tknzr.HasMoreTokens() )
        {
            const wxString tok = tknzr.GetNextToken();
            if ( const wxLanguageInfo *li = wxLocale::FindLanguageInfo(tok) )
            {
                preferred.push_back(li->CanonicalName);
            }
        }
        if ( preferred.empty() )
        {
            wxLogTrace(TRACE_I18N, " - LANGUAGE was set, but it didn't contain any languages recognized by the system");
        }
    }

    LogTraceArray(" - preferred languages from environment", preferred);
    for ( auto j = preferred.begin();
          j != preferred.end();
          ++j )
    {
        wxString lang(*j);
        if ( available.Index(lang) != wxNOT_FOUND )
            return lang;
        size_t pos = lang.find('_');
        if ( pos != wxString::npos )
        {
            lang = lang.substr(0, pos);
            if ( available.Index(lang) != wxNOT_FOUND )
                return lang;
        }
    }

    return GetPreferredUILanguageFallback(available);
}

#endif

} // anonymous namespace

// ----------------------------------------------------------------------------
// Plural forms parser
// ----------------------------------------------------------------------------

/*
                                Simplified Grammar

Expression:
    LogicalOrExpression '?' Expression ':' Expression
    LogicalOrExpression

LogicalOrExpression:
    LogicalAndExpression "||" LogicalOrExpression   // to (a || b) || c
    LogicalAndExpression

LogicalAndExpression:
    EqualityExpression "&&" LogicalAndExpression    // to (a && b) && c
    EqualityExpression

EqualityExpression:
    RelationalExpression "==" RelationalExperession
    RelationalExpression "!=" RelationalExperession
    RelationalExpression

RelationalExpression:
    MultiplicativeExpression '>' MultiplicativeExpression
    MultiplicativeExpression '<' MultiplicativeExpression
    MultiplicativeExpression ">=" MultiplicativeExpression
    MultiplicativeExpression "<=" MultiplicativeExpression
    MultiplicativeExpression

MultiplicativeExpression:
    PmExpression '%' PmExpression
    PmExpression

PmExpression:
    N
    Number
    '(' Expression ')'
*/

class wxPluralFormsToken
{
public:
    enum Type
    {
        T_ERROR, T_EOF, T_NUMBER, T_N, T_PLURAL, T_NPLURALS, T_EQUAL, T_ASSIGN,
        T_GREATER, T_GREATER_OR_EQUAL, T_LESS, T_LESS_OR_EQUAL,
        T_REMINDER, T_NOT_EQUAL,
        T_LOGICAL_AND, T_LOGICAL_OR, T_QUESTION, T_COLON, T_SEMICOLON,
        T_LEFT_BRACKET, T_RIGHT_BRACKET
    };
    Type type() const { return m_type; }
    void setType(Type t) { m_type = t; }
    // for T_NUMBER only
    using Number = int;
    Number number() const { return m_number; }
    void setNumber(Number num) { m_number = num; }
private:
    Type m_type;
    Number m_number;
};


class wxPluralFormsScanner
{
public:
    explicit wxPluralFormsScanner(const char* s);
    const wxPluralFormsToken& token() const { return m_token; }
    bool nextToken();  // returns false if error
private:
    const char* m_s;
    wxPluralFormsToken m_token;
};

wxPluralFormsScanner::wxPluralFormsScanner(const char* s) : m_s(s)
{
    nextToken();
}

bool wxPluralFormsScanner::nextToken()
{
    wxPluralFormsToken::Type type = wxPluralFormsToken::T_ERROR;
    while (isspace((unsigned char) *m_s))
    {
        ++m_s;
    }
    if (*m_s == 0)
    {
        type = wxPluralFormsToken::T_EOF;
    }
    else if (isdigit((unsigned char) *m_s))
    {
        wxPluralFormsToken::Number number = *m_s++ - '0';
        while (isdigit((unsigned char) *m_s))
        {
            number = number * 10 + (*m_s++ - '0');
        }
        m_token.setNumber(number);
        type = wxPluralFormsToken::T_NUMBER;
    }
    else if (isalpha((unsigned char) *m_s))
    {
        const char* begin = m_s++;
        while (isalnum((unsigned char) *m_s))
        {
            ++m_s;
        }
        size_t size = m_s - begin;
        if (size == 1 && memcmp(begin, "n", size) == 0)
        {
            type = wxPluralFormsToken::T_N;
        }
        else if (size == 6 && memcmp(begin, "plural", size) == 0)
        {
            type = wxPluralFormsToken::T_PLURAL;
        }
        else if (size == 8 && memcmp(begin, "nplurals", size) == 0)
        {
            type = wxPluralFormsToken::T_NPLURALS;
        }
    }
    else if (*m_s == '=')
    {
        ++m_s;
        if (*m_s == '=')
        {
            ++m_s;
            type = wxPluralFormsToken::T_EQUAL;
        }
        else
        {
            type = wxPluralFormsToken::T_ASSIGN;
        }
    }
    else if (*m_s == '>')
    {
        ++m_s;
        if (*m_s == '=')
        {
            ++m_s;
            type = wxPluralFormsToken::T_GREATER_OR_EQUAL;
        }
        else
        {
            type = wxPluralFormsToken::T_GREATER;
        }
    }
    else if (*m_s == '<')
    {
        ++m_s;
        if (*m_s == '=')
        {
            ++m_s;
            type = wxPluralFormsToken::T_LESS_OR_EQUAL;
        }
        else
        {
            type = wxPluralFormsToken::T_LESS;
        }
    }
    else if (*m_s == '%')
    {
        ++m_s;
        type = wxPluralFormsToken::T_REMINDER;
    }
    else if (*m_s == '!' && m_s[1] == '=')
    {
        m_s += 2;
        type = wxPluralFormsToken::T_NOT_EQUAL;
    }
    else if (*m_s == '&' && m_s[1] == '&')
    {
        m_s += 2;
        type = wxPluralFormsToken::T_LOGICAL_AND;
    }
    else if (*m_s == '|' && m_s[1] == '|')
    {
        m_s += 2;
        type = wxPluralFormsToken::T_LOGICAL_OR;
    }
    else if (*m_s == '?')
    {
        ++m_s;
        type = wxPluralFormsToken::T_QUESTION;
    }
    else if (*m_s == ':')
    {
        ++m_s;
        type = wxPluralFormsToken::T_COLON;
    } else if (*m_s == ';') {
        ++m_s;
        type = wxPluralFormsToken::T_SEMICOLON;
    }
    else if (*m_s == '(')
    {
        ++m_s;
        type = wxPluralFormsToken::T_LEFT_BRACKET;
    }
    else if (*m_s == ')')
    {
        ++m_s;
        type = wxPluralFormsToken::T_RIGHT_BRACKET;
    }
    m_token.setType(type);
    return type != wxPluralFormsToken::T_ERROR;
}

class wxPluralFormsNode;

// NB: Can't use wxDEFINE_SCOPED_PTR_TYPE because wxPluralFormsNode is not
//     fully defined yet:
class wxPluralFormsNodePtr
{
public:
    explicit wxPluralFormsNodePtr(wxPluralFormsNode *p = nullptr) : m_p(p) {}
    ~wxPluralFormsNodePtr();
    wxPluralFormsNode& operator*() const { return *m_p; }
    wxPluralFormsNode* operator->() const { return m_p; }
    wxPluralFormsNode* get() const { return m_p; }
    wxPluralFormsNode* release();
    void reset(wxPluralFormsNode *p);

private:
    wxPluralFormsNode *m_p;
};

class wxPluralFormsNode
{
public:
    explicit wxPluralFormsNode(const wxPluralFormsToken& t) : m_token(t) {}
    const wxPluralFormsToken& token() const { return m_token; }
    const wxPluralFormsNode* node(unsigned i) const
        { return m_nodes[i].get(); }
    void setNode(unsigned i, wxPluralFormsNode* n);
    wxPluralFormsNode* releaseNode(unsigned i);
    wxPluralFormsToken::Number evaluate(wxPluralFormsToken::Number n) const;

private:
    wxPluralFormsToken m_token;
    wxPluralFormsNodePtr m_nodes[3];
};

wxPluralFormsNodePtr::~wxPluralFormsNodePtr()
{
    delete m_p;
}
wxPluralFormsNode* wxPluralFormsNodePtr::release()
{
    wxPluralFormsNode *p = m_p;
    m_p = nullptr;
    return p;
}
void wxPluralFormsNodePtr::reset(wxPluralFormsNode *p)
{
    if (p != m_p)
    {
        delete m_p;
        m_p = p;
    }
}


void wxPluralFormsNode::setNode(unsigned i, wxPluralFormsNode* n)
{
    m_nodes[i].reset(n);
}

wxPluralFormsNode*  wxPluralFormsNode::releaseNode(unsigned i)
{
    return m_nodes[i].release();
}

wxPluralFormsToken::Number
wxPluralFormsNode::evaluate(wxPluralFormsToken::Number n) const
{
    switch (token().type())
    {
        // leaf
        case wxPluralFormsToken::T_NUMBER:
            return token().number();
        case wxPluralFormsToken::T_N:
            return n;
        // 2 args
        case wxPluralFormsToken::T_EQUAL:
            return node(0)->evaluate(n) == node(1)->evaluate(n);
        case wxPluralFormsToken::T_NOT_EQUAL:
            return node(0)->evaluate(n) != node(1)->evaluate(n);
        case wxPluralFormsToken::T_GREATER:
            return node(0)->evaluate(n) > node(1)->evaluate(n);
        case wxPluralFormsToken::T_GREATER_OR_EQUAL:
            return node(0)->evaluate(n) >= node(1)->evaluate(n);
        case wxPluralFormsToken::T_LESS:
            return node(0)->evaluate(n) < node(1)->evaluate(n);
        case wxPluralFormsToken::T_LESS_OR_EQUAL:
            return node(0)->evaluate(n) <= node(1)->evaluate(n);
        case wxPluralFormsToken::T_REMINDER:
            {
                wxPluralFormsToken::Number number = node(1)->evaluate(n);
                if (number != 0)
                {
                    return node(0)->evaluate(n) % number;
                }
                else
                {
                    return 0;
                }
            }
        case wxPluralFormsToken::T_LOGICAL_AND:
            return node(0)->evaluate(n) && node(1)->evaluate(n);
        case wxPluralFormsToken::T_LOGICAL_OR:
            return node(0)->evaluate(n) || node(1)->evaluate(n);
        // 3 args
        case wxPluralFormsToken::T_QUESTION:
            return node(0)->evaluate(n)
                ? node(1)->evaluate(n)
                : node(2)->evaluate(n);
        default:
            return 0;
    }
}


class wxPluralFormsCalculator
{
public:
    // input: number, returns msgstr index
    int evaluate(int n) const;

    // input: text after "Plural-Forms:" (e.g. "nplurals=2; plural=(n != 1);"),
    // if s == 0, creates default handler
    // returns 0 if error
    static wxPluralFormsCalculator* make(const char* s = nullptr);

    void  init(wxPluralFormsToken::Number nplurals, wxPluralFormsNode* plural);

private:
    wxPluralFormsToken::Number m_nplurals{0};
    wxPluralFormsNodePtr m_plural{nullptr};
};

wxDEFINE_SCOPED_PTR(wxPluralFormsCalculator, wxPluralFormsCalculatorPtr)

void wxPluralFormsCalculator::init(wxPluralFormsToken::Number nplurals,
                                wxPluralFormsNode* plural)
{
    m_nplurals = nplurals;
    m_plural.reset(plural);
}

int wxPluralFormsCalculator::evaluate(int n) const
{
    if (m_plural.get() == nullptr)
    {
        return 0;
    }
    wxPluralFormsToken::Number number = m_plural->evaluate(n);
    if (number < 0 || number > m_nplurals)
    {
        return 0;
    }
    return number;
}


class wxPluralFormsParser
{
public:
    explicit wxPluralFormsParser(wxPluralFormsScanner& scanner) : m_scanner(scanner) {}
    bool parse(wxPluralFormsCalculator& rCalculator);

private:
    wxPluralFormsNode* parsePlural();
    // stops at T_SEMICOLON, returns 0 if error
    wxPluralFormsScanner& m_scanner;
    const wxPluralFormsToken& token() const;
    bool nextToken();

    wxPluralFormsNode* expression();
    wxPluralFormsNode* logicalOrExpression();
    wxPluralFormsNode* logicalAndExpression();
    wxPluralFormsNode* equalityExpression();
    wxPluralFormsNode* multiplicativeExpression();
    wxPluralFormsNode* relationalExpression();
    wxPluralFormsNode* pmExpression();
};

bool wxPluralFormsParser::parse(wxPluralFormsCalculator& rCalculator)
{
    if (token().type() != wxPluralFormsToken::T_NPLURALS)
        return false;
    if (!nextToken())
        return false;
    if (token().type() != wxPluralFormsToken::T_ASSIGN)
        return false;
    if (!nextToken())
        return false;
    if (token().type() != wxPluralFormsToken::T_NUMBER)
        return false;
    wxPluralFormsToken::Number nplurals = token().number();
    if (!nextToken())
        return false;
    if (token().type() != wxPluralFormsToken::T_SEMICOLON)
        return false;
    if (!nextToken())
        return false;
    if (token().type() != wxPluralFormsToken::T_PLURAL)
        return false;
    if (!nextToken())
        return false;
    if (token().type() != wxPluralFormsToken::T_ASSIGN)
        return false;
    if (!nextToken())
        return false;
    wxPluralFormsNode* plural = parsePlural();
    if (plural == nullptr)
        return false;
    if (token().type() != wxPluralFormsToken::T_SEMICOLON)
        return false;
    if (!nextToken())
        return false;
    if (token().type() != wxPluralFormsToken::T_EOF)
        return false;
    rCalculator.init(nplurals, plural);
    return true;
}

wxPluralFormsNode* wxPluralFormsParser::parsePlural()
{
    wxPluralFormsNode* p = expression();
    if (p == nullptr)
    {
        return nullptr;
    }
    wxPluralFormsNodePtr n(p);
    if (token().type() != wxPluralFormsToken::T_SEMICOLON)
    {
        return nullptr;
    }
    return n.release();
}

const wxPluralFormsToken& wxPluralFormsParser::token() const
{
    return m_scanner.token();
}

bool wxPluralFormsParser::nextToken()
{
    return m_scanner.nextToken();
}

wxPluralFormsNode* wxPluralFormsParser::expression()
{
    wxPluralFormsNode* p = logicalOrExpression();
    if (p == nullptr)
        return nullptr;
    wxPluralFormsNodePtr n(p);
    if (token().type() == wxPluralFormsToken::T_QUESTION)
    {
        wxPluralFormsNodePtr qn(new wxPluralFormsNode(token()));
        if (!nextToken())
        {
            return nullptr;
        }
        p = expression();
        if (p == nullptr)
        {
            return nullptr;
        }
        qn->setNode(1, p);
        if (token().type() != wxPluralFormsToken::T_COLON)
        {
            return nullptr;
        }
        if (!nextToken())
        {
            return nullptr;
        }
        p = expression();
        if (p == nullptr)
        {
            return nullptr;
        }
        qn->setNode(2, p);
        qn->setNode(0, n.release());
        return qn.release();
    }
    return n.release();
}

wxPluralFormsNode*wxPluralFormsParser::logicalOrExpression()
{
    wxPluralFormsNode* p = logicalAndExpression();
    if (p == nullptr)
        return nullptr;
    wxPluralFormsNodePtr ln(p);
    if (token().type() == wxPluralFormsToken::T_LOGICAL_OR)
    {
        wxPluralFormsNodePtr un(new wxPluralFormsNode(token()));
        if (!nextToken())
        {
            return nullptr;
        }
        p = logicalOrExpression();
        if (p == nullptr)
        {
            return nullptr;
        }
        wxPluralFormsNodePtr rn(p);    // right
        if (rn->token().type() == wxPluralFormsToken::T_LOGICAL_OR)
        {
            // see logicalAndExpression comment
            un->setNode(0, ln.release());
            un->setNode(1, rn->releaseNode(0));
            rn->setNode(0, un.release());
            return rn.release();
        }


        un->setNode(0, ln.release());
        un->setNode(1, rn.release());
        return un.release();
    }
    return ln.release();
}

wxPluralFormsNode* wxPluralFormsParser::logicalAndExpression()
{
    wxPluralFormsNode* p = equalityExpression();
    if (p == nullptr)
        return nullptr;
    wxPluralFormsNodePtr ln(p);   // left
    if (token().type() == wxPluralFormsToken::T_LOGICAL_AND)
    {
        wxPluralFormsNodePtr un(new wxPluralFormsNode(token()));  // up
        if (!nextToken())
        {
            return nullptr;
        }
        p = logicalAndExpression();
        if (p == nullptr)
        {
            return nullptr;
        }
        wxPluralFormsNodePtr rn(p);    // right
        if (rn->token().type() == wxPluralFormsToken::T_LOGICAL_AND)
        {
// transform 1 && (2 && 3) -> (1 && 2) && 3
//     u                  r
// l       r     ->   u      3
//       2   3      l   2
            un->setNode(0, ln.release());
            un->setNode(1, rn->releaseNode(0));
            rn->setNode(0, un.release());
            return rn.release();
        }

        un->setNode(0, ln.release());
        un->setNode(1, rn.release());
        return un.release();
    }
    return ln.release();
}

wxPluralFormsNode* wxPluralFormsParser::equalityExpression()
{
    wxPluralFormsNode* p = relationalExpression();
    if (p == nullptr)
        return nullptr;
    wxPluralFormsNodePtr n(p);
    if (token().type() == wxPluralFormsToken::T_EQUAL
        || token().type() == wxPluralFormsToken::T_NOT_EQUAL)
    {
        wxPluralFormsNodePtr qn(new wxPluralFormsNode(token()));
        if (!nextToken())
        {
            return nullptr;
        }
        p = relationalExpression();
        if (p == nullptr)
        {
            return nullptr;
        }
        qn->setNode(1, p);
        qn->setNode(0, n.release());
        return qn.release();
    }
    return n.release();
}

wxPluralFormsNode* wxPluralFormsParser::relationalExpression()
{
    wxPluralFormsNode* p = multiplicativeExpression();
    if (p == nullptr)
        return nullptr;
    wxPluralFormsNodePtr n(p);
    if (token().type() == wxPluralFormsToken::T_GREATER
            || token().type() == wxPluralFormsToken::T_LESS
            || token().type() == wxPluralFormsToken::T_GREATER_OR_EQUAL
            || token().type() == wxPluralFormsToken::T_LESS_OR_EQUAL)
    {
        wxPluralFormsNodePtr qn(new wxPluralFormsNode(token()));
        if (!nextToken())
        {
            return nullptr;
        }
        p = multiplicativeExpression();
        if (p == nullptr)
        {
            return nullptr;
        }
        qn->setNode(1, p);
        qn->setNode(0, n.release());
        return qn.release();
    }
    return n.release();
}

wxPluralFormsNode* wxPluralFormsParser::multiplicativeExpression()
{
    wxPluralFormsNode* p = pmExpression();
    if (p == nullptr)
        return nullptr;
    wxPluralFormsNodePtr n(p);
    if (token().type() == wxPluralFormsToken::T_REMINDER)
    {
        wxPluralFormsNodePtr qn(new wxPluralFormsNode(token()));
        if (!nextToken())
        {
            return nullptr;
        }
        p = pmExpression();
        if (p == nullptr)
        {
            return nullptr;
        }
        qn->setNode(1, p);
        qn->setNode(0, n.release());
        return qn.release();
    }
    return n.release();
}

wxPluralFormsNode* wxPluralFormsParser::pmExpression()
{
    wxPluralFormsNodePtr n;
    if (token().type() == wxPluralFormsToken::T_N
        || token().type() == wxPluralFormsToken::T_NUMBER)
    {
        n.reset(new wxPluralFormsNode(token()));
        if (!nextToken())
        {
            return nullptr;
        }
    }
    else if (token().type() == wxPluralFormsToken::T_LEFT_BRACKET) {
        if (!nextToken())
        {
            return nullptr;
        }
        wxPluralFormsNode* p = expression();
        if (p == nullptr)
        {
            return nullptr;
        }
        n.reset(p);
        if (token().type() != wxPluralFormsToken::T_RIGHT_BRACKET)
        {
            return nullptr;
        }
        if (!nextToken())
        {
            return nullptr;
        }
    }
    else
    {
        return nullptr;
    }
    return n.release();
}

wxPluralFormsCalculator* wxPluralFormsCalculator::make(const char* s)
{
    wxPluralFormsCalculatorPtr calculator(new wxPluralFormsCalculator);
    if (s != nullptr)
    {
        wxPluralFormsScanner scanner(s);
        wxPluralFormsParser p(scanner);
        if (!p.parse(*calculator))
        {
            return nullptr;
        }
    }
    return calculator.release();
}




// ----------------------------------------------------------------------------
// wxMsgCatalogFile corresponds to one disk-file message catalog.
//
// This is a "low-level" class and is used only by wxMsgCatalog
// NOTE: for the documentation of the binary catalog (.MO) files refer to
//       the GNU gettext manual:
//       http://www.gnu.org/software/autoconf/manual/gettext/MO-Files.html
// ----------------------------------------------------------------------------

class wxMsgCatalogFile
{
public:
    using DataBuffer = wxScopedCharBuffer;

    wxMsgCatalogFile() = default;
    ~wxMsgCatalogFile() = default;

    wxMsgCatalogFile(const wxMsgCatalogFile&) = delete;
	wxMsgCatalogFile& operator=(const wxMsgCatalogFile&) = delete;

    // load the catalog from disk
    bool LoadFile(const wxString& filename,
                  wxPluralFormsCalculatorPtr& rPluralFormsCalculator);
    bool LoadData(const DataBuffer& data,
                  wxPluralFormsCalculatorPtr& rPluralFormsCalculator);

    // fills the hash with string-translation pairs
    bool FillHash(wxStringToStringHashMap& hash, const wxString& domain) const;

    // return the charset of the strings in this catalog or empty string if
    // none/unknown
    wxString GetCharset() const { return m_charset; }

private:
    // this implementation is binary compatible with GNU gettext() version 0.10

    // an entry in the string table
    struct wxMsgTableEntry
    {
        size_t32   nLen;           // length of the string
        size_t32   ofsString;      // pointer to the string
    };

    // header of a .mo file
    struct wxMsgCatalogHeader
    {
        size_t32  magic,          // offset +00:  magic id
                  revision,       //        +04:  revision
                  numStrings;     //        +08:  number of strings in the file
        size_t32  ofsOrigTable,   //        +0C:  start of original string table
                  ofsTransTable;  //        +10:  start of translated string table
        size_t32  nHashSize,      //        +14:  hash table size
                  ofsHashTable;   //        +18:  offset of hash table start
    };

    // all data is stored here
    DataBuffer m_data;

    // data description
    size_t32          m_numStrings;   // number of strings in this domain
    const
    wxMsgTableEntry  *m_pOrigTable,   // pointer to original   strings
                     *m_pTransTable;  //            translated

    wxString m_charset;               // from the message catalog header


    // swap the 2 halves of 32 bit integer if needed
    size_t32 Swap(size_t32 ui) const
    {
        return m_bSwapped ? (ui << 24) | ((ui & 0xff00) << 8) |
                            ((ui >> 8) & 0xff00) | (ui >> 24)
                            : ui;
    }

    const char* StringAtOfs(const wxMsgTableEntry* pTable, size_t32 n) const
    {
        const wxMsgTableEntry * const ent = pTable + n;

        // this check could fail for a corrupt message catalog
        size_t32 ofsString = Swap(ent->ofsString);
        if ( ofsString + Swap(ent->nLen) > m_data.length())
        {
            return nullptr;
        }

        return m_data.data() + ofsString;
    }

    bool m_bSwapped;   // wrong endianness?
};

// ----------------------------------------------------------------------------
// wxMsgCatalogFile class
// ----------------------------------------------------------------------------

// open disk file and read in it's contents
bool wxMsgCatalogFile::LoadFile(const wxString& filename,
                                wxPluralFormsCalculatorPtr& rPluralFormsCalculator)
{
    wxFile fileMsg(filename);
    if ( !fileMsg.IsOpened() )
        return false;

    // get the file size (assume it is less than 4GB...)
    wxFileOffset lenFile = fileMsg.Length();
    if ( lenFile == wxInvalidOffset )
        return false;

    auto nSize = static_cast<size_t>(lenFile);
    wxASSERT_MSG( nSize == lenFile + size_t(0), wxS("message catalog bigger than 4GB?") );

    wxMemoryBuffer filedata;

    // read the whole file in memory
    if ( fileMsg.Read(filedata.GetWriteBuf(nSize), nSize) != lenFile )
        return false;

    filedata.UngetWriteBuf(nSize);

    bool ok = LoadData
              (
                  DataBuffer::CreateOwned((char*)filedata.release(), nSize),
                  rPluralFormsCalculator
              );
    if ( !ok )
    {
        wxLogWarning(_("'%s' is not a valid message catalog."), filename.c_str());
        return false;
    }

    return true;
}


bool wxMsgCatalogFile::LoadData(const DataBuffer& data,
                                wxPluralFormsCalculatorPtr& rPluralFormsCalculator)
{
    // examine header
    bool bValid = data.length() > sizeof(wxMsgCatalogHeader);

    const wxMsgCatalogHeader* pHeader = reinterpret_cast<const wxMsgCatalogHeader*>(data.data());
    if ( bValid ) {
        // we'll have to swap all the integers if it's true
        m_bSwapped = pHeader->magic == MSGCATALOG_MAGIC_SW;

        // check the magic number
        bValid = m_bSwapped || pHeader->magic == MSGCATALOG_MAGIC;
    }

    if ( !bValid ) {
        // it's either too short or has incorrect magic number
        wxLogWarning(_("Invalid message catalog."));
        return false;
    }

    m_data = data;

    // initialize
    m_numStrings  = Swap(pHeader->numStrings);
    m_pOrigTable  = reinterpret_cast<const wxMsgTableEntry*>(data.data() +
                    Swap(pHeader->ofsOrigTable));
    m_pTransTable = reinterpret_cast<const wxMsgTableEntry*>(data.data() +
                    Swap(pHeader->ofsTransTable));

    // now parse catalog's header and try to extract catalog charset and
    // plural forms formula from it:

    const char* headerData = StringAtOfs(m_pOrigTable, 0);
    if ( headerData && headerData[0] == '\0' )
    {
        // Extract the charset:
        const char * const header = StringAtOfs(m_pTransTable, 0);
        const char *
            cset = strstr(header, "Content-Type: text/plain; charset=");
        if ( cset )
        {
            cset += 34; // strlen("Content-Type: text/plain; charset=")

            const char * const csetEnd = strchr(cset, '\n');
            if ( csetEnd )
            {
                m_charset = wxString(cset, csetEnd - cset);
                if ( m_charset == wxS("CHARSET") )
                {
                    // "CHARSET" is not valid charset, but lazy translator
                    m_charset.clear();
                }
            }
        }
        // else: incorrectly filled Content-Type header

        // Extract plural forms:
        const char * plurals = strstr(header, "Plural-Forms:");
        if ( plurals )
        {
            plurals += 13; // strlen("Plural-Forms:")
            const char * const pluralsEnd = strchr(plurals, '\n');
            if ( pluralsEnd )
            {
                const size_t pluralsLen = pluralsEnd - plurals;
                wxCharBuffer buf(pluralsLen);
                strncpy(buf.data(), plurals, pluralsLen);
                wxPluralFormsCalculator * const
                    pCalculator = wxPluralFormsCalculator::make(buf);
                if ( pCalculator )
                {
                    rPluralFormsCalculator.reset(pCalculator);
                }
                else
                {
                    wxLogVerbose(_("Failed to parse Plural-Forms: '%s'"),
                                 buf.data());
                }
            }
        }

        if ( !rPluralFormsCalculator.get() )
            rPluralFormsCalculator.reset(wxPluralFormsCalculator::make());
    }

    // everything is fine
    return true;
}

bool wxMsgCatalogFile::FillHash(wxStringToStringHashMap& hash,
                                const wxString& domain) const
{
    wxUnusedVar(domain); // silence warning in Unicode build

    // conversion to use to convert catalog strings to the GUI encoding
    wxMBConv *inputConv = nullptr;

    std::unique_ptr<wxMBConv> inputConvPtr; // just to delete inputConv if needed

    if ( !m_charset.empty() )
    {
        {
            inputConv = new wxCSConv(m_charset);

            // As we allocated it ourselves, we need to delete it, so ensure
            // this happens.
            inputConvPtr.reset(inputConv);
        }
    }
    else // no need or not possible to convert the encoding
    {
        // we must somehow convert the narrow strings in the message catalog to
        // wide strings, so use the default conversion if we have no charset
        inputConv = wxConvCurrent;
    }

    for (size_t32 i = 0; i < m_numStrings; i++)
    {
        const char *data = StringAtOfs(m_pOrigTable, i);
        if (!data)
            return false; // may happen for invalid MO files

        wxString msgid;
        msgid = wxString(data, *inputConv);

        data = StringAtOfs(m_pTransTable, i);
        if (!data)
            return false; // may happen for invalid MO files

        size_t length = Swap(m_pTransTable[i].nLen);
        size_t offset = 0;
        size_t index = 0;
        while (offset < length)
        {
            const char * const str = data + offset;

            wxString msgstr;
            msgstr = wxString(str, *inputConv);

            if ( !msgstr.empty() )
            {
                hash[index == 0 ? msgid : msgid + wxChar(index)] = msgstr;
            }

            // skip this string
            // IMPORTANT: accesses to the 'data' pointer are valid only for
            //            the first 'length+1' bytes (GNU specs says that the
            //            final NUL is not counted in length); using wxStrnlen()
            //            we make sure we don't access memory beyond the valid range
            //            (which otherwise may happen for invalid MO files):
            offset += wxStrnlen(str, length - offset) + 1;
            ++index;
        }
    }

    return true;
}


// ----------------------------------------------------------------------------
// wxMsgCatalog class
// ----------------------------------------------------------------------------

/* static */
wxMsgCatalog *wxMsgCatalog::CreateFromFile(const wxString& filename,
                                           const wxString& domain)
{
    std::unique_ptr<wxMsgCatalog> cat(new wxMsgCatalog(domain));

    wxMsgCatalogFile file;

    if ( !file.LoadFile(filename, cat->m_pluralFormsCalculator) )
        return nullptr;

    if ( !file.FillHash(cat->m_messages, domain) )
        return nullptr;

    return cat.release();
}

/* static */
wxMsgCatalog *wxMsgCatalog::CreateFromData(const wxScopedCharBuffer& data,
                                           const wxString& domain)
{
    std::unique_ptr<wxMsgCatalog> cat(new wxMsgCatalog(domain));

    wxMsgCatalogFile file;

    if ( !file.LoadData(data, cat->m_pluralFormsCalculator) )
        return nullptr;

    if ( !file.FillHash(cat->m_messages, domain) )
        return nullptr;

    return cat.release();
}

const wxString *wxMsgCatalog::GetString(const wxString& str, unsigned n, const wxString& context) const
{
    int index = 0;
    if (n != std::numeric_limits<unsigned int>::max())
    {
        index = m_pluralFormsCalculator->evaluate(n);
    }
    wxStringToStringHashMap::const_iterator i;
    if (index != 0)
    {
        if (context.IsEmpty())
            i = m_messages.find(wxString(str) + wxChar(index));   // plural, no context
        else
            i = m_messages.find(wxString(context) + wxString('\x04') + wxString(str) + wxChar(index));   // plural, context
    }
    else
    {
        if (context.IsEmpty())
            i = m_messages.find(str); // no context
        else
            i = m_messages.find(wxString(context) + wxString('\x04') + wxString(str)); // context
    }

    if ( i != m_messages.end() )
    {
        return &i->second;
    }
    else
        return nullptr;
}


// ----------------------------------------------------------------------------
// wxTranslations
// ----------------------------------------------------------------------------

namespace
{

wxTranslations *gs_translations = nullptr;
bool gs_translationsOwned = false;

} // anonymous namespace


/*static*/
wxTranslations *wxTranslations::Get()
{
    return gs_translations;
}

/*static*/
void wxTranslations::Set(wxTranslations *t)
{
    if ( gs_translationsOwned )
        delete gs_translations;
    gs_translations = t;
    gs_translationsOwned = true;
}

/*static*/
void wxTranslations::SetNonOwned(wxTranslations *t)
{
    if ( gs_translationsOwned )
        delete gs_translations;
    gs_translations = t;
    gs_translationsOwned = false;
}


wxTranslations::wxTranslations()
    : m_loader(new wxFileTranslationsLoader)
{
}


wxTranslations::~wxTranslations()
{
    delete m_loader;

    // free catalogs memory
    while ( m_pMsgCat != nullptr )
    {
        wxMsgCatalog* pTmpCat = m_pMsgCat;
        m_pMsgCat = m_pMsgCat->m_pNext;
        delete pTmpCat;
    }
}


void wxTranslations::SetLoader(wxTranslationsLoader *loader)
{
    wxCHECK_RET( loader, "loader can't be NULL" );

    delete m_loader;
    m_loader = loader;
}


void wxTranslations::SetLanguage(wxLanguage lang)
{
    if ( lang == wxLANGUAGE_DEFAULT )
        SetLanguage(wxString());
    else
        SetLanguage(wxLocale::GetLanguageCanonicalName(lang));
}

void wxTranslations::SetLanguage(const wxString& lang)
{
    m_lang = lang;
}


std::vector<wxString> wxTranslations::GetAvailableTranslations(const wxString& domain) const
{
    wxCHECK_MSG( m_loader, std::vector<wxString>(), "loader can't be NULL" );

    return m_loader->GetAvailableTranslations(domain);
}


bool wxTranslations::AddStdCatalog()
{
    // Try loading the message catalog for this version first, but fall back to
    // the name without the version if it's not found, as message catalogs
    // typically won't have the version in their names under non-Unix platforms
    // (i.e. where they're not installed by our own "make install").
    if ( AddCatalog("wxstd-" wxSTRINGIZE(wxMAJOR_VERSION) "." wxSTRINGIZE(wxMINOR_VERSION)) )
        return true;

    if ( AddCatalog(wxS("wxstd")) )
        return true;

    return false;
}

bool wxTranslations::AddCatalog(const wxString& domain,
                                wxLanguage msgIdLanguage)
{
    const wxString msgIdLang = wxLocale::GetLanguageCanonicalName(msgIdLanguage);
    const wxString domain_lang = GetBestTranslation(domain, msgIdLang);

    if ( domain_lang.empty() )
    {
        wxLogTrace(TRACE_I18N,
                    wxS("no suitable translation for domain '%s' found"),
                    domain);
        return false;
    }

    wxLogTrace(TRACE_I18N,
                wxS("adding '%s' translation for domain '%s' (msgid language '%s')"),
                domain_lang, domain, msgIdLang);

    return LoadCatalog(domain, domain_lang, msgIdLang);
}


bool wxTranslations::LoadCatalog(const wxString& domain, const wxString& lang, const wxString& msgIdLang)
{
    wxCHECK_MSG( m_loader, false, "loader can't be NULL" );

    wxMsgCatalog *cat = nullptr;

#if wxUSE_FONTMAP
    // first look for the catalog for this language and the current locale:
    // notice that we don't use the system name for the locale as this would
    // force us to install catalogs in different locations depending on the
    // system but always use the canonical name
    const wxFontEncoding encSys = wxLocale::GetSystemEncoding();
    if ( encSys != wxFONTENCODING_SYSTEM )
    {
        wxString fullname(lang);
        fullname << wxS('.') << wxFontMapperBase::GetEncodingName(encSys);

        cat = m_loader->LoadCatalog(domain, fullname);
    }
#endif // wxUSE_FONTMAP

    if ( !cat )
    {
        // Next try: use the provided name language name:
        cat = m_loader->LoadCatalog(domain, lang);
    }

    if ( !cat )
    {
        // Also try just base locale name: for things like "fr_BE" (Belgium
        // French) we should use fall back on plain "fr" if no Belgium-specific
        // message catalogs exist
        wxString baselang = lang.BeforeFirst('_');
        if ( lang != baselang )
            cat = m_loader->LoadCatalog(domain, baselang);
    }

    if ( !cat )
    {
        // It is OK to not load catalog if the msgid language and m_language match,
        // in which case we can directly display the texts embedded in program's
        // source code:
        if ( msgIdLang == lang )
            return true;
    }

    if ( cat )
    {
        // add it to the head of the list so that in GetString it will
        // be searched before the catalogs added earlier

        cat->m_pNext = m_pMsgCat;
        m_pMsgCat = cat;
        m_catalogMap[domain] = cat;

        return true;
    }
    else
    {
        // Nothing worked, the catalog just isn't there
        wxLogTrace(TRACE_I18N,
                   R"(Catalog "%s.mo" not found for language "%s".)",
                   domain, lang);
        return false;
    }
}

// check if the given catalog is loaded
bool wxTranslations::IsLoaded(const wxString& domain) const
{
    return FindCatalog(domain) != nullptr;
}

wxString wxTranslations::GetBestTranslation(const wxString& domain,
                                            wxLanguage msgIdLanguage)
{
    const wxString lang = wxLocale::GetLanguageCanonicalName(msgIdLanguage);
    return GetBestTranslation(domain, lang);
}

wxString wxTranslations::GetBestTranslation(const wxString& domain,
                                            const wxString& msgIdLanguage)
{
    // explicitly set language should always be respected
    if ( !m_lang.empty() )
        return m_lang;

    std::vector<wxString> available(GetAvailableTranslations(domain));
    // it's OK to have duplicates, so just add msgid language
    available.push_back(msgIdLanguage);
    available.push_back(msgIdLanguage.BeforeFirst('_'));

    wxLogTrace(TRACE_I18N, "choosing best language for domain '%s'", domain);
    LogTraceArray(" - available translations", available);
    // TODO: Lambda
    wxString lang = GetPreferredUILanguage(available);
    wxLogTrace(TRACE_I18N, " => using language '%s'", lang);
    return lang;
}


/* static */
const wxString& wxTranslations::GetUntranslatedString(const wxString& str)
{
    wxLocaleUntranslatedStrings& strings = wxThreadInfo.untranslatedStrings;

    wxLocaleUntranslatedStrings::iterator i = strings.find(str);
    if ( i == strings.end() )
        return *strings.insert(str).first;

    return *i;
}


const wxString *wxTranslations::GetTranslatedString(const wxString& origString,
                                                    const wxString& domain,
                                                    const wxString& context) const
{
    return GetTranslatedString(origString, std::numeric_limits<unsigned int>::max(), domain, context);
}

const wxString *wxTranslations::GetTranslatedString(const wxString& origString,
                                                    unsigned n,
                                                    const wxString& domain,
                                                    const wxString& context) const
{
    if ( origString.empty() )
        return nullptr;

    const wxString *trans = nullptr;

    if ( !domain.empty() )
    {
        wxMsgCatalog* pMsgCat = FindCatalog(domain);

        // does the catalog exist?
        if ( pMsgCat != nullptr )
            trans = pMsgCat->GetString(origString, n, context);
    }
    else
    {
        // search in all domains
        for ( wxMsgCatalog* pMsgCat = m_pMsgCat; pMsgCat != nullptr; pMsgCat = pMsgCat->m_pNext )
        {
            trans = pMsgCat->GetString(origString, n, context);
            if ( trans != nullptr )   // take the first found
                break;
        }
    }

    if ( trans == nullptr )
    {
        wxLogTrace
        (
            TRACE_I18N,
            "string \"%s\"%s not found in %s%slocale '%s'.",
            origString,
            (n != std::numeric_limits<unsigned int>::max() ? wxString::Format("[%ld]", (long)n) : wxString()),
            (!domain.empty() ? wxString::Format("domain '%s' ", domain) : wxString()),
            (!context.empty() ? wxString::Format("context '%s' ", context) : wxString()),
            m_lang
        );
    }

    return trans;
}

wxString wxTranslations::GetHeaderValue(const wxString& header,
                                        const wxString& domain) const
{
    if ( header.empty() )
        return wxEmptyString;

    const wxString *trans = nullptr;

    if ( !domain.empty() )
    {
        wxMsgCatalog* pMsgCat = FindCatalog(domain);

        // does the catalog exist?
        if ( pMsgCat == nullptr )
            return wxEmptyString;

        trans = pMsgCat->GetString(wxEmptyString, std::numeric_limits<unsigned int>::max());
    }
    else
    {
        // search in all domains
        for ( wxMsgCatalog* pMsgCat = m_pMsgCat; pMsgCat != nullptr; pMsgCat = pMsgCat->m_pNext )
        {
            trans = pMsgCat->GetString(wxEmptyString, std::numeric_limits<unsigned int>::max());
            if ( trans != nullptr )   // take the first found
                break;
        }
    }

    if ( !trans || trans->empty() )
        return wxEmptyString;

    size_t found = trans->find(header + wxS(": "));
    if ( found == wxString::npos )
        return wxEmptyString;

    found += header.length() + 2 /* ': ' */;

    // Every header is separated by \n

    size_t endLine = trans->find(wxS('\n'), found);
    size_t len = (endLine == wxString::npos) ?
                wxString::npos : (endLine - found);

    return trans->substr(found, len);
}


// find catalog by name
wxMsgCatalog *wxTranslations::FindCatalog(const wxString& domain) const
{
    const wxMsgCatalogMap::const_iterator found = m_catalogMap.find(domain);

    return found == m_catalogMap.end() ? NULL : found->second;
}

// ----------------------------------------------------------------------------
// wxFileTranslationsLoader
// ----------------------------------------------------------------------------

namespace
{

// the list of the directories to search for message catalog files
std::vector<wxString> gs_searchPrefixes;

// return the directories to search for message catalogs under the given
// prefix, separated by wxPATH_SEP
wxString GetMsgCatalogSubdirs(const wxString& prefix, const wxString& lang)
{
    // Search first in Unix-standard prefix/lang/LC_MESSAGES, then in
    // prefix/lang.
    //
    // Note that we use LC_MESSAGES on all platforms and not just Unix, because
    // it doesn't cost much to look into one more directory and doing it this
    // way has two important benefits:
    // a) we don't break compatibility with wx-2.6 and older by stopping to
    //    look in a directory where the catalogs used to be and thus silently
    //    breaking apps after they are recompiled against the latest wx
    // b) it makes it possible to package app's support files in the same
    //    way on all target platforms
    const wxString prefixAndLang = wxFileName(prefix, lang).GetFullPath();

    wxString searchPath;
    searchPath.reserve(4*prefixAndLang.length());

    searchPath
#ifdef __WXOSX__
               << prefixAndLang << ".lproj/LC_MESSAGES" << wxPATH_SEP
               << prefixAndLang << ".lproj" << wxPATH_SEP
#endif
               << prefixAndLang << wxFILE_SEP_PATH << "LC_MESSAGES" << wxPATH_SEP
               << prefixAndLang << wxPATH_SEP
               ;

    return searchPath;
}

bool HasMsgCatalogInDir(const wxString& dir, const wxString& domain)
{
    return wxFileName(dir, domain, "mo").FileExists() ||
           wxFileName(dir + wxFILE_SEP_PATH + "LC_MESSAGES", domain, "mo").FileExists();
}

// get prefixes to locale directories; if lang is empty, don't point to
// OSX's .lproj bundles
std::vector<wxString> GetSearchPrefixes()
{
    std::vector<wxString> paths;

    // first take the entries explicitly added by the program
    paths = gs_searchPrefixes;

#if wxUSE_STDPATHS
    // then look in the standard location
    wxString stdp;
    stdp = wxStandardPaths::Get().GetResourcesDir();
    const auto path_iter = std::find_if(paths.cbegin(), paths.cend(),
        [stdp](const auto& path){
            return stdp.IsSameAs(path);
        });
    if ( path_iter == std::cend(paths) )
        paths.push_back(stdp);

  #ifdef wxHAS_STDPATHS_INSTALL_PREFIX
    stdp = wxStandardPaths::Get().GetInstallPrefix() + "/share/locale";
    if ( paths.Index(stdp) == wxNOT_FOUND )
        paths.Add(stdp);
  #endif
#endif // wxUSE_STDPATHS

    // last look in default locations
#ifdef __UNIX__
    // LC_PATH is a standard env var containing the search path for the .mo
    // files
    const char *pszLcPath = wxGetenv("LC_PATH");
    if ( pszLcPath )
    {
        const wxString lcp = pszLcPath;
        if ( paths.Index(lcp) == wxNOT_FOUND )
            paths.Add(lcp);
    }

    // also add the one from where wxWin was installed:
    wxString wxp = wxGetInstallPrefix();
    if ( !wxp.empty() )
    {
        wxp += wxS("/share/locale");
        if ( paths.Index(wxp) == wxNOT_FOUND )
            paths.Add(wxp);
    }
#endif // __UNIX__

    return paths;
}

// construct the search path for the given language
wxString GetFullSearchPath(const wxString& lang)
{
    wxString searchPath;
    searchPath.reserve(500);

    for ( const auto& i : GetSearchPrefixes() )
    {
        const wxString p = GetMsgCatalogSubdirs(i, lang);

        if ( !searchPath.empty() )
            searchPath += wxPATH_SEP;
        searchPath += p;
    }

    return searchPath;
}

} // anonymous namespace


void wxFileTranslationsLoader::AddCatalogLookupPathPrefix(const wxString& prefix)
{

    if (std::cend(gs_searchPrefixes) == std::find_if(gs_searchPrefixes.cbegin(), gs_searchPrefixes.cend(),
        [prefix](const auto& searchPrefix) {
            return prefix.IsSameAs(searchPrefix);
        }))
    {
        gs_searchPrefixes.push_back(prefix);
    }
    //else: already have it
}


wxMsgCatalog *wxFileTranslationsLoader::LoadCatalog(const wxString& domain,
                                                    const wxString& lang)
{
    wxString searchPath = GetFullSearchPath(lang);

    //LogTraceLargeArray
    //(
        // FIXME: Add Format overload for std::vector<wxString>
    //    wxString::Format("looking for \"%s.mo\" in search path", domain),
    //    wxSplit(searchPath, wxPATH_SEP[0])
    //);

    wxFileName fn(domain);
    fn.SetExt(wxS("mo"));

    wxString strFullName;
    if ( !wxFindFileInPath(&strFullName, searchPath, fn.GetFullPath()) )
        return nullptr;

    // open file and read its data
    wxLogVerbose(_("using catalog '%s' from '%s'."), domain, strFullName.c_str());
    wxLogTrace(TRACE_I18N, wxS("Using catalog \"%s\"."), strFullName.c_str());

    return wxMsgCatalog::CreateFromFile(strFullName, domain);
}


std::vector<wxString> wxFileTranslationsLoader::GetAvailableTranslations(const wxString& domain) const
{
    std::vector<wxString> langs;
    const std::vector<wxString> prefixes = GetSearchPrefixes();

    LogTraceLargeArray
    (
        wxString::Format("looking for available translations of \"%s\" in search path", domain),
        prefixes
    );

    for ( const auto& i : prefixes )
    {
        if ( i.empty() )
            continue;
        wxDir dir;
        if ( !dir.Open(i) )
            continue;

        wxString lang;
        for ( bool ok = dir.GetFirst(&lang, wxString(), wxDIR_DIRS);
              ok;
              ok = dir.GetNext(&lang) )
        {
            const wxString langdir = i + wxFILE_SEP_PATH + lang;
            if ( HasMsgCatalogInDir(langdir, domain) )
            {
#ifdef __WXOSX__
                wxString rest;
                if ( lang.EndsWith(".lproj", &rest) )
                    lang = rest;
#endif // __WXOSX__

                wxLogTrace(TRACE_I18N,
                           "found %s translation of \"%s\" in %s",
                           lang, domain, langdir);
                langs.push_back(lang);
            }
        }
    }

    return langs;
}


// ----------------------------------------------------------------------------
// wxResourceTranslationsLoader
// ----------------------------------------------------------------------------

#ifdef __WINDOWS__

wxMsgCatalog *wxResourceTranslationsLoader::LoadCatalog(const wxString& domain,
                                                        const wxString& lang)
{
    const void *mo_data = nullptr;
    size_t mo_size = 0;

    const wxString resname = wxString::Format("%s_%s", domain, lang);

    if ( !wxLoadUserResource(&mo_data, &mo_size,
                             resname,
                             GetResourceType().t_str(),
                             GetModule()) )
        return nullptr;

    wxLogTrace(TRACE_I18N,
               "Using catalog from Windows resource \"%s\".", resname);

    wxMsgCatalog *cat = wxMsgCatalog::CreateFromData(
        wxCharBuffer::CreateNonOwned(static_cast<const char*>(mo_data), mo_size),
        domain);

    if ( !cat )
    {
        wxLogWarning(_("Resource '%s' is not a valid message catalog."), resname);
    }

    return cat;
}

namespace
{

struct EnumCallbackData
{
    wxString prefix;
    std::vector<wxString> langs;
};

BOOL CALLBACK EnumTranslations(HMODULE WXUNUSED(hModule),
                               LPCTSTR WXUNUSED(lpszType),
                               LPTSTR lpszName,
                               LONG_PTR lParam)
{
    wxString name(lpszName);
    name.MakeLower(); // resource names are case insensitive

    EnumCallbackData *data = reinterpret_cast<EnumCallbackData*>(lParam);

    wxString lang;
    if ( name.StartsWith(data->prefix, &lang) && !lang.empty() )
        data->langs.push_back(lang);

    return TRUE; // continue enumeration
}

} // anonymous namespace


std::vector<wxString> wxResourceTranslationsLoader::GetAvailableTranslations(const wxString& domain) const
{
    EnumCallbackData data;
    data.prefix = domain + "_";
    data.prefix.MakeLower(); // resource names are case insensitive

    if ( !EnumResourceNames(GetModule(),
                            GetResourceType().t_str(),
                            EnumTranslations,
                            reinterpret_cast<LONG_PTR>(&data)) )
    {
        const DWORD err = GetLastError();
        if ( err != NO_ERROR && err != ERROR_RESOURCE_TYPE_NOT_FOUND )
        {
            wxLogSysError(_("Couldn't enumerate translations"));
        }
    }

    return data.langs;
}

#endif // __WINDOWS__


// ----------------------------------------------------------------------------
// wxTranslationsModule module (for destruction of gs_translations)
// ----------------------------------------------------------------------------

class wxTranslationsModule: public wxModule
{
    wxDECLARE_DYNAMIC_CLASS(wxTranslationsModule);
public:
    bool OnInit() override
    {
        return true;
    }

    void OnExit() override
    {
        if ( gs_translationsOwned )
            delete gs_translations;
        gs_translations = nullptr;
        gs_translationsOwned = true;
    }
};

wxIMPLEMENT_DYNAMIC_CLASS(wxTranslationsModule, wxModule);

#endif // wxUSE_INTL
