///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/textentrytest.h
// Purpose:     Base class implementing wxTextEntry unit tests
// Author:      Vadim Zeitlin
// Created:     2008-09-19 (extracted from textctrltest.cpp)
// Copyright:   (c) 2007, 2008 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TESTS_CONTROLS_TEXTENTRYTEST_H_
#define _WX_TESTS_CONTROLS_TEXTENTRYTEST_H_

class WXDLLIMPEXP_FWD_CORE wxTextEntry;

template<typename TextEntryT>
class TextEntryTest
{
protected:
    void SetValue()
    {
        CHECK( m_entry->wxTextEntryBase::IsEmpty() );

        m_entry->SetValue("foo");
        CHECK_EQ( "foo", m_entry->GetValue() );

        m_entry->SetValue("");
        CHECK( m_entry->wxTextEntryBase::IsEmpty() );

        m_entry->SetValue("hi");
        CHECK_EQ( "hi", m_entry->GetValue() );

        m_entry->SetValue("bye");
        CHECK_EQ( "bye", m_entry->GetValue() );
    }

    void TextChangeEvents()
    {
        EventCounter updated(m_entry.get(), wxEVT_TEXT);

        // notice that SetValue() generates an event even if the text didn't change
    #ifndef __WXQT__
        m_entry->SetValue("");
        CHECK_EQ( 1, updated.GetCount() );
        updated.Clear();
    #else
        WARN("Events are only sent when text changes in WxQt");
    #endif

        m_entry->SetValue("foo");
        CHECK_EQ( 1, updated.GetCount() );
        updated.Clear();

    #ifndef __WXQT__
        m_entry->SetValue("foo");
        CHECK_EQ( 1, updated.GetCount() );
        updated.Clear();
    #else
        WARN("Events are only sent when text changes in WxQt");
    #endif

        m_entry->SetValue("");
        CHECK_EQ( 1, updated.GetCount() );
        updated.Clear();

        m_entry->ChangeValue("bar");
        CHECK_EQ( 0, updated.GetCount() );

        m_entry->AppendText("bar");
        CHECK_EQ( 1, updated.GetCount() );
        updated.Clear();

        m_entry->Replace(3, 6, "baz");
        CHECK_EQ( 1, updated.GetCount() );
        updated.Clear();

        m_entry->Remove(0, 3);
        CHECK_EQ( 1, updated.GetCount() );
        updated.Clear();

        m_entry->WriteText("foo");
        CHECK_EQ( 1, updated.GetCount() );
        updated.Clear();

        m_entry->Clear();
        CHECK_EQ( 1, updated.GetCount() );
        updated.Clear();

        m_entry->ChangeValue("");
        CHECK_EQ( 0, updated.GetCount() );
        updated.Clear();

        m_entry->ChangeValue("non-empty");
        CHECK_EQ( 0, updated.GetCount() );
        updated.Clear();

        m_entry->ChangeValue("");
        CHECK_EQ( 0, updated.GetCount() );
        updated.Clear();
    }

    /*
    void CheckStringSelection(const char *sel)
    {
        //if constexr(std::is_same<m_entry)
        CHECK_EQ( sel, GetTestEntry()->GetStringSelection() );
    }
    */

    /*
    void AssertSelection(int from, int to, const char *sel)
    {
        CHECK( m_entry->HasSelection() );

        long fromReal,
            toReal;
        m_entry->GetSelection(&fromReal, &toReal);
        CHECK_EQ( from, fromReal );
        CHECK_EQ( to, toReal );

        CHECK_EQ( from, m_entry->GetInsertionPoint() );

        CheckStringSelection(sel);
    }
    */

