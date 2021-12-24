///////////////////////////////////////////////////////////////////////////////
// Name:        wx/mediactrl.h
// Purpose:     wxMediaCtrl class
// Author:      Ryan Norton <wxprojects@comcast.net>
// Modified by:
// Created:     11/07/04
// Copyright:   (c) Ryan Norton
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MEDIACTRL_H_
#define _WX_MEDIACTRL_H_

#if wxUSE_MEDIACTRL

#include "wx/control.h"
#include "wx/geometry/rect.h"

import WX.Cmn.Uri;
import WX.File.Flags;

enum class wxMediaState
{
    Stopped,
    Paused,
    Playing
};

enum wxMediaCtrlPlayerControls
{
    wxMEDIACTRLPLAYERCONTROLS_NONE           =   0,
    //Step controls like fastforward, step one frame etc.
    wxMEDIACTRLPLAYERCONTROLS_STEP           =   1 << 0,
    //Volume controls like the speaker icon, volume slider, etc.
    wxMEDIACTRLPLAYERCONTROLS_VOLUME         =   1 << 1,
    wxMEDIACTRLPLAYERCONTROLS_DEFAULT        =
                    wxMEDIACTRLPLAYERCONTROLS_STEP |
                    wxMEDIACTRLPLAYERCONTROLS_VOLUME
};

inline constexpr char wxMEDIABACKEND_DIRECTSHOW[]   = "wxAMMediaBackend";
inline constexpr char wxMEDIABACKEND_MCI[]          = "wxMCIMediaBackend";
inline constexpr char wxMEDIABACKEND_QUICKTIME[]    = "wxQTMediaBackend";
inline constexpr char wxMEDIABACKEND_GSTREAMER[]    = "wxGStreamerMediaBackend";
inline constexpr char wxMEDIABACKEND_REALPLAYER[]   = "wxRealPlayerMediaBackend";
inline constexpr char wxMEDIABACKEND_WMP10[]        = "wxWMP10MediaBackend";

class wxMediaEvent : public wxNotifyEvent
{
public:
    wxMediaEvent(wxEventType commandType = wxEVT_NULL, int winid = 0)
        : wxNotifyEvent(commandType, winid)
    {}

    wxMediaEvent(const wxMediaEvent &clone) = default;

    // ------------------------------------------------------------------------
    // wxMediaEvent::Clone
    //
    // Allocates a copy of this object.
    // Required for wxEvtHandler::AddPendingEvent
    // ------------------------------------------------------------------------
    std::unique_ptr<wxEvent> Clone() const override
    {   return std::make_unique<wxMediaEvent>(*this);     }
};

class wxMediaCtrl : public wxControl
{
public:
    wxMediaCtrl() = default;

    wxMediaCtrl(wxWindow* parent, wxWindowID winid,
                const wxString& fileName = {},
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const wxString& szBackend = {},
                const wxValidator& validator = {},
                const wxString& name = "mediaCtrl")
                : m_imp(nullptr), m_bLoaded(false)
    {   Create(parent, winid, fileName, pos, size, style,
               szBackend, validator, name);                             }

    wxMediaCtrl(wxWindow* parent, wxWindowID winid,
                const wxURI& location,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const wxString& szBackend = {},
                const wxValidator& validator = {},
                const wxString& name = "mediaCtrl")
                : m_imp(nullptr), m_bLoaded(false)
    {   Create(parent, winid, location, pos, size, style,
               szBackend, validator, name);                             }

    ~wxMediaCtrl();

    [[maybe_unused]] bool Create(wxWindow* parent, wxWindowID winid,
                const wxString& fileName = {},
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const wxString& szBackend = {},
                const wxValidator& validator = {},
                const wxString& name = "mediaCtrl");

    [[maybe_unused]] bool Create(wxWindow* parent, wxWindowID winid,
                const wxURI& location,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const wxString& szBackend = {},
                const wxValidator& validator = {},
                const wxString& name = "mediaCtrl");

