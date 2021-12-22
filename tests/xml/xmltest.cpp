///////////////////////////////////////////////////////////////////////////////
// Name:        tests/xml/xmltest.cpp
// Purpose:     XML classes unit test
// Author:      Vaclav Slavik
// Created:     2008-03-29
// Copyright:   (c) 2008 Vaclav Slavik
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#include "wx/xml/xml.h"

import WX.Cmn.StrStream;

// ----------------------------------------------------------------------------
// helpers for testing XML tree
// ----------------------------------------------------------------------------

namespace
{

template<typename XNode, typename... ChildNodes>
void CheckXml(XNode* n, ChildNodes&&... child_nodes)
{
    wxXmlNode *child = n->GetChildren();

    std::vector<wxString> childNodesInOrder { child_nodes... }; 

    for (const auto& childNode : childNodesInOrder)
    {
        CHECK( child );
        CHECK_EQ( childNode, child->GetName() );
        CHECK( child->GetChildren() == nullptr );
        CHECK( child->GetParent() == n );

        child = child->GetNext();
    }
}

} // anon namespace

TEST_CASE("InsertChild")
{
    auto root = std::make_unique<wxXmlNode>(wxXML_ELEMENT_NODE, "root");
    root->AddChild(new wxXmlNode(wxXML_ELEMENT_NODE, "1"));
    wxXmlNode *two = new wxXmlNode(wxXML_ELEMENT_NODE, "2");
    root->AddChild(two);
    root->AddChild(new wxXmlNode(wxXML_ELEMENT_NODE, "3"));
    CheckXml(root.get(), "1", "2", "3");

    // check inserting in front:
    root->InsertChild(new wxXmlNode(wxXML_ELEMENT_NODE, "A"), nullptr);
    CheckXml(root.get(), "A", "1", "2", "3");
    root->InsertChild(new wxXmlNode(wxXML_ELEMENT_NODE, "B"), root->GetChildren());
    CheckXml(root.get(), "B", "A", "1", "2", "3");

    // and in the middle:
    root->InsertChild(new wxXmlNode(wxXML_ELEMENT_NODE, "C"), two);
    CheckXml(root.get(), "B", "A", "1", "C", "2", "3");
}

TEST_CASE("InsertChildAfter")
{
    auto root = std::make_unique<wxXmlNode>(wxXML_ELEMENT_NODE, "root");

    root->InsertChildAfter(new wxXmlNode(wxXML_ELEMENT_NODE, "1"), nullptr);
    CheckXml(root.get(), "1");

    wxXmlNode *two = new wxXmlNode(wxXML_ELEMENT_NODE, "2");
    root->AddChild(two);
    wxXmlNode *three = new wxXmlNode(wxXML_ELEMENT_NODE, "3");
    root->AddChild(three);
    CheckXml(root.get(), "1", "2", "3");

    // check inserting in the middle:
    root->InsertChildAfter(new wxXmlNode(wxXML_ELEMENT_NODE, "A"), root->GetChildren());
    CheckXml(root.get(), "1", "A", "2", "3");
    root->InsertChildAfter(new wxXmlNode(wxXML_ELEMENT_NODE, "B"), two);
    CheckXml(root.get(), "1", "A", "2", "B", "3");

    // and at the end:
    root->InsertChildAfter(new wxXmlNode(wxXML_ELEMENT_NODE, "C"), three);
    CheckXml(root.get(), "1", "A", "2", "B", "3", "C");
}

