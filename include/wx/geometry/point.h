#ifndef _WX_POINT_H
#define _WX_POINT_H

#include "wx/list.h"
#include "wx/math.h"

#include <cmath>

template<typename NumericalT>
struct WXDLLIMPEXP_CORE wxPoint2D
{
    using value_type = NumericalT;

    constexpr wxPoint2D() noexcept {}

    constexpr wxPoint2D(value_type a, value_type b) noexcept
        : x{ a },
          y{ b }
    {
    }

    // TODO: Misleading for floating point types.
    constexpr bool operator==(const wxPoint2D<value_type>& b) const noexcept
    {
        return x == b.x && y == b.y;
    }

    // TODO: Misleading for floating point types.
    constexpr bool operator!=(const wxPoint2D<value_type>& b) const noexcept
    {
        return !(*this == b);
    }

    template<typename ScalarT = value_type>
    constexpr wxPoint2D<value_type>& operator*=(ScalarT val) noexcept
    {
        x *= val;
        y *= val;

        return *this;
    }


    template<typename ScalarT = value_type>
    constexpr wxPoint2D<value_type>& operator/=(ScalarT val) noexcept
    {
        x /= val;
        y /= val;

        return *this;
    }

    template<typename NumericalU = value_type>
    constexpr wxPoint2D<value_type>& operator-=(const wxPoint2D<NumericalU>& val) noexcept
    {
        x -= val.x;
        y -= val.y;

        return *this;
    }

    // TODO: Add constraints.
    template<typename PointLike>
    constexpr wxPoint2D<value_type>& operator-=(const PointLike& val) noexcept
    {
        x -= val.x;
        y -= val.y;

        return *this;
    }

    template<typename NumericalU = value_type>
    constexpr wxPoint2D<value_type>& operator+=(const wxPoint2D<NumericalU>& val) noexcept
    {
        x += val.x;
        y += val.y;

        return *this;
    }

    // TODO: Add constraints.
    template<typename PointLike>
    constexpr wxPoint2D<value_type>& operator+=(const PointLike& val) noexcept
    {
        x += val.x;
        y += val.y;

        return *this;
    }

    constexpr wxPoint2D<value_type> operator-() const noexcept
    {
        return { -x, -y };
    }

    // TODO: Add fmt stream operators.

    value_type x{};
    value_type y{};
};

template<typename NumericalT, typename NumericalU = NumericalT>
constexpr auto VectorDotProduct(const wxPoint2D<NumericalT>& vecA, const wxPoint2D<NumericalU>& vecB) noexcept
{
    return vecA.x * vecB.x + vecA.y * vecB.y;
}

template<typename NumericalT, typename NumericalU = NumericalT>
constexpr auto VectorCrossProduct(const wxPoint2D<NumericalT>& vecA, const wxPoint2D<NumericalU>& vecB) noexcept
{
    return vecA.x * vecB.y - vecB.x * vecA.y;
}

template<typename NumericalT>
auto GetVectorLength(const wxPoint2D<NumericalT>& vec) noexcept
{
    return std::hypot(vec.x, vec.y);
}

template<typename NumericalT>
auto GetVectorAngle(const wxPoint2D<NumericalT>& vec) noexcept
{
    // FIXME: Double equality
    if (vec.x == 0.0)
    {
        if (vec.y >= 0)
            return 90.0;
        else
            return 270.0;
    }
    // FIXME: Double equality
    if (vec.y == 0.0)
    {
        if (vec.x >= 0)
            return 0.0;
        else
            return 180.0;
    }

    auto deg = wxRadToDeg(std::atan2(vec.y, vec.x));

    if (deg < 0.0)
    {
        deg += 360.0;
    }

    return deg;
}

template<typename NumericalT>
void SetVectorAngle(wxPoint2D<NumericalT>& vec, auto angle) noexcept
{
    const auto length = GetLength(vec);

    vec.x = length * std::cos(wxDegToRad(angle));
    vec.y = length * std::sin(wxDegToRad(angle));
}