    bool DoCreate(const wxClassInfo* instance,
                wxWindow* parent, wxWindowID winid,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                unsigned int style = 0,
                const wxValidator& validator = {},
                const wxString& name = "mediaCtrl");

    bool Play();
    bool Pause();
    bool Stop();

    bool Load(const wxString& fileName);

    wxMediaState GetState();

    wxFileOffset Seek(wxFileOffset where, wxSeekMode mode = wxSeekMode::FromStart);
    wxFileOffset Tell(); //FIXME: This should be const
    wxFileOffset Length(); //FIXME: This should be const

    double GetPlaybackRate();           //All but MCI & GStreamer
    bool SetPlaybackRate(double dRate); //All but MCI & GStreamer

    bool Load(const wxURI& location);
    bool Load(const wxURI& location, const wxURI& proxy);

    wxFileOffset GetDownloadProgress(); // DirectShow only
    wxFileOffset GetDownloadTotal();    // DirectShow only

    double GetVolume();
    bool   SetVolume(double dVolume);

    bool    ShowPlayerControls(
        wxMediaCtrlPlayerControls flags = wxMEDIACTRLPLAYERCONTROLS_DEFAULT);

    //helpers for the wxPython people
    bool LoadURI(const wxString& fileName)
    {   return Load(wxURI(fileName));       }
    bool LoadURIWithProxy(const wxString& fileName, const wxString& proxy)
    {   return Load(wxURI(fileName), wxURI(proxy));       }

protected:
    static const wxClassInfo* NextBackend(wxClassInfo::const_iterator* it);

    void OnMediaFinished(wxMediaEvent& evt);
    void DoMoveWindow(wxRect boundary) override;
    wxSize DoGetBestSize() const override;

    class wxMediaBackend* m_imp{nullptr};
    bool m_bLoaded{false};

    wxDECLARE_DYNAMIC_CLASS(wxMediaCtrl);
};

// ----------------------------------------------------------------------------
//
// wxMediaBackend
//
// Derive from this and use standard wxWidgets RTTI
// (wxDECLARE_DYNAMIC_CLASS and wxIMPLEMENT_CLASS) to make a backend
// for wxMediaCtrl.  Backends are searched alphabetically -
// the one with the earliest letter is tried first.
//
// Note that this is currently not API or ABI compatible -
// so statically link or make the client compile on-site.
//
// ----------------------------------------------------------------------------

class wxMediaBackend : public wxObject
{
public:
    virtual bool CreateControl([[maybe_unused]] wxControl* ctrl,
                               [[maybe_unused]] wxWindow* parent,
                               [[maybe_unused]] wxWindowID winid,
                               [[maybe_unused]] const wxPoint& pos,
                               [[maybe_unused]] const wxSize& size,
                               [[maybe_unused]] unsigned int style,
                               [[maybe_unused]] const wxValidator& validator,
                               [[maybe_unused]] const wxString& name)
    {   return false;                   }

    virtual bool Play()
    {   return false;                   }
    virtual bool Pause()
    {   return false;                   }
    virtual bool Stop()
    {   return false;                   }

    virtual bool Load([[maybe_unused]] const wxString& fileName)
    {   return false;                   }
    virtual bool Load([[maybe_unused]] const wxURI& location)
    {   return false;                   }

    virtual bool SetPosition([[maybe_unused]] std::int64_t where)
    {   return 0;                       }
    virtual std::int64_t GetPosition()
    {   return 0;                       }
    virtual std::int64_t GetDuration()
    {   return 0;                       }

    virtual void Move([[maybe_unused]] wxRect boundary)
    {                                   }
    virtual wxSize GetVideoSize() const
    {   return {0, 0};             }

    virtual double GetPlaybackRate()
    {   return 0.0;                     }
    virtual bool SetPlaybackRate([[maybe_unused]] double dRate)
    {   return false;                   }

