/////////////////////////////////////////////////////////////////////////////
// Name:        wx/windowptr.h
// Purpose:     smart pointer for holding wxWindow instances
// Author:      Vaclav Slavik
// Created:     2013-09-01
// Copyright:   (c) 2013 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_WINDOWPTR_H_
#define _WX_WINDOWPTR_H_

// ----------------------------------------------------------------------------
// wxWindowPtr: A smart pointer with correct wxWindow destruction.
// ----------------------------------------------------------------------------

namespace wxPrivate
{

struct wxWindowDeleter
{
    void operator()(wxWindow *win)
    {
        win->Destroy();
    }
};

} // namespace wxPrivate

template<typename T>
class wxWindowPtr : public std::shared_ptr<T>
{
public:
    using element_type = T;

    explicit wxWindowPtr(element_type* win)
        : std::shared_ptr<T>(win, wxPrivate::wxWindowDeleter())
    {
    }

    wxWindowPtr() = default;
    wxWindowPtr(const wxWindowPtr& tocopy) : std::shared_ptr<T>(tocopy) {}

    wxWindowPtr& operator=(const wxWindowPtr& tocopy)
    {
        std::shared_ptr<T>::operator=(tocopy);
        return *this;
    }

    wxWindowPtr& operator=(element_type* win)
    {
        return operator=(wxWindowPtr(win));
    }

    void reset(T* ptr = NULL)
    {
        std::shared_ptr<T>::reset(ptr, wxPrivate::wxWindowDeleter());
    }
};

#endif // _WX_WINDOWPTR_H_
