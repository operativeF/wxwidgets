/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/font.cpp
// Purpose:     wxFont class
// Author:      Julian Smart
// Modified by:
// Created:     01/02/97
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/msw/private.h"

#include "wx/font.h"

#include "wx/list.h"
#include "wx/utils.h"
#include "wx/log.h"
#include "wx/module.h"

#include "wx/encinfo.h"
#include "wx/fontutil.h"
#include "wx/fontmap.h"

#include "wx/scopeguard.h"

import WX.WinDef;
import Utils.Strings;
import WX.Cmn.SysOpt;

import WX.Win.UniqueHnd;

import <charconv>;
import <cmath>;
import <string>;
import <vector>;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// the mask used to extract the pitch from LOGFONTW::lfPitchAndFamily field
constexpr int PITCH_MASK = FIXED_PITCH | VARIABLE_PITCH;

// ----------------------------------------------------------------------------
// wxFontRefData - the internal description of the font
// ----------------------------------------------------------------------------

using namespace msw::utils;

class wxFontRefData: public wxGDIRefData
{
public:
    explicit wxFontRefData(const wxFontInfo& info = wxFontInfo());

    explicit wxFontRefData(const wxNativeFontInfo& info, WXHFONT hFont = nullptr)
    {
        Init(info, hFont);
    }

    wxFontRefData(const wxFontRefData& data)  
    {
        Init(data.m_nativeFontInfo);
    }

    // operations
    bool Alloc();

    void Free()
    {
        m_hFont.reset();
    }

    // all wxFont accessors
    double GetFractionalPointSize() const
    {
        return m_nativeFontInfo.GetFractionalPointSize();
    }

    wxSize GetPixelSize() const
    {
        return m_nativeFontInfo.GetPixelSize();
    }

    bool IsUsingSizeInPixels() const
    {
        return m_sizeUsingPixels;
    }

    wxFontFamily GetFamily() const
    {
        return m_nativeFontInfo.GetFamily();
    }

    wxFontStyle GetStyle() const
    {
        return m_nativeFontInfo.GetStyle();
    }

    int GetNumericWeight() const
    {
        return m_nativeFontInfo.GetNumericWeight();
    }

    bool GetUnderlined() const
    {
        return m_nativeFontInfo.GetUnderlined();
    }

    bool GetStrikethrough() const
    {
        return m_nativeFontInfo.GetStrikethrough();
    }

    std::string GetFaceName() const
    {
        std::string facename = m_nativeFontInfo.GetFaceName();
        if ( facename.empty() )
        {
            facename = GetMSWFaceName();
            if ( !facename.empty() )
            {
                // cache the face name, it shouldn't change unless the family
                // does and wxNativeFontInfo::SetFamily() resets the face name
                // Don't call this->SetFaceName(), because it deletes the WXHFONT.
                const_cast<wxNativeFontInfo &>(m_nativeFontInfo).SetFaceName(facename);
            }
        }

        return facename;
    }

    wxFontEncoding GetEncoding() const
    {
        return m_nativeFontInfo.GetEncoding();
    }

    WXHFONT GetHFONT() const
    {
        AllocIfNeeded();

        return m_hFont.get();
    }

    bool HasHFONT() const
    {
        return m_hFont != nullptr;
    }

    int GetLogFontHeight() const
    {
        return m_nativeFontInfo.lf.lfHeight;
    }

    int GetLogFontHeightAtPPI(int ppi) const
    {
        return wxNativeFontInfo::GetLogFontHeightAtPPI(
            m_nativeFontInfo.pointSize, ppi);
    }

    // ... and setters: notice that all of them invalidate the currently
    // allocated WXHFONT, if any, so that the next call to GetHFONT() recreates a
    // new one
    void SetFractionalPointSize(double pointSize)
    {
        m_hFont.reset();

        m_nativeFontInfo.SetFractionalPointSize(pointSize);
        m_sizeUsingPixels = false;
    }

