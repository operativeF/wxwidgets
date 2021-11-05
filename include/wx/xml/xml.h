/////////////////////////////////////////////////////////////////////////////
// Name:        wx/xml/xml.h
// Purpose:     wxXmlDocument - XML parser & data holder class
// Author:      Vaclav Slavik
// Created:     2000/03/05
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////


#ifndef _WX_XML_H_
#define _WX_XML_H_

#include "wx/defs.h"

#if wxUSE_XML

#include "wx/string.h"
#include "wx/object.h"
#include "wx/textbuf.h"
#include "wx/versioninfo.h"

#ifdef WXMAKINGDLL_XML
    #define WXDLLIMPEXP_XML WXEXPORT
#elif defined(WXUSINGDLL)
    #define WXDLLIMPEXP_XML WXIMPORT
#else // not making nor using DLL
    #define WXDLLIMPEXP_XML
#endif

class wxXmlNode;
class wxXmlAttribute;
class wxXmlDocument;
class wxXmlIOHandler;
class wxInputStream;
class wxOutputStream;

// Represents XML node type.
enum wxXmlNodeType
{
    // note: values are synchronized with xmlElementType from libxml
    wxXML_ELEMENT_NODE       =  1,
    wxXML_ATTRIBUTE_NODE     =  2,
    wxXML_TEXT_NODE          =  3,
    wxXML_CDATA_SECTION_NODE =  4,
    wxXML_ENTITY_REF_NODE    =  5,
    wxXML_ENTITY_NODE        =  6,
    wxXML_PI_NODE            =  7,
    wxXML_COMMENT_NODE       =  8,
    wxXML_DOCUMENT_NODE      =  9,
    wxXML_DOCUMENT_TYPE_NODE = 10,
    wxXML_DOCUMENT_FRAG_NODE = 11,
    wxXML_NOTATION_NODE      = 12,
    wxXML_HTML_DOCUMENT_NODE = 13
};


// Represents node property(ies).
// Example: in <img src="hello.gif" id="3"/> "src" is property with value
//          "hello.gif" and "id" is prop. with value "3".

class WXDLLIMPEXP_XML wxXmlAttribute
{
public:
    wxXmlAttribute()  = default;
    wxXmlAttribute(const wxString& name, const wxString& value,
                  wxXmlAttribute *next = nullptr)
            : m_name(name), m_value(value), m_next(next) {}
    virtual ~wxXmlAttribute() = default;

    const wxString& GetName() const { return m_name; }
    const wxString& GetValue() const { return m_value; }
    wxXmlAttribute *GetNext() const { return m_next; }

    void SetName(const wxString& name) { m_name = name; }
    void SetValue(const wxString& value) { m_value = value; }
    void SetNext(wxXmlAttribute *next) { m_next = next; }

private:
    wxString m_name;
    wxString m_value;
    wxXmlAttribute *m_next{nullptr};
};

// Represents node in XML document. Node has name and may have content and
// attributes. Most common node types are wxXML_TEXT_NODE (name and attributes
// are irrelevant) and wxXML_ELEMENT_NODE (e.g. in <title>hi</title> there is
// element with name="title", irrelevant content and one child (wxXML_TEXT_NODE
// with content="hi").
class WXDLLIMPEXP_XML wxXmlNode
{
public:
    wxXmlNode() = default;

    wxXmlNode(wxXmlNode *parent, wxXmlNodeType type,
              const wxString& name, const wxString& content = {},
              wxXmlAttribute *attrs = nullptr, wxXmlNode *next = nullptr,
              int lineNo = -1);

    virtual ~wxXmlNode();

    // copy ctor & operator=. Note that this does NOT copy siblings
    // and parent pointer, i.e. m_parent and m_next will be NULL
    // after using copy ctor and are never unmodified by operator=.
    // On the other hand, it DOES copy children and attributes.
    wxXmlNode(const wxXmlNode& node);
    wxXmlNode& operator=(const wxXmlNode& node);

    // user-friendly creation:
    wxXmlNode(wxXmlNodeType type, const wxString& name,
              const wxString& content = {},
              int lineNo = -1);
    virtual void AddChild(wxXmlNode *child);
    virtual bool InsertChild(wxXmlNode *child, wxXmlNode *followingNode);
    virtual bool InsertChildAfter(wxXmlNode *child, wxXmlNode *precedingNode);
    virtual bool RemoveChild(wxXmlNode *child);
    virtual void AddAttribute(const wxString& name, const wxString& value);
    virtual bool DeleteAttribute(const wxString& name);

    // access methods:
    wxXmlNodeType GetType() const { return m_type; }
    const wxString& GetName() const { return m_name; }
    const wxString& GetContent() const { return m_content; }

    bool IsWhitespaceOnly() const;
    int GetDepth(wxXmlNode *grandparent = nullptr) const;

    // Gets node content from wxXML_ENTITY_NODE
    // The problem is, <tag>content<tag> is represented as
    // wxXML_ENTITY_NODE name="tag", content=""
    //    |-- wxXML_TEXT_NODE or
    //        wxXML_CDATA_SECTION_NODE name="" content="content"
    wxString GetNodeContent() const;

    wxXmlNode *GetParent() const { return m_parent; }
    wxXmlNode *GetNext() const { return m_next; }
    wxXmlNode *GetChildren() const { return m_children; }

    wxXmlAttribute *GetAttributes() const { return m_attrs; }
    bool GetAttribute(const wxString& attrName, wxString* value) const;
    wxString GetAttribute(const wxString& attrName,
                          const wxString& defaultVal = "") const;
    bool HasAttribute(const wxString& attrName) const;

