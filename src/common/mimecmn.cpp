/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/mimecmn.cpp
// Purpose:     classes and functions to manage MIME types
// Author:      Vadim Zeitlin
// Modified by:
//  Chris Elliott (biol75@york.ac.uk) 5 Dec 00: write support for Win32
// Created:     23.09.98
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence (part of wxExtra library)
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_MIMETYPE

#include "wx/mimetype.h"

#include "wx/string.h"
#include "wx/intl.h"
#include "wx/log.h"
#include "wx/module.h"
#include "wx/crt.h"
#include "wx/file.h"
#include "wx/iconloc.h"
#include "wx/confbase.h"

// implementation classes:
#if defined(WX_WINDOWS)
    #include "wx/msw/mimetype.h"
#elif ( defined(__DARWIN__) )
    #include "wx/osx/mimetype.h"
#else // Unix
    #include "wx/unix/mimetype.h"
#endif

// ----------------------------------------------------------------------------
// wxMimeTypeCommands
// ----------------------------------------------------------------------------

void
wxMimeTypeCommands::AddOrReplaceVerb(const wxString& verb, const wxString& cmd)
{
    const auto& verb_iter = std::find_if(m_verbs.cbegin(), m_verbs.cend(),
    [verb](const auto& m_verb)
    {
        return verb.IsSameAs(m_verb, false);
    });

    if ( verb_iter == m_verbs.cend() )
    {
        m_verbs.push_back(verb);
        m_commands.push_back(cmd);
    }
    else
    {
        const auto n = std::distance(std::cbegin(m_verbs), verb_iter);
        m_commands[n] = cmd;
    }
}

wxString
wxMimeTypeCommands::GetCommandForVerb(const wxString& verb, size_t *idx) const
{
    wxString s;

    const auto& verb_iter = std::find_if(m_verbs.cbegin(), m_verbs.cend(),
        [verb](const auto& m_verb)
        {
            return verb.IsSameAs(m_verb, false);
        });

    if ( verb_iter != m_verbs.cend() )
    {
        const auto n = std::distance(std::cbegin(m_verbs), verb_iter);
        
        s = m_commands[(size_t)n];
        if ( idx )
            *idx = n;
    }
    else if ( idx )
    {
        // different from any valid index
        *idx = (size_t)-1;
    }

    return s;
}

wxString wxMimeTypeCommands::GetVerbCmd(size_t n) const
{
    return m_verbs[n] + wxT('=') + m_commands[n];
}

// ----------------------------------------------------------------------------
// wxFileTypeInfo
// ----------------------------------------------------------------------------

void wxFileTypeInfo::DoVarArgInit(const wxString& mimeType,
                                  const wxString& openCmd,
                                  const wxString& printCmd,
                                  const wxString& desc,
                                  va_list argptr)
{
    m_mimeType = mimeType;
    m_openCmd = openCmd;
    m_printCmd = printCmd;
    m_desc = desc;

    for ( ;; )
    {
        // icc gives this warning in its own va_arg() macro, argh

        const wxArgNormalizedString ext(WX_VA_ARG_STRING(argptr));

        if ( !ext )
        {
            // NULL terminates the list
            break;
        }

        m_exts.push_back(ext.GetString());
    }
}

void wxFileTypeInfo::VarArgInit(const wxString *mimeType,
                                const wxString *openCmd,
                                const wxString *printCmd,
                                const wxString *desc,
                                ...)
{
    va_list argptr;
    va_start(argptr, desc);

    DoVarArgInit(*mimeType, *openCmd, *printCmd, *desc, argptr);

    va_end(argptr);
}


wxFileTypeInfo::wxFileTypeInfo(const std::vector<wxString>& sArray)
    : m_mimeType(sArray[0u])
    , m_openCmd( sArray[1u])
    , m_printCmd(sArray[2u])
    , m_desc(    sArray[3u])
{
    std::copy(sArray.cbegin(), sArray.cend(), std::back_inserter(m_exts));
}

