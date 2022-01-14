///////////////////////////////////////////////////////////////////////////////
// Name:        tests/events/clone.cpp
// Purpose:     Test wxEvent::Clone() implementation by all event classes
// Author:      Vadim Zeitlin, based on the code by Francesco Montorsi
// Created:     2009-03-22
// Copyright:   (c) 2009 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/event.h"
#include "wx/timer.h"

export module WX.Test.EventClone;

import WX.MetaTest;
import WX.Test.Prec;

namespace ut = boost::ut;

ut::suite EventCloneTest = []
{
    using namespace ut;
    
    // Dummy timer needed just to create a wxTimerEvent.
    wxTimer dummyTimer;

    // check if event classes implement Clone() correctly
    // NOTE: the check is done against _all_ event classes which are linked to
    //       the executable currently running, which are not necessarily all
    //       wxWidgets event classes.
    const wxClassInfo *ci = wxClassInfo::GetFirst();

    for (; ci; ci = ci->GetNext())
    {
        auto cn = std::string(ci->wxGetClassName());

        // is this class derived from wxEvent?
        if ( !ci->IsKindOf(CLASSINFO(wxEvent)) ||
             cn == "wxEvent" )
            continue;

        // FIXME: Info for metatest?
        //INFO("Event class \"" << cn << "\"");

        wxEvent* test;
        if ( ci->IsDynamic() )
        {
            test = dynamic_cast<wxEvent*>(ci->CreateObject());
        }
        else if ( cn == "wxTimerEvent" )
        {
            test = new wxTimerEvent(dummyTimer);
        }

        expect( test ) << fmt::format("Can't create objects of type {}.", cn);

        auto cloned = test->Clone();
        delete test;

        expect( cloned.get() );
        expect( cloned->wxGetClassInfo() == ci );
    }
};

