/////////////////////////////////////////////////////////////////////////////
// Name:        wx/weakref.h
// Purpose:     wxWeakRef - Generic weak references for wxWidgets
// Author:      Arne Steinarson
// Created:     27 Dec 07
// Copyright:   (c) 2007 Arne Steinarson
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_WEAKREF_H_
#define _WX_WEAKREF_H_

#include "wx/debug.h"

#include "wx/tracker.h"

#include <cassert>
import <concepts>;

// A weak reference to an object of type T (which must inherit from wxTrackable)
template <class T>
class wxWeakRef : public wxTrackerNode
{
public:
    using element_type = T;

    // Default ctor
    wxWeakRef() : m_pobj(nullptr) { }

    // Ctor from the object of this type: this is needed as the template ctor
    // below is not used by at least g++4 when a literal NULL is used
    wxWeakRef(T *pobj) : m_pobj(nullptr), m_ptbase(nullptr)
    {
        this->Assign(pobj);
    }

    // When we have the full type here, static_cast<> will always work
    // (or give a straight compiler error).
    template <class TDerived>
    wxWeakRef(TDerived* pobj) : m_pobj(NULL), m_ptbase(NULL)
    {
        this->Assign(pobj);
    }

    // We need this copy ctor, since otherwise a default compiler (binary) copy
    // happens (if embedded as an object member).
    wxWeakRef(const wxWeakRef<T>& wr) : m_pobj(NULL), m_ptbase(NULL)
    {
        this->Assign(wr.get());
    }

    wxWeakRef<T>& operator=(const wxWeakRef<T>& wr)
    {
        this->AssignCopy(wr);
        return *this;
    }

    ~wxWeakRef() { this->Release(); }

    // Smart pointer functions
    T& operator*() const    { return *this->m_pobj; }
    T* operator->() const   { return this->m_pobj; }

    T* get() const          { return this->m_pobj; }
    operator T*() const     { return this->m_pobj; }

public:
    void Release()
    {
        // Release old object if any
        if ( m_pobj )
        {
            // Remove ourselves from object tracker list
            m_ptbase->RemoveNode(this);
            m_pobj = nullptr;
            m_ptbase = nullptr;
        }
    }

    void OnObjectDestroy() override
    {
        // Tracked object itself removes us from list of trackers
        wxASSERT(m_pobj != nullptr);
        m_pobj = nullptr;
        m_ptbase = nullptr;
    }

protected:
    // Assign receives most derived class here and can use that
    template <class TDerived>
    void Assign( TDerived* pobj )
    {
        static_assert( std::derived_from<TDerived, wxTrackable>,
            "Tracked class should inherit from wxTrackable" );

        wxTrackable *ptbase = pobj;
        DoAssign(pobj, ptbase);
    }

    void AssignCopy(const wxWeakRef& wr)
    {
        DoAssign(wr.m_pobj, wr.m_ptbase);
    }

    void DoAssign(T* pobj, wxTrackable *ptbase)
    {
        if ( m_pobj == pobj )
            return;

        Release();

        // Now set new trackable object
        if ( pobj )
        {
            // Add ourselves to object tracker list
            ptbase->AddNode( this );
            m_pobj = pobj;
            m_ptbase = ptbase;
        }
    }

    T *m_pobj;
    wxTrackable *m_ptbase{nullptr};
};


#ifndef wxNO_RTTI

// Weak ref implementation assign objects are queried for wxTrackable
// using dynamic_cast<>
template <class T>
class wxWeakRefDynamic : public wxTrackerNode
{
public:
    wxWeakRefDynamic() : m_pobj(NULL) { }

    wxWeakRefDynamic(T* pobj) : m_pobj(pobj)
    {
        Assign(pobj);
    }

    wxWeakRefDynamic(const wxWeakRef<T>& wr)
    {
        Assign(wr.get());
    }

    ~wxWeakRefDynamic() { Release(); }

    // Smart pointer functions
    T& operator*() const    { return *m_pobj; }
    T* operator->() const   { return m_pobj; }

    T* get() const          { return m_pobj; }
    operator T* () const    { return m_pobj; }

    T* operator = (T* pobj) { Assign(pobj); return m_pobj; }

    // Assign from another weak ref, point to same object
    T* operator = (const wxWeakRef<T> &wr) { Assign( wr.get() ); return m_pobj; }

    void Release()
    {
        // Release old object if any
        if( m_pobj )
        {
            // Remove ourselves from object tracker list
            wxTrackable *pt = dynamic_cast<wxTrackable*>(m_pobj);
            pt->RemoveNode(this);
            m_pobj = NULL;
        }
    }

    void OnObjectDestroy() override
    {
        wxASSERT_MSG(m_pobj, "tracked object should have removed us itself");

        m_pobj = NULL;
    }

protected:
    void Assign(T *pobj)
    {
        if ( m_pobj == pobj )
            return;

        Release();

        // Now set new trackable object
        if ( pobj )
        {
            // Add ourselves to object tracker list
            wxTrackable *pt = dynamic_cast<wxTrackable*>(pobj);
            if ( pt )
            {
                pt->AddNode(this);
                m_pobj = pobj;
            }
            else
            {
                // If the object we want to track does not support wxTackable, then
                // log a message and keep the NULL object pointer.
                wxFAIL_MSG( "Tracked class should inherit from wxTrackable" );
            }
        }
    }

    T *m_pobj;
};

#endif // RTTI enabled


// Provide some basic types of weak references
class wxEvtHandler;
class wxWindow;


using wxEvtHandlerRef = wxWeakRef<wxEvtHandler>;
using wxWindowRef = wxWeakRef<wxWindow>;

#endif // _WX_WEAKREF_H_

