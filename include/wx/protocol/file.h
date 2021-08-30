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

#include "wx/defs.h"

#if wxUSE_PROTOCOL_FILE

#include "wx/protocol/protocol.h"

class WXDLLIMPEXP_NET wxFileProto: public wxProtocol
{
public:
    wxFileProto() = default;
    ~wxFileProto() = default;

    wxFileProto(const wxFileProto&) = delete;
    wxFileProto& operator=(const wxFileProto&) = delete;
    wxFileProto(wxFileProto&&) = default;
    wxFileProto& operator=(wxFileProto&&) = default;


    bool Abort() override { return true; }
    wxString GetContentType() const override { return wxEmptyString; }

    wxInputStream *GetInputStream(const wxString& path) override;

	wxClassInfo *GetClassInfo() const override;
	static wxClassInfo ms_classInfo;
	static wxObject* wxCreateObject();
    DECLARE_PROTOCOL(wxFileProto)
};

#endif // wxUSE_PROTOCOL_FILE

#endif // __WX_PROTO_FILE_H__
