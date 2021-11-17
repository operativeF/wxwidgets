/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/mimetype.cpp
// Purpose:     classes and functions to manage MIME types
// Author:      Vadim Zeitlin
// Modified by:
// Created:     23.09.98
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWidgets licence (part of base library)
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_MIMETYPE

#include "wx/msw/mimetype.h"

#include "wx/msw/private.h"

#include "wx/intl.h"
#include "wx/log.h"
#include "wx/crt.h"
#if wxUSE_GUI
    #include "wx/msgdlg.h"
#endif

#include "wx/file.h"
#include "wx/iconloc.h"
#include "wx/confbase.h"
#include "wx/dynlib.h"

#ifdef WX_WINDOWS
    #include "wx/msw/registry.h"
    #include "wx/msw/wrapshl.h"

    // For MSVC we can link in the required library explicitly, for the other
    // compilers (e.g. MinGW) this needs to be done at makefiles level.
    #ifdef __VISUALC__
        #pragma comment(lib, "shlwapi")
    #endif
#endif // OS

#include <fmt/core.h>

import <cctype>;
import <string>;

// Unfortunately the corresponding SDK constants are absent from the headers
// shipped with some old MinGW versions (e.g. 4.2.1 from Debian) and we can't
// even test whether they're defined or not, as they're enum elements and not
// preprocessor constants. So we have to always use our own constants.
#define wxASSOCF_NOTRUNCATE (static_cast<ASSOCF>(0x20))
#define wxASSOCSTR_DEFAULTICON (static_cast<ASSOCSTR>(15))


// in case we're compiling in non-GUI mode
class wxIcon;

// These classes use Windows registry to retrieve the required information.
//
// Keys used (not all of them are documented, so it might actually stop working
// in future versions of Windows...):
//  1. "HKCR\MIME\Database\Content Type" contains subkeys for all known MIME
//     types, each key has a string value "Extension" which gives (dot preceded)
//     extension for the files of this MIME type.
//
//  2. "HKCR\.ext" contains
//   a) unnamed value containing the "filetype"
//   b) value "Content Type" containing the MIME type
//
// 3. "HKCR\filetype" contains
//   a) unnamed value containing the description
//   b) subkey "DefaultIcon" with single unnamed value giving the icon index in
//      an icon file
//   c) shell\open\command and shell\open\print subkeys containing the commands
//      to open/print the file (the positional parameters are introduced by %1,
//      %2, ... in these strings, we change them to %s ourselves)

// Notice that HKCR can be used only when reading from the registry, when
// writing to it, we need to write to HKCU\Software\Classes instead as HKCR is
// a merged view of that location and HKLM\Software\Classes that we generally
// wouldn't have the write permissions to but writing to HKCR will try writing
// to the latter unless the key being written to already exists under the
// former, resulting in a "Permission denied" error without administrative
// permissions. So the right thing to do is to use HKCR when reading, to
// respect both per-user and machine-global associations, but only write under
// HKCU.
constexpr char CLASSES_ROOT_KEY[] = "Software\\Classes\\";

// although I don't know of any official documentation which mentions this
// location, uses it, so it isn't likely to change
constexpr char MIME_DATABASE_KEY[] = "MIME\\Database\\Content Type\\";

// this function replaces Microsoft %1 with Unix-like %s
static bool CanonicalizeParams(std::string& command)
{
    // transform it from '%1' to '%s' style format string (now also test for %L
    // as apparently MS started using it as well for the same purpose)

    // NB: we don't make any attempt to verify that the string is valid, i.e.
    //     doesn't contain %2, or second %1 or .... But we do make sure that we
    //     return a string with _exactly_ one '%s'!
    bool foundFilename = false;
    const size_t len = command.length();
    // FIXME: Use a lambda.
    for ( size_t n = 0; (n < len) && !foundFilename; n++ )
    {
        if ( command[n] == '%' &&
                (n + 1 < len) &&
                (command[n + 1] == '1' || command[n + 1] == 'L') )
        {
            // replace it with '%s'
            command[n + 1] = 's';

            foundFilename = true;
        }
    }

    if ( foundFilename )
    {
        // Some values also contain an addition %* expansion string which is
        // presumably supposed to be replaced with the names of the other files
        // accepted by the command. As we don't support more than one file
        // anyhow, simply ignore it.
        wx::utils::EraseSubstr(command, " %*");
    }

    return foundFilename;
}

