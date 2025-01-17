///////////////////////////////////////////////////////////////////////////////
// Name:        msw/ole/safearray.cpp
// Purpose:     Implementation of wxSafeArrayBase class.
// Author:      PB
// Created:     2012-09-23
// Copyright:   (c) 2012 wxWidgets development team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_OLE && wxUSE_VARIANT

#include "wx/variant.h"

#include "wx/msw/ole/safearray.h"

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxSafeArrayBase
// ----------------------------------------------------------------------------

void wxSafeArrayBase::Destroy()
{
    if ( m_array )
    {
        Unlock();
        HRESULT hr = SafeArrayDestroy(m_array);
        if ( FAILED(hr) )
        {
            wxLogApiError("SafeArrayDestroy()", hr);
        }
        m_array = nullptr;
    }
}

SAFEARRAY* wxSafeArrayBase::Detach()
{
    wxCHECK_MSG( m_array, nullptr, "Uninitialized safe array" );

    Unlock();
    SAFEARRAY* array = m_array;
    m_array = nullptr;
    return array;
}

size_t wxSafeArrayBase::GetDim() const
{
    wxASSERT( m_array );

    return SafeArrayGetDim(m_array);
}

bool wxSafeArrayBase::GetLBound(size_t dim, long& bound) const
{
    wxCHECK_MSG( m_array, false, "Uninitialized safe array" );
    wxCHECK_MSG( dim > 0, false, "Invalid dimension index" );

    HRESULT hr = SafeArrayGetLBound(m_array, dim, (LONG*)&bound);
    if ( FAILED(hr) )
    {
        wxLogApiError("SafeArrayGetLBound()", hr);
        return false;
    }
    return true;
}

bool wxSafeArrayBase::GetUBound(size_t dim, long& bound) const
{
    wxCHECK_MSG( m_array, false, "Uninitialized safe array" );
    wxCHECK_MSG( dim > 0, false, "Invalid dimension index" );

    HRESULT hr = SafeArrayGetUBound(m_array, dim, (LONG*)&bound);
    if ( FAILED(hr) )
    {
        wxLogApiError("SafeArrayGetUBound()", hr);
        return false;
    }
    return true;
}

size_t wxSafeArrayBase::GetCount(size_t dim) const
{
    long lBound, uBound;

    if ( GetLBound(dim, lBound) && GetUBound(dim, uBound) )
        return uBound - lBound + 1;
    return 0;
}

bool wxSafeArrayBase::Lock()
{
    wxCHECK_MSG( m_array, false, "Uninitialized safe array" );

    HRESULT hr = SafeArrayLock(m_array);
    if ( FAILED(hr) )
    {
        wxLogApiError("SafeArrayLock()", hr);
        return false;
    }
    return true;
}

bool wxSafeArrayBase::Unlock()
{
    wxCHECK_MSG( m_array, false, "Uninitialized safe array" );

    HRESULT hr = SafeArrayUnlock(m_array);
    if ( FAILED(hr) )
    {
        wxLogApiError("SafeArrayUnlock()", hr);
        return false;
    }
    return true;
}

#endif  // wxUSE_OLE && wxUSE_VARIANT
