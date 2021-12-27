///////////////////////////////////////////////////////////////////////////////
// Name:        tests/thread/misc.cpp
// Purpose:     Miscellaneous wxThread test cases
// Author:      Francesco Montorsi (extracted from console sample)
// Created:     2010-05-10
// Copyright:   (c) 2010 wxWidgets team
///////////////////////////////////////////////////////////////////////////////

#include "testprec.h"

#include "wx/thread.h"
#include "wx/utils.h"

#include <chrono>

using namespace std::chrono_literals;

// ----------------------------------------------------------------------------
// globals
// ----------------------------------------------------------------------------

static size_t gs_counter = (size_t)-1;
static wxCriticalSection gs_critsect;
static wxSemaphore gs_cond;

class MyJoinableThread : public wxThread
{
public:
    MyJoinableThread(size_t n) : wxThread( wxThreadKind::Joinable)
        { m_n = n; Create(); }

    // thread execution starts here
    ExitCode Entry() override;

private:
    size_t m_n;
};

wxThread::ExitCode MyJoinableThread::Entry()
{
    wxUIntPtr res = 1;
    for ( size_t n = 1; n < m_n; n++ )
    {
        res *= n;

        // it's a loooong calculation :-)
        wxSleep(100ms);
    }

    return (ExitCode)res;
}

class MyDetachedThread : public wxThread
{
public:
    MyDetachedThread(size_t n, wxChar ch)
    {
        m_n = n;
        m_ch = ch;
        m_cancelled = false;

        Create();
    }

    // thread execution starts here
    ExitCode Entry() override;

    // and stops here
    void OnExit() override;

private:
    size_t m_n; // number of characters to write
    wxChar m_ch;  // character to write

    bool m_cancelled;   // false if we exit normally
};

wxThread::ExitCode MyDetachedThread::Entry()
{
    {
        wxCriticalSectionLocker lock(gs_critsect);
        if ( gs_counter == (size_t)-1 )
            gs_counter = 1;
        else
            gs_counter++;
    }

    for ( size_t n = 0; n < m_n; n++ )
    {
        if ( TestDestroy() )
        {
            m_cancelled = true;

            break;
        }

        //wxPutchar(m_ch);
        //fflush(stdout);

        wxSleep(100ms);
    }

    return 0;
}

void MyDetachedThread::OnExit()
{
    //wxLogTrace(wxT("thread"), wxT("Thread %ld is in OnExit"), GetId());

    wxCriticalSectionLocker lock(gs_critsect);
    if ( !--gs_counter && !m_cancelled )
        gs_cond.Post();
}

class MyWaitingThread : public wxThread
{
public:
    MyWaitingThread( wxMutex *mutex, wxCondition *condition )
    {
        m_mutex = mutex;
        m_condition = condition;

        Create();
    }

    ExitCode Entry() override
    {
        //wxPrintf(wxT("Thread %lu has started running.\n"), GetId());
        gs_cond.Post();

        //wxPrintf(wxT("Thread %lu starts to wait...\n"), GetId());

        m_mutex->Lock();
        m_condition->Wait();
        m_mutex->Unlock();

        //wxPrintf(wxT("Thread %lu finished to wait, exiting.\n"), GetId());

        return 0;
    }

private:
    wxMutex *m_mutex;
    wxCondition *m_condition;
};

// semaphore tests
#include "wx/datetime.h"

class MySemaphoreThread : public wxThread
{
public:
    MySemaphoreThread(int i, wxSemaphore *sem)
        : wxThread( wxThreadKind::Joinable),
          m_sem(sem),
          m_i(i)
    {
        Create();
    }

    ExitCode Entry() override
    {
        wxUnusedVar(m_i);

        //wxPrintf(wxT("%s: Thread #%d (%ld) starting to wait for semaphore...\n"),
        //         wxDateTime::Now().FormatTime().c_str(), m_i, (long)GetId());

        m_sem->Wait();

        //wxPrintf(wxT("%s: Thread #%d (%ld) acquired the semaphore.\n"),
        //         wxDateTime::Now().FormatTime().c_str(), m_i, (long)GetId());

        Sleep(1000);

        //wxPrintf(wxT("%s: Thread #%d (%ld) releasing the semaphore.\n"),
        //         wxDateTime::Now().FormatTime().c_str(), m_i, (long)GetId());

        m_sem->Post();

        return 0;
    }

private:
    wxSemaphore *m_sem;
    int m_i;
};

