/////////////////////////////////////////////////////////////////////////////
// Name:        wx/dcclient.h
// Purpose:     wxClientDC base header
// Author:      Julian Smart
// Copyright:   (c) Julian Smart
// Licence:     wxWindows Licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_DCCLIENT_H_BASE_
#define _WX_DCCLIENT_H_BASE_

#include "wx/dc.h"

//-----------------------------------------------------------------------------
// wxWindowDC
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxWindowDC : public wxDC
{
public:
    wxWindowDC(wxWindow *win);

protected:
    wxWindowDC(std::unique_ptr<wxDCImpl> impl) : wxDC(std::move(impl)) { }

private:
    wxDECLARE_ABSTRACT_CLASS(wxWindowDC);
};

//-----------------------------------------------------------------------------
// wxClientDC
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxClientDC : public wxWindowDC
{
public:
    wxClientDC(wxWindow *win);

protected:
    wxClientDC(std::unique_ptr<wxDCImpl> impl) : wxWindowDC(std::move(impl)) { }

private:
    wxDECLARE_ABSTRACT_CLASS(wxClientDC);
};

//-----------------------------------------------------------------------------
// wxPaintDC
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_CORE wxPaintDC : public wxClientDC
{
public:
    wxPaintDC(wxWindow *win);

protected:
    wxPaintDC(std::unique_ptr<wxDCImpl> impl) : wxClientDC(std::move(impl)) { }

private:
    wxDECLARE_ABSTRACT_CLASS(wxPaintDC);
};

#endif // _WX_DCCLIENT_H_BASE_
