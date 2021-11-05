///////////////////////////////////////////////////////////////////////////////
// Name:        wx/cmdline.h
// Purpose:     wxCmdLineParser and related classes for parsing the command
//              line options
// Author:      Vadim Zeitlin
// Modified by:
// Created:     04.01.00
// Copyright:   (c) 2000 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CMDLINE_H_
#define _WX_CMDLINE_H_

#include "wx/defs.h"

#include "wx/string.h"
#include "wx/cmdargs.h"

#include <vector>

// determines ConvertStringToArgs() behaviour
enum class wxCmdLineSplitType
{
    DOS,
    UNIX
};

#if wxUSE_CMDLINE_PARSER

class wxCmdLineParser;
class wxDateTime;

// by default, options are optional (sic) and each call to AddParam() allows
// one more parameter - this may be changed by giving non-default flags to it
enum wxCmdLineEntryFlags
{
    wxCMD_LINE_OPTION_MANDATORY = 0x01, // this option must be given
    wxCMD_LINE_PARAM_OPTIONAL   = 0x02, // the parameter may be omitted
    wxCMD_LINE_PARAM_MULTIPLE   = 0x04, // the parameter may be repeated
    wxCMD_LINE_OPTION_HELP      = 0x08, // this option is a help request
    wxCMD_LINE_NEEDS_SEPARATOR  = 0x10, // must have sep before the value
    wxCMD_LINE_SWITCH_NEGATABLE = 0x20, // this switch can be negated (e.g. /S-)
    wxCMD_LINE_HIDDEN           = 0x40  // this switch is not listed by Usage()
};

// an option value or parameter may be a string (the most common case), a
// number or a date
enum class wxCmdLineParamType
{
    String,  // should be 0 (default)
    Number,
    Date,
    Double,
    None
};

// for constructing the cmd line description using Init()
enum class wxCmdLineEntryType
{
    Switch,
    Option,
    Param,
    UsageText,
    None         // to terminate the list
};

// Possible return values of wxCmdLineParser::FoundSwitch()
enum class wxCmdLineSwitchState
{
    Off,  // Found but turned off/negated.
    NotFound, // Not found at all.
    On         // Found in normal state.
};

// ----------------------------------------------------------------------------
// wxCmdLineEntryDesc is a description of one command line
// switch/option/parameter
// ----------------------------------------------------------------------------

struct wxCmdLineEntryDesc
{
    wxCmdLineEntryType kind;
    const char *shortName;
    const char *longName;
    const char *description;
    wxCmdLineParamType type;
    unsigned int flags;
};

// the list of wxCmdLineEntryDesc objects should be terminated with this one
#define wxCMD_LINE_DESC_END \
        { wxCmdLineEntryType::None, NULL, NULL, NULL, wxCmdLineParamType::None, 0x0 }

// ----------------------------------------------------------------------------
// wxCmdLineArg contains the value for one command line argument
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_BASE wxCmdLineArg
{
public:
    virtual ~wxCmdLineArg() = default;

    virtual double GetDoubleVal() const = 0;
    virtual long GetLongVal() const = 0;
    virtual const wxString& GetStrVal() const = 0;
#if wxUSE_DATETIME
    virtual const wxDateTime& GetDateVal() const = 0;
#endif // wxUSE_DATETIME

    virtual bool IsNegated() const = 0;

    virtual wxCmdLineEntryType GetKind() const = 0;
    virtual wxString GetShortName() const = 0;
    virtual wxString GetLongName() const = 0;
    virtual wxCmdLineParamType GetType() const = 0;
};

