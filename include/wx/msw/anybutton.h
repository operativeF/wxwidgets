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
// button image data
// ----------------------------------------------------------------------------

// we use different data classes for owner drawn buttons and for themed XP ones

class wxButtonImageData
{
public:
    virtual ~wxButtonImageData() = default;

	wxButtonImageData& operator=(wxButtonImageData&&) = delete;

    virtual wxBitmap GetBitmap(wxAnyButtonBase::State which) const = 0;
    virtual void SetBitmap(const wxBitmap& bitmap, wxAnyButtonBase::State which) = 0;

    virtual wxSize GetBitmapMargins() const = 0;
    virtual void SetBitmapMargins(wxCoord x, wxCoord y) = 0;

    virtual wxDirection GetBitmapPosition() const = 0;
    virtual void SetBitmapPosition(wxDirection dir) = 0;
};

// ----------------------------------------------------------------------------
// Common button functionality
// ----------------------------------------------------------------------------

class wxAnyButton : public wxAnyButtonBase
{
public:
    ~wxAnyButton();
    
    wxAnyButton& operator=(wxAnyButton&&) = delete;

    void SetLabel(std::string_view label) override;
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

    std::unique_ptr<wxButtonImageData> m_imageData;

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
