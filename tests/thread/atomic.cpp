///////////////////////////////////////////////////////////////////////////////
// Name:        tests/thread/atomic.cpp
// Purpose:     wxAtomic??? unit test
// Author:      Armel Asselin
// Created:     2006-12-14
// Copyright:   (c) 2006 Armel Asselin
///////////////////////////////////////////////////////////////////////////////

#include "doctest.h"

#include "testprec.h"

#include "wx/thread.h"
#include "wx/log.h"

import <atomic>;

using wxArrayThread = std::vector<wxThread*>;

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// number of times to run the loops: the code takes too long to run if we use
// the bigger value with generic atomic operations implementation
#ifdef wxHAS_ATOMIC_OPS
    static const std::int32_t ITERATIONS_NUM = 10000000;
#else
    static const std::int32_t ITERATIONS_NUM = 1000;
#endif

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

enum class ETestType
{
    IncAndDecMixed,
    IncOnly,
    DecOnly
};

class AtomicThread : public wxThread
{
public:
    AtomicThread(std::atomic_int& operateOn, ETestType testType) : wxThread( wxThreadKind::Joinable),
        m_operateOn(operateOn), m_testType(testType) {}

    // thread execution starts here
    void* Entry() override;

public:
    std::atomic_int& m_operateOn;
    ETestType m_testType;
};

static void TestWithThreads(int count, ETestType testType)
{
    std::atomic_int    int1 = 0;

    wxArrayThread  threads;

    for (int i = 0; i < count; ++i)
    {
        ETestType actualThreadType;
        switch (testType)
        {
        default:
            actualThreadType = testType;
            break;
        case ETestType::IncOnly:
            actualThreadType = (i & 1) == 0 ? ETestType::IncOnly : ETestType::DecOnly;
            break;
        }

        AtomicThread* thread = new AtomicThread(int1, actualThreadType);

        if (thread->Create() != wxThreadError::None)
        {
            wxLogError(wxT("Can't create thread!"));
            delete thread;
        }
        else
            threads.Add(thread);
    }

    for (int i = 0; i < count; ++i)
    {
        threads[i]->Run();
    }


    for (int i = 0; i < count; ++i)
    {
        // each thread should return 0, else it detected some problem
        CHECK(threads[i]->Wait() == (wxThread::ExitCode)0);
        delete threads[i];
    }

    CHECK(int1 == 0);
}

TEST_CASE("TestTenThreadsMix")
{
    TestWithThreads(10, ETestType::IncAndDecMixed);
}

TEST_CASE("TestTwoThreadsMix")
{
    TestWithThreads(2, ETestType::IncAndDecMixed);
}

TEST_CASE("TestTenThreadsSeparate")
{
    TestWithThreads(10, ETestType::IncOnly);
}

TEST_CASE("TestTwoThreadsSeparate")
{
    TestWithThreads(2, ETestType::IncOnly);
}

void* AtomicThread::Entry()
{
    std::int32_t negativeValuesSeen = 0;

    for ( std::int32_t i = 0; i < ITERATIONS_NUM; ++i )
    {
        switch ( m_testType )
        {
            case ETestType::IncAndDecMixed:
                ++m_operateOn;
                --m_operateOn;

                if (m_operateOn < 0)
                    ++negativeValuesSeen;
                break;

            case ETestType::IncOnly:
                ++m_operateOn;
                break;

            case ETestType::DecOnly:
                --m_operateOn;
                break;
        }
    }

    return wxUIntToPtr(negativeValuesSeen);
}