    void SetPixelSize(const wxSize& pixelSize)
    {
        wxCHECK_RET( pixelSize.x >= 0, "negative font width" );
        wxCHECK_RET( pixelSize.y != 0, "zero font height" );

        m_hFont.reset();

        m_nativeFontInfo.SetPixelSize(pixelSize);
        m_sizeUsingPixels = true;
    }

    void SetFamily(wxFontFamily family)
    {
        m_hFont.reset();

        m_nativeFontInfo.SetFamily(family);
    }

    void SetStyle(wxFontStyle style)
    {
        m_hFont.reset();

        m_nativeFontInfo.SetStyle(style);
    }

    void SetNumericWeight(int weight)
    {
        m_hFont.reset();

        m_nativeFontInfo.SetNumericWeight(weight);
    }

    bool SetFaceName(const std::string& faceName)
    {
        m_hFont.reset();

        return m_nativeFontInfo.SetFaceName(faceName);
    }

    void SetUnderlined(bool underlined)
    {
        m_hFont.reset();

        m_nativeFontInfo.SetUnderlined(underlined);
    }

    void SetStrikethrough(bool strikethrough)
    {
        m_hFont.reset();

        m_nativeFontInfo.SetStrikethrough(strikethrough);
    }

    void SetEncoding(wxFontEncoding encoding)
    {
        m_hFont.reset();

        m_nativeFontInfo.SetEncoding(encoding);
    }

    void SetLogFontHeight(int height)
    {
        m_hFont.reset();

        m_nativeFontInfo.lf.lfHeight = height;
    }

    const wxNativeFontInfo& GetNativeFontInfo() const
    {
        // we need to create the font now to get the corresponding LOGFONTW if
        // it hadn't been done yet
        AllocIfNeeded();

        // ensure that we have a valid face name in our font information:
        // GetFaceName() will try to retrieve it from our WXHFONT and save it if
        // it was successful
        (void)GetFaceName();

        return m_nativeFontInfo;
    }

    void SetNativeFontInfo(const wxNativeFontInfo& nativeFontInfo)
    {
        m_hFont.reset();

        m_nativeFontInfo = nativeFontInfo;
    }

protected:
    void Init(const wxNativeFontInfo& info, WXHFONT hFont = nullptr);

    void AllocIfNeeded() const
    {
        if ( !m_hFont )
            const_cast<wxFontRefData *>(this)->Alloc();
    }

    // retrieve the face name really being used by the font: this is used to
    // get the face name selected by the system when we don't specify it (but
    // use just the family for example)
    std::string GetMSWFaceName() const
    {
        using msw::utils::unique_dcwnd;

        unique_dcwnd screenDC{::GetDC(nullptr)};

        SelectInHDC selectFont(screenDC.get(), (WXHFONT)GetHFONT());

        WXUINT otmSize = ::GetOutlineTextMetricsW(screenDC.get(), 0, nullptr);
        if ( !otmSize )
        {
            wxLogLastError("GetOutlineTextMetrics(NULL)");
            return {};
        }

        OUTLINETEXTMETRICW* const
            otm = static_cast<OUTLINETEXTMETRICW *>(malloc(otmSize));
        wxON_BLOCK_EXIT1( free, otm );

        otm->otmSize = otmSize;
        if ( !::GetOutlineTextMetricsW(screenDC.get(), otmSize, otm) )
        {
            wxLogLastError("GetOutlineTextMetrics()");
            return {};
        }

        // in spite of its type, the otmpFamilyName field of OUTLINETEXTMETRIC
        // gives an offset in _bytes_ of the face (not family!) name from the
        // struct start while the name itself is an array of TCHARs
        //
        // FWIW otmpFaceName contains the same thing as otmpFamilyName followed
        // by a possible " Italic" or " Bold" or something else suffix
        return boost::nowide::narrow(reinterpret_cast<WCHAR*>(otm) +
                    wxPtrToUInt(otm->otmpFamilyName)/sizeof(WCHAR));
    }

    // Windows font handle, created on demand in GetHFONT()
    unique_font            m_hFont{nullptr};

    // Native font info
    wxNativeFontInfo m_nativeFontInfo;

