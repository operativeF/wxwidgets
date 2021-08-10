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

// for compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_MIMETYPE

#include "wx/mimetype.h"

#ifndef WX_PRECOMP
    #include "wx/dynarray.h"
    #include "wx/string.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/module.h"
    #include "wx/crt.h"
#endif //WX_PRECOMP

#include "wx/file.h"
#include "wx/iconloc.h"
#include "wx/confbase.h"

// other standard headers
#include <cctype>

// implementation classes:
#if defined(__WINDOWS__)
    #include "wx/msw/mimetype.h"
#elif ( defined(__DARWIN__) )
    #include "wx/osx/mimetype.h"
#else // Unix
    #include "wx/unix/mimetype.h"
#endif

#include <fmt/core.h>

// ----------------------------------------------------------------------------
// wxMimeTypeCommands
// ----------------------------------------------------------------------------

void
wxMimeTypeCommands::AddOrReplaceVerb(const std::string& verb, const std::string& cmd)
{
    const auto& verb_iter = std::find_if(m_verbs.cbegin(), m_verbs.cend(),
    [verb](const auto& m_verb)
    {
        return wx::utils::IsSameAsNoCase(verb, m_verb);
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

std::string
wxMimeTypeCommands::GetCommandForVerb(const std::string& verb, size_t *idx) const
{
    std::string s;

    const auto& verb_iter = std::find_if(m_verbs.cbegin(), m_verbs.cend(),
        [verb](const auto& m_verb)
        {
            return wx::utils::IsSameAsNoCase(verb, m_verb);
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

std::string wxMimeTypeCommands::GetVerbCmd(size_t n) const
{
    return m_verbs[n] + wxT('=') + m_commands[n];
}

// ----------------------------------------------------------------------------
// wxFileTypeInfo
// ----------------------------------------------------------------------------

wxFileTypeInfo::wxFileTypeInfo(const std::vector<std::string>& sArray)
    : m_mimeType(sArray[0u])
    , m_openCmd( sArray[1u])
    , m_printCmd(sArray[2u])
    , m_desc(    sArray[3u])
{
    std::copy(sArray.cbegin(), sArray.cend(), std::back_inserter(m_exts));
}

/* static */
std::string wxFileType::ExpandCommand(const std::string& command,
                                   const wxFileType::MessageParameters& params)
{
    bool hasFilename = false;

    // We consider that only the file names with spaces in them need to be
    // handled specially. This is not perfect, but this can be done easily
    // under all platforms while handling the file names with quotes in them,
    // for example, needs to be done differently.
    const bool needToQuoteFilename = params.GetFileName().find_first_of(" \t")
                                        != std::string::npos;

    std::string str;
    for ( const char* pc = command.c_str(); *pc != '\0'; pc++ ) {
        if ( *pc == '%' ) {
            switch ( *++pc ) {
                case 's':
                    // don't quote the file name if it's already quoted: notice
                    // that we check for a quote following it and not preceding
                    // it as at least under Windows we can have commands
                    // containing "file://%s" (with quotes) in them so the
                    // argument may be quoted even if there is no quote
                    // directly before "%s" itself
                    if ( needToQuoteFilename && pc[1] != '"' )
                        str += fmt::format("\"{}\"", params.GetFileName());
                    else
                        str += params.GetFileName();
                    hasFilename = true;
                    break;

                case 't':
                    // '%t' expands into MIME type (quote it too just to be
                    // consistent)
                    str += fmt::format("\'{}\'", params.GetMimeType());
                    break;

                case '{':
                    {
                        const char* pEnd = wxStrchr(pc, '}');
                        if ( pEnd == nullptr ) {
                            std::string mimetype;
                            wxLogWarning(_("Unmatched '{' in an entry for mime type %s."),
                                         params.GetMimeType().c_str());
                            str += "%{";
                        }
                        else {
                            std::string param(pc + 1, pEnd - pc - 1);
                            str += fmt::format("\'{}\'", params.GetParamValue(param));
                            pc = pEnd;
                        }
                    }
                    break;

                case 'n':
                case 'F':
                    // TODO %n is the number of parts, %F is an array containing
                    //      the names of temp files these parts were written to
                    //      and their mime types.
                    break;

                default:
                    wxLogDebug(wxT("Unknown field %%%c in command '%s'."),
                               *pc, command.c_str());
                    str += *pc;
            }
        }
        else {
            str += *pc;
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
        str += " < ";
        if ( needToQuoteFilename )
            str += '"';
        str += params.GetFileName();
        if ( needToQuoteFilename )
            str += '"';
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

bool wxFileType::GetExtensions(std::vector<std::string>& extensions)
{
    if ( m_info )
    {
        extensions = m_info->GetExtensions();
        return true;
    }

    return m_impl->GetExtensions(extensions);
}

bool wxFileType::GetMimeType(std::string* mimeType) const
{
    wxCHECK_MSG( mimeType, false, wxT("invalid parameter in GetMimeType") );

    if ( m_info )
    {
        *mimeType = m_info->GetMimeType();

        return true;
    }

    return m_impl->GetMimeType(mimeType);
}

bool wxFileType::GetMimeTypes(std::vector<std::string>& mimeTypes) const
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
            iconLoc->SetFileName(m_info->GetIconFile());
#ifdef __WINDOWS__
            iconLoc->SetIndex(m_info->GetIconIndex());
#endif // __WINDOWS__
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
        iconloc->SetFileName(ExpandCommand(iconloc->GetFileName(), params));
    }

    return true;
}

bool wxFileType::GetDescription(std::string* desc) const
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
wxFileType::GetOpenCommand(std::string* openCmd,
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

std::string wxFileType::GetOpenCommand(const std::string& filename) const
{
    std::string cmd;
    if ( !GetOpenCommand(&cmd, filename) )
    {
        // return empty string to indicate an error
        cmd.clear();
    }

    return cmd;
}

bool
wxFileType::GetPrintCommand(std::string* printCmd,
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

std::string
wxFileType::GetExpandedCommand(const std::string& verb,
                               const wxFileType::MessageParameters& params) const
{
    return m_impl->GetExpandedCommand(verb, params);
}


size_t wxFileType::GetAllCommands(std::vector<std::string> *verbs,
                                  std::vector<std::string> *commands,
                                  const wxFileType::MessageParameters& params) const
{
    if ( verbs )
        verbs->clear();
    if ( commands )
        commands->clear();

#if defined (__WINDOWS__)  || defined(__UNIX__)
    return m_impl->GetAllCommands(verbs, commands, params);
#else // !__WINDOWS__ || __UNIX__
    // we don't know how to retrieve all commands, so just try the 2 we know
    // about
    size_t count = 0;
    std::string cmd;
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
#endif // __WINDOWS__/| __UNIX__
}

bool wxFileType::Unassociate()
{
#if defined(__WINDOWS__)
    return m_impl->Unassociate();
#elif defined(__UNIX__)
    return m_impl->Unassociate(this);
#else
    wxFAIL_MSG( wxT("not implemented") ); // TODO
    return false;
#endif
}

bool wxFileType::SetCommand(const std::string& cmd,
                            const std::string& verb,
                            bool overwriteprompt)
{
#if defined (__WINDOWS__)  || defined(__UNIX__)
    return m_impl->SetCommand(cmd, verb, overwriteprompt);
#else
    wxUnusedVar(cmd);
    wxUnusedVar(verb);
    wxUnusedVar(overwriteprompt);
    wxFAIL_MSG(wxT("not implemented"));
    return false;
#endif
}

bool wxFileType::SetDefaultIcon(const std::string& cmd, int index)
{
    std::string sTmp = cmd;
#ifdef __WINDOWS__
    // VZ: should we do this?
    // chris elliott : only makes sense in MS windows
    if ( sTmp.empty() )
        GetOpenCommand(&sTmp, wxFileType::MessageParameters("", ""));
#endif
    wxCHECK_MSG( !sTmp.empty(), false, wxT("need the icon file") );

#if defined (__WINDOWS__) || defined(__UNIX__)
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

bool wxMimeTypesManager::IsOfType(const std::string& mimeType,
                                  const std::string& wildcard)
{
    wxASSERT_MSG( mimeType.find('*') == std::string::npos,
                  wxT("first MIME type can't contain wildcards") );

    if ( wx::utils::IsSameAsNoCase(wx::utils::BeforeFirst(wildcard, '/'), 
                                   wx::utils::BeforeFirst(mimeType, '/')) )
    {
        std::string strSubtype = wx::utils::AfterFirst(wildcard, '/');

        if ( strSubtype == "*" ||
             wx::utils::IsSameAsNoCase(strSubtype, wx::utils::AfterFirst(mimeType, '/')) )
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

#if defined(__WINDOWS__) || defined(__UNIX__)
    return m_impl->Associate(ftInfo);
#else // other platforms
    wxUnusedVar(ftInfo);
    wxFAIL_MSG( wxT("not implemented") ); // TODO
    return NULL;
#endif // platforms
}

wxFileType *
wxMimeTypesManager::GetFileTypeFromExtension(const std::string& ext)
{
    EnsureImpl();

    std::string::const_iterator i = ext.begin();
    const std::string::const_iterator end = ext.end();
    std::string extWithoutDot;
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
                    return wx::utils::IsSameAsNoCase(ext, anExt);
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
wxMimeTypesManager::GetFileTypeFromMimeType(const std::string& mimeType)
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
size_t wxMimeTypesManager::EnumAllFileTypes(std::vector<std::string>& mimetypes)
{
    EnsureImpl();
    size_t countAll = m_impl->EnumAllFileTypes(mimetypes);

    // add the fallback filetypes
    for ( const auto& fallback : m_fallbacks ) {
        const auto fallMime = fallback.GetMimeType();
        const auto iter_idx = std::find_if(mimetypes.cbegin(), mimetypes.cend(),
            [fallMime](const auto& aType)
            {
                return wx::utils::IsSameAsNoCase(aType, fallMime);
            });
        if ( iter_idx == std::cend(mimetypes) ) {
            mimetypes.push_back(fallback.GetMimeType());
            countAll++;
        }
    }

    return countAll;
}

void wxMimeTypesManager::Initialize(int mcapStyle,
                                    const std::string& sExtraDir)
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
