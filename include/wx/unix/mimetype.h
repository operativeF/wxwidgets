/////////////////////////////////////////////////////////////////////////////
// Name:        wx/unix/mimetype.h
// Purpose:     classes and functions to manage MIME types
// Author:      Vadim Zeitlin
// Modified by:
// Created:     23.09.98
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence (part of wxExtra library)
/////////////////////////////////////////////////////////////////////////////

#ifndef _MIMETYPE_IMPL_H
#define _MIMETYPE_IMPL_H

#include "wx/mimetype.h"

#if wxUSE_MIMETYPE

class wxMimeTypeCommands;

using wxMimeCommandsArray = std::vector<wxMimeTypeCommands*>;

// this is the real wxMimeTypesManager for Unix
class wxMimeTypesManagerImpl
{
public:
    wxMimeTypesManagerImpl();
    virtual ~wxMimeTypesManagerImpl();

    // load all data into memory - done when it is needed for the first time
    void Initialize(int mailcapStyles = wxMAILCAP_ALL,
                    const wxString& extraDir = {});

    // and delete the data here
    void ClearData();

    // implement containing class functions
    wxFileType *GetFileTypeFromExtension(const wxString& ext);
    wxFileType *GetFileTypeFromMimeType(const wxString& mimeType);

    size_t EnumAllFileTypes(wxArrayString& mimetypes);

    void AddFallback(const wxFileTypeInfo& filetype);

    // add information about the given mimetype
    void AddMimeTypeInfo(const wxString& mimetype,
                         const wxString& extensions,
                         const wxString& description);
    void AddMailcapInfo(const wxString& strType,
                        const wxString& strOpenCmd,
                        const wxString& strPrintCmd,
                        const wxString& strTest,
                        const wxString& strDesc);

    // add a new record to the user .mailcap/.mime.types files
    wxFileType *Associate(const wxFileTypeInfo& ftInfo);
    // remove association
    bool Unassociate(wxFileType *ft);

        // get the string containing space separated extensions for the given
        // file type
    wxString GetExtension(size_t index) { return m_aExtensions[index]; }

protected:
    void InitIfNeeded();

    wxArrayString m_aTypes,         // MIME types
                  m_aDescriptions,  // descriptions (just some text)
                  m_aExtensions,    // space separated list of extensions
                  m_aIcons;         // Icon filenames

    // verb=command pairs for this file type
    wxMimeCommandsArray m_aEntries;

    // are we initialized?
    bool m_initialized;

    wxString GetCommand(const wxString &verb, size_t nIndex) const;

    // Read XDG *.desktop file
    void LoadXDGApp(const wxString& filename);
    // Scan XDG directory
    void LoadXDGAppsFilesFromDir(const wxString& dirname);

    // Load XDG globs files
    void LoadXDGGlobs(const wxString& filename);

    // functions used to do associations
    virtual int AddToMimeData(const wxString& strType,
                      const wxString& strIcon,
                      wxMimeTypeCommands *entry,
                      const wxArrayString& strExtensions,
                      const wxString& strDesc,
                      bool replaceExisting = true);
    virtual bool DoAssociation(const wxString& strType,
                       const wxString& strIcon,
                       wxMimeTypeCommands *entry,
                       const wxArrayString& strExtensions,
                       const wxString& strDesc);

    virtual wxString GetIconFromMimeType(const wxString& mime);

    // give it access to m_aXXX variables
    friend class wxFileTypeImpl;
};

class wxFileTypeImpl
{
public:
    // initialization functions
    // this is used to construct a list of mimetypes which match;
    // if built with GetFileTypeFromMimetype index 0 has the exact match and
    // index 1 the type / * match
    // if built with GetFileTypeFromExtension, index 0 has the mimetype for
    // the first extension found, index 1 for the second and so on

    void Init(wxMimeTypesManagerImpl *manager, size_t index)
        { m_manager = manager; m_index.Add(index); }

    bool GetExtensions(wxArrayString& extensions);
    bool GetMimeType(std::string* mimeType) const
        { *mimeType = m_manager->m_aTypes[m_index[0]].ToStdString(); return true; }
    bool GetMimeTypes(wxArrayString& mimeTypes) const;
    bool GetIcon(wxIconLocation *iconLoc) const;

    bool GetDescription(wxString *desc) const
        { *desc = m_manager->m_aDescriptions[m_index[0]]; return true; }

    bool GetOpenCommand(wxString *openCmd,
                        const wxFileType::MessageParameters& params) const
    {
        *openCmd = GetExpandedCommand("open", params);
        return (! openCmd -> IsEmpty() );
    }

    bool GetPrintCommand(wxString *printCmd,
                         const wxFileType::MessageParameters& params) const
    {
        *printCmd = GetExpandedCommand("print", params);
        return (! printCmd -> IsEmpty() );
    }

        // return the number of commands defined for this file type, 0 if none
    size_t GetAllCommands(wxArrayString *verbs, wxArrayString *commands,
                          const wxFileType::MessageParameters& params) const;


    // remove the record for this file type
    // probably a mistake to come here, use wxMimeTypesManager.Unassociate (ft) instead
    bool Unassociate(wxFileType *ft)
    {
        return m_manager->Unassociate(ft);
    }

    // set an arbitrary command, ask confirmation if it already exists and
    // overwriteprompt is TRUE
    bool SetCommand(const wxString& cmd, const wxString& verb, bool overwriteprompt = true);
    bool SetDefaultIcon(const wxString& strIcon = {}, int index = 0);

    wxString
    GetExpandedCommand(const wxString & verb,
                       const wxFileType::MessageParameters& params) const;
private:
    wxMimeTypesManagerImpl *m_manager;
    wxArrayInt              m_index; // in the wxMimeTypesManagerImpl arrays
};

#endif // wxUSE_MIMETYPE

#endif // _MIMETYPE_IMPL_H