    /*
    void Selection()
    {
        m_entry->SetValue("0123456789");

        m_entry->SetSelection(2, 4);
        AssertSelection(2, 4, "23"); // not "234"!

        m_entry->SetSelection(3, -1);
        AssertSelection(3, 10, "3456789");

        m_entry->SelectAll();
        AssertSelection(0, 10, "0123456789");

        m_entry->SetSelection(0, 0);
        CHECK( !m_entry->HasSelection() );
    }
    */
    void InsertionPoint()
    {
        CHECK_EQ( 0, m_entry->GetLastPosition() );
        CHECK_EQ( 0, m_entry->GetInsertionPoint() );

        m_entry->SetValue("0"); // should put the insertion point in front
        CHECK_EQ( 1, m_entry->GetLastPosition() );
        CHECK_EQ( 0, m_entry->GetInsertionPoint() );

        m_entry->AppendText("12"); // should update the insertion point position
        CHECK_EQ( 3, m_entry->GetLastPosition() );
        CHECK_EQ( 3, m_entry->GetInsertionPoint() );

        m_entry->SetInsertionPoint(1);
        CHECK_EQ( 3, m_entry->GetLastPosition() );
        CHECK_EQ( 1, m_entry->GetInsertionPoint() );

        m_entry->SetValue("012"); // shouldn't change the position if no real change
        CHECK_EQ( 1, m_entry->GetInsertionPoint() );

        m_entry->ChangeValue("012"); // same as for SetValue()
        CHECK_EQ( 1, m_entry->GetInsertionPoint() );

        m_entry->SetInsertionPointEnd();
        CHECK_EQ( 3, m_entry->GetInsertionPoint() );

        m_entry->SetInsertionPoint(0);
        m_entry->WriteText("-"); // should move it after the written text
        CHECK_EQ( 4, m_entry->GetLastPosition() );
        CHECK_EQ( 1, m_entry->GetInsertionPoint() );

        m_entry->SetValue("something different"); // should still reset the caret
        CHECK_EQ( 0, m_entry->GetInsertionPoint() );
    }

    void Replace()
    {
        m_entry->SetValue("Hello replace!"
                        "0123456789012");
        m_entry->SetInsertionPoint(0);

        m_entry->Replace(6, 13, "changed");

        CHECK_EQ("Hello changed!"
                            "0123456789012",
                            m_entry->GetValue());
        CHECK_EQ(13, m_entry->GetInsertionPoint());

        m_entry->Replace(13, -1, "");
        CHECK_EQ("Hello changed", m_entry->GetValue());
        CHECK_EQ(13, m_entry->GetInsertionPoint());

        m_entry->Replace(0, 6, "Un");
        CHECK_EQ("Unchanged", m_entry->GetValue());
        CHECK_EQ(2, m_entry->GetInsertionPoint());
    }

    void WriteText()
    {
        m_entry->SetValue("foo");
        m_entry->SetInsertionPoint(3);
        m_entry->WriteText("bar");
        CHECK_EQ( "foobar", m_entry->GetValue() );

        m_entry->SetValue("foo");
        m_entry->SetInsertionPoint(0);
        m_entry->WriteText("bar");
        CHECK_EQ( "barfoo", m_entry->GetValue() );

        m_entry->SetValue("abxxxhi");
        m_entry->SetSelection(2, 5);
        m_entry->WriteText("cdefg");
        CHECK_EQ( "abcdefghi", m_entry->GetValue() );
        CHECK_EQ( 7, m_entry->GetInsertionPoint() );
        CHECK_EQ( false, m_entry->HasSelection() );
    }

    #if wxUSE_UIACTIONSIMULATOR

    class TextEventHandler
    {
    public:
        explicit TextEventHandler(wxWindow* win)
            : m_win(win)
        {
            m_win->Bind(wxEVT_TEXT, &TextEventHandler::OnText, this);
        }

        ~TextEventHandler()
        {
            m_win->Unbind(wxEVT_TEXT, &TextEventHandler::OnText, this);
        }

        const wxString& GetLastString() const
        {
            return m_string;
        }

    private:
        void OnText(wxCommandEvent& event)
        {
            m_string = event.GetString();
        }

        wxWindow* const m_win;

        wxString m_string;
    };

