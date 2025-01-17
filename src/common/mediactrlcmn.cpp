/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/mediactrlcmn.cpp
// Purpose:     wxMediaCtrl common code
// Author:      Ryan Norton <wxprojects@comcast.net>
// Modified by:
// Created:     11/07/04
// Copyright:   (c) Ryan Norton
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// TODO: Platform specific backend defaults?

#if wxUSE_MEDIACTRL

#include "wx/hash.h"
#include "wx/log.h"
#include "wx/mediactrl.h"

import WX.File.Flags;

//===========================================================================
//
// Implementation
//
//===========================================================================

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RTTI and Event implementations
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

wxIMPLEMENT_CLASS(wxMediaCtrl, wxControl);
wxDEFINE_EVENT( wxEVT_MEDIA_STATECHANGED, wxMediaEvent );
wxDEFINE_EVENT( wxEVT_MEDIA_PLAY, wxMediaEvent );
wxDEFINE_EVENT( wxEVT_MEDIA_PAUSE, wxMediaEvent );
wxIMPLEMENT_CLASS(wxMediaBackend, wxObject);
wxDEFINE_EVENT( wxEVT_MEDIA_FINISHED, wxMediaEvent );
wxDEFINE_EVENT( wxEVT_MEDIA_LOADED, wxMediaEvent );
wxDEFINE_EVENT( wxEVT_MEDIA_STOP, wxMediaEvent );

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  wxMediaCtrl
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// wxMediaCtrl::Create (file version)
// wxMediaCtrl::Create (URL version)
//
// Searches for a backend that is installed on the system (backends
// starting with lower characters in the alphabet are given priority),
// and creates the control from it
//
// This searches by searching the global RTTI hashtable, class by class,
// attempting to call CreateControl on each one found that is a derivative
// of wxMediaBackend - if it succeeded Create returns true, otherwise
// it keeps iterating through the hashmap.
//---------------------------------------------------------------------------
bool wxMediaCtrl::Create(wxWindow* parent, wxWindowID id,
                const wxString& fileName,
                const wxPoint& pos,
                const wxSize& size,
                unsigned int style,
                const wxString& szBackend,
                const wxValidator& validator,
                const wxString& name)
{
    if(!szBackend.empty())
    {
        wxClassInfo* pClassInfo = wxClassInfo::FindClass(szBackend);

        if(!pClassInfo || !DoCreate(pClassInfo, parent, id,
                                    pos, size, style, validator, name))
        {
            m_imp = nullptr;
            return false;
        }

        if (!fileName.empty())
        {
            if (!Load(fileName))
            {
                wxDELETE(m_imp);
                return false;
            }
        }

        SetInitialSize(size);
        return true;
    }
    else
    {
        wxClassInfo::const_iterator it = wxClassInfo::begin_classinfo();

        const wxClassInfo* classInfo;

        while((classInfo = NextBackend(&it)) != nullptr)
        {
            if(!DoCreate(classInfo, parent, id,
                         pos, size, style, validator, name))
                continue;

            if (!fileName.empty())
            {
                if (Load(fileName))
                {
                    SetInitialSize(size);
                    return true;
                }
                else
                    delete m_imp;
            }
            else
            {
                SetInitialSize(size);
                return true;
            }
        }

        m_imp = nullptr;
        return false;
    }
}

bool wxMediaCtrl::Create(wxWindow* parent, wxWindowID id,
                         const wxURI& location,
                         const wxPoint& pos,
                         const wxSize& size,
                         unsigned int style,
                         const wxString& szBackend,
                         const wxValidator& validator,
                         const wxString& name)
{
    if(!szBackend.empty())
    {
        wxClassInfo* pClassInfo = wxClassInfo::FindClass(szBackend);
        if(!pClassInfo || !DoCreate(pClassInfo, parent, id,
                                    pos, size, style, validator, name))
        {
            m_imp = nullptr;
            return false;
        }

        if (!Load(location))
        {
            wxDELETE(m_imp);
            return false;
        }

        SetInitialSize(size);
        return true;
    }
    else
    {
        wxClassInfo::const_iterator it  = wxClassInfo::begin_classinfo();

        const wxClassInfo* classInfo;

        while((classInfo = NextBackend(&it)) != nullptr)
        {
            if(!DoCreate(classInfo, parent, id,
                         pos, size, style, validator, name))
                continue;

            if (Load(location))
            {
                SetInitialSize(size);
                return true;
            }
            else
                delete m_imp;
        }

        m_imp = nullptr;
        return false;
    }
}

