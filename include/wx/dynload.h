/////////////////////////////////////////////////////////////////////////////
// Name:         wx/dynload.h
// Purpose:      Dynamic loading framework
// Author:       Ron Lee, David Falkinder, Vadim Zeitlin and a cast of 1000's
//               (derived in part from dynlib.cpp (c) 1998 Guilhem Lavaux)
// Modified by:
// Created:      03/12/01
// Copyright:    (c) 2001 Ron Lee <ron@debian.org>
// Licence:      wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DYNAMICLOADER_H__
#define _WX_DYNAMICLOADER_H__

#if wxUSE_DYNAMIC_LOADER

#include "wx/dynlib.h"
#include "wx/module.h"

import <unordered_map>;

class wxPluginLibrary;

using wxDLManifest = std::unordered_map<std::string, wxPluginLibrary*>;

using wxDLImports = wxDLManifest;

// ---------------------------------------------------------------------------
// wxPluginLibrary
// ---------------------------------------------------------------------------

// NOTE: Do not attempt to use a base class pointer to this class.
//       wxDL is not virtual and we deliberately hide some of it's
//       methods here.
//
//       Unless you know exactly why you need to, you probably shouldn't
//       instantiate this class directly anyway, use wxPluginManager
//       instead.

class wxPluginLibrary : public wxDynamicLibrary
{
public:

    inline static wxDLImports* ms_classes{nullptr};  // Static hash of all imported classes.

    wxPluginLibrary( const std::string &libname, unsigned int flags = wxDL_DEFAULT );
    ~wxPluginLibrary();

    wxPluginLibrary& operator=(wxPluginLibrary&&) = delete;

    wxPluginLibrary  *RefLib();
    bool              UnrefLib();

        // These two are called by the PluginSentinel on (PLUGGABLE) object
        // creation/destruction.  There is usually no reason for the user to
        // call them directly.  We have to separate this from the link count,
        // since the two are not interchangeable.

        // FIXME: for even better debugging PluginSentinel should register
        //        the name of the class created too, then we can state
        //        exactly which object was not destroyed which may be
        //        difficult to find otherwise.  Also this code should
        //        probably only be active in DEBUG mode, but let's just
        //        get it right first.

    void  RefObj() { ++m_objcount; }
    void  UnrefObj()
    {
        wxASSERT_MSG( m_objcount > 0, "Too many objects deleted??" );
        --m_objcount;
    }

        // Override/hide some base class methods

    bool  IsLoaded() const { return m_linkcount > 0; }
    void  Unload() { UnrefLib(); }

private:

    // These pointers may be NULL but if they are not, then m_ourLast follows
    // m_ourFirst in the linked list, i.e. can be found by calling GetNext() a
    // sufficient number of times.
    const wxClassInfo    *m_ourFirst; // first class info in this plugin
    const wxClassInfo    *m_ourLast;  // ..and the last one

    size_t          m_linkcount{1};    // Ref count of library link calls
    size_t          m_objcount{0};     // ..and (pluggable) object instantiations.
    wxModuleList    m_wxmodules;    // any wxModules that we initialised.

    void    UpdateClasses();        // Update ms_classes
    void    RestoreClasses();       // Removes this library from ms_classes
    void    RegisterModules();      // Init any wxModules in the lib.
    void    UnregisterModules();    // Cleanup any wxModules we installed.
};


class wxPluginManager
{
public:

        // Static accessors.

    static wxPluginLibrary    *wxLoadLibrary( const std::string &libname,
                                              unsigned int flags = wxDL_DEFAULT );
    static bool                UnloadLibrary(const std::string &libname);

        // Instance methods.

    wxPluginManager()  = default;
    wxPluginManager(const std::string &libname, unsigned int flags = wxDL_DEFAULT)
    {
        Load(libname, flags);
    }
    ~wxPluginManager() { if ( IsLoaded() ) Unload(); }

    // We could allow this class to be copied if we really
    // wanted to, but not without modification.
    wxPluginManager& operator=(wxPluginManager&&) = delete;

    bool   Load(const std::string &libname, unsigned int flags = wxDL_DEFAULT);
    void   Unload();

    bool   IsLoaded() const { return m_entry && m_entry->IsLoaded(); }
    void* GetSymbol(const std::string& symbol, bool* success = nullptr)
    {
        return m_entry->GetSymbol( symbol, success );
    }

    static void CreateManifest() { ms_manifest = new wxDLManifest(wxKEY_STRING); }
    static void ClearManifest() { delete ms_manifest; ms_manifest = nullptr; }

private:
    // return the pointer to the entry for the library with given name in
    // ms_manifest or NULL if none
    static wxPluginLibrary *FindByName(const std::string& name)
    {
        const wxDLManifest::iterator i = ms_manifest->find(name);

        return i == ms_manifest->end() ? NULL : i->second;
    }

    inline static wxDLManifest* ms_manifest{nullptr};  // Static hash of loaded libs.
    wxPluginLibrary*     m_entry{nullptr};      // Cache our entry in the manifest.
};


#endif  // wxUSE_DYNAMIC_LOADER
#endif  // _WX_DYNAMICLOADER_H__

