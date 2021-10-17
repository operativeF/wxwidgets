/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/msgout.cpp
// Purpose:     wxMessageOutput implementation
// Author:      Mattia Barbon
// Modified by:
// Created:     17.07.02
// Copyright:   (c) the wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////


// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#ifndef WX_PRECOMP
    #if defined(__WINDOWS__)
        #include "wx/msw/private.h"
    #endif
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
#include "wx/stringutils.h"

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
void wxMessageOutput::DoPrintfWchar(const wxChar *format, ...)
{
    va_list args;
    va_start(args, format);
    wxString out;

    out.PrintfV(format, args);
    va_end(args);

    Output(out);
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

void wxMessageOutputBest::Output(const std::string& str)
{
#ifdef __WINDOWS__
    // decide whether to use console output or not
    wxAppTraits * const traits = wxApp::GetTraitsIfExists();
    const bool hasStderr = traits ? traits->CanUseStderr() : false;

    if ( !(m_flags == wxMessageOutputFlags::MsgBox) )
    {
        if ( hasStderr && traits->WriteToStderr(AppendLineFeedIfNeeded(str)) )
            return;
    }

    std::string title;
    if ( wxTheApp )
        title = wxTheApp->GetAppDisplayName();
    else // Use some title to avoid default "Error"
        title = _("Message").ToStdString();

    ::MessageBoxW(nullptr, boost::nowide::widen(str).c_str(), boost::nowide::widen(title).c_str(), MB_ICONINFORMATION | MB_OK);
#else // !__WINDOWS__
    wxUnusedVar(m_flags);

    // TODO: use the native message box for the other ports too
    wxMessageOutputStderr::Output(str);
#endif // __WINDOWS__/!__WINDOWS__
}

// ----------------------------------------------------------------------------
// wxMessageOutputWithConv
// ----------------------------------------------------------------------------

wxString wxMessageOutputWithConv::AppendLineFeedIfNeeded(const wxString& str)
{
    wxString strLF(str);
    if ( strLF.empty() || *strLF.rbegin() != '\n' )
        strLF += '\n';

    return strLF;
}

wxCharBuffer wxMessageOutputWithConv::PrepareForOutput(const wxString& str)
{
    wxString strWithLF = AppendLineFeedIfNeeded(str);

#if defined(__WINDOWS__)
    // Determine whether the encoding is UTF-16. In that case, the file
    // should have been opened in "wb" mode, and EOL conversion must be done
    // here as it won't be done at stdio level.
    if ( m_conv->GetMBNulLen() == 2 )
    {
        strWithLF.Replace("\n", "\r\n");
    }
#endif // __WINDOWS__

    return m_conv->cWX2MB(strWithLF.c_str());
}

// ----------------------------------------------------------------------------
// wxMessageOutputStderr
// ----------------------------------------------------------------------------

wxMessageOutputStderr::wxMessageOutputStderr(FILE *fp, const wxMBConv& conv)
                     : wxMessageOutputWithConv(conv),
                       m_fp(fp)
{
}

void wxMessageOutputStderr::Output(const std::string& str)
{
    const wxCharBuffer& buf = PrepareForOutput(str);
    fwrite(buf, buf.length(), 1, m_fp);
    fflush(m_fp);
}

// ----------------------------------------------------------------------------
// wxMessageOutputDebug
// ----------------------------------------------------------------------------

void wxMessageOutputDebug::Output(const std::string& str)
{
#if defined(__WINDOWS__)
    std::string out = AppendLineFeedIfNeeded(str);

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

void wxMessageOutputLog::Output(const std::string& str)
{
    std::string out = str;

    wx::utils::ReplaceAll(out, "\t", "        ");

    wxLogMessage("%s", out.c_str());
}

#endif // wxUSE_BASE

// ----------------------------------------------------------------------------
// wxMessageOutputMessageBox
// ----------------------------------------------------------------------------

#if wxUSE_GUI && wxUSE_MSGDLG

void wxMessageOutputMessageBox::Output(const std::string& str)
{
    std::string out = str;

    // the native MSW msg box understands the TABs, others don't
#ifndef __WINDOWS__
    out.Replace(wxT("\t"), wxT("        "));
#endif

    std::string title = "wxWidgets";
    if (wxTheApp) title = wxTheApp->GetAppDisplayName();

    wxMessageBox(out, title);
}

#endif // wxUSE_GUI
