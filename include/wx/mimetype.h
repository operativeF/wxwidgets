/////////////////////////////////////////////////////////////////////////////
// Name:        wx/mimetype.h
// Purpose:     classes and functions to manage MIME types
// Author:      Vadim Zeitlin
// Modified by:
//  Chris Elliott (biol75@york.ac.uk) 5 Dec 00: write support for Win32
// Created:     23.09.98
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence (part of wxExtra library)
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MIMETYPE_H_
#define _WX_MIMETYPE_H_

// ----------------------------------------------------------------------------
// headers and such
// ----------------------------------------------------------------------------

#if wxUSE_MIMETYPE

// the things we really need
#include "wx/string.h"

import Utils.Strings;

import <vector>;

// fwd decls
class wxIconLocation;
class wxFileTypeImpl;
class wxMimeTypesManagerImpl;

// these constants define the MIME information source under UNIX and are used
// by wxMimeTypesManager::Initialize()
enum wxMailcapStyle
{
    wxMAILCAP_STANDARD = 1,
    wxMAILCAP_NETSCAPE = 2,
    wxMAILCAP_KDE = 4,
    wxMAILCAP_GNOME = 8,

    wxMAILCAP_ALL = 15
};

/*
    TODO: would it be more convenient to have this class?

class wxMimeType : public wxString
{
public:
    // all string ctors here

    wxString GetType() const { return BeforeFirst(wxT('/')); }
    wxString GetSubType() const { return AfterFirst(wxT('/')); }

    void SetSubType(const wxString& subtype)
    {
        *this = GetType() + wxT('/') + subtype;
    }

    bool Matches(const wxMimeType& wildcard)
    {
        // implement using wxMimeTypesManager::IsOfType()
    }
};

*/

// wxMimeTypeCommands stores the verbs defined for the given MIME type with
// their values
class wxMimeTypeCommands
{
public:
    wxMimeTypeCommands() = default;

    wxMimeTypeCommands(const std::vector<std::string>& verbs,
                       const std::vector<std::string>& commands)
        : m_verbs(verbs),
          m_commands(commands)
    {
    }

    // add a new verb with the command or replace the old value
    void AddOrReplaceVerb(const std::string& verb, const std::string& cmd);
    void Add(const std::string& s)
    {
        m_verbs.push_back(wx::utils::BeforeFirst(s, '='));
        m_commands.push_back(wx::utils::AfterFirst(s, '='));
    }

    // access the commands
    size_t GetCount() const
    {
        return m_verbs.size();
    }
    
    // FIXME: Unsafe
    const std::string& GetVerb(size_t n) const { return m_verbs[n]; }
    const std::string& GetCmd(size_t n) const { return m_commands[n]; }

    // TODO: string_view
    bool HasVerb(const std::string& verb) const
    {
        return std::find(m_verbs.cbegin(), m_verbs.cend(), verb) != m_verbs.cend();
    }

    // returns empty string and wxNOT_FOUND in idx if no such verb
    std::string GetCommandForVerb(const std::string& verb, size_t *idx = nullptr) const;

    // get a "verb=command" string
    std::string GetVerbCmd(size_t n) const;

private:
    std::vector<std::string> m_verbs;
    std::vector<std::string> m_commands;
};

// ----------------------------------------------------------------------------
// wxFileTypeInfo: static container of information accessed via wxFileType.
//
// This class is used with wxMimeTypesManager::AddFallbacks() and Associate()
// ----------------------------------------------------------------------------

class wxFileTypeInfo
{
private:
    void DoVarArgInit(const wxString& mimeType,
                      const wxString& openCmd,
                      const wxString& printCmd,
                      const wxString& desc,
                      va_list argptr);