/* static */
wxString wxFileType::ExpandCommand(const wxString& command,
                                   const wxFileType::MessageParameters& params)
{
    bool hasFilename = false;

    // We consider that only the file names with spaces in them need to be
    // handled specially. This is not perfect, but this can be done easily
    // under all platforms while handling the file names with quotes in them,
    // for example, needs to be done differently.
    const bool needToQuoteFilename = params.GetFileName().find_first_of(" \t")
                                        != wxString::npos;

    wxString str;
    for ( const wxChar *pc = command.c_str(); *pc != wxT('\0'); pc++ ) {
        if ( *pc == wxT('%') ) {
            switch ( *++pc ) {
                case wxT('s'):
                    // don't quote the file name if it's already quoted: notice
                    // that we check for a quote following it and not preceding
                    // it as at least under Windows we can have commands
                    // containing "file://%s" (with quotes) in them so the
                    // argument may be quoted even if there is no quote
                    // directly before "%s" itself
                    if ( needToQuoteFilename && pc[1] != '"' )
                        str << wxT('"') << params.GetFileName() << wxT('"');
                    else
                        str << params.GetFileName();
                    hasFilename = true;
                    break;

                case wxT('t'):
                    // '%t' expands into MIME type (quote it too just to be
                    // consistent)
                    str << wxT('\'') << params.GetMimeType() << wxT('\'');
                    break;

                case wxT('{'):
                    {
                        const wxChar *pEnd = wxStrchr(pc, wxT('}'));
                        if ( pEnd == nullptr ) {
                            wxString mimetype;
                            wxLogWarning(_("Unmatched '{' in an entry for mime type %s."),
                                         params.GetMimeType().c_str());
                            str << wxT("%{");
                        }
                        else {
                            wxString param(pc + 1, pEnd - pc - 1);
                            str << wxT('\'') << params.GetParamValue(param) << wxT('\'');
                            pc = pEnd;
                        }
                    }
                    break;

                case wxT('n'):
                case wxT('F'):
                    // TODO %n is the number of parts, %F is an array containing
                    //      the names of temp files these parts were written to
                    //      and their mime types.
                    break;

                default:
                    wxLogDebug(wxT("Unknown field %%%c in command '%s'."),
                               *pc, command.c_str());
                    str << *pc;
            }
        }
        else {
            str << *pc;
        }
    }

    // metamail(1) man page states that if the mailcap entry doesn't have '%s'
    // the program will accept the data on stdin so normally we should append
    // "< %s" to the end of the command in such case, but not all commands
    // behave like this, in particular a common test is 'test -n "$DISPLAY"'
    // and appending "< %s" to this command makes the test fail... I don't
    // know of the correct solution, try to guess what we have to do.

    // test now carried out on reading file so test should never get here
    if ( !hasFilename && !str.empty()
#ifdef __UNIX__
                      && !str.StartsWith(wxT("test "))
#endif // Unix
       )
    {
        str << wxT(" < ");
        if ( needToQuoteFilename )
            str << '"';
        str << params.GetFileName();
        if ( needToQuoteFilename )
            str << '"';
    }

    return str;
}

wxFileType::wxFileType(const wxFileTypeInfo& info)
    : m_info(&info)
{
}

wxFileType::wxFileType()
    : m_impl(new wxFileTypeImpl)
{
}

wxFileType::~wxFileType()
{
    delete m_impl;
}

bool wxFileType::GetExtensions(std::vector<wxString>& extensions)
{
    if ( m_info )
    {
        extensions = m_info->GetExtensions();
        return true;
    }

    return m_impl->GetExtensions(extensions);
}

bool wxFileType::GetMimeType(wxString *mimeType) const
{
    wxCHECK_MSG( mimeType, false, wxT("invalid parameter in GetMimeType") );

    if ( m_info )
    {
        *mimeType = m_info->GetMimeType();

        return true;
    }

    return m_impl->GetMimeType(mimeType);
}

bool wxFileType::GetMimeTypes(std::vector<wxString>& mimeTypes) const
{
    if ( m_info )
    {
        mimeTypes.clear();
        mimeTypes.push_back(m_info->GetMimeType());

        return true;
    }

    return m_impl->GetMimeTypes(mimeTypes);
}

