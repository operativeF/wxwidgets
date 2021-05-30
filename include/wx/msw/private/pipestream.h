///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/private/pipestream.h
// Purpose:     MSW wxPipeInputStream and wxPipeOutputStream declarations
// Author:      Vadim Zeitlin
// Created:     2013-06-08 (extracted from src/msw/utilsexc.cpp)
// Copyright:   (c) 2013 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_PRIVATE_PIPESTREAM_H_
#define _WX_MSW_PRIVATE_PIPESTREAM_H_

class wxPipeInputStream : public wxInputStream
{
public:
    explicit wxPipeInputStream(HANDLE hInput);
    ~wxPipeInputStream() override;

    // returns true if the pipe is still opened
    bool IsOpened() const { return m_hInput != INVALID_HANDLE_VALUE; }

    // returns true if there is any data to be read from the pipe
    bool CanRead() const override;

protected:
    size_t OnSysRead(void *buffer, size_t len) override;

protected:
    HANDLE m_hInput;

    wxPipeInputStream(const wxPipeInputStream&) = delete;
	wxPipeInputStream& operator=(const wxPipeInputStream&) = delete;
};

class wxPipeOutputStream: public wxOutputStream
{
public:
    explicit wxPipeOutputStream(HANDLE hOutput);
    ~wxPipeOutputStream() override { Close(); }
    bool Close() override;

protected:
    size_t OnSysWrite(const void *buffer, size_t len) override;

protected:
    HANDLE m_hOutput;

    wxPipeOutputStream(const wxPipeOutputStream&) = delete;
	wxPipeOutputStream& operator=(const wxPipeOutputStream&) = delete;
};

#endif // _WX_MSW_PRIVATE_PIPESTREAM_H_
