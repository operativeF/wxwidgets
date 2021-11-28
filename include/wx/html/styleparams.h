/////////////////////////////////////////////////////////////////////////////
// Name:        wx/html/styleparams.h
// Purpose:     wxHtml helper code for extracting style parameters
// Author:      Nigel Paton
// Copyright:   wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_HTML_STYLEPARAMS_H_
#define _WX_HTML_STYLEPARAMS_H_

#if wxUSE_HTML

import Utils.Strings;

class wxHtmlTag;

// This is a private class used by wxHTML to parse "style" attributes of HTML
// elements. Currently both parsing and support for the parsed values is pretty
// trivial.
class wxHtmlStyleParams
{
public:
    // Construct a style parameters object corresponding to the style attribute
    // of the given HTML tag.
    wxHtmlStyleParams(const wxHtmlTag& tag);

    // Check whether the named parameter is present or not.
    bool HasParam(const wxString& par) const
    {
        const auto parUpper = wx::utils::ToUpperCopy(par);
        auto paramMatch = std::find_if(m_names.begin(), m_names.end(), [parUpper](const auto& name){ return parUpper == wx::utils::ToUpperCopy(name); });

        return paramMatch != m_names.end();
    }

    // Get the value of the named parameter, return empty string if none.
    wxString GetParam(const wxString& par) const
    {
        const auto parUpper = wx::utils::ToUpperCopy(par);
        auto paramMatch = std::find_if(m_names.begin(), m_names.end(), [parUpper](const auto& name){ return parUpper == wx::utils::ToUpperCopy(name); });

        return paramMatch == m_names.end() ? wxString() : *paramMatch;
    }

private:
    // Arrays if names and values of the parameters
    std::vector<wxString> m_names;
    std::vector<wxString> m_values;
};

#endif // wxUSE_HTML

#endif // _WX_HTML_STYLEPARAMS_H_