void wxFileTypeImpl::Init(const std::string& strFileType, const std::string& ext)
{
    // VZ: does it? (FIXME)
    wxCHECK_RET( !ext.empty(), "needs an extension" );

    if ( ext[0u] != '.' ) {
        m_ext = '.';
    }
    m_ext += ext;

    m_strFileType = strFileType;
    if ( strFileType.empty() ) {
        m_strFileType = wx::utils::AfterFirst(m_ext, '.') + "_auto_file";
    }

    m_suppressNotify = false;
}

std::string wxFileTypeImpl::GetVerbPath(const std::string& verb) const
{
    return fmt::format("{}\\shell{}\\command", m_strFileType, verb);
}

size_t wxFileTypeImpl::GetAllCommands(std::vector<std::string> *verbs,
                                      std::vector<std::string> *commands,
                                      const wxFileType::MessageParameters& params) const
{
    wxCHECK_MSG( !m_ext.empty(), 0, "GetAllCommands() needs an extension" );

    if ( m_strFileType.empty() )
    {
        // get it from the registry
        wxFileTypeImpl *self = const_cast<wxFileTypeImpl *>(this);
        wxRegKey rkey(wxRegKey::HKCR, m_ext);
        if ( !rkey.Exists() || !rkey.QueryValue({}, self->m_strFileType) )
        {
            wxLogDebug("Can't get the filetype for extension '%s'.",
                       m_ext.c_str());

            return 0;
        }
    }

    // enum all subkeys of HKCR\filetype\shell
    size_t count = 0;
    wxRegKey rkey(wxRegKey::HKCR, m_strFileType  + "\\shell");
    long dummy;
    std::string verb;
    bool ok = rkey.GetFirstKey(verb, dummy);
    while ( ok )
    {
        std::string command = wxFileType::ExpandCommand(GetCommand(verb), params);

        // we want the open bverb to eb always the first

        if ( wx::utils::CmpNoCase(verb, "open") == 0 )
        {
            if ( verbs )
                verbs->insert(std::begin(*verbs), verb);
            if ( commands )
                commands->insert(std::begin(*commands), command);
        }
        else // anything else than "open"
        {
            if ( verbs )
                verbs->push_back(verb);
            if ( commands )
                commands->push_back(command);
        }

        count++;

        ok = rkey.GetNextKey(verb, dummy);
    }

    return count;
}

void wxFileTypeImpl::MSWNotifyShell()
{
    if (!m_suppressNotify)
        ::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST | SHCNF_FLUSHNOWAIT, nullptr, nullptr);
}

void wxFileTypeImpl::MSWSuppressNotifications(bool supress)
{
    m_suppressNotify = supress;
}

// ----------------------------------------------------------------------------
// modify the registry database
// ----------------------------------------------------------------------------

bool wxFileTypeImpl::EnsureExtKeyExists()
{
    wxRegKey rkey(wxRegKey::HKCU, CLASSES_ROOT_KEY + m_ext);
    if ( !rkey.Exists() )
    {
        if ( !rkey.Create() || !rkey.SetValue({}, m_strFileType) )
        {
            wxLogError(_("Failed to create registry entry for '%s' files."),
                       m_ext.c_str());
            return false;
        }
    }

    return true;
}

// ----------------------------------------------------------------------------
// get the command to use
// ----------------------------------------------------------------------------

