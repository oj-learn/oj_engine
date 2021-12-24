#pragma once

#include "base/lock.h"
#include <deque>
#include <functional>
#include <memory>

/*---------------------------------------------------------------------------------
Push可多个线程，Pop只可以一个线程
---------------------------------------------------------------------------------*/
template <typename xx_t>
class queue_rw_t {
public:
    //-----------------------------------------------------------------------------
    using lock_t = std::shared_ptr<ILockObj>;

private:
    //-----------------------------------------------------------------------------
    int              m_sizeWrite;
    lock_t           m_mutexWrite;
    std::deque<xx_t> m_queueWrite;  // 交换队列1
    std::deque<xx_t> m_queueRead;   // 交换队列2

public:
    //-----------------------------------------------------------------------------
    queue_rw_t(lock_t lock = std::make_shared<CLockStdMutex>())
    {
        m_mutexWrite = lock;
        m_sizeWrite  = 0;
    }

    //-----------------------------------------------------------------------------
    ~queue_rw_t()
    {
        m_queueRead.clear();

        {
            std::lock_guard l(*m_mutexWrite);
            m_sizeWrite = 0;
            m_queueWrite.clear();
        }
    }

public:
    //-----------------------------------------------------------------------------
    bool empty() const
    {
        return m_queueRead.empty() && m_sizeWrite <= 0;
    }

    //-----------------------------------------------------------------------------
    int size() const
    {
        return m_queueRead.size() + m_sizeWrite;
    }

    //-----------------------------------------------------------------------------
    void push(const xx_t& xx)
    {
        std::lock_guard l(*m_mutexWrite);
        m_queueWrite.emplace_back(xx);
        m_sizeWrite += 1;
    }

    //-----------------------------------------------------------------------------
    void pull(xx_t& xxRet)
    {
        pull([&xxRet](xx_t& xx) -> int {
            xxRet = xx;
            return 1;
        });
    }

    //-----------------------------------------------------------------------------
    void pull(std::function<int(xx_t&)>&& fun)
    {
        if (m_queueRead.empty() && m_sizeWrite > 0) {
            std::lock_guard l(*m_mutexWrite);
            m_sizeWrite = 0;
            m_queueRead.swap(m_queueWrite);
        }

        while (!m_queueRead.empty()) {
            auto xx = m_queueRead.front();
            m_queueRead.pop_front();

            if (fun(xx) != 0) {
                break;
            }
        }
    }
};
