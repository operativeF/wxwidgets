///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msgout.h
// Purpose:     wxMessageOutput class. Shows a message to the user
// Author:      Mattia Barbon
// Modified by:
// Created:     17.07.02
// Copyright:   (c) Mattia Barbon
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSGOUT_H_
#define _WX_MSGOUT_H_

#include "wx/chartype.h"
#include "wx/strvararg.h"

// ----------------------------------------------------------------------------
// wxMessageOutput is a class abstracting formatted output target, i.e.
// something you can printf() to
// ----------------------------------------------------------------------------

class wxMessageOutput
{
public:
    virtual ~wxMessageOutput() = default;

    // gets the current wxMessageOutput object (may be NULL during
    // initialization or shutdown)
    static wxMessageOutput* Get();

    // sets the global wxMessageOutput instance; returns the previous one
    static wxMessageOutput* Set(wxMessageOutput* msgout);

    // show a message to the user
    // void Printf(const wxString& format, ...) = 0;
    WX_DEFINE_VARARG_FUNC_VOID(Printf, 1, (const wxFormatString&),
                               DoPrintfWchar, DoPrintfUtf8)

    // called by DoPrintf() to output formatted string but can also be called
    // directly if no formatting is needed
    virtual void Output(const std::string& str) = 0;

protected:
#if !wxUSE_UTF8_LOCALE_ONLY
    void DoPrintfWchar(const wxChar *format, ...);
#endif
#if wxUSE_UNICODE_UTF8
    void DoPrintfUtf8(const char *format, ...);
#endif

private:
    static wxMessageOutput* ms_msgOut;
};

// ----------------------------------------------------------------------------
// helper mix-in for output targets that can use difference encodings
// ----------------------------------------------------------------------------

class wxMessageOutputWithConv
{
protected:
    explicit wxMessageOutputWithConv(const wxMBConv& conv)
        : m_conv(conv.Clone())
    {
    }

    ~wxMessageOutputWithConv()
    {
        delete m_conv;
    }

    // return the string with "\n" appended if it doesn't already terminate
    // with it (in which case it's returned unchanged)
    wxString AppendLineFeedIfNeeded(const wxString& str);

    // Prepare the given string for output by appending a new line to it, if
    // necessary, and converting it to a narrow string using our conversion
    // object.
    wxCharBuffer PrepareForOutput(const wxString& str);

    const wxMBConv* const m_conv;
};

// ----------------------------------------------------------------------------
// implementation which sends output to stderr or specified file
// ----------------------------------------------------------------------------

class wxMessageOutputStderr : public wxMessageOutput,
                                               protected wxMessageOutputWithConv
{
public:
    wxMessageOutputStderr(FILE *fp = stderr,
                          const wxMBConv &conv = wxConvWhateverWorks);

    wxMessageOutputStderr& operator=(wxMessageOutputStderr&&) = delete;

    void Output(const std::string& str) override;

protected:
    FILE *m_fp;
};

// ----------------------------------------------------------------------------
// implementation showing the message to the user in "best" possible way:
// uses stderr or message box if available according to the flag given to ctor.
// ----------------------------------------------------------------------------

enum class wxMessageOutputFlags
{
    StdErr, // use stderr if available (this is the default)
    MsgBox  // always use message box if available
};

class wxMessageOutputBest : public wxMessageOutputStderr
{
public:
    wxMessageOutputBest() = default;
    wxMessageOutputBest(wxMessageOutputFlags flags)
        : m_flags(flags) { }

    void Output(const std::string& str) override;

private:
    wxMessageOutputFlags m_flags{wxMessageOutputFlags::StdErr};
};

// ----------------------------------------------------------------------------
// implementation which shows output in a message box
// ----------------------------------------------------------------------------

#if wxUSE_GUI && wxUSE_MSGDLG

class wxMessageOutputMessageBox : public wxMessageOutput
{
public:
    void Output(const std::string& str) override;
};

#endif // wxUSE_GUI && wxUSE_MSGDLG

// ----------------------------------------------------------------------------
// implementation using the native way of outputting debug messages
// ----------------------------------------------------------------------------

class wxMessageOutputDebug : public wxMessageOutputStderr
{
public:
    void Output(const std::string& str) override;
};

// ----------------------------------------------------------------------------
// implementation using wxLog (mainly for backwards compatibility)
// ----------------------------------------------------------------------------

class wxMessageOutputLog : public wxMessageOutput
{
public:
    void Output(const std::string& str) override;
};

#endif // _WX_MSGOUT_H_
