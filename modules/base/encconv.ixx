/////////////////////////////////////////////////////////////////////////////
// Name:        wx/encconv.h
// Purpose:     wxEncodingConverter class for converting between different
//              font encodings
// Author:      Vaclav Slavik
// Copyright:   (c) 1999 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

module;

#include "wx/string.h"

export module WX.Cmn.EncConv;

import WX.Base.FontEnc;

import <vector>;

export
{

enum
{
    wxCONVERT_STRICT,
    wxCONVERT_SUBSTITUTE
};

enum
{
    wxPLATFORM_CURRENT = -1,

    wxPLATFORM_UNIX = 0,
    wxPLATFORM_WINDOWS,
    wxPLATFORM_MAC
};

// ----------------------------------------------------------------------------
// types
// ----------------------------------------------------------------------------

using wxFontEncodingArray = std::vector<wxFontEncoding>;

//--------------------------------------------------------------------------------
// wxEncodingConverter
//                  This class is capable of converting strings between any two
//                  8bit encodings/charsets. It can also convert from/to Unicode
//--------------------------------------------------------------------------------

class wxEncodingConverter
{
    public:
            ~wxEncodingConverter() { delete[] m_Table; }

            wxEncodingConverter& operator=(wxEncodingConverter&&) = delete;

            // Initialize conversion. Both output or input encoding may
            // be wxFONTENCODING_UNICODE.
            //
            // All subsequent calls to Convert() will interpret it's argument
            // as a string in input_enc encoding and will output string in
            // output_enc encoding.
            //
            // You must call this method before calling Convert. You may call
            // it more than once in order to switch to another conversion
            //
            // Method affects behaviour of Convert() in case input character
            // cannot be converted because it does not exist in output encoding:
            //     wxCONVERT_STRICT --
            //              follow behaviour of GNU Recode - just copy unconvertable
            //              characters to output and don't change them (it's integer
            //              value will stay the same)
            //     wxCONVERT_SUBSTITUTE --
            //              try some (lossy) substitutions - e.g. replace
            //              unconvertable latin capitals with acute by ordinary
            //              capitals, replace en-dash or em-dash by '-' etc.
            //     both modes guarantee that output string will have same length
            //     as input string
            //
            // Returns false if given conversion is impossible, true otherwise
            // (conversion may be impossible either if you try to convert
            // to Unicode with non-Unicode build of wxWidgets or if input
            // or output encoding is not supported.)
            bool Init(wxFontEncoding input_enc, wxFontEncoding output_enc, int method = wxCONVERT_STRICT);

            // Convert input string according to settings passed to Init.
            // Note that you must call Init before using Convert!
            bool Convert(const char* input, char* output) const;
            bool Convert(char* str) const { return Convert(str, str); }
            wxString Convert(const wxString& input) const;

            bool Convert(const char* input, wchar_t* output) const;
            bool Convert(const wchar_t* input, char* output) const;
            bool Convert(const wchar_t* input, wchar_t* output) const;
            bool Convert(wchar_t* str) const { return Convert(str, str); }

            // Return equivalent(s) for given font that are used
            // under given platform. wxPLATFORM_CURRENT means the platform
            // this binary was compiled for
            //
            // Examples:
            //     current platform          enc    returned value
            // -----------------------------------------------------
            //     unix                   CP1250         {ISO8859_2}
            //     unix                ISO8859_2                  {}
            //     windows             ISO8859_2            {CP1250}
            //
            // Equivalence is defined in terms of convertibility:
            // 2 encodings are equivalent if you can convert text between
            // then without losing information (it may - and will - happen
            // that you lose special chars like quotation marks or em-dashes
            // but you shouldn't lose any diacritics and language-specific
            // characters when converting between equivalent encodings).
            //
            // Convert() method is not limited to converting between
            // equivalent encodings, it can convert between arbitrary
            // two encodings!
            //
            // Remember that this function does _NOT_ check for presence of
            // fonts in system. It only tells you what are most suitable
            // encodings. (It usually returns only one encoding)
            //
            // Note that argument enc itself may be present in returned array!
            // (so that you can -- as a side effect -- detect whether the
            // encoding is native for this platform or not)
            static wxFontEncodingArray GetPlatformEquivalents(wxFontEncoding enc, int platform = wxPLATFORM_CURRENT);

            // Similar to GetPlatformEquivalent, but this one will return ALL
            // equivalent encodings, regardless the platform, including itself.
            static wxFontEncodingArray GetAllEquivalents(wxFontEncoding enc);

            // Return true if [any text in] one multibyte encoding can be
            // converted to another one losslessly.
            //
            // Do not call this with wxFONTENCODING_UNICODE, it doesn't make
            // sense (always works in one sense and always depends on the text
            // to convert in the other)
            static bool CanConvert(wxFontEncoding encIn, wxFontEncoding encOut)
            {
                auto equivalents = GetAllEquivalents(encIn);
                return std::ranges::find(equivalents, encOut) != equivalents.end();
            }

    private:
            wchar_t *m_Table{nullptr};
            bool m_UnicodeInput{false};
            bool m_UnicodeOutput{false};
            bool m_JustCopy{false};
};

} // export