bool wxFileType::GetIcon(wxIconLocation *iconLoc) const
{
    if ( m_info )
    {
        if ( iconLoc )
        {
            iconLoc->SetFileName(m_info->GetIconFile().ToStdString());
#ifdef WX_WINDOWS
            iconLoc->SetIndex(m_info->GetIconIndex());
#endif // WX_WINDOWS
        }

        return true;
    }

    return m_impl->GetIcon(iconLoc);
}

bool
wxFileType::GetIcon(wxIconLocation *iconloc,
                    const MessageParameters& params) const
{
    // FIXME: We're checking iconloc twice.
    if ( !GetIcon(iconloc) )
    {
        return false;
    }

    // we may have "%s" in the icon location string, at least under Windows, so
    // expand this
    if ( iconloc )
    {
        iconloc->SetFileName(ExpandCommand(iconloc->GetFileName(), params).ToStdString());
    }

    return true;
}

bool wxFileType::GetDescription(wxString *desc) const
{
    wxCHECK_MSG( desc, false, wxT("invalid parameter in GetDescription") );

    if ( m_info )
    {
        *desc = m_info->GetDescription();

        return true;
    }

    return m_impl->GetDescription(desc);
}

bool
wxFileType::GetOpenCommand(wxString *openCmd,
                           const wxFileType::MessageParameters& params) const
{
    wxCHECK_MSG( openCmd, false, wxT("invalid parameter in GetOpenCommand") );

    if ( m_info )
    {
        *openCmd = ExpandCommand(m_info->GetOpenCommand(), params);

        return true;
    }

    return m_impl->GetOpenCommand(openCmd, params);
}

wxString wxFileType::GetOpenCommand(const wxString& filename) const
{
    wxString cmd;
    if ( !GetOpenCommand(&cmd, filename) )
    {
        // return empty string to indicate an error
        cmd.clear();
    }

    return cmd;
}

bool
wxFileType::GetPrintCommand(wxString *printCmd,
                            const wxFileType::MessageParameters& params) const
{
    wxCHECK_MSG( printCmd, false, wxT("invalid parameter in GetPrintCommand") );

    if ( m_info )
    {
        *printCmd = ExpandCommand(m_info->GetPrintCommand(), params);

        return true;
    }

    return m_impl->GetPrintCommand(printCmd, params);
}

wxString
wxFileType::GetExpandedCommand(const wxString& verb,
                               const wxFileType::MessageParameters& params) const
{
    return m_impl->GetExpandedCommand(verb, params);
}


size_t wxFileType::GetAllCommands(std::vector<wxString> *verbs,
                                  std::vector<wxString> *commands,
                                  const wxFileType::MessageParameters& params) const
{
    if ( verbs )
        verbs->clear();
    if ( commands )
        commands->clear();

#if defined (WX_WINDOWS)  || defined(__UNIX__)
    return m_impl->GetAllCommands(verbs, commands, params);
#else // !WX_WINDOWS || __UNIX__
    // we don't know how to retrieve all commands, so just try the 2 we know
    // about
    size_t count = 0;
    wxString cmd;
    if ( GetOpenCommand(&cmd, params) )
    {
        if ( verbs )
            verbs->push_back(wxT("Open"));
        if ( commands )
            commands->push_back(cmd);
        count++;
    }

    if ( GetPrintCommand(&cmd, params) )
    {
        if ( verbs )
            verbs->push_back(wxT("Print"));
        if ( commands )
            commands->push_back(cmd);

        count++;
    }

    return count;
#endif // WX_WINDOWS/| __UNIX__
}

bool wxFileType::Unassociate()
{
#if defined(WX_WINDOWS)
    return m_impl->Unassociate();
#elif defined(__UNIX__)
    return m_impl->Unassociate(this);
#else
    wxFAIL_MSG( wxT("not implemented") ); // TODO
    return false;
#endif
}

