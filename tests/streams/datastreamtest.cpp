///////////////////////////////////////////////////////////////////////////////
// Name:        tests/streams/datastreamtest.cpp
// Purpose:     wxDataXXXStream Unit Test
// Author:      Ryan Norton
// Created:     2004-08-14
// Copyright:   (c) 2004 Ryan Norton
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#include "wx/datstrm.h"

#include "testfile.h"

import WX.Cmn.WFStream;

import <vector>;

static bool ms_useBigEndianFormat = false;

static double TestFloatRW(double fValue)
{
    TempFile f("mytext.dat");

    {
        wxFileOutputStream pFileOutput( f.GetName() );
        wxDataOutputStream pDataOutput( pFileOutput );
        if ( ms_useBigEndianFormat )
            pDataOutput.BigEndianOrdered(true);

        pDataOutput << fValue;
    }

    wxFileInputStream pFileInput( f.GetName() );
    wxDataInputStream pDataInput( pFileInput );
    if ( ms_useBigEndianFormat )
        pDataInput.BigEndianOrdered(true);

    double fInFloat;

    pDataInput >> fInFloat;

    return fInFloat;
}

template <class T>
class TestMultiRW {
public:
    typedef std::vector<T> ValueArray;
    typedef void (wxDataOutputStream::*FnWriter)(const T *buffer, size_t size);
    typedef void (wxDataInputStream::*FnReader)(T *buffer, size_t size);

private:
    bool m_ok;

private:
    void ProcessData(const T *Values,
                     typename ValueArray::size_type Size,
                     FnWriter pfnWriter,
                     FnReader pfnReader)
    {
        ValueArray InValues(Size);

        TempFile f("mytext.dat");

        {
            wxFileOutputStream FileOutput( f.GetName() );
            wxDataOutputStream DataOutput( FileOutput );

            (DataOutput.*pfnWriter)(Values, Size);
        }

        {
            wxFileInputStream FileInput( f.GetName() );
            wxDataInputStream DataInput( FileInput );

            (DataInput.*pfnReader)(&*InValues.begin(), InValues.size());
        }

        m_ok = true;
        for (typename ValueArray::size_type idx=0; idx!=Size; ++idx) {
            if (InValues[idx]!=Values[idx]) {
                m_ok = false;
                break;
            }
        }
    }


public:
    TestMultiRW(const T *Values,
                size_t Size,
                FnWriter pfnWriter,
                FnReader pfnReader)
    {
        ProcessData(Values, (typename ValueArray::size_type) Size, pfnWriter, pfnReader);
    }
    TestMultiRW(const ValueArray &Values,
                FnWriter pfnWriter,
                FnReader pfnReader)
    {
        ProcessData(&*Values.begin(), Values.size(), pfnWriter, pfnReader);
    }

    bool IsOk() const
    {
        return m_ok;
    }
};

template <class T>
static
T TestRW(const T &Value)
{
    T InValue;

    TempFile f("mytext.dat");

    {
        wxFileOutputStream FileOutput( f.GetName() );
        wxDataOutputStream DataOutput( FileOutput );

        DataOutput << Value;
    }

    {
        wxFileInputStream FileInput( f.GetName() );
        wxDataInputStream DataInput( FileInput );

        DataInput >> InValue;
    }

    return InValue;
}

TEST_CASE("FloatRW")
{
    CHECK( TestFloatRW(5.5) == 5.5 );
    CHECK( TestFloatRW(5) == 5 );
    CHECK( TestFloatRW(5.55) == 5.55 );
    CHECK( TestFloatRW(55555.555555) == 55555.555555 );
}

TEST_CASE("DoubleRW")
{
    CHECK( TestFloatRW(2132131.1232132) == 2132131.1232132 );
    CHECK( TestFloatRW(21321343431.1232143432) == 21321343431.1232143432 );
}

TEST_CASE("StringRW")
{
    wxString s(wxT("Test1"));
    CHECK_EQ( TestRW(s), s );

    s.append(2, wxT('\0'));
    s.append(wxT("Test2"));
    CHECK_EQ( TestRW(s), s );

    s = wxString::FromUTF8("\xc3\xbc"); // U+00FC LATIN SMALL LETTER U WITH DIAERESIS
    CHECK_EQ( TestRW(s), s );
}

#if wxUSE_LONGLONG
TEST_CASE("LongLongRW")
{
    TestMultiRW<wxLongLong>::ValueArray ValuesLL;
    TestMultiRW<wxULongLong>::ValueArray ValuesULL;

    ValuesLL.push_back(wxLongLong(0l));
    ValuesLL.push_back(wxLongLong(1l));
    ValuesLL.push_back(wxLongLong(-1l));
    ValuesLL.push_back(wxLongLong(0x12345678l));
    ValuesLL.push_back(wxLongLong(0x12345678l, 0xabcdef01l));

    ValuesULL.push_back(wxULongLong(0l));
    ValuesULL.push_back(wxULongLong(1l));
    ValuesULL.push_back(wxULongLong(0x12345678l));
    ValuesULL.push_back(wxULongLong(0x12345678l, 0xabcdef01l));

    CHECK( TestRW(wxLongLong(0x12345678l)) == wxLongLong(0x12345678l) );
    CHECK( TestRW(wxLongLong(0x12345678l, 0xabcdef01l)) == wxLongLong(0x12345678l, 0xabcdef01l) );
    CHECK( TestMultiRW<wxLongLong>(ValuesLL, &wxDataOutputStream::WriteLL, &wxDataInputStream::ReadLL).IsOk() );
    CHECK( TestMultiRW<wxULongLong>(ValuesULL, &wxDataOutputStream::WriteLL, &wxDataInputStream::ReadLL).IsOk() );
}
#endif

#if wxHAS_INT64
TEST_CASE("Int64RW")
{
    TestMultiRW<std::int64_t>::ValueArray ValuesI64;
    TestMultiRW<std::uint64_t>::ValueArray ValuesUI64;

    ValuesI64.push_back(std::int64_t(0l));
    ValuesI64.push_back(std::int64_t(1l));
    ValuesI64.push_back(std::int64_t(-1l));
    ValuesI64.push_back(std::int64_t(0x12345678l));
    ValuesI64.push_back((std::int64_t(0x12345678l) << 32) + std::int64_t(0xabcdef01l));

    ValuesUI64.push_back(std::uint64_t(0l));
    ValuesUI64.push_back(std::uint64_t(1l));
    ValuesUI64.push_back(std::uint64_t(0x12345678l));
    ValuesUI64.push_back((std::uint64_t(0x12345678l) << 32) + std::uint64_t(0xabcdef01l));

    CHECK( TestRW(std::uint64_t(0x12345678l)) == std::uint64_t(0x12345678l) );
    CHECK( TestRW((std::uint64_t(0x12345678l) << 32) + std::uint64_t(0xabcdef01l)) == (std::uint64_t(0x12345678l) << 32) + std::uint64_t(0xabcdef01l) );
    CHECK( TestMultiRW<std::int64_t>(ValuesI64, &wxDataOutputStream::Write64, &wxDataInputStream::Read64).IsOk() );
    CHECK( TestMultiRW<std::uint64_t>(ValuesUI64, &wxDataOutputStream::Write64, &wxDataInputStream::Read64).IsOk() );
}
#endif

//void DataStreamTestCase::NaNRW()
//{
//    //TODO?
//}
