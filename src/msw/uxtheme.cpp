///////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/uxtheme.cpp
// Purpose:     implements wxUxThemeEngine class: support for XP themes
// Author:      John Platts, Vadim Zeitlin
// Modified by:
// Created:     2003
// Copyright:   (c) 2003 John Platts, Vadim Zeitlin
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#if wxUSE_UXTHEME

#include "wx/toplevel.h"
#include "wx/log.h"
#include "wx/module.h"

#include "wx/msw/uxtheme.h"

#include <string>

bool wxUxThemeIsActive()
{
    return ::IsAppThemed() && ::IsThemeActive();
}
#else
bool wxUxThemeIsActive()
{
    return false;
}
#endif // wxUSE_UXTHEME