bool wxFileType::SetCommand(const wxString& cmd,
                            const wxString& verb,
                            bool overwriteprompt)
{
#if defined (WX_WINDOWS)  || defined(__UNIX__)
    return m_impl->SetCommand(cmd, verb, overwriteprompt);
#else
    wxUnusedVar(cmd);
    wxUnusedVar(verb);
    wxUnusedVar(overwriteprompt);
    wxFAIL_MSG(wxT("not implemented"));
    return false;
#endif
}

bool wxFileType::SetDefaultIcon(const wxString& cmd, int index)
{
    wxString sTmp = cmd;
#ifdef WX_WINDOWS
    // VZ: should we do this?
    // chris elliott : only makes sense in MS windows
    if ( sTmp.empty() )
        GetOpenCommand(&sTmp, wxFileType::MessageParameters({}, {}));
#endif
    wxCHECK_MSG( !sTmp.empty(), false, wxT("need the icon file") );

#if defined (WX_WINDOWS) || defined(__UNIX__)
    return m_impl->SetDefaultIcon (cmd, index);
#else
    wxUnusedVar(index);
    wxFAIL_MSG(wxT("not implemented"));
    return false;
#endif
}

// ----------------------------------------------------------------------------
// wxMimeTypesManagerFactory
// ----------------------------------------------------------------------------

wxMimeTypesManagerFactory *wxMimeTypesManagerFactory::m_factory = nullptr;

/* static */
void wxMimeTypesManagerFactory::Set(wxMimeTypesManagerFactory *factory)
{
    delete m_factory;

    m_factory = factory;
}

/* static */
wxMimeTypesManagerFactory *wxMimeTypesManagerFactory::Get()
{
    if ( !m_factory )
        m_factory = new wxMimeTypesManagerFactory;

    return m_factory;
}

wxMimeTypesManagerImpl *wxMimeTypesManagerFactory::CreateMimeTypesManagerImpl()
{
    return new wxMimeTypesManagerImpl;
}

// ----------------------------------------------------------------------------
// wxMimeTypesManager
// ----------------------------------------------------------------------------

void wxMimeTypesManager::EnsureImpl()
{
    if ( !m_impl )
        m_impl = wxMimeTypesManagerFactory::Get()->CreateMimeTypesManagerImpl();
}

bool wxMimeTypesManager::IsOfType(const wxString& mimeType,
                                  const wxString& wildcard)
{
    wxASSERT_MSG( mimeType.Find(wxT('*')) == wxNOT_FOUND,
                  wxT("first MIME type can't contain wildcards") );

    // all comparaisons are case insensitive (2nd arg of IsSameAs() is false)
    if ( wildcard.BeforeFirst(wxT('/')).
            IsSameAs(mimeType.BeforeFirst(wxT('/')), false) )
    {
        wxString strSubtype = wildcard.AfterFirst(wxT('/'));

        if ( strSubtype == wxT("*") ||
             strSubtype.IsSameAs(mimeType.AfterFirst(wxT('/')), false) )
        {
            // matches (either exactly or it's a wildcard)
            return true;
        }
    }

    return false;
}

wxMimeTypesManager::~wxMimeTypesManager()
{
    delete m_impl;
}

bool wxMimeTypesManager::Unassociate(wxFileType *ft)
{
    EnsureImpl();

#if defined(__UNIX__) && !defined(__CYGWIN__) && !defined(__WINE__)
    return m_impl->Unassociate(ft);
#else
    return ft->Unassociate();
#endif
}


wxFileType *
wxMimeTypesManager::Associate(const wxFileTypeInfo& ftInfo)
{
    EnsureImpl();

#if defined(WX_WINDOWS) || defined(__UNIX__)
    return m_impl->Associate(ftInfo);
#else // other platforms
    wxUnusedVar(ftInfo);
    wxFAIL_MSG( wxT("not implemented") ); // TODO
    return NULL;
#endif // platforms
}

