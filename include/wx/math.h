/**
* Name:        wx/math.h
* Purpose:     Declarations/definitions of common math functions
* Author:      John Labenski and others
* Modified by:
* Created:     02/02/03
* Copyright:   (c) John Labenski
* Licence:     wxWindows licence
*/

#ifndef _WX_MATH_H_
#define _WX_MATH_H_

#define _USE_MATH_DEFINES
#include <cmath>
#include <numbers>

#include "wx/defs.h"

/* Scaling factors for various unit conversions: 1 inch = 2.54 cm */
// FIXME: Problematic, as double aren't commutative.
inline constexpr double METRIC_CONVERSION_CONSTANT = 1 / 25.4;

inline constexpr double mm2inches = METRIC_CONVERSION_CONSTANT;

inline constexpr double inches2mm = 1 / mm2inches;

inline constexpr double mm2twips = METRIC_CONVERSION_CONSTANT * 1440;

inline constexpr double twips2mm = 1 / mm2twips;

inline constexpr double mm2pt = METRIC_CONVERSION_CONSTANT * 72;

inline constexpr double pt2mm = 1 / mm2pt;

inline int wxRound(double x)
{
    wxASSERT_MSG(x > double(INT_MIN) - 0.5 && x < double(INT_MAX) + 0.5,
        "argument out of supported range");

    return int(std::lround(x));
}

inline int wxRound(float x)
{
    wxASSERT_MSG(x > float(INT_MIN) && x < float(INT_MAX),
        "argument out of supported range");

    return int(std::lround(x));
}

inline int wxRound(long double x) { return wxRound(double(x)); }

// Convert between degrees and radians.
static constexpr double wxDegToRad(double deg) { return (deg * std::numbers::pi) / 180.0; }
static constexpr double wxRadToDeg(double rad) { return (rad * 180.0) / std::numbers::pi; }

// Count trailing zeros.
WXDLLIMPEXP_BASE unsigned int wxCTZ(std::uint32_t x);

#if defined(__WINDOWS__)
    #define wxMulDivInt32( a , b , c ) ::MulDiv( a , b , c )
#else
    #define wxMulDivInt32( a , b , c ) (wxRound((a)*(((double)b)/((double)c))))
#endif

/* Compute the greatest common divisor of two positive integers */
WXDLLIMPEXP_BASE unsigned int wxGCD(unsigned int u, unsigned int v);

#endif /* _WX_MATH_H_ */
