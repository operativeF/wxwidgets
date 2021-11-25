///////////////////////////////////////////////////////////////////////////////
// Name:        tests/controls/markup.cpp
// Purpose:     wxMarkupParser and related classes unit tests
// Author:      Vadim Zeitlin
// Created:     2011-02-17
// Copyright:   (c) 2011 Vadim Zeitlin <vadim@wxwidgets.org>
///////////////////////////////////////////////////////////////////////////////

module;

#include "doctest.h"

#include "testprec.h"

#include "wx/private/markupparser.h"

export module WX.Test.Markup;

TEST_CASE("Markup tests")
{
    SUBCASE("RoundTrip")
    {
        // Define a wxMarkupParserOutput object which produces the same markup
        // string on output. This is, of course, perfectly useless, but allows us
        // to test that parsing works as expected.
        class RoundTripOutput : public wxMarkupParserOutput
        {
        public:
            RoundTripOutput() { }

            void Reset() { m_text.clear(); }

            const wxString& GetText() const { return m_text; }


            void OnText(const std::string& text) override { m_text += text; }

            void OnBoldStart() override { m_text += "<b>"; }
            void OnBoldEnd() override { m_text += "</b>"; }

            void OnItalicStart() override { m_text += "<i>"; }
            void OnItalicEnd() override { m_text += "</i>"; }

            void OnUnderlinedStart() override { m_text += "<u>"; }
            void OnUnderlinedEnd() override { m_text += "</u>"; }

            void OnStrikethroughStart() override { m_text += "<s>"; }
            void OnStrikethroughEnd() override { m_text += "</s>"; }

            void OnBigStart() override { m_text += "<big>"; }
            void OnBigEnd() override { m_text += "</big>"; }

            void OnSmallStart() override { m_text += "<small>"; }
            void OnSmallEnd() override { m_text += "</small>"; }

            void OnTeletypeStart() override { m_text += "<tt>"; }
            void OnTeletypeEnd() override { m_text += "</tt>"; }

            void OnSpanStart(const wxMarkupSpanAttributes& attrs) override
            {
                m_text << "<span";

                if ( !attrs.m_fgCol.empty() )
                    m_text << " foreground='" << attrs.m_fgCol << "'";

                if ( !attrs.m_bgCol.empty() )
                    m_text << " background='" << attrs.m_bgCol << "'";

                if ( !attrs.m_fontFace.empty() )
                    m_text << " face='" << attrs.m_fontFace << "'";

                wxString size;
                switch ( attrs.m_sizeKind )
                {
                    case wxMarkupSpanAttributes::Size_Unspecified:
                        break;

                    case wxMarkupSpanAttributes::Size_Relative:
                        size << (attrs.m_fontSize > 0 ? "larger" : "smaller");
                        break;

                    case wxMarkupSpanAttributes::Size_Symbolic:
                        {
                            CHECK( attrs.m_fontSize >= -3 );
                            CHECK( attrs.m_fontSize <= 3 );
                            static const char *cssSizes[] =
                            {
                                "xx-small", "x-small", "small",
                                "medium",
                                "large", "x-large", "xx-large",
                            };

                            size << cssSizes[attrs.m_fontSize + 3];
                        }
                        break;

                    case wxMarkupSpanAttributes::Size_PointParts:
                        size.Printf("%u", attrs.m_fontSize);
                        break;
                }

                if ( !size.empty() )
                    m_text << " size='" << size << '\'';

                // TODO: Handle the rest of attributes.

                m_text << ">";
            }

            void OnSpanEnd(const wxMarkupSpanAttributes& WXUNUSED(attrs)) override
            {
                m_text += "</span>";
            }

        private:
            wxString m_text;
        };


        RoundTripOutput output;
        wxMarkupParser parser(output);

        #define CHECK_PARSES_OK(text) \
            output.Reset(); \
            CHECK( parser.Parse(text) ); \
            CHECK_EQ( text, output.GetText() )

        #define CHECK_PARSES_AS(text, result) \
            output.Reset(); \
            CHECK( parser.Parse(text) ); \
            CHECK_EQ( result, output.GetText() )

        #define CHECK_DOESNT_PARSE(text) \
            CHECK( !parser.Parse(text) )

        CHECK_PARSES_OK( "" );
        CHECK_PARSES_OK( "foo" );
        CHECK_PARSES_OK( "foo<b>bar</b>" );
        CHECK_PARSES_OK( "1<big>2<small>3</small>4<big>5</big></big>6" );
        CHECK_PARSES_OK( "first <span foreground='red'>second</span> last" );
        CHECK_PARSES_OK( "first <span foreground='red' "
                                     "background='#ffffff'>second </span> last" );
        CHECK_PARSES_OK( "<span size='10240'>10pt</span>" );
        CHECK_PARSES_OK( "<span size='x-small'>much smaller</span>" );
        CHECK_PARSES_OK( "<span size='larger'>larger</span>" );
        CHECK_PARSES_OK
        (
            "<u>Please</u> notice: <i><b>any</b></i> <span foreground='grey'>"
            "<s><tt>bugs</tt></s></span> in this code are <span foreground='red' "
            "size='xx-large'>NOT</span> allowed."
        );

        CHECK_PARSES_OK( "foo&bar" );
        CHECK_PARSES_AS( "foo&amp;bar", "foo&bar" );
        CHECK_PARSES_AS( "&lt;O&apos;Reilly&gt;", "<O'Reilly>" );

        CHECK_DOESNT_PARSE( "<" );
        CHECK_DOESNT_PARSE( "<b" );
        CHECK_DOESNT_PARSE( "<b>" );
        CHECK_DOESNT_PARSE( "<b></i>" );
        CHECK_DOESNT_PARSE( "<b><i></b></i>" );
        CHECK_DOESNT_PARSE( "<foo></foo>" );

        #undef CHECK_PARSES_OK
        #undef CHECK_DOESNT_PARSE
    }

    SUBCASE("Quote")
    {
        CHECK_EQ( "", wxMarkupParser::Quote("") );
        CHECK_EQ( "foo", wxMarkupParser::Quote("foo") );
        CHECK_EQ( "&lt;foo&gt;", wxMarkupParser::Quote("<foo>") );
        CHECK_EQ( "B&amp;B", wxMarkupParser::Quote("B&B") );
        CHECK_EQ( "&quot;&quot;", wxMarkupParser::Quote("\"\"") );
    }

    SUBCASE("Strip")
    {
        #define CHECK_STRIP( text, stripped ) \
            CHECK_EQ( stripped, wxMarkupParser::Strip(text) )

        CHECK_STRIP( "", "" );
        CHECK_STRIP( "foo", "foo" );
        CHECK_STRIP( "&lt;foo&gt;", "<foo>" );
        CHECK_STRIP( "<b>Big</b> problem", "Big problem" );
        CHECK_STRIP( "<span foreground='red'>c</span>"
                     "<span background='green'>o</span>"
                     "<span background='blue'>l</span>"
                     "<span background='green'>o</span>"
                     "<span foreground='yellow'>u</span>"
                     "<span background='green'>r</span>",
                     "colour" );

        #undef CHECK_STRIP
    }
}