    // are we using m_nativeFontInfo.lf.lfHeight for point size or pixel size?
    bool             m_sizeUsingPixels;

};

#define M_FONTDATA ((wxFontRefData*)m_refData)

// ----------------------------------------------------------------------------
// wxFontRefData
// ----------------------------------------------------------------------------

wxFontRefData::wxFontRefData(const wxFontInfo& info)
    : m_sizeUsingPixels(info.IsUsingSizeInPixels())
{
    if ( m_sizeUsingPixels )
    {
        m_nativeFontInfo.SetPixelSize(info.GetPixelSize());
    }
    else
    {
        m_nativeFontInfo.SetSizeOrDefault(info.GetFractionalPointSize());
    }

    SetStyle(info.GetStyle());
    SetNumericWeight(info.GetNumericWeight());
    SetUnderlined(info.IsUnderlined());
    SetStrikethrough(info.IsStrikethrough());

    // set the family/facename
    SetFamily(info.GetFamily());
    if ( info.HasFaceName() )
        SetFaceName(info.GetFaceName());

    // deal with encoding now (it may override the font family and facename
    // so do it after setting them)
    SetEncoding(info.GetEncoding());
}

void wxFontRefData::Init(const wxNativeFontInfo& info, WXHFONT hFont)
{
    // hFont may be zero, or it be passed in case we really want to
    // use the exact font created in the underlying system
    // (for example where we can't guarantee conversion from WXHFONT
    // to LOGFONTW back to WXHFONT)
    m_hFont.reset(hFont);
    m_nativeFontInfo = info;

    // size of native fonts is expressed in pixels
    m_sizeUsingPixels = true;
}

