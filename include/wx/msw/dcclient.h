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

class wxWindowDCImpl : public wxMSWDCImpl
{
public:
    // default ctor
    wxWindowDCImpl( wxDC *owner );

    // Create a DC corresponding to the whole window
    wxWindowDCImpl( wxDC *owner, wxWindow *win );

	wxWindowDCImpl& operator=(wxWindowDCImpl&&) = delete;

    wxSize DoGetSize() const override;

protected:
    // initialize the newly created DC
    void InitDC();

    wxDECLARE_CLASS(wxWindowDCImpl);
};

class wxClientDCImpl : public wxWindowDCImpl
{
public:
    // default ctor
    wxClientDCImpl( wxDC *owner );

    // Create a DC corresponding to the client area of the window
    wxClientDCImpl( wxDC *owner, wxWindow *win );

	wxClientDCImpl& operator=(wxClientDCImpl&&) = delete;

    wxSize DoGetSize() const override;

protected:
    void InitDC();

    wxDECLARE_CLASS(wxClientDCImpl);
};

class wxPaintDCImpl : public wxClientDCImpl
{
public:
    wxPaintDCImpl( wxDC *owner );

    // Create a DC corresponding for painting the window in OnPaint()
    wxPaintDCImpl( wxDC *owner, wxWindow *win );

    ~wxPaintDCImpl();

	wxPaintDCImpl& operator=(wxPaintDCImpl&&) = delete;

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

struct wxPaintDCEx : public wxPaintDC
{
    wxPaintDCEx(wxWindow *canvas, WXHDC dc);
    wxPaintDCEx& operator=(wxPaintDCEx&&) = delete;

    wxDECLARE_CLASS(wxPaintDCEx);
};

#endif
    // _WX_DCCLIENT_H_