    void Editable()
    {
        EventCounter updated(m_entry.get(), wxEVT_TEXT);

        m_entry->SetFocus();
        wxYield();

    #ifdef __WXGTK__
        // For some reason, wxBitmapComboBox doesn't appear on the screen without
        // this (due to wxTLW size hacks perhaps?). It would be nice to avoid doing
        // this, but without this hack the test often (although not always) fails.
        wxMilliSleep(50);
    #endif // __WGTK__

        // Check that we get the expected number of events.
        wxUIActionSimulator sim;
        sim.Text("abcdef");
        wxYield();

        CHECK_EQ("abcdef", m_entry->GetValue());
        CHECK_EQ(6, updated.GetCount());

        wxYield();

        // And that the event carries the right value.
        TextEventHandler handler(m_entry.get());

        sim.Text("g");
        wxYield();

        CHECK_EQ("abcdefg", handler.GetLastString());

        // ... even if we generate the event programmatically and whether it uses
        // the same value as the control has right now
        m_entry->SetValue("abcdefg");
        CHECK_EQ("abcdefg", handler.GetLastString());

        // ... or not
        m_entry->SetValue("abcdef");
        CHECK_EQ("abcdef", handler.GetLastString());

        // Check that making the control not editable does indeed prevent it from
        // being edited.
        updated.Clear();

        m_entry->SetEditable(false);
        sim.Text("gh");
        wxYield();

        CHECK_EQ("abcdef", m_entry->GetValue());
        CHECK_EQ(0, updated.GetCount());
    }

    #endif // wxUSE_UIACTIONSIMULATOR

    void Hint()
    {
        m_entry->SetHint("This is a hint");
        CHECK_EQ("", m_entry->GetValue());
    }

    void CopyPaste()
    {
    #ifndef __WXOSX__

        m_entry->AppendText("sometext");
        m_entry->SelectAll();

        if(m_entry->CanCopy() && m_entry->CanPaste())
        {
            m_entry->Copy();
            m_entry->Clear();
            CHECK(m_entry->wxTextEntryBase::IsEmpty());

            wxYield();

            m_entry->Paste();
            CHECK_EQ("sometext", m_entry->GetValue());
        }
    #endif
    }

    void UndoRedo()
    {
        m_entry->AppendText("sometext");

        if(m_entry->CanUndo())
        {
            m_entry->Undo();
            CHECK(m_entry->wxTextEntryBase::IsEmpty());

            if(m_entry->CanRedo())
            {
                m_entry->Redo();
                CHECK_EQ("sometext", m_entry->GetValue());
            }
        }
    }

    // this should be inserted in the derived class CPPUNIT_TEST_SUITE
    // definition to run all wxTextEntry tests as part of it
    #define wxTEXT_ENTRY_TESTS() \
            SUBCASE( "SetValue" ) { SetValue(); } \
            SUBCASE( "TextChangeEvents" ) { TextChangeEvents(); } \
            /*SUBCASE( "Selection" ) { Selection(); }*/ \
            SUBCASE( "InsertionPoint" ) { InsertionPoint(); } \
            SUBCASE( "Replace" ) { Replace(); } \
            SUBCASE( "Editable" ) { Editable(); } \
            SUBCASE( "Hint" ) { Hint(); } \
            SUBCASE( "CopyPaste" ) { CopyPaste(); } \
            SUBCASE( "UndoRedo" ) { UndoRedo(); } \
            SUBCASE( "WriteText" )  { WriteText(); }

    std::unique_ptr<TextEntryT> m_entry;

private:
    /*
    // Selection() test helper: verify that selection is as described by the
    // function parameters
    void AssertSelection(int from, int to, const char *sel)
    {
        wxTextEntry * const entry = GetTestEntry();

        CHECK( m_entry->HasSelection() );

        long fromReal,
            toReal;
        m_entry->GetSelection(&fromReal, &toReal);
        CHECK_EQ( from, fromReal );
        CHECK_EQ( to, toReal );

        CHECK_EQ( from, m_entry->GetInsertionPoint() );

        CheckStringSelection(sel);
    }
    */
    // helper of AssertSelection(): check that the text selected in the control
    // is the given one
    //
    // this is necessary to disable testing this in wxComboBox test as it
    // doesn't provide any way to access the string selection directly, its
    // GetStringSelection() method returns the currently selected string in the
    // wxChoice part of the control, not the selected text

