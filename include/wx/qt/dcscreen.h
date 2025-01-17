/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/dcscreen.h
// Author:      Sean D'Epagnier
// Copyright:   (c) Sean D'Epagnier
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_QT_DCSCREEN_H_
#define _WX_QT_DCSCREEN_H_

#include "wx/qt/dcclient.h"

class wxScreenDCImpl : public wxWindowDCImpl
{
public:
    wxScreenDCImpl( wxScreenDC *owner );

    ~wxScreenDCImpl();

protected:
    wxSize DoGetSize() const override;
    QPixmap *GetQPixmap() override;

    wxDECLARE_ABSTRACT_CLASS(wxScreenDCImpl);
};

#endif // _WX_QT_DCSCREEN_H_
