/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/appprogress.h
// Purpose:     wxAppProgressIndicator OS X implementation
// Author:      Tobias Taschner
// Created:     2014-10-22
// Copyright:   (c) 2014 wxWidgets development team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_OSX_APPPROG_H_
#define _WX_OSX_APPPROG_H_

#include "wx/window.h"

class wxAppProgressIndicator
    : public wxAppProgressIndicatorBase
{
public:
    wxAppProgressIndicator(wxWindow* parent = NULL, int maxValue = 100);
    virtual ~wxAppProgressIndicator();

    bool IsAvailable() const override;

    void SetValue(int value) override;
    void SetRange(int range) override;
    void Pulse() override;
    void Reset() override;

private:
    int m_maxValue;
    void *m_dockIcon;

    wxAppProgressIndicator(const wxAppProgressIndicator&) = delete;
	wxAppProgressIndicator& operator=(const wxAppProgressIndicator&) = delete;
};

#endif  // _WX_OSX_APPPROG_H_
