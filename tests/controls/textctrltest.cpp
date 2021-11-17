///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/textctrltest.cpp
// Purpose:     wxTextCtrl unit test
// Author:      Vadim Zeitlin
// Created:     2007-09-25
// Copyright:   (c) 2007 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#if wxUSE_TEXTCTRL


#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/textctrl.h"

    #include <fmt/core.h>
#endif // WX_PRECOMP


#include "wx/uiaction.h"

#if wxUSE_CLIPBOARD
    #include "wx/clipbrd.h"
    #include "wx/dataobj.h"
#endif // wxUSE_CLIPBOARD

#ifdef __WXGTK__
    #include "wx/stopwatch.h"
#endif

#include "textentrytest.h"
#include "testableframe.h"
#include "asserthelper.h"

constexpr int TEXT_HEIGHT = 200;

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

static std::unique_ptr<wxTextCtrl> CreateText(long extraStyles)
{
    const int h = (extraStyles & wxTE_MULTILINE) ? TEXT_HEIGHT : -1;

    return std::make_unique<wxTextCtrl>(wxTheApp->GetTopWindow(), wxID_ANY, "",
                            wxDefaultPosition, wxSize(400, h), extraStyles);
}

static void DoPositionToCoordsTestWithStyle(unsigned int style)
{
    auto m_entry = CreateText(style | wxTE_MULTILINE);

    // Asking for invalid index should fail.
    CHECK_THROWS(m_entry->PositionToCoords(1));

    // Getting position shouldn't return wxDefaultPosition except if the method
    // is not implemented at all in the current port.
    const wxPoint pos0 = m_entry->PositionToCoords(0);
    if (pos0 == wxDefaultPosition)
    {
#if defined(__WXMSW__) || defined(__WXGTK20__)
        FAIL("PositionToCoords() unexpectedly failed.");
#endif
        return;
    }

    CHECK(pos0.x >= 0);
    CHECK(pos0.y >= 0);


    m_entry->SetValue("Hello");
    wxYield(); // Let GTK layout the control correctly.

    // Position of non-first character should be positive.
    const long posHello4 = m_entry->PositionToCoords(4).x;
    CHECK(posHello4 > 0);

    // Asking for position beyond the last character should succeed and return
    // reasonable result.
    CHECK(m_entry->PositionToCoords(5).x > posHello4);

    // But asking for the next position should fail.
    CHECK_THROWS(m_entry->PositionToCoords(6));

    // Test getting the coordinates of the last character when it is in the
    // beginning of a new line to exercise MSW code which has specific logic
    // for it.
    m_entry->AppendText("\n");
    const wxPoint posLast = m_entry->PositionToCoords(m_entry->GetLastPosition());
    CHECK_EQ(pos0.x, posLast.x);
    CHECK(posLast.y > 0);


    // Add enough contents to the control to make sure it has a scrollbar.
    m_entry->SetValue("First line" + wxString(50, '\n') + "Last line");
    m_entry->SetInsertionPoint(0);
    wxYield(); // Let GTK layout the control correctly.

    // This shouldn't change anything for the first position coordinates.
    CHECK_EQ(pos0, m_entry->PositionToCoords(0));

    // And the last one must be beyond the window boundary and so not be
    // visible -- but getting its coordinate should still work.
    CHECK
    (
        m_entry->PositionToCoords(m_entry->GetLastPosition()).y > TEXT_HEIGHT
    );


    // Now make it scroll to the end and check that the first position now has
    // negative offset as its above the visible part of the window while the
    // last position is in its bounds.
    m_entry->SetInsertionPointEnd();

    const int pos = m_entry->GetInsertionPoint();

    // wxGTK needs to yield here to update the text control.
#ifdef __WXGTK__
    wxStopWatch sw;
    while (m_entry->PositionToCoords(0).y == 0 ||
        m_entry->PositionToCoords(pos).y > TEXT_HEIGHT)
    {
        if (sw.Time() > 1000)
        {
            FAIL("Timed out waiting for wxTextCtrl update.");
            break;
        }

        wxYield();
    }
#endif // __WXGTK__

    wxPoint coords = m_entry->PositionToCoords(0);
    INFO("First position coords = " << coords);
    CHECK(coords.y < 0);

    coords = m_entry->PositionToCoords(pos);
    INFO("Position is " << pos << ", coords = " << coords);
    CHECK(coords.y <= TEXT_HEIGHT);
}

