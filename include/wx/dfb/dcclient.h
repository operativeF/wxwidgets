/////////////////////////////////////////////////////////////////////////////
// Name:        wx/dfb/dcclient.h
// Purpose:     wxWindowDCImpl, wxClientDCImpl and wxPaintDCImpl
// Author:      Vaclav Slavik
// Created:     2006-08-10
// Copyright:   (c) 2006 REA Elektronik GmbH
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DFB_DCCLIENT_H_
#define _WX_DFB_DCCLIENT_H_

#include "wx/dfb/dc.h"

class wxWindow;

//-----------------------------------------------------------------------------
// wxWindowDCImpl
//-----------------------------------------------------------------------------

class wxWindowDCImpl : public wxDFBDCImpl
{
public:
    wxWindowDCImpl(wxDC *owner) : wxDFBDCImpl(owner), m_shouldFlip(false) { }
    wxWindowDCImpl(wxDC *owner, wxWindow *win);
    ~wxWindowDCImpl();

protected:
    // initializes the DC for painting on given window; if rect!=NULL, then
    // for painting only on the given region of the window
    void InitForWin(wxWindow *win, const wxRect *rect);

private:
    wxRect    m_winRect; // rectangle of the window being painted

    bool m_shouldFlip; // flip the surface when done?

    friend class wxOverlayImpl; // for m_shouldFlip;

    wxDECLARE_DYNAMIC_CLASS(wxWindowDCImpl);
    wxWindowDCImpl(const wxWindowDCImpl&) = delete;
	wxWindowDCImpl& operator=(const wxWindowDCImpl&) = delete;
};

//-----------------------------------------------------------------------------
// wxClientDCImpl
//-----------------------------------------------------------------------------

class wxClientDCImpl : public wxWindowDCImpl
{
public:
    wxClientDCImpl(wxDC *owner) : wxWindowDCImpl(owner) { }
    wxClientDCImpl(wxDC *owner, wxWindow *win);

    wxDECLARE_DYNAMIC_CLASS(wxClientDCImpl);
    wxClientDCImpl(const wxClientDCImpl&) = delete;
	wxClientDCImpl& operator=(const wxClientDCImpl&) = delete;
};


//-----------------------------------------------------------------------------
// wxPaintDCImpl
//-----------------------------------------------------------------------------

class wxPaintDCImpl : public wxClientDCImpl
{
public:
    wxPaintDCImpl(wxDC *owner) : wxClientDCImpl(owner) { }
    wxPaintDCImpl(wxDC *owner, wxWindow *win) : wxClientDCImpl(owner, win) { }

    wxDECLARE_DYNAMIC_CLASS(wxPaintDCImpl);
    wxPaintDCImpl(const wxPaintDCImpl&) = delete;
	wxPaintDCImpl& operator=(const wxPaintDCImpl&) = delete;
};

#endif // _WX_DFB_DCCLIENT_H_
