///////////////////////////////////////////////////////////////////////////////
// Name:        wx/debugrpt.h
// Purpose:     declaration of wxDebugReport class
// Author:      Vadim Zeitlin
// Created:     2005-01-17
// Copyright:   (c) 2005 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DEBUGRPT_H_
#define _WX_DEBUGRPT_H_

#if wxUSE_DEBUGREPORT && wxUSE_XML

#include "wx/debug.h"

#include "wx/string.h"

import WX.File.Filename;

import <vector>;

class wxXmlNode;

// ----------------------------------------------------------------------------
// wxDebugReport: generate a debug report, processing is done in derived class
// ----------------------------------------------------------------------------

class wxDebugReport
{
    friend class wxDebugReportDialog;

public:
    // this is used for the functions which may report either the current state
    // or the state during the last (fatal) exception
    enum class Context { Current, Exception };


    // ctor creates a temporary directory where we create the files which will
    // be included in the report, use IsOk() to check for errors
    wxDebugReport();

    // dtor normally destroys the temporary directory created in the ctor (with
    // all the files it contains), call Reset() to prevent this from happening
    virtual ~wxDebugReport();

    // return the name of the directory used for this report
    const wxString& GetDirectory() const { return m_dir; }

    // return true if the object was successfully initialized
    bool IsOk() const { return !GetDirectory().empty(); }

    // reset the directory name we use, the object can't be used any more after
    // this as it becomes invalid/uninitialized
    void Reset() { m_dir.clear(); }


    // add another file to the report: the file must already exist, its name
    // can be either absolute in which case it is copied to the debug report
    // directory or relative to GetDirectory()
    //
    // description is shown to the user in the report summary
    virtual void AddFile(const wxString& filename, const wxString& description);

    // convenience function: write the given text to a file with the given name
    // and then add it to the report (the difference with AddFile() is that the
    // file will be created by this function and doesn't have to already exist)
    bool AddText(const wxString& filename,
                 const wxString& text,
                 const wxString& description);

#if wxUSE_STACKWALKER
    // add an XML file containing the current or exception context and the
    // stack trace
    bool AddCurrentContext() { return AddContext(Context::Current); }
    bool AddExceptionContext() { return AddContext(Context::Exception); }
    virtual bool AddContext(Context ctx);
#endif

#if wxUSE_CRASHREPORT
    // add a file with crash report
    bool AddCurrentDump() { return AddDump(Context::Current); }
    bool AddExceptionDump() { return AddDump(Context::Exception); }
    virtual bool AddDump(Context ctx);
#endif // wxUSE_CRASHREPORT

    // add all available information to the report
    void AddAll(Context context = Context::Exception);


    // process this report: the base class simply notifies the user that the
    // report has been generated, this is usually not enough -- instead you
    // should override this method to do something more useful to you
    bool Process();

    // get the name used as base name for various files, by default
    // wxApp::GetName()
    virtual wxString GetReportName() const;

    // get the files in this report
    size_t GetFilesCount() const { return m_files.size(); }
    bool GetFile(size_t n, wxString *name, wxString *desc) const;

    // remove the file from report: this is used by wxDebugReportPreview to
    // allow the user to remove files potentially containing private
    // information from the report
    void RemoveFile(const wxString& name);

protected:
#if wxUSE_STACKWALKER
    // used by AddContext()
    virtual bool DoAddSystemInfo(wxXmlNode *nodeSystemInfo);
    virtual bool DoAddLoadedModules(wxXmlNode *nodeModules);
    virtual bool DoAddExceptionInfo(wxXmlNode *nodeContext);
    virtual void DoAddCustomContext([[maybe_unused]] wxXmlNode * nodeRoot) { }
#endif

    // used by Process()
    virtual bool DoProcess();

    // return the location where the report will be saved
    virtual wxFileName GetSaveLocation() const;

private:
    // name of the report directory
    wxString m_dir;

    // the arrays of files in this report and their descriptions
    std::vector<wxString> m_files;
    std::vector<wxString> m_descriptions;
};

#if wxUSE_ZIPSTREAM

// ----------------------------------------------------------------------------
// wxDebugReportCompress: compress all files of this debug report in a .ZIP
// ----------------------------------------------------------------------------

class wxDebugReportCompress : public wxDebugReport
{
public:
    // you can optionally specify the directory and/or name of the file where
    // the debug report should be generated, a default location under the
    // directory containing temporary files will be used if you don't
    //
    // both of these functions should be called before Process()ing the report
    // if they're called at all
    void SetCompressedFileDirectory(const wxString& dir);
    void SetCompressedFileBaseName(const wxString& name);

    // returns the full path of the compressed file (empty if creation failed)
    const wxString& GetCompressedFileName() const { return m_zipfile; }

protected:
    bool DoProcess() override;

    // return the location where the report will be saved
    wxFileName GetSaveLocation() const override;

private:
    // user-specified file directory/base name, use defaults if empty
    wxString m_zipDir,
             m_zipName;

    // full path to the ZIP file we created
    wxString m_zipfile;
};

// ----------------------------------------------------------------------------
// wxDebugReportUploader: uploads compressed file using HTTP POST request
// ----------------------------------------------------------------------------

class wxDebugReportUpload : public wxDebugReportCompress
{
public:
    // this class will upload the compressed file created by its base class to
    // an HTML multipart/form-data form at the specified address
    //
    // the URL is the base address, input is the name of the "type=file"
    // control on the form used for the file name and action is the value of
    // the form action field
    wxDebugReportUpload(const wxString& url,
                        const wxString& input,
                        const wxString& action,
                        const wxString& curl = "curl");

protected:
    bool DoProcess() override;

    // this function may be overridden in a derived class to show the output
    // from curl: this may be an HTML page or anything else that the server
    // returned
    //
    // return value becomes the return value of Process()
    virtual bool OnServerReply([[maybe_unused]] const std::vector<wxString>& reply)
    {
        return true;
    }

private:
    // the full URL to use with HTTP POST request
    wxString m_uploadURL;

    // the name of the input field containing the file name in the form at
    // above URL
    wxString m_inputField;

    // the curl command (by default it is just "curl" but could be full path to
    // curl or a wrapper script with curl-compatible syntax)
    wxString m_curlCmd;
};

#endif // wxUSE_ZIPSTREAM


// ----------------------------------------------------------------------------
// wxDebugReportPreview: presents the debug report to the user and allows him
//                       to veto report entirely or remove some parts of it
// ----------------------------------------------------------------------------

class wxDebugReportPreview
{
public:
    // present the report to the user and allow him to modify it by removing
    // some or all of the files and, potentially, adding some notes
    //
    // return true if the report should be processed or false if the user chose
    // to cancel report generation or removed all files from it
    virtual bool Show(wxDebugReport& dbgrpt) const = 0;

    // dtor is trivial as well but should be virtual for a base class
    virtual ~wxDebugReportPreview() = default;
};

#if wxUSE_GUI

// ----------------------------------------------------------------------------
// wxDebugReportPreviewStd: standard debug report preview window
// ----------------------------------------------------------------------------

class wxDebugReportPreviewStd : public wxDebugReportPreview
{
public:
    bool Show(wxDebugReport& dbgrpt) const override;
};

#endif // wxUSE_GUI

#endif // wxUSE_DEBUGREPORT && wxUSE_XML

#endif // _WX_DEBUGRPT_H_
