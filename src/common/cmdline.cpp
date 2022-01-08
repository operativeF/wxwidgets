///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/cmdline.cpp
// Purpose:     wxCmdLineParser implementation
// Author:      Vadim Zeitlin
// Modified by:
// Created:     05.01.00
// Copyright:   (c) 2000 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/string.h"
#include "wx/intl.h"
#include "wx/app.h"

#include "wx/datetime.h"
#include "wx/msgout.h"
#include "wx/apptrait.h"
#include "wx/scopeguard.h"
#include "wx/wxcrt.h"

#include <boost/nowide/convert.hpp>

module WX.Cmn.CommandLine;

import Utils.Chars;
import WX.File.Filename;

import <charconv>;
import <clocale>;             // for LC_ALL
import <vector>;

#if wxUSE_CMDLINE_PARSER

// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

static std::string GetTypeName(wxCmdLineParamType type);

static std::string GetOptionName(wxString::const_iterator p,
                                 wxString::const_iterator end,
                                 const wxChar *allowedChars);

static std::string GetShortOptionName(wxString::const_iterator p,
                                      wxString::const_iterator end);

static std::string GetLongOptionName(wxString::const_iterator p,
                                     wxString::const_iterator end);

// ----------------------------------------------------------------------------
// private structs
// ----------------------------------------------------------------------------


class wxCmdLineArgImpl: public wxCmdLineArg
{
public:
    wxCmdLineArgImpl(wxCmdLineEntryType k,
                 const wxString& shrt,
                 const wxString& lng,
                 wxCmdLineParamType typ);

    wxCmdLineArgImpl& SetDoubleVal(double val);
    wxCmdLineArgImpl& SetLongVal(long val);
    wxCmdLineArgImpl& SetStrVal(const wxString& val);
#if wxUSE_DATETIME
    wxCmdLineArgImpl& SetDateVal(const wxDateTime& val);
#endif // wxUSE_DATETIME

    bool HasValue() const { return m_hasVal; }
    wxCmdLineArgImpl& SetHasValue() { m_hasVal = true; return *this; }

    wxCmdLineArgImpl& SetNegated() { m_isNegated = true; return *this; }

    // Reset to the initial state, called before parsing another command line.
    void Reset()
    {
        m_hasVal =
        m_isNegated = false;
    }

    std::string shortName;
    std::string longName;

    wxCmdLineParamType type;
    wxCmdLineEntryType kind;

    // from wxCmdLineArg
    wxCmdLineEntryType GetKind() const override { return kind; }
    wxString GetShortName() const override {
        wxASSERT_MSG( kind == wxCmdLineEntryType::Option || kind == wxCmdLineEntryType::Switch,
                      "kind mismatch in wxCmdLineArg" );
        return shortName;
    }
    wxString GetLongName() const override {
        wxASSERT_MSG( kind == wxCmdLineEntryType::Option || kind == wxCmdLineEntryType::Switch,
                      "kind mismatch in wxCmdLineArg" );
        return longName;
    }
    wxCmdLineParamType GetType() const override {
        wxASSERT_MSG( kind == wxCmdLineEntryType::Option,
                      "kind mismatch in wxCmdLineArg" );
        return type;
    }
    double GetDoubleVal() const override;
    long GetLongVal() const override;
    const wxString& GetStrVal() const override;
#if wxUSE_DATETIME
    const wxDateTime& GetDateVal() const override;
#endif // wxUSE_DATETIME
    bool IsNegated() const override {
        wxASSERT_MSG( kind == wxCmdLineEntryType::Switch,
                      "kind mismatch in wxCmdLineArg" );
        return m_isNegated;
    }

private:
    // can't use union easily here, so just store all possible data fields, we
    // don't waste much (might still use union later if the number of supported
    // types increases)

    void Check(wxCmdLineParamType typ) const;

    wxString m_strVal;
#if wxUSE_DATETIME
    wxDateTime m_dateVal;
#endif // wxUSE_DATETIME

    double m_doubleVal;
    long m_longVal;

    bool m_hasVal;
    bool m_isNegated;
};

// an internal representation of an option
struct wxCmdLineOption: public wxCmdLineArgImpl
{
    wxCmdLineOption(wxCmdLineEntryType k,
                    const wxString& shrt,
                    const wxString& lng,
                    const wxString& desc,
                    wxCmdLineParamType typ,
                    unsigned int fl)
        : wxCmdLineArgImpl(k, shrt, lng, typ),
          description(desc),
          flags(fl)
    {
    }

    wxString description;
    unsigned int flags;
};

struct wxCmdLineParam
{
    wxCmdLineParam(const wxString& desc,
                   wxCmdLineParamType typ,
                   unsigned int fl)
        : description(desc),
          type(typ),
          flags(fl)
    {
    }

    wxString description;
    wxCmdLineParamType type;
    int flags;
};

using wxArrayOptions = std::vector<wxCmdLineOption>;
using wxArrayParams  = std::vector<wxCmdLineParam>;
using wxArrayArgs    = std::vector<wxCmdLineArgImpl>;

// the parser internal state
struct wxCmdLineParserData
{
    // options
    std::string m_switchChars;     // characters which may start an option
    std::string m_logo;            // some extra text to show in Usage()

    // cmd line data
    wxArrayOptions m_options;   // all possible options and switches
    wxArrayParams m_paramDesc;  // description of all possible params
    wxArrayArgs m_parsedArguments; // all options and parameters in parsing order

    std::vector<wxString> m_arguments;  // == argv, argc == m_arguments.GetCount()
    std::vector<wxString> m_parameters; // all params found

    bool m_enableLongOptions{ true };   // true if long options are enabled

