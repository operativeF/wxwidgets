/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/anybutton.h
// Purpose:     wxAnyButton class
// Author:      Julian Smart
// Created:     1997-02-01 (extracted from button.h)
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_ANYBUTTON_H_
#define _WX_MSW_ANYBUTTON_H_

// ----------------------------------------------------------------------------
// Common button functionality
// ----------------------------------------------------------------------------

class wxAnyButton : public wxAnyButtonBase
{
public:
    ~wxAnyButton();
    
    wxAnyButton& operator=(wxAnyButton&&) = delete;

    void SetLabel(const std::string& label) override;
    bool SetBackgroundColour(const wxColour &colour) override;
    bool SetForegroundColour(const wxColour &colour) override;

    WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam) override;

    bool MSWOnDraw(WXDRAWITEMSTRUCT *item) override;

    // returns true if the platform should explicitly apply a theme border
    bool CanApplyThemeBorder() const override { return false; }

protected:
    wxSize DoGetBestSize() const override;

    wxBitmap DoGetBitmap(State which) const override;
    void DoSetBitmap(const wxBitmap& bitmap, State which) override;
    wxSize DoGetBitmapMargins() const override;
    void DoSetBitmapMargins(wxCoord x, wxCoord y) override;
    void DoSetBitmapPosition(wxDirection dir) override;

#if wxUSE_MARKUP
    bool DoSetLabelMarkup(const std::string& markup) override;
#endif // wxUSE_MARKUP

    // Increases the passed in size to account for the button image.
    //
    // Should only be called if we do have a button, i.e. if m_imageData is
    // non-NULL.
    void AdjustForBitmapSize(wxSize& size) const;
    void AdjustForBitmapMargins(wxSize& size) const;

    class wxButtonImageData *m_imageData{nullptr};

#if wxUSE_MARKUP
    class wxMarkupText *m_markupText{nullptr};
#endif // wxUSE_MARKUP

    // Switches button into owner-drawn mode: this is used if we need to draw
    // something not supported by the native control, such as using non default
    // colours or a bitmap on pre-XP systems.
    void MakeOwnerDrawn();
    bool IsOwnerDrawn() const;

    virtual bool MSWIsPushed() const;
};

#endif // _WX_MSW_ANYBUTTON_H_
