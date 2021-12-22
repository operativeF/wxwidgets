///////////////////////////////////////////////////////////////////////////////
// Name:        wx/convauto.h
// Purpose:     wxConvAuto class declaration
// Author:      Vadim Zeitlin
// Created:     2006-04-03
// Copyright:   (c) 2006 Vadim Zeitlin
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/strconv.h"

export module WX.Cmn.ConvAuto;

import WX.Base.FontEnc;

export
{

// ----------------------------------------------------------------------------
// wxConvAuto: uses BOM to automatically detect input encoding
// ----------------------------------------------------------------------------

// All currently recognized BOM values.
enum class wxBOM
{
    Unknown = -1,
    None,
    UTF32BE,
    UTF32LE,
    UTF16BE,
    UTF16LE,
    UTF8
};

class wxConvAuto : public wxMBConv
{
public:
    // default ctor, the real conversion will be created on demand
    wxConvAuto(wxFontEncoding enc = wxFONTENCODING_DEFAULT)
    {
        
        // We don't initialize m_encDefault here as different ctors do it
        // differently.
        m_conv = nullptr;
        m_bomType = wxBOM::Unknown;
        m_ownsConv = false;
        m_consumedBOM = false;
    

        m_encDefault = enc;
    }

    // copy ctor doesn't initialize anything neither as conversion can only be
    // deduced on first use
    wxConvAuto(const wxConvAuto& other) : wxMBConv()
    {
        
        // We don't initialize m_encDefault here as different ctors do it
        // differently.
        m_conv = nullptr;
        m_bomType = wxBOM::Unknown;
        m_ownsConv = false;
        m_consumedBOM = false;
    

        m_encDefault = other.m_encDefault;
    }

    ~wxConvAuto()
    {
        if ( m_ownsConv )
            delete m_conv;
    }

    wxConvAuto& operator=(const wxConvAuto&) = delete;

    // get/set the fall-back encoding used when the input text doesn't have BOM
    // and isn't UTF-8
    //
    // special values are wxFONTENCODING_MAX meaning not to use any fall back
    // at all (but just fail to convert in this case) and wxFONTENCODING_SYSTEM
    // meaning to use the encoding of the system locale
    static wxFontEncoding GetFallbackEncoding() { return ms_defaultMBEncoding; }
    static void SetFallbackEncoding(wxFontEncoding enc);
    static void DisableFallbackEncoding()
    {
        SetFallbackEncoding(wxFONTENCODING_MAX);
    }


    // override the base class virtual function(s) to use our m_conv
    size_t ToWChar(wchar_t *dst, size_t dstLen,
                           const char *src, size_t srcLen = wxNO_LEN) const override;

    size_t FromWChar(char *dst, size_t dstLen,
                             const wchar_t *src, size_t srcLen = wxNO_LEN) const override;

    size_t GetMBNulLen() const override { return m_conv->GetMBNulLen(); }

    bool IsUTF8() const override { return m_conv && m_conv->IsUTF8(); }

    wxMBConv *Clone() const override { return new wxConvAuto(*this); }

    // return the BOM type of this buffer
    static wxBOM DetectBOM(const char *src, size_t srcLen);

    // return the characters composing the given BOM.
    static const char* GetBOMChars(wxBOM bomType, size_t* count);

    wxBOM GetBOM() const
    {
        return m_bomType;
    }

    wxFontEncoding GetEncoding() const;

    // Return true if the fall-back encoding is used
    bool IsUsingFallbackEncoding() const
    {
        return m_ownsConv && m_bomType == wxBOM::None;
    }

private:    
    // initialize m_conv with the UTF-8 conversion
    void InitWithUTF8()
    {
        m_conv = &wxConvUTF8;
        m_ownsConv = false;
    }

    // create the correct conversion object for the given BOM type
    void InitFromBOM(wxBOM bomType);

    // create the correct conversion object for the BOM present in the
    // beginning of the buffer
    //
    // return false if the buffer is too short to allow us to determine if we
    // have BOM or not
    bool InitFromInput(const char *src, size_t len);

    // adjust src and len to skip over the BOM (identified by m_bomType) at the
    // start of the buffer
    void SkipBOM(const char **src, size_t *len) const;


    // fall-back multibyte encoding to use, may be wxFONTENCODING_SYSTEM or
    // wxFONTENCODING_MAX but not wxFONTENCODING_DEFAULT

    // we use latin1 by default as it seems the least bad choice: the files we need
    // to detect input of don't always come from the user system (they are often
    // received from other machines) and so using wxFONTENCODING_SYSTEM doesn't
    // seem to be a good idea and there is no other reasonable alternative
    inline static wxFontEncoding ms_defaultMBEncoding{wxFONTENCODING_ISO8859_1};

    // conversion object which we really use, NULL until the first call to
    // either ToWChar() or FromWChar()
    wxMBConv *m_conv;

    // the multibyte encoding to use by default if input isn't Unicode
    wxFontEncoding m_encDefault;

    // our BOM type
    wxBOM m_bomType;

    // true if we allocated m_conv ourselves, false if we just use an existing
    // global conversion
    bool m_ownsConv;

    // true if we already skipped BOM when converting (and not just calculating
    // the size)
    bool m_consumedBOM;
};

} // export