    // methods
    wxCmdLineParserData();
    void SetArguments(int argc, char **argv);
    void SetArguments(int argc, wxChar **argv);
    void SetArguments(int argc, const wxCmdLineArgsArray& argv);
    void SetArguments(const wxString& cmdline);

    std::size_t FindOption(const std::string& name);
    std::size_t FindOptionByLongName(const std::string& name);

    // Find the option by either its short or long name.
    //
    // Asserts and returns NULL if option with this name is not found.
    const wxCmdLineOption* FindOptionByAnyName(const std::string& name);
};

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxCmdLineArg
// ----------------------------------------------------------------------------

wxCmdLineArgImpl::wxCmdLineArgImpl(wxCmdLineEntryType k,
                const wxString& shrt,
                const wxString& lng,
                wxCmdLineParamType typ)
{
    // wxCmdLineEntryType::UsageText uses only description, shortName and longName is empty
    if ( k != wxCmdLineEntryType::UsageText && k != wxCmdLineEntryType::Param)
    {
        wxASSERT_MSG
        (
            !shrt.empty() || !lng.empty(),
                "option should have at least one name"
        );

        // FIXME: Use narrow strings.
        // wxASSERT_MSG
        // (
        //     GetShortOptionName(shrt.begin(), shrt.end()).length() == shrt.Len(),
        //     "Short option contains invalid characters"
        // );

        // wxASSERT_MSG
        // (
        //     GetLongOptionName(lng.begin(), lng.end()).length() == lng.Len(),
        //     "Long option contains invalid characters"
        // );
    }

    kind = k;

    shortName = shrt;
    longName = lng;

    type = typ;

    Reset();
}

void wxCmdLineArgImpl::Check([[maybe_unused]] wxCmdLineParamType typ) const
{
    // NB: Type is always wxCmdLineParamType::None for booleans, so mismatch between
    //  switches / options / params is well checked by this test
    // The parameters have type == wxCmdLineParamType::String and thus can be
    //  retrieved only by GetStrVal()
    wxASSERT_MSG( type == typ, "type mismatch in wxCmdLineArg" );
}

double wxCmdLineArgImpl::GetDoubleVal() const
{
    Check(wxCmdLineParamType::Double);
    return m_doubleVal;
}

long wxCmdLineArgImpl::GetLongVal() const
{
    Check(wxCmdLineParamType::Number);
    return m_longVal;
}

const wxString& wxCmdLineArgImpl::GetStrVal() const
{
    Check(wxCmdLineParamType::String);
    return m_strVal;
}

#if wxUSE_DATETIME
const wxDateTime& wxCmdLineArgImpl::GetDateVal() const
{
    Check(wxCmdLineParamType::Date);
    return m_dateVal;
}
#endif // wxUSE_DATETIME

wxCmdLineArgImpl& wxCmdLineArgImpl::SetDoubleVal(double val)
{
    Check(wxCmdLineParamType::Double);
    m_doubleVal = val;
    m_hasVal = true;
    return *this;
}

wxCmdLineArgImpl& wxCmdLineArgImpl::SetLongVal(long val)
{
    Check(wxCmdLineParamType::Number);
    m_longVal = val;
    m_hasVal = true;
    return *this;
}

wxCmdLineArgImpl& wxCmdLineArgImpl::SetStrVal(const wxString& val)
{
    Check(wxCmdLineParamType::String);
    m_strVal = val;
    m_hasVal = true;
    return *this;
}

#if wxUSE_DATETIME
wxCmdLineArgImpl& wxCmdLineArgImpl::SetDateVal(const wxDateTime& val)
{
    Check(wxCmdLineParamType::Date);
    m_dateVal = val;
    m_hasVal = true;
    return *this;
}
#endif // wxUSE_DATETIME

// ----------------------------------------------------------------------------
// wxCmdLineArgsArrayRef
// ----------------------------------------------------------------------------

size_t wxCmdLineArgs::size() const
{
    return m_parser.m_data->m_parsedArguments.size();
}

// ----------------------------------------------------------------------------
// wxCmdLineArgsArrayRef::const_iterator
// ----------------------------------------------------------------------------

wxCmdLineArgs::const_iterator::reference
    wxCmdLineArgs::const_iterator::operator *() const
{
    return m_parser->m_data->m_parsedArguments[m_index];
}

wxCmdLineArgs::const_iterator::pointer
    wxCmdLineArgs::const_iterator::operator ->() const
{
    return &**this;
}

wxCmdLineArgs::const_iterator &wxCmdLineArgs::const_iterator::operator ++ ()
{
    ++m_index;
    return *this;
}

wxCmdLineArgs::const_iterator wxCmdLineArgs::const_iterator::operator ++ (int)
{
    wxCmdLineArgs::const_iterator tmp(*this);
    ++*this;
    return tmp;
}

wxCmdLineArgs::const_iterator &wxCmdLineArgs::const_iterator::operator -- ()
{
    --m_index;
    return *this;
}

wxCmdLineArgs::const_iterator wxCmdLineArgs::const_iterator::operator -- (int)
{
    wxCmdLineArgs::const_iterator tmp(*this);
    --*this;
    return tmp;
}

// ----------------------------------------------------------------------------
// wxCmdLineParserData
// ----------------------------------------------------------------------------

wxCmdLineParserData::wxCmdLineParserData()
#ifdef __UNIX_LIKE__
    : m_switchChars("-")
#else // !Unix
    : m_switchChars("/-")
#endif
{
}

namespace
{

// Small helper function setting locale for all categories.
//
// We define it because wxSetlocale() can't be easily used with wxScopeGuard as
// it has several overloads -- while this one can.
inline char *SetAllLocaleFacets(const char *loc)
{
    return wxSetlocale(LC_ALL, loc);
}

} // private namespace