// ----------------------------------------------------------------------------
// wxCmdLineArgs is a container of command line arguments actually parsed and
// allows enumerating them using the standard iterator-based approach.
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_BASE wxCmdLineArgs
{
public:
    class WXDLLIMPEXP_BASE const_iterator
    {
    public:
        using difference_type = int;
        using value_type = wxCmdLineArg;
        using pointer = const wxCmdLineArg *;
        using reference = const wxCmdLineArg &;

// We avoid dependency on standard library by default but if we do use
// std::string, then it's ok to use iterator tags as well.
        using iterator_category = std::bidirectional_iterator_tag;

        const_iterator()  = default;
        reference operator *() const;
        pointer operator ->() const;
        const_iterator &operator ++ ();
        const_iterator operator ++ (int);
        const_iterator &operator -- ();
        const_iterator operator -- (int);

        bool operator == (const const_iterator &other) const {
            return m_parser==other.m_parser && m_index==other.m_index;
        }
        bool operator != (const const_iterator &other) const {
            return !operator==(other);
        }

    private:
        const_iterator (const wxCmdLineParser& parser, size_t index)
            : m_parser(&parser), m_index(index) {
        }

        const wxCmdLineParser* m_parser{nullptr};
        size_t m_index{0};

        friend class wxCmdLineArgs;
    };

    wxCmdLineArgs (const wxCmdLineParser& parser) : m_parser(parser) {}

    const_iterator begin() const { return const_iterator(m_parser, 0); }
    const_iterator end() const { return const_iterator(m_parser, size()); }

    size_t size() const;

private:
    const wxCmdLineParser& m_parser;
};

