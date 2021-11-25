/////////////////////////////////////////////////////////////////////////////
// Name:        wx/meta/if.h
// Purpose:     declares wxIf<> metaprogramming construct
// Author:      Vaclav Slavik
// Created:     2008-01-22
// Copyright:   (c) 2008 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_META_IF_H_
#define _WX_META_IF_H_

namespace wxPrivate
{

template <bool Cond>
struct wxIfImpl;

// specialization for true:
template <>
struct wxIfImpl<true>
{
    template<typename TTrue, typename TFalse> struct Result
    {
        using value = TTrue;
    };
};

// specialization for false:
template<>
struct wxIfImpl<false>
{
    template<typename TTrue, typename TFalse> struct Result
    {
        using value = TFalse;
    };
};

} // namespace wxPrivate

// wxIf<> template defines nested type "value" which is the same as
// TTrue if the condition Cond (boolean compile-time constant) was met and
// TFalse if it wasn't.
//
// See std::vector<T> in vector.h for usage example
template<bool Cond, typename TTrue, typename TFalse>
struct wxIf
{
    using value = typename wxPrivate::wxIfImpl<Cond>::template Result<TTrue, TFalse>::value;
};

#endif // _WX_META_IF_H_
