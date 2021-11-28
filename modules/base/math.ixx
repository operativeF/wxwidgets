/**
* Name:        wx/math.h
* Purpose:     Declarations/definitions of common math functions
* Author:      John Labenski and others
* Modified by:
* Created:     02/02/03
* Copyright:   (c) John Labenski
* Licence:     wxWindows licence
*/

export module Utils.Math;

import <cmath>;
import <cstdint>;
import <numbers>;

export
{

/* Scaling factors for various unit conversions: 1 inch = 2.54 cm */
// FIXME: Problematic, as double aren't commutative.
inline constexpr double METRIC_CONVERSION_CONSTANT = 1 / 25.4;

inline constexpr double mm2inches = METRIC_CONVERSION_CONSTANT;

inline constexpr double inches2mm = 1 / mm2inches;

inline constexpr double mm2twips = METRIC_CONVERSION_CONSTANT * 1440;

inline constexpr double twips2mm = 1 / mm2twips;

inline constexpr double mm2pt = METRIC_CONVERSION_CONSTANT * 72;

inline constexpr double pt2mm = 1 / mm2pt;

// Convert between degrees and radians.
inline constexpr double wxDegToRad(double deg) { return (deg * std::numbers::pi) / 180.0; }
inline constexpr double wxRadToDeg(double rad) { return (rad * 180.0) / std::numbers::pi; }

inline std::int32_t wxMulDivInt32(std::int32_t a, std::int32_t b, std::int32_t c)
{
    return std::lround(a * static_cast<double>(b) / static_cast<double>(c));
}

} // export