void wxCmdLineParserData::SetArguments(int argc, char **argv)
{
    m_arguments.clear();

    // Command-line arguments are supposed to be in the user locale encoding
    // (what else?) but wxLocale probably wasn't initialized yet as we're
    // called early during the program startup and so our locale might not have
    // been set from the environment yet. To work around this problem we
    // temporarily change the locale here. The only drawback is that changing
    // the locale is thread-unsafe but precisely because we're called so early
    // it's hopefully safe to assume that no other threads had been created yet.
    const wxCharBuffer locOld(SetAllLocaleFacets(nullptr));
    SetAllLocaleFacets("");
    wxON_BLOCK_EXIT1( SetAllLocaleFacets, locOld.data() );

    for ( int n = 0; n < argc; n++ )
    {
        // try to interpret the string as being in the current locale
        wxString arg(argv[n]);

        // but just in case we guessed wrongly and the conversion failed, do
        // try to salvage at least something
        if ( arg.empty() && argv[n][0] != '\0' )
            arg = wxString(argv[n], wxConvISO8859_1);

        m_arguments.push_back(arg);
    }
}

void wxCmdLineParserData::SetArguments(int argc, wxChar **argv)
{
    m_arguments.clear();

    for ( int n = 0; n < argc; n++ )
    {
        m_arguments.push_back(boost::nowide::narrow(argv[n]));
    }
}

void wxCmdLineParserData::SetArguments([[maybe_unused]] int argc,
                                       const wxCmdLineArgsArray& argv)
{
    m_arguments = argv.GetArguments();
}

void wxCmdLineParserData::SetArguments(const wxString& cmdLine)
{
    m_arguments.clear();

    if(wxTheApp && wxTheApp->argc > 0)
        m_arguments.push_back(wxTheApp->argv[0]);
    else
        m_arguments.push_back({});

    std::vector<wxString> args = wxCmdLineParser::ConvertStringToArgs(cmdLine);

    m_arguments.insert(m_arguments.end(), args.begin(), args.end()); 
}

std::size_t wxCmdLineParserData::FindOption(const std::string& name)
{
    if ( !name.empty() )
    {
        const size_t count = m_options.size();
        for ( size_t n = 0; n < count; n++ )
        {
            if ( m_options[n].shortName == name )
            {
                // found
                return n;
            }
        }
    }

    return std::string::npos;
}

std::size_t wxCmdLineParserData::FindOptionByLongName(const std::string& name)
{
    const std::size_t count = m_options.size();
    for ( std::size_t n = 0; n < count; n++ )
    {
        if ( m_options[n].longName == name )
        {
            // found
            return n;
        }
    }

    return std::string::npos;
}

const wxCmdLineOption*
wxCmdLineParserData::FindOptionByAnyName(const std::string& name)
{
    auto i = FindOption(name);
    if ( i == std::string::npos )
    {
        i = FindOptionByLongName(name);

        if ( i == std::string::npos )
        {
            wxFAIL_MSG( "Unknown option " + name );
            return nullptr;
        }
    }

    return &m_options[i];
}

// ----------------------------------------------------------------------------
// construction and destruction
// ----------------------------------------------------------------------------

void wxCmdLineParser::Init()
{
    m_data = new wxCmdLineParserData;
}

void wxCmdLineParser::SetCmdLine(int argc, char **argv)
{
    m_data->SetArguments(argc, argv);
}

void wxCmdLineParser::SetCmdLine(int argc, wxChar **argv)
{
    m_data->SetArguments(argc, argv);
}

void wxCmdLineParser::SetCmdLine(int argc, const wxCmdLineArgsArray& argv)
{
    m_data->SetArguments(argc, argv);
}

void wxCmdLineParser::SetCmdLine(const wxString& cmdline)
{
    m_data->SetArguments(cmdline);
}

wxCmdLineParser::~wxCmdLineParser()
{
    delete m_data;
}

// ----------------------------------------------------------------------------
// options
// ----------------------------------------------------------------------------

void wxCmdLineParser::SetSwitchChars(const std::string& switchChars)
{
    m_data->m_switchChars = switchChars;
}

void wxCmdLineParser::EnableLongOptions(bool enable)
{
    m_data->m_enableLongOptions = enable;
}

bool wxCmdLineParser::AreLongOptionsEnabled() const
{
    return m_data->m_enableLongOptions;
}

void wxCmdLineParser::SetLogo(const std::string& logo)
{
    m_data->m_logo = logo;
}

// ----------------------------------------------------------------------------
// command line construction
// ----------------------------------------------------------------------------

void wxCmdLineParser::SetDesc(const wxCmdLineEntryDesc *desc)
{
    for ( ;; desc++ )
    {
        switch ( desc->kind )
        {
            case wxCmdLineEntryType::Switch:
                AddSwitch(desc->shortName, desc->longName,
                          wxGetTranslation(desc->description),
                          desc->flags);
                break;

            case wxCmdLineEntryType::Option:
                AddOption(desc->shortName, desc->longName,
                          wxGetTranslation(desc->description),
                          desc->type, desc->flags);
                break;

            case wxCmdLineEntryType::Param:
                AddParam(wxGetTranslation(desc->description),
                         desc->type, desc->flags);
                break;

            case wxCmdLineEntryType::UsageText:
                AddUsageText(wxGetTranslation(desc->description));
                break;

            default:
                wxFAIL_MSG( "unknown command line entry type" );
                [[fallthrough]];

            case wxCmdLineEntryType::None:
                return;
        }
    }
}

void wxCmdLineParser::AddSwitch(const wxString& shortName,
                                const wxString& longName,
                                const wxString& desc,
                                unsigned int flags)
{
    wxASSERT_MSG( m_data->FindOption(shortName) == wxNOT_FOUND,
                  "duplicate switch" );

    wxCmdLineOption option = wxCmdLineOption(wxCmdLineEntryType::Switch,
                                                  shortName, longName, desc,
                                                  wxCmdLineParamType::None, flags);

    m_data->m_options.push_back(option);
}

