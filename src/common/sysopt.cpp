/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/sysopt.cpp
// Purpose:     wxSystemOptions
// Author:      Julian Smart
// Modified by:
// Created:     2001-07-10
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_SYSTEM_OPTIONS

#include "wx/sysopt.h"
#include "wx/app.h"
#include "wx/string.h"

import Utils.Strings;

// ----------------------------------------------------------------------------
// private globals
// ----------------------------------------------------------------------------

static std::vector<wxString> gs_optionNames;
static std::vector<wxString> gs_optionValues;

// ============================================================================
// wxSystemOptions implementation
// ============================================================================

// Option functions (arbitrary name/value mapping)
void wxSystemOptions::SetOption(const wxString& name, const wxString& value)
{
    const auto& possible_name = std::find_if(gs_optionNames.cbegin(), gs_optionNames.cend(),
        [name](const auto& optName)
        {
            return name.IsSameAs(optName, false);
        });
    if (possible_name == gs_optionNames.cend())
    {
        gs_optionNames.push_back(name);
        gs_optionValues.push_back(value);
    }
    else
    {
        const auto idx = std::distance(std::cbegin(gs_optionNames), possible_name);
        gs_optionNames[idx] = name;
        gs_optionValues[idx] = value;
    }
}

void wxSystemOptions::SetOption(const wxString& name, int value)
{
    SetOption(name, wxString::Format("%d", value));
}

wxString wxSystemOptions::GetOption(std::string_view name)
{
    wxString val;

    const auto match_iter = std::find_if(gs_optionNames.cbegin(), gs_optionNames.cend(),
        [name](const auto& optName)
        {
            return wx::utils::IsSameAsNoCase(name, optName.ToStdString());
        });
    if ( match_iter != std::cend(gs_optionNames) )
    {
        val = *match_iter;
    }
    else // not set explicitly
    {
        // look in the environment: first for a variable named "wx_appname_name"
        // which can be set to affect the behaviour or just this application
        // and then for "wx_name" which can be set to change the option globally
        std::string var = {name.begin(), name.end()};

        wx::utils::ReplaceAll(var, ".", "_"); // '.'s not allowed in env var names
        wx::utils::ReplaceAll(var, "-", "_"); // and neither are '-'s

        wxString appname;
        if ( wxTheApp )
            appname = wxTheApp->GetAppName();

        if ( !appname.empty() )
            val = wxGetenv("wx_" + appname + wxT('_') + var);

        if ( val.empty() )
            val = wxGetenv("wx_" + var);
    }

    return val;
}

int wxSystemOptions::GetOptionInt(std::string_view name)
{
    return wxAtoi (GetOption(name));
}

bool wxSystemOptions::HasOption(std::string_view name)
{
    return !GetOption(name).empty();
}

#endif // wxUSE_SYSTEM_OPTIONS
