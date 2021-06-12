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

#include "wx/defs.h"

#if wxUSE_MEDIACTRL

#include "wx/control.h"
#include "wx/uri.h"

enum wxMediaState
{
    wxMEDIASTATE_STOPPED,
    wxMEDIASTATE_PAUSED,
    wxMEDIASTATE_PLAYING
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

inline constexpr wxChar wxMEDIABACKEND_DIRECTSHOW[]   = wxT("wxAMMediaBackend");
inline constexpr wxChar wxMEDIABACKEND_MCI[]          = wxT("wxMCIMediaBackend");
inline constexpr wxChar wxMEDIABACKEND_QUICKTIME[]    = wxT("wxQTMediaBackend");
inline constexpr wxChar wxMEDIABACKEND_GSTREAMER[]    = wxT("wxGStreamerMediaBackend");
inline constexpr wxChar wxMEDIABACKEND_REALPLAYER[]   = wxT("wxRealPlayerMediaBackend");
inline constexpr wxChar wxMEDIABACKEND_WMP10[]        = wxT("wxWMP10MediaBackend");

class WXDLLIMPEXP_MEDIA wxMediaEvent : public wxNotifyEvent
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
    wxEvent *Clone() const override
    {   return new wxMediaEvent(*this);     }


    // Put this class on wxWidget's RTTI table
    wxDECLARE_DYNAMIC_CLASS(wxMediaEvent);
};

class WXDLLIMPEXP_MEDIA wxMediaCtrl : public wxControl
{
public:
    wxMediaCtrl() = default;

    wxMediaCtrl(wxWindow* parent, wxWindowID winid,
                const wxString& fileName = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& szBackend = wxEmptyString,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxT("mediaCtrl"))
                : m_imp(nullptr), m_bLoaded(false)
    {   Create(parent, winid, fileName, pos, size, style,
               szBackend, validator, name);                             }

    wxMediaCtrl(wxWindow* parent, wxWindowID winid,
                const wxURI& location,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& szBackend = wxEmptyString,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxT("mediaCtrl"))
                : m_imp(nullptr), m_bLoaded(false)
    {   Create(parent, winid, location, pos, size, style,
               szBackend, validator, name);                             }

    ~wxMediaCtrl() override;

    bool Create(wxWindow* parent, wxWindowID winid,
                const wxString& fileName = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& szBackend = wxEmptyString,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxT("mediaCtrl"));

    bool Create(wxWindow* parent, wxWindowID winid,
                const wxURI& location,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& szBackend = wxEmptyString,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxT("mediaCtrl"));

    bool DoCreate(const wxClassInfo* instance,
                wxWindow* parent, wxWindowID winid,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxT("mediaCtrl"));

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
    void DoMoveWindow(int x, int y, int w, int h) override;
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

class WXDLLIMPEXP_MEDIA wxMediaBackend : public wxObject
{
public:
    wxMediaBackend() = default;

    ~wxMediaBackend() override;

    virtual bool CreateControl(wxControl* WXUNUSED(ctrl),
                               wxWindow* WXUNUSED(parent),
                               wxWindowID WXUNUSED(winid),
                               const wxPoint& WXUNUSED(pos),
                               const wxSize& WXUNUSED(size),
                               long WXUNUSED(style),
                               const wxValidator& WXUNUSED(validator),
                               const wxString& WXUNUSED(name))
    {   return false;                   }

    virtual bool Play()
    {   return false;                   }
    virtual bool Pause()
    {   return false;                   }
    virtual bool Stop()
    {   return false;                   }

    virtual bool Load(const wxString& WXUNUSED(fileName))
    {   return false;                   }
    virtual bool Load(const wxURI& WXUNUSED(location))
    {   return false;                   }

    virtual bool SetPosition(wxLongLong WXUNUSED(where))
    {   return 0;                       }
    virtual wxLongLong GetPosition()
    {   return 0;                       }
    virtual wxLongLong GetDuration()
    {   return 0;                       }

    virtual void Move(int WXUNUSED(x), int WXUNUSED(y),
                      int WXUNUSED(w), int WXUNUSED(h))
    {                                   }
    virtual wxSize GetVideoSize() const
    {   return {0, 0};             }

    virtual double GetPlaybackRate()
    {   return 0.0;                     }
    virtual bool SetPlaybackRate(double WXUNUSED(dRate))
    {   return false;                   }

    virtual wxMediaState GetState()
    {   return wxMEDIASTATE_STOPPED;    }

    virtual double GetVolume()
    {   return 0.0;                     }
    virtual bool SetVolume(double WXUNUSED(dVolume))
    {   return false;                   }

    virtual bool Load(const wxURI& WXUNUSED(location),
                      const wxURI& WXUNUSED(proxy))
    {   return false;                   }

    virtual bool   ShowPlayerControls(
                    wxMediaCtrlPlayerControls WXUNUSED(flags))
    {   return false;                   }
    virtual bool   IsInterfaceShown()
    {   return false;                   }

    virtual wxLongLong GetDownloadProgress()
    {    return 0;                      }
    virtual wxLongLong GetDownloadTotal()
    {    return 0;                      }

    virtual void MacVisibilityChanged()
    {                                   }
    virtual void RESERVED9() {}

    wxDECLARE_DYNAMIC_CLASS(wxMediaBackend);
};


//Our events
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_MEDIA, wxEVT_MEDIA_FINISHED, wxMediaEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_MEDIA, wxEVT_MEDIA_STOP, wxMediaEvent );

//Function type(s) our events need
typedef void (wxEvtHandler::*wxMediaEventFunction)(wxMediaEvent&);

#define wxMediaEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxMediaEventFunction, func)

//Macro for usage with message maps
#define EVT_MEDIA_FINISHED(winid, fn)   wxDECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_FINISHED, winid, wxID_ANY, wxMediaEventHandler(fn), NULL ),
#define EVT_MEDIA_STOP(winid, fn)       wxDECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_STOP, winid, wxID_ANY, wxMediaEventHandler(fn), NULL ),

wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_MEDIA, wxEVT_MEDIA_LOADED, wxMediaEvent );
#define EVT_MEDIA_LOADED(winid, fn)     wxDECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_LOADED, winid, wxID_ANY, wxMediaEventHandler(fn), NULL ),

wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_MEDIA, wxEVT_MEDIA_STATECHANGED, wxMediaEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_MEDIA, wxEVT_MEDIA_PLAY, wxMediaEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_MEDIA, wxEVT_MEDIA_PAUSE, wxMediaEvent );
#define EVT_MEDIA_STATECHANGED(winid, fn)   wxDECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_STATECHANGED, winid, wxID_ANY, wxMediaEventHandler(fn), NULL ),
#define EVT_MEDIA_PLAY(winid, fn)           wxDECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_PLAY, winid, wxID_ANY, wxMediaEventHandler(fn), NULL ),
#define EVT_MEDIA_PAUSE(winid, fn)          wxDECLARE_EVENT_TABLE_ENTRY( wxEVT_MEDIA_PAUSE, winid, wxID_ANY, wxMediaEventHandler(fn), NULL ),

class WXDLLIMPEXP_MEDIA wxMediaBackendCommonBase : public wxMediaBackend
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