bool wxFontRefData::Alloc()
{
    m_hFont.reset(::CreateFontIndirectW(&m_nativeFontInfo.lf));

    if ( !m_hFont )
    {
        wxLogLastError("CreateFont");
        return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
// wxNativeFontInfo
// ----------------------------------------------------------------------------

wxNativeFontInfo::wxNativeFontInfo(const LOGFONTW& lf_, const wxWindow* win)
    : lf(lf_),
      pointSize(GetPointSizeAtPPI(lf.lfHeight, win ? win->GetDPI().y : 0))
{ }

/* static */
double wxNativeFontInfo::GetPointSizeAtPPI(int lfHeight, int ppi)
{
    using msw::utils::unique_dcwnd;

    unique_dcwnd screenDC{::GetDC(nullptr)};

    if ( ppi == 0 )
        ppi = ::GetDeviceCaps(screenDC.get(), LOGPIXELSY);

    return std::abs(lfHeight) * 72.0 / ppi;
}

/* static */
int wxNativeFontInfo::GetLogFontHeightAtPPI(double size, int ppi)
{
    return -(std::lround(size * ppi / 72.0));
}

void wxNativeFontInfo::Init()
{
    wxZeroMemory(lf);

    // we get better font quality if we use PROOF_QUALITY instead of
    // DEFAULT_QUALITY but some fonts (e.g. "Terminal 6pt") are not available
    // then so we allow to set a global option to choose between quality and
    // wider font selection
    lf.lfQuality = wxSystemOptions::GetOptionInt("msw.font.no-proof-quality")
                    ? DEFAULT_QUALITY
                    : PROOF_QUALITY;

    pointSize = 0;
}

double wxNativeFontInfo::GetFractionalPointSize() const
{
    return pointSize;
}

wxSize wxNativeFontInfo::GetPixelSize() const
{
    wxSize ret;
    ret.y = std::abs((int)lf.lfHeight);
    ret.x = lf.lfWidth;
    return ret;
}

wxFontStyle wxNativeFontInfo::GetStyle() const
{
    return lf.lfItalic ? wxFontStyle::Italic : wxFontStyle::Normal;
}

int wxNativeFontInfo::GetNumericWeight() const
{
    return lf.lfWeight;
}

bool wxNativeFontInfo::GetUnderlined() const
{
    return lf.lfUnderline != 0;
}

bool wxNativeFontInfo::GetStrikethrough() const
{
    return lf.lfStrikeOut != 0;
}

std::string wxNativeFontInfo::GetFaceName() const
{
    return boost::nowide::narrow(lf.lfFaceName);
}

wxFontFamily wxNativeFontInfo::GetFamily() const
{
    wxFontFamily family;

    // extract family from pitch-and-family
    switch ( lf.lfPitchAndFamily & ~PITCH_MASK )
    {
        case 0:
            family = wxFontFamily::Unknown;
            break;

        case FF_ROMAN:
            family = wxFontFamily::Roman;
            break;

        case FF_SWISS:
            family = wxFontFamily::Swiss;
            break;

        case FF_SCRIPT:
            family = wxFontFamily::Script;
            break;

        case FF_MODERN:
            family = wxFontFamily::Modern;
            break;

        case FF_DECORATIVE:
            family = wxFontFamily::Decorative;
            break;

        default:
            wxFAIL_MSG( "unknown LOGFONTW::lfFamily value" );
            family = wxFontFamily::Unknown;
                // just to avoid a warning
    }

    return family;
}

wxFontEncoding wxNativeFontInfo::GetEncoding() const
{
    return wxGetFontEncFromCharSet(lf.lfCharSet);
}

void wxNativeFontInfo::SetFractionalPointSize(double pointSizeNew)
{
    // We don't have the correct DPI to use here, so use that of the
    // primary screen and rely on WXAdjustToPPI() changing it later if
    // necessary.

    using msw::utils::unique_dcwnd;

    unique_dcwnd screenDC{::GetDC(nullptr)};

    const int ppi = ::GetDeviceCaps(screenDC.get(), LOGPIXELSY);
    lf.lfHeight = GetLogFontHeightAtPPI(pointSizeNew, ppi);

    pointSize = pointSizeNew;
}

void wxNativeFontInfo::SetPixelSize(const wxSize& pixelSize)
{
    // MSW accepts both positive and negative heights here but they mean
    // different things: positive specifies the cell height while negative
    // specifies the character height. We used to just pass the value to MSW
    // unchanged but changed the behaviour for positive values in 2.9.1 to
    // match other ports and, more importantly, the expected behaviour. So now
    // passing the negative height doesn't make sense at all any more but we
    // still accept it for compatibility with the existing code which worked
    // around the wrong interpretation of the height argument in older wxMSW
    // versions by passing a negative value explicitly itself.
    lf.lfHeight = -std::abs(pixelSize.y);
    lf.lfWidth = pixelSize.x;

    // We don't have the right DPI to use here neither, but we need to update
    // the point size too, so fall back to the default.
    pointSize = GetPointSizeAtPPI(lf.lfHeight);
}

void wxNativeFontInfo::SetStyle(wxFontStyle style)
{
    switch ( style )
    {
        default:
            wxFAIL_MSG( "unknown font style" );
            [[fallthrough]];

        case wxFontStyle::Normal:
            lf.lfItalic = FALSE;
            break;

        case wxFontStyle::Italic:
        case wxFontStyle::Slant:
            lf.lfItalic = TRUE;
            break;
    }
}

void wxNativeFontInfo::SetNumericWeight(int weight)
{
    lf.lfWeight = weight;
}

void wxNativeFontInfo::SetUnderlined(bool underlined)
{
    lf.lfUnderline = underlined;
}

void wxNativeFontInfo::SetStrikethrough(bool strikethrough)
{
    lf.lfStrikeOut = strikethrough;
}

bool wxNativeFontInfo::SetFaceName(const std::string& facename)
{
    auto wideFacename = boost::nowide::widen(facename);
    std::copy(wideFacename.begin(), wideFacename.end(), std::begin(lf.lfFaceName));
    return true;
}

void wxNativeFontInfo::SetFamily(wxFontFamily family)
{
    BYTE ff_family = FF_DONTCARE;

    switch ( family )
    {
        case wxFontFamily::Script:
            ff_family = FF_SCRIPT;
            break;

        case wxFontFamily::Decorative:
            ff_family = FF_DECORATIVE;
            break;

        case wxFontFamily::Roman:
            ff_family = FF_ROMAN;
            break;

        case wxFontFamily::Teletype:
        case wxFontFamily::Modern:
            ff_family = FF_MODERN;
            break;

        case wxFontFamily::Swiss:
        case wxFontFamily::Default:
            ff_family = FF_SWISS;
            break;

        case wxFontFamily::Unknown:
            wxFAIL_MSG( "invalid font family" );
            return;
    }

    wxCHECK_RET( ff_family != FF_DONTCARE, "unknown wxFontFamily" );

    lf.lfPitchAndFamily = (BYTE)(DEFAULT_PITCH) | ff_family;

    // reset the facename so that CreateFontIndirect() will automatically choose a
    // face name based only on the font family.
    lf.lfFaceName[0] = '\0';
}

void wxNativeFontInfo::SetEncoding(wxFontEncoding encoding)
{
    wxNativeEncodingInfo info;
    if ( !wxGetNativeFontEncoding(encoding, &info) )
    {
#if wxUSE_FONTMAP
        if ( wxFontMapper::Get()->GetAltForEncoding(encoding, &info) )
        {
            if ( !info.facename.empty() )
            {
                // if we have this encoding only in some particular facename, use
                // the facename - it is better to show the correct characters in a
                // wrong facename than unreadable text in a correct one
                SetFaceName(info.facename);
            }
        }
        else
#endif // wxUSE_FONTMAP
        {
            // unsupported encoding, replace with the default
            info.charset = DEFAULT_CHARSET;
        }
    }

    lf.lfCharSet = (BYTE)info.charset;
}

bool wxNativeFontInfo::FromString(const std::string& s)
{
    wxStringTokenizer tokenizer(s, ";", wxStringTokenizerMode::RetEmptyAll);

    long l;

    // first the version
    std::string token = tokenizer.GetNextToken();
    std::from_chars_result tokRes = std::from_chars(token.data(), token.data() + token.size(), l);
    if ( tokRes.ec != std::errc() )
        return false;

    // If fractional point size is not present (which can happen if we have a
    // string in version 0 or even with version 1 if it doesn't contain a valid
    // point size), ensure that we set it from lfHeight below.
    bool setPointSizeFromHeight = true;
    switch ( l )
    {
        case 0:
            // Fractional point size is not present in this version, so nothing
            // special to do.
            break;

        case 1:
            {
                const auto doubleStr = tokenizer.GetNextToken();
                double d;
                tokRes = std::from_chars(doubleStr.data(), doubleStr.data() + doubleStr.size(), d);
                if (tokRes.ec != std::errc())
                    return false;

                // If the size is present but 0, ignore it and still use
                // lfHeight, as with v0 strings.
                // FIXME: Double equality
                if ( !(d == 0.0) )
                {
                    pointSize = d;
                    setPointSizeFromHeight = false;
                }
            }
            break;

        default:
            // Unknown version.
            return false;
    }

    token = tokenizer.GetNextToken();
    tokRes = std::from_chars(token.data(), token.data() + token.size(), l);
    if ( tokRes.ec != std::errc() )
        return false;

    lf.lfHeight = l;
    if ( setPointSizeFromHeight )
        pointSize = GetPointSizeAtPPI(l);

    token = tokenizer.GetNextToken();
    tokRes = std::from_chars(token.data(), token.data() + token.size(), l);
    if ( tokRes.ec != std::errc() )
        return false;
    lf.lfWidth = l;

    token = tokenizer.GetNextToken();
    tokRes = std::from_chars(token.data(), token.data() + token.size(), l);
    if ( tokRes.ec != std::errc() )
        return false;
    lf.lfEscapement = l;

    token = tokenizer.GetNextToken();
    tokRes = std::from_chars(token.data(), token.data() + token.size(), l);
    if ( tokRes.ec != std::errc() )
        return false;
    lf.lfOrientation = l;

    token = tokenizer.GetNextToken();
    tokRes = std::from_chars(token.data(), token.data() + token.size(), l);
    if ( tokRes.ec != std::errc() )
        return false;
    lf.lfWeight = l;

    token = tokenizer.GetNextToken();
    tokRes = std::from_chars(token.data(), token.data() + token.size(), l);
    if ( tokRes.ec != std::errc() )
        return false;
    lf.lfItalic = static_cast<BYTE>(l);

    token = tokenizer.GetNextToken();
    tokRes = std::from_chars(token.data(), token.data() + token.size(), l);
    if ( tokRes.ec != std::errc() )
        return false;
    lf.lfUnderline = static_cast<BYTE>(l);

    token = tokenizer.GetNextToken();
    tokRes = std::from_chars(token.data(), token.data() + token.size(), l);
    if ( tokRes.ec != std::errc() )
        return false;
    lf.lfStrikeOut = static_cast<BYTE>(l);

    token = tokenizer.GetNextToken();
    tokRes = std::from_chars(token.data(), token.data() + token.size(), l);
    if ( tokRes.ec != std::errc() )
        return false;
    lf.lfCharSet = static_cast<BYTE>(l);

    token = tokenizer.GetNextToken();
    tokRes = std::from_chars(token.data(), token.data() + token.size(), l);
    if ( tokRes.ec != std::errc() )
        return false;
    lf.lfOutPrecision = static_cast<BYTE>(l);

    token = tokenizer.GetNextToken();
    tokRes = std::from_chars(token.data(), token.data() + token.size(), l);
    if ( tokRes.ec != std::errc() )
        return false;
    lf.lfClipPrecision = static_cast<BYTE>(l);

    token = tokenizer.GetNextToken();
    tokRes = std::from_chars(token.data(), token.data() + token.size(), l);
    if ( tokRes.ec != std::errc() )
        return false;
    lf.lfQuality = static_cast<BYTE>(l);

    token = tokenizer.GetNextToken();
    tokRes = std::from_chars(token.data(), token.data() + token.size(), l);
    if ( tokRes.ec != std::errc() )
        return false;
    lf.lfPitchAndFamily = static_cast<BYTE>(l);

    if ( !tokenizer.HasMoreTokens() )
        return false;

    // the face name may be empty
    SetFaceName(tokenizer.GetNextToken());

    return true;
}

std::string wxNativeFontInfo::ToString() const
{
    // FIXME: Precision?
    auto facename = boost::nowide::narrow(lf.lfFaceName);
    return fmt::format("{:d};{:f};{:d};{:d};{:d};{:d};{:d};{:d};{:d};{:d};{:d};{:d};{:d};{:d};{:d};{}",
             1, // version
             pointSize,
             lf.lfHeight,
             lf.lfWidth,
             lf.lfEscapement,
             lf.lfOrientation,
             lf.lfWeight,
             lf.lfItalic,
             lf.lfUnderline,
             lf.lfStrikeOut,
             lf.lfCharSet,
             lf.lfOutPrecision,
             lf.lfClipPrecision,
             lf.lfQuality,
             lf.lfPitchAndFamily,
             facename);
}

// ----------------------------------------------------------------------------
// wxFont
// ----------------------------------------------------------------------------

wxFont::wxFont(const std::string& fontdesc)
{
    wxNativeFontInfo info;
    if ( info.FromString(fontdesc) )
        Create(info);
}

wxFont::wxFont(const wxFontInfo& info)
{
    m_refData = new wxFontRefData(info);
}

bool wxFont::Create(const wxNativeFontInfo& info, WXHFONT hFont)
{
    UnRef();

    m_refData = new wxFontRefData(info, hFont);

    return RealizeResource();
}

bool wxFont::DoCreate(const wxFontInfo& info)
{
    UnRef();

    m_refData = new wxFontRefData(info);

    return RealizeResource();
}

// ----------------------------------------------------------------------------
// real implementation
// ----------------------------------------------------------------------------

wxGDIRefData *wxFont::CreateGDIRefData() const
{
    return new wxFontRefData();
}

wxGDIRefData *wxFont::CloneGDIRefData(const wxGDIRefData *data) const
{
    return new wxFontRefData(*dynamic_cast<const wxFontRefData *>(data));
}

bool wxFont::RealizeResource()
{
    // NOTE: the GetHFONT() call automatically triggers a reallocation of
    //       the WXHFONT if necessary (will do nothing if we already have the resource);
    //       it returns NULL only if there is a failure in wxFontRefData::Alloc()...
    return GetHFONT() != nullptr;
}

bool wxFont::FreeResource([[maybe_unused]] bool force)
{
    if ( !M_FONTDATA )
        return false;

    M_FONTDATA->Free();

    return true;
}

WXHANDLE wxFont::GetResourceHandle() const
{
    return (WXHANDLE)GetHFONT();
}

WXHFONT wxFont::GetHFONT() const
{
    // NOTE: wxFontRefData::GetHFONT() will automatically call
    //       wxFontRefData::Alloc() if necessary
    return M_FONTDATA ? M_FONTDATA->GetHFONT() : nullptr;
}

bool wxFont::IsFree() const
{
    return M_FONTDATA && !M_FONTDATA->HasHFONT();
}

// ----------------------------------------------------------------------------
// change font attribute: we recreate font when doing it
// ----------------------------------------------------------------------------

void wxFont::SetFractionalPointSize(double pointSize)
{
    AllocExclusive();

    M_FONTDATA->SetFractionalPointSize(pointSize);
}

void wxFont::SetPixelSize(const wxSize& pixelSize)
{
    AllocExclusive();

    M_FONTDATA->SetPixelSize(pixelSize);
}

void wxFont::WXAdjustToPPI(const wxSize& ppi)
{
    // We only use vertical component here as we only adjust LOGFONTW::lfHeight.
    const int heightNew = M_FONTDATA->GetLogFontHeightAtPPI(ppi.y);

    if ( heightNew != M_FONTDATA->GetLogFontHeight() )
    {
        AllocExclusive();

        M_FONTDATA->SetLogFontHeight(heightNew);
    }
}

void wxFont::SetFamily(wxFontFamily family)
{
    AllocExclusive();

    M_FONTDATA->SetFamily(family);
}

void wxFont::SetStyle(wxFontStyle style)
{
    AllocExclusive();

    M_FONTDATA->SetStyle(style);
}

void wxFont::SetNumericWeight(int weight)
{
    AllocExclusive();

    M_FONTDATA->SetNumericWeight(weight);
}

bool wxFont::SetFaceName(const std::string& faceName)
{
    AllocExclusive();

    if ( !M_FONTDATA->SetFaceName(faceName) )
        return false;

    // NB: using win32's GetObject() API on M_FONTDATA->GetHFONT()
    //     to retrieve a LOGFONTW and then compare lf.lfFaceName
    //     with given facename is not reliable at all:
    //     Windows copies the facename given to ::CreateFontIndirect()
    //     without any validity check.
    //     Thus we use wxFontBase::SetFaceName to check if facename
    //     is valid...
    return wxFontBase::SetFaceName(faceName);
}

void wxFont::SetUnderlined(bool underlined)
{
    AllocExclusive();

    M_FONTDATA->SetUnderlined(underlined);
}

void wxFont::SetStrikethrough(bool strikethrough)
{
    AllocExclusive();

    M_FONTDATA->SetStrikethrough(strikethrough);
}

void wxFont::SetEncoding(wxFontEncoding encoding)
{
    AllocExclusive();

    M_FONTDATA->SetEncoding(encoding);
}

void wxFont::DoSetNativeFontInfo(const wxNativeFontInfo& info)
{
    AllocExclusive();

    M_FONTDATA->SetNativeFontInfo(info);
}

// ----------------------------------------------------------------------------
// accessors
// ----------------------------------------------------------------------------

double wxFont::GetFractionalPointSize() const
{
    wxCHECK_MSG( IsOk(), 0, "invalid font" );

    return M_FONTDATA->GetFractionalPointSize();
}

wxSize wxFont::GetPixelSize() const
{
    wxCHECK_MSG( IsOk(), wxDefaultSize, "invalid font" );

    return M_FONTDATA->GetPixelSize();
}

bool wxFont::IsUsingSizeInPixels() const
{
    wxCHECK_MSG( IsOk(), 0, "invalid font" );

    return M_FONTDATA->IsUsingSizeInPixels();
}

wxFontFamily wxFont::DoGetFamily() const
{
    return M_FONTDATA->GetFamily();
}

wxFontStyle wxFont::GetStyle() const
{
    wxCHECK_MSG( IsOk(), wxFontStyle::Max, "invalid font" );

    return M_FONTDATA->GetStyle();
}

int wxFont::GetNumericWeight() const
{
    wxCHECK_MSG(IsOk(), wxFONTWEIGHT_MAX, "invalid font");

    return M_FONTDATA->GetNumericWeight();
}

bool wxFont::GetUnderlined() const
{
    wxCHECK_MSG( IsOk(), false, "invalid font" );

    return M_FONTDATA->GetUnderlined();
}

bool wxFont::GetStrikethrough() const
{
    wxCHECK_MSG( IsOk(), false, "invalid font" );

    return M_FONTDATA->GetStrikethrough();
}

std::string wxFont::GetFaceName() const
{
    wxCHECK_MSG( IsOk(), {}, "invalid font" );

    return M_FONTDATA->GetFaceName();
}

wxFontEncoding wxFont::GetEncoding() const
{
    wxCHECK_MSG( IsOk(), wxFONTENCODING_DEFAULT, "invalid font" );

    return M_FONTDATA->GetEncoding();
}

const wxNativeFontInfo *wxFont::GetNativeFontInfo() const
{
    return IsOk() ? &(M_FONTDATA->GetNativeFontInfo()) : nullptr;
}

bool wxFont::IsFixedWidth() const
{
    wxCHECK_MSG( IsOk(), false, "invalid font" );

    // LOGFONTW doesn't contain the correct pitch information so we need to call
    // GetTextMetrics() to get it
    using msw::utils::unique_dcwnd;

    unique_dcwnd screenDC{::GetDC(nullptr)};

    SelectInHDC selectFont(screenDC.get(), M_FONTDATA->GetHFONT());

    TEXTMETRICW tm;
    if ( !::GetTextMetricsW(screenDC.get(), &tm) )
    {
        wxLogLastError("GetTextMetrics");
        return false;
    }

    // Quoting MSDN description of TMPF_FIXED_PITCH: "Note very carefully that
    // those meanings are the opposite of what the constant name implies."
    return !(tm.tmPitchAndFamily & TMPF_FIXED_PITCH);
}

// ----------------------------------------------------------------------------
// Private fonts support
// ----------------------------------------------------------------------------

#if wxUSE_PRIVATE_FONTS

namespace
{

// Contains the file names of all fonts added by AddPrivateFont().
std::vector<std::string> gs_privateFontFileNames;

} // anonymous namespace

// Accessor for use in src/msw/graphics.cpp only.
extern const std::vector<std::string>& wxGetPrivateFontFileNames()
{
    return gs_privateFontFileNames;
}

// We need to use a module to clean up the list of private fonts when the
// library is shut down.
class wxPrivateFontsListModule : public wxModule
{
public:
    bool OnInit() override { return true; }
    void OnExit() override { gs_privateFontFileNames.clear(); }

private:
    wxDECLARE_DYNAMIC_CLASS(wxPrivateFontsListModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxPrivateFontsListModule, wxModule);

bool wxFontBase::AddPrivateFont(const std::string& filename)
{
    if ( !::AddFontResourceExW(boost::nowide::widen(filename).c_str(), FR_PRIVATE, nullptr) )
    {
        wxLogSysError(_("Font file \"%s\" couldn't be loaded"), filename);
        return false;
    }

    // Remember it for use in wxGDIPlusRenderer::Load().
    gs_privateFontFileNames.push_back(filename);
    return true;
}

#endif // wxUSE_PRIVATE_FONTS