wxFileType *
wxMimeTypesManager::GetFileTypeFromExtension(const wxString& ext)
{
    EnsureImpl();

    wxString::const_iterator i = ext.begin();
    const wxString::const_iterator end = ext.end();
    wxString extWithoutDot;
    if ( i != end && *i == '.' )
        extWithoutDot.assign(++i, ext.end());
    else
        extWithoutDot = ext;

    wxCHECK_MSG( !ext.empty(), nullptr, wxT("extension can't be empty") );

    wxFileType *ft = m_impl->GetFileTypeFromExtension(extWithoutDot);

    if ( !ft ) {
        // check the fallbacks
        //
        // TODO linear search is potentially slow, perhaps we should use a
        //       sorted array?
        for ( const auto& fallback : m_fallbacks ) {
            const auto iter_idx = std::find_if(fallback.GetExtensions().cbegin(), fallback.GetExtensions().cend(),
                [ext](const auto& anExt)
                {
                    return ext.IsSameAs(anExt);
                });
            if ( iter_idx != std::cend(fallback.GetExtensions())) {
                ft = new wxFileType(fallback);

                break;
            }
        }
    }

    return ft;
}

wxFileType *
wxMimeTypesManager::GetFileTypeFromMimeType(const wxString& mimeType)
{
    EnsureImpl();
    wxFileType *ft = m_impl->GetFileTypeFromMimeType(mimeType);

    if ( !ft ) {
        // check the fallbacks
        //
        // TODO linear search is potentially slow, perhaps we should use a
        //      sorted array?
        for ( const auto& fallback : m_fallbacks ) {
            if ( wxMimeTypesManager::IsOfType(mimeType,
                                              fallback.GetMimeType()) ) {
                ft = new wxFileType(fallback);

                break;
            }
        }
    }

    return ft;
}

// TODO: Null ptr check / reference
void wxMimeTypesManager::AddFallbacks(const wxFileTypeInfo *filetypes)
{
    EnsureImpl();
    for ( const wxFileTypeInfo *ft = filetypes; ft && ft->IsValid(); ft++ ) {
        AddFallback(*ft);
    }
}

// TODO: Return vector
size_t wxMimeTypesManager::EnumAllFileTypes(std::vector<wxString>& mimetypes)
{
    EnsureImpl();
    size_t countAll = m_impl->EnumAllFileTypes(mimetypes);

    // add the fallback filetypes
    for ( const auto& fallback : m_fallbacks ) {
        const auto fallMime = fallback.GetMimeType();
        const auto iter_idx = std::find_if(mimetypes.cbegin(), mimetypes.cend(),
            [fallMime](const auto& aType)
            {
                return fallMime.IsSameAs(aType);
            });
        if ( iter_idx == std::cend(mimetypes) ) {
            mimetypes.push_back(fallback.GetMimeType());
            countAll++;
        }
    }

    return countAll;
}

void wxMimeTypesManager::Initialize(int mcapStyle,
                                    const wxString& sExtraDir)
{
#if defined(__UNIX__) && !defined(__CYGWIN__) && !defined(__WINE__)
    EnsureImpl();

    m_impl->Initialize(mcapStyle, sExtraDir);
#else
    // FIXME: ??
    std::ignore = mcapStyle;
    (void)sExtraDir;
#endif // Unix
}

// and this function clears all the data from the manager
void wxMimeTypesManager::ClearData()
{
#if defined(__UNIX__) && !defined(__CYGWIN__) && !defined(__WINE__)
    EnsureImpl();

    m_impl->ClearData();
#endif // Unix
}

// private object
static wxMimeTypesManager gs_mimeTypesManager;

// and public pointer
wxMimeTypesManager *wxTheMimeTypesManager = &gs_mimeTypesManager;

class wxMimeTypeCmnModule: public wxModule
{
public:
    bool OnInit() override { return true; }
    void OnExit() override
    {
        wxMimeTypesManagerFactory::Set(nullptr);

        if ( gs_mimeTypesManager.m_impl != nullptr )
        {
            wxDELETE(gs_mimeTypesManager.m_impl);
            gs_mimeTypesManager.m_fallbacks.clear();
        }
    }

    wxDECLARE_DYNAMIC_CLASS(wxMimeTypeCmnModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxMimeTypeCmnModule, wxModule);

#endif // wxUSE_MIMETYPE
