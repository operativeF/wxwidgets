/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/dcclient.h
// Purpose:     wxClientDC, wxPaintDC and wxWindowDC classes
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DCCLIENT_H_
#define _WX_DCCLIENT_H_

#include "wx/dc.h"
#include "wx/dcgraph.h"

//-----------------------------------------------------------------------------
// classes
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_FWD_CORE wxPaintDC;
class WXDLLIMPEXP_FWD_CORE wxWindow;

class wxWindowDCImpl: public wxGCDCImpl
{
public:
    wxWindowDCImpl( wxDC *owner );
    wxWindowDCImpl( wxDC *owner, wxWindow *window );
    virtual ~wxWindowDCImpl();

    wxSize DoGetSize() const override;
    wxBitmap DoGetAsBitmap(const wxRect *subrect) const override;

protected:
    wxPoint OSXGetOrigin() const override;

    bool m_release;
    int m_width;
    int m_height;
    wxPoint m_origin;

    wxDECLARE_CLASS(wxWindowDCImpl);
    wxWindowDCImpl(const wxWindowDCImpl&) = delete;
	wxWindowDCImpl& operator=(const wxWindowDCImpl&) = delete;
};


class wxClientDCImpl: public wxWindowDCImpl
{
public:
    wxClientDCImpl( wxDC *owner );
    wxClientDCImpl( wxDC *owner, wxWindow *window );
    virtual ~wxClientDCImpl();

private:
    wxDECLARE_CLASS(wxClientDCImpl);
    wxClientDCImpl(const wxClientDCImpl&) = delete;
	wxClientDCImpl& operator=(const wxClientDCImpl&) = delete;
};


class wxPaintDCImpl: public wxWindowDCImpl
{
public:
    wxPaintDCImpl( wxDC *owner );
    wxPaintDCImpl( wxDC *owner, wxWindow *win );
    virtual ~wxPaintDCImpl();

protected:
    wxDECLARE_CLASS(wxPaintDCImpl);
    wxPaintDCImpl(const wxPaintDCImpl&) = delete;
	wxPaintDCImpl& operator=(const wxPaintDCImpl&) = delete;
};


#endif
    // _WX_DCCLIENT_H_
