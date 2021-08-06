///////////////////////////////////////////////////////////////////////////////
// Name:        wx/dlist.h
// Purpose:     wxDList<T> which is a template version of wxList
// Author:      Robert Roebling
// Created:     18.09.2008
// Copyright:   (c) 2008 wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DLIST_H_
#define _WX_DLIST_H_

#include "wx/defs.h"
#include "wx/utils.h"

#include <algorithm>
#include <iterator>
#include <list>

template<typename T>
class wxDList: public std::list<T*>
{
private:
    bool m_destroy{ false };
    using BaseListType = std::list<T *>;
    using ListType = wxDList<T>;

public:
    using iterator = typename BaseListType::iterator;

    class compatibility_iterator
    {
    private:
        friend class wxDList<T>;

        iterator m_iter;
        ListType *m_list;

    public:
        compatibility_iterator()
            : m_iter(), m_list( NULL ) {}
        compatibility_iterator( ListType* li, iterator i )
            : m_iter( i ), m_list( li ) {}
        compatibility_iterator( const ListType* li, iterator i )
            : m_iter( i ), m_list( const_cast<ListType*>(li) ) {}

        compatibility_iterator* operator->() { return this; }
        const compatibility_iterator* operator->() const { return this; }

        bool operator==(const compatibility_iterator& i) const
        {
            wxASSERT_MSG( m_list && i.m_list,
                          "comparing invalid iterators is illegal" );
            return (m_list == i.m_list) && (m_iter == i.m_iter);
        }
        bool operator!=(const compatibility_iterator& i) const
            { return !( operator==( i ) ); }
        operator bool() const
            { return m_list ? m_iter != m_list->end() : false; }
        bool operator !() const
            { return !( operator bool() ); }

        T* GetData() const { return *m_iter; }
        void SetData( T* e ) { *m_iter = e; }

        compatibility_iterator GetNext() const
        {
            iterator i = m_iter;
            return compatibility_iterator( m_list, ++i );
        }

        compatibility_iterator GetPrevious() const
        {
            if ( m_iter == m_list->begin() )
                return compatibility_iterator();

            iterator i = m_iter;
            return compatibility_iterator( m_list, --i );
        }

        int IndexOf() const
        {
            return *this ? std::distance( m_list->begin(), m_iter )
                : wxNOT_FOUND;
        }
    };

public:
    wxDList()  = default;

    ~wxDList() { Clear(); }

    compatibility_iterator Find( const T* e ) const
    {
        return compatibility_iterator( this,
                std::find( const_cast<ListType*>(this)->begin(),
                           const_cast<ListType*>(this)->end(), e ) );
    }

    bool IsEmpty() const
        { return this->empty(); }
    size_t GetCount() const
        { return this->size(); }

    compatibility_iterator Item( size_t idx ) const
    {
        iterator i = const_cast<ListType*>(this)->begin();
        std::advance( i, idx );
        return compatibility_iterator( this, i );
    }

    T* operator[](size_t idx) const
    {
        return Item(idx).GetData();
    }

    compatibility_iterator GetFirst() const
    {
        return compatibility_iterator( this, const_cast<ListType*>(this)->begin() );
    }
    compatibility_iterator GetLast() const
    {
        iterator i = const_cast<ListType*>(this)->end();
        return compatibility_iterator( this, !(this->empty()) ? --i : i );
    }
    compatibility_iterator Member( T* e ) const
        { return Find( e ); }
    compatibility_iterator Nth( int n ) const
        { return Item( n ); }
    int IndexOf( T* e ) const
        { return Find( e ).IndexOf(); }

    compatibility_iterator Append( T* e )
    {
        this->push_back( e );
        return GetLast();
    }

    compatibility_iterator Insert( T* e )
    {
        this->push_front( e );
        return compatibility_iterator( this, this->begin() );
    }

    compatibility_iterator Insert( compatibility_iterator & i, T* e )
    {
        return compatibility_iterator( this, this->insert( i.m_iter, e ) );
    }

    compatibility_iterator Insert( size_t idx, T* e )
    {
        return compatibility_iterator( this,
                this->insert( Item( idx ).m_iter, e ) );
    }

    void DeleteContents( bool destroy )
        { m_destroy = destroy; }

    bool GetDeleteContents() const
        { return m_destroy; }

    void Erase( const compatibility_iterator& i )
    {
        if ( m_destroy )
            delete i->GetData();
        this->erase( i.m_iter );
    }

    bool DeleteNode( const compatibility_iterator& i )
    {
        if( i )
        {
            Erase( i );
            return true;
        }
        return false;
    }

    bool DeleteObject( T* e )
    {
        return DeleteNode( Find( e ) );
    }

    void Clear()
    {
        if ( m_destroy )
        {
            iterator it, en;
            for ( it = this->begin(), en = this->end(); it != en; ++it )
                delete *it;
        }
        this->clear();
    }
};

#endif // _WX_DLIST_H_
