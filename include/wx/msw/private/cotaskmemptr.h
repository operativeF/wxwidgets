///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/private/cotaskmemptr.h
// Purpose:     RAII class for pointers to be freed with ::CoTaskMemFree().
// Author:      PB
// Created:     2020-06-09
// Copyright:   (c) 2020 wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_PRIVATE_COTASKMEMPTR_H_
#define _WX_MSW_PRIVATE_COTASKMEMPTR_H_

// ----------------------------------------------------------------------------
// wxCoTaskMemPtr: A barebone RAII class for pointers to be freed with ::CoTaskMemFree().
// ----------------------------------------------------------------------------

template <class T>
class wxCoTaskMemPtr
{
public:
    using element_type = T;

    wxCoTaskMemPtr()
        : m_ptr(nullptr)
    {}

    explicit wxCoTaskMemPtr(T* ptr)
        : m_ptr(ptr)
    {}

    // Uses ::CoTaskMemAlloc() to allocate size bytes.
    explicit wxCoTaskMemPtr(size_t size)
        : m_ptr(static_cast<T*>(::CoTaskMemAlloc(size)))
    {}

    ~wxCoTaskMemPtr()
    {
        ::CoTaskMemFree(m_ptr);
    }

    void reset(T* ptr = NULL)
    {
        if ( m_ptr != ptr )
        {
            ::CoTaskMemFree(m_ptr);
            m_ptr = ptr;
        }
    }

    operator T*() const
    {
        return m_ptr;
    }

    // It would be better to forbid direct access completely but we do need it,
    // so provide it but it can only be used to initialize the pointer,
    // not to modify an existing one.
    T** operator&()
    {
        wxASSERT_MSG(!m_ptr,
                     "Can't get direct access to initialized pointer");

        return &m_ptr;
    }

    // Gives up the ownership of the pointer,
    // making the caller responsible for freeing it.
    T* release()
    {
        T* ptr(m_ptr);

        m_ptr = nullptr;
        return ptr;
    }

private:
    T* m_ptr;

    wxCoTaskMemPtr(const wxCoTaskMemPtr<T>&)  = delete;
	wxCoTaskMemPtr& operator=(const wxCoTaskMemPtr<T>&) = delete;
};

#endif // _WX_MSW_PRIVATE_COTASKMEMPTR_H_
