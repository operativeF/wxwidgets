///////////////////////////////////////////////////////////////////////////////
// Name:        wx/memtext.h
// Purpose:     wxMemoryText allows to use wxTextBuffer without a file
// Created:     14.11.01
// Author:      Morten Hanssen
// Copyright:   (c) 2001 Morten Hanssen
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MEMTEXT_H
#define _WX_MEMTEXT_H

// there is no separate setting for wxMemoryText, it's smallish anyhow
#if wxUSE_TEXTBUFFER

#include "wx/textbuf.h"

import <string>;

class wxMemoryText : public wxTextBuffer
{
public:
    wxMemoryText() = default;
    wxMemoryText(const std::string& name) : wxTextBuffer(name) { }

    wxMemoryText& operator=(wxMemoryText&&) = delete;

protected:
    bool OnExists() const override
        { return false; }

    bool OnOpen([[maybe_unused]] const std::string& strBufferName,
                        [[maybe_unused]] wxTextBufferOpenMode OpenMode) override
        { return true; }

    bool OnClose() override
        { return true; }

    bool OnRead([[maybe_unused]] const wxMBConv& conv) override
        { return true; }

    bool OnWrite([[maybe_unused]] wxTextFileType typeNew,
                         [[maybe_unused]] const wxMBConv& conv = wxMBConvUTF8()) override
        { return true; }
};

#endif // wxUSE_TEXTBUFFER

#endif // _WX_MEMTEXT_H

