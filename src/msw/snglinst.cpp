///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/snglinst.cpp
// Purpose:     implements wxSingleInstanceChecker class for Win32 using
//              named mutexes
// Author:      Vadim Zeitlin
// Modified by:
// Created:     08.06.01
// Copyright:   (c) 2001 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_SNGLINST_CHECKER

#include "wx/msw/private.h"

#include "wx/log.h"
#include "wx/snglinst.h"

// ----------------------------------------------------------------------------
// wxSingleInstanceCheckerImpl: the real implementation class
// ----------------------------------------------------------------------------

class wxSingleInstanceCheckerImpl
{
public:
	wxSingleInstanceCheckerImpl& operator=(wxSingleInstanceCheckerImpl&&) = delete;

    bool Create(const std::string& name)
    {
        boost::nowide::wstackstring stackName{name.c_str()};
        m_hMutex = ::CreateMutexW(nullptr, FALSE, stackName.get());
        if ( !m_hMutex )
        {
            wxLogLastError("CreateMutex");

            return false;
        }

        // mutex was either created or opened - see what really happened
        m_wasOpened = ::GetLastError() == ERROR_ALREADY_EXISTS;

        return true;
    }

    bool WasOpened() const
    {
        wxCHECK_MSG( m_hMutex, false,
                     "can't be called if mutex creation failed" );

        return m_wasOpened;
    }

    ~wxSingleInstanceCheckerImpl()
    {
        if ( m_hMutex )
        {
            if ( !::CloseHandle(m_hMutex) )
            {
                wxLogLastError("CloseHandle(mutex)");
            }
        }
    }

private:
    // the mutex handle, may be NULL
    HANDLE m_hMutex{ nullptr };

    // the result of the CreateMutex() call
    bool m_wasOpened{false};
};

// ============================================================================
// wxSingleInstanceChecker implementation
// ============================================================================

bool wxSingleInstanceChecker::Create(const std::string& name,
                                     [[maybe_unused]] const std::string& path)
{
    wxASSERT_MSG( !m_impl,
                  "calling wxSingleInstanceChecker::Create() twice?" );

    // creating unnamed mutex doesn't have the same semantics!
    wxASSERT_MSG( !name.empty(), "mutex name can't be empty" );

    m_impl = new wxSingleInstanceCheckerImpl;

    return m_impl->Create(name);
}

bool wxSingleInstanceChecker::DoIsAnotherRunning() const
{
    wxCHECK_MSG( m_impl, false, "must call Create() first" );

    // if the mutex had been opened, another instance is running - otherwise we
    // would have created it
    return m_impl->WasOpened();
}

wxSingleInstanceChecker::~wxSingleInstanceChecker()
{
    delete m_impl;
}

#endif // wxUSE_SNGLINST_CHECKER