static void DoPositionToXYMultiLine(unsigned int style)
{
    auto m_entry = CreateText(style | wxTE_MULTILINE | wxTE_DONTWRAP);

#if defined(__WXMSW__)
    const bool isRichEdit = (style & (wxTE_RICH | wxTE_RICH2)) != 0;
#endif

    typedef struct { long x, y; } XYPos;
    bool ok;
    wxString text;

    // empty field
    m_entry->Clear();
    const long numChars_0 = 0;
    wxASSERT(numChars_0 == text.Length());
    XYPos coords_0[numChars_0 + 1] =
    { { 0, 0 } };

    CHECK_EQ(numChars_0, m_entry->GetLastPosition());
    for (long i = 0; i < (long)WXSIZEOF(coords_0); i++)
    {
        long x, y;
        ok = m_entry->PositionToXY(i, &x, &y);
        CHECK_EQ(true, ok);
        CHECK_EQ(coords_0[i].x, x);
        CHECK_EQ(coords_0[i].y, y);
    }
    ok = m_entry->PositionToXY(WXSIZEOF(coords_0), nullptr, nullptr);
    CHECK_EQ(false, ok);

    // one line
    text = wxS("1234");
    m_entry->SetValue(text);
    const long numChars_1 = 4;
    wxASSERT(numChars_1 == text.Length());
    XYPos coords_1[numChars_1 + 1] =
    { { 0, 0 }, { 1, 0 }, { 2, 0}, { 3, 0 }, { 4, 0 } };

    CHECK_EQ(numChars_1, m_entry->GetLastPosition());
    for (long i = 0; i < (long)WXSIZEOF(coords_1); i++)
    {
        long x, y;
        ok = m_entry->PositionToXY(i, &x, &y);
        CHECK_EQ(true, ok);
        CHECK_EQ(coords_1[i].x, x);
        CHECK_EQ(coords_1[i].y, y);
    }
    ok = m_entry->PositionToXY(WXSIZEOF(coords_1), nullptr, nullptr);
    CHECK_EQ(false, ok);

    // few lines
    text = wxS("123\nab\nX");
    m_entry->SetValue(text);

#if defined(__WXMSW__)
    // Take into account that every new line mark occupies
    // two characters, not one.
    const long numChars_msw_2 = 8 + 2;
    // Note: Two new line characters refer to the same X-Y position.
    XYPos coords_2_msw[numChars_msw_2 + 1] =
    { { 0, 0 },{ 1, 0 },{ 2, 0 },{ 3, 0 },{ 3, 0 },
      { 0, 1 },{ 1, 1 },{ 2, 1 },{ 2, 1 },
      { 0, 2 },{ 1, 2 } };
#endif // WXMSW

    const long numChars_2 = 8;
    wxASSERT(numChars_2 == text.Length());
    XYPos coords_2[numChars_2 + 1] =
    { { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 },
      { 0, 1 }, { 1, 1 }, { 2, 1 },
      { 0, 2 }, { 1, 2 } };

    const long& ref_numChars_2 =
#if defined(__WXMSW__)
        isRichEdit ? numChars_2 : numChars_msw_2;
#else
        numChars_2;
#endif

    XYPos* ref_coords_2 =
#if defined(__WXMSW__)
        isRichEdit ? coords_2 : coords_2_msw;
#else
        coords_2;
#endif

    CHECK_EQ(ref_numChars_2, m_entry->GetLastPosition());
    for (long i = 0; i < ref_numChars_2 + 1; i++)
    {
        long x, y;
        ok = m_entry->PositionToXY(i, &x, &y);
        CHECK_EQ(true, ok);
        CHECK_EQ(ref_coords_2[i].x, x);
        CHECK_EQ(ref_coords_2[i].y, y);
    }
    ok = m_entry->PositionToXY(ref_numChars_2 + 1, nullptr, nullptr);
    CHECK_EQ(false, ok);

    // only empty lines
    text = wxS("\n\n\n");
    m_entry->SetValue(text);

#if defined(__WXMSW__)
    // Take into account that every new line mark occupies
    // two characters, not one.
    const long numChars_msw_3 = 3 + 3;
    // Note: Two new line characters refer to the same X-Y position.
    XYPos coords_3_msw[numChars_msw_3 + 1] =
    { { 0, 0 },{ 0, 0 },
      { 0, 1 },{ 0, 1 },
      { 0, 2 },{ 0, 2 },
      { 0, 3 } };
#endif // WXMSW

    const long numChars_3 = 3;
    wxASSERT(numChars_3 == text.Length());
    XYPos coords_3[numChars_3 + 1] =
    { { 0, 0 },
      { 0, 1 },
      { 0, 2 },
      { 0, 3 } };

    const long& ref_numChars_3 =
#if defined(__WXMSW__)
        isRichEdit ? numChars_3 : numChars_msw_3;
#else
        numChars_3;
#endif

    XYPos* ref_coords_3 =
#if defined(__WXMSW__)
        isRichEdit ? coords_3 : coords_3_msw;
#else
        coords_3;
#endif

    CHECK_EQ(ref_numChars_3, m_entry->GetLastPosition());
    for (long i = 0; i < ref_numChars_3 + 1; i++)
    {
        long x, y;
        ok = m_entry->PositionToXY(i, &x, &y);
        CHECK_EQ(true, ok);
        CHECK_EQ(ref_coords_3[i].x, x);
        CHECK_EQ(ref_coords_3[i].y, y);
    }
    ok = m_entry->PositionToXY(ref_numChars_3 + 1, nullptr, nullptr);
    CHECK_EQ(false, ok);

    // mixed empty/non-empty lines
    text = wxS("123\na\n\nX\n\n");
    m_entry->SetValue(text);

#if defined(__WXMSW__)
    // Take into account that every new line mark occupies
    // two characters, not one.
    const long numChars_msw_4 = 10 + 5;
    // Note: Two new line characters refer to the same X-Y position.
    XYPos coords_4_msw[numChars_msw_4 + 1] =
    { { 0, 0 },{ 1, 0 },{ 2, 0 },{ 3, 0 },{ 3, 0 },
      { 0, 1 },{ 1, 1 },{ 1, 1 },
      { 0, 2 },{ 0, 2 },
      { 0, 3 },{ 1, 3 },{ 1, 3 },
      { 0, 4 },{ 0, 4 },
      { 0, 5 } };
#endif // WXMSW

    const long numChars_4 = 10;
    wxASSERT(numChars_4 == text.Length());
    XYPos coords_4[numChars_4 + 1] =
    { { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 },
      { 0, 1 }, { 1, 1 },
      { 0, 2 },
      { 0, 3 }, { 1, 3 },
      { 0, 4 },
      { 0, 5 } };

    const long& ref_numChars_4 =
#if defined(__WXMSW__)
        isRichEdit ? numChars_4 : numChars_msw_4;
#else
        numChars_4;
#endif

    XYPos* ref_coords_4 =
#if defined(__WXMSW__)
        isRichEdit ? coords_4 : coords_4_msw;
#else
        coords_4;
#endif

    CHECK_EQ(ref_numChars_4, m_entry->GetLastPosition());
    for (long i = 0; i < ref_numChars_4 + 1; i++)
    {
        long x, y;
        ok = m_entry->PositionToXY(i, &x, &y);
        CHECK_EQ(true, ok);
        CHECK_EQ(ref_coords_4[i].x, x);
        CHECK_EQ(ref_coords_4[i].y, y);
    }
    ok = m_entry->PositionToXY(ref_numChars_4 + 1, nullptr, nullptr);
    CHECK_EQ(false, ok);
}

