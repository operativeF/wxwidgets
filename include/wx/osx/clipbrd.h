/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/clipbrd.h
// Purpose:     Clipboard functionality.
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CLIPBRD_H_
#define _WX_CLIPBRD_H_

#if wxUSE_CLIPBOARD

#include "wx/osx/core/cfref.h"

//-----------------------------------------------------------------------------
// wxClipboard
//-----------------------------------------------------------------------------

class wxClipboard : public wxClipboardBase
{
public:
    wxClipboard();
    virtual ~wxClipboard();

    // open the clipboard before SetData() and GetData()
    bool Open() override;

    // close the clipboard after SetData() and GetData()
    void Close() override;

    // query whether the clipboard is opened
    bool IsOpened() const override;

    // set the clipboard data. all other formats will be deleted.
    bool SetData( wxDataObject *data ) override;

    // add to the clipboard data.
    bool AddData( wxDataObject *data ) override;

    // ask if data in correct format is available
    bool IsSupported( const wxDataFormat& format ) override;

    // fill data with data on the clipboard (if available)
    bool GetData( wxDataObject& data ) override;

    // clears wxTheClipboard and the system's clipboard if possible
    void Clear() override;

    // flushes the clipboard: this means that the data which is currently on
    // clipboard will stay available even after the application exits (possibly
    // eating memory), otherwise the clipboard will be emptied on exit
    bool Flush() override;

private:
    wxDataObject     *m_data;
    bool              m_open;
    wxCFRef<PasteboardRef> m_pasteboard;

    wxDECLARE_DYNAMIC_CLASS(wxClipboard);
};

#endif // wxUSE_CLIPBOARD

#endif // _WX_CLIPBRD_H_