    void VarArgInit(const wxString *mimeType,
                    const wxString *openCmd,
                    const wxString *printCmd,
                    const wxString *desc,
                    // the other parameters form a NULL terminated list of
                    // extensions
                    ...);

public:
    // NB: This is a helper to get implicit conversion of variadic ctor's
    //     fixed arguments into something that can be passed to VarArgInit().
    //     Do not use, it's used by the ctor only.
    struct CtorString
    {
#ifndef wxNO_IMPLICIT_WXSTRING_ENCODING
        CtorString(const char *str) : m_str(str) {}
#endif
        CtorString(const wchar_t *str) : m_str(str) {}
        CtorString(const wxString& str) : m_str(str) {}
        CtorString(const wxCStrData& str) : m_str(str) {}
#ifndef wxNO_IMPLICIT_WXSTRING_ENCODING
        CtorString(const wxScopedCharBuffer& str) : m_str(str) {}
#endif
        CtorString(const wxScopedWCharBuffer& str) : m_str(str) {}

        operator const wxString*() const { return &m_str; }

        wxString m_str;
    };

    // ctors

    // Ctor specifying just the MIME type (which is mandatory), the other
    // fields can be set later if needed.
    wxFileTypeInfo(const wxString& mimeType)
        : m_mimeType(mimeType)
    {
    }

    // Ctor allowing to specify the values of all fields at once:
    //
    // wxFileTypeInfo(const wxString& mimeType,
    //               const wxString& openCmd,
    //               const wxString& printCmd,
    //               const wxString& desc,
    //               // the other parameters form a list of extensions for this
    //               // file type and should be terminated with wxNullPtr (not
    //               // just NULL!)
    //               ...);
    WX_DEFINE_VARARG_FUNC_CTOR(wxFileTypeInfo,
                               4, (const CtorString&,
                                   const CtorString&,
                                   const CtorString&,
                                   const CtorString&),
                               VarArgInit, VarArgInit)

        // the array elements correspond to the parameters of the ctor above in
        // the same order
    wxFileTypeInfo(const std::vector<std::string>& sArray);

        // invalid item - use this to terminate the array passed to
        // wxMimeTypesManager::AddFallbacks
    wxFileTypeInfo() = default;

    // test if this object can be used
    bool IsValid() const { return !m_mimeType.empty(); }

    // setters
        // set the open/print commands
    void SetOpenCommand(const wxString& command) { m_openCmd = command; }
    void SetPrintCommand(const wxString& command) { m_printCmd = command; }

        // set the description
    void SetDescription(const wxString& desc) { m_desc = desc; }

        // add another extension corresponding to this file type
    void AddExtension(const wxString& ext) { m_exts.push_back(ext); }

        // set the icon info
    void SetIcon(const wxString& iconFile, int iconIndex = 0)
    {
        m_iconFile = iconFile;
        m_iconIndex = iconIndex;
    }
        // set the short desc
    void SetShortDesc(const wxString& shortDesc) { m_shortDesc = shortDesc; }

        // get the MIME type
    const std::string& GetMimeType() const { return m_mimeType; }
        // get the open command
    const wxString& GetOpenCommand() const { return m_openCmd; }
        // get the print command
    const wxString& GetPrintCommand() const { return m_printCmd; }
        // get the short description (only used under Win32 so far)
    const wxString& GetShortDesc() const { return m_shortDesc; }
        // get the long, user visible description
    const wxString& GetDescription() const { return m_desc; }
        // get the array of all extensions
    const std::vector<std::string>& GetExtensions() const { return m_exts; }
    size_t GetExtensionsCount() const {return m_exts.size(); }
        // get the icon info
    const wxString& GetIconFile() const { return m_iconFile; }
    int GetIconIndex() const { return m_iconIndex; }

private:
    std::string m_mimeType;    // the MIME type in "type/subtype" form
    wxString m_openCmd,     // command to use for opening the file (%s allowed)
             m_printCmd,    // command to use for printing the file (%s allowed)
             m_shortDesc,   // a short string used in the registry
             m_desc;        // a free form description of this file type

    // icon stuff
    wxString m_iconFile;      // the file containing the icon
    int      m_iconIndex{};   // icon index in this file

    std::vector<std::string> m_exts;   // the extensions which are mapped on this filetype


#if 0 // TODO
    // the additional (except "open" and "print") command names and values
    std::vector<wxString> m_commandNames;
    std::vector<wxString> m_commandValues;
#endif // 0
};

using wxArrayFileTypeInfo = std::vector<wxFileTypeInfo>;

