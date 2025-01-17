///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/subwin.h
// Purpose:     helper for implementing the controls with subwindows
// Author:      Vadim Zeitlin
// Modified by:
// Created:     2004-12-11
// Copyright:   (c) 2004 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_SUBWIN_H_
#define _WX_MSW_SUBWIN_H_

#include "wx/msw/private.h"

import Utils.Geometry;

// ----------------------------------------------------------------------------
// wxSubwindows contains all HWNDs making part of a single wx control
// ----------------------------------------------------------------------------

class wxSubwindows
{
public:
    // the number of subwindows can be specified either as parameter to ctor or
    // later in Create()
    wxSubwindows(size_t n = 0)
    { 
     if ( n ) Create(n);
    }

    wxSubwindows& operator=(wxSubwindows&&) = delete;
    
    // allocate enough space for the given number of windows
    void Create(size_t n)
    {
        wxASSERT_MSG( !m_hwnds, "Create() called twice?" );

        m_count = n;
        m_hwnds = (WXHWND *)calloc(n, sizeof(WXHWND));
        m_ids = new wxWindowIDRef[n];
    }

    ~wxSubwindows()
    {
        for ( size_t n = 0; n < m_count; n++ )
        {
            if ( m_hwnds[n] )
                ::DestroyWindow(m_hwnds[n]);
        }

        free(m_hwnds);
        delete [] m_ids;
    }

    // get the number of subwindows
    size_t GetCount() const { return m_count; }

    // access a given window
    WXHWND& Get(size_t n)
    {
        wxASSERT_MSG( n < m_count, "subwindow index out of range" );

        return m_hwnds[n];
    }

    WXHWND operator[](size_t n) const
    {
        return const_cast<wxSubwindows *>(this)->Get(n);
    }

    // initialize the given window: id will be stored in wxWindowIDRef ensuring
    // that it is not reused while this object exists
    void Set(size_t n, WXHWND hwnd, wxWindowID id)
    {
        wxASSERT_MSG( n < m_count, "subwindow index out of range" );

        m_hwnds[n] = hwnd;
        m_ids[n] = id;
    }

    // check if we have this window
    bool HasWindow(WXHWND hwnd)
    {
        for ( size_t n = 0; n < m_count; n++ )
        {
            if ( m_hwnds[n] == hwnd )
                return true;
        }

        return false;
    }


    // methods which are forwarded to all subwindows
    // ---------------------------------------------

    // show/hide everything
    void Show(bool show)
    {
        int sw = show ? SW_SHOW : SW_HIDE;
        for ( size_t n = 0; n < m_count; n++ )
        {
            if ( m_hwnds[n] )
                ::ShowWindow(m_hwnds[n], sw);
        }
    }

    // enable/disable everything
    void Enable(bool enable)
    {
        for ( size_t n = 0; n < m_count; n++ )
        {
            if ( m_hwnds[n] )
                ::EnableWindow(m_hwnds[n], enable);
        }
    }

    // set font for all windows
    void SetFont(const wxFont& font)
    {
        for ( size_t n = 0; n < m_count; n++ )
        {
            if ( m_hwnds[n] )
            {
                wxSetWindowFont(m_hwnds[n], font);

                // otherwise the window might not be redrawn correctly
                ::InvalidateRect(m_hwnds[n], nullptr, FALSE /* don't erase bg */);
            }
        }
    }

    // add all windows to update region to force redraw
    void Refresh()
    {
        for ( size_t n = 0; n < m_count; n++ )
        {
            if ( m_hwnds[n] )
            {
                ::InvalidateRect(m_hwnds[n], nullptr, FALSE /* don't erase bg */);
            }
        }
    }

    // find the bounding box for all windows
    wxRect GetBoundingBox() const
    {
        wxRect r;
        for ( size_t n = 0; n < m_count; n++ )
        {
            if ( m_hwnds[n] )
            {
                RECT rc;

                ::GetWindowRect(m_hwnds[n], &rc);

                r.Union(wxRectFromRECT(rc));
            }
        }

        return r;
    }

private:
    // number of elements in m_hwnds array
    size_t m_count{0};

    // the HWNDs we contain
    WXHWND *m_hwnds{nullptr};

    // the IDs of the windows
    wxWindowIDRef *m_ids{nullptr};
};

// convenient macro to forward a few methods which are usually propagated to
// subwindows to a wxSubwindows object
//
// parameters should be:
//  - cname the name of the class implementing these methods
//  - base the name of its base class
//  - subwins the name of the member variable of type wxSubwindows *
#define WX_FORWARD_STD_METHODS_TO_SUBWINDOWS(cname, base, subwins)            \
    bool cname::ContainsHWND(WXHWND hWnd) const                               \
    {                                                                         \
        return subwins && subwins->HasWindow((WXHWND)hWnd);                     \
    }                                                                         \
                                                                              \
    bool cname::Show(bool show)                                               \
    {                                                                         \
        if ( !base::Show(show) )                                              \
            return false;                                                     \
                                                                              \
        if ( subwins )                                                        \
            subwins->Show(show);                                              \
                                                                              \
        return true;                                                          \
    }                                                                         \
                                                                              \
    bool cname::Enable(bool enable)                                           \
    {                                                                         \
        if ( !base::Enable(enable) )                                          \
            return false;                                                     \
                                                                              \
        if ( subwins )                                                        \
            subwins->Enable(enable);                                          \
                                                                              \
        return true;                                                          \
    }                                                                         \
                                                                              \
    bool cname::SetFont(const wxFont& font)                                   \
    {                                                                         \
        if ( !base::SetFont(font) )                                           \
            return false;                                                     \
                                                                              \
        if ( subwins )                                                        \
            subwins->SetFont(font);                                           \
                                                                              \
        return true;                                                          \
    }                                                                         \
                                                                              \
    bool cname::SetForegroundColour(const wxColour& colour)                   \
    {                                                                         \
        if ( !base::SetForegroundColour(colour) )                             \
            return false;                                                     \
                                                                              \
        if ( subwins )                                                        \
            subwins->Refresh();                                               \
                                                                              \
        return true;                                                          \
    }                                                                         \
                                                                              \
    bool cname::SetBackgroundColour(const wxColour& colour)                   \
    {                                                                         \
        if ( !base::SetBackgroundColour(colour) )                             \
            return false;                                                     \
                                                                              \
        if ( subwins )                                                        \
            subwins->Refresh();                                               \
                                                                              \
        return true;                                                          \
    }                                                                         \


#endif // _WX_MSW_SUBWIN_H_