static void DoXYToPositionMultiLine(unsigned int style)
{
    auto m_entry = CreateText(style | wxTE_MULTILINE | wxTE_DONTWRAP);

#if defined(__WXMSW__)
    const bool isRichEdit = (style & (wxTE_RICH | wxTE_RICH2)) != 0;
#endif

    wxString text;
    // empty field
    m_entry->Clear();
    const long maxLineLength_0 = 0 + 1;
    const long numLines_0 = 1;
    CHECK_EQ(numLines_0, m_entry->GetNumberOfLines());
    long pos_0[numLines_0 + 1][maxLineLength_0 + 1] =
    { {  0, -1 },
      { -1, -1 } };
    for (long y = 0; y < numLines_0 + 1; y++)
        for (long x = 0; x < maxLineLength_0 + 1; x++)
        {
            long p = m_entry->XYToPosition(x, y);
            INFO("x=" << x << ", y=" << y);
            CHECK_EQ(pos_0[y][x], p);
        }

    // one line
    text = wxS("1234");
    m_entry->SetValue(text);
    const long maxLineLength_1 = 4 + 1;
    const long numLines_1 = 1;
    CHECK_EQ(numLines_1, m_entry->GetNumberOfLines());
    long pos_1[numLines_1 + 1][maxLineLength_1 + 1] =
    { {  0,  1,  2,  3,  4, -1 },
      { -1, -1, -1, -1, -1, -1 } };
    for (long y = 0; y < numLines_1 + 1; y++)
        for (long x = 0; x < maxLineLength_1 + 1; x++)
        {
            long p = m_entry->XYToPosition(x, y);
            INFO("x=" << x << ", y=" << y);
            CHECK_EQ(pos_1[y][x], p);
        }

    // few lines
    text = wxS("123\nab\nX");
    m_entry->SetValue(text);
    const long maxLineLength_2 = 4;
    const long numLines_2 = 3;
    CHECK_EQ(numLines_2, m_entry->GetNumberOfLines());
#if defined(__WXMSW__)
    // Note: New lines are occupied by two characters.
    long pos_2_msw[numLines_2 + 1][maxLineLength_2 + 1] =
    { {  0,  1,  2,  3, -1 },   // New line occupies positions 3, 4
      {  5,  6,  7, -1, -1 },   // New line occupies positions 7, 8
      {  9, 10, -1, -1, -1 },
      { -1, -1, -1, -1, -1 } };
#endif // WXMSW
    long pos_2[numLines_2 + 1][maxLineLength_2 + 1] =
    { {  0,  1,  2,  3, -1 },
      {  4,  5,  6, -1, -1 },
      {  7,  8, -1, -1, -1 },
      { -1, -1, -1, -1, -1 } };

    long(&ref_pos_2)[numLines_2 + 1][maxLineLength_2 + 1] =
#if defined(__WXMSW__)
        isRichEdit ? pos_2 : pos_2_msw;
#else
        pos_2;
#endif

    for (long y = 0; y < numLines_2 + 1; y++)
        for (long x = 0; x < maxLineLength_2 + 1; x++)
        {
            long p = m_entry->XYToPosition(x, y);
            INFO("x=" << x << ", y=" << y);
            CHECK_EQ(ref_pos_2[y][x], p);
        }

    // only empty lines
    text = wxS("\n\n\n");
    m_entry->SetValue(text);
    const long maxLineLength_3 = 1;
    const long numLines_3 = 4;
    CHECK_EQ(numLines_3, m_entry->GetNumberOfLines());
#if defined(__WXMSW__)
    // Note: New lines are occupied by two characters.
    long pos_3_msw[numLines_3 + 1][maxLineLength_3 + 1] =
    { {  0, -1 },    // New line occupies positions 0, 1
      {  2, -1 },    // New line occupies positions 2, 3
      {  4, -1 },    // New line occupies positions 4, 5
      {  6, -1 },
      { -1, -1 } };
#endif // WXMSW
    long pos_3[numLines_3 + 1][maxLineLength_3 + 1] =
    { {  0, -1 },
      {  1, -1 },
      {  2, -1 },
      {  3, -1 },
      { -1, -1 } };

    long(&ref_pos_3)[numLines_3 + 1][maxLineLength_3 + 1] =
#if defined(__WXMSW__)
        isRichEdit ? pos_3 : pos_3_msw;
#else
        pos_3;
#endif

    for (long y = 0; y < numLines_3 + 1; y++)
        for (long x = 0; x < maxLineLength_3 + 1; x++)
        {
            long p = m_entry->XYToPosition(x, y);
            INFO("x=" << x << ", y=" << y);
            CHECK_EQ(ref_pos_3[y][x], p);
        }

    // mixed empty/non-empty lines
    text = wxS("123\na\n\nX\n\n");
    m_entry->SetValue(text);
    const long maxLineLength_4 = 4;
    const long numLines_4 = 6;
    CHECK_EQ(numLines_4, m_entry->GetNumberOfLines());
#if defined(__WXMSW__)
    // Note: New lines are occupied by two characters.
    long pos_4_msw[numLines_4 + 1][maxLineLength_4 + 1] =
    { {  0,  1,  2,  3, -1 },    // New line occupies positions 3, 4
      {  5,  6, -1, -1, -1 },    // New line occupies positions 6, 7
      {  8, -1, -1, -1, -1 },    // New line occupies positions 8, 9
      { 10, 11, -1, -1, -1 },    // New line occupies positions 11, 12
      { 13, -1, -1, -1, -1 },    // New line occupies positions 13, 14
      { 15, -1, -1, -1, -1 },
      { -1, -1, -1, -1, -1 } };
#endif // WXMSW
    long pos_4[numLines_4 + 1][maxLineLength_4 + 1] =
    { {  0,  1,  2,  3, -1 },
      {  4,  5, -1, -1, -1 },
      {  6, -1, -1, -1, -1 },
      {  7,  8, -1, -1, -1 },
      {  9, -1, -1, -1, -1 },
      { 10, -1, -1, -1, -1 },
      { -1, -1, -1, -1, -1 } };

    long(&ref_pos_4)[numLines_4 + 1][maxLineLength_4 + 1] =
#if defined(__WXMSW__)
        isRichEdit ? pos_4 : pos_4_msw;
#else
        pos_4;
#endif

    for (long y = 0; y < numLines_4 + 1; y++)
        for (long x = 0; x < maxLineLength_4 + 1; x++)
        {
            long p = m_entry->XYToPosition(x, y);
            INFO("x=" << x << ", y=" << y);
            CHECK_EQ(ref_pos_4[y][x], p);
        }
}

