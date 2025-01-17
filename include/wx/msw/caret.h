///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/caret.h
// Purpose:     wxCaret class - the MSW implementation of wxCaret
// Author:      Vadim Zeitlin
// Modified by:
// Created:     23.05.99
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CARET_H_
#define _WX_CARET_H_

class wxCaret : public wxCaretBase
{
public:
    wxCaret() = default;
        // create the caret of given (in pixels) width and height and associate
        // with the given window
    wxCaret(wxWindowBase *window, const wxSize& size)
    {
        Create(window, size);
    }

    wxCaret& operator=(wxCaret&&) = delete;

    // process wxWindow notifications
    void OnSetFocus() override;
    void OnKillFocus() override;

protected:
    // override base class virtuals
    void DoMove() override;
    void DoShow() override;
    void DoHide() override;
    void DoSize() override;

    // helper function which creates the system caret
    bool MSWCreateCaret();

private:
    bool m_hasCaret{false};
};

#endif // _WX_CARET_H_


