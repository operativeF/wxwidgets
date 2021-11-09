/////////////////////////////////////////////////////////////////////////////
// Name:        src/html/styleparams.cpp
// Purpose:     wxHtml helper code for extracting style parameters
// Author:      Nigel Paton
// Copyright:   wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_HTML

#include "wx/tokenzr.h"
#include "wx/html/htmltag.h"
#include "wx/html/styleparams.h"

wxHtmlStyleParams::wxHtmlStyleParams(const wxHtmlTag& tag)
{
    wxString wd = tag.GetParam("STYLE");

    // Make sure no whitespace
    wd.Trim(true).Trim(false);
    if ( wd.empty() )
        return;

    // Check for bracketed entries
    // Only support element properties and not pseudo-element or pseudo-classes
    if (wd.Find('{') == 0)
    {
        // Extract string up to end bracket
        int endBracket = wd.Find('}');
        if (endBracket != wxNOT_FOUND)
        {
            // Replace original string with bracketed options
            wd = wd.SubString(1, endBracket - 1);
            // Make sure no whitespace
            wd.Trim(true).Trim(false);
        }
        else
        {
            // Syntax problem change to blank string
            wd.clear();
        }
    }

    // Should now have a semi-colon delimited list of options
    // Each option is a name and a value separated by a colon
    // Split the list into names and values
    wxStringTokenizer tkz(wd, ";", wxStringTokenizerMode::StrTok);
    while ( tkz.HasMoreTokens() )
    {
        wxString token = tkz.GetNextToken();
        // Split into name and value
        int colonIndex = token.Find(':');
        if ((colonIndex != wxNOT_FOUND) && // Not a name value pair
            (colonIndex != 0))             // No name
        {
            wxString tempString;
            // Extract and trim name
            tempString = token.SubString(0, colonIndex - 1);
            tempString.Trim(true).Trim(false);
            // Add to name list
            m_names.push_back(tempString);

            // Extract and trim values
            tempString = token.SubString(colonIndex + 1, token.Length() - 1);
            tempString.Trim(true).Trim(false);
            // Add to values list
            m_values.push_back(tempString);
        }
    }
}

#endif // wxUSE_HTML