// ----------------------------------------------------------------------------
// wxCmdLineParser is a class for parsing command line.
//
// It has the following features:
//
// 1. distinguishes options, switches and parameters; allows option grouping
// 2. allows both short and long options
// 3. automatically generates the usage message from the cmd line description
// 4. does type checks on the options values (number, date, ...)
//
// To use it you should:
//
// 1. construct it giving it the cmd line to parse and optionally its desc
// 2. construct the cmd line description using AddXXX() if not done in (1)
// 3. call Parse()
// 4. use GetXXX() to retrieve the parsed info
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_BASE wxCmdLineParser
{
public:
    // ctors and initializers
    // ----------------------

    // default ctor or ctor giving the cmd line in either Unix or Win form
    wxCmdLineParser() { Init(); }
    wxCmdLineParser(int argc, char **argv) { Init(); SetCmdLine(argc, argv); }
    wxCmdLineParser(int argc, wxChar **argv) { Init(); SetCmdLine(argc, argv); }
    wxCmdLineParser(int argc, const wxCmdLineArgsArray& argv)
        { Init(); SetCmdLine(argc, argv); }
    wxCmdLineParser(const wxString& cmdline) { Init(); SetCmdLine(cmdline); }

    // the same as above, but also gives the cmd line description - otherwise,
    // use AddXXX() later
    wxCmdLineParser(const wxCmdLineEntryDesc *desc)
        { Init(); SetDesc(desc); }
    wxCmdLineParser(const wxCmdLineEntryDesc *desc, int argc, char **argv)
        { Init(); SetCmdLine(argc, argv); SetDesc(desc); }
    wxCmdLineParser(const wxCmdLineEntryDesc *desc, int argc, wxChar **argv)
        { Init(); SetCmdLine(argc, argv); SetDesc(desc); }
    wxCmdLineParser(const wxCmdLineEntryDesc *desc,
                    int argc,
                    const wxCmdLineArgsArray& argv)
        { Init(); SetCmdLine(argc, argv); SetDesc(desc); }
    wxCmdLineParser(const wxCmdLineEntryDesc *desc, const wxString& cmdline)
        { Init(); SetCmdLine(cmdline); SetDesc(desc); }

    // set cmd line to parse after using one of the ctors which don't do it
    void SetCmdLine(int argc, char **argv);
    void SetCmdLine(int argc, wxChar **argv);
    void SetCmdLine(int argc, const wxCmdLineArgsArray& argv);
    void SetCmdLine(const wxString& cmdline);

    ~wxCmdLineParser();

	wxCmdLineParser& operator=(wxCmdLineParser&&) = delete;

    // set different parser options
    // ----------------------------

    // by default, '-' is switch char under Unix, '-' or '/' under Win:
    // switchChars contains all characters with which an option or switch may
    // start
    void SetSwitchChars(const wxString& switchChars);

    // long options are not POSIX-compliant, this option allows to disable them
    void EnableLongOptions(bool enable = true);
    void DisableLongOptions() { EnableLongOptions(false); }

    bool AreLongOptionsEnabled() const;

    // extra text may be shown by Usage() method if set by this function
    void SetLogo(const wxString& logo);

    // construct the cmd line description
    // ----------------------------------

    // take the cmd line description from the wxCmdLineEntryType::None terminated table
    void SetDesc(const wxCmdLineEntryDesc *desc);

    // a switch: i.e. an option without value
    void AddSwitch(const wxString& name, const wxString& lng = {},
                   const wxString& desc = {},
                   unsigned int flags = 0);
    void AddLongSwitch(const wxString& lng,
                       const wxString& desc = {},
                       unsigned int flags = 0)
    {
        AddSwitch(wxString(), lng, desc, flags);
    }

    // an option taking a value of the given type
    void AddOption(const wxString& name, const wxString& lng = {},
                   const wxString& desc = {},
                   wxCmdLineParamType type = wxCmdLineParamType::String,
                   unsigned int flags = 0);
    void AddLongOption(const wxString& lng,
                       const wxString& desc = {},
                       wxCmdLineParamType type = wxCmdLineParamType::String,
                       unsigned int flags = 0)
    {
        AddOption(wxString(), lng, desc, type, flags);
    }

    // a parameter
    void AddParam(const wxString& desc = {},
                  wxCmdLineParamType type = wxCmdLineParamType::String,
                  unsigned int flags = 0);

    // add an explanatory text to be shown to the user in help
    void AddUsageText(const wxString& text);

    // actions
    // -------

    // parse the command line, return 0 if ok, -1 if "-h" or "--help" option
    // was encountered and the help message was given or a positive value if a
    // syntax error occurred
    //
    // if showUsage is true, Usage() is called in case of syntax error or if
    // help was requested
    int Parse(bool showUsage = true);

    // give the usage message describing all program options
    void Usage() const;

    // return the usage string, call Usage() to directly show it to the user
    wxString GetUsageString() const;

    // get the command line arguments
    // ------------------------------

    // returns true if the given switch was found
    bool Found(const wxString& name) const;

    // Returns wxCmdLineSwitchState::NotFound if the switch was not found at all,
    // wxCmdLineSwitchState::On if it was found in normal state and wxCmdLineSwitchState::Off if
    // it was found but negated (i.e. followed by "-", this can only happen for
    // the switches with wxCMD_LINE_SWITCH_NEGATABLE flag).
    wxCmdLineSwitchState FoundSwitch(const wxString& name) const;

    // returns true if an option taking a string value was found and stores the
    // value in the provided pointer
    bool Found(const wxString& name, wxString *value) const;

    // returns true if an option taking an integer value was found and stores
    // the value in the provided pointer
    bool Found(const wxString& name, long *value) const;

    // returns true if an option taking a double value was found and stores
    // the value in the provided pointer
    bool Found(const wxString& name, double *value) const;

#if wxUSE_DATETIME
    // returns true if an option taking a date value was found and stores the
    // value in the provided pointer
    bool Found(const wxString& name, wxDateTime *value) const;
#endif // wxUSE_DATETIME

    // gets the number of parameters found
    size_t GetParamCount() const;

    // gets the value of Nth parameter (as string only for now)
    wxString GetParam(size_t n = 0u) const;

    // returns a reference to the container of all command line arguments
    wxCmdLineArgs GetArguments() const { return wxCmdLineArgs(*this); }

    // Resets switches and options
    void Reset();

    // break down the command line in arguments
    static std::vector<wxString>
    ConvertStringToArgs(const wxString& cmdline,
                        wxCmdLineSplitType type = wxCmdLineSplitType::DOS);

private:
    // common part of all ctors
    void Init();

    struct wxCmdLineParserData *m_data;

    friend class wxCmdLineArgs;
    friend class wxCmdLineArgs::const_iterator;
};

#else // !wxUSE_CMDLINE_PARSER

// this function is always available (even if !wxUSE_CMDLINE_PARSER) because it
// is used by wxWin itself under Windows
class WXDLLIMPEXP_BASE wxCmdLineParser
{
public:
    static std::vector<wxString>
    ConvertStringToArgs(const wxString& cmdline,
                        wxCmdLineSplitType type = wxCmdLineSplitType::DOS);
};

#endif // wxUSE_CMDLINE_PARSER/!wxUSE_CMDLINE_PARSER

#endif // _WX_CMDLINE_H_
