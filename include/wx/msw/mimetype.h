/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/mimetype.h
// Purpose:     classes and functions to manage MIME types
// Author:      Vadim Zeitlin
// Modified by:
// Created:     23.09.98
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWidgets licence (part of base library)
/////////////////////////////////////////////////////////////////////////////

#ifndef _MIMETYPE_IMPL_H
#define _MIMETYPE_IMPL_H

#include "wx/defs.h"

#if wxUSE_MIMETYPE

#include "wx/mimetype.h"

// ----------------------------------------------------------------------------
// wxFileTypeImpl is the MSW version of wxFileType, this is a private class
// and is never used directly by the application
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_BASE wxFileTypeImpl
{
public:
    // ctor
    wxFileTypeImpl() = default;

    // one of these Init() function must be called (ctor can't take any
    // arguments because it's common)

        // initialize us with our file type name and extension - in this case
        // we will read all other data from the registry
    void Init(const std::string& strFileType, const std::string& ext);

    // implement accessor functions
    bool GetExtensions(std::vector<std::string>& extensions);
    bool GetMimeType(std::string *mimeType) const;
    bool GetMimeTypes(std::vector<std::string>& mimeTypes) const;
    bool GetIcon(wxIconLocation *iconLoc) const;
    bool GetDescription(std::string *desc) const;
    bool GetOpenCommand(std::string *openCmd,
                        const wxFileType::MessageParameters& params) const
    {
        *openCmd = GetExpandedCommand("open", params);
        return !openCmd->empty();
    }

    bool GetPrintCommand(std::string *printCmd,
                         const wxFileType::MessageParameters& params) const
    {
        *printCmd = GetExpandedCommand("print", params);
        return !printCmd->empty();
    }

    size_t GetAllCommands(std::vector<std::string>* verbs, std::vector<std::string>* commands,
                          const wxFileType::MessageParameters& params) const;

    void Unassociate();

    // set an arbitrary command, ask confirmation if it already exists and
    // overwriteprompt is true
    bool SetCommand(const std::string& cmd,
                    const std::string& verb,
                    bool overwriteprompt = true);

    bool SetDefaultIcon(const std::string& cmd = {}, int index = 0);

    // this is called  by Associate
    bool SetDescription (const std::string& desc);

    // This is called by all our own methods modifying the registry to let the
    // Windows Shell know about the changes.
    //
    // It is also called from Associate() and Unassociate() which suppress the
    // internally generated notifications using the method below, which is why
    // it has to be public.
    void MSWNotifyShell();

    // Call before/after performing several registry changes in a row to
    // temporarily suppress multiple notifications that would be generated for
    // them and generate a single one at the end using MSWNotifyShell()
    // explicitly.
    void MSWSuppressNotifications(bool suppress);

    std::string
    GetExpandedCommand(const std::string& verb,
                       const wxFileType::MessageParameters& params) const;
private:
    // helper function: reads the command corresponding to the specified verb
    // from the registry (returns an empty string if not found)
    std::string GetCommand(const std::string& verb) const;

    // get the registry path for the given verb
    std::string GetVerbPath(const std::string& verb) const;

    // check that the registry key for our extension exists, create it if it
    // doesn't, return false if this failed
    bool EnsureExtKeyExists();

    std::string m_strFileType,         // may be empty
             m_ext;
    bool m_suppressNotify;

    // these methods are not publicly accessible (as wxMimeTypesManager
    // doesn't know about them), and should only be called by Unassociate

    bool RemoveOpenCommand();
    bool RemoveCommand(const std::string& verb);
    void RemoveMimeType();
    void RemoveDefaultIcon();
    void RemoveDescription();
};

class WXDLLIMPEXP_BASE wxMimeTypesManagerImpl
{
public:
    // nothing to do here, we don't load any data but just go and fetch it from
    // the registry when asked for
    wxMimeTypesManagerImpl() = default;

    // implement containing class functions
    wxFileType *GetFileTypeFromExtension(const std::string& ext);
    wxFileType *GetOrAllocateFileTypeFromExtension(const std::string& ext);
    wxFileType *GetFileTypeFromMimeType(const std::string& mimeType);

    size_t EnumAllFileTypes(std::vector<std::string>& mimetypes);

    // create a new filetype association
    wxFileType *Associate(const wxFileTypeInfo& ftInfo);

    // create a new filetype with the given name and extension
    wxFileType *CreateFileType(const std::string& filetype, const std::string& ext);
};

#endif // wxUSE_MIMETYPE

#endif
  //_MIMETYPE_IMPL_H

