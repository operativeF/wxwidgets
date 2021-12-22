///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/debugrpt.cpp
// Purpose:     wxDebugReport and related classes implementation
// Author:      Vadim Zeitlin
// Modified by:
// Created:     2005-01-17
// Copyright:   (c) 2005 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/app.h"
#include "wx/log.h"
#include "wx/intl.h"
#include "wx/utils.h"

WX_CHECK_BUILD_OPTIONS("wxQA")

#if wxUSE_DEBUGREPORT && wxUSE_XML

#include "wx/debugrpt.h"
#if wxUSE_FFILE
import WX.Cmn.FFile;
#elif wxUSE_FILE
    #include "wx/file.h"
#endif

#include "wx/filename.h"
#include "wx/dir.h"
#include "wx/dynlib.h"

#include "wx/xml/xml.h"

#if wxUSE_STACKWALKER
    #include "wx/stackwalk.h"
#endif

#if wxUSE_CRASHREPORT
    #include "wx/msw/crashrpt.h"
#endif

#if wxUSE_ZIPSTREAM
    import WX.Cmn.ZipStream;
    import WX.Cmn.WFStream;
#endif // wxUSE_ZIPSTREAM

// ----------------------------------------------------------------------------
// XmlStackWalker: stack walker specialization which dumps stack in XML
// ----------------------------------------------------------------------------

#if wxUSE_STACKWALKER

class XmlStackWalker : public wxStackWalker
{
public:
    explicit XmlStackWalker(wxXmlNode *nodeStack) : m_nodeStack(nodeStack)
    {
    }

    bool IsOk() const { return m_isOk; }

protected:
    void OnStackFrame(const wxStackFrame& frame) override;

    wxXmlNode *m_nodeStack;
    bool m_isOk{false};
};

// ----------------------------------------------------------------------------
// local functions
// ----------------------------------------------------------------------------

static inline void
HexProperty(wxXmlNode *node, const wxChar *name, wxUIntPtr value)
{
    node->AddAttribute(name, wxString::Format("%#zx", value));
}

static inline void
NumProperty(wxXmlNode *node, const wxChar *name, unsigned long value)
{
    node->AddAttribute(name, wxString::Format("%lu", value));
}

static inline void
TextElement(wxXmlNode *node, const wxChar *name, const wxString& value)
{
    wxXmlNode *nodeChild = new wxXmlNode(wxXML_ELEMENT_NODE, name);
    node->AddChild(nodeChild);
    nodeChild->AddChild(new wxXmlNode(wxXML_TEXT_NODE, {}, value));
}

#if wxUSE_CRASHREPORT && defined(__INTEL__)

static inline void
HexElement(wxXmlNode *node, const wxChar *name, unsigned long value)
{
    TextElement(node, name, wxString::Format("%08lx", value));
}

#endif // wxUSE_CRASHREPORT

// ============================================================================
// XmlStackWalker implementation
// ============================================================================

void XmlStackWalker::OnStackFrame(const wxStackFrame& frame)
{
    m_isOk = true;

    wxXmlNode *nodeFrame = new wxXmlNode(wxXML_ELEMENT_NODE, "frame");
    m_nodeStack->AddChild(nodeFrame);

    NumProperty(nodeFrame, "level", frame.GetLevel());
    wxString func = frame.GetName();
    if ( !func.empty() )
        nodeFrame->AddAttribute("function", func);

    HexProperty(nodeFrame, "offset", frame.GetOffset());
    HexProperty(nodeFrame, "address", wxPtrToUInt(frame.GetAddress()));

    wxString module = frame.GetModule();
    if ( !module.empty() )
        nodeFrame->AddAttribute("module", module);

    if ( frame.HasSourceLocation() )
    {
        nodeFrame->AddAttribute("file", frame.GetFileName());
        NumProperty(nodeFrame, "line", frame.GetLine());
    }

    const size_t nParams = frame.GetParamCount();
    if ( nParams )
    {
        wxXmlNode *nodeParams = new wxXmlNode(wxXML_ELEMENT_NODE, "parameters");
        nodeFrame->AddChild(nodeParams);

        for ( size_t n = 0; n < nParams; n++ )
        {
            wxXmlNode *
                nodeParam = new wxXmlNode(wxXML_ELEMENT_NODE, "parameter");
            nodeParams->AddChild(nodeParam);

            NumProperty(nodeParam, "number", n);

            wxString type, name, value;
            if ( !frame.GetParam(n, &type, &name, &value) )
                continue;

            if ( !type.empty() )
                TextElement(nodeParam, "type", type);

            if ( !name.empty() )
                TextElement(nodeParam, "name", name);

            if ( !value.empty() )
                TextElement(nodeParam, "value", value);
        }
    }
}

