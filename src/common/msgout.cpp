/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/msgout.cpp
// Purpose:     wxMessageOutput implementation
// Author:      Mattia Barbon
// Modified by:
// Created:     17.07.02
// Copyright:   (c) the wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifdef WX_WINDOWS
    #include "wx/msw/private.h"
#endif

#include "wx/string.h"
#include "wx/ffile.h"
#include "wx/app.h"
#include "wx/intl.h"
#include "wx/log.h"
#if wxUSE_GUI
    #include "wx/msgdlg.h"
#endif // wxUSE_GUI

#include "wx/apptrait.h"
#include "wx/msgout.h"

#include <boost/nowide/convert.hpp>

import Utils.Strings;

// ===========================================================================
// implementation
// ===========================================================================

#if wxUSE_BASE

// ----------------------------------------------------------------------------
// wxMessageOutput
// ----------------------------------------------------------------------------

wxMessageOutput* wxMessageOutput::ms_msgOut = nullptr;

wxMessageOutput* wxMessageOutput::Get()
{
    if ( !ms_msgOut && wxTheApp )
    {
        ms_msgOut = wxTheApp->GetTraits()->CreateMessageOutput();
    }

    return ms_msgOut;
}

wxMessageOutput* wxMessageOutput::Set(wxMessageOutput* msgout)
{
    wxMessageOutput* old = ms_msgOut;
    ms_msgOut = msgout;
    return old;
}

#if !wxUSE_UTF8_LOCALE_ONLY
void wxMessageOutput::DoPrintfWchar(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    wxString out;

    out.PrintfV(format, args);
    va_end(args);

    Output(out.ToStdString());
}
#endif // !wxUSE_UTF8_LOCALE_ONLY

#if wxUSE_UNICODE_UTF8
void wxMessageOutput::DoPrintfUtf8(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    wxString out;

    out.PrintfV(format, args);
    va_end(args);

    Output(out);
}
#endif // wxUSE_UNICODE_UTF8

// ----------------------------------------------------------------------------
// wxMessageOutputBest
// ----------------------------------------------------------------------------

void wxMessageOutputBest::Output(std::string_view str)
{
#ifdef WX_WINDOWS
    // decide whether to use console output or not
    wxAppTraits * const traits = wxApp::GetTraitsIfExists();
    const bool hasStderr = traits ? traits->CanUseStderr() : false;

    if ( !(m_flags == wxMessageOutputFlags::MsgBox) )
    {
        if ( hasStderr && traits->WriteToStderr(AppendLineFeedIfNeeded(std::string{str.begin(), str.end()})) )
            return;
    }

    std::string title;
    if ( wxTheApp )
        title = wxTheApp->GetAppDisplayName();
    else // Use some title to avoid default "Error"
        title = _("Message");

    ::MessageBoxW(nullptr, boost::nowide::widen(str).c_str(), boost::nowide::widen(title).c_str(), MB_ICONINFORMATION | MB_OK);
#else // !WX_WINDOWS
    wxUnusedVar(m_flags);

    // TODO: use the native message box for the other ports too
    wxMessageOutputStderr::Output(str);
#endif // WX_WINDOWS/!WX_WINDOWS
}

// ----------------------------------------------------------------------------
// wxMessageOutputWithConv
// ----------------------------------------------------------------------------

std::string wxMessageOutputWithConv::AppendLineFeedIfNeeded(const std::string& str)
{
    std::string strLF{str};
    if ( strLF.empty() || *strLF.rbegin() != '\n' )
        strLF += '\n';

    return strLF;
}

std::string wxMessageOutputWithConv::PrepareForOutput(std::string_view str)
{
    std::string strWithLF = AppendLineFeedIfNeeded(std::string{str.begin(), str.end()});

#if defined(WX_WINDOWS)
    // Determine whether the encoding is UTF-16. In that case, the file
    // should have been opened in "wb" mode, and EOL conversion must be done
    // here as it won't be done at stdio level.
    if ( m_conv->GetMBNulLen() == 2 )
    {
        wx::utils::ReplaceAll(strWithLF, "\n", "\r\n");
    }
#endif // WX_WINDOWS

    return strWithLF;
}

// ----------------------------------------------------------------------------
// wxMessageOutputStderr
// ----------------------------------------------------------------------------

wxMessageOutputStderr::wxMessageOutputStderr(FILE *fp, const wxMBConv& conv)
                     : wxMessageOutputWithConv(conv),
                       m_fp(fp)
{
}

void wxMessageOutputStderr::Output(std::string_view str)
{
    const auto buf = PrepareForOutput(str);
    fwrite(buf.data(), buf.length(), 1, m_fp);
    fflush(m_fp);
}

// ----------------------------------------------------------------------------
// wxMessageOutputDebug
// ----------------------------------------------------------------------------

void wxMessageOutputDebug::Output(std::string_view str)
{
#if defined(WX_WINDOWS)
    std::string out = AppendLineFeedIfNeeded(std::string{str.begin(), str.end()});

    wx::utils::ReplaceAll(out, "\t", "        ");
    wx::utils::ReplaceAll(out, "\n", "\r\n");

    ::OutputDebugStringW(boost::nowide::widen(out).c_str());
#else
    // TODO: use native debug output function for the other ports too
    wxMessageOutputStderr::Output(str);
#endif // platform
}

// ----------------------------------------------------------------------------
// wxMessageOutputLog
// ----------------------------------------------------------------------------

void wxMessageOutputLog::Output(std::string_view str)
{
    std::string out = {str.begin(), str.end()};

    wx::utils::ReplaceAll(out, "\t", "        ");

    wxLogMessage("%s", out.c_str());
}

#endif // wxUSE_BASE

// ----------------------------------------------------------------------------
// wxMessageOutputMessageBox
// ----------------------------------------------------------------------------

#if wxUSE_GUI && wxUSE_MSGDLG

void wxMessageOutputMessageBox::Output(std::string_view str)
{
    std::string out = {str.begin(), str.end()};

    // the native MSW msg box understands the TABs, others don't
#ifndef WX_WINDOWS
    out.Replace("\t"), wxT("        ");
#endif

    std::string title = "wxWidgets";
    if (wxTheApp) title = wxTheApp->GetAppDisplayName();

    wxMessageBox(out, title);
}

#endif // wxUSE_GUI