template<typename NumericalT>
void SetVectorLength(wxPoint2D<NumericalT>& vec, auto length) noexcept
{
    const auto priorLength = GetVectorLength(vec);

    vec.x = vec.x * length / priorLength;
    vec.y = vec.y * length / priorLength;
}

template<typename NumericalT>
constexpr void Normalize(wxPoint2D<NumericalT>& vec) noexcept
{
    SetVectorLength(vec, 1.0);
}

template<typename NumericalT, typename NumericalU = NumericalT>
auto GetVectorDistance(const wxPoint2D<NumericalT>& vecA, const wxPoint2D<NumericalU>& vecB) noexcept
{
    return std::hypot(vecA.x - vecB.x, vecA.y - vecB.y);
}

// TODO: Avoid using std::pow for integers?
template<typename NumericalT, typename NumericalU = NumericalT>
auto GetVectorDistanceSquare(const wxPoint2D<NumericalT>& vecA, const wxPoint2D<NumericalU>& vecB) noexcept
{
    return std::pow(GetDistance(vecA, vecB), 2.0);
}

template<typename NumericalT>
constexpr wxPoint2D<NumericalT> operator+(wxPoint2D<NumericalT> vecA, const wxPoint2D<NumericalT>& vecB) noexcept
{
    vecA += vecB;
    return vecA;
}

template<typename NumericalT, typename PointLike>
constexpr wxPoint2D<NumericalT> operator+(wxPoint2D<NumericalT> vec, const PointLike& ptl) noexcept
{

    vec += ptl;
    return vec;
}

template<typename NumericalT, typename PointLike>
constexpr wxPoint2D<NumericalT> operator+(const PointLike& ptl, wxPoint2D<NumericalT> vec) noexcept
{
    vec += ptl;
    return vec;
}

template<typename NumericalT>
constexpr wxPoint2D<NumericalT> operator-(wxPoint2D<NumericalT> vecA, const wxPoint2D<NumericalT>& vecB) noexcept
{
    vecA -= vecB;
    return vecA;
}

template<typename NumericalT, typename PointLike>
constexpr wxPoint2D<NumericalT> operator-(const PointLike& ptl, wxPoint2D<NumericalT> vec) noexcept
{
    vec = -vec;
    vec += ptl;
    return vec;
}

template<typename NumericalT, typename PointLike>
constexpr wxPoint2D<NumericalT> operator-(wxPoint2D<NumericalT> vec, const PointLike& ptl) noexcept
{
    vec -= ptl;
    return vec;
}


template<typename NumericalT, typename ScalarT = NumericalT>
constexpr wxPoint2D<NumericalT> operator*(wxPoint2D<NumericalT> vec, ScalarT val) noexcept
{
    vec *= val;
    return vec;
}

template<typename NumericalT, typename ScalarT = NumericalT>
constexpr wxPoint2D<NumericalT> operator*(ScalarT val, const wxPoint2D<NumericalT>& vec) noexcept
{
    return vec * val;
}

template<typename NumericalT, typename ScalarT = NumericalT>
constexpr wxPoint2D<NumericalT>  operator/(wxPoint2D<NumericalT> vec, ScalarT val) noexcept
{
    vec /= val;
    return vec;
}

using wxPoint2DInt = wxPoint2D<int>;
using wxPoint2DDouble = wxPoint2D<double>;
using wxPoint2DFloat = wxPoint2D<float>;

using wxPoint = wxPoint2D<int>;
using wxRealPoint = wxPoint2DDouble;
using wxScale     = wxPoint2DDouble;

constexpr bool IsFullySpecified(const wxPoint& pt) { return pt.x != wxDefaultCoord && pt.y != wxDefaultCoord; }

WX_DECLARE_LIST_WITH_DECL(wxPoint, wxPointList, class WXDLLIMPEXP_CORE);

#endif // _WX_POINT_H