#endif // wxUSE_STACKWALKER

// ============================================================================
// wxDebugReport implementation
// ============================================================================

// ----------------------------------------------------------------------------
// initialization and cleanup
// ----------------------------------------------------------------------------

wxDebugReport::wxDebugReport()
{
    // get a temporary directory name
    wxString appname = GetReportName();

    // we can't use CreateTempFileName() because it creates a file, not a
    // directory, so do our best to create a unique name ourselves
    //
    // of course, this doesn't protect us against malicious users...
#if wxUSE_DATETIME
    m_dir.Printf("%s%c%s_dbgrpt-%lu-%s",
                 wxFileName::GetTempDir(), wxFILE_SEP_PATH, appname,
                 wxGetProcessId(),
                 wxDateTime::Now().Format("%Y%m%dT%H%M%S"));
#else
    m_dir.Printf("%s%c%s_dbgrpt-%lu",
                 wxFileName::GetTempDir(), wxFILE_SEP_PATH, appname,
                 wxGetProcessId());
#endif

    // as we are going to save the process state there use restrictive
    // permissions
    if ( !wxMkdir(m_dir, 0700) )
    {
        wxLogSysError(_("Failed to create directory \"%s\""), m_dir.c_str());
        wxLogError(_("Debug report couldn't be created."));

        Reset();
    }
}

wxDebugReport::~wxDebugReport()
{
    if ( !m_dir.empty() )
    {
        // remove all files in this directory
        wxDir dir(m_dir);
        wxString file;
        for ( bool cont = dir.GetFirst(&file); cont; cont = dir.GetNext(&file) )
        {
            if ( wxRemove(wxFileName(m_dir, file).GetFullPath()) != 0 )
            {
                wxLogSysError(_("Failed to remove debug report file \"%s\""),
                              file.c_str());
                m_dir.clear();
                break;
            }
        }
    }

    if ( !m_dir.empty() )
    {
        if ( wxRmDir(m_dir) != 0 )
        {
            wxLogSysError(_("Failed to clean up debug report directory \"%s\""),
                          m_dir.c_str());
        }
    }
}

// ----------------------------------------------------------------------------
// various helpers
// ----------------------------------------------------------------------------

wxString wxDebugReport::GetReportName() const
{
    if ( wxTheApp )
        return wxTheApp->GetAppName();

    return "wx";
}

void
wxDebugReport::AddFile(const wxString& filename, const wxString& description)
{
    wxString name;
    wxFileName fn(filename);
    if ( fn.IsAbsolute() )
    {
        // we need to copy the file to the debug report directory: give it the
        // same name there
        name = fn.GetFullName();

        if (!wxCopyFile(fn.GetFullPath(),
                        wxFileName(GetDirectory(), name).GetFullPath()))
           return;
    }
    else // file relative to the report directory
    {
        name = filename;

        wxASSERT_MSG( wxFileName(GetDirectory(), name).FileExists(),
                      "file should exist in debug report directory" );
    }

    m_files.push_back(name);
    m_descriptions.push_back(description);
}

bool
wxDebugReport::AddText(const wxString& filename,
                       const wxString& text,
                       const wxString& description)
{
#if wxUSE_FFILE || wxUSE_FILE
    wxASSERT_MSG( !wxFileName(filename).IsAbsolute(),
                  "filename should be relative to debug report directory" );

    const wxString fullPath = wxFileName(GetDirectory(), filename).GetFullPath();
#if wxUSE_FFILE
    wxFFile file(fullPath, "w");
#elif wxUSE_FILE
    wxFile file(fullPath, wxFile::write);
#endif
    if ( !file.IsOpened() || !file.Write(text, wxConvAuto()) )
        return false;

    AddFile(filename, description);

    return true;
#else // !wxUSE_FFILE && !wxUSE_FILE
    return false;
#endif
}

void wxDebugReport::RemoveFile(const wxString& name)
{
    const auto iter_n = std::find_if(m_files.cbegin(), m_files.cend(),
    [name](const auto& filen){
        return name.IsSameAs(filen);
    });
    const int n = std::distance(std::cbegin(m_files), iter_n);

    wxCHECK_RET( n != wxNOT_FOUND, "No such file in wxDebugReport" );

    m_files.erase(std::begin(m_files) + n);
    m_descriptions.erase(std::begin(m_descriptions) + n);

    wxRemove(wxFileName(GetDirectory(), name).GetFullPath());
}

