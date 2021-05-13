/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/dcclient.h
// Author:      Peter Most, Javier Torres, Mariano Reingart
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_DCCLIENT_H_
#define _WX_QT_DCCLIENT_H_

#include "wx/qt/dc.h"

#include "wx/scopedptr.h"

class QPicture;

class WXDLLIMPEXP_CORE wxWindowDCImpl : public wxQtDCImpl
{
public:
    wxWindowDCImpl( wxDC *owner );
    wxWindowDCImpl( wxDC *owner, wxWindow *win );

    ~wxWindowDCImpl();

protected:
    wxWindow *m_window;

private:
    wxDECLARE_CLASS(wxWindowDCImpl);
    wxWindowDCImpl(const wxWindowDCImpl&) = delete;
	wxWindowDCImpl& operator=(const wxWindowDCImpl&) = delete;
};


class WXDLLIMPEXP_CORE wxClientDCImpl : public wxWindowDCImpl
{
public:
    wxClientDCImpl( wxDC *owner );
    wxClientDCImpl( wxDC *owner, wxWindow *win );

    ~wxClientDCImpl();
private:
    wxScopedPtr<QPicture> m_pict;

    wxDECLARE_CLASS(wxClientDCImpl);
    wxClientDCImpl(const wxClientDCImpl&) = delete;
	wxClientDCImpl& operator=(const wxClientDCImpl&) = delete;
};


class WXDLLIMPEXP_CORE wxPaintDCImpl : public wxWindowDCImpl
{
public:
    wxPaintDCImpl( wxDC *owner );
    wxPaintDCImpl( wxDC *owner, wxWindow *win );
private:
    wxDECLARE_CLASS(wxPaintDCImpl);
    wxPaintDCImpl(const wxPaintDCImpl&) = delete;
	wxPaintDCImpl& operator=(const wxPaintDCImpl&) = delete;
};

#endif // _WX_QT_DCCLIENT_H_
