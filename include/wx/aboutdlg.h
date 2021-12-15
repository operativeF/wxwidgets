///////////////////////////////////////////////////////////////////////////////
// Name:        wx/aboutdlg.h
// Purpose:     declaration of wxAboutDialog class
// Author:      Vadim Zeitlin
// Created:     2006-10-07
// Copyright:   (c) 2006 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_ABOUTDLG_H_
#define _WX_ABOUTDLG_H_

#if wxUSE_ABOUTDLG

#include "wx/app.h"
#include "wx/icon.h"

import <string>;
import <vector>;

// ----------------------------------------------------------------------------
// wxAboutDialogInfo: information shown by the standard "About" dialog
// ----------------------------------------------------------------------------

class wxAboutDialogInfo
{
public:
    // all fields are initially uninitialized

    // accessors for various simply fields
    // -----------------------------------

    // name of the program, if not used defaults to wxApp::GetAppDisplayName()
    void SetName(std::string name) { m_name = name; }
    std::string GetName() const
        { return m_name.empty() ? wxTheApp->GetAppDisplayName() : m_name; }

    // version should contain program version without "version" word (e.g.,
    // "1.2" or "RC2") while longVersion may contain the full version including
    // "version" word (e.g., "Version 1.2" or "Release Candidate 2")
    //
    // if longVersion is empty, it is automatically constructed from version
    //
    // generic and gtk native: use short version only, as a suffix to the
    // program name msw and osx native: use long version
    void SetVersion(const std::string& version,
                    const std::string& longVersion = {});

    bool HasVersion() const { return !m_version.empty(); }
    const std::string& GetVersion() const { return m_version; }
    const std::string& GetLongVersion() const { return m_longVersion; }

    // brief, but possibly multiline, description of the program
    void SetDescription(const std::string& desc) { m_description = desc; }
    bool HasDescription() const { return !m_description.empty(); }
    const std::string& GetDescription() const { return m_description; }

    // short string containing the program copyright information
    void SetCopyright(const std::string& copyright) { m_copyright = copyright; }
    bool HasCopyright() const { return !m_copyright.empty(); }
    const std::string& GetCopyright() const { return m_copyright; }

    // long, multiline string containing the text of the program licence
    void SetLicence(const std::string& licence) { m_licence = licence; }
    void SetLicense(const std::string& licence) { m_licence = licence; }
    bool HasLicence() const { return !m_licence.empty(); }
    const std::string& GetLicence() const { return m_licence; }

    // icon to be shown in the dialog, defaults to the main frame icon
    void SetIcon(const wxIcon& icon) { m_icon = icon; }
    bool HasIcon() const { return m_icon.IsOk(); }
    wxIcon GetIcon() const;

    // web site for the program and its description (defaults to URL itself if
    // empty)
    void SetWebSite(const std::string& url, const std::string& desc = {})
    {
        m_url = url;
        m_urlDesc = desc.empty() ? url : desc;
    }

    bool HasWebSite() const { return !m_url.empty(); }

    const std::string& GetWebSiteURL() const { return m_url; }
    const std::string& GetWebSiteDescription() const { return m_urlDesc; }

    // accessors for the arrays
    // ------------------------

    // the list of developers of the program
    void SetDevelopers(const std::vector<std::string>& developers)
        { m_developers = developers; }
    void AddDeveloper(const std::string& developer)
        { m_developers.push_back(developer); }

    bool HasDevelopers() const { return !m_developers.empty(); }
    const std::vector<std::string>& GetDevelopers() const { return m_developers; }

    // the list of documentation writers
    void SetDocWriters(const std::vector<std::string>& docwriters)
        { m_docwriters = docwriters; }
    void AddDocWriter(const std::string& docwriter)
        { m_docwriters.push_back(docwriter); }

    bool HasDocWriters() const { return !m_docwriters.empty(); }
    const std::vector<std::string>& GetDocWriters() const { return m_docwriters; }

    // the list of artists for the program art
    void SetArtists(const std::vector<std::string>& artists)
        { m_artists = artists; }
    void AddArtist(const std::string& artist)
        { m_artists.push_back(artist); }

    bool HasArtists() const { return !m_artists.empty(); }
    const std::vector<std::string>& GetArtists() const { return m_artists; }

    // the list of translators
    void SetTranslators(const std::vector<std::string>& translators)
        { m_translators = translators; }
    void AddTranslator(const std::string& translator)
        { m_translators.push_back(translator); }

    bool HasTranslators() const { return !m_translators.empty(); }
    const std::vector<std::string>& GetTranslators() const { return m_translators; }


    // implementation only
    // -------------------

    // "simple" about dialog shows only textual information (with possibly
    // default icon but without hyperlink nor any long texts such as the
    // licence text)
    bool IsSimple() const
        { return !HasWebSite() && !HasIcon() && !HasLicence(); }

    // get the description and credits (i.e. all of developers, doc writers,
    // artists and translators) as a one long multiline string
    std::string GetDescriptionAndCredits() const;

    // returns the copyright with the (C) string substituted by the Unicode
    // character U+00A9
    std::string GetCopyrightToDisplay() const;

private:
    std::vector<std::string> m_developers;
    std::vector<std::string> m_docwriters;
    std::vector<std::string> m_artists;
    std::vector<std::string> m_translators;

    std::string m_name;
    std::string m_version;
    std::string m_longVersion;
    std::string m_description;
    std::string m_copyright;
    std::string m_licence;

    std::string m_url;
    std::string m_urlDesc;

    wxIcon m_icon;
};

// functions to show the about dialog box
void wxAboutBox(const wxAboutDialogInfo& info, wxWindow* parent = nullptr);

#endif // wxUSE_ABOUTDLG

#endif // _WX_ABOUTDLG_H_

