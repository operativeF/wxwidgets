/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/sckfile.cpp
// Purpose:     File protocol
// Author:      Guilhem Lavaux
// Modified by:
// Created:     20/07/97
// Copyright:   (c) 1997, 1998 Guilhem Lavaux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if wxUSE_STREAMS && wxUSE_PROTOCOL_FILE

#include "wx/uri.h"
#include "wx/protocol/file.h"

import WX.Cmn.WFStream;

// ----------------------------------------------------------------------------
// wxFileProto
// ----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxFileProto, wxProtocol);
IMPLEMENT_PROTOCOL(wxFileProto, "file", NULL, false)

wxInputStream *wxFileProto::GetInputStream(const std::string& path)
{
    wxFileInputStream *retval = new wxFileInputStream(wxURI::Unescape(path));
    if ( retval->IsOk() )
    {
        m_lastError = wxPROTO_NOERR;
        return retval;
    }

    m_lastError = wxPROTO_NOFILE;
    delete retval;

    return nullptr;
}

#endif // wxUSE_STREAMS && wxUSE_PROTOCOL_FILE