void wxCmdLineParser::AddOption(const wxString& shortName,
                                const wxString& longName,
                                const wxString& desc,
                                wxCmdLineParamType type,
                                unsigned int flags)
{
    wxASSERT_MSG( m_data->FindOption(shortName) == wxNOT_FOUND,
                  "duplicate option" );

    wxCmdLineOption option = wxCmdLineOption(wxCmdLineEntryType::Option,
                                                  shortName, longName, desc,
                                                  type, flags);

    m_data->m_options.push_back(option);
}

void wxCmdLineParser::AddParam(const wxString& desc,
                               wxCmdLineParamType type,
                               unsigned int flags)
{
    // do some consistency checks: a required parameter can't follow an
    // optional one and nothing should follow a parameter with MULTIPLE flag
#if wxDEBUG_LEVEL
    if ( !m_data->m_paramDesc.empty() )
    {
        wxCmdLineParam& param = m_data->m_paramDesc.back();

        wxASSERT_MSG( !(param.flags & wxCMD_LINE_PARAM_MULTIPLE),
                      "all parameters after the one with wxCMD_LINE_PARAM_MULTIPLE style will be ignored" );

        if ( !(flags & wxCMD_LINE_PARAM_OPTIONAL) )
        {
            wxASSERT_MSG( !(param.flags & wxCMD_LINE_PARAM_OPTIONAL),
                          "a required parameter can't follow an optional one" );
        }
    }
#endif // wxDEBUG_LEVEL

    wxCmdLineParam param = wxCmdLineParam(desc, type, flags);

    m_data->m_paramDesc.push_back(param);
}

void wxCmdLineParser::AddUsageText(const wxString& text)
{
    wxASSERT_MSG( !text.empty(), "text can't be empty" );

    wxCmdLineOption option = wxCmdLineOption(wxCmdLineEntryType::UsageText,
                                                  {}, {},
                                                  text, wxCmdLineParamType::None, 0);

    m_data->m_options.push_back(option);
}

// ----------------------------------------------------------------------------
// access to parse command line
// ----------------------------------------------------------------------------

bool wxCmdLineParser::Found(const wxString& name) const
{
    const wxCmdLineOption* const opt = m_data->FindOptionByAnyName(name);

    return opt && opt->HasValue();
}

wxCmdLineSwitchState wxCmdLineParser::FoundSwitch(const wxString& name) const
{
    const wxCmdLineOption* const opt = m_data->FindOptionByAnyName(name);

    if ( !opt || !opt->HasValue() )
        return wxCmdLineSwitchState::NotFound;

    return opt->IsNegated() ? wxCmdLineSwitchState::Off : wxCmdLineSwitchState::On;
}

bool wxCmdLineParser::Found(const wxString& name, wxString *value) const
{
    const wxCmdLineOption* const opt = m_data->FindOptionByAnyName(name);

    if ( !opt || !opt->HasValue() )
        return false;

    wxCHECK_MSG( value, false, "NULL pointer in wxCmdLineOption::Found" );

    *value = opt->GetStrVal();

    return true;
}

bool wxCmdLineParser::Found(const wxString& name, long *value) const
{
    const wxCmdLineOption* const opt = m_data->FindOptionByAnyName(name);

    if ( !opt || !opt->HasValue() )
        return false;

    wxCHECK_MSG( value, false, "NULL pointer in wxCmdLineOption::Found" );

    *value = opt->GetLongVal();

    return true;
}

bool wxCmdLineParser::Found(const wxString& name, double *value) const
{
    const wxCmdLineOption* const opt = m_data->FindOptionByAnyName(name);

    if ( !opt || !opt->HasValue() )
        return false;

    wxCHECK_MSG( value, false, "NULL pointer in wxCmdLineOption::Found" );

    *value = opt->GetDoubleVal();

    return true;
}

#if wxUSE_DATETIME
bool wxCmdLineParser::Found(const wxString& name, wxDateTime *value) const
{
    const wxCmdLineOption* const opt = m_data->FindOptionByAnyName(name);

    if ( !opt || !opt->HasValue() )
        return false;

    wxCHECK_MSG( value, false, "NULL pointer in wxCmdLineOption::Found" );

    *value = opt->GetDateVal();

    return true;
}
#endif // wxUSE_DATETIME

size_t wxCmdLineParser::GetParamCount() const
{
    return m_data->m_parameters.size();
}

wxString wxCmdLineParser::GetParam(size_t n) const
{
    wxCHECK_MSG( n < GetParamCount(), {}, "invalid param index" );

    return m_data->m_parameters[n];
}

// Resets switches and options
void wxCmdLineParser::Reset()
{
    for ( size_t i = 0; i < m_data->m_options.size(); i++ )
    {
        m_data->m_options[i].Reset();
    }

    m_data->m_parsedArguments.clear();
}


// ----------------------------------------------------------------------------
// the real work is done here
// ----------------------------------------------------------------------------