constexpr int lenPattern = 100;

// Pattern for the line.
consteval std::array<unsigned char, lenPattern> getLinePattern()
{
    std::array<unsigned char, lenPattern> lpArray{};

    for (int i = 0; i < lpArray.size() - 1; i++)
    {
        lpArray[i] = unsigned char('0' + i % 10);
    }

    lpArray[lpArray.size() - 1] = unsigned char('\0');

    return lpArray;
}

constexpr auto linePattern = getLinePattern();

// ----------------------------------------------------------------------------
// tests themselves
// ----------------------------------------------------------------------------

using TextCtrlTest = TextEntryTest<wxTextCtrl>;

TEST_CASE_FIXTURE(TextCtrlTest, "Text control test")
{
    SUBCASE("Single-line tests")
    {
        m_entry = CreateText(0);

        wxTEXT_ENTRY_TESTS();

        SUBCASE("Read-only")
        {
#if wxUSE_UIACTIONSIMULATOR
            // we need a read only control for this test so recreate it
            m_entry = CreateText(wxTE_READONLY);

            EventCounter updated(m_entry.get(), wxEVT_TEXT);

            m_entry->SetFocus();

            wxUIActionSimulator sim;
            sim.Text("abcdef");
            wxYield();

            CHECK_EQ("", m_entry->GetValue());
            CHECK_EQ(0, updated.GetCount());

            // SetEditable() is supposed to override wxTE_READONLY
            m_entry->SetEditable(true);

#if defined(__WXOSX__) || defined(__WXUNIVERSAL__)
            // a ready only text field might not have been focusable at all
            m_entry->SetFocus();
#endif

            sim.Text("abcdef");
            wxYield();

            CHECK_EQ("abcdef", m_entry->GetValue());
            CHECK_EQ(6, updated.GetCount());
#endif
        }

        SUBCASE("Max length")
        {
#if wxUSE_UIACTIONSIMULATOR
            EventCounter updated(m_entry.get(), wxEVT_TEXT);
            EventCounter maxlen(m_entry.get(), wxEVT_TEXT_MAXLEN);

            m_entry->SetFocus();
            wxYield();
            m_entry->SetMaxLength(10);

            wxUIActionSimulator sim;
            sim.Text("abcdef");
            wxYield();

            CHECK_EQ(0, maxlen.GetCount());

            sim.Text("ghij");
            wxYield();

            CHECK_EQ(0, maxlen.GetCount());
            CHECK_EQ(10, updated.GetCount());

            maxlen.Clear();
            updated.Clear();

            sim.Text("k");
            wxYield();

            CHECK_EQ(1, maxlen.GetCount());
            CHECK_EQ(0, updated.GetCount());

            maxlen.Clear();
            updated.Clear();

            m_entry->SetMaxLength(0);

            sim.Text("k");
            wxYield();

            CHECK_EQ(0, maxlen.GetCount());
            CHECK_EQ(1, updated.GetCount());
#endif
        }

        SUBCASE("StreamInput")
        {
#ifndef __WXOSX__
            {
                // Ensure we use decimal point and not a comma.
                LocaleSetter setCLocale("C");

                *m_entry << "stringinput"
                    << 10
                    << 1000L
                    << 3.14f
                    << 2.71
                    << 'a'
                    << L'b';
            }

            CHECK_EQ("stringinput1010003.142.71ab", m_entry->GetValue());

            m_entry->SetValue("");

#if wxHAS_TEXT_WINDOW_STREAM

            std::ostream stream(m_entry.get());

            // We don't test a wide character as this is not a wide stream
            stream << "stringinput"
                << 10
                << 1000L
                << 3.14f
                << 2.71
                << 'a';

            stream.flush();

            CHECK_EQ("stringinput1010003.142.71a", m_entry->GetValue());

#endif // wxHAS_TEXT_WINDOW_STREAM
#endif // !__WXOSX__
        }

        SUBCASE("Redirector")
        {
#if wxHAS_TEXT_WINDOW_STREAM

            wxStreamToTextRedirector redirect(m_entry.get());

            std::cout << "stringinput"
                << 10
                << 1000L
                << 3.14f
                << 2.71
                << 'a';

            CHECK_EQ("stringinput1010003.142.71a", m_entry->GetValue());

#endif
        }

        SUBCASE("HitTestSingleLine")
        {
#ifdef __WXQT__
            WARN("Does not work under WxQt");
#else
            m_entry->ChangeValue("Hit me");

            // We don't know the size of the text borders, so we can't really do any
            // exact tests, just try to verify that the results are roughly as
            // expected.
            const wxSize sizeChar = m_entry->GetTextExtent("X");
            const int yMid = sizeChar.y / 2;

            long pos = -1;

#ifdef __WXGTK__
            wxYield();
#endif

            // Hitting a point near the left side of the control should find one of the
            // first few characters under it.
            SUBCASE("Normal")
            {
                REQUIRE(m_entry->HitTest(wxPoint(2 * sizeChar.x, yMid), &pos) == wxTextCtrlHitTestResult::OnText);
                CHECK(pos >= 0);
                CHECK(pos < 3);
            }

            // Hitting a point well beyond the end of the text shouldn't find any valid
            // character.
            SUBCASE("Beyond")
            {
                REQUIRE(m_entry->HitTest(wxPoint(20 * sizeChar.x, yMid), &pos) == wxTextCtrlHitTestResult::Beyond);
                CHECK(pos == m_entry->GetLastPosition());
            }

            // Making the control scroll, by ensuring that its contents is too long to
            // show inside its window, should change the hit test result for the same
            // position as used above.
            SUBCASE("Scrolled")
            {
                m_entry->ChangeValue(wxString(200, 'X'));
                m_entry->SetInsertionPointEnd();

#ifdef __WXGTK__
                // wxGTK must be given an opportunity to lay the text out.
                for (wxStopWatch sw; sw.Time() < 50; )
                    wxYield();
#endif

                REQUIRE(m_entry->HitTest(wxPoint(2 * sizeChar.x, yMid), &pos) == wxTextCtrlHitTestResult::OnText);
                CHECK(pos > 3);

                // Using negative coordinates works even under Xvfb, so test at least
                // for this -- however this only works in wxGTK, not wxMSW.
#ifdef __WXGTK__
                REQUIRE(m_entry->HitTest(wxPoint(-2 * sizeChar.x, yMid), &pos) == wxTextCtrlHitTestResult::OnText);
                CHECK(pos > 3);
#endif // __WXGTK__
            }
#endif
        }

        SUBCASE("PositionToXYSingleLine")
        {
            m_entry = CreateText(wxTE_DONTWRAP);

            bool ok;
            wxString text;
            // empty field
            m_entry->Clear();
            const long numChars_0 = 0;
            CHECK_EQ(numChars_0, m_entry->GetLastPosition());
            for (long i = 0; i <= numChars_0; i++)
            {
                long x0, y0;
                ok = m_entry->PositionToXY(i, &x0, &y0);
                CHECK_EQ(true, ok);
                CHECK_EQ(i, x0);
                CHECK_EQ(0, y0);
            }
            ok = m_entry->PositionToXY(numChars_0 + 1, nullptr, nullptr);
            CHECK_EQ(false, ok);

            // pure one line
            text = wxS("1234");
            m_entry->SetValue(text);
            const std::size_t numChars_1 = text.Length();
            CHECK_EQ(numChars_1, m_entry->GetLastPosition());
            for (long i = 0; i <= numChars_1; i++)
            {
                long x1, y1;
                ok = m_entry->PositionToXY(i, &x1, &y1);
                CHECK_EQ(true, ok);
                CHECK_EQ(i, x1);
                CHECK_EQ(0, y1);
            }
            ok = m_entry->PositionToXY(numChars_1 + 1, nullptr, nullptr);
            CHECK_EQ(false, ok);

            // with new line characters
            text = wxS("123\nab\nX");
            m_entry->SetValue(text);
            const std::size_t numChars_2 = text.Length();
            CHECK_EQ(numChars_2, m_entry->GetLastPosition());
            for (long i = 0; i <= numChars_2; i++)
            {
                long x2, y2;
                ok = m_entry->PositionToXY(i, &x2, &y2);
                CHECK_EQ(true, ok);
                CHECK_EQ(i, x2);
                CHECK_EQ(0, y2);
            }
            ok = m_entry->PositionToXY(numChars_2 + 1, nullptr, nullptr);
            CHECK_EQ(false, ok);
        }

        SUBCASE("XYToPositionSingleLine")
        {
            m_entry = CreateText(wxTE_DONTWRAP);

            wxString text;
            // empty field
            m_entry->Clear();
            CHECK_EQ(1, m_entry->GetNumberOfLines());
            for (long x = 0; x < m_entry->GetLastPosition() + 2; x++)
            {
                long p0 = m_entry->XYToPosition(x, 0);
                if (x <= m_entry->GetLastPosition())
                    CHECK_EQ(x, p0);
                else
                    CHECK_EQ(-1, p0);

                p0 = m_entry->XYToPosition(x, 1);
                CHECK_EQ(-1, p0);
            }

            // pure one line
            text = wxS("1234");
            m_entry->SetValue(text);
            CHECK_EQ(1, m_entry->GetNumberOfLines());
            for (long x = 0; x < m_entry->GetLastPosition() + 2; x++)
            {
                long p1 = m_entry->XYToPosition(x, 0);
                if (x <= m_entry->GetLastPosition())
                    CHECK_EQ(x, p1);
                else
                    CHECK_EQ(-1, p1);

                p1 = m_entry->XYToPosition(x, 1);
                CHECK_EQ(-1, p1);
            }

            // with new line characters
            text = wxS("123\nab\nX");
            m_entry->SetValue(text);
            CHECK_EQ(1, m_entry->GetNumberOfLines());
            for (long x = 0; x < m_entry->GetLastPosition() + 2; x++)
            {
                long p2 = m_entry->XYToPosition(x, 0);
                if (x <= m_entry->GetLastPosition())
                    CHECK_EQ(x, p2);
                else
                    CHECK_EQ(-1, p2);

                p2 = m_entry->XYToPosition(x, 1);
                CHECK_EQ(-1, p2);
            }
        }

        wxTEXT_ENTRY_TESTS();
    }

    SUBCASE("Multiline tests")
    {
        m_entry = CreateText(wxTE_MULTILINE);

        SUBCASE("MultiLine replace")
        {
            m_entry->SetValue("Hello replace\n"
                            "0123456789012");
            m_entry->SetInsertionPoint(0);

            m_entry->Replace(6, 13, "changed");

            CHECK_EQ("Hello changed\n"
                                 "0123456789012",
                                 m_entry->GetValue());
            CHECK_EQ(13, m_entry->GetInsertionPoint());

            m_entry->Replace(13, -1, "");
            CHECK_EQ("Hello changed", m_entry->GetValue());
            CHECK_EQ(13, m_entry->GetInsertionPoint());
        }

        SUBCASE("StreamInput")
        {
#ifndef __WXOSX__
            {
                // Ensure we use decimal point and not a comma.
                LocaleSetter setCLocale("C");

                *m_entry << "stringinput"
                    << 10
                    << 1000L
                    << 3.14f
                    << 2.71
                    << 'a'
                    << L'b';
            }

            CHECK_EQ("stringinput1010003.142.71ab", m_entry->GetValue());

            m_entry->SetValue("");

#if wxHAS_TEXT_WINDOW_STREAM

            std::ostream stream(m_entry.get());

            // We don't test a wide character as this is not a wide stream
            stream << "stringinput"
                << 10
                << 1000L
                << 3.14f
                << 2.71
                << 'a';

            stream.flush();

            CHECK_EQ("stringinput1010003.142.71a", m_entry->GetValue());

#endif // wxHAS_TEXT_WINDOW_STREAM
#endif // !__WXOSX__
        }

        SUBCASE("Redirector")
        {
#if wxHAS_TEXT_WINDOW_STREAM

            wxStreamToTextRedirector redirect(m_entry.get());

            std::cout << "stringinput"
                << 10
                << 1000L
                << 3.14f
                << 2.71
                << 'a';

            CHECK_EQ("stringinput1010003.142.71a", m_entry->GetValue());

#endif
        }

#if !WX_WINDOWS
        SUBCASE("Url")
        {
            // FIXME: There's probably a good reason this fails on MSVC. Find it.
            if (wxGetEnv("APPVEYOR", nullptr))
                return;

            m_entry = CreateText(wxTE_RICH | wxTE_AUTO_URL);

            EventCounter url(m_entry.get(), wxEVT_TEXT_URL);

            m_entry->AppendText("http://www.wxwidgets.org");

            wxUIActionSimulator sim;
            // FIXME: Make this dependant on actual realized screen, not hardcoded coords.
            sim.MouseMove(m_entry->ClientToScreen(wxPoint(10, 10)));
            sim.MouseClick();

            wxYield();

            CHECK_EQ(1, url.GetCount());
        }
#endif

        SUBCASE("Style")
        {
#if !defined(__WXOSX__) && !defined(__WXQT__)
            // We need wxTE_RICH under windows for style support
            m_entry = CreateText(wxTE_MULTILINE | wxTE_RICH);

            // Red text on a white background
            m_entry->SetDefaultStyle(wxTextAttr(*wxRED, *wxWHITE));

            CHECK_EQ(m_entry->GetDefaultStyle().GetTextColour(), *wxRED);
            CHECK_EQ(m_entry->GetDefaultStyle().GetBackgroundColour(),
                *wxWHITE);

            m_entry->AppendText("red on white ");

            // Red text on a grey background
            m_entry->SetDefaultStyle(wxTextAttr(wxNullColour, *wxLIGHT_GREY));

            CHECK_EQ(m_entry->GetDefaultStyle().GetTextColour(), *wxRED);
            CHECK_EQ(m_entry->GetDefaultStyle().GetBackgroundColour(),
                *wxLIGHT_GREY);

            m_entry->AppendText("red on grey ");

            // Blue text on a grey background
            m_entry->SetDefaultStyle(wxTextAttr(*wxBLUE));


            CHECK_EQ(m_entry->GetDefaultStyle().GetTextColour(), *wxBLUE);
            CHECK_EQ(m_entry->GetDefaultStyle().GetBackgroundColour(),
                *wxLIGHT_GREY);

            m_entry->AppendText("blue on grey");

            // Get getting the style at a specific location
            wxTextAttr style;

            // We have to check that styles are supported
            if (!m_entry->GetStyle(3, style))
            {
                WARN("Retrieving text style not supported, skipping test.");
                return;
            }

            CHECK(style.GetTextColour() == *wxRED);
            CHECK(style.GetBackgroundColour() == *wxWHITE);

            // And then setting the style
            REQUIRE(m_entry->SetStyle(15, 18, style));

            REQUIRE(m_entry->GetStyle(17, style));
            CHECK(style.GetTextColour() == *wxRED);
            CHECK(style.GetBackgroundColour() == *wxWHITE);
#else
            WARN("Does not work under WxQt or OSX");
#endif
    }

        SUBCASE("FontStyle")
        {
            // We need wxTE_RICH under MSW and wxTE_MULTILINE under GTK for style
            // support so recreate the control with these styles.
            m_entry = CreateText(wxTE_RICH);

            // Check that we get back the same font from GetStyle() after setting it
            // with SetDefaultStyle().
            wxFont fontIn(14,
                wxFontFamily::Default,
                wxFontStyle::Normal,
                wxFONTWEIGHT_NORMAL);
            wxTextAttr attrIn;
            attrIn.SetFont(fontIn);
            if (!m_entry->SetDefaultStyle(attrIn))
            {
                // Skip the test if the styles are not supported.
                return;
            }

            m_entry->AppendText("Default font size 14");

            wxTextAttr attrOut;
            m_entry->GetStyle(5, attrOut);

            CHECK(attrOut.HasFont());

            wxFont fontOut = attrOut.GetFont();
#ifdef __WXMSW__
            // Under MSW we get back an encoding in the font even though we hadn't
            // specified it originally. It's not really a problem but we need this hack
            // to prevent the assert below from failing because of it.
            fontOut.SetEncoding(fontIn.GetEncoding());
#endif
            CHECK_EQ(fontIn, fontOut);


            // Also check the same for SetStyle().
            fontIn.SetPointSize(10);
            fontIn.SetWeight(wxFONTWEIGHT_BOLD);
            attrIn.SetFont(fontIn);
            m_entry->SetStyle(0, 6, attrIn);

            m_entry->GetStyle(4, attrOut);
            CHECK(attrOut.HasFont());

            fontOut = attrOut.GetFont();
#ifdef __WXMSW__
            fontOut.SetEncoding(fontIn.GetEncoding());
#endif
            CHECK_EQ(fontIn, fontOut);
        }

        SUBCASE("Lines")
        {
            m_entry->SetValue("line1\nline2\nlong long line 3");
            m_entry->Refresh();
            m_entry->Update();

            CHECK_EQ(3, m_entry->GetNumberOfLines());
            CHECK_EQ(5, m_entry->GetLineLength(0));
            CHECK_EQ("line2", m_entry->GetLineText(1));
            CHECK_EQ(16, m_entry->GetLineLength(2));

            m_entry->AppendText("\n\nMore text on line 5");

            CHECK_EQ(5, m_entry->GetNumberOfLines());
            CHECK_EQ(0, m_entry->GetLineLength(3));
            CHECK_EQ("", m_entry->GetLineText(3));

            // Verify that wrapped lines count as (at least) lines (but it can be more
            // if it's wrapped more than once).
            //
            // This currently doesn't work neither in wxGTK, wxUniv, or wxOSX/Cocoa, see
            // #12366, where GetNumberOfLines() always returns the number of logical,
            // not physical, lines.
            m_entry->AppendText('\n' + wxString(50, '1') + ' ' + wxString(50, '2'));
#if defined(__WXGTK__) || defined(__WXOSX_COCOA__) || defined(__WXUNIVERSAL__) || defined(__WXQT__)
            CHECK_EQ(6, m_entry->GetNumberOfLines());
#else
            CHECK(m_entry->GetNumberOfLines() > 6);
#endif
        }

#if wxUSE_LOG
        SUBCASE("LogTextCtrl")
        {
            CHECK(m_entry->IsEmpty());

            wxLogTextCtrl* logtext = new wxLogTextCtrl(m_entry.get());

            wxLog* old = wxLog::SetActiveTarget(logtext);

            logtext->LogText("text");

            delete wxLog::SetActiveTarget(old);

            CHECK(!m_entry->IsEmpty());
        }
#endif // wxUSE_LOG

        SUBCASE("LongText")
        {
            // This test is only possible under MSW as in the other ports
            // SetMaxLength() can't be used with multi line text controls.
#ifdef __WXMSW__
            m_entry = CreateText(wxTE_MULTILINE | wxTE_DONTWRAP);

            const int numLines = 1000;

            // Fill the control.
            m_entry->SetMaxLength(15000);
            for (int i = 0; i < numLines; i++)
            {
                m_entry->AppendText(fmt::format("[{:3d}] {}\n", i, linePattern.data()));
            }

            // Check the content.
            for (int i = 0; i < numLines; i++)
            {
                CHECK_EQ(m_entry->GetLineText(i), fmt::format("[{:3d}] {}", i, linePattern.data()));
        }
#endif // __WXMSW__
    }

        SUBCASE("PositionToCoords")
        {
            DoPositionToCoordsTestWithStyle(0);
        }

        SUBCASE("PositionToCoordsRich")
        {
            DoPositionToCoordsTestWithStyle(wxTE_RICH);
        }

        SUBCASE("PositionToCoordsRich2")
        {
            DoPositionToCoordsTestWithStyle(wxTE_RICH2);
        }

        SUBCASE("PositionToXYMultiLine")
        {
            DoPositionToXYMultiLine(0);
        }

#if wxUSE_RICHEDIT
        SUBCASE("PositionToXYMultiLineRich")
        {
            DoPositionToXYMultiLine(wxTE_RICH);
        }

        SUBCASE("PositionToXYMultiLineRich2")
        {
            DoPositionToXYMultiLine(wxTE_RICH2);
        }

        SUBCASE("XYToPositionMultiLineRich")
        {
            DoXYToPositionMultiLine(wxTE_RICH);
        }

        SUBCASE("XYToPositionMultiLineRich2")
        {
            DoXYToPositionMultiLine(wxTE_RICH2);
        }
#endif // wxUSE_RICHEDIT

        SUBCASE("XYToPositionMultiLine")
        {
            DoXYToPositionMultiLine(0);
        }

        wxTEXT_ENTRY_TESTS();
    }
}

