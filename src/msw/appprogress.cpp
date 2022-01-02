/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/appprog.cpp
// Purpose:     Implementation of wxAppProgressIndicator.
// Author:      Chaobin Zhang <zhchbin@gmail.com>
// Created:     2014-09-05
// Copyright:   (c) 2014 wxWidgets development team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_TASKBARBUTTON

#include "wx/toplevel.h"
#include "wx/appprogress.h"
#include "wx/msw/taskbarbutton.h"

#include <memory>

import <algorithm>;
import <utility>;

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
            std::unique_ptr<wxTaskBarButton> button = wxTaskBarButton::Create(*it);
            if ( button )
                m_taskBarButtons.emplace_back(std::move(button));
        }
    }
    else
    {
        std::unique_ptr<wxTaskBarButton> button = wxTaskBarButton::Create(parent);
        if ( button )
            m_taskBarButtons.emplace_back(std::move(button));
    }

    Reset();
    SetRange(m_maxValue);
}

wxAppProgressIndicator::~wxAppProgressIndicator()
{
    Reset();
}

bool wxAppProgressIndicator::IsAvailable() const
{
    return !m_taskBarButtons.empty();
}

void wxAppProgressIndicator::SetValue(int value)
{
    wxASSERT_MSG( value <= m_maxValue, "invalid progress value" );

    std::ranges::for_each(m_taskBarButtons,
            [value](auto& button){
                button->SetProgressValue(value);
            });
}

void wxAppProgressIndicator::SetRange(int range)
{
    m_maxValue = range;

    std::ranges::for_each(m_taskBarButtons,
            [range](auto& button){
                button->SetProgressRange(range);
            });
}

void wxAppProgressIndicator::Pulse()
{
    std::ranges::for_each(m_taskBarButtons,
        [](auto& button){
            button->PulseProgress();
        });
}

void wxAppProgressIndicator::Reset()
{
    std::ranges::for_each(m_taskBarButtons,
        [](auto& button){
            button->SetProgressState(wxTaskBarButtonState::NoProgress);
        });
}

#endif // wxUSE_TASKBARBUTTON