int wxCmdLineParser::Parse(bool showUsage)
{
    bool maybeOption = true;    // can the following arg be an option?
    bool ok = true;             // true until an error is detected
    bool helpRequested = false; // true if "-h" was given
    bool hadRepeatableParam = false; // true if found param with MULTIPLE flag

    size_t currentParam = 0;    // the index in m_paramDesc

    size_t countParam = m_data->m_paramDesc.size();
    std::string errorMsg;

    Reset();

    // parse everything
    m_data->m_parameters.clear();
    wxString arg;
    size_t count = m_data->m_arguments.size();
    for ( size_t n = 1; ok && (n < count); n++ )    // 0 is program name
    {
        arg = m_data->m_arguments[n];

        // special case: "--" should be discarded and all following arguments
        // should be considered as parameters, even if they start with '-' and
        // not like options (this is POSIX-like)
        if ( arg == "--" )
        {
            maybeOption = false;

            continue;
        }
#ifdef __WXOSX__
        if ( arg == "-ApplePersistenceIgnoreState" ||
             arg == "-AppleTextDirection" ||
             arg == "-AppleLocale" ||
             arg == "-AppleLanguages" )
        {
            maybeOption = false;
            n++;

            continue;
        }
#endif

        // empty argument or just '-' is not an option but a parameter
        if ( maybeOption && arg.length() > 1 &&
                // FIXME-UTF8: use wc_str() after removing ANSI build
                std::strchr(m_data->m_switchChars.c_str(), arg[0u]) )
        {
            bool isLong;
            std::string name;
            int optInd = wxNOT_FOUND;   // init to suppress warnings

            // an option or a switch: find whether it's a long or a short one
            if ( arg.length() >= 3 && arg[0u] == '-' && arg[1u] == '-' )
            {
                // a long one
                isLong = true;

                // Skip leading "--"
                auto p = arg.begin() + 2;

                bool longOptionsEnabled = AreLongOptionsEnabled();

                name = GetLongOptionName(p, arg.end());

                if (longOptionsEnabled)
                {
                    std::string errorOpt;

                    optInd = m_data->FindOptionByLongName(name);
                    if ( optInd == wxNOT_FOUND )
                    {
                        // Check if this could be a negatable long option.
                        if ( name.back() == '-' )
                        {
                            name.pop_back();

                            optInd = m_data->FindOptionByLongName(name);
                            if ( optInd != wxNOT_FOUND )
                            {
                                if ( !(m_data->m_options[optInd].flags &
                                        wxCMD_LINE_SWITCH_NEGATABLE) )
                                {
                                    errorOpt = fmt::format(fmt::runtime(_("Option '{}' can't be negated")), name);
                                    optInd = wxNOT_FOUND;
                                }
                            }
                        }

                        if ( optInd == wxNOT_FOUND )
                        {
                            if ( errorOpt.empty() )
                            {
                                errorOpt = fmt::format(fmt::runtime(_("Unknown long option '{}'")), name);
                            }

                            errorMsg += fmt::format("{}\n", errorOpt);
                        }
                    }
                }
                else
                {
                    optInd = wxNOT_FOUND; // Sanity check

                    // Print the argument including leading "--"
                    name.insert(0, "--");
                    // name.Prepend( "--" );
                    errorMsg += fmt::format(fmt::runtime(_("Unknown option '{}'\n")), name);
                }

            }
            else // not a long option
            {
                isLong = false;

                // a short one: as they can be cumulated, we try to find the
                // longest substring which is a valid option
                auto p = arg.begin() + 1;

                name = GetShortOptionName(p, arg.end());

                size_t len = name.length();
                do
                {
                    if ( len == 0 )
                    {
                        // we couldn't find a valid option name in the
                        // beginning of this string
                        errorMsg += fmt::format(fmt::runtime(_("Unknown option '{}'\n")), name);

                        break;
                    }
                    else
                    {
                        optInd = m_data->FindOption(name.substr(len));

                        // will try with one character less the next time
                        len--;
                    }
                }
                while ( optInd == wxNOT_FOUND );

                len++;  // compensates extra len-- above
                if ( (optInd != wxNOT_FOUND) && (len != name.length()) )
                {
                    // first of all, the option name is only part of this
                    // string
                    name = name.substr(len);

                    // our option is only part of this argument, there is
                    // something else in it - it is either the value of this
                    // option or other switches if it is a switch
                    if ( m_data->m_options[(size_t)optInd].kind
                            == wxCmdLineEntryType::Switch )
                    {
                        // if the switch is negatable and it is just followed
                        // by '-' the '-' is considered to be part of this
                        // switch
                        if ( (m_data->m_options[(size_t)optInd].flags &
                                    wxCMD_LINE_SWITCH_NEGATABLE) &&
                                arg[len] == '-' )
                            ++len;

                        // pretend that all the rest of the argument is the
                        // next argument, in fact
                        wxString arg2 = arg[0u];
                        arg2 += arg.substr(len + 1); // +1 for leading '-'
                        // TODO: Verify this change after changing to std::vector
                        m_data->m_arguments.insert
                            (m_data->m_arguments.begin() + n + 1, arg2);
                        count++;

                        // only leave the part which wasn't extracted into the
                        // next argument in this one
                        arg = arg.substr(len + 1);
                    }
                    //else: it's our value, we'll deal with it below
                }
            }

            if ( optInd == wxNOT_FOUND )
            {
                ok = false;

                continue;   // will break, in fact
            }

            // look at what follows:

            // +1 for leading '-'
            auto p = arg.begin() + 1 + name.length();
            auto end = arg.end();

            if ( isLong )
                ++p;    // for another leading '-'

            wxCmdLineOption& opt = m_data->m_options[(size_t)optInd];
            if ( opt.kind == wxCmdLineEntryType::Switch )
            {
                // we must check that there is no value following the switch
                bool negated = (opt.flags & wxCMD_LINE_SWITCH_NEGATABLE) &&
                                    p != arg.end() && *p == '-';

                if ( !negated && p != arg.end() )
                {
                    errorMsg += fmt::format(fmt::runtime(_("Unexpected characters following option '{}'.\n")), name);

                    ok = false;
                }
                else // no value, as expected
                {
                    // nothing more to do
                    opt.SetHasValue();
                    if ( negated )
                        opt.SetNegated();

                    if ( opt.flags & wxCMD_LINE_OPTION_HELP )
                    {
                        helpRequested = true;

                        // it's not an error, but we still stop here
                        ok = false;
                    }
                }
            }
            else // it's an option. not a switch
            {
                switch ( p == end ? '\0' : *p )
                {
                    case '=':
                    case ':':
                        // the value follows
                        ++p;
                        break;

                    case '\0':
                        // the value is in the next argument
                        if ( ++n == count )
                        {
                            // ... but there is none
                            errorMsg += fmt::format(fmt::runtime(_("Option '{}' requires a value.\n")), name);

                            ok = false;
                        }
                        else
                        {
                            // ... take it from there
                            p = m_data->m_arguments[n].begin();
                            end = m_data->m_arguments[n].end();
                        }
                        break;

                    default:
                        // the value is right here: this may be legal or
                        // not depending on the option style
                        if ( opt.flags & wxCMD_LINE_NEEDS_SEPARATOR )
                        {
                            errorMsg += fmt::format(fmt::runtime(_("Separator expected after the option '{}'.\n")), name);

                            ok = false;
                        }
                }

                if ( ok )
                {
                    // FIXME: Indexing with arguments from wxString.
                    wxString value{p, end};
                    switch ( opt.type )
                    {
                        default:
                            wxFAIL_MSG( "unknown option type" );
                            [[fallthrough]];

                        case wxCmdLineParamType::String:
                            opt.SetStrVal(value);
                            break;

                        case wxCmdLineParamType::Number:
                            {
                                long val;
                                auto [p, ec] = std::from_chars(value.data(), value.data() + value.size(), val);

                                if ( ec == std::errc() )
                                {
                                    opt.SetLongVal(val);
                                }
                                else
                                {
                                    errorMsg += fmt::format(fmt::runtime(_("'{}' is not a correct numeric value for option '{}'.\n")), value.ToStdString(), name);

                                    ok = false;
                                }
                            }
                            break;

                        case wxCmdLineParamType::Double:
                            {
                                double val;
                                auto [p, ec] = std::from_chars(value.data(), value.data() + value.size(), val);

                                if ( ec == std::errc() )
                                {
                                    opt.SetDoubleVal(val);
                                }
                                else
                                {
                                    errorMsg += fmt::format(fmt::runtime(_("'{}' is not a correct numeric value for option '{}'.\n")), value.ToStdString(), name);

                                    ok = false;
                                }
                            }
                            break;

#if wxUSE_DATETIME
                        case wxCmdLineParamType::Date:
                            {
                                wxDateTime dt;
                                wxString::const_iterator endDate;
                                wxString tmpValue{value};
                                if ( !dt.ParseDate(tmpValue, &endDate) || endDate != tmpValue.end() )
                                {
                                    errorMsg += fmt::format(fmt::runtime(_("Option '{}': '{}' cannot be converted to a date.\n")), name, value.ToStdString());

                                    ok = false;
                                }
                                else
                                {
                                    opt.SetDateVal(dt);
                                }
                            }
                            break;
#endif // wxUSE_DATETIME
                    }
                }
            }

            if (ok)
                m_data->m_parsedArguments.push_back(opt);
        }
        else // not an option, must be a parameter
        {
            if ( currentParam < countParam )
            {
                wxCmdLineParam& param = m_data->m_paramDesc[currentParam];

                // TODO check the param type

                m_data->m_parameters.push_back(arg);
                m_data->m_parsedArguments.push_back (
                    wxCmdLineArgImpl(wxCmdLineEntryType::Param, wxString(), wxString(),
                                     wxCmdLineParamType::String).SetStrVal(arg));

                if ( !(param.flags & wxCMD_LINE_PARAM_MULTIPLE) )
                {
                    currentParam++;
                }
                else
                {
                    wxASSERT_MSG( currentParam == countParam - 1,
                                  "all parameters after the one with wxCMD_LINE_PARAM_MULTIPLE style are ignored" );

                    // remember that we did have this last repeatable parameter
                    hadRepeatableParam = true;
                }
            }
            else
            {
                errorMsg += fmt::format(fmt::runtime(_("Unexpected parameter '{}'\n")), arg.ToStdString());

                ok = false;
            }
        }
    }

    // verify that all mandatory options were given
    if ( ok )
    {
        size_t countOpt = m_data->m_options.size();
        for ( size_t n = 0; ok && (n < countOpt); n++ )
        {
            wxCmdLineOption& opt = m_data->m_options[n];
            if ( (opt.flags & wxCMD_LINE_OPTION_MANDATORY) && !opt.HasValue() )
            {
                std::string optName;
                if ( opt.longName.empty() )
                {
                    optName = opt.shortName;
                }
                else
                {
                    if ( AreLongOptionsEnabled() )
                    {
                        optName = fmt::format( fmt::runtime(_("{} (or {})")), opt.shortName, opt.longName );
                    }
                    else
                    {
                        optName = fmt::format( "{}", opt.shortName );
                    }
                }

                errorMsg += fmt::format(fmt::runtime(_("The value for the option '{}' must be specified.\n")), optName);

                ok = false;
            }
        }

        for ( ; ok && (currentParam < countParam); currentParam++ )
        {
            wxCmdLineParam& param = m_data->m_paramDesc[currentParam];
            if ( (currentParam == countParam - 1) &&
                 (param.flags & wxCMD_LINE_PARAM_MULTIPLE) &&
                 hadRepeatableParam )
            {
                // special case: currentParam wasn't incremented, but we did
                // have it, so don't give error
                continue;
            }

            if ( !(param.flags & wxCMD_LINE_PARAM_OPTIONAL) )
            {
                errorMsg += fmt::format(fmt::runtime(_("The required parameter '{}' was not specified.\n")), param.description.ToStdString());

                ok = false;
            }
        }
    }

    // if there was an error during parsing the command line, show this error
    // and also the usage message if it had been requested
    if ( !ok && (!errorMsg.empty() || (helpRequested && showUsage)) )
    {
        wxMessageOutput* msgOut = wxMessageOutput::Get();
        if ( msgOut )
        {
            wxString usage;
            if ( showUsage )
                usage = GetUsageString();

            msgOut->Printf( "%s%s", usage.c_str(), errorMsg.c_str() );
        }
        else
        {
            wxFAIL_MSG( "no wxMessageOutput object?" );
        }
    }

    return ok ? 0 : helpRequested ? -1 : 1;
}