#if 0
void TextCtrlTestCase::ProcessEnter()
{
#if wxUSE_UIACTIONSIMULATOR
    wxTestableFrame* frame = wxStaticCast(wxTheApp->GetTopWindow(),
        wxTestableFrame);

    EventCounter count(m_entry, wxEVT_TEXT_ENTER);

    m_entry->SetFocus();

    wxUIActionSimulator sim;
    sim.Char(WXK_RETURN);
    wxYield();

    CHECK_EQ(0, frame->GetEventCount(wxEVT_TEXT_ENTER));

    // we need a text control with wxTE_PROCESS_ENTER for this test
    delete m_entry;
    CreateText(wxTE_PROCESS_ENTER);

    m_entry->SetFocus();

    sim.Char(WXK_RETURN);
    wxYield();

    CHECK_EQ(1, frame->GetEventCount(wxEVT_TEXT_ENTER));
#endif
}
#endif

TEST_CASE("wxTextCtrl::ProcessEnter")
{
    class TextCtrlCreator : public TextLikeControlCreator
    {
    public:
        explicit TextCtrlCreator(int styleToAdd = 0)
            : m_styleToAdd(styleToAdd)
        {
        }

        wxControl* Create(wxWindow* parent, int style) const override
        {
            return new wxTextCtrl(parent, wxID_ANY, wxString(),
                                  wxDefaultPosition, wxDefaultSize,
                                  style | m_styleToAdd);
        }

        TextLikeControlCreator* CloneAsMultiLine() const override
        {
            return new TextCtrlCreator(wxTE_MULTILINE);
        }

    private:
        int m_styleToAdd;
    };

    TestProcessEnter(TextCtrlCreator());
}

