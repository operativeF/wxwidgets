/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/helpbest.cpp
// Purpose:     Tries to load MS HTML Help, falls back to wxHTML upon failure
// Author:      Mattia Barbon
// Modified by:
// Created:     02/04/2001
// Copyright:   (c) Mattia Barbon
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////




#include "wx/log.h"

#include "wx/filename.h"

#if wxUSE_HELP && wxUSE_MS_HTML_HELP \
    && wxUSE_WXHTML_HELP && !defined(__WXUNIVERSAL__)

#include "wx/msw/helpchm.h"
#include "wx/html/helpctrl.h"
#include "wx/msw/helpbest.h"

bool wxBestHelpController::Initialize( const std::string& filename )
{
    // try wxCHMHelpController
    auto chm = std::make_unique<wxCHMHelpController>(m_parentWindow);

    m_helpControllerType = HelpControllerType::Chm;
    // do not warn upon failure
    wxLogNull dontWarnOnFailure;

    if( chm->Initialize( GetValidFilename( filename ) ) )
    {
        m_helpController = std::move(chm);
        m_parentWindow = nullptr;
        return true;
    }

    // try wxHtmlHelpController
    auto html = std::make_unique<wxHtmlHelpController>(m_style, m_parentWindow);

    m_helpControllerType = HelpControllerType::Html;
    if( html->Initialize( GetValidFilename( filename ) ) )
    {
        m_helpController = std::move(html);
        m_parentWindow = nullptr;
        return true;
    }

    return false;
}

std::string wxBestHelpController::GetValidFilename( const std::string& filename ) const
{
    wxFileName fn{filename};

    switch( m_helpControllerType )
    {
        case HelpControllerType::Chm:
            fn.SetExt("chm");
            if( fn.FileExists() )
                return fn.GetFullPath();

            return filename;

        case HelpControllerType::Html:
            fn.SetExt("htb");
            if( fn.FileExists() )
                return fn.GetFullPath();

            fn.SetExt("zip");
            if( fn.FileExists() )
                return fn.GetFullPath();

            fn.SetExt("hhp");
            if( fn.FileExists() )
                return fn.GetFullPath();

            return filename;

        default:
            // we CAN'T get here
            wxFAIL_MSG( wxT("wxBestHelpController: Must call Initialize, first!") );
    }

    return {};
}

#endif
    // wxUSE_HELP && wxUSE_MS_HTML_HELP && wxUSE_WXHTML_HELP