TEST_CASE("LoadSave")
{
    // NB: this is not real XRC but rather some XRC-like XML fragment which
    //     exercises different XML constructs to check that they're saved back
    //     correctly
    //
    // Also note that there should be no blank lines here as they disappear
    // after saving.
    const char *xmlText =
"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
"<resource xmlns=\"http://www.wxwidgets.org/wxxrc\" version=\"2.3.0.1\">\n"
"  <!-- Test comment -->\n"
"  <object class=\"wxDialog\" name=\"my_dialog\">\n"
"    <children>\n"
"      <grandchild id=\"1\"/>\n"
"    </children>\n"
"    <subobject/>\n"
"  </object>\n"
"</resource>\n"
    ;

    wxStringInputStream sis(xmlText);

    wxXmlDocument doc;
    CHECK( doc.Load(sis) );

    wxStringOutputStream sos;
    CHECK( doc.Save(sos) );

    CHECK_EQ( xmlText, sos.GetString() );


    const char *utf8xmlText =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<word>\n"
"  <lang name=\"fr\">\xc3\xa9t\xc3\xa9</lang>\n"
"  <lang name=\"ru\">\xd0\xbb\xd0\xb5\xd1\x82\xd0\xbe</lang>\n"
"</word>\n"
    ;

    wxStringInputStream sis8(wxString::FromUTF8(utf8xmlText));
    CHECK( doc.Load(sis8) );

    // this contents can't be represented in Latin-1 as it contains Cyrillic
    // letters
    doc.SetFileEncoding("ISO-8859-1");
    CHECK( !doc.Save(sos) );

    // but it should work in UTF-8
    wxStringOutputStream sos8;
    doc.SetFileEncoding("UTF-8");
    CHECK( doc.Save(sos8) );
    CHECK_EQ( wxString(utf8xmlText),
                          wxString(sos8.GetString().ToUTF8()) );

    const char *xmlTextProlog =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<!DOCTYPE resource PUBLIC \"Public-ID\" 'System\"ID\"'>\n"
"<!-- Prolog comment -->\n"
"<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n"
"<resource xmlns=\"http://www.wxwidgets.org/wxxrc\" version=\"2.3.0.1\">\n"
"  <!-- Test comment -->\n"
"  <object class=\"wxDialog\" name=\"my_dialog\">\n"
"    <children>\n"
"      <grandchild id=\"1\"/>\n"
"    </children>\n"
"    <subobject/>\n"
"  </object>\n"
"</resource>\n"
"<!-- Trailing comment -->\n"
    ;

    wxStringInputStream sisp(xmlTextProlog);
    CHECK( doc.Load(sisp, "UTF-8") );

    wxStringOutputStream sosp;
    CHECK( doc.Save(sosp) );

    CHECK_EQ( xmlTextProlog, sosp.GetString() );
}

TEST_CASE("CDATA")
{
    const char *xmlText =
        "<?xml version=\"1.0\" encoding=\"windows-1252\"?>\n"
        "<name>\n"
        "  <![CDATA[Giovanni Mittone]]>\n"
        "</name>\n"
    ;

    wxStringInputStream sis(xmlText);
    wxXmlDocument doc;
    CHECK( doc.Load(sis) );

    wxXmlNode *n = doc.GetRoot();
    CHECK( n );

    n = n->GetChildren();
    CHECK( n );

    // check that both leading ("  ") and trailing white space is not part of
    // the node contents when CDATA is used and wxXMLDOC_KEEP_WHITESPACE_NODES
    // is not
    CHECK_EQ( "Giovanni Mittone", n->GetContent() );
}

TEST_CASE("PI")
{
    const char *xmlText =
        "<?xml version=\"1.0\" encoding=\"windows-1252\"?>\n"
        "<root>\n"
        "  <?robot index=\"no\" follow=\"no\"?>\n"
        "</root>\n"
    ;

    wxStringInputStream sis(xmlText);
    wxXmlDocument doc;
    CHECK( doc.Load(sis) );

    wxXmlNode *n = doc.GetRoot();
    CHECK( n );

    n = n->GetChildren();
    CHECK( n );

    CHECK_EQ( "index=\"no\" follow=\"no\"", n->GetContent() );
}

TEST_CASE("Escaping")
{
    // Verify that attribute values are escaped correctly, see
    // https://trac.wxwidgets.org/ticket/12275

    const char *xmlText =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<root text=\"hello&#xD;&#xA;this is a new line\">\n"
"  <x/>\n"
"</root>\n"
    ;

    wxStringInputStream sis(xmlText);

    wxXmlDocument doc;
    CHECK( doc.Load(sis) );

    wxStringOutputStream sos;
    CHECK( doc.Save(sos) );

    CHECK_EQ( xmlText, sos.GetString() );
}

