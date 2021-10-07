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

#include <atomic>

// ----------------------------------------------------------------------------
// Version of wxRefCounter with MT-safe count
// ----------------------------------------------------------------------------

class wxRefCounterMT
{
public:
    wxRefCounterMT() = default;

    void IncRef() { ++m_count; }
    void DecRef()
    {
        if ( (--m_count) == 0 )
            delete this;
    }

    wxRefCounterMT(const wxRefCounterMT&) = delete;
	wxRefCounterMT& operator=(const wxRefCounterMT&) = delete;
protected:
    virtual ~wxRefCounterMT() = default;

private:
    // Ref count is atomic to allow IncRef() and DecRef() to be concurrently
    // called from different threads.
    std::atomic_int m_count{1};
};

#endif // _WX_PRIVATE_REFCOUNTERMT_H_
