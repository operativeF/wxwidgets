/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/module.cpp
// Purpose:     Modules initialization/destruction
// Author:      Wolfram Gloger/adapted by Guilhem Lavaux
// Modified by:
// Created:     04/11/98
// Copyright:   (c) Wolfram Gloger and Guilhem Lavaux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/module.h"
#include "wx/intl.h"
#include "wx/log.h"

import <algorithm>;

constexpr char TRACE_MODULE[] = "module";

wxIMPLEMENT_ABSTRACT_CLASS(wxModule, wxObject)

void wxModule::RegisterModule(wxModule* module)
{
    module->m_state = State_Registered;
    ms_modules.push_back(module);
}

void wxModule::UnregisterModule(wxModule* module)
{
    std::erase(ms_modules, module);

    delete module;
}

// Collect up all module-derived classes, create an instance of each,
// and register them.
void wxModule::RegisterModules()
{
    for (wxClassInfo::const_iterator it  = wxClassInfo::begin_classinfo(),
                                     end = wxClassInfo::end_classinfo();
         it != end; ++it)
    {
        const wxClassInfo* classInfo = *it;

        if ( classInfo->IsKindOf(wxCLASSINFO(wxModule)) &&
             (classInfo != (& (wxModule::ms_classInfo))) )
        {
            wxLogTrace(TRACE_MODULE, "Registering module %s",
                       classInfo->wxGetClassName());
            wxModule* module = (wxModule *)classInfo->CreateObject();
            wxModule::RegisterModule(module);
        }
    }
}

bool wxModule::DoInitializeModule(wxModule *module,
                                  wxModuleList &initializedModules)
{
    if ( module->m_state == State_Initializing )
    {
        wxLogError(_("Circular dependency involving module \"%s\" detected."),
                   module->wxGetClassInfo()->wxGetClassName());
        return false;
    }

    module->m_state = State_Initializing;

    // translate named dependencies to the normal ones first
    if ( !module->ResolveNamedDependencies() )
      return false;

    const wxArrayClassInfo& dependencies = module->m_dependencies;

    // satisfy module dependencies by loading them before the current module
    for ( unsigned int i = 0; i < dependencies.size(); ++i )
    {
        wxClassInfo * cinfo = dependencies[i];

        // Check if the module is already initialized
        wxModuleList::const_iterator it;
        for ( it = initializedModules.begin();
              it != initializedModules.end();
              ++it )
        {
            if ( (*it)->wxGetClassInfo() == cinfo )
                break;
        }

        if ( it != initializedModules.end() )
        {
            // this dependency is already initialized, nothing to do
            continue;
        }

        // find the module in the registered modules list
        for ( it = ms_modules.begin(); it != ms_modules.end(); ++it )
        {
            wxModule *moduleDep = *it;
            if ( moduleDep->wxGetClassInfo() == cinfo )
            {
                if ( !DoInitializeModule(moduleDep, initializedModules ) )
                {
                    // failed to initialize a dependency, so fail this one too
                    return false;
                }

                break;
            }
        }

        if ( it == ms_modules.end() )
        {
            wxLogError(_("Dependency \"%s\" of module \"%s\" doesn't exist."),
                       cinfo->wxGetClassName(),
                       module->wxGetClassInfo()->wxGetClassName());
            return false;
        }
    }

    if ( !module->Init() )
    {
        wxLogError(_("Module \"%s\" initialization failed"),
                   module->wxGetClassInfo()->wxGetClassName());
        return false;
    }

    wxLogTrace(TRACE_MODULE, "Module \"%s\" initialized",
               module->wxGetClassInfo()->wxGetClassName());

    module->m_state = State_Initialized;
    initializedModules.push_back(module);

    return true;
}

// Initialize user-defined modules
bool wxModule::InitializeModules()
{
    wxModuleList initializedModules;

    for ( wxModuleList::const_iterator it = ms_modules.begin();
          it != ms_modules.end();
          ++it )
    {
        wxModule *module = *it;

        // the module could have been already initialized as dependency of
        // another one
        if ( module->m_state == State_Registered )
        {
            if ( !DoInitializeModule( module, initializedModules ) )
            {
                // failed to initialize all modules, so clean up the already
                // initialized ones
                DoCleanUpModules(initializedModules);

                return false;
            }
        }
    }

    // remember the real initialisation order
    ms_modules = initializedModules;

    ms_areInitialized = true;

    return true;
}

void wxModule::CleanUpModules()
{
    DoCleanUpModules(ms_modules);

    ms_areInitialized = false;
}

// Clean up all currently initialized modules
void wxModule::DoCleanUpModules(const wxModuleList& modules)
{
    // cleanup user-defined modules in the reverse order compared to their
    // initialization -- this ensures that dependencies are respected
    for ( wxModuleList::const_reverse_iterator rit = modules.rbegin();
          rit != modules.rend();
          ++rit )
    {
        wxLogTrace(TRACE_MODULE, "Cleanup module %s",
                   (*rit)->wxGetClassInfo()->wxGetClassName());

        wxModule * module = *rit;

        wxASSERT_MSG( module->m_state == State_Initialized,
                        "not initialized module being cleaned up" );

        module->Exit();
        module->m_state = State_Registered;
    }

    // clear all modules, even the non-initialized ones
    for ( wxModuleList::const_iterator it = ms_modules.begin();
          it != ms_modules.end();
          ++it )
    {
        delete *it;
    }

    ms_modules.clear();
}

bool wxModule::ResolveNamedDependencies()
{
    // first resolve required dependencies
    for ( size_t i = 0; i < m_namedDependencies.size(); ++i )
    {
        wxClassInfo *info = wxClassInfo::FindClass(m_namedDependencies[i]);

        if ( !info )
        {
            // required dependency not found
            return false;
        }

        // add it even if it is not derived from wxModule because
        // DoInitializeModule() will make sure a module with the same class
        // info exists and fail if it doesn't
        m_dependencies.push_back(info);
    }

    return true;
}