TEST_CASE("DetachRoot")
{
    const char *xmlTextProlog =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<!-- Prolog comment -->\n"
"<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n"
"<resource xmlns=\"http://www.wxwidgets.org/wxxrc\" version=\"2.3.0.1\">\n"
"  <!-- Test comment -->\n"
"  <object class=\"wxDialog\" name=\"my_dialog\">\n"
"    <children>\n"
"      <grandchild id=\"1\"/>\n"
"    </children>\n"
"    <subobject/>\n"
"  </object>\n"
"</resource>\n"
"<!-- Trailing comment -->\n"
    ;
    const char *xmlTextHtm =
"<?xml version=\"1.0\" encoding=\"windows-1252\"?>\n"
"<html>\n"
"  <head>\n"
"    <title>Testing wxXml</title>\n"
"  </head>\n"
"  <body>\n"
"    <p>Some body text</p>\n"
"  </body>\n"
"</html>\n"
    ;
    wxXmlDocument doc;

    wxStringInputStream sish(xmlTextHtm);
    CHECK( doc.Load(sish) );

    wxXmlNode *root = doc.DetachRoot();

    wxStringInputStream sisp(xmlTextProlog);
    CHECK( doc.Load(sisp) );

    doc.SetRoot(root);

    wxStringOutputStream sos;
    CHECK( doc.Save(sos) );

    const char *xmlTextResult1 =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<!-- Prolog comment -->\n"
"<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n"
"<html>\n"
"  <head>\n"
"    <title>Testing wxXml</title>\n"
"  </head>\n"
"  <body>\n"
"    <p>Some body text</p>\n"
"  </body>\n"
"</html>\n"
"<!-- Trailing comment -->\n"
    ;
    CHECK_EQ( xmlTextResult1, sos.GetString() );

    wxStringInputStream sisp2(xmlTextProlog);
    CHECK( doc.Load(sisp2) );

    root = doc.DetachRoot();

    wxStringInputStream sish2(xmlTextHtm);
    CHECK( doc.Load(sish2) );

    doc.SetRoot(root);

    wxStringOutputStream sos2;
    CHECK( doc.Save(sos2) );

    const char *xmlTextResult2 =
"<?xml version=\"1.0\" encoding=\"windows-1252\"?>\n"
"<resource xmlns=\"http://www.wxwidgets.org/wxxrc\" version=\"2.3.0.1\">\n"
"  <!-- Test comment -->\n"
"  <object class=\"wxDialog\" name=\"my_dialog\">\n"
"    <children>\n"
"      <grandchild id=\"1\"/>\n"
"    </children>\n"
"    <subobject/>\n"
"  </object>\n"
"</resource>\n"
    ;
    CHECK_EQ( xmlTextResult2, sos2.GetString() );
}

TEST_CASE("AppendToProlog")
{
    const char *xmlText =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<root>\n"
"  <p>Some text</p>\n"
"</root>\n"
    ;
    wxXmlDocument rootdoc;
    wxStringInputStream sis(xmlText);
    CHECK( rootdoc.Load(sis) );
    wxXmlNode *root = rootdoc.DetachRoot();

    wxXmlNode *comment1 = new wxXmlNode(wxXML_COMMENT_NODE, "comment",
        " 1st prolog entry ");
    wxXmlNode *pi = new wxXmlNode(wxXML_PI_NODE, "xml-stylesheet",
        "href=\"style.css\" type=\"text/css\"");
    wxXmlNode *comment2 = new wxXmlNode(wxXML_COMMENT_NODE, "comment",
        " 3rd prolog entry ");

    wxXmlDocument doc;
    doc.AppendToProlog( comment1 );
    doc.AppendToProlog( pi );
    doc.SetRoot( root );
    doc.AppendToProlog( comment2 );

    wxStringOutputStream sos;
    CHECK( doc.Save(sos) );

    const char *xmlTextResult =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<!-- 1st prolog entry -->\n"
"<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n"
"<!-- 3rd prolog entry -->\n"
"<root>\n"
"  <p>Some text</p>\n"
"</root>\n"
    ;
    CHECK_EQ( xmlTextResult, sos.GetString() );
}

TEST_CASE("SetRoot")
{
    wxXmlDocument doc;
    CHECK( !doc.IsOk() );
    wxXmlNode *root = new wxXmlNode(wxXML_ELEMENT_NODE, "root");

    // Test for the problem of https://trac.wxwidgets.org/ticket/13135
    doc.SetRoot( root );
    wxXmlNode *docNode = doc.GetDocumentNode();
    CHECK( docNode );
    CHECK( root == docNode->GetChildren() );
    CHECK( doc.IsOk() );

    // Other tests.
    CHECK( docNode == root->GetParent() );
    doc.SetRoot(nullptr); // Removes from doc but dosn't free mem, doc node left.
    CHECK( !doc.IsOk() );

    wxXmlNode *comment = new wxXmlNode(wxXML_COMMENT_NODE, "comment", "Prolog Comment");
    wxXmlNode *pi = new wxXmlNode(wxXML_PI_NODE, "target", "PI instructions");
    doc.AppendToProlog(comment);
    doc.SetRoot( root );
    doc.AppendToProlog(pi);
    CHECK( doc.IsOk() );
    wxXmlNode *node = docNode->GetChildren();
    CHECK( node );
    CHECK( node->GetType() == wxXML_COMMENT_NODE );
    CHECK( node->GetParent() == docNode );
    node = node->GetNext();
    CHECK( node );
    CHECK( node->GetType() == wxXML_PI_NODE );
    CHECK( node->GetParent() == docNode );
    node = node->GetNext();
    CHECK( node );
    CHECK( node->GetType() == wxXML_ELEMENT_NODE );
    CHECK( node->GetParent() == docNode );
    node = node->GetNext();
    CHECK( !node );
    doc.SetRoot(nullptr);
    CHECK( !doc.IsOk() );
    doc.SetRoot(root);
    CHECK( doc.IsOk() );
}

