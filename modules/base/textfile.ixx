///////////////////////////////////////////////////////////////////////////////
// Name:        wx/textfile.h
// Purpose:     class wxTextFile to work with text files of _small_ size
//              (file is fully loaded in memory) and which understands CR/LF
//              differences between platforms.
// Author:      Vadim Zeitlin
// Modified by:
// Created:     03.04.98
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/textbuf.h"
#include "wx/file.h"

export module WX.Cmn.TextFile;

import <string>;

// ----------------------------------------------------------------------------
// wxTextFile
// ----------------------------------------------------------------------------

export class wxTextFile : public wxTextBuffer
{
public:
    // constructors
    wxTextFile() = default;
    wxTextFile(const std::string& strFileName);

    wxTextFile& operator=(wxTextFile&&) = delete;

protected:
    // implement the base class pure virtuals
    bool OnExists() const override;
    bool OnOpen(const std::string &strBufferName,
                        wxTextBufferOpenMode openMode) override;
    bool OnClose() override;
    bool OnRead(const wxMBConv& conv) override;
    bool OnWrite(wxTextFileType typeNew, const wxMBConv& conv) override;

private:
    wxFile m_file;
}; // export class wxTextFile
