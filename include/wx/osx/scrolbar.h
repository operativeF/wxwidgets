/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/scrolbar.h
// Purpose:     wxScrollBar class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_SCROLBAR_H_
#define _WX_SCROLBAR_H_

// Scrollbar item
class wxScrollBar : public wxScrollBarBase
{
public:
    wxScrollBar() { m_pageSize = 0; m_viewSize = 0; m_objectSize = 0; }
    virtual ~wxScrollBar();

    wxScrollBar(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxSB_HORIZONTAL,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxASCII_STR(wxScrollBarNameStr))
    {
        Create(parent, id, pos, size, style, validator, name);
    }
    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxSB_HORIZONTAL,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxASCII_STR(wxScrollBarNameStr));

    int GetThumbPosition() const override;
    int GetThumbSize() const override { return m_viewSize; }
    int GetPageSize() const override { return m_pageSize; }
    int GetRange() const override { return m_objectSize; }

    void SetThumbPosition(int viewStart) override;
    virtual void SetScrollbar(int position, int thumbSize, int range,
            int pageSize, bool refresh = true) override;

    // needed for RTTI
    void SetThumbSize( int s ) { SetScrollbar( GetThumbPosition() , s , GetRange() , GetPageSize() , true ) ; }
    void SetPageSize( int s ) { SetScrollbar( GetThumbPosition() , GetThumbSize() , GetRange() , s , true ) ; }
    void SetRange( int s ) { SetScrollbar( GetThumbPosition() , GetThumbSize() , s , GetPageSize() , true ) ; }

        // implementation only from now on
    void Command(wxCommandEvent& event) override;
    void TriggerScrollEvent( wxEventType scrollEvent ) override;
    bool OSXHandleClicked( double timestampsec ) override;
protected:
    wxSize DoGetBestSize() const override;

    int m_pageSize;
    int m_viewSize;
    int m_objectSize;

    wxDECLARE_DYNAMIC_CLASS(wxScrollBar);
    wxDECLARE_EVENT_TABLE();
};

#endif // _WX_SCROLBAR_H_
