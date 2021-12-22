///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/textbuf.cpp
// Purpose:     implementation of wxTextBuffer class
// Created:     14.11.01
// Author:      Morten Hanssen, Vadim Zeitlin
// Copyright:   (c) 1998-2001 wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/string.h"
#include "wx/intl.h"
#include "wx/log.h"

module WX.Cmn.TextBuffer;

import <string>;

// ============================================================================
// wxTextBuffer class implementation
// ============================================================================

// ----------------------------------------------------------------------------
// static methods (always compiled in)
// ----------------------------------------------------------------------------

std::string wxTextBuffer::GetEOL(wxTextFileType type)
{
    switch ( type ) {
        default:
            wxFAIL_MSG("bad buffer type in wxTextBuffer::GetEOL.");
            [[fallthrough]]; // fall through nevertheless - we must return something...

        case wxTextFileType::None: return {};
        case wxTextFileType::Unix: return "\n";
        case wxTextFileType::Dos:  return "\r\n";
        case wxTextFileType::Mac:  return "\r";
    }
}

std::string wxTextBuffer::Translate(std::string_view text, wxTextFileType type)
{
    // don't do anything if there is nothing to do
    if ( type == wxTextFileType::None )
        return {text.begin(), text.end()};

    // nor if it is empty
    if ( text.empty() )
        return {};

    std::string eol = GetEOL(type);
    std::string result;

    // optimization: we know that the length of the new string will be about
    // the same as the length of the old one, so prealloc memory to avoid
    // unnecessary relocations
    result.reserve(text.size());

    char chLast{};
    for ( auto ch : text )
    {
        switch ( ch ) {
            case '\n':
                // Dos/Unix line termination
                result += eol;
                chLast = 0;
                break;

            case '\r':
                if ( chLast == '\r' ) {
                    // Mac empty line
                    result += eol;
                }
                else {
                    // just remember it: we don't know whether it is just "\r"
                    // or "\r\n" yet
                    chLast = '\r';
                }
                break;

            default:
                if ( chLast == '\r' ) {
                    // Mac line termination
                    result += eol;

                    // reset chLast to avoid inserting another eol before the
                    // next character
                    chLast = 0;
                }

                // add to the current line
                result += ch;
        }
    }

    if ( chLast ) {
        // trailing '\r'
        result += eol;
    }

    return result;
}

#if wxUSE_TEXTBUFFER

// ----------------------------------------------------------------------------
// ctors & dtor
// ----------------------------------------------------------------------------

wxTextBuffer::wxTextBuffer(const wxString& strBufferName)
            : m_strBufferName(strBufferName)
{
}

// ----------------------------------------------------------------------------
// buffer operations
// ----------------------------------------------------------------------------

bool wxTextBuffer::Exists() const
{
    return OnExists();
}

bool wxTextBuffer::Create(const wxString& strBufferName)
{
    m_strBufferName = strBufferName;

    return Create();
}

bool wxTextBuffer::Create()
{
    // buffer name must be either given in ctor or in Create(const wxString&)
    wxASSERT( !m_strBufferName.empty() );

    // if the buffer already exists do nothing
    if ( Exists() ) return false;

    if ( !OnOpen(m_strBufferName, wxTextBufferOpenMode::WriteAccess) )
        return false;

    OnClose();
    return true;
}

bool wxTextBuffer::Open(const wxString& strBufferName, const wxMBConv& conv)
{
    m_strBufferName = strBufferName;

    return Open(conv);
}

bool wxTextBuffer::Open(const wxMBConv& conv)
{
    // buffer name must be either given in ctor or in Open(const wxString&)
    wxASSERT( !m_strBufferName.empty() );

    // open buffer in read-only mode
    if ( !OnOpen(m_strBufferName, wxTextBufferOpenMode::ReadAccess) )
        return false;

    // read buffer into memory
    m_isOpened = OnRead(conv);

    OnClose();

    return m_isOpened;
}

bool wxTextBuffer::Close()
{
    Clear();
    m_isOpened = false;

    return true;
}

bool wxTextBuffer::Write(wxTextFileType typeNew, const wxMBConv& conv)
{
    return OnWrite(typeNew, conv);
}

#endif // wxUSE_TEXTBUFFER
