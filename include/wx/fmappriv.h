///////////////////////////////////////////////////////////////////////////////
// Name:        wx/fmappriv.h
// Purpose:     private wxFontMapper stuff, not to be used by the library users
// Author:      Vadim Zeitlin
// Modified by:
// Created:     21.06.2003 (extracted from common/fontmap.cpp)
// Copyright:   (c) 1999-2003 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_FMAPPRIV_H_
#define _WX_FMAPPRIV_H_

import <string>;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// a special pseudo encoding which means "don't ask me about this charset
// any more" -- we need it to avoid driving the user crazy with asking him
// time after time about the same charset which he [presumably] doesn't
// have the fonts for
enum { wxFONTENCODING_UNKNOWN = -2 };

// the config paths we use
#if wxUSE_CONFIG

inline constexpr char FONTMAPPER_ROOT_PATH[]            = "/wxWindows/FontMapper";
inline constexpr char FONTMAPPER_CHARSET_PATH[]         = "Charsets";
inline constexpr char FONTMAPPER_CHARSET_ALIAS_PATH[]   = "Aliases";

#endif // wxUSE_CONFIG

// ----------------------------------------------------------------------------
// wxFontMapperPathChanger: change the config path during our lifetime
// ----------------------------------------------------------------------------

#if wxUSE_CONFIG && wxUSE_FILECONFIG

class wxFontMapperPathChanger
{
public:
    wxFontMapperPathChanger(wxFontMapperBase *fontMapper, const std::string& path)
    {
        m_fontMapper = fontMapper;
        m_ok = m_fontMapper->ChangePath(path, &m_pathOld);
    }

    bool IsOk() const { return m_ok; }

    ~wxFontMapperPathChanger()
    {
        if ( IsOk() )
            m_fontMapper->RestorePath(m_pathOld);
    }

    wxFontMapperPathChanger& operator=(wxFontMapperPathChanger&&) = delete;

private:
    // the fontmapper object we're working with
    wxFontMapperBase *m_fontMapper;

    // the old path to be restored if m_ok
    wxString m_pathOld;

    // have we changed the path successfully?
    bool m_ok;
};

#endif // wxUSE_CONFIG

#endif // _WX_FMAPPRIV_H_