// ----------------------------------------------------------------------------
// give the usage message
// ----------------------------------------------------------------------------

void wxCmdLineParser::Usage() const
{
    wxMessageOutput* msgOut = wxMessageOutput::Get();
    if ( msgOut )
    {
        msgOut->Printf( "%s", GetUsageString().c_str() );
    }
    else
    {
        wxFAIL_MSG( "no wxMessageOutput object?" );
    }
}

std::string wxCmdLineParser::GetUsageString() const
{
    std::string appname;
    if ( m_data->m_arguments.empty() )
    {
        if ( wxTheApp )
            appname = wxTheApp->GetAppName();
    }
    else // use argv[0]
    {
        appname = wxFileName(m_data->m_arguments[0]).GetName();
    }

    // we construct the brief cmd line desc on the fly, but not the detailed
    // help message below because we want to align the options descriptions
    // and for this we must first know the longest one of them
    std::string usage;

    std::vector<std::string> namesOptions;
    std::vector<std::string> descOptions;

    if ( !m_data->m_logo.empty() )
    {
        usage += fmt::format("{}\n", m_data->m_logo);
    }

    // FIXME: Provide a way to have translations in fmt lib strings.
    usage +=  fmt::format(fmt::runtime(_("Usage: {}")), appname);

    // the switch char is usually '-' but this can be changed with
    // SetSwitchChars() and then the first one of possible chars is used
    char chSwitch = m_data->m_switchChars.empty() ? '-'
                                             : m_data->m_switchChars[0u];

    bool areLongOptionsEnabled = AreLongOptionsEnabled();
    size_t count = m_data->m_options.size();
    for ( size_t n = 0; n < count; n++ )
    {
        wxCmdLineOption& opt = m_data->m_options[n];

        std::string option;
        std::string negator;

        if ( opt.flags & wxCMD_LINE_HIDDEN )
            continue;

        if ( opt.kind != wxCmdLineEntryType::UsageText )
        {
            usage += ' ';
            if ( !(opt.flags & wxCMD_LINE_OPTION_MANDATORY) )
            {
                usage += '[';
            }

            if ( opt.flags & wxCMD_LINE_SWITCH_NEGATABLE )
                negator = "[-]";

            if ( !opt.shortName.empty() )
            {
                usage += fmt::format("{}{}{}", chSwitch, opt.shortName, negator);
            }
            else if ( areLongOptionsEnabled && !opt.longName.empty() )
            {
                usage += fmt::format("--{}{}", opt.longName, negator);
            }
            else
            {
                if (!opt.longName.empty())
                {
                    wxFAIL_MSG( "option with only a long name while long "
                                "options are disabled" );
                }
                else
                {
                    wxFAIL_MSG( "option without neither short nor long name" );
                }
            }

            if ( !opt.shortName.empty() )
            {
                option += fmt::format("  {}{}", chSwitch, opt.shortName);
            }

            if ( areLongOptionsEnabled && !opt.longName.empty() )
            {
                option += fmt::format("{}--{}", (option.empty() ? "  " : ", "), opt.longName);
            }

            if ( opt.kind != wxCmdLineEntryType::Switch )
            {
                auto val = fmt::format("<{}>", GetTypeName(opt.type));
                usage += fmt::format(" {}", val);
                option += fmt::format("{}{}", (opt.longName.empty() ? ':' : '='), val);
            }

            if ( !(opt.flags & wxCMD_LINE_OPTION_MANDATORY) )
            {
                usage += ']';
            }
        }

        namesOptions.push_back(option);
        descOptions.push_back(opt.description);
    }

    count = m_data->m_paramDesc.size();
    for ( size_t n = 0; n < count; n++ )
    {
        wxCmdLineParam& param = m_data->m_paramDesc[n];

        if ( param.flags & wxCMD_LINE_HIDDEN )
            continue;

        usage += ' ';

        if ( param.flags & wxCMD_LINE_PARAM_OPTIONAL )
        {
            usage += '[';
        }

        usage += param.description;

        if ( param.flags & wxCMD_LINE_PARAM_MULTIPLE )
        {
            usage += "...";
        }

        if ( param.flags & wxCMD_LINE_PARAM_OPTIONAL )
        {
            usage += ']';
        }
    }

    usage += '\n';

    // set to number of our own options, not counting the standard ones
    count = namesOptions.size();

    // get option names & descriptions for standard options, if any:
    std::string stdDesc;
    if ( wxAppTraits *traits = wxApp::GetTraitsIfExists() )
        stdDesc = traits->GetStandardCmdLineOptions(namesOptions, descOptions);

    // now construct the detailed help message
    size_t len, lenMax = 0;
    for ( size_t n = 0; n < namesOptions.size(); n++ )
    {
        len = namesOptions[n].length();
        if ( len > lenMax )
            lenMax = len;
    }

    for ( size_t n = 0; n < namesOptions.size(); n++ )
    {
        if ( n == count )
            usage += fmt::format("\n{}", stdDesc);

        len = namesOptions[n].length();
        // desc contains text if name is empty
        if (len == 0)
        {
            usage += fmt::format("{}\n", descOptions[n]);
        }
        else
        {
            std::string spacing(std::string::size_type{lenMax - len}, ' ');
            usage += fmt::format("{}{}\t{}\n", namesOptions[n], spacing, descOptions[n]);
        }
    }

    return usage;
}

// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

static std::string GetTypeName(wxCmdLineParamType type)
{
    wxString s;
    switch ( type )
    {
        default:
            wxFAIL_MSG( "unknown option type" );
            [[fallthrough]];

        case wxCmdLineParamType::String:
            s = _("str");
            break;

        case wxCmdLineParamType::Number:
            s = _("num");
            break;

        case wxCmdLineParamType::Double:
            s = _("double");
            break;

        case wxCmdLineParamType::Date:
            s = _("date");
            break;
    }

    return s.ToStdString();
}

/*
Returns a string which is equal to the string pointed to by p, but up to the
point where p contains an character that's not allowed.
Allowable characters are letters and numbers, and characters pointed to by
the parameter allowedChars.

For example, if p points to "abcde-@-_", and allowedChars is "-_",
this function returns "abcde-".
*/
static std::string GetOptionName(wxString::const_iterator p,
                                 wxString::const_iterator end,
                                 const wxChar* allowedChars)
{
    std::string argName;

    while ( p != end && (wx::utils::isAlNum(*p) || wxStrchr(allowedChars, *p)) )
    {
        argName += *p++;
    }

    return argName;
}

// Besides alphanumeric characters, short and long options can
// have other characters.

// A short option additionally can have these // FIXME: Convert to narrow string
#define wxCMD_LINE_CHARS_ALLOWED_BY_SHORT_OPTION L"_?"

