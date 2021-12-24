#pragma once

#include <algorithm>
#include <string>
#include <vector>

/*---------------------------------------------------------------------------------
实作出一个内存块，这个内存块可以给 string_view使用
---------------------------------------------------------------------------------*/
class buffer_arena_t {
private:
    using memory_t = std::vector<char>;
    //-----------------------------------------------------------------------------
    int                   cur_buffer_used = 0;
    const int             buffer_page_size;
    std::vector<memory_t> buffers;

public:
    //-----------------------------------------------------------------------------
    buffer_arena_t(int page_size = 4 * 1024);
    //-----------------------------------------------------------------------------
    ~buffer_arena_t() = default;

public:
    //-----------------------------------------------------------------------------
    std::string_view write(std::string_view in);
    //-----------------------------------------------------------------------------
    char* get(int m_size, int align_size);
    //-----------------------------------------------------------------------------
    template <typename T>
    T* get(int count)
    {
        return count == 0 ? nullptr : reinterpret_cast<T*>(get(sizeof(T) * count, alignof(T)));
    }
};