// Helper wrapping AssocQueryString() Win32 function: returns the value of the
// given associated string for the specified extension (which may or not have
// the leading period).
//
// Returns empty string if the association is not found.
static
std::string wxAssocQueryString(ASSOCSTR assoc,
                            std::string ext,
                            const std::string& verb = {})
{
    DWORD dwSize = MAX_PATH;
    wchar_t bufOut[MAX_PATH] = { 0 };

    if ( ext.empty() || ext[0] != '.' )
        ext.insert(0, ".");

    HRESULT hr = ::AssocQueryStringW
                 (
                    wxASSOCF_NOTRUNCATE,// Fail if buffer is too small.
                    assoc,              // The association to retrieve.
                    boost::nowide::widen(ext).c_str(), // The extension to retrieve it for.
                    verb.empty() ? nullptr
                                 : boost::nowide::widen(verb).c_str(),
                    bufOut,             // The buffer for output value.
                    &dwSize             // And its size
                 );

    // Do not use SUCCEEDED() here as S_FALSE could, in principle, be returned
    // but would still be an error in this context.
    if ( hr != S_OK )
    {
        // The only really expected error here is that no association is
        // defined, anything else is not expected. The confusing thing is that
        // different errors are returned for this expected error under
        // different Windows versions: XP returns ERROR_FILE_NOT_FOUND while 7
        // returns ERROR_NO_ASSOCIATION. Just check for both to be sure.
        if ( hr != HRESULT_FROM_WIN32(ERROR_NO_ASSOCIATION) &&
                hr != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) )
        {
            wxLogApiError("AssocQueryString", hr);
        }

        return {};
    }

    return boost::nowide::narrow(bufOut);
}


std::string wxFileTypeImpl::GetCommand(const std::string& verb) const
{
    std::string command = wxAssocQueryString(ASSOCSTR_COMMAND, m_ext, verb);
    const bool foundFilename = CanonicalizeParams(command);

#if wxUSE_IPC
    std::string ddeCommand = wxAssocQueryString(ASSOCSTR_DDECOMMAND, m_ext);
    std::string ddeTopic = wxAssocQueryString(ASSOCSTR_DDETOPIC, m_ext);

    if ( !ddeCommand.empty() && !ddeTopic.empty() )
    {
        std::string ddeServer = wxAssocQueryString( ASSOCSTR_DDEAPPLICATION, m_ext );

        wx::utils::ReplaceAll(ddeCommand, "%1", "%s");

        if ( ddeTopic.empty() )
            ddeTopic = "System";

        // HACK: we use a special feature of wxExecute which exists
        //       just because we need it here: it will establish DDE
        //       conversation with the program it just launched
        command.insert(0, "WX_DDE#");
        command += fmt::format("#{}#{}#{}", ddeServer, ddeTopic, ddeCommand);
    }
    else
#endif // wxUSE_IPC
    if ( !foundFilename && !command.empty() )
    {
        // we didn't find any '%1' - the application doesn't know which
        // file to open (note that we only do it if there is no DDEExec
        // subkey)
        //
        // HACK: append the filename at the end, hope that it will do
        command += " %s";
    }

    return command;
}


std::string
wxFileTypeImpl::GetExpandedCommand(const std::string & verb,
                                   const wxFileType::MessageParameters& params) const
{
    std::string cmd = GetCommand(verb);

    // Some viewers don't define the "open" verb but do define "show" one, try
    // to use it as a fallback.
    if ( cmd.empty() && (verb == "open") )
        cmd = GetCommand("show");

    return wxFileType::ExpandCommand(cmd, params);
}

// ----------------------------------------------------------------------------
// getting other stuff
// ----------------------------------------------------------------------------

// TODO this function is half implemented
bool wxFileTypeImpl::GetExtensions(std::vector<std::string>& extensions)
{
    if ( m_ext.empty() ) {
        // the only way to get the list of extensions from the file type is to
        // scan through all extensions in the registry - too slow...
        return false;
    }
    else {
        extensions.clear();
        extensions.push_back(m_ext);

        // it's a lie too, we don't return _all_ extensions...
        return true;
    }
}

bool wxFileTypeImpl::GetMimeType(std::string* mimeType) const
{
    // suppress possible error messages
    wxLogNull nolog;
    wxRegKey key(wxRegKey::HKCR, m_ext);

    return key.Open(wxRegKey::Read) &&
                key.QueryValue("Content Type", *mimeType);
}

bool wxFileTypeImpl::GetMimeTypes(std::vector<std::string>& mimeTypes) const
{
    std::string s;

    if ( !GetMimeType(&s) )
    {
        return false;
    }

    mimeTypes.clear();
    mimeTypes.push_back(s);
    return true;
}