    int GetLineNumber() const { return m_lineNo; }

    void SetType(wxXmlNodeType type) { m_type = type; }
    void SetName(const wxString& name) { m_name = name; }
    void SetContent(const wxString& con) { m_content = con; }

    void SetParent(wxXmlNode *parent) { m_parent = parent; }
    void SetNext(wxXmlNode *next) { m_next = next; }
    void SetChildren(wxXmlNode *child) { m_children = child; }

    void SetAttributes(wxXmlAttribute *attr) { m_attrs = attr; }
    virtual void AddAttribute(wxXmlAttribute *attr);

    // If true, don't do encoding conversion to improve efficiency - node content is ASCII text
    bool GetNoConversion() const { return m_noConversion; }
    void SetNoConversion(bool noconversion) { m_noConversion = noconversion; }

private:
    void AddProperty(const wxString& name, const wxString& value);
    bool DeleteProperty(const wxString& name);
    void AddProperty(wxXmlAttribute *attr);

private:
    wxXmlNodeType m_type;
    wxString m_name;
    wxString m_content;
    wxXmlAttribute *m_attrs{nullptr};
    wxXmlNode* m_parent{nullptr};
    wxXmlNode* m_children{nullptr};
    wxXmlNode* m_next{nullptr};
    int m_lineNo{-1}; // line number in original file, or -1
    bool m_noConversion{false}; // don't do encoding conversion - node is plain text

    void DoFree();
    void DoCopy(const wxXmlNode& node);
};

class WXDLLIMPEXP_XML wxXmlDoctype
{
public:
    explicit
    wxXmlDoctype(const wxString& rootName = {},
                 const wxString& systemId = {},
                 const wxString& publicId = {})
                 : m_rootName(rootName),
                   m_systemId(systemId),
                   m_publicId(publicId)
                 {}

    // Default copy ctor and assignment operators are ok.

    bool IsValid() const;
    void Clear();

    const wxString& GetRootName() const { return m_rootName; }
    const wxString& GetSystemId() const { return m_systemId; }
    const wxString& GetPublicId() const { return m_publicId; }

    wxString GetFullString() const;

private:
    wxString m_rootName;
    wxString m_systemId;
    wxString m_publicId;
};



// special indentation value for wxXmlDocument::Save
#define wxXML_NO_INDENTATION           (-1)

// flags for wxXmlDocument::Load
enum wxXmlDocumentLoadFlag
{
    wxXMLDOC_NONE = 0,
    wxXMLDOC_KEEP_WHITESPACE_NODES = 1
};


// This class holds XML data/document as parsed by XML parser.

class WXDLLIMPEXP_XML wxXmlDocument : public wxObject
{
public:
    wxXmlDocument();
    wxXmlDocument(const wxString& filename,
                  const wxString& encoding = "UTF-8");
    wxXmlDocument(wxInputStream& stream,
                  const wxString& encoding = "UTF-8");
    ~wxXmlDocument() { wxDELETE(m_docNode); }

    wxXmlDocument(const wxXmlDocument& doc);
    wxXmlDocument& operator=(const wxXmlDocument& doc);

    // Parses .xml file and loads data. Returns TRUE on success, FALSE
    // otherwise.
    virtual bool Load(const wxString& filename,
                      const wxString& encoding = "UTF-8", int flags = wxXMLDOC_NONE);
    virtual bool Load(wxInputStream& stream,
                      const wxString& encoding = "UTF-8", int flags = wxXMLDOC_NONE);

    // Saves document as .xml file.
    virtual bool Save(const wxString& filename, int indentstep = 2) const;
    virtual bool Save(wxOutputStream& stream, int indentstep = 2) const;

    bool IsOk() const { return GetRoot() != nullptr; }

    // Returns root node of the document.
    wxXmlNode *GetRoot() const;
    // Returns the document node.
    wxXmlNode *GetDocumentNode() const { return m_docNode; }


    // Returns version of document (may be empty).
    const wxString& GetVersion() const { return m_version; }
    // Returns encoding of document (may be empty).
    // Note: this is the encoding original file was saved in, *not* the
    // encoding of in-memory representation!
    const wxString& GetFileEncoding() const { return m_fileEncoding; }
    const wxXmlDoctype& GetDoctype() const { return m_doctype; }
    // Returns file type of document
    wxTextFileType GetFileType() const { return m_fileType; }
    wxString GetEOL() const { return m_eol; }

    // Write-access methods:
    wxXmlNode *DetachDocumentNode() { wxXmlNode *old=m_docNode; m_docNode=nullptr; return old; }
    void SetDocumentNode(wxXmlNode *node) { wxDELETE(m_docNode); m_docNode = node; }
    wxXmlNode *DetachRoot();
    void SetRoot(wxXmlNode *node);
    void SetVersion(const wxString& version) { m_version = version; }
    void SetFileEncoding(const wxString& encoding) { m_fileEncoding = encoding; }
    void SetDoctype(const wxXmlDoctype& doctype) { m_doctype = doctype; }
    void SetFileType(wxTextFileType fileType);
    void AppendToProlog(wxXmlNode *node);

    static wxVersionInfo GetLibraryVersionInfo();

private:
    wxString   m_version;
    wxString   m_fileEncoding;
    wxXmlDoctype m_doctype;
    wxXmlNode *m_docNode{nullptr};
    wxTextFileType m_fileType;
    wxString m_eol;

    void DoCopy(const wxXmlDocument& doc);

    wxDECLARE_CLASS(wxXmlDocument);
};

#endif // wxUSE_XML

#endif // _WX_XML_H_
