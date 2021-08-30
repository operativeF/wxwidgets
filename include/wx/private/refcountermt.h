///////////////////////////////////////////////////////////////////////////////
// Name:        wx/private/refcountermt.h
// Purpose:     wxRefCounterMT class: MT-safe version of wxRefCounter
// Author:      Vadim Zeitlin
// Created:     2021-01-11
// Copyright:   (c) 2021 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PRIVATE_REFCOUNTERMT_H_
#define _WX_PRIVATE_REFCOUNTERMT_H_

#include "wx/atomic.h"

// ----------------------------------------------------------------------------
// Version of wxRefCounter with MT-safe count
// ----------------------------------------------------------------------------

class wxRefCounterMT
{
public:
    wxRefCounterMT() noexcept { m_count = 1; }

    void IncRef() { wxAtomicInc(m_count); }
    void DecRef()
    {
        if ( wxAtomicDec(m_count) == 0 )
            delete this;
    }

protected:
    virtual ~wxRefCounterMT() = default;

private:
    // Ref count is atomic to allow IncRef() and DecRef() to be concurrently
    // called from different threads.
    wxAtomicInt m_count;

    wxRefCounterMT(const wxRefCounterMT&) = delete;
	wxRefCounterMT& operator=(const wxRefCounterMT&) = delete;
};

#endif // _WX_PRIVATE_REFCOUNTERMT_H_
