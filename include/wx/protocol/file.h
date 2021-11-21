/////////////////////////////////////////////////////////////////////////////
// Name:        wx/protocol/file.h
// Purpose:     File protocol
// Author:      Guilhem Lavaux
// Modified by:
// Created:     1997
// Copyright:   (c) 1997, 1998 Guilhem Lavaux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_PROTO_FILE_H__
#define __WX_PROTO_FILE_H__

#if wxUSE_PROTOCOL_FILE

#include "wx/defs.h"

#include "wx/protocol/protocol.h"

class WXDLLIMPEXP_NET wxFileProto: public wxProtocol
{
public:
    wxFileProto& operator=(wxFileProto&&) = delete;


    bool Abort() override { return true; }
    wxString GetContentType() const override { return {}; }

    wxInputStream *GetInputStream(const wxString& path) override;

	wxClassInfo *wxGetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
    DECLARE_PROTOCOL(wxFileProto)
};

#endif // wxUSE_PROTOCOL_FILE

#endif // __WX_PROTO_FILE_H__
