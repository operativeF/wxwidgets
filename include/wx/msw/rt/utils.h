///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/rt/utils.h
// Purpose:     Windows Runtime Objects helper functions and objects
// Author:      Tobias Taschner
// Created:     2015-09-05
// Copyright:   (c) 2015 wxWidgets development team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef   _WX_MSW_RTUTILS_H
#define   _WX_MSW_RTUTILS_H

#if wxUSE_WINRT

#include "wx/string.h"

#include <winstring.h>

namespace wxWinRT
{

bool IsAvailable();

bool Initialize();

void Uninitialize();

HRESULT GetActivationFactory(const wxString& activatableClassId, REFIID iid, void ** factory);

// RAII class initializing WinRT in its ctor and undoing it in its dtor.
class Initializer
{
public:
    Initializer()
        : m_ok(Initialize())
    {
    }

    bool IsOk() const
    {
        return m_ok;
    }

    ~Initializer()
    {
        if (m_ok)
            Uninitialize();
    }

private:
    const bool m_ok;
};

// Simple class to convert wxString to HSTRING
// This just wraps a reference to the wxString object,
// which needs a life time greater than the TempStringRef object
class TempStringRef
{
public:
    HSTRING Get() const { return m_hstring; }

    operator HSTRING() const { return m_hstring; };

    TempStringRef(const wxString& str);

private:
    HSTRING             m_hstring;
    HSTRING_HEADER      m_header;

    TempStringRef(const TempStringRef&) = delete;
	TempStringRef& operator=(const TempStringRef&) = delete;
};

} // namespace wxWinRT

#endif // wxUSE_WINRT

#endif // _WX_MSW_RTUTILS_H