bool wxFileTypeImpl::GetIcon(wxIconLocation *iconLoc) const
{
    std::string strIcon = wxAssocQueryString(wxASSOCSTR_DEFAULTICON, m_ext);

    if ( !strIcon.empty() )
    {
        std::string strFullPath = wx::utils::BeforeLast(strIcon, ','),
        strIndex = wx::utils::AfterLast(strIcon, ',');

        // index may be omitted, in which case BeforeLast(',') is empty and
        // AfterLast(',') is the whole string
        if ( strFullPath.empty() ) {
            strFullPath = strIndex;
            strIndex = "0";
        }

        // if the path contains spaces, it can be enclosed in quotes but we
        // must not pass a filename in that format to any file system function,
        // so remove them here.
        if ( strFullPath.starts_with('"') && strFullPath.ends_with('"') )
            strFullPath = strFullPath.substr(1, strFullPath.length() - 2);

        if ( iconLoc )
        {
            iconLoc->SetFileName(wxExpandEnvVars(strFullPath));

            iconLoc->SetIndex(wxAtoi(strIndex));
        }

      return true;
    }

    // no such file type or no value or incorrect icon entry
    return false;
}

bool wxFileTypeImpl::GetDescription(std::string *desc) const
{
    // suppress possible error messages
    wxLogNull nolog;
    wxRegKey key(wxRegKey::HKCR, m_strFileType);

    if ( key.Open(wxRegKey::Read) ) {
        // it's the default value of the key
        if ( key.QueryValue({}, *desc) ) {
            return true;
        }
    }

    return false;
}

// helper function
wxFileType *
wxMimeTypesManagerImpl::CreateFileType(const std::string& filetype, const std::string& ext)
{
    wxFileType *fileType = new wxFileType;
    fileType->m_impl->Init(filetype, ext);
    return fileType;
}

// extension -> file type
wxFileType *
wxMimeTypesManagerImpl::GetFileTypeFromExtension(const std::string& ext)
{
    // add the leading point if necessary
    std::string str;
    if ( ext[0u] != '.' ) {
        str = '.';
    }
    str += ext;

    // suppress possible error messages
    wxLogNull nolog;

    bool knownExtension = false;

    std::string strFileType;
    wxRegKey key(wxRegKey::HKCR, str);
    if ( key.Open(wxRegKey::Read) ) {
        // it's the default value of the key
        if ( key.QueryValue({}, strFileType) ) {
            // create the new wxFileType object
            return CreateFileType(strFileType, ext);
        }
        else {
            // this extension doesn't have a filetype, but it's known to the
            // system and may be has some other useful keys (open command or
            // content-type), so still return a file type object for it
            knownExtension = true;
        }
    }

    if ( !knownExtension )
    {
        // unknown extension
        return nullptr;
    }

    return CreateFileType({}, ext);
}

// MIME type -> extension -> file type
wxFileType *
wxMimeTypesManagerImpl::GetFileTypeFromMimeType(const std::string& mimeType)
{
    std::string strKey = MIME_DATABASE_KEY;
    strKey += mimeType;

    // suppress possible error messages
    wxLogNull nolog;

    std::string ext;
    wxRegKey key(wxRegKey::HKCR, strKey);
    if ( key.Open(wxRegKey::Read) ) {
        if ( key.QueryValue("Extension", ext) ) {
            return GetFileTypeFromExtension(ext);
        }
    }

    // unknown MIME type
    return nullptr;
}

size_t wxMimeTypesManagerImpl::EnumAllFileTypes(std::vector<std::string>& mimetypes)
{
    // enumerate all keys under MIME_DATABASE_KEY
    wxRegKey key(wxRegKey::HKCR, MIME_DATABASE_KEY);

    std::string type;
    long cookie;
    bool cont = key.GetFirstKey(type, cookie);
    while ( cont )
    {
        mimetypes.push_back(type);

        cont = key.GetNextKey(type, cookie);
    }

    return mimetypes.size();
}

// ----------------------------------------------------------------------------
// create a new association
// ----------------------------------------------------------------------------

