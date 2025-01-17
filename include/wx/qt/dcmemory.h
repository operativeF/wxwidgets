/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/dcmemory.h
// Author:      Peter Most, Javier Torres, Mariano Reingart
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_DCMEMORY_H_
#define _WX_QT_DCMEMORY_H_

#include "wx/qt/dcclient.h"

class wxMemoryDCImpl : public wxQtDCImpl
{
public:
    wxMemoryDCImpl( wxMemoryDC *owner );
    wxMemoryDCImpl( wxMemoryDC *owner, wxBitmap& bitmap );
    wxMemoryDCImpl( wxMemoryDC *owner, wxDC *dc );
    ~wxMemoryDCImpl();

    wxBitmap DoGetAsBitmap(const wxRect *subrect) const override;
    void DoSelect(const wxBitmap& bitmap) override;

    const wxBitmap& GetSelectedBitmap() const override;
    wxBitmap& GetSelectedBitmap() override;

private:
    wxBitmap m_selected;

    DECLARE_CLASS(wxMemoryDCImpl);
    DECLARE_NO_COPY_CLASS(wxMemoryDCImpl);
};

#endif // _WX_QT_DCMEMORY_H_