    //void CheckStringSelection(const char* sel)
    //{
    //    if constexpr
    //}
};

// Helper used for creating the control of the specific type (currently either
// wxTextCtrl or wxComboBox) with the given flag.
template<typename TextCtrlT>
struct TextControlCreator
{
    // Create the control of the right type using the given parent and style.
    wxControl* Create(wxWindow* parent, int style) const = 0;

    // Return another creator similar to this one, but creating multiline
    // version of the control. If the returned pointer is non-null, it must be
    // deleted by the caller.
    std::unique_ptr<TextControlCreator> CloneAsMultiLine() const
    {
        return std::make_unique<TextControlCreator>(wxTE_MULTILINE);
    }
};

/*
// Use the given control creator to check that various combinations of
// specifying and not specifying wxTE_PROCESS_ENTER and handling or not
// handling the resulting event work as expected.

void TestProcessEnter(const TextLikeControlCreator& controlCreator)
{
    if ( !EnableUITests() )
    {
        WARN("Skipping wxTE_PROCESS_ENTER tests: wxUIActionSimulator use disabled");
        return;
    }

    SUBCASE("Without wxTE_PROCESS_ENTER")
    {
        TestDialog dlg(controlCreator, ProcessEnter_No);
        REQUIRE( dlg.ShowModal() == wxID_OK );
        CHECK( !dlg.GotEnter() );
    }

    SUBCASE("With wxTE_PROCESS_ENTER but skipping")
    {
        TestDialog dlgProcessEnter(controlCreator, ProcessEnter_ButSkip);
        REQUIRE( dlgProcessEnter.ShowModal() == wxID_OK );
        CHECK( dlgProcessEnter.GotEnter() );
    }

    SUBCASE("With wxTE_PROCESS_ENTER without skipping")
    {
        TestDialog dlgProcessEnter(controlCreator, ProcessEnter_WithoutSkipping);
        REQUIRE( dlgProcessEnter.ShowModal() == wxID_APPLY );
        CHECK( dlgProcessEnter.GotEnter() );
    }

    SUBCASE("Without wxTE_PROCESS_ENTER but with wxTE_MULTILINE")
    {
        std::unique_ptr<TextLikeControlCreator>
            multiLineCreator(controlCreator.CloneAsMultiLine());
        if ( !multiLineCreator )
            return;

        TestDialog dlg(*multiLineCreator, ProcessEnter_No);
        REQUIRE( dlg.ShowModal() == wxID_CLOSE );
        CHECK( !dlg.GotEnter() );
    }

    SUBCASE("With wxTE_PROCESS_ENTER and wxTE_MULTILINE but skipping")
    {
        std::unique_ptr<TextLikeControlCreator>
            multiLineCreator(controlCreator.CloneAsMultiLine());
        if ( !multiLineCreator )
            return;

        TestDialog dlg(*multiLineCreator, ProcessEnter_ButSkip);
        REQUIRE( dlg.ShowModal() == wxID_CLOSE );
        CHECK( dlg.GotEnter() );
    }

    SUBCASE("With wxTE_PROCESS_ENTER and wxTE_MULTILINE without skipping")
    {
        std::unique_ptr<TextLikeControlCreator>
            multiLineCreator(controlCreator.CloneAsMultiLine());
        if ( !multiLineCreator )
            return;

        TestDialog dlg(*multiLineCreator, ProcessEnter_WithoutSkipping);
        REQUIRE( dlg.ShowModal() == wxID_APPLY );
        CHECK( dlg.GotEnter() );
    }
}

#else // !wxUSE_UIACTIONSIMULATOR

void TestProcessEnter(const TextLikeControlCreator& WXUNUSED(controlCreator))
{
    WARN("Skipping wxTE_PROCESS_ENTER tests: wxUIActionSimulator not available");
}

#endif // wxUSE_UIACTIONSIMULATOR/!wxUSE_UIACTIONSIMULATOR
*/

#endif // _WX_TESTS_CONTROLS_TEXTENTRYTEST_H_
