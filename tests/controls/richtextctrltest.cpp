///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/richtextctrltest.cpp
// Purpose:     wxRichTextCtrl unit test
// Author:      Steven Lamerton
// Created:     2010-07-07
// Copyright:   (c) 2010 Steven Lamerton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_RICHTEXT


#ifndef WX_PRECOMP
    #include "wx/app.h"
#endif // WX_PRECOMP

#include "wx/richtext/richtextctrl.h"
#include "wx/richtext/richtextstyles.h"
#include "testableframe.h"
#include "asserthelper.h"
#include "wx/uiaction.h"


// Helper function for Table test
static wxRichTextTable* GetCurrentTableInstance(wxRichTextParagraph* para)
{
    wxRichTextTable* table = wxDynamicCast(para->FindObjectAtPosition(0), wxRichTextTable);
    CHECK(table);
    return table;
}

TEST_CASE("Richtext control test")
{
    auto m_rich = std::make_unique<wxRichTextCtrl>(wxTheApp->GetTopWindow(),
                                                   wxID_ANY, "",
                                                   wxDefaultPosition,
                                                   wxSize(400, 200),
                                                   wxWANTS_CHARS);

    SUBCASE("IsModified")
    {
        CHECK_EQ( false, m_rich->IsModified() );
        m_rich->WriteText("abcdef");
        CHECK_EQ( true, m_rich->IsModified() );
    }

#if wxUSE_UIACTIONSIMULATOR

    SUBCASE("CharacterEvent")
    {
        EventCounter character(m_rich.get(), wxEVT_RICHTEXT_CHARACTER);
        EventCounter content(m_rich.get(), wxEVT_RICHTEXT_CONTENT_INSERTED);

        m_rich->SetFocus();

        wxUIActionSimulator sim;
        sim.Text("abcdef");
        wxYield();

        CHECK_EQ(6, character.GetCount());
        CHECK_EQ(6, content.GetCount());

        character.Clear();
        content.Clear();

        //As these are not characters they shouldn't count
        sim.Char(WXK_RETURN);
        sim.Char(WXK_SHIFT);
        wxYield();

        CHECK_EQ(0, character.GetCount());
        CHECK_EQ(1, content.GetCount());
    }

    SUBCASE("DeleteEvent")
    {
        EventCounter deleteevent(m_rich.get(), wxEVT_RICHTEXT_DELETE);
        EventCounter contentdelete(m_rich.get(), wxEVT_RICHTEXT_CONTENT_DELETED);

        m_rich->SetFocus();

        wxUIActionSimulator sim;
        sim.Text("abcdef");
        sim.Char(WXK_BACK);
        sim.Char(WXK_DELETE);
        wxYield();

        CHECK_EQ(2, deleteevent.GetCount());
        //Only one as the delete doesn't delete anthing
        CHECK_EQ(1, contentdelete.GetCount());
    }

    SUBCASE("ReturnEvent")
    {
        EventCounter returnevent(m_rich.get(), wxEVT_RICHTEXT_RETURN);

        m_rich->SetFocus();

        wxUIActionSimulator sim;
        sim.Char(WXK_RETURN);
        wxYield();

        CHECK_EQ(1, returnevent.GetCount());
    }

    SUBCASE("UrlEvent")
    {
        EventCounter url(m_rich.get(), wxEVT_TEXT_URL);

        m_rich->BeginURL("http://www.wxwidgets.org");
        m_rich->WriteText("http://www.wxwidgets.org");
        m_rich->EndURL();

        wxUIActionSimulator sim;
        sim.MouseMove(m_rich->ClientToScreen(wxPoint(10, 10)));
        wxYield();

        sim.MouseClick();
        wxYield();

        CHECK_EQ(1, url.GetCount());
    }

    SUBCASE("TextEvent")
    {
        EventCounter updated(m_rich.get(), wxEVT_TEXT);

        m_rich->SetFocus();

        wxUIActionSimulator sim;
        sim.Text("abcdef");
        wxYield();

        CHECK_EQ("abcdef", m_rich->GetValue());
        CHECK_EQ(6, updated.GetCount());
    }

    SUBCASE("Editable")
    {
        EventCounter updated(m_rich.get(), wxEVT_TEXT);

        m_rich->SetFocus();

        wxUIActionSimulator sim;
        sim.Text("abcdef");
        wxYield();

        CHECK_EQ("abcdef", m_rich->GetValue());
        CHECK_EQ(6, updated.GetCount());
        updated.Clear();

        m_rich->SetEditable(false);
        sim.Text("gh");
        wxYield();

        CHECK_EQ("abcdef", m_rich->GetValue());
        CHECK_EQ(0, updated.GetCount());
    }
#endif

    SUBCASE("StyleEvent")
    {
        EventCounter stylechanged(m_rich.get(), wxEVT_RICHTEXT_STYLE_CHANGED);

        m_rich->SetValue("Sometext");
        m_rich->SetStyle(0, 8, wxTextAttr(*wxRED, *wxWHITE));

        CHECK_EQ(1, stylechanged.GetCount());
    }

    SUBCASE("BufferResetEvent")
    {
        EventCounter reset(m_rich.get(), wxEVT_RICHTEXT_BUFFER_RESET);

        m_rich->AppendText("more text!");
        m_rich->SetValue("");

        CHECK_EQ(1, reset.GetCount());

        reset.Clear();
        m_rich->AppendText("more text!");
        m_rich->Clear();

        CHECK_EQ(1, reset.GetCount());

        reset.Clear();

        //We expect a buffer reset here as setvalue clears the existing text
        m_rich->SetValue("replace");
        CHECK_EQ(1, reset.GetCount());
    }

#ifndef __WXOSX__
    SUBCASE("CutCopyPaste")
    {
        m_rich->AppendText("sometext");
        m_rich->SelectAll();

        if(m_rich->CanCut() && m_rich->CanPaste())
        {
            m_rich->Cut();
            CHECK(m_rich->IsEmpty());

            wxYield();

            m_rich->Paste();
            CHECK_EQ("sometext", m_rich->GetValue());
        }

        m_rich->SelectAll();

        if(m_rich->CanCopy() && m_rich->CanPaste())
        {
            m_rich->Copy();
            m_rich->Clear();
            CHECK(m_rich->IsEmpty());

            wxYield();

            m_rich->Paste();
            CHECK_EQ("sometext", m_rich->GetValue());
        }
    }
#endif

    SUBCASE("UndoRedo")
    {
        m_rich->AppendText("sometext");

        CHECK(m_rich->CanUndo());

        m_rich->Undo();

        CHECK(m_rich->IsEmpty());
        CHECK(m_rich->CanRedo());

        m_rich->Redo();

        CHECK_EQ("sometext", m_rich->GetValue());

        m_rich->AppendText("Batch undo");
        m_rich->SelectAll();

        //Also test batch operations
        m_rich->BeginBatchUndo("batchtest");

        m_rich->ApplyBoldToSelection();
        m_rich->ApplyItalicToSelection();

        m_rich->EndBatchUndo();

        CHECK(m_rich->CanUndo());

        m_rich->Undo();

        CHECK(!m_rich->IsSelectionBold());
        CHECK(!m_rich->IsSelectionItalics());
        CHECK(m_rich->CanRedo());

        m_rich->Redo();

        CHECK(m_rich->IsSelectionBold());
        CHECK(m_rich->IsSelectionItalics());

        //And surpressing undo
        m_rich->BeginSuppressUndo();

        m_rich->AppendText("Can't undo this");

        CHECK(m_rich->CanUndo());

        m_rich->EndSuppressUndo();
    }

    SUBCASE("CaretPosition")
    {
        m_rich->AddParagraph("This is paragraph one");
        m_rich->AddParagraph("Paragraph two\n has \nlots of\n lines");

        m_rich->SetInsertionPoint(2);

        CHECK_EQ(1, m_rich->GetCaretPosition());

        m_rich->MoveToParagraphStart();

        CHECK_EQ(0, m_rich->GetCaretPosition());

        m_rich->MoveRight();
        m_rich->MoveRight(2);
        m_rich->MoveLeft(1);
        m_rich->MoveLeft(0);

        CHECK_EQ(2, m_rich->GetCaretPosition());

        m_rich->MoveToParagraphEnd();

        CHECK_EQ(21, m_rich->GetCaretPosition());

        m_rich->MoveToLineStart();

        CHECK_EQ(0, m_rich->GetCaretPosition());

        m_rich->MoveToLineEnd();

        CHECK_EQ(21, m_rich->GetCaretPosition());
    }

    SUBCASE("Selection")
    {
        m_rich->SetValue("some more text");

        m_rich->SelectAll();

        CHECK_EQ("some more text", m_rich->GetStringSelection());

        m_rich->SelectNone();

        CHECK_EQ("", m_rich->GetStringSelection());

        m_rich->SelectWord(1);

        CHECK_EQ("some", m_rich->GetStringSelection());

        m_rich->SetSelection(5, 14);

        CHECK_EQ("more text", m_rich->GetStringSelection());

        wxRichTextRange range(5, 9);

        m_rich->SetSelectionRange(range);

        CHECK_EQ("more", m_rich->GetStringSelection());
    }

    SUBCASE("Range")
    {
        wxRichTextRange range(0, 10);

        CHECK_EQ(0, range.GetStart());
        CHECK_EQ(10, range.GetEnd());
        CHECK_EQ(11, range.GetLength());
        CHECK(range.Contains(5));

        wxRichTextRange outside(12, 14);

        CHECK(outside.IsOutside(range));

        wxRichTextRange inside(6, 7);

        CHECK(inside.IsWithin(range));

        range.LimitTo(inside);

        CHECK(inside == range);
        CHECK(inside + range == outside);
        CHECK(outside - range == inside);

        range.SetStart(4);
        range.SetEnd(6);

        CHECK_EQ(4, range.GetStart());
        CHECK_EQ(6, range.GetEnd());
        CHECK_EQ(3, range.GetLength());

        inside.SetRange(6, 4);
        inside.Swap();

        CHECK(inside == range);
    }

    SUBCASE("Alignment")
    {
        m_rich->SetValue("text to align");
        m_rich->SelectAll();

        m_rich->ApplyAlignmentToSelection(wxTextAttrAlignment::Right);

        CHECK(m_rich->IsSelectionAligned(wxTextAttrAlignment::Right));

        m_rich->BeginAlignment(wxTextAttrAlignment::Centre);
        m_rich->AddParagraph("middle aligned");
        m_rich->EndAlignment();

        m_rich->SetSelection(20, 25);

        CHECK(m_rich->IsSelectionAligned(wxTextAttrAlignment::Centre));
    }

    SUBCASE("Bold")
    {
        m_rich->SetValue("text to bold");
        m_rich->SelectAll();
        m_rich->ApplyBoldToSelection();

        CHECK(m_rich->IsSelectionBold());

        m_rich->BeginBold();
        m_rich->AddParagraph("bold paragraph");
        m_rich->EndBold();
        m_rich->AddParagraph("not bold paragraph");

        m_rich->SetSelection(15, 20);

        CHECK(m_rich->IsSelectionBold());

        m_rich->SetSelection(30, 35);

        CHECK(!m_rich->IsSelectionBold());
    }

    SUBCASE("Italic")
    {
        m_rich->SetValue("text to italic");
        m_rich->SelectAll();
        m_rich->ApplyItalicToSelection();

        CHECK(m_rich->IsSelectionItalics());

        m_rich->BeginItalic();
        m_rich->AddParagraph("italic paragraph");
        m_rich->EndItalic();
        m_rich->AddParagraph("not italic paragraph");

        m_rich->SetSelection(20, 25);

        CHECK(m_rich->IsSelectionItalics());

        m_rich->SetSelection(35, 40);

        CHECK(!m_rich->IsSelectionItalics());
    }

    SUBCASE("Table")
    {
        m_rich->BeginSuppressUndo();
        wxRichTextTable* table = m_rich->WriteTable(1, 1);
        m_rich->EndSuppressUndo();
        CHECK(table);
        CHECK(m_rich->CanUndo() == false);

        // Run the tests twice: first for the original table, then for a contained one
        for (int t = 0; t < 2; ++t)
        {
            // Undo() and Redo() switch table instances, so invalidating 'table'
            // The containing paragraph isn't altered, and so can be used to find the current object
            wxRichTextParagraph* para = wxDynamicCast(table->GetParent(), wxRichTextParagraph);
            CHECK(para);

            CHECK(table->GetColumnCount() == 1);
            CHECK(table->GetRowCount() == 1);

            // Test adding columns and rows
            for (size_t n = 0; n < 3; ++n)
            {
                m_rich->BeginBatchUndo("Add col and row");

                table->AddColumns(0, 1);
                table->AddRows(0, 1);

                m_rich->EndBatchUndo();
            }
            CHECK(table->GetColumnCount() == 4);
            CHECK(table->GetRowCount() == 4);

            // Test deleting columns and rows
            for (size_t n = 0; n < 3; ++n)
            {
                m_rich->BeginBatchUndo("Delete col and row");

                table->DeleteColumns(table->GetColumnCount() - 1, 1);
                table->DeleteRows(table->GetRowCount() - 1, 1);

                m_rich->EndBatchUndo();
            }
            CHECK(table->GetColumnCount() == 1);
            CHECK(table->GetRowCount() == 1);

            // Test undo, first of the deletions...
            CHECK(m_rich->CanUndo());
            for (size_t n = 0; n < 3; ++n)
            {
                m_rich->Undo();
            }
            table = GetCurrentTableInstance(para);
            CHECK(table->GetColumnCount() == 4);
            CHECK(table->GetRowCount() == 4);

            // ...then the additions
            for (size_t n = 0; n < 3; ++n)
            {
                m_rich->Undo();
            }
            table = GetCurrentTableInstance(para);
            CHECK(table->GetColumnCount() == 1);
            CHECK(table->GetRowCount() == 1);
            CHECK(m_rich->CanUndo() == false);

            // Similarly test redo. Additions:
            CHECK(m_rich->CanRedo());
            for (size_t n = 0; n < 3; ++n)
            {
                m_rich->Redo();
            }
            table = GetCurrentTableInstance(para);
            CHECK(table->GetColumnCount() == 4);
            CHECK(table->GetRowCount() == 4);

            // Deletions:
            for (size_t n = 0; n < 3; ++n)
            {
                m_rich->Redo();
            }
            table = GetCurrentTableInstance(para);
            CHECK(table->GetColumnCount() == 1);
            CHECK(table->GetRowCount() == 1);
            CHECK(m_rich->CanRedo() == false);

            // Now test multiple addition and deletion, and also suppression
            m_rich->BeginSuppressUndo();
            table->AddColumns(0, 3);
            table->AddRows(0, 3);
            CHECK(table->GetColumnCount() == 4);
            CHECK(table->GetRowCount() == 4);

            // Only delete 2 of these. This makes it easy to be sure we're dealing with the child table when we loop
            table->DeleteColumns(0, 2);
            table->DeleteRows(0, 2);
            CHECK(table->GetColumnCount() == 2);
            CHECK(table->GetRowCount() == 2);
            m_rich->EndSuppressUndo();

            m_rich->GetCommandProcessor()->ClearCommands(); // otherwise the command-history from this loop will cause CHECK failures in the next one

            if (t == 0)
            {
                // For round 2, re-run the tests on another table inside the last cell of the first one
                wxRichTextCell* cell = table->GetCell(table->GetRowCount() - 1, table->GetColumnCount() - 1);
                CHECK(cell);
                m_rich->SetFocusObject(cell);
                m_rich->BeginSuppressUndo();
                table = m_rich->WriteTable(1, 1);
                m_rich->EndSuppressUndo();
                CHECK(table);
            }
        }

        // Test ClearTable()
        table->ClearTable();
        CHECK_EQ(0, table->GetCells().GetCount());
        CHECK_EQ(0, table->GetColumnCount());
        CHECK_EQ(0, table->GetRowCount());

        m_rich->Clear();
        m_rich->SetFocusObject(nullptr);
    }

    SUBCASE("Underline")
    {
        m_rich->SetValue("text to underline");
        m_rich->SelectAll();
        m_rich->ApplyUnderlineToSelection();

        CHECK(m_rich->IsSelectionUnderlined());

        m_rich->BeginUnderline();
        m_rich->AddParagraph("underline paragraph");
        m_rich->EndUnderline();
        m_rich->AddParagraph("not underline paragraph");

        m_rich->SetSelection(20, 25);

        CHECK(m_rich->IsSelectionUnderlined());

        m_rich->SetSelection(40, 45);

        CHECK(!m_rich->IsSelectionUnderlined());
    }

    SUBCASE("Indent")
    {
        m_rich->BeginLeftIndent(12, -5);
        m_rich->BeginRightIndent(14);
        m_rich->AddParagraph("A paragraph with indents");
        m_rich->EndLeftIndent();
        m_rich->EndRightIndent();
        m_rich->AddParagraph("No more indent");

        wxTextAttr indent;
        m_rich->GetStyle(5, indent);

        CHECK_EQ(12, indent.GetLeftIndent());
        CHECK_EQ(-5, indent.GetLeftSubIndent());
        CHECK_EQ(14, indent.GetRightIndent());

        m_rich->GetStyle(35, indent);

        CHECK_EQ(0, indent.GetLeftIndent());
        CHECK_EQ(0, indent.GetLeftSubIndent());
        CHECK_EQ(0, indent.GetRightIndent());
    }

    SUBCASE("LineSpacing")
    {
        m_rich->BeginLineSpacing(20);
        m_rich->AddParagraph("double spaced");
        m_rich->EndLineSpacing();
        m_rich->BeginLineSpacing(wxTEXT_ATTR_LINE_SPACING_HALF);
        m_rich->AddParagraph("1.5 spaced");
        m_rich->EndLineSpacing();
        m_rich->AddParagraph("normally spaced");

        wxTextAttr spacing;
        m_rich->GetStyle(5, spacing);

        CHECK_EQ(20, spacing.GetLineSpacing());

        m_rich->GetStyle(20, spacing);

        CHECK_EQ(15, spacing.GetLineSpacing());

        m_rich->GetStyle(30, spacing);

        CHECK_EQ(10, spacing.GetLineSpacing());
    }

    SUBCASE("ParagraphSpacing")
    {
        m_rich->BeginParagraphSpacing(15, 20);
        m_rich->AddParagraph("spaced paragraph");
        m_rich->EndParagraphSpacing();
        m_rich->AddParagraph("non-spaced paragraph");

        wxTextAttr spacing;
        m_rich->GetStyle(5, spacing);

        CHECK_EQ(15, spacing.GetParagraphSpacingBefore());
        CHECK_EQ(20, spacing.GetParagraphSpacingAfter());

        m_rich->GetStyle(25, spacing);

        //Make sure we test against the defaults
        CHECK_EQ(m_rich->GetBasicStyle().GetParagraphSpacingBefore(),
                             spacing.GetParagraphSpacingBefore());
        CHECK_EQ(m_rich->GetBasicStyle().GetParagraphSpacingAfter(),
                             spacing.GetParagraphSpacingAfter());
    }

    SUBCASE("TextColour")
    {
        m_rich->BeginTextColour(*wxRED);
        m_rich->AddParagraph("red paragraph");
        m_rich->EndTextColour();
        m_rich->AddParagraph("default paragraph");

        wxTextAttr colour;
        m_rich->GetStyle(5, colour);

        CHECK_EQ(*wxRED, colour.GetTextColour());

        m_rich->GetStyle(25, colour);

        CHECK_EQ(m_rich->GetBasicStyle().GetTextColour(),
                             colour.GetTextColour());
    }

    SUBCASE("NumberedBullet")
    {
        m_rich->BeginNumberedBullet(1, 15, 20);
        m_rich->AddParagraph("bullet one");
        m_rich->EndNumberedBullet();
        m_rich->BeginNumberedBullet(2, 25, -5);
        m_rich->AddParagraph("bullet two");
        m_rich->EndNumberedBullet();

        wxTextAttr bullet;
        m_rich->GetStyle(5, bullet);

        CHECK(bullet.HasBulletStyle());
        CHECK(bullet.HasBulletNumber());
        CHECK_EQ(1, bullet.GetBulletNumber());
        CHECK_EQ(15, bullet.GetLeftIndent());
        CHECK_EQ(20, bullet.GetLeftSubIndent());

        m_rich->GetStyle(15, bullet);

        CHECK(bullet.HasBulletStyle());
        CHECK(bullet.HasBulletNumber());
        CHECK_EQ(2, bullet.GetBulletNumber());
        CHECK_EQ(25, bullet.GetLeftIndent());
        CHECK_EQ(-5, bullet.GetLeftSubIndent());
    }

    SUBCASE("SymbolBullet")
    {
        m_rich->BeginSymbolBullet("*", 15, 20);
        m_rich->AddParagraph("bullet one");
        m_rich->EndSymbolBullet();
        m_rich->BeginSymbolBullet("%", 25, -5);
        m_rich->AddParagraph("bullet two");
        m_rich->EndSymbolBullet();

        wxTextAttr bullet;
        m_rich->GetStyle(5, bullet);

        CHECK(bullet.HasBulletStyle());
        CHECK(bullet.HasBulletText());
        CHECK_EQ("*", bullet.GetBulletText());
        CHECK_EQ(15, bullet.GetLeftIndent());
        CHECK_EQ(20, bullet.GetLeftSubIndent());

        m_rich->GetStyle(15, bullet);

        CHECK(bullet.HasBulletStyle());
        CHECK(bullet.HasBulletText());
        CHECK_EQ("%", bullet.GetBulletText());
        CHECK_EQ(25, bullet.GetLeftIndent());
        CHECK_EQ(-5, bullet.GetLeftSubIndent());
    }

    SUBCASE("FontSize")
    {
        m_rich->BeginFontSize(24);
        m_rich->AddParagraph("Large text");
        m_rich->EndFontSize();

        wxTextAttr size;
        m_rich->GetStyle(5, size);

        CHECK(size.HasFontSize());
        CHECK_EQ(24, size.GetFontSize());
    }

    SUBCASE("Font")
    {
        wxFont font(14, wxFONTFAMILY_DEFAULT, wxFontStyle::Normal, wxFONTWEIGHT_NORMAL);
        m_rich->BeginFont(font);
        m_rich->AddParagraph("paragraph with font");
        m_rich->EndFont();

        wxTextAttr fontstyle;
        m_rich->GetStyle(5, fontstyle);

        CHECK_EQ(font, fontstyle.GetFont());
    }

    SUBCASE("Delete")
    {
        m_rich->AddParagraph("here is a long long line in a paragraph");
        m_rich->SetSelection(0, 6);

        CHECK(m_rich->CanDeleteSelection());

        m_rich->DeleteSelection();

        CHECK_EQ("is a long long line in a paragraph", m_rich->GetValue());

        m_rich->SetSelection(0, 5);

        CHECK(m_rich->CanDeleteSelection());

        m_rich->DeleteSelectedContent();

        CHECK_EQ("long long line in a paragraph", m_rich->GetValue());

        m_rich->Delete(wxRichTextRange(14, 29));

        CHECK_EQ("long long line", m_rich->GetValue());
    }

    SUBCASE("Url")
    {
        m_rich->BeginURL("http://www.wxwidgets.org");
        m_rich->WriteText("http://www.wxwidgets.org");
        m_rich->EndURL();

        wxTextAttr url;
        m_rich->GetStyle(5, url);

        CHECK(url.HasURL());
        CHECK_EQ("http://www.wxwidgets.org", url.GetURL());
    }
}

#endif //wxUSE_RICHTEXT