using ArrayThreads = std::vector<wxThread*>;

// ----------------------------------------------------------------------------
// test class
// ----------------------------------------------------------------------------

class MiscThreadTestCase : public CppUnit::TestCase
{
public:
    MiscThreadTestCase();

private:
    CPPUNIT_TEST_SUITE( MiscThreadTestCase );
        CPPUNIT_TEST( TestJoinable );
        CPPUNIT_TEST( TestDetached );
        CPPUNIT_TEST( TestThreadSuspend );
        CPPUNIT_TEST( TestThreadDelete );
        CPPUNIT_TEST( TestThreadRun );
        CPPUNIT_TEST( TestThreadConditions );
        CPPUNIT_TEST( TestSemaphore );
    CPPUNIT_TEST_SUITE_END();

    void TestJoinable();
    void TestDetached();
    void TestSemaphore();

    void TestThreadSuspend();
    void TestThreadDelete();
    void TestThreadRun();
    void TestThreadConditions();

    MiscThreadTestCase(const MiscThreadTestCase&) = delete;
	MiscThreadTestCase& operator=(const MiscThreadTestCase&) = delete;
};

// register in the unnamed registry so that these tests are run by default
CPPUNIT_TEST_SUITE_REGISTRATION( MiscThreadTestCase );

// also include in its own registry so that these tests can be run alone
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( MiscThreadTestCase, "MiscThreadTestCase" );

MiscThreadTestCase::MiscThreadTestCase()
{
    int nCPUs = wxThread::GetCPUCount();
    if ( nCPUs != -1 )
        wxThread::SetConcurrency(nCPUs);
}

void MiscThreadTestCase::TestJoinable()
{
    // calc 10! in the background
    MyJoinableThread thread(10);
    CPPUNIT_ASSERT_EQUAL( wxThreadError::None, thread.Run() );
    CPPUNIT_ASSERT_EQUAL( 362880, (wxUIntPtr)thread.Wait() );
}

void MiscThreadTestCase::TestDetached()
{
    static const size_t nThreads = 3;
    MyDetachedThread *threads[nThreads];

    size_t n;
    for ( n = 0; n < nThreads; n++ )
    {
        threads[n] = new MyDetachedThread(10, 'A' + n);
    }

    threads[0]->SetPriority(wxPRIORITY_MIN);
    threads[1]->SetPriority(wxPRIORITY_MAX);

    for ( n = 0; n < nThreads; n++ )
    {
        CPPUNIT_ASSERT_EQUAL( wxThreadError::None, threads[n]->Run() );
    }

    // wait until all threads terminate
    CPPUNIT_ASSERT_EQUAL( wxSemaError::None, gs_cond.Wait() );
}

void MiscThreadTestCase::TestSemaphore()
{
    static const int SEM_LIMIT = 3;

    wxSemaphore sem(SEM_LIMIT, SEM_LIMIT);
    ArrayThreads threads;

    for ( int i = 0; i < 3*SEM_LIMIT; i++ )
    {
        threads.Add(new MySemaphoreThread(i, &sem));
        CPPUNIT_ASSERT_EQUAL( wxThreadError::None, threads.Last()->Run() );
    }

    for ( size_t n = 0; n < threads.GetCount(); n++ )
    {
        CPPUNIT_ASSERT_EQUAL( 0, (wxUIntPtr)threads[n]->Wait() );
        delete threads[n];
    }
}

void MiscThreadTestCase::TestThreadSuspend()
{
    MyDetachedThread *thread = new MyDetachedThread(15, 'X');

    CPPUNIT_ASSERT_EQUAL( wxThreadError::None, thread->Run() );

    // this is for this demo only, in a real life program we'd use another
    // condition variable which would be signaled from wxThread::Entry() to
    // tell us that the thread really started running - but here just wait a
    // bit and hope that it will be enough (the problem is, of course, that
    // the thread might still not run when we call Pause() which will result
    // in an error)
    wxSleep(300ms);

    for ( size_t n = 0; n < 3; n++ )
    {
        thread->Pause();

        if ( n > 0 )
        {
            // don't sleep but resume immediately the first time
            wxSleep(300ms);
        }

        CPPUNIT_ASSERT_EQUAL( wxThreadError::None, thread->Resume() );
    }

    // wait until the thread terminates
    CPPUNIT_ASSERT_EQUAL( wxSemaError::None, gs_cond.Wait() );
}