//---------------------------------------------------------------------------
// wxMediaCtrl::DoCreate
//
// Attempts to create the control from a backend
//---------------------------------------------------------------------------
bool wxMediaCtrl::DoCreate(const wxClassInfo* classInfo,
                            wxWindow* parent, wxWindowID id,
                            const wxPoint& pos,
                            const wxSize& size,
                            unsigned int style,
                            const wxValidator& validator,
                            const wxString& name)
{
    m_imp = (wxMediaBackend*)classInfo->CreateObject();

    if( m_imp->CreateControl(this, parent, id, pos, size,
                             style, validator, name) )
    {
        return true;
    }

    delete m_imp;
    return false;
}

//---------------------------------------------------------------------------
// wxMediaCtrl::NextBackend (static)
//
//
// Search through the RTTI hashmap one at a
// time, attempting to create each derivative
// of wxMediaBackend
//
//
// STL isn't compatible with and will have a compilation error
// on a wxNode, however, wxHashTable::compatibility_iterator is
// incompatible with the old 2.4 stable version - but since
// we're in 2.5+ only we don't need to worry about the new version
//---------------------------------------------------------------------------
const wxClassInfo* wxMediaCtrl::NextBackend(wxClassInfo::const_iterator* it)
{
    for ( wxClassInfo::const_iterator end = wxClassInfo::end_classinfo();
          *it != end; ++(*it) )
    {
        const wxClassInfo* classInfo = **it;
        if ( classInfo->IsKindOf(wxCLASSINFO(wxMediaBackend))  &&
             classInfo != wxCLASSINFO(wxMediaBackend) )
        {
            ++(*it);
            return classInfo;
        }
    }

    //
    // Nope - couldn't successfully find one... fail
    //
    return nullptr;
}


//---------------------------------------------------------------------------
// wxMediaCtrl Destructor
//
// Free up the backend if it exists
//---------------------------------------------------------------------------
wxMediaCtrl::~wxMediaCtrl()
{
    delete m_imp;
}

//---------------------------------------------------------------------------
// wxMediaCtrl::Load (file version)
// wxMediaCtrl::Load (URL version)
// wxMediaCtrl::Load (URL & Proxy version)
// wxMediaCtrl::Load (wxInputStream version)
//
// Here we call load of the backend - keeping
// track of whether it was successful or not - which
// will determine which later method calls work
//---------------------------------------------------------------------------
bool wxMediaCtrl::Load(const wxString& fileName)
{
    if(m_imp)
        return (m_bLoaded = m_imp->Load(fileName));
    return false;
}

bool wxMediaCtrl::Load(const wxURI& location)
{
    if(m_imp)
        return (m_bLoaded = m_imp->Load(location));
    return false;
}

bool wxMediaCtrl::Load(const wxURI& location, const wxURI& proxy)
{
    if(m_imp)
        return (m_bLoaded = m_imp->Load(location, proxy));
    return false;
}

//---------------------------------------------------------------------------
// wxMediaCtrl::Play
// wxMediaCtrl::Pause
// wxMediaCtrl::Stop
// wxMediaCtrl::GetPlaybackRate
// wxMediaCtrl::SetPlaybackRate
// wxMediaCtrl::Seek --> SetPosition
// wxMediaCtrl::Tell --> GetPosition
// wxMediaCtrl::Length --> GetDuration
// wxMediaCtrl::GetState
// wxMediaCtrl::DoGetBestSize
// wxMediaCtrl::SetVolume
// wxMediaCtrl::GetVolume
// wxMediaCtrl::ShowInterface
// wxMediaCtrl::GetDownloadProgress
// wxMediaCtrl::GetDownloadTotal
//
// 1) Check to see whether the backend exists and is loading
// 2) Call the backend's version of the method, returning success
//    if the backend's version succeeds
//---------------------------------------------------------------------------
bool wxMediaCtrl::Play()
{
    if(m_imp && m_bLoaded)
        return m_imp->Play();
    return false;
}

bool wxMediaCtrl::Pause()
{
    if(m_imp && m_bLoaded)
        return m_imp->Pause();
    return false;
}

bool wxMediaCtrl::Stop()
{
    if(m_imp && m_bLoaded)
        return m_imp->Stop();
    return false;
}

double wxMediaCtrl::GetPlaybackRate()
{
    if(m_imp && m_bLoaded)
        return m_imp->GetPlaybackRate();
    return 0;
}

bool wxMediaCtrl::SetPlaybackRate(double dRate)
{
    if(m_imp && m_bLoaded)
        return m_imp->SetPlaybackRate(dRate);
    return false;
}

wxFileOffset wxMediaCtrl::Seek(wxFileOffset where, wxSeekMode mode)
{
    wxFileOffset offset;

    switch (mode)
    {
    case wxSeekMode::FromStart:
        offset = where;
        break;
    case wxSeekMode::FromEnd:
        offset = Length() - where;
        break;
//    case wxSeekMode::FromCurrent:
    default:
        offset = Tell() + where;
        break;
    }

    if(m_imp && m_bLoaded && m_imp->SetPosition(offset))
        return offset;
    return wxInvalidOffset;
}

