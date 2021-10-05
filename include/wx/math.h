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

#include "wx/defs.h"

#include <cmath>
#include <cstdlib>
#include <numbers>

/* Scaling factors for various unit conversions: 1 inch = 2.54 cm */
// FIXME: Problematic, as double aren't commutative.
constexpr double METRIC_CONVERSION_CONSTANT = 1 / 25.4;

constexpr double mm2inches = METRIC_CONVERSION_CONSTANT;

constexpr double inches2mm = 1 / mm2inches;

constexpr double mm2twips = METRIC_CONVERSION_CONSTANT * 1440;

constexpr double twips2mm = 1 / mm2twips;

constexpr double mm2pt = METRIC_CONVERSION_CONSTANT * 72;

constexpr double pt2mm = 1 / mm2pt;

// Convert between degrees and radians.
static constexpr double wxDegToRad(double deg) { return (deg * std::numbers::pi) / 180.0; }
static constexpr double wxRadToDeg(double rad) { return (rad * 180.0) / std::numbers::pi; }

#if defined(__WINDOWS__)
    #define wxMulDivInt32( a , b , c ) ::MulDiv( a , b , c )
#else
    #define wxMulDivInt32( a , b , c ) (std::lround((a)*(((double)b)/((double)c))))
#endif

#endif /* _WX_MATH_H_ */
