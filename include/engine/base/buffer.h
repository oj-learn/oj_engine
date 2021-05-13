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
    char*   m_buff     = nullptr;
    int32_t m_buffSize = 0;
    int32_t m_type     = 0;
    int32_t m_posRead  = 0;
    int32_t m_posWrite = 0;
    int64_t m_extra    = 0;

public:
    //-----------------------------------------------------------------------------
    buffer_t(char* buffer = nullptr, int32_t size = 0, int32_t type = 0);
    //-----------------------------------------------------------------------------
    virtual ~buffer_t();
    //-----------------------------------------------------------------------------
    static auto create() -> sptr_t;
    //-----------------------------------------------------------------------------
    static auto create(int32_t nSize, int32_t type = 0) -> sptr_t;
    //-----------------------------------------------------------------------------
    static auto create(std::vector<char>&& buffer, int32_t type = 0) -> sptr_t;
    //-----------------------------------------------------------------------------
    static auto create(std::string&& buffer, int32_t type = 0) -> sptr_t;
    //-----------------------------------------------------------------------------
    static auto create(char* buffer, int32_t nSize, int32_t type = 0) -> sptr_t;
    //-----------------------------------------------------------------------------
    static auto create(sptr_t data) -> sptr_t;

public:
    //-----------------------------------------------------------------------------
    auto write(const char* buffer, const int32_t bufferSize) -> int32_t;
    //-----------------------------------------------------------------------------
    void writeAsync(char*& ptrRet, int32_t& sizeRet);
    //-----------------------------------------------------------------------------
    void writeAsyncFinish(int32_t nSize);
    //-----------------------------------------------------------------------------
    auto read(char* const buffer, const int32_t bufferSize) -> int32_t;
    //-----------------------------------------------------------------------------
    void readAsync(char*& ptrRet, int32_t& sizeRet);
    //-----------------------------------------------------------------------------
    void readAsyncFinish(int32_t nSize);

public:
    //-----------------------------------------------------------------------------
    auto data() -> const char*;
    //-----------------------------------------------------------------------------
    auto size() -> int32_t;
    //-----------------------------------------------------------------------------
    auto type() -> int32_t;
    void typeSet(int32_t type);
    //-----------------------------------------------------------------------------
    auto extra() -> int64_t;
    void extraSet(int64_t v);
    //-----------------------------------------------------------------------------
    void posReset();
    //-----------------------------------------------------------------------------
    auto writeSizeRemain() -> int32_t;
    //-----------------------------------------------------------------------------
    auto readSizeRemain() -> int32_t;
    //-----------------------------------------------------------------------------
    bool readFull();
};

/*---------------------------------------------------------------------------------
动态大小
---------------------------------------------------------------------------------*/
class data_dynamic_t : public buffer_t {
private:
    std::vector<char> m_buffer;

public:
    //-----------------------------------------------------------------------------
    data_dynamic_t(int32_t nSize, int32_t type)
        : m_buffer(nSize <= 0 ? 1 : nSize, 0)
    {
        m_buff     = m_buffer.data();
        m_buffSize = nSize;
        m_type     = type;
    }
    //-----------------------------------------------------------------------------
    data_dynamic_t(std::vector<char>&& buffer, int32_t type)
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
    data_string_t(std::string& buffer, int32_t type)
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
template <int32_t BUFFER_SIZE = 4096>
class data_static_t : public buffer_t {
private:
    char m_buffer[BUFFER_SIZE] = { 0 };

public:
    //-----------------------------------------------------------------------------
    data_static_t()
    {
        m_buff     = m_buffer;
        m_buffSize = BUFFER_SIZE;
    }
};
