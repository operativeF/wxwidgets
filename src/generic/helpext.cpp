/////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/helpext.cpp
// Purpose:     an external help controller for wxWidgets
// Author:      Karsten Ballueder
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Karsten Ballueder
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_HELP

#ifdef __WXMSW__
#include <windows.h>
#endif

#include "wx/list.h"
#include "wx/string.h"
#include "wx/utils.h"
#include "wx/intl.h"
#include "wx/msgdlg.h"
#include "wx/choicdlg.h"
#include "wx/log.h"

#include "wx/filename.h"
#include "wx/textfile.h"
#include "wx/generic/helpext.h"

#include <sys/stat.h>

#if !defined(WX_WINDOWS)
    #include   <unistd.h>
#endif

#include <boost/nowide/convert.hpp>

#include <string>
#include <vector>

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// Name for map file.
constexpr char WXEXTHELP_MAPFILE[] = "wxhelp.map";

// Character introducing comments/documentation field in map file.
constexpr char WXEXTHELP_COMMENTCHAR = ';';

// The ID of the Contents section
constexpr int WXEXTHELP_CONTENTS_ID = 0;

// Name of environment variable to set help browser.
constexpr char WXEXTHELP_ENVVAR_BROWSER[] = "WX_HELPBROWSER";

// Is browser a netscape browser?
constexpr char WXEXTHELP_ENVVAR_BROWSERISNETSCAPE[] =  "WX_HELPBROWSER_NS";

wxExtHelpController::wxExtHelpController(wxWindow* parentWindow)
                   : wxHelpControllerBase(parentWindow)
{
    auto* browser = wxGetenv(boost::nowide::widen(WXEXTHELP_ENVVAR_BROWSER).c_str());
    if (browser)
    {
        m_BrowserName = boost::nowide::narrow(browser);
        browser = wxGetenv(boost::nowide::widen(WXEXTHELP_ENVVAR_BROWSERISNETSCAPE).c_str());
        m_BrowserIsNetscape = browser && (wxAtoi(browser) != 0);
    }
}

wxExtHelpController::~wxExtHelpController()
{
    DeleteList();
}

void wxExtHelpController::SetViewer(const std::string& viewer, unsigned int flags)
{
    m_BrowserName = viewer;
    m_BrowserIsNetscape = (flags & wxHELP_NETSCAPE) != 0;
}

bool wxExtHelpController::DisplayHelp(const std::string &relativeURL)
{
    // construct hte URL to open -- it's just a file
    wxString url("file://" + m_helpDir);
    url << wxFILE_SEP_PATH << relativeURL;

    // use the explicit browser program if specified
    if ( !m_BrowserName.empty() )
    {
        if ( m_BrowserIsNetscape )
        {
            wxString command;
            command << m_BrowserName
                    << " -remote openURL(" << url << wxT(')');
            if ( wxExecute(command, wxEXEC_SYNC) != -1 )
                return true;
        }

        if ( wxExecute(m_BrowserName + wxT(' ') + url, wxEXEC_SYNC) != -1 )
            return true;
    }
    //else: either no browser explicitly specified or we failed to open it

    // just use default browser
    return wxLaunchDefaultBrowser(url);
}

class wxExtHelpMapEntry : public wxObject
{
public:
    int      entryid;
    std::string url;
    std::string doc;

    wxExtHelpMapEntry(int iid, std::string const &iurl, std::string const &idoc)
        : entryid(iid), url(iurl), doc(idoc)
        { }
};

void wxExtHelpController::DeleteList()
{
    if (m_MapList)
    {
        wxList::compatibility_iterator node = m_MapList->GetFirst();
        while (node)
        {
            delete (wxExtHelpMapEntry *)node->GetData();
            m_MapList->Erase(node);
            node = m_MapList->GetFirst();
        }

        wxDELETE(m_MapList);
    }
}

// This must be called to tell the controller where to find the documentation.
//  @param file - NOT a filename, but a directory name.
//  @return true on success
bool wxExtHelpController::Initialize(const std::string& file)
{
    return LoadFile(file);
}

