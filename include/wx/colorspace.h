/////////////////////////////////////////////////////////////////////////////
// Name:        wx/colorspace.h
// Purpose:     Color Space definitions
// Author:      Thomas Figueroa
// Modified by:
// Created:     2021-10-25
// Copyright:   Thomas Figueroa
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _COLOR_SPACE_H
#define _COLOR_SPACE_H

#include <concepts>
#include <utility>

namespace wx::color::colorspace::rgb
{

template<typename ColorSpaceT>
concept RGBColorSpaceable = requires
{
    typename ColorSpaceT::alpha;      // Transfer function offset
    typename ColorSpaceT::beta;       // Linear-domain threshold
    typename ColorSpaceT::gamma;      // Decoding gamma
    typename ColorSpaceT::delta;      // Linear gain
    typename ColorSpaceT::betadelta;  // Transition point
    typename ColorSpaceT::WhitePoint; // Standard illuminant point
    typename ColorSpaceT::RedPoint;   // Primary red point
    typename ColorSpaceT::GreenPoint; // Primary green point
    typename ColorSpaceT::BluePoint;  // Primary blue point
};

// Reference white points
constexpr auto wp_D65            = std::pair{0.3127, 0.3290};
constexpr auto wp_C              = std::pair{0.3101, 0.3162};
constexpr auto wp_D50_ISO22028   = std::pair{0.3457, 0.3585};
constexpr auto wp_6300K          = std::pair{0.3140, 0.3510};
constexpr auto wp_D50            = std::pair{0.34567, 0.35850};
constexpr auto wp_E              = std::pair{1.0 / 3.0, 1.0 / 3.0};

struct sRGB
{
    static constexpr auto alpha      = 1.055;
    static constexpr auto beta       = 0.0031308;
    static constexpr auto gamma      = 12.0 / 5.0;
    static constexpr auto delta      = 12.92;
    static constexpr auto betadelta  = 0.04045;
    static constexpr auto WhitePoint = wp_D65;
    static constexpr auto RedPoint   = std::pair{0.64, 0.33};
    static constexpr auto GreenPoint = std::pair{0.30, 0.60};
    static constexpr auto BluePoint  = std::pair{0.15, 0.06};
};

using IEC_61966_2_1 = sRGB;

struct HDTV
{
    static constexpr auto alpha      = 1.099;
    static constexpr auto beta       = 0.004;
    static constexpr auto gamma      = 20.0 / 9.0;
    static constexpr auto delta      = 4.5;
    static constexpr auto betadelta  = 0.018;
    static constexpr auto WhitePoint = wp_D65;
    static constexpr auto RedPoint   = std::pair{0.64, 0.33};
    static constexpr auto GreenPoint = std::pair{0.30, 0.60};
    static constexpr auto BluePoint  = std::pair{0.15, 0.06};
};

using BT_709 = HDTV;

struct AdobeRGB98
{
    static constexpr auto alpha      = 1.0;
    static constexpr auto beta       = 0.0;
    static constexpr auto gamma      = 563.0 / 256.0;
    static constexpr auto delta      = 1.0;
    static constexpr auto betadelta  = 0.0;
    static constexpr auto WhitePoint = wp_D65;
    static constexpr auto RedPoint   = std::pair{0.64, 0.33};
    static constexpr auto GreenPoint = std::pair{0.21, 0.71};
    static constexpr auto BluePoint  = std::pair{0.15, 0.06};
};

struct PAL
{
    static constexpr auto alpha      = 1.0;
    static constexpr auto beta       = 0.0;
    static constexpr auto gamma      = 14.0 / 5.0;
    static constexpr auto delta      = 1.0;
    static constexpr auto betadelta  = 0.0;
    static constexpr auto WhitePoint = wp_D65;
    static constexpr auto RedPoint   = std::pair{0.64, 0.33};
    static constexpr auto GreenPoint = std::pair{0.29, 0.60};
    static constexpr auto BluePoint  = std::pair{0.15, 0.06};
};

using SECAM  = PAL;
using BT_601_G = PAL;

struct NTSC
{
    static constexpr auto alpha      = 1.1115;
    static constexpr auto beta       = 0.0057;
    static constexpr auto gamma      = 20.0 / 9.0;
    static constexpr auto delta      = 4.0;
    static constexpr auto betadelta  = 0.0228;
    static constexpr auto WhitePoint = wp_D65;
    static constexpr auto RedPoint   = std::pair{0.64, 0.33};
    static constexpr auto GreenPoint = std::pair{0.30, 0.60};
    static constexpr auto BluePoint  = std::pair{0.15, 0.06};
};

struct NTSC_FCC
{
    static constexpr auto alpha      = 1.0;
    static constexpr auto beta       = 0.0;
    static constexpr auto gamma      = 11.0 / 5.0;
    static constexpr auto delta      = 1.0;
    static constexpr auto betadelta  = 0.0;
    static constexpr auto WhitePoint = wp_C;
    static constexpr auto RedPoint   = std::pair{0.67, 0.33};
    static constexpr auto GreenPoint = std::pair{0.21, 0.71};
    static constexpr auto BluePoint  = std::pair{0.14, 0.08};
};

using BT_601_M = NTSC_FCC;

struct eciRGB
{
    static constexpr auto alpha      = 1.16;
    static constexpr auto beta       = 0.008856;
    static constexpr auto gamma      = 3.0;
    static constexpr auto delta      = 9.033;
    static constexpr auto betadelta  = 0.08;
    static constexpr auto WhitePoint = wp_D50_ISO22028;
    static constexpr auto RedPoint   = std::pair{0.67, 0.33};
    static constexpr auto GreenPoint = std::pair{0.21, 0.71};
    static constexpr auto BluePoint  = std::pair{0.14, 0.08};
};

using ISO_22028_4 = eciRGB;

struct DCIP3
{
    static constexpr auto alpha      = 1.0;
    static constexpr auto beta       = 0.0;
    static constexpr auto gamma      = 13.0 / 5.0;
    static constexpr auto delta      = 1.0;
    static constexpr auto betadelta  = 0.0;
    static constexpr auto WhitePoint = wp_6300K;
    static constexpr auto RedPoint   = std::pair{0.68, 0.32};
    static constexpr auto GreenPoint = std::pair{0.265, 0.69};
    static constexpr auto BluePoint  = std::pair{0.15, 0.06};
};

using SMPTE_RP_431_2 = DCIP3;

struct DisplayP3
{
    static constexpr auto alpha      = 1.055;
    static constexpr auto beta       = 0.0031308;
    static constexpr auto gamma      = 12.0 / 5.0;
    static constexpr auto delta      = 12.92;
    static constexpr auto betadelta  = 0.04045;
    static constexpr auto WhitePoint = wp_D65;
    static constexpr auto RedPoint   = std::pair{0.68, 0.32};
    static constexpr auto GreenPoint = std::pair{0.265, 0.69};
    static constexpr auto BluePoint  = std::pair{0.15, 0.06};
};

using SMPTE_EG_432_2 = DisplayP3;

struct UHDTV
{
    static constexpr auto alpha      = 1.0993;
    static constexpr auto beta       = 0.018054;
    static constexpr auto gamma      = 12.0 / 5.0;
    static constexpr auto delta      = 4.5;
    static constexpr auto betadelta  = 0.081243;
    static constexpr auto WhitePoint = wp_D65;
    static constexpr auto RedPoint   = std::pair{0.708, 0.292};
    static constexpr auto GreenPoint = std::pair{0.170, 0.797};
    static constexpr auto BluePoint  = std::pair{0.131, 0.046};
};

using BT_2020 = UHDTV;
using BT_2100 = UHDTV;

struct AdobeWideGamutRGB
{
    static constexpr auto alpha      = 1.0;
    static constexpr auto beta       = 0.0;
    static constexpr auto gamma      = 563.0 / 256.0;
    static constexpr auto delta      = 1.0;
    static constexpr auto betadelta  = 0.0;
    static constexpr auto WhitePoint = wp_D50;
    static constexpr auto RedPoint   = std::pair{0.735, 0.265};
    static constexpr auto GreenPoint = std::pair{0.115, 0.826};
    static constexpr auto BluePoint  = std::pair{0.157, 0.018};
};

struct ProPhotoRGB
{
    static constexpr auto alpha      = 1.0;
    static constexpr auto beta       = 0.001953125;
    static constexpr auto gamma      = 9.0 / 5.0;
    static constexpr auto delta      = 16.0;
    static constexpr auto betadelta  = 0.031248;
    static constexpr auto WhitePoint = wp_D50;
    static constexpr auto RedPoint   = std::pair{0.7347, 0.2653};
    static constexpr auto GreenPoint = std::pair{0.1596, 0.8404};
    static constexpr auto BluePoint  = std::pair{0.0366, 0.0001};
};

using ISO_22028_2 = ProPhotoRGB;

} // namespace wx::color::colorspace

#endif // _COLOR_SPACE_H