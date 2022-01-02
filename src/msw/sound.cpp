/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/sound.cpp
// Purpose:     wxSound
// Author:      Julian Smart
// Modified by: 2005-07-29: Vadim Zeitlin: redesign
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_SOUND

#include "wx/msw/private.h"

#include "wx/sound.h"

#include <mmsystem.h>

import WX.WinDef;

import <string>;

// ----------------------------------------------------------------------------
// wxSoundData
// ----------------------------------------------------------------------------

// ABC for different sound data representations
struct wxSoundData
{
    virtual ~wxSoundData() = default;

    // return true if we had been successfully initialized
    virtual bool IsOk() const = 0;

    // get the flag corresponding to our content for PlaySound()
    virtual WXDWORD GetSoundFlag() const = 0;

    // get the data to be passed to PlaySound()
    virtual LPCWSTR GetSoundData() const = 0;
};

// class for in-memory sound data
class wxSoundDataMemory : public wxSoundData
{
public:
    // we copy the data
    wxSoundDataMemory(size_t size, const void* buf);

    wxSoundDataMemory(const wxSoundDataMemory&) = delete;
	wxSoundDataMemory& operator=(const wxSoundDataMemory&) = delete;

    void *GetPtr() const { return m_waveDataPtr; }

    bool IsOk() const override { return GetPtr() != nullptr; }
    WXDWORD GetSoundFlag() const override { return SND_MEMORY; }
    LPCWSTR GetSoundData() const override { return (LPCWSTR)GetPtr(); }

private:
    GlobalPtr m_waveData;
    GlobalPtrLock m_waveDataPtr;
};

// class for sound files and resources
class wxSoundDataFile : public wxSoundData
{
public:
    wxSoundDataFile(const std::string& filename, bool isResource);

    wxSoundDataFile(const wxSoundDataFile&) = delete;
	wxSoundDataFile& operator=(const wxSoundDataFile&) = delete;

    bool IsOk() const override { return !m_name.empty(); }
    WXDWORD GetSoundFlag() const override
    {
        return m_isResource ? SND_RESOURCE : SND_FILENAME;
    }
    LPCWSTR GetSoundData() const override { return m_name.c_str(); }

private:
    const wxString m_name;
    const bool m_isResource;
};

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxSoundData-derived classes
// ----------------------------------------------------------------------------

wxSoundDataMemory::wxSoundDataMemory(size_t size, const void* buf)
                 : m_waveData(size),
                   m_waveDataPtr(m_waveData)
{
    if ( IsOk() )
        ::CopyMemory(m_waveDataPtr, buf, size);
}

wxSoundDataFile::wxSoundDataFile(const std::string& filename, bool isResource)
               : m_name(filename),
                 m_isResource(isResource)
{
    // check for file/resource existence?
}

// ----------------------------------------------------------------------------
// wxSound
// ----------------------------------------------------------------------------

wxSound::wxSound(const std::string& filename, bool isResource)
{
    Create(filename, isResource);
}

wxSound::wxSound(size_t size, const void* data)
{
    Create(size, data);
}

wxSound::~wxSound()
{
    Free();
}

void wxSound::Free()
{
    wxDELETE(m_data);
}

bool wxSound::CheckCreatedOk()
{
    if ( m_data && !m_data->IsOk() )
        Free();

    return m_data != nullptr;
}

bool wxSound::Create(const std::string& filename, bool isResource)
{
    Free();

    m_data = new wxSoundDataFile(filename, isResource);

    return CheckCreatedOk();
}

bool wxSound::Create(size_t size, const void* data)
{
    Free();

    m_data = new wxSoundDataMemory(size, data);

    return CheckCreatedOk();
}

bool wxSound::DoPlay(unsigned flags) const
{
    if ( !IsOk() || !m_data->IsOk() )
        return false;

    WXDWORD flagsMSW = m_data->GetSoundFlag();
    WXHMODULE hmod = flagsMSW == SND_RESOURCE ? wxGetInstance() : nullptr;

    // we don't want replacement default sound
    flagsMSW |= SND_NODEFAULT;

    // NB: wxSOUND_SYNC is 0, don't test for it
    flagsMSW |= (flags & wxSOUND_ASYNC) ? SND_ASYNC : SND_SYNC;
    if ( flags & wxSOUND_LOOP )
    {
        // looping only works with async flag
        flagsMSW |= SND_LOOP | SND_ASYNC;
    }

    return ::PlaySoundW(m_data->GetSoundData(), hmod, flagsMSW) != FALSE;
}

/* static */
void wxSound::Stop()
{
    ::PlaySoundW(nullptr, nullptr, 0);
}

#endif // wxUSE_SOUND

