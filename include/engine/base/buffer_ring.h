#pragma once

#include "./buffer.h"
#include <list>

/*---------------------------------------------------------------------------------
实作出一个，适用于一个线程专门写一个线程专门读的场景，减少锁消耗。
BLOCK_SIZE，数据以块单位来存储，有1种情形会增长块数量，1：写追上读了，会在他们之间插入块。
有1种情形会减少块数量，1：写追读的情况下，写和读的距离>=设定的初始快数量。
---------------------------------------------------------------------------------*/
class buffer_ring_t {
    //-----------------------------------------------------------------------------
    using block_list_t = std::list<buffer_t::sptr_t>;
    using block_iter_t = typename block_list_t::iterator;

private:
    //-----------------------------------------------------------------------------
    int32_t      m_blockCount;
    int32_t      m_blockSize;
    int64_t      m_sizeWrite;
    int64_t      m_sizeRead;
    block_list_t m_blockList;
    block_iter_t m_blockItWrite;
    block_iter_t m_blockItRead;

public:
    //-----------------------------------------------------------------------------
    buffer_ring_t(int32_t nBlockCount, int32_t nBlockSize);
    //-----------------------------------------------------------------------------
    ~buffer_ring_t();

public:
    //-----------------------------------------------------------------------------
    auto write(const char* buffer, const int32_t bufferSize) -> int32_t;
    //-----------------------------------------------------------------------------
    auto writeAsync(char*& ptrRet, int32_t& sizeRet) -> int32_t;
    //-----------------------------------------------------------------------------
    void writeAsyncFinish(int32_t nSize);

public:
    //-----------------------------------------------------------------------------
    auto read(char* const buffer, const int32_t bufferSize) -> int32_t;
    //-----------------------------------------------------------------------------
    auto readAsync(char*& ptrRet, int32_t& sizeRet) -> int32_t;
    //-----------------------------------------------------------------------------
    void readAsyncFinish(int32_t nSize);
    //-----------------------------------------------------------------------------
    auto readSizeRemain() -> int32_t;
};
