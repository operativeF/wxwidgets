///////////////////////////////////////////////////////////////////////////////
// Name:        wx/snglinst.h
// Purpose:     wxSingleInstanceChecker can be used to restrict the number of
//              simultaneously running copies of a program to one
// Author:      Vadim Zeitlin
// Modified by:
// Created:     08.06.01
// Copyright:   (c) 2001 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SNGLINST_H_
#define _WX_SNGLINST_H_

#if wxUSE_SNGLINST_CHECKER

#include "wx/app.h"

// ----------------------------------------------------------------------------
// wxSingleInstanceChecker
// ----------------------------------------------------------------------------

class wxSingleInstanceChecker
{
public:
    // default ctor, use Create() after it
    wxSingleInstanceChecker() = default;

    // like Create() but no error checking (dangerous!)
    wxSingleInstanceChecker(const std::string& name,
                            const std::string& path = {})
    {
        Create(name, path);
    }

    wxSingleInstanceChecker& operator=(wxSingleInstanceChecker&&) = delete;

    // notice that calling Create() is optional now, if you don't do it before
    // calling IsAnotherRunning(), CreateDefault() is used automatically
    //
    // name it is used as the mutex name under Win32 and the lock file name
    // under Unix so it should be as unique as possible and must be non-empty
    //
    // path is optional and is ignored under Win32 and used as the directory to
    // create the lock file in under Unix (default is wxGetHomeDir())
    //
    // returns false if initialization failed, it doesn't mean that another
    // instance is running - use IsAnotherRunning() to check it
    [[maybe_unused]] bool Create(const std::string& name, const std::string& path = {});

    // use the default name, which is a combination of wxTheApp->GetAppName()
    // and wxGetUserId() for mutex/lock file
    //
    // this is called implicitly by IsAnotherRunning() if the checker hadn't
    // been created until then
    bool CreateDefault()
    {
        wxCHECK_MSG( wxTheApp, false, "must have application instance" );
        return Create(wxTheApp->GetAppName() + '-' + wxGetUserId());
    }

    // is another copy of this program already running?
    bool IsAnotherRunning() const
    {
        if ( !m_impl )
        {
            if ( !const_cast<wxSingleInstanceChecker *>(this)->CreateDefault() )
            {
                // if creation failed, return false as it's better to not
                // prevent this instance from starting up if there is an error
                return false;
            }
        }

        return DoIsAnotherRunning();
    }

    // dtor is not virtual, this class is not meant to be used polymorphically
    ~wxSingleInstanceChecker();

private:    
    // do check if another instance is running, called only if m_impl != NULL
    bool DoIsAnotherRunning() const;

    // the implementation details (platform specific)
    class wxSingleInstanceCheckerImpl *m_impl{nullptr};
};

#endif // wxUSE_SNGLINST_CHECKER

#endif // _WX_SNGLINST_H_