wxFileOffset wxMediaCtrl::Tell()
{
    if(m_imp && m_bLoaded)
        return (wxFileOffset) m_imp->GetPosition().ToLong();
    return wxInvalidOffset;
}

wxFileOffset wxMediaCtrl::Length()
{
    if(m_imp && m_bLoaded)
        return (wxFileOffset) m_imp->GetDuration().ToLong();
    return wxInvalidOffset;
}

wxMediaState wxMediaCtrl::GetState()
{
    if(m_imp && m_bLoaded)
        return m_imp->GetState();
    return wxMediaState::Stopped;
}

wxSize wxMediaCtrl::DoGetBestSize() const
{
    if(m_imp)
        return m_imp->GetVideoSize();
    return {0, 0};
}

double wxMediaCtrl::GetVolume()
{
    if(m_imp && m_bLoaded)
        return m_imp->GetVolume();
    return 0.0;
}

bool wxMediaCtrl::SetVolume(double dVolume)
{
    if(m_imp && m_bLoaded)
        return m_imp->SetVolume(dVolume);
    return false;
}

bool wxMediaCtrl::ShowPlayerControls(wxMediaCtrlPlayerControls flags)
{
    if(m_imp)
        return m_imp->ShowPlayerControls(flags);
    return false;
}

wxFileOffset wxMediaCtrl::GetDownloadProgress()
{
    if(m_imp && m_bLoaded)
        return (wxFileOffset) m_imp->GetDownloadProgress().ToLong();
    return wxInvalidOffset;
}

wxFileOffset wxMediaCtrl::GetDownloadTotal()
{
    if(m_imp && m_bLoaded)
        return (wxFileOffset) m_imp->GetDownloadTotal().ToLong();
    return wxInvalidOffset;
}

//---------------------------------------------------------------------------
// wxMediaCtrl::DoMoveWindow
//
// 1) Call parent's version so that our control's window moves where
//    it's supposed to
// 2) If the backend exists and is loaded, move the video
//    of the media to where our control's window is now located
//---------------------------------------------------------------------------
void wxMediaCtrl::DoMoveWindow(wxRect boundary)
{
    wxControl::DoMoveWindow(boundary);

    if(m_imp)
        m_imp->Move(boundary);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  wxMediaBackendCommonBase
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void wxMediaBackendCommonBase::NotifyMovieSizeChanged()
{
    // our best size changed after opening a new file
    m_ctrl->InvalidateBestSize();
    m_ctrl->SetSize(m_ctrl->GetSize());

    // if the parent of the control has a sizer ask it to refresh our size
    wxWindow * const parent = m_ctrl->GetParent();
    if ( parent->GetSizer() )
    {
        m_ctrl->GetParent()->Layout();
        m_ctrl->GetParent()->Refresh();
        m_ctrl->GetParent()->Update();
    }
}

void wxMediaBackendCommonBase::NotifyMovieLoaded()
{
    NotifyMovieSizeChanged();

    // notify about movie being fully loaded
    QueueEvent(wxEVT_MEDIA_LOADED);
}

bool wxMediaBackendCommonBase::SendStopEvent()
{
    wxMediaEvent theEvent(wxEVT_MEDIA_STOP, m_ctrl->GetId());

    return !m_ctrl->GetEventHandler()->ProcessEvent(theEvent) || theEvent.IsAllowed();
}

void wxMediaBackendCommonBase::QueueEvent(wxEventType evtType)
{
    wxMediaEvent theEvent(evtType, m_ctrl->GetId());
    m_ctrl->GetEventHandler()->AddPendingEvent(theEvent);
}

void wxMediaBackendCommonBase::QueuePlayEvent()
{
    QueueEvent(wxEVT_MEDIA_STATECHANGED);
    QueueEvent(wxEVT_MEDIA_PLAY);
}

void wxMediaBackendCommonBase::QueuePauseEvent()
{
    QueueEvent(wxEVT_MEDIA_STATECHANGED);
    QueueEvent(wxEVT_MEDIA_PAUSE);
}

void wxMediaBackendCommonBase::QueueStopEvent()
{
    QueueEvent(wxEVT_MEDIA_STATECHANGED);
    QueueEvent(wxEVT_MEDIA_STOP);
}


//
// Force link default backends in -
// see https://wiki.wxwidgets.org/RTTI
//
#include "wx/html/forcelnk.h"

#ifdef __WXMSW__ // MSW has huge backends so we do it separately
FORCE_LINK(wxmediabackend_am)
FORCE_LINK(wxmediabackend_wmp10)
#else
FORCE_LINK(basewxmediabackends)
#endif

#endif //wxUSE_MEDIACTRL