bool wxExtHelpController::ParseMapFileLine(const std::string& line)
{
    const char *p = line.c_str();

    // skip whitespace
    while ( isascii(*p) && wxIsspace(*p) )
        p++;

    // skip empty lines and comments
    if ( *p == wxT('\0') || *p == WXEXTHELP_COMMENTCHAR )
        return true;

    // the line is of the form "num url" so we must have an integer now
    char *end;
    const unsigned long id = wxStrtoul(p, &end, 0);

    if ( end == p )
        return false;

    p = end;
    while ( isascii(*p) && wxIsspace(*p) )
        p++;

    // next should be the URL
    wxString url;
    url.reserve(line.length());
    while ( isascii(*p) && !wxIsspace(*p) )
        url += *p++;

    while ( isascii(*p) && wxIsspace(*p) )
        p++;

    // and finally the optional description of the entry after comment
    wxString doc;
    if ( *p == WXEXTHELP_COMMENTCHAR )
    {
        p++;
        while ( isascii(*p) && wxIsspace(*p) )
            p++;
        doc = p;
    }

    m_MapList->Append(new wxExtHelpMapEntry(id, url, doc));
    m_NumOfEntries++;

    return true;
}

// file is a misnomer as it's the name of the base help directory
bool wxExtHelpController::LoadFile(const std::string& file)
{
    wxFileName helpDir(wxFileName::DirName(file));
    helpDir.MakeAbsolute();

    bool dirExists = false;

#if wxUSE_INTL
    // If a locale is set, look in file/localename, i.e. If passed
    // "/usr/local/myapp/help" and the current wxLocale is set to be "de", then
    // look in "/usr/local/myapp/help/de/" first and fall back to
    // "/usr/local/myapp/help" if that doesn't exist.
    const wxLocale * const loc = wxGetLocale();
    if ( loc )
    {
        wxString locName = loc->GetName();

        // the locale is in general of the form xx_YY.zzzz, try the full firm
        // first and then also more general ones
        wxFileName helpDirLoc(helpDir);
        helpDirLoc.AppendDir(locName);
        dirExists = helpDirLoc.DirExists();

        if ( ! dirExists )
        {
            // try without encoding
            const wxString locNameWithoutEncoding = locName.BeforeLast(wxT('.'));
            if ( !locNameWithoutEncoding.empty() )
            {
                helpDirLoc = helpDir;
                helpDirLoc.AppendDir(locNameWithoutEncoding);
                dirExists = helpDirLoc.DirExists();
            }
        }

        if ( !dirExists )
        {
            // try without country part
            wxString locNameWithoutCountry = locName.BeforeLast(wxT('_'));
            if ( !locNameWithoutCountry.empty() )
            {
                helpDirLoc = helpDir;
                helpDirLoc.AppendDir(locNameWithoutCountry);
                dirExists = helpDirLoc.DirExists();
            }
        }

        if ( dirExists )
            helpDir = helpDirLoc;
    }
#endif // wxUSE_INTL

    if ( ! dirExists && !helpDir.DirExists() )
    {
        wxLogError(_("Help directory \"%s\" not found."),
                   helpDir.GetFullPath().c_str());
        return false;
    }

    const wxFileName mapFile(helpDir.GetFullPath(), WXEXTHELP_MAPFILE);
    if ( ! mapFile.FileExists() )
    {
        wxLogError(_("Help file \"%s\" not found."),
                   mapFile.GetFullPath().c_str());
        return false;
    }

    DeleteList();
    m_MapList = new wxList;
    m_NumOfEntries = 0;

    wxTextFile input;
    if ( !input.Open(mapFile.GetFullPath()) )
        return false;

    for ( wxString& line = input.GetFirstLine();
          !input.Eof();
          line = input.GetNextLine() )
    {
        if ( !ParseMapFileLine(line) )
        {
            wxLogWarning(_("Line %lu of map file \"%s\" has invalid syntax, skipped."),
                         (unsigned long)input.GetCurrentLine(),
                         mapFile.GetFullPath().c_str());
        }
    }

    if ( !m_NumOfEntries )
    {
        wxLogError(_("No valid mappings found in the file \"%s\"."),
                   mapFile.GetFullPath().c_str());
        return false;
    }

    m_helpDir = helpDir.GetFullPath(); // now it's valid
    return true;
}


