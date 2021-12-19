///////////////////////////////////////////////////////////////////////////////
// Name:        wx/base64.h
// Purpose:     declaration of BASE64 encoding/decoding functionality
// Author:      Charles Reimers, Vadim Zeitlin
// Created:     2007-06-18
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/string.h"
#include "wx/buffer.h"

export module WX.Cmn.Base64;

#if wxUSE_BASE64

export
{

// ----------------------------------------------------------------------------
// encoding functions
// ----------------------------------------------------------------------------

// return the size needed for the buffer containing the encoded representation
// of a buffer of given length
constexpr size_t wxBase64EncodedSize(size_t len) { return 4*((len+2)/3); }

// raw base64 encoding function which encodes the contents of a buffer of the
// specified length into the buffer of the specified size
//
// returns the length of the encoded data or wxCONV_FAILED if the buffer is not
// large enough; to determine the needed size you can either allocate a buffer
// of wxBase64EncodedSize(srcLen) size or call the function with NULL buffer in
// which case the required size will be returned
size_t
wxBase64Encode(char *dst, size_t dstLen, const void *src, size_t srcLen);

// encode the contents of the given buffer using base64 and return as string
// (there is no error return)
inline wxString wxBase64Encode(const void *src, size_t srcLen)
{
    const size_t dstLen = wxBase64EncodedSize(srcLen);
    wxCharBuffer dst(dstLen);
    wxBase64Encode(dst.data(), dstLen, src, srcLen);

    return wxASCII_STR(dst);
}

inline wxString wxBase64Encode(const wxMemoryBuffer& buf)
{
    return wxBase64Encode(buf.GetData(), buf.GetDataLen());
}

// ----------------------------------------------------------------------------
// decoding functions
// ----------------------------------------------------------------------------

// elements of this enum specify the possible behaviours of wxBase64Decode()
// when an invalid character is encountered
enum class wxBase64DecodeMode
{
    // normal behaviour: stop at any invalid characters
    Strict,

    // skip whitespace characters
    SkipWS,

    // the most lenient behaviour: simply ignore all invalid characters
    Relaxed
};

// return the buffer size necessary for decoding a base64 string of the given
// length
constexpr std::size_t wxBase64DecodedSize(std::size_t srcLen) { return 3 * srcLen / 4; }

// raw decoding function which decodes the contents of the string of specified
// length (or NUL-terminated by default) into the provided buffer of the given
// size
//
// the function normally stops at any character invalid inside a base64-encoded
// string (i.e. not alphanumeric nor '+' nor '/') but can be made to skip the
// whitespace or all invalid characters using its mode argument
//
// returns the length of the decoded data or wxCONV_FAILED if an error occurs
// such as the buffer is too small or the encoded string is invalid; in the
// latter case the posErr is filled with the position where the decoding
// stopped if it is not NULL
size_t
wxBase64Decode(void *dst, size_t dstLen,
               const char *src, size_t srcLen = wxNO_LEN,
               wxBase64DecodeMode mode = wxBase64DecodeMode::Strict,
               size_t *posErr = nullptr);

inline size_t
wxBase64Decode(void *dst, size_t dstLen,
               const wxString& src,
               wxBase64DecodeMode mode = wxBase64DecodeMode::Strict,
               size_t *posErr = nullptr)
{
    // don't use str.length() here as the ASCII buffer is shorter than it for
    // strings with embedded NULs
    return wxBase64Decode(dst, dstLen, src.ToAscii(), wxNO_LEN, mode, posErr);
}

// decode the contents of the given string; the returned buffer is empty if an
// error occurs during decoding
wxMemoryBuffer
wxBase64Decode(const char *src, size_t srcLen = wxNO_LEN,
               wxBase64DecodeMode mode = wxBase64DecodeMode::Strict,
               size_t *posErr = nullptr);

inline wxMemoryBuffer
wxBase64Decode(const wxString& src,
               wxBase64DecodeMode mode = wxBase64DecodeMode::Strict,
               size_t *posErr = nullptr)
{
    // don't use str.length() here as the ASCII buffer is shorter than it for
    // strings with embedded NULs
    return wxBase64Decode(src.ToAscii(), wxNO_LEN, mode, posErr);
}

} // export

#endif // wxUSE_BASE64
