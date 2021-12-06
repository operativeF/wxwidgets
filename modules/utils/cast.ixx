/////////////////////////////////////////////////////////////////////////////
// Name:        modules/util/cast.ixx
// Purpose:     An explicit narrowing cast 
// Author:      Thomas Figueroa
// Modified by:
// Created:     05.12.2021
// Copyright:   (c) Thomas Figueroa
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

export module WX.Utils.Cast;

import <type_traits>;

export namespace wx
{

template<typename T, typename U>
constexpr T narrow_cast(U&& u)
{
    return static_cast<T>(std::forward<U>(u));
}

} // export namespace wx