// ----------------------------------------------------------------------------
// wxFileType: gives access to all information about the files of given type.
//
// This class holds information about a given "file type". File type is the
// same as MIME type under Unix, but under Windows it corresponds more to an
// extension than to MIME type (in fact, several extensions may correspond to a
// file type). This object may be created in many different ways and depending
// on how it was created some fields may be unknown so the return value of all
// the accessors *must* be checked!
// ----------------------------------------------------------------------------

class wxFileType
{
friend class wxMimeTypesManagerImpl;  // it has access to m_impl

public:
    // An object of this class must be passed to Get{Open|Print}Command. The
    // default implementation is trivial and doesn't know anything at all about
    // parameters, only filename and MIME type are used (so it's probably ok for
    // Windows where %{param} is not used anyhow)
    class MessageParameters
    {
    public:
        MessageParameters() = default;
        MessageParameters(const std::string& filename,
                          const std::string& mimetype = {})
            : m_filename(filename), m_mimetype(mimetype) { }

        // accessors (called by GetOpenCommand)
            // filename
        const std::string& GetFileName() const { return m_filename; }
            // mime type
        const std::string& GetMimeType() const { return m_mimetype; }

        // override this function in derived class
        virtual std::string GetParamValue([[maybe_unused]] const std::string& name) const
            { return {}; }

        // virtual dtor as in any base class
        virtual ~MessageParameters() = default;

    protected:
        std::string m_filename;
        std::string m_mimetype;
    };

    // ctor from static data
    wxFileType(const wxFileTypeInfo& ftInfo);

    // accessors: all of them return true if the corresponding information
    // could be retrieved/found, false otherwise (and in this case all [out]
    // parameters are unchanged)
        // return the MIME type for this file type
    bool GetMimeType(std::string *mimeType) const;
    bool GetMimeTypes(std::vector<std::string>& mimeTypes) const;
        // fill passed in array with all extensions associated with this file
        // type
    bool GetExtensions(std::vector<std::string>& extensions);
        // get the icon corresponding to this file type and of the given size
    bool GetIcon(wxIconLocation *iconloc) const;
    bool GetIcon(wxIconLocation *iconloc,
                 const MessageParameters& params) const;
        // get a brief file type description ("*.txt" => "text document")
    bool GetDescription(std::string *desc) const;

    // get the command to be used to open/print the given file.
        // get the command to execute the file of given type
    bool GetOpenCommand(std::string *openCmd,
                        const MessageParameters& params) const;
        // a simpler to use version of GetOpenCommand() -- it only takes the
        // filename and returns an empty string on failure
    std::string GetOpenCommand(const std::string& filename) const;
        // get the command to print the file of given type
    bool GetPrintCommand(std::string *printCmd,
                         const MessageParameters& params) const;


        // return the number of commands defined for this file type, 0 if none
    size_t GetAllCommands(std::vector<std::string> *verbs, std::vector<std::string> *commands,
                          const wxFileType::MessageParameters& params) const;

    // set an arbitrary command, ask confirmation if it already exists and
    // overwriteprompt is true
    bool SetCommand(const std::string& cmd, const std::string& verb,
        bool overwriteprompt = true);

    bool SetDefaultIcon(const std::string& cmd = {}, int index = 0);


    // remove the association for this filetype from the system MIME database:
    // notice that it will only work if the association is defined in the user
    // file/registry part, we will never modify the system-wide settings
    bool Unassociate();

    // operations
        // expand a string in the format of GetOpenCommand (which may contain
        // '%s' and '%t' format specifiers for the file name and mime type
        // and %{param} constructions).
    static std::string ExpandCommand(const std::string& command,
                                  const MessageParameters& params);

    // dtor (not virtual, shouldn't be derived from)
    ~wxFileType();

    std::string
    GetExpandedCommand(const std::string& verb,
                       const wxFileType::MessageParameters& params) const;
private:
    // default ctor is private because the user code never creates us
    wxFileType();

    // no copy ctor/assignment operator
    wxFileType(const wxFileType&);
    wxFileType& operator=(const wxFileType&);

