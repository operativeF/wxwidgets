/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/font.h
// Purpose:     wxFont class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FONT_H_
#define _WX_FONT_H_

import WX.WinDef;

import Utils.Geometry;

import <string>;

// ----------------------------------------------------------------------------
// wxFont
// ----------------------------------------------------------------------------

class wxFont : public wxFontBase
{
public:
    wxFont() = default;
    wxFont(const wxFontInfo& info);

    wxFont(int size,
           wxFontFamily family,
           wxFontStyle style,
           wxFontWeight weight,
           bool underlined = false,
           const std::string& face = {},
           wxFontEncoding encoding = wxFONTENCODING_DEFAULT)
    {
        Create(size, family, style, weight, underlined, face, encoding);
    }

    [[maybe_unused]] bool Create(int size,
                wxFontFamily family,
                wxFontStyle style,
                wxFontWeight weight,
                bool underlined = false,
                const std::string& face = {},
                wxFontEncoding encoding = wxFONTENCODING_DEFAULT)
    {
        return DoCreate(InfoFromLegacyParams(size,
                                             family,
                                             style,
                                             weight,
                                             underlined,
                                             face,
                                             encoding));
    }

    wxFont(const wxSize& pixelSize,
           wxFontFamily family,
           wxFontStyle style,
           wxFontWeight weight,
           bool underlined = false,
           const std::string& face = {},
           wxFontEncoding encoding = wxFONTENCODING_DEFAULT)
    {
        Create(pixelSize, family, style, weight,
                     underlined, face, encoding);
    }

    wxFont(const wxNativeFontInfo& info, WXHFONT hFont = nullptr)
    {
        Create(info, hFont);
    }

    wxFont(const std::string& fontDesc);


    [[maybe_unused]] bool Create(const wxSize& pixelSize,
                wxFontFamily family,
                wxFontStyle style,
                wxFontWeight weight,
                bool underlined = false,
                const std::string& face = {},
                wxFontEncoding encoding = wxFONTENCODING_DEFAULT)
    {
        return DoCreate(InfoFromLegacyParams(pixelSize,
                                             family,
                                             style,
                                             weight,
                                             underlined,
                                             face,
                                             encoding));
    }

    [[maybe_unused]] bool Create(const wxNativeFontInfo& info, WXHFONT hFont = nullptr);

    
    double GetFractionalPointSize() const override;
    wxSize GetPixelSize() const override;
    bool IsUsingSizeInPixels() const override;
    wxFontStyle GetStyle() const override;
    int GetNumericWeight() const override;
    bool GetUnderlined() const override;
    bool GetStrikethrough() const override;
    std::string GetFaceName() const override;
    wxFontEncoding GetEncoding() const override;
    const wxNativeFontInfo *GetNativeFontInfo() const override;

    void SetFractionalPointSize(double pointSize) override;
    void SetPixelSize(const wxSize& pixelSize) override;
    void SetFamily(wxFontFamily family) override;
    void SetStyle(wxFontStyle style) override;
    void SetNumericWeight(int weight) override;
    bool SetFaceName(const std::string& faceName) override;
    void SetUnderlined(bool underlined) override;
    void SetStrikethrough(bool strikethrough) override;
    void SetEncoding(wxFontEncoding encoding) override;

    wxDECLARE_COMMON_FONT_METHODS();

    bool IsFixedWidth() const override;

    // MSW needs to modify the font object when the DPI of the window it
    // is used with changes, this function can be used to do it.
    //
    // This method is not considered to be part of wxWidgets public API.
    void WXAdjustToPPI(const wxSize& ppi);

    // implementation only from now on
    // -------------------------------

    bool IsFree() const override;
    bool RealizeResource() override;
    WXHANDLE GetResourceHandle() const override;
    bool FreeResource(bool force = false) override;

    // for consistency with other wxMSW classes
    WXHFONT GetHFONT() const;

protected:
    // Common helper of overloaded Create() methods.
    bool DoCreate(const wxFontInfo& info);

    void DoSetNativeFontInfo(const wxNativeFontInfo& info) override;
    wxFontFamily DoGetFamily() const override;

    // implement wxObject virtuals which are used by AllocExclusive()
    wxGDIRefData *CreateGDIRefData() const override;
    wxGDIRefData *CloneGDIRefData(const wxGDIRefData *data) const override;

private:
    wxDECLARE_DYNAMIC_CLASS(wxFont);
};

#endif // _WX_FONT_H_
