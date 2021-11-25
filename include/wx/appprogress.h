/////////////////////////////////////////////////////////////////////////////
// Name:        wx/appprogress.h
// Purpose:     wxAppProgressIndicator interface.
// Author:      Chaobin Zhang <zhchbin@gmail.com>
// Created:     2014-09-05
// Copyright:   (c) 2014 wxWidgets development team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_APPPROG_H_
#define _WX_APPPROG_H_

class wxAppProgressIndicatorBase
{
public:
    virtual ~wxAppProgressIndicatorBase() = default;

    wxAppProgressIndicatorBase& operator=(wxAppProgressIndicatorBase&&) = delete;
    
    virtual bool IsAvailable() const = 0;

    virtual void SetValue(int value) = 0;
    virtual void SetRange(int range) = 0;
    virtual void Pulse() = 0;
    virtual void Reset() = 0;
};

#if defined(__WXMSW__) && wxUSE_TASKBARBUTTON
    #include "wx/msw/appprogress.h"
#elif defined(__WXOSX_COCOA__)
    #include "wx/osx/appprogress.h"
#else
    class wxAppProgressIndicator : public wxAppProgressIndicatorBase
    {
    public:
        wxAppProgressIndicator([[maybe_unused]] wxWindow* parent = NULL,
                               [[maybe_unused]] int maxValue = 100)
        {
        }

        bool IsAvailable() const override { return false; }

        void SetValue([[maybe_unused]] int value) override { }
        void SetRange([[maybe_unused]] int range) override { }
        void Pulse() override { }
        void Reset() override { }
    };
#endif

#endif  // _WX_APPPROG_H_
