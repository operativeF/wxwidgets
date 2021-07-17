/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/dcclient.h
// Purpose:     wxClientDC class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DCCLIENT_H_
#define _WX_DCCLIENT_H_

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wx/dc.h"
#include "wx/msw/dc.h"
#include "wx/dcclient.h"

class wxPaintDCInfo;

// ----------------------------------------------------------------------------
// DC classes
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxWindowDCImpl : public wxMSWDCImpl
{
public:
    // default ctor
    wxWindowDCImpl( wxDC *owner );

    // Create a DC corresponding to the whole window
    wxWindowDCImpl( wxDC *owner, wxWindow *win );

    wxWindowDCImpl(const wxWindowDCImpl&) = delete;
	wxWindowDCImpl& operator=(const wxWindowDCImpl&) = delete;
    wxWindowDCImpl(wxWindowDCImpl&&) = default;
	wxWindowDCImpl& operator=(wxWindowDCImpl&&) = default;

    ~wxWindowDCImpl() = default;

    wxSize DoGetSize() const override;

protected:
    // initialize the newly created DC
    void InitDC();

    wxDECLARE_CLASS(wxWindowDCImpl);
};

class WXDLLIMPEXP_CORE wxClientDCImpl : public wxWindowDCImpl
{
public:
    // default ctor
    wxClientDCImpl( wxDC *owner );

    // Create a DC corresponding to the client area of the window
    wxClientDCImpl( wxDC *owner, wxWindow *win );

    ~wxClientDCImpl() override = default;

    wxClientDCImpl(const wxClientDCImpl&) = delete;
	wxClientDCImpl& operator=(const wxClientDCImpl&) = delete;
    wxClientDCImpl(wxClientDCImpl&&) = default;
	wxClientDCImpl& operator=(wxClientDCImpl&&) = default;

    wxSize DoGetSize() const override;

protected:
    void InitDC();

    wxDECLARE_CLASS(wxClientDCImpl);
};

class WXDLLIMPEXP_CORE wxPaintDCImpl : public wxClientDCImpl
{
public:
    wxPaintDCImpl( wxDC *owner );

    // Create a DC corresponding for painting the window in OnPaint()
    wxPaintDCImpl( wxDC *owner, wxWindow *win );

    ~wxPaintDCImpl() override;

    wxPaintDCImpl(const wxPaintDCImpl&) = delete;
	wxPaintDCImpl& operator=(const wxPaintDCImpl&) = delete;
    wxPaintDCImpl(wxPaintDCImpl&&) = default;
	wxPaintDCImpl& operator=(wxPaintDCImpl&&) = default;

    // find the entry for this DC in the cache (keyed by the window)
    static WXHDC FindDCInCache(wxWindow* win);

    // This must be called by the code handling WM_PAINT to remove the DC
    // cached for this window for the duration of this message processing.
    static void EndPaint(wxWindow *win);

protected:
    // Find the DC for this window in the cache, return NULL if not found.
    static wxPaintDCInfo *FindInCache(wxWindow* win);

    wxDECLARE_CLASS(wxPaintDCImpl);
};

/*
 * wxPaintDCEx
 * This struct is used when an application sends an HDC with the WM_PAINT
 * message. It is used in HandlePaint and need not be used by an application.
 */

struct WXDLLIMPEXP_CORE wxPaintDCEx : public wxPaintDC
{
    wxPaintDCEx(wxWindow *canvas, WXHDC dc);

    wxDECLARE_CLASS(wxPaintDCEx);
    wxPaintDCEx(const wxPaintDCEx&) = delete;
    wxPaintDCEx& operator=(const wxPaintDCEx&) = delete;
    wxPaintDCEx(wxPaintDCEx&&) = default;
    wxPaintDCEx& operator=(wxPaintDCEx&&) = default;
};

#endif
    // _WX_DCCLIENT_H_