TEST_CASE("wxTextCtrl::GetBestSize")
{
    struct GetBestSizeFor
    {
        wxSize operator()(const wxString& text) const
        {
            std::unique_ptr<wxTextCtrl>
                t(new wxTextCtrl(wxTheApp->GetTopWindow(), wxID_ANY, text,
                                 wxDefaultPosition, wxDefaultSize,
                                 wxTE_MULTILINE));
            return t->GetBestSize();
        }
    } getBestSizeFor;

    wxString s;
    const wxSize sizeEmpty = getBestSizeFor(s);

    // Empty control should have some reasonable vertical size.
    CHECK( sizeEmpty.y > 0 );

    s += "1\n2\n3\n4\n5\n";
    const wxSize sizeMedium = getBestSizeFor(s);

    // Control with a few lines of text in it should be taller.
    CHECK( sizeMedium.y > sizeEmpty.y );

    s += "6\n7\n8\n9\n10\n";
    const wxSize sizeLong = getBestSizeFor(s);

    // And a control with many lines in it should be even more so.
    CHECK( sizeLong.y > sizeMedium.y );

    s += s;
    s += s;
    s += s;
    const wxSize sizeVeryLong = getBestSizeFor(s);

    // However there is a cutoff at 10 lines currently, so anything longer than
    // that should still have the same best size.
    CHECK( sizeVeryLong.y == sizeLong.y );
}

