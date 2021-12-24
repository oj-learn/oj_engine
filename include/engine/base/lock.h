#pragma once

#include <atomic>
#include <mutex>
#include <thread>
// #include <windows.h>

//-----------------------------------------------------------------------------
// ILockObj 锁接口统一
//-----------------------------------------------------------------------------
class ILockObj {
public:
    virtual void lock(void)   = 0;
    virtual void unlock(void) = 0;
};


//-----------------------------------------------------------------------------
// std::mutex
//-----------------------------------------------------------------------------
class CLockStdMutex : public ILockObj {
public:
    CLockStdMutex() {}
    virtual ~CLockStdMutex(void) { m_mutex.unlock(); }

public:
    void lock(void) override { m_mutex.lock(); }
    void unlock(void) override { m_mutex.unlock(); }

private:
    std::mutex m_mutex;
};

//-----------------------------------------------------------------------------
/* test_and_set() 函数检查 std::atomic_flag 标志，
如果 std::atomic_flag 之前没有被设置过，则设置 std::atomic_flag 的标志，并返回先前该 std::atomic_flag 对象是否被设置过，
如果之前 std::atomic_flag 对象已被设置，则返回 true，否则返回 false。*/
//-----------------------------------------------------------------------------
class CLockAtomicflag : public ILockObj {
public:
    CLockAtomicflag() { m_flg.clear(); }
    virtual ~CLockAtomicflag() { m_flg.clear(); }

public:
    void lock(void) override
    {
        while (m_flg.test_and_set()) {
            std::this_thread::yield();
        };
    }
    void unlock(void) override { m_flg.clear(); }

private:
    std::atomic_flag m_flg;
};

/*
//-----------------------------------------------------------------------------
// CCriticalSection
//-----------------------------------------------------------------------------
class CLockCS : public ILockObj {
public:
    CLockCS() { InitializeCriticalSection(&m_CritSec); }
    virtual ~CLockCS() { DeleteCriticalSection(&m_CritSec); }

public:
    void Lock(void) override { EnterCriticalSection(&m_CritSec); }
    void Unlock(void) override { LeaveCriticalSection(&m_CritSec); }

private:
    CRITICAL_SECTION m_CritSec;
};

//-----------------------------------------------------------------------------
// CMutexLock
//-----------------------------------------------------------------------------
class CLockHMutex : public ILockObj {
public:
    CLockHMutex()
        : m_hMutex(NULL) {}
    virtual ~CLockHMutex(void)
    {
        if (m_hMutex)
            ::CloseHandle(m_hMutex);
    }

public:
    void Lock(void) override
    {
        if (m_hMutex)
            ::WaitForSingleObject(m_hMutex, INFINITE);
    }
    void Unlock(void) override
    {
        if (m_hMutex)
            ::ReleaseMutex(m_hMutex);
    }

private:
    HANDLE m_hMutex;
};

//-----------------------------------------------------------------------------
// InterlockedExchange(a,b)能以原子操作的方式交换俩个参数a, b，并返回a以前的值；
//-----------------------------------------------------------------------------
class CLockVolatileVal : public ILockObj {
public:
    CLockVolatileVal(void) { m_ics = 0; }
    virtual ~CLockVolatileVal(void) {}

public:
    void Lock(void) override
    {
        while (InterlockedExchange(&m_ics, 1) == 1) {
            std::this_thread::yield();
        };
    }
    void Unlock(void) override { InterlockedExchange(&m_ics, 0); }

private:
    volatile long m_ics;
};
*/