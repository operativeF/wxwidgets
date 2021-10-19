/////////////////////////////////////////////////////////////////////////////
// Name:        wx/richtext/richtexthtml.h
// Purpose:     HTML I/O for wxRichTextCtrl
// Author:      Julian Smart
// Modified by:
// Created:     2005-09-30
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_RICHTEXTHTML_H_
#define _WX_RICHTEXTHTML_H_

/*!
 * Includes
 */

#include "wx/richtext/richtextbuffer.h"

// Use CSS styles where applicable, otherwise use non-CSS workarounds
#define wxRICHTEXT_HANDLER_USE_CSS 0x1000

/*!
 * wxRichTextHTMLHandler
 */

class WXDLLIMPEXP_RICHTEXT wxRichTextHTMLHandler: public wxRichTextFileHandler
{
    wxDECLARE_DYNAMIC_CLASS(wxRichTextHTMLHandler);
public:
    wxRichTextHTMLHandler(const std::string& name = "HTML", const std::string& ext = "html", int type = wxRICHTEXT_TYPE_HTML);

    /// Can we save using this handler?
    bool CanSave() const override { return true; }

    /// Can we load using this handler?
    bool CanLoad() const override { return false; }

    /// Can we handle this filename (if using files)? By default, checks the extension.
    bool CanHandle(const fs::path& filename) const override;

// Accessors and operations unique to this handler

    /// Set and get the list of image locations generated by the last operation
    void SetTemporaryImageLocations(const std::vector<std::string>& locations) { m_imageLocations = locations; }
    const std::vector<std::string>& GetTemporaryImageLocations() const { return m_imageLocations; }

    /// Clear the image locations generated by the last operation
    void ClearTemporaryImageLocations() { m_imageLocations.clear(); }

    /// Delete the in-memory or temporary files generated by the last operation
    bool DeleteTemporaryImages();

    /// Delete the in-memory or temporary files generated by the last operation. This is a static
    /// function that can be used to delete the saved locations from an earlier operation,
    /// for example after the user has viewed the HTML file.
    static bool DeleteTemporaryImages(int flags, const std::vector<std::string>& imageLocations);

    /// Reset the file counter, in case, for example, the same names are required each time
    static void SetFileCounter(int counter) { sm_fileCounter = counter; }

    /// Set and get the directory for storing temporary files. If empty, the system
    /// temporary directory will be used.
    void SetTempDir(const std::string& tempDir) { m_tempDir = tempDir; }
    const std::string& GetTempDir() const { return m_tempDir; }

    /// Set and get mapping from point size to HTML font size. There should be 7 elements,
    /// one for each HTML font size, each element specifying the maximum point size for that
    /// HTML font size. E.g. 8, 10, 13, 17, 22, 29, 100
    void SetFontSizeMapping(const std::vector<int>& fontSizeMapping) { m_fontSizeMapping = fontSizeMapping; }
    std::vector<int> GetFontSizeMapping() const { return m_fontSizeMapping; }

protected:

// Implementation

#if wxUSE_STREAMS
    bool DoLoadFile(wxRichTextBuffer *buffer, wxInputStream& stream) override;
    bool DoSaveFile(wxRichTextBuffer *buffer, wxOutputStream& stream) override;

    /// Output character formatting
    void BeginCharacterFormatting(const wxRichTextAttr& currentStyle, const wxRichTextAttr& thisStyle, const wxRichTextAttr& paraStyle, wxTextOutputStream& stream );
    void EndCharacterFormatting(const wxRichTextAttr& currentStyle, const wxRichTextAttr& thisStyle, const wxRichTextAttr& paraStyle, wxTextOutputStream& stream );

    /// Output paragraph formatting
    void BeginParagraphFormatting(const wxRichTextAttr& currentStyle, const wxRichTextAttr& thisStyle, wxTextOutputStream& stream);
    void EndParagraphFormatting(const wxRichTextAttr& currentStyle, const wxRichTextAttr& thisStyle, wxTextOutputStream& stream);

    /// Output font tag
    void OutputFont(const wxRichTextAttr& style, wxTextOutputStream& stream);

    /// Closes lists to level (-1 means close all)
    void CloseLists(int level, wxTextOutputStream& str);

    /// Writes an image to its base64 equivalent, or to the memory filesystem, or to a file
    void WriteImage(wxRichTextImage* image, wxOutputStream& stream);

    /// Converts from pt to size property compatible height
    long PtToSize(long size);

    /// Typical base64 encoder
    wxChar* b64enc(unsigned char* input, size_t in_len);

    /// Gets the mime type of the given wxBitmapType
    const wxChar* GetMimeType(wxBitmapType imageType);

    /// Gets the html equivalent of the specified value
    std::string GetAlignment(const wxRichTextAttr& thisStyle);

    /// Generates &nbsp; array for indentations
    std::string SymbolicIndent(long indent);

    /// Finds the html equivalent of the specified bullet
    int TypeOfList(const wxRichTextAttr& thisStyle, std::string& tag);
#endif

// Data members

    wxRichTextBuffer* m_buffer{nullptr};

    /// Indentation values of the table tags
    std::vector<int>      m_indents;

    /// Stack of list types: 0 = ol, 1 = ul
    std::vector<int>      m_listTypes;

    /// Is there any opened font tag?
    bool            m_font{false};

    /// Are we in a table?
    bool            m_inTable{false};

    /// A list of the image files or in-memory images created by the last operation.
    std::vector<std::string>   m_imageLocations;

    /// A location for the temporary files
    std::string        m_tempDir;

    /// A mapping from point size to HTML font size
    std::vector<int>      m_fontSizeMapping;

    /// A counter for generating filenames
    static int      sm_fileCounter;
};

#endif
    // _WX_RICHTEXTXML_H_