bool wxDebugReport::GetFile(size_t n, wxString *name, wxString *desc) const
{
    if ( n >= m_files.size() )
        return false;

    if ( name )
        *name = m_files[n];
    if ( desc )
        *desc = m_descriptions[n];

    return true;
}

void wxDebugReport::AddAll(Context context)
{
#if wxUSE_STACKWALKER
    AddContext(context);
#endif // wxUSE_STACKWALKER

#if wxUSE_CRASHREPORT
    AddDump(context);
#endif // wxUSE_CRASHREPORT

#if !wxUSE_STACKWALKER && !wxUSE_CRASHREPORT
    wxUnusedVar(context);
#endif
}

// ----------------------------------------------------------------------------
// adding basic text information about current context
// ----------------------------------------------------------------------------

#if wxUSE_STACKWALKER

bool wxDebugReport::DoAddSystemInfo(wxXmlNode *nodeSystemInfo)
{
    nodeSystemInfo->AddAttribute("description", wxGetOsDescription());

    return true;
}

bool wxDebugReport::DoAddLoadedModules(wxXmlNode *nodeModules)
{
    wxDynamicLibraryDetailsArray modules(wxDynamicLibrary::ListLoaded());

    if ( modules.empty() )
        return false;

    for ( size_t n = 0; n != modules.size(); n++ )
    {
        const wxDynamicLibraryDetails& info = modules[n];

        wxXmlNode *nodeModule = new wxXmlNode(wxXML_ELEMENT_NODE, "module");
        nodeModules->AddChild(nodeModule);

        wxString path = info.GetPath();
        if ( path.empty() )
            path = info.GetName();
        if ( !path.empty() )
            nodeModule->AddAttribute("path", path);

        void *addr = nullptr;
        size_t len = 0;
        if ( info.GetAddress(&addr, &len) )
        {
            HexProperty(nodeModule, "address", wxPtrToUInt(addr));
            HexProperty(nodeModule, "size", len);
        }

        wxString ver = info.GetVersion();
        if ( !ver.empty() )
        {
            nodeModule->AddAttribute("version", ver);
        }
    }

    return true;
}

bool wxDebugReport::DoAddExceptionInfo(wxXmlNode *nodeContext)
{
#if wxUSE_CRASHREPORT
    wxCrashContext c;
    if ( !c.code )
        return false;

    wxXmlNode *nodeExc = new wxXmlNode(wxXML_ELEMENT_NODE, "exception");
    nodeContext->AddChild(nodeExc);

    HexProperty(nodeExc, "code", c.code);
    nodeExc->AddAttribute("name", c.GetExceptionString());
    HexProperty(nodeExc, "address", wxPtrToUInt(c.addr));

#ifdef __INTEL__
    wxXmlNode *nodeRegs = new wxXmlNode(wxXML_ELEMENT_NODE, "registers");
    nodeContext->AddChild(nodeRegs);
    HexElement(nodeRegs, "eax", c.regs.eax);
    HexElement(nodeRegs, "ebx", c.regs.ebx);
    HexElement(nodeRegs, "ecx", c.regs.edx);
    HexElement(nodeRegs, "edx", c.regs.edx);
    HexElement(nodeRegs, "esi", c.regs.esi);
    HexElement(nodeRegs, "edi", c.regs.edi);

    HexElement(nodeRegs, "ebp", c.regs.ebp);
    HexElement(nodeRegs, "esp", c.regs.esp);
    HexElement(nodeRegs, "eip", c.regs.eip);

    HexElement(nodeRegs, "cs", c.regs.cs);
    HexElement(nodeRegs, "ds", c.regs.ds);
    HexElement(nodeRegs, "es", c.regs.es);
    HexElement(nodeRegs, "fs", c.regs.fs);
    HexElement(nodeRegs, "gs", c.regs.gs);
    HexElement(nodeRegs, "ss", c.regs.ss);

    HexElement(nodeRegs, "flags", c.regs.flags);
#endif // __INTEL__

    return true;
#else // !wxUSE_CRASHREPORT
    wxUnusedVar(nodeContext);

    return false;
#endif // wxUSE_CRASHREPORT/!wxUSE_CRASHREPORT
}