    // the static container of wxFileType data: if it's not NULL, it means that
    // this object is used as fallback only
    const wxFileTypeInfo *m_info{nullptr};

    // the object which implements the real stuff like reading and writing
    // to/from system MIME database
    wxFileTypeImpl *m_impl{nullptr};
};

//----------------------------------------------------------------------------
// wxMimeTypesManagerFactory
//----------------------------------------------------------------------------

class wxMimeTypesManagerFactory
{
public:
    virtual ~wxMimeTypesManagerFactory() = default;

    virtual wxMimeTypesManagerImpl *CreateMimeTypesManagerImpl();

    static void Set( wxMimeTypesManagerFactory *factory );
    static wxMimeTypesManagerFactory *Get();

private:
    static wxMimeTypesManagerFactory *m_factory;
};

// ----------------------------------------------------------------------------
// wxMimeTypesManager: interface to system MIME database.
//
// This class accesses the information about all known MIME types and allows
// the application to retrieve information (including how to handle data of
// given type) about them.
// ----------------------------------------------------------------------------

class wxMimeTypesManager
{
public:
    // static helper functions
    // -----------------------

        // check if the given MIME type is the same as the other one: the
        // second argument may contain wildcards ('*'), but not the first. If
        // the types are equal or if the mimeType matches wildcard the function
        // returns true, otherwise it returns false
    static bool IsOfType(const std::string& mimeType, const std::string& wildcard);

    wxMimeTypesManager() = default;
    ~wxMimeTypesManager();

    wxMimeTypesManager(const wxMimeTypesManager&) = delete;
    wxMimeTypesManager& operator=(const wxMimeTypesManager&) = delete;

    // NB: the following 2 functions are for Unix only and don't do anything
    //     elsewhere

    // loads data from standard files according to the mailcap styles
    // specified: this is a bitwise OR of wxMailcapStyle values
    //
    // use the extraDir parameter if you want to look for files in another
    // directory
    void Initialize(int mailcapStyle = wxMAILCAP_ALL,
                    const std::string& extraDir = {});

    // and this function clears all the data from the manager
    void ClearData();

    // Database lookup: all functions return a pointer to wxFileType object
    // whose methods may be used to query it for the information you're
    // interested in. If the return value is !NULL, caller is responsible for
    // deleting it.
        // get file type from file extension
    wxFileType *GetFileTypeFromExtension(const std::string& ext);
        // get file type from MIME type (in format <category>/<format>)
    wxFileType *GetFileTypeFromMimeType(const std::string& mimeType);

    // enumerate all known MIME types
    //
    // returns the number of retrieved file types
    size_t EnumAllFileTypes(std::vector<std::string>& mimetypes);

    // these functions can be used to provide default values for some of the
    // MIME types inside the program itself
    //
    // The filetypes array should be terminated by either NULL entry or an
    // invalid wxFileTypeInfo (i.e. the one created with default ctor)
    void AddFallbacks(const wxFileTypeInfo *filetypes);
    void AddFallback(const wxFileTypeInfo& ft) { m_fallbacks.push_back(ft); }

    // create or remove associations

        // create a new association using the fields of wxFileTypeInfo (at least
        // the MIME type and the extension should be set)
        // if the other fields are empty, the existing values should be left alone
    wxFileType *Associate(const wxFileTypeInfo& ftInfo);

        // undo Associate()
    bool Unassociate(wxFileType *ft) ;

private:
    // the fallback info which is used if the information is not found in the
    // real system database
    wxArrayFileTypeInfo m_fallbacks;

    // the object working with the system MIME database
    wxMimeTypesManagerImpl *m_impl{nullptr};

    // if m_impl is NULL, create one
    void EnsureImpl();

    friend class wxMimeTypeCmnModule;
};


// ----------------------------------------------------------------------------
// global variables
// ----------------------------------------------------------------------------

// FIXME: make private when modularized.
// private object
static inline wxMimeTypesManager gs_mimeTypesManager;

// and public pointer
inline wxMimeTypesManager *wxTheMimeTypesManager = &gs_mimeTypesManager;

#endif // wxUSE_MIMETYPE

#endif
  //_WX_MIMETYPE_H_
