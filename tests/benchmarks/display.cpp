/////////////////////////////////////////////////////////////////////////////
// Name:        tests/benchmarks/display.cpp
// Purpose:     wxDisplay benchmarks
// Author:      Vadim Zeitlin
// Created:     2018-09-30
// Copyright:   (c) 2018 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "wx/display.h"
#include "wx/gdicmn.h"

#include "bench.h"

BENCHMARK_FUNC(GetDisplaySize)
{
    return wxDisplay().GetGeometry().GetSize().x > 0;
}

BENCHMARK_FUNC(DisplayGetGeometry)
{
    return wxDisplay().GetGeometry().GetSize().x > 0;
}