#if wxUSE_CLIPBOARD

static void LongPasteTest(unsigned int style)
{
    std::unique_ptr<wxTextCtrl>
        text(new wxTextCtrl(wxTheApp->GetTopWindow(), wxID_ANY, std::string{},
            wxDefaultPosition, wxDefaultSize, style));

    // This could actually be much higher, but it makes the test proportionally
    // slower, so use a relatively small (but still requiring more space than
    // the default maximum length under MSW) number here.
    const int NUM_LINES = 10000;

    // Build a longish string.
    std::string s;
    s.reserve(NUM_LINES * 5 + 10);
    for (int n = 0; n != NUM_LINES; ++n)
    {
        s += fmt::format("{:04d}\n", n);
    }

    s += "THE END";

    wxClipboardLocker lock;

    wxTheClipboard->AddData(new wxTextDataObject(s));

    wxUIActionSimulator sim;
    sim.MouseMove(wxDefaultPosition);

    wxYield();

    text->ChangeValue("THE BEGINNING\n");
    text->SetInsertionPointEnd();
    text->Paste();

    const int numLines = text->GetNumberOfLines();

    CHECK(numLines == NUM_LINES + 2);
    CHECK(text->GetLineText(numLines - 1) == "THE END");
}

TEST_CASE("wxTextCtrl::LongPaste: Plain")
{
    LongPasteTest(wxTE_MULTILINE);
}