// A long option can have the same characters as a short option and a '-'. // FIXME: Convert to narrow string.
#define wxCMD_LINE_CHARS_ALLOWED_BY_LONG_OPTION \
    wxCMD_LINE_CHARS_ALLOWED_BY_SHORT_OPTION L"-"

static std::string GetShortOptionName(wxString::const_iterator p,
                                      wxString::const_iterator end)
{
    return GetOptionName(p, end, wxCMD_LINE_CHARS_ALLOWED_BY_SHORT_OPTION);
}

static std::string GetLongOptionName(wxString::const_iterator p,
                                     wxString::const_iterator end)
{
    return GetOptionName(p, end, wxCMD_LINE_CHARS_ALLOWED_BY_LONG_OPTION);
}

#endif // wxUSE_CMDLINE_PARSER

// ----------------------------------------------------------------------------
// global functions
// ----------------------------------------------------------------------------

/*
   This function is mainly used under Windows (as under Unix we always get the
   command line arguments as argc/argv anyhow) and so it tries to follow
   Windows conventions for the command line handling, not Unix ones. For
   instance, backslash is not special except when it precedes double quote when
   it does quote it.

   TODO: Rewrite this to follow the even more complicated rule used by Windows
         CommandLineToArgv():

    * A string of backslashes not followed by a quotation mark has no special
      meaning.
    * An even number of backslashes followed by a quotation mark is treated as
      pairs of protected backslashes, followed by a word terminator.
    * An odd number of backslashes followed by a quotation mark is treated as
      pairs of protected backslashes, followed by a protected quotation mark.

    See http://blogs.msdn.com/b/oldnewthing/archive/2010/09/17/10063629.aspx

    It could also be useful to provide a converse function which is also
    non-trivial, see

    http://blogs.msdn.com/b/twistylittlepassagesallalike/archive/2011/04/23/everyone-quotes-arguments-the-wrong-way.aspx
 */

/* static */
std::vector<wxString>
wxCmdLineParser::ConvertStringToArgs(const wxString& cmdline,
                                     wxCmdLineSplitType type)
{
    std::vector<wxString> args;

    wxString arg;
    arg.reserve(1024);

    const wxString::const_iterator end = cmdline.end();
    wxString::const_iterator p = cmdline.begin();

    for ( ;; )
    {
        // skip white space
        while ( p != end && (*p == ' ' || *p == '\t') )
            ++p;

        // anything left?
        if ( p == end )
            break;

        // parse this parameter
        bool lastBS = false,
             isInsideQuotes = false;
        wxChar chDelim = '\0';
        for ( arg.clear(); p != end; ++p )
        {
            const wxChar ch = *p;

            if ( type == wxCmdLineSplitType::DOS )
            {
                if ( ch == '"' )
                {
                    if ( !lastBS )
                    {
                        isInsideQuotes = !isInsideQuotes;

                        // don't put quote in arg
                        continue;
                    }
                    //else: quote has no special meaning but the backslash
                    //      still remains -- makes no sense but this is what
                    //      Windows does
                }
                // note that backslash does *not* quote the space, only quotes do
                else if ( !isInsideQuotes && (ch == ' ' || ch == '\t') )
                {
                    ++p;    // skip this space anyhow
                    break;
                }

                lastBS = !lastBS && ch == '\\';
            }
            else // type == wxCMD_LINE_SPLIT_UNIX
            {
                if ( !lastBS )
                {
                    if ( isInsideQuotes )
                    {
                        if ( ch == chDelim )
                        {
                            isInsideQuotes = false;

                            continue;   // don't use the quote itself
                        }
                    }
                    else // not in quotes and not escaped
                    {
                        if ( ch == '\'' || ch == '"' )
                        {
                            isInsideQuotes = true;
                            chDelim = ch;

                            continue;   // don't use the quote itself
                        }

                        if ( ch == ' ' || ch == '\t' )
                        {
                            ++p;    // skip this space anyhow
                            break;
                        }
                    }

                    lastBS = ch == '\\';
                    if ( lastBS )
                        continue;
                }
                else // escaped by backslash, just use as is
                {
                    lastBS = false;
                }
            }

            arg += ch;
        }

        args.push_back(arg);
    }

    return args;
}