    virtual wxMediaState GetState()
    {   return wxMediaState::Stopped;    }

    virtual double GetVolume()
    {   return 0.0;                     }
    virtual bool SetVolume([[maybe_unused]] double dVolume)
    {   return false;                   }

    virtual bool Load([[maybe_unused]] const wxURI& location,
                      [[maybe_unused]] const wxURI& proxy)
    {   return false;                   }

    virtual bool   ShowPlayerControls(
                    [[maybe_unused]] wxMediaCtrlPlayerControls flags)
    {   return false;                   }
    virtual bool   IsInterfaceShown()
    {   return false;                   }

    virtual std::int64_t GetDownloadProgress()
    {    return 0;                      }
    virtual std::int64_t GetDownloadTotal()
    {    return 0;                      }

    virtual void MacVisibilityChanged()
    {                                   }
    virtual void RESERVED9() {}

    wxDECLARE_DYNAMIC_CLASS(wxMediaBackend);
};


//Our events
wxDECLARE_EVENT( wxEVT_MEDIA_FINISHED, wxMediaEvent );
wxDECLARE_EVENT( wxEVT_MEDIA_STOP, wxMediaEvent );

//Function type(s) our events need
typedef void (wxEvtHandler::*wxMediaEventFunction)(wxMediaEvent&);

#define wxMediaEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxMediaEventFunction, func)

//Macro for usage with message maps
#define EVT_MEDIA_FINISHED(winid, fn)   wxDECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_FINISHED, winid, wxID_ANY, wxMediaEventHandler(fn), NULL ),
#define EVT_MEDIA_STOP(winid, fn)       wxDECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_STOP, winid, wxID_ANY, wxMediaEventHandler(fn), NULL ),

wxDECLARE_EVENT( wxEVT_MEDIA_LOADED, wxMediaEvent );
#define EVT_MEDIA_LOADED(winid, fn)     wxDECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_LOADED, winid, wxID_ANY, wxMediaEventHandler(fn), NULL ),

wxDECLARE_EVENT( wxEVT_MEDIA_STATECHANGED, wxMediaEvent );
wxDECLARE_EVENT( wxEVT_MEDIA_PLAY, wxMediaEvent );
wxDECLARE_EVENT( wxEVT_MEDIA_PAUSE, wxMediaEvent );
#define EVT_MEDIA_STATECHANGED(winid, fn)   wxDECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_STATECHANGED, winid, wxID_ANY, wxMediaEventHandler(fn), NULL ),
#define EVT_MEDIA_PLAY(winid, fn)           wxDECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_PLAY, winid, wxID_ANY, wxMediaEventHandler(fn), NULL ),
#define EVT_MEDIA_PAUSE(winid, fn)          wxDECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_PAUSE, winid, wxID_ANY, wxMediaEventHandler(fn), NULL ),

class wxMediaBackendCommonBase : public wxMediaBackend
{
public:
    // add a pending wxMediaEvent of the given type
    void QueueEvent(wxEventType evtType);

    // notify that the movie playback is finished
    void QueueFinishEvent()
    {
        QueueEvent(wxEVT_MEDIA_STATECHANGED);
        QueueEvent(wxEVT_MEDIA_FINISHED);
    }

    // send the stop event and return true if it hasn't been vetoed
    bool SendStopEvent();

    // Queue pause event
    void QueuePlayEvent();

    // Queue pause event
    void QueuePauseEvent();

    // Queue stop event (no veto)
    void QueueStopEvent();

protected:
    // call this when the movie size has changed but not because it has just
    // been loaded (in this case, call NotifyMovieLoaded() below)
    void NotifyMovieSizeChanged();

    // call this when the movie is fully loaded
    void NotifyMovieLoaded();


    wxMediaCtrl *m_ctrl;      // parent control
};

#endif // wxUSE_MEDIACTRL

#endif // _WX_MEDIACTRL_H_