wxFileType *wxMimeTypesManagerImpl::Associate(const wxFileTypeInfo& ftInfo)
{
    wxCHECK_MSG( !ftInfo.GetExtensions().empty(), nullptr,
                 "Associate() needs extension" );

    bool ok;
    size_t iExtCount = 0;
    std::string filetype;
    std::string extWithDot;

    std::string ext = ftInfo.GetExtensions()[iExtCount];

    wxCHECK_MSG( !ext.empty(), nullptr,
                 "Associate() needs non empty extension" );

    if ( ext[0u] != '.' )
        extWithDot = '.';
    extWithDot += ext;

    // start by setting the entries under ".ext"
    // default is filetype; content type is mimetype
    const std::string& filetypeOrig = ftInfo.GetShortDesc();

    wxRegKey key(wxRegKey::HKCU, CLASSES_ROOT_KEY + extWithDot);
    if ( !key.Exists() )
    {
        // create the mapping from the extension to the filetype
        ok = key.Create();
        if ( ok )
        {

            if ( filetypeOrig.empty() )
            {
                // make it up from the extension
                filetype += extWithDot.substr(1) + "_file";
            }
            else
            {
                // just use the provided one
                filetype = filetypeOrig;
            }

            key.SetValue({}, filetype);
        }
    }
    else
    {
        // key already exists, maybe we want to change it ??
        if (!filetypeOrig.empty())
        {
            filetype = filetypeOrig;
            key.SetValue({}, filetype);
        }
        else
        {
            key.QueryValue({}, filetype);
        }
    }

    // now set a mimetypeif we have it, but ignore it if none
    const std::string& mimetype = ftInfo.GetMimeType();
    if ( !mimetype.empty() )
    {
        // set the MIME type
        ok = key.SetValue("Content Type", mimetype);

        if ( ok )
        {
            // create the MIME key
            std::string strKey = MIME_DATABASE_KEY;
            strKey += mimetype;
            wxRegKey keyMIME(wxRegKey::HKCU, CLASSES_ROOT_KEY + strKey);
            ok = keyMIME.Create();

            if ( ok )
            {
                // and provide a back link to the extension
                keyMIME.SetValue("Extension", extWithDot);
            }
        }
    }


    // now make other extensions have the same filetype

    for (iExtCount=1; iExtCount < ftInfo.GetExtensionsCount(); iExtCount++ )
    {
        ext = ftInfo.GetExtensions()[iExtCount];
        if ( ext[0u] != '.' )
           extWithDot = '.';
        extWithDot += ext;

        wxRegKey key2(wxRegKey::HKCU, CLASSES_ROOT_KEY + extWithDot);
        if ( !key2.Exists() )
            key2.Create();
        key2.SetValue({}, filetype);

        // now set any mimetypes we may have, but ignore it if none
        const std::string& mimetype2 = ftInfo.GetMimeType();
        if ( !mimetype2.empty() )
        {
            // set the MIME type
            ok = key2.SetValue("Content Type", mimetype2);

            if ( ok )
            {
                // create the MIME key
                std::string strKey = MIME_DATABASE_KEY;
                strKey += mimetype2;
                wxRegKey keyMIME(wxRegKey::HKCU, CLASSES_ROOT_KEY + strKey);
                ok = keyMIME.Create();

                if ( ok )
                {
                    // and provide a back link to the extension
                    keyMIME.SetValue("Extension", extWithDot);
                }
            }
        }

    } // end of for loop; all extensions now point to .ext\Default

    // create the filetype key itself (it will be empty for now, but
    // SetCommand(), SetDefaultIcon() &c will use it later)
    wxRegKey keyFT(wxRegKey::HKCU, CLASSES_ROOT_KEY + filetype);
    keyFT.Create();

    wxFileType *ft = CreateFileType(filetype, extWithDot);

    if (ft)
    {
        ft->m_impl->MSWSuppressNotifications(true);
        if (! ftInfo.GetOpenCommand ().empty() ) ft->SetCommand (ftInfo.GetOpenCommand (), "open" );
        if (! ftInfo.GetPrintCommand().empty() ) ft->SetCommand (ftInfo.GetPrintCommand(), "print" );
        // chris: I don't like the ->m_impl-> here FIX this ??
        if (! ftInfo.GetDescription ().empty() ) ft->m_impl->SetDescription (ftInfo.GetDescription ()) ;
        if (! ftInfo.GetIconFile().empty() ) ft->SetDefaultIcon (ftInfo.GetIconFile(), ftInfo.GetIconIndex() );

        ft->m_impl->MSWSuppressNotifications(false);
        ft->m_impl->MSWNotifyShell();
    }

    return ft;
}