bool wxDebugReport::AddContext(wxDebugReport::Context ctx)
{
    wxCHECK_MSG( IsOk(), false, "use IsOk() first" );

    // create XML dump of current context
    wxXmlDocument xmldoc;
    wxXmlNode *nodeRoot = new wxXmlNode(wxXML_ELEMENT_NODE, "report");
    xmldoc.SetRoot(nodeRoot);
    nodeRoot->AddAttribute("version"), wxT("1.0");
    nodeRoot->AddAttribute("kind"), ctx == Context::Current ? wxT("user"
                                                             : "exception");

    // add system information
    wxXmlNode *nodeSystemInfo = new wxXmlNode(wxXML_ELEMENT_NODE, "system");
    if ( DoAddSystemInfo(nodeSystemInfo) )
        nodeRoot->AddChild(nodeSystemInfo);
    else
        delete nodeSystemInfo;

    // add information about the loaded modules
    wxXmlNode *nodeModules = new wxXmlNode(wxXML_ELEMENT_NODE, "modules");
    if ( DoAddLoadedModules(nodeModules) )
        nodeRoot->AddChild(nodeModules);
    else
        delete nodeModules;

    // add CPU context information: this only makes sense for exceptions as our
    // current context is not very interesting otherwise
    if ( ctx == Context::Exception )
    {
        wxXmlNode *nodeContext = new wxXmlNode(wxXML_ELEMENT_NODE, "context");
        if ( DoAddExceptionInfo(nodeContext) )
            nodeRoot->AddChild(nodeContext);
        else
            delete nodeContext;
    }

    // add stack traceback
#if wxUSE_STACKWALKER
    wxXmlNode *nodeStack = new wxXmlNode(wxXML_ELEMENT_NODE, "stack");
    XmlStackWalker sw(nodeStack);
#if wxUSE_ON_FATAL_EXCEPTION
    if ( ctx == Context::Exception )
    {
        sw.WalkFromException();
    }
    else // Context_Current
#endif // wxUSE_ON_FATAL_EXCEPTION
    {
        sw.Walk();
    }

    if ( sw.IsOk() )
        nodeRoot->AddChild(nodeStack);
    else
        delete nodeStack;
#endif // wxUSE_STACKWALKER

    // finally let the user add any extra information he needs
    DoAddCustomContext(nodeRoot);


    // save the entire context dump in a file
    wxFileName fn(m_dir, GetReportName(), "xml");

    if ( !xmldoc.Save(fn.GetFullPath()) )
        return false;

    AddFile(fn.GetFullName(), _("process context description"));

    return true;
}

#endif // wxUSE_STACKWALKER

// ----------------------------------------------------------------------------
// adding core dump
// ----------------------------------------------------------------------------

#if wxUSE_CRASHREPORT

bool wxDebugReport::AddDump(Context ctx)
{
    wxCHECK_MSG( IsOk(), false, "use IsOk() first" );

    wxFileName fn(m_dir, GetReportName(), "dmp");
    wxCrashReport::SetFileName(fn.GetFullPath());

    if ( !(ctx == Context::Exception ? wxCrashReport::Generate()
                                    : wxCrashReport::GenerateNow()) )
            return false;

    AddFile(fn.GetFullName(), _("dump of the process state (binary)"));

    return true;
}

#endif // wxUSE_CRASHREPORT

// ----------------------------------------------------------------------------
// report processing
// ----------------------------------------------------------------------------

bool wxDebugReport::Process()
{
    if ( !GetFilesCount() )
    {
        wxLogError(_("Debug report generation has failed."));

        return false;
    }

    if ( !DoProcess() )
    {
        wxLogError(_("Processing debug report has failed, leaving the files in \"%s\" directory."),
                   GetDirectory().c_str());

        Reset();

        return false;
    }

    return true;
}

bool wxDebugReport::DoProcess()
{
    wxString msg(_("A debug report has been generated. It can be found in"));
    msg << "\n"
           "\t") << GetDirectory() << wxT("\n\n"
        << _("And includes the following files:\n");

    wxString name, desc;
    const size_t count = GetFilesCount();
    for ( size_t n = 0; n < count; n++ )
    {
        GetFile(n, &name, &desc);
        msg += wxString::Format("\t%s: %s\n", name, desc);
    }

    msg += _("\nPlease send this report to the program maintainer, thank you!\n");

    wxLogMessage("%s", msg.c_str());

    // we have to do this or the report would be deleted, and we don't even
    // have any way to ask the user if he wants to keep it from here
    Reset();

    return true;
}

