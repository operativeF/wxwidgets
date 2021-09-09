/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/appprog.cpp
// Purpose:     Implementation of wxAppProgressIndicator.
// Author:      Chaobin Zhang <zhchbin@gmail.com>
// Created:     2014-09-05
// Copyright:   (c) 2014 wxWidgets development team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"


#if wxUSE_TASKBARBUTTON

#include "wx/toplevel.h"
#include "wx/appprogress.h"
#include "wx/msw/taskbarbutton.h"

// ----------------------------------------------------------------------------
// wxAppProgressIndicator Implementation.
// ----------------------------------------------------------------------------
wxAppProgressIndicator::wxAppProgressIndicator(wxWindow* parent, int maxValue)
    : m_maxValue(maxValue)
{
    if ( parent == nullptr )
    {
        for ( wxWindowList::const_iterator it = wxTopLevelWindows.begin();
              it != wxTopLevelWindows.end();
              ++it )
        {
            wxTaskBarButton* const button = wxTaskBarButton::New(*it);
            if ( button )
                m_taskBarButtons.push_back(button);
        }
    }
    else
    {
        wxTaskBarButton* const button = wxTaskBarButton::New(parent);
        if ( button )
            m_taskBarButtons.push_back(button);
    }

    Reset();
    SetRange(m_maxValue);
}

wxAppProgressIndicator::~wxAppProgressIndicator()
{
    Reset();

    for ( auto* button : m_taskBarButtons )
    {
        delete button;
    }
}

bool wxAppProgressIndicator::IsAvailable() const
{
    return !m_taskBarButtons.empty();
}

void wxAppProgressIndicator::SetValue(int value)
{
    wxASSERT_MSG( value <= m_maxValue, wxT("invalid progress value") );

    std::for_each(m_taskBarButtons.begin(), m_taskBarButtons.end(),
            [value](auto& button){
                button->SetProgressValue(value);
            });
}

void wxAppProgressIndicator::SetRange(int range)
{
    m_maxValue = range;

    std::for_each(m_taskBarButtons.begin(), m_taskBarButtons.end(),
            [range](auto& button){
                button->SetProgressRange(range);
            });
}

void wxAppProgressIndicator::Pulse()
{
    std::for_each(m_taskBarButtons.begin(), m_taskBarButtons.end(),
        [](auto& button){
            button->PulseProgress();
        });
}

void wxAppProgressIndicator::Reset()
{
    std::for_each(m_taskBarButtons.begin(), m_taskBarButtons.end(),
        [](auto& button){
            button->SetProgressState(wxTaskBarButtonState::NoProgress);
        });
}

#endif // wxUSE_TASKBARBUTTON