bool wxFileTypeImpl::SetCommand(const std::string& cmd,
                                const std::string& verb,
                                bool WXUNUSED(overwriteprompt))
{
    wxCHECK_MSG( !m_ext.empty() && !verb.empty(), false,
                 "SetCommand() needs an extension and a verb" );

    if ( !EnsureExtKeyExists() )
        return false;

    wxRegKey rkey(wxRegKey::HKCU, CLASSES_ROOT_KEY + GetVerbPath(verb));

    // TODO:
    // 1. translate '%s' to '%1' instead of always adding it
    // 2. create DDEExec value if needed (undo GetCommand)
    bool result = rkey.Create() && rkey.SetValue({}, cmd + " \"%1\"" );

    if ( result )
        MSWNotifyShell();

    return result;
}

bool wxFileTypeImpl::SetDefaultIcon(const std::string& cmd, int index)
{
    wxCHECK_MSG( !m_ext.empty(), false, "SetDefaultIcon() needs extension" );
    wxCHECK_MSG( !m_strFileType.empty(), false, "File key not found" );
//    the next line fails on a SMBshare, I think because it is case mangled
//    wxCHECK_MSG( !wxFileExists(cmd), false, "Icon file not found." );

    if ( !EnsureExtKeyExists() )
        return false;

    wxRegKey rkey(wxRegKey::HKCU,
                  CLASSES_ROOT_KEY + m_strFileType + "\\DefaultIcon");

    bool result = rkey.Create() &&
           rkey.SetValue({},
                         fmt::format("{:s},{:d}", cmd.c_str(), index));

    if ( result )
        MSWNotifyShell();

    return result;
}

bool wxFileTypeImpl::SetDescription (const std::string& desc)
{
    wxCHECK_MSG( !m_strFileType.empty(), false, "File key not found" );
    wxCHECK_MSG( !desc.empty(), false, "No file description supplied" );

    if ( !EnsureExtKeyExists() )
        return false;

    wxRegKey rkey(wxRegKey::HKCU, CLASSES_ROOT_KEY + m_strFileType );

    return rkey.Create() &&
           rkey.SetValue({}, desc);
}

// ----------------------------------------------------------------------------
// remove file association
// ----------------------------------------------------------------------------

bool wxFileTypeImpl::Unassociate()
{
    MSWSuppressNotifications(true);
    bool result = true;
    if ( !RemoveOpenCommand() )
        result = false;
    if ( !RemoveDefaultIcon() )
        result = false;
    if ( !RemoveMimeType() )
        result = false;
    if ( !RemoveDescription() )
        result = false;

    MSWSuppressNotifications(false);
    MSWNotifyShell();

    return result;
}

bool wxFileTypeImpl::RemoveOpenCommand()
{
   return RemoveCommand("open");
}

bool wxFileTypeImpl::RemoveCommand(const std::string& verb)
{
    wxCHECK_MSG( !m_ext.empty() && !verb.empty(), false,
                 "RemoveCommand() needs an extension and a verb" );

    wxRegKey rkey(wxRegKey::HKCU, CLASSES_ROOT_KEY + GetVerbPath(verb));

    // if the key already doesn't exist, it's a success
    const bool result = !rkey.Exists() || rkey.DeleteSelf();

    if ( result )
        MSWNotifyShell();

    return result;
}

bool wxFileTypeImpl::RemoveMimeType()
{
    wxCHECK_MSG( !m_ext.empty(), false, "RemoveMimeType() needs extension" );

    wxRegKey rkey(wxRegKey::HKCU, CLASSES_ROOT_KEY + m_ext);
    return !rkey.Exists() || rkey.DeleteSelf();
}

bool wxFileTypeImpl::RemoveDefaultIcon()
{
    wxCHECK_MSG( !m_ext.empty(), false,
                 "RemoveDefaultIcon() needs extension" );

    wxRegKey rkey (wxRegKey::HKCU,
                   CLASSES_ROOT_KEY + m_strFileType  + "\\DefaultIcon");
    const bool result = !rkey.Exists() || rkey.DeleteSelf();

    if ( result )
        MSWNotifyShell();

    return result;
}

bool wxFileTypeImpl::RemoveDescription()
{
    wxCHECK_MSG( !m_ext.empty(), false,
                 "RemoveDescription() needs extension" );

    wxRegKey rkey (wxRegKey::HKCU, CLASSES_ROOT_KEY + m_strFileType );
    return !rkey.Exists() || rkey.DeleteSelf();
}

#endif // wxUSE_MIMETYPE