wxFileName wxDebugReport::GetSaveLocation() const
{
    wxFileName fn;
    fn.SetPath(GetDirectory());
    return fn;
}

// ============================================================================
// wxDebugReport-derived classes
// ============================================================================

#if wxUSE_ZIPSTREAM

// ----------------------------------------------------------------------------
// wxDebugReportCompress
// ----------------------------------------------------------------------------

void wxDebugReportCompress::SetCompressedFileDirectory(const wxString& dir)
{
    wxASSERT_MSG( m_zipfile.empty(), "Too late: call this before Process()" );

    m_zipDir = dir;
}

void wxDebugReportCompress::SetCompressedFileBaseName(const wxString& name)
{
    wxASSERT_MSG( m_zipfile.empty(), "Too late: call this before Process()" );

    m_zipName = name;
}

wxFileName wxDebugReportCompress::GetSaveLocation() const
{
    // Use the default directory as a basis for the save location, e.g.
    // %temp%/someName becomes %temp%/someName.zip.
    wxFileName fn(GetDirectory());
    if ( !m_zipDir.empty() )
        fn.SetPath(m_zipDir);
    if ( !m_zipName.empty() )
        fn.SetName(m_zipName);
    fn.SetExt("zip");
    return fn;
}

bool wxDebugReportCompress::DoProcess()
{
static constexpr bool HAS_FILE_STREAMS = (wxUSE_STREAMS && (wxUSE_FILE || wxUSE_FFILE));
#if HAS_FILE_STREAMS
    const size_t count = GetFilesCount();
    if ( !count )
        return false;

    // create the streams
    const wxString ofullPath = GetSaveLocation().GetFullPath();
#if wxUSE_FFILE
    wxFFileOutputStream os(ofullPath, "wb");
#elif wxUSE_FILE
    wxFileOutputStream os(ofullPath);
#endif
    if ( !os.IsOk() )
        return false;
    wxZipOutputStream zos(os, 9);

    // add all files to the ZIP one
    wxString name, desc;
    for ( size_t n = 0; n < count; n++ )
    {
        GetFile(n, &name, &desc);

        wxZipEntry *ze = new wxZipEntry(name);
        ze->SetComment(desc);

        if ( !zos.PutNextEntry(ze) )
            return false;

        const wxString ifullPath = wxFileName(GetDirectory(), name).GetFullPath();
#if wxUSE_FFILE
        wxFFileInputStream is(ifullPath);
#elif wxUSE_FILE
        wxFileInputStream is(ifullPath);
#endif
        if ( !is.IsOk() || !zos.Write(is).IsOk() )
            return false;
    }

    if ( !zos.Close() )
        return false;

    m_zipfile = ofullPath;

    return true;
#else
    return false;
#endif // HAS_FILE_STREAMS
}

// ----------------------------------------------------------------------------
// wxDebugReportUpload
// ----------------------------------------------------------------------------

wxDebugReportUpload::wxDebugReportUpload(const wxString& url,
                                         const wxString& input,
                                         const wxString& action,
                                         const wxString& curl)
                   : m_uploadURL(url),
                     m_inputField(input),
                     m_curlCmd(curl)
{
    if ( m_uploadURL.Last() != wxT('/') )
        m_uploadURL += wxT('/');
    m_uploadURL += action;
}

bool wxDebugReportUpload::DoProcess()
{
    if ( !wxDebugReportCompress::DoProcess() )
        return false;


    std::vector<wxString> output;
    std::vector<wxString> errors;

    int rc = wxExecute(wxString::Format
                       (
                            "%s -F \"%s=@%s\" %s",
                            m_curlCmd.c_str(),
                            m_inputField.c_str(),
                            GetCompressedFileName().c_str(),
                            m_uploadURL.c_str()
                       ),
                       output,
                       errors);
    if ( rc == -1 )
    {
        wxLogError(_("Failed to execute curl, please install it in PATH."));
    }
    else if ( rc != 0 )
    {
        const size_t count = errors.size();
        if ( count )
        {
            for ( size_t n = 0; n < count; n++ )
            {
                wxLogWarning("%s", errors[n].c_str());
            }
        }

        wxLogError(_("Failed to upload the debug report (error code %d)."), rc);
    }
    else // rc == 0
    {
        if ( OnServerReply(output) )
            return true;
    }

    return false;
}

#endif // wxUSE_ZIPSTREAM

#endif // wxUSE_DEBUGREPORT
