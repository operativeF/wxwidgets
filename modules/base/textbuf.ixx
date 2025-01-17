///////////////////////////////////////////////////////////////////////////////
// Name:        wx/textbuf.h
// Purpose:     class wxTextBuffer to work with text buffers of _small_ size
//              (buffer is fully loaded in memory) and which understands CR/LF
//              differences between platforms.
// Created:     14.11.01
// Author:      Morten Hanssen, Vadim Zeitlin
// Copyright:   (c) 1998-2001 Morten Hanssen, Vadim Zeitlin
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

module;

#include "wx/string.h"

export module WX.Cmn.TextBuffer;

import WX.Cmn.ConvAuto;

import <string>;
import <vector>;

export
{

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// the line termination type (kept wxTextFileType name for compatibility)
enum class wxTextFileType
{
    None,     // incomplete (the last line of the file only)
    Unix,     // line is terminated with 'LF' = 0xA = 10 = '\n'
    Dos,      //                         'CR' 'LF'
    Mac,      //                         'CR' = 0xD = 13 = '\r'
    Os2 = Dos //                         'CR' 'LF'
};

#if wxUSE_TEXTBUFFER

// ----------------------------------------------------------------------------
// wxTextBuffer
// ----------------------------------------------------------------------------

using wxArrayLinesType = std::vector<wxTextFileType>;

#endif // wxUSE_TEXTBUFFER

class wxTextBuffer
{
protected:
    // default ctor, use Open(string)
    wxTextBuffer() = default;

    // ctor from filename
    wxTextBuffer(const wxString& strBufferName);

public:
    wxTextBuffer& operator=(wxTextBuffer&&) = delete;

    virtual ~wxTextBuffer() = default;

    // constants and static functions
    // default type for current platform (determined at compile time)
    inline static const wxTextFileType typeDefault = 
#if defined(WX_WINDOWS)
  wxTextFileType::Dos;
#elif defined(__UNIX__)
  wxTextFileType::Unix;
#else
  wxTextFileType::None;
  #error  "wxTextBuffer: unsupported platform."
#endif

    // this function returns a string which is identical to "text" passed in
    // except that the line terminator characters are changed to correspond the
    // given type. Called with the default argument, the function translates
    // the string to the native format (Unix for Unix, DOS for Windows, ...).
    static std::string Translate(std::string_view text,
                                 wxTextFileType type = typeDefault);

    // get the buffer termination string
    static std::string GetEOL(wxTextFileType type = typeDefault);

    // the static methods of this class are compiled in even when
    // !wxUSE_TEXTBUFFER because they are used by the library itself, but the
    // rest can be left out
#if wxUSE_TEXTBUFFER

    // buffer operations
    // -----------------

    // buffer exists?
    bool Exists() const;

    // create the buffer if it doesn't already exist
    bool Create();

    // same as Create() but with (another) buffer name
    bool Create(const wxString& strBufferName);

    // Open() also loads buffer in memory on success
    bool Open(const wxMBConv& conv = wxConvAuto());

    // same as Open() but with (another) buffer name
    bool Open(const wxString& strBufferName, const wxMBConv& conv = wxConvAuto());

    // closes the buffer and frees memory, losing all changes
    bool Close();

    // is buffer currently opened?
    bool IsOpened() const { return m_isOpened; }

    // get the number of lines in the buffer
    size_t GetLineCount() const { return m_aLines.size(); }

    // the returned line may be modified (but don't add CR/LF at the end!)
          wxString& GetLine(size_t n)          { return m_aLines[n]; }
    const wxString& GetLine(size_t n)    const { return m_aLines[n]; }
          wxString& operator[](size_t n)       { return m_aLines[n]; }
    const wxString& operator[](size_t n) const { return m_aLines[n]; }

    // the current line has meaning only when you're using
    // GetFirstLine()/GetNextLine() functions, it doesn't get updated when
    // you're using "direct access" i.e. GetLine()
    size_t GetCurrentLine() const { return m_nCurLine; }
    void GoToLine(size_t n) { m_nCurLine = n; }
    bool Eof() const { return m_nCurLine == m_aLines.size(); }

    // these methods allow more "iterator-like" traversal of the list of
    // lines, i.e. you may write something like:
    //  for ( str = GetFirstLine(); !Eof(); str = GetNextLine() ) { ... }

    wxString& GetFirstLine()
        { return m_aLines.empty() ? ms_eof : m_aLines[m_nCurLine = 0]; }
    wxString& GetNextLine()
        { return ++m_nCurLine == m_aLines.size() ? ms_eof
                                                 : m_aLines[m_nCurLine]; }
    wxString& GetPrevLine()
        { wxASSERT(m_nCurLine > 0); return m_aLines[--m_nCurLine]; }
    wxString& GetLastLine()
        { return m_aLines.empty() ? ms_eof : m_aLines[m_nCurLine = m_aLines.size() - 1]; }

    // get the type of the line (see also GetEOL)
    wxTextFileType GetLineType(size_t n) const { return m_aTypes[n]; }

    // get the name of the buffer
    const std::string& GetName() const { return m_strBufferName; }

    // add/remove lines
    // ----------------

    // add a line to the end
    void AddLine(const wxString& str, wxTextFileType type = typeDefault)
        { m_aLines.push_back(str); m_aTypes.push_back(type); }
    // insert a line before the line number n
    void InsertLine(const wxString& str,
                  size_t n,
                  wxTextFileType type = typeDefault)
    {
        m_aLines.insert(m_aLines.begin() + n, str);
        m_aTypes.insert(m_aTypes.begin() + n, type);
    }

    // delete one line
    void RemoveLine(size_t n)
    {
        m_aLines.erase(m_aLines.begin() + n);
        m_aTypes.erase(m_aTypes.begin() + n);
    }

    // remove all lines
    void Clear() { m_aLines.clear(); m_aTypes.clear(); m_nCurLine = 0; }

    // change the buffer (default argument means "don't change type")
    // possibly in another format
    bool Write(wxTextFileType typeNew = wxTextFileType::None,
               const wxMBConv& conv = wxConvAuto());

protected:
    enum class wxTextBufferOpenMode { ReadAccess, WriteAccess };

    // Must implement these in derived classes.
    virtual bool OnExists() const = 0;
    virtual bool OnOpen(const std::string& strBufferName,
                        wxTextBufferOpenMode openmode) = 0;
    virtual bool OnClose() = 0;
    virtual bool OnRead(const wxMBConv& conv) = 0;
    virtual bool OnWrite(wxTextFileType typeNew, const wxMBConv& conv) = 0;

    inline static wxString ms_eof;     // dummy string returned at EOF
    std::string m_strBufferName;   // name of the buffer

private:
    wxArrayLinesType m_aTypes;   // type of each line
    std::vector<wxString>    m_aLines;   // lines of file

    size_t        m_nCurLine{0}; // number of current line in the buffer

    bool          m_isOpened{false}; // was the buffer successfully opened the last time?
#endif // wxUSE_TEXTBUFFER
};

} // export
