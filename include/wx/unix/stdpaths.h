///////////////////////////////////////////////////////////////////////////////
// Name:        wx/unix/stdpaths.h
// Purpose:     wxStandardPaths for Unix systems
// Author:      Vadim Zeitlin
// Modified by:
// Created:     2004-10-19
// Copyright:   (c) 2004 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_UNIX_STDPATHS_H_
#define _WX_UNIX_STDPATHS_H_

// ----------------------------------------------------------------------------
// wxStandardPaths
// ----------------------------------------------------------------------------

class wxStandardPaths : public wxStandardPathsBase
{
public:
    // tries to determine the installation prefix automatically (Linux only right
    // now) and returns /usr/local if it failed
    void DetectPrefix();

    // set the program installation directory which is /usr/local by default
    //
    // under some systems (currently only Linux) the program directory can be
    // determined automatically but for portable programs you should always set
    // it explicitly
    void SetInstallPrefix(const wxString& prefix);

    // get the program installation prefix
    //
    // if the prefix had been previously by SetInstallPrefix, returns that
    // value, otherwise calls DetectPrefix()
    wxString GetInstallPrefix() const;


    
    wxString GetExecutablePath() const override;
    wxString GetConfigDir() const override;
    wxString GetUserConfigDir() const override;
    wxString GetDataDir() const override;
    wxString GetLocalDataDir() const override;
    wxString GetUserDataDir() const override;
    wxString GetPluginsDir() const override;
    virtual wxString GetLocalizedResourcesDir(const wxString& lang,
                                              ResourceCat category) const override;
#ifndef __VMS
    wxString GetUserDir(Dir userDir) const override;
#endif
    virtual wxString MakeConfigFileName(const wxString& basename,
                                        ConfigFileConv conv = ConfigFileConv_Ext
                                        ) const override;

protected:
    // Ctor is protected, use wxStandardPaths::Get() instead of instantiating
    // objects of this class directly.
    wxStandardPaths() { }

private:
    wxString m_prefix;
};

#endif // _WX_UNIX_STDPATHS_H_