TEST_CASE("CopyNode")
{
    const char *xmlText =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<root>\n"
"  <first><sub1/><sub2/><sub3/></first>\n"
"  <second/>\n"
"</root>\n"
    ;
    wxXmlDocument doc;
    wxStringInputStream sis(xmlText);
    CHECK( doc.Load(sis) );

    wxXmlNode* const root = doc.GetRoot();
    CHECK( root );

    wxXmlNode* const first = root->GetChildren();
    CHECK( first );

    wxXmlNode* const second = first->GetNext();
    CHECK( second );

    *first = *second;

    wxStringOutputStream sos;
    CHECK( doc.Save(sos) );

    const char *xmlTextResult =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<root>\n"
"  <second/>\n"
"  <second/>\n"
"</root>\n"
    ;
    CHECK_EQ( xmlTextResult, sos.GetString() );
}

TEST_CASE("CopyDocument")
{
    const char *xmlText =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<!DOCTYPE resource PUBLIC \"Public-ID\" \"System'ID'\">\n"
"<!-- 1st prolog entry -->\n"
"<root>\n"
"  <first>Text</first>\n"
"  <second/>\n"
"</root>\n"
    ;
    wxXmlDocument doc1;
    wxStringInputStream sis(xmlText);
    CHECK( doc1.Load(sis) );

    wxXmlDocument doc2 = doc1;

    wxStringOutputStream sos;
    CHECK(doc2.Save(sos));

    CHECK_EQ( xmlText, sos.GetString() );
}

TEST_CASE("Doctype")
{
    const char *xmlText =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE root PUBLIC \"Public-ID\" 'System\"ID\"'>\n"
        "<root>\n"
        "  <content/>\n"
        "</root>\n"
    ;

    wxStringInputStream sis(xmlText);
    wxXmlDocument doc;
    CHECK( doc.Load(sis) );

    wxXmlDoctype dt = doc.GetDoctype();

    CHECK_EQ( "root", dt.GetRootName() );
    CHECK_EQ( "System\"ID\"", dt.GetSystemId() );
    CHECK_EQ( "Public-ID", dt.GetPublicId() );

    CHECK( dt.IsValid() );
    CHECK_EQ( "root PUBLIC \"Public-ID\" 'System\"ID\"'", dt.GetFullString() );
    dt = wxXmlDoctype( dt.GetRootName(), dt.GetSystemId() );
    CHECK( dt.IsValid() );
    CHECK_EQ( "root SYSTEM 'System\"ID\"'", dt.GetFullString() );
    dt = wxXmlDoctype( dt.GetRootName() );
    CHECK( dt.IsValid() );
    CHECK_EQ( "root", dt.GetFullString() );

    doc.SetDoctype(dt);
    wxStringOutputStream sos;
    CHECK(doc.Save(sos));
    const char *xmlText1 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE root>\n"
        "<root>\n"
        "  <content/>\n"
        "</root>\n"
    ;
    CHECK_EQ( xmlText1, sos.GetString() );

    doc.SetDoctype(wxXmlDoctype());
    wxStringOutputStream sos2;
    CHECK(doc.Save(sos2));
    const char *xmlText2 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<root>\n"
        "  <content/>\n"
        "</root>\n"
    ;
    CHECK_EQ( xmlText2, sos2.GetString() );

    doc.SetDoctype(wxXmlDoctype("root", "Sys'id"));
    wxStringOutputStream sos3;
    CHECK(doc.Save(sos3));
    const char *xmlText3 =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE root SYSTEM \"Sys'id\">\n"
        "<root>\n"
        "  <content/>\n"
        "</root>\n"
    ;
    CHECK_EQ( xmlText3, sos3.GetString() );

    dt = wxXmlDoctype( "", "System\"ID\"", "Public-ID" );
    CHECK( !dt.IsValid() );
    CHECK_EQ( "", dt.GetFullString() );
    // Strictly speaking, this is illegal for XML but is legal for SGML.
    dt = wxXmlDoctype( "root", "", "Public-ID" );
    CHECK( dt.IsValid() );
    CHECK_EQ( "root PUBLIC \"Public-ID\"", dt.GetFullString() );

    // Using both single and double quotes in system ID is not allowed.
    dt = wxXmlDoctype( "root", "O'Reilly (\"editor\")", "Public-ID" );
    CHECK( !dt.IsValid() );
}