// wxTE_RICH[2] style only makes any different under MSW, so don't bother
// testing it under the other platforms.

#ifdef __WXMSW__

TEST_CASE("wxTextCtrl::LongPaste: Rich")
{
    LongPasteTest(wxTE_MULTILINE | wxTE_RICH);
}

TEST_CASE("Rich v2")
{
    LongPasteTest(wxTE_MULTILINE | wxTE_RICH2);
}
#endif // __WXMSW__

#endif // wxUSE_CLIPBOARD

TEST_CASE("wxTextCtrl::EventsOnCreate")
{
    wxWindow* const parent = wxTheApp->GetTopWindow();

    EventCounter updated(parent, wxEVT_TEXT);

    std::unique_ptr<wxTextCtrl> text(new wxTextCtrl(parent, wxID_ANY, "Hello"));

    // Creating the control shouldn't result in any wxEVT_TEXT events.
    CHECK( updated.GetCount() == 0 );

    // Check that modifying using SetValue() it does generate the event, just
    // to verify that this test works (there are more detailed tests for this
    // in TextEntryTestCase::TextChangeEvents()).
    text->SetValue("Bye");
    CHECK( updated.GetCount() == 1 );
}

TEST_CASE("wxTextCtrl::InitialCanUndo")
{
    wxWindow* const parent = wxTheApp->GetTopWindow();

    const long styles[] = { 0, wxTE_RICH, wxTE_RICH2 };

    for ( size_t n = 0; n < WXSIZEOF(styles); n++ )
    {
        const unsigned int style = styles[n];

#ifdef __MINGW32_TOOLCHAIN__
        if ( style == wxTE_RICH2 )
        {
            // We can't call ITextDocument::Undo() in wxMSW code when using
            // MinGW32, so this test would always fail with it.
            WARN("Skipping test known to fail with MinGW-32.");
        }
        continue;
#endif // __MINGW32_TOOLCHAIN__

        INFO("wxTextCtrl with style " << style);

        std::unique_ptr<wxTextCtrl> text(new wxTextCtrl(parent, wxID_ANY, "",
                                                    wxDefaultPosition,
                                                    wxDefaultSize,
                                                    style));

        CHECK( !text->CanUndo() );
    }
}

#endif //wxUSE_TEXTCTRL