void MiscThreadTestCase::TestThreadDelete()
{
    // FIXME:
    // As above, using Sleep() is only for testing here - we must use some
    // synchronisation object instead to ensure that the thread is still
    // running when we delete it - deleting a detached thread which already
    // terminated will lead to a crash!

    MyDetachedThread *thread0 = new MyDetachedThread(30, 'W');
    CPPUNIT_ASSERT_EQUAL( wxThreadError::MiscError, thread0->Delete() );
        // delete a thread which didn't start to run yet.

    MyDetachedThread *thread1 = new MyDetachedThread(30, 'Y');
    CPPUNIT_ASSERT_EQUAL( wxThreadError::None, thread1->Run() );
    wxSleep(300ms);
    CPPUNIT_ASSERT_EQUAL( wxThreadError::None, thread1->Delete() );
        // delete a running thread

    MyDetachedThread *thread2 = new MyDetachedThread(30, 'Z');
    CPPUNIT_ASSERT_EQUAL( wxThreadError::None, thread2->Run() );
    wxSleep(300ms);
    CPPUNIT_ASSERT_EQUAL( wxThreadError::None, thread2->Pause() );
    CPPUNIT_ASSERT_EQUAL( wxThreadError::None, thread2->Delete() );
        // delete a sleeping thread

    MyJoinableThread thread3(20);
    CPPUNIT_ASSERT_EQUAL( wxThreadError::None, thread3.Run() );
    CPPUNIT_ASSERT_EQUAL( wxThreadError::None, thread3.Delete() );
        // delete a joinable running thread

    MyJoinableThread thread4(2);
    CPPUNIT_ASSERT_EQUAL( wxThreadError::None, thread4.Run() );
    wxSleep(300ms);
    CPPUNIT_ASSERT_EQUAL( wxThreadError::None, thread4.Delete() );
        // delete a joinable thread which already terminated
}

void MiscThreadTestCase::TestThreadRun()
{
    MyJoinableThread thread1(2);
    CPPUNIT_ASSERT_EQUAL( wxThreadError::None, thread1.Run() );
    thread1.Wait();     // wait until the thread ends

    // verify that running twice the same thread fails
    CHECK_THROWS( thread1.Run() );
}

void MiscThreadTestCase::TestThreadConditions()
{
    wxMutex mutex;
    wxCondition condition(mutex);

    // otherwise its difficult to understand which log messages pertain to
    // which condition
    //wxLogTrace(wxT("thread"), wxT("Local condition var is %08x, gs_cond = %08x"),
    //           condition.GetId(), gs_cond.GetId());

    // create and launch threads
    MyWaitingThread *threads[10];

    size_t n;
    for ( n = 0; n < WXSIZEOF(threads); n++ )
    {
        threads[n] = new MyWaitingThread( &mutex, &condition );
    }

    for ( n = 0; n < WXSIZEOF(threads); n++ )
    {
        CPPUNIT_ASSERT_EQUAL( wxThreadError::None, threads[n]->Run() );
    }

    // wait until all threads run
    // NOTE: main thread is waiting for the other threads to start
    size_t nRunning = 0;
    while ( nRunning < WXSIZEOF(threads) )
    {
        CPPUNIT_ASSERT_EQUAL( wxSemaError::None, gs_cond.Wait() );

        nRunning++;

        // note that main thread is already running
    }

    wxSleep(500ms);

#if 1
    // now wake one of them up
    CPPUNIT_ASSERT_EQUAL( wxCondError::None, condition.Signal() );
#endif

    wxSleep(200ms);

    // wake all the (remaining) threads up, so that they can exit
    CPPUNIT_ASSERT_EQUAL( wxCondError::None, condition.Broadcast() );

    // give them time to terminate (dirty!)
    wxSleep(500ms);
}
