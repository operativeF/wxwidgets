/////////////////////////////////////////////////////////////////////////////
// Name:        wx/translation.h
// Purpose:     Internationalization and localisation for wxWidgets
// Author:      Vadim Zeitlin, Vaclav Slavik,
//              Michael N. Filippov <michael@idisys.iae.nsk.su>
// Created:     2010-04-23
// Copyright:   (c) 1998 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
//              (c) 2010 Vaclav Slavik <vslavik@fastmail.fm>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TRANSLATION_H_
#define _WX_TRANSLATION_H_

#include "wx/string.h"

#include "wx/buffer.h"
#include "wx/language.h"
#include "wx/strconv.h"
#include "wx/scopedptr.h"

import WX.WinDef;
import WX.Cmn.Translation;

import <unordered_map>;
import <vector>;

#if wxUSE_INTL

// ----------------------------------------------------------------------------
// macros
// ----------------------------------------------------------------------------

// gettext() style macros (notice that xgettext should be invoked with
// --keyword="_" --keyword="wxPLURAL:1,2" options
// to extract the strings from the sources)
#ifndef WXINTL_NO_GETTEXT_MACRO
#ifndef wxNO_IMPLICIT_WXSTRING_ENCODING
    #define _(s)                               wxGetTranslation((s))
#else
    #define _(s)                               wxGetTranslation(s)
#endif
    #define wxPLURAL(sing, plur, n)            wxGetTranslation((sing), (plur), n)
#endif

// wx-specific macro for translating strings in the given context: if you use
// them, you need to also add
// --keyword="wxGETTEXT_IN_CONTEXT:1c,2" --keyword="wxGETTEXT_IN_CONTEXT_PLURAL:1c,2,3"
// options to xgettext invocation.
#ifndef wxNO_IMPLICIT_WXSTRING_ENCODING
#define wxGETTEXT_IN_CONTEXT(c, s) \
    wxGetTranslation((s), std::string(), c)
#else
#define wxGETTEXT_IN_CONTEXT(c, s) \
    wxGetTranslation(s, std::string(), c)
#endif
#define wxGETTEXT_IN_CONTEXT_PLURAL(c, sing, plur, n) \
    wxGetTranslation((sing), (plur), n, std::string(), c)

// another one which just marks the strings for extraction, but doesn't
// perform the translation (use -kwxTRANSLATE with xgettext!)
#define wxTRANSLATE(str) str

#else // !wxUSE_INTL

// the macros should still be defined - otherwise compilation would fail

#if !defined(WXINTL_NO_GETTEXT_MACRO)
    #if !defined(_)
#ifndef wxNO_IMPLICIT_WXSTRING_ENCODING
        #define _(s)                 (s)
#else
        #define _(s)                 (s)
#endif
    #endif
    #define wxPLURAL(sing, plur, n)  ((n) == 1 ? (sing) : (plur))
    #define wxGETTEXT_IN_CONTEXT(c, s)                     (s)
    #define wxGETTEXT_IN_CONTEXT_PLURAL(c, sing, plur, n)  wxPLURAL(sing, plur, n)
#endif

#define wxTRANSLATE(str) str

#endif // wxUSE_INTL/!wxUSE_INTL

// define this one just in case it occurs somewhere (instead of preferred
// wxTRANSLATE) too
#if !defined(WXINTL_NO_GETTEXT_MACRO)
    #if !defined(gettext_noop)
        #define gettext_noop(str) (str)
    #endif
    #if !defined(N_)
        #define N_(s)             (s)
    #endif
#endif

#endif // _WX_TRANSLATION_H_
