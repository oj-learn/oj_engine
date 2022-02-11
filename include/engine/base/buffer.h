#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

/*---------------------------------------------------------------------------------
data_t
---------------------------------------------------------------------------------*/
class buffer_t {
public:
    //-----------------------------------------------------------------------------
    using sptr_t = std::shared_ptr<buffer_t>;

protected:
    //-----------------------------------------------------------------------------
    char* m_buff     = nullptr;
    int   m_buffSize = 0;
    int   m_type     = 0;
    int   m_posRead  = 0;
    int   m_posWrite = 0;
    long  m_extra    = 0;

public:
    //-----------------------------------------------------------------------------
    buffer_t(char* buffer = nullptr, int size = 0, int type = 0);
    //-----------------------------------------------------------------------------
    virtual ~buffer_t();
    //-----------------------------------------------------------------------------
    static auto create() -> sptr_t;
    //-----------------------------------------------------------------------------
    static auto create(int nSize, int type = 0) -> sptr_t;
    //-----------------------------------------------------------------------------
    static auto create(std::vector<char>&& buffer, int type = 0) -> sptr_t;
    //-----------------------------------------------------------------------------
    static auto create(std::string&& buffer, int type = 0) -> sptr_t;
    //-----------------------------------------------------------------------------
    static auto create(char* buffer, int nSize, int type = 0) -> sptr_t;
    //-----------------------------------------------------------------------------
    static auto create(sptr_t data) -> sptr_t;

public:
    //-----------------------------------------------------------------------------
    auto write(const char* buffer, const int bufferSize) -> int;
    //-----------------------------------------------------------------------------
    void writeAsync(char*& ptrRet, int& sizeRet);
    //-----------------------------------------------------------------------------
    void writeAsyncFinish(int nSize) { m_posWrite += nSize; }
    //-----------------------------------------------------------------------------
    auto read(char* const buffer, const int bufferSize) -> int;
    //-----------------------------------------------------------------------------
    void readAsync(char*& ptrRet, int& sizeRet);
    //-----------------------------------------------------------------------------
    void readAsyncFinish(int nSize) { m_posRead += nSize; }

public:
    //-----------------------------------------------------------------------------
    auto data() -> const char* { return m_buff; }
    //-----------------------------------------------------------------------------
    auto size() -> int { return m_buffSize; }
    //-----------------------------------------------------------------------------
    auto type() -> int { return m_type; }
    void typeSet(int type) { m_type = type; }
    //-----------------------------------------------------------------------------
    auto extra() -> long { return m_extra; }
    void extraSet(long v) { m_extra = v; }
    //-----------------------------------------------------------------------------
    void posReset() { m_posWrite = 0, m_posRead = 0; }
    //-----------------------------------------------------------------------------
    auto writeSizeRemain() -> int { return m_buffSize - m_posWrite; }
    //-----------------------------------------------------------------------------
    auto readSizeRemain() -> int { return m_posWrite - m_posRead; }
    //-----------------------------------------------------------------------------
    bool readFull() { return m_posRead == m_buffSize; }
};

/*---------------------------------------------------------------------------------
动态大小
---------------------------------------------------------------------------------*/
class data_dynamic_t : public buffer_t {
private:
    std::vector<char> m_buffer;

public:
    //-----------------------------------------------------------------------------
    data_dynamic_t(int nSize, int type)
        : m_buffer(nSize, 0)
    {
        m_buff     = m_buffer.data();
        m_buffSize = nSize;
        m_type     = type;
    }
    //-----------------------------------------------------------------------------
    data_dynamic_t(std::vector<char>&& buffer, int type)
        : m_buffer(std::move(buffer))
    {
        m_buff     = m_buffer.data();
        m_buffSize = m_buffer.size();
        m_type     = type;
    }
};

/*---------------------------------------------------------------------------------
动态大小
---------------------------------------------------------------------------------*/
class data_string_t : public buffer_t {
private:
    std::string m_buffer;

public:
    //-----------------------------------------------------------------------------
    data_string_t(std::string& buffer, int type)
        : m_buffer(std::move(buffer))
    {
        m_buff     = m_buffer.data();
        m_buffSize = m_buffer.size();
        m_type     = type;
    }
};

/*---------------------------------------------------------------------------------
静态大小
---------------------------------------------------------------------------------*/
template <int BUFFER_SIZE = 4096>
class data_static_t : public buffer_t {
private:
    char m_buffer[BUFFER_SIZE] = { 0 };

public:
    //-----------------------------------------------------------------------------
    data_static_t(int type = 0)
    {
        m_buff     = m_buffer;
        m_buffSize = BUFFER_SIZE;
        m_type     = type;
    }
};