bool wxExtHelpController::DisplayContents()
{
    if (! m_NumOfEntries)
        return false;

    std::string contents;
    wxList::compatibility_iterator node = m_MapList->GetFirst();
    wxExtHelpMapEntry *entry;
    while (node)
    {
        entry = (wxExtHelpMapEntry *)node->GetData();
        if (entry->entryid == WXEXTHELP_CONTENTS_ID)
        {
            contents = entry->url;
            break;
        }

        node = node->GetNext();
    }

    bool rc = false;
    wxString file;
    file << m_helpDir << wxFILE_SEP_PATH << contents;
    if (file.Contains(wxT('#')))
        file = file.BeforeLast(wxT('#'));
    if ( wxFileExists(file) )
        rc = DisplaySection(WXEXTHELP_CONTENTS_ID);

    // if not found, open homemade toc:
    return rc ? true : KeywordSearch("");
}

bool wxExtHelpController::DisplaySection(int sectionNo)
{
    if (! m_NumOfEntries)
        return false;

    wxBusyCursor b; // display a busy cursor
    wxList::compatibility_iterator node = m_MapList->GetFirst();
    while (node)
    {
        wxExtHelpMapEntry* entry;
        entry = (wxExtHelpMapEntry *)node->GetData();
        if (entry->entryid == sectionNo)
            return DisplayHelp(entry->url);
        node = node->GetNext();
    }

    return false;
}

bool wxExtHelpController::DisplaySection(const std::string& section)
{
    bool isFilename = (section.find(".htm") != std::string::npos);

    if (isFilename)
        return DisplayHelp(section);
    else
        return KeywordSearch(section);
}

bool wxExtHelpController::DisplayBlock(long blockNo)
{
    return DisplaySection((int)blockNo);
}

bool wxExtHelpController::KeywordSearch(const std::string& k,
                                   wxHelpSearchMode WXUNUSED(mode))
{
   if (! m_NumOfEntries)
      return false;

   std::vector<std::string> choices(m_NumOfEntries);

   wxString *urls = new wxString[m_NumOfEntries];

   int          idx = 0;
   bool         rc = false;
   bool         showAll = k.empty();

   wxList::compatibility_iterator node = m_MapList->GetFirst();

   {
        // display a busy cursor
        wxBusyCursor b;
        wxString compA, compB;
        wxExtHelpMapEntry *entry;

        // we compare case insensitive
        if (! showAll)
        {
            compA = k;
            compA.LowerCase();
        }

        while (node)
        {
            entry = (wxExtHelpMapEntry *)node->GetData();
            compB = entry->doc;

            bool testTarget = ! compB.empty();
            if (testTarget && ! showAll)
            {
                compB.LowerCase();
                testTarget = compB.Contains(compA);
            }

            if (testTarget)
            {
                urls[idx] = entry->url;
                // doesn't work:
                // choices[idx] = (**i).doc.Contains((**i).doc.Before(WXEXTHELP_COMMENTCHAR));
                //if (choices[idx].empty()) // didn't contain the ';'
                //   choices[idx] = (**i).doc;
                choices[idx].clear();
                for (int j=0; ; j++)
                {
                    wxChar targetChar = entry->doc.c_str()[j];
                    if ((targetChar == 0) || (targetChar == WXEXTHELP_COMMENTCHAR))
                        break;

                    choices[idx] = targetChar;
                }

                idx++;
            }

            node = node->GetNext();
        }
    }

    switch (idx)
    {
    case 0:
        wxMessageBox(_("No entries found."));
        break;

    case 1:
        rc = DisplayHelp(urls[0]);
        break;

    default:
        if (showAll)
            idx = wxGetSingleChoiceIndex("Help Index",
                                         "Help Index",
                                         choices);
        else
            idx = wxGetSingleChoiceIndex("Relevant entries:",
                                         "Entries found",
                                         choices);

        if (idx >= 0)
            rc = DisplayHelp(urls[idx]);
        break;
    }

    delete [] urls;

    return rc;
}


bool wxExtHelpController::Quit()
{
   return true;
}

void wxExtHelpController::OnQuit()
{
}

#endif // wxUSE_HELP
