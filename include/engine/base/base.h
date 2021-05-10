#pragma once

#include "./buffer.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <uuid/uuid.h>

/*---------------------------------------------------------------------------------
命名规则
1: 类型统一 xxx_t
2: 函数统一 小写+下划线
3: 变量统一 小 

sum  // 加
difference // 减
product // 乘
quotient // 除
remainder // 求余
父(parent)、子(child)和同胞(sibling)
---------------------------------------------------------------------------------*/

//注释方式 1
//---------------------------------------------------------------------------------

//注释方式 2
/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/

//注释方式 3
/***************************************  ***************************************/

/*---------------------------------------------------------------------------------
状态定义
---------------------------------------------------------------------------------*/
enum class status_t : int32_t {
    null,
    unknow,
    ptr,
    exist,
    make,
    open,
    interruptcut,
    free,
    wait,
    close,
};

/*---------------------------------------------------------------------------------
channel
---------------------------------------------------------------------------------*/
struct channel_t {
    //-----------------------------------------------------------------------------
    using data_t  = buffer_t::sptr_t;
    using write_t = std::function<status_t(const data_t)>;
    using read_t  = std::function<status_t(data_t&)>;
    using post_t  = std::shared_ptr<write_t>;
    using w_t     = std::weak_ptr<write_t>;
    using r_t     = std::weak_ptr<read_t>;
    //-----------------------------------------------------------------------------
    static auto makeWrite(write_t&& w)
    {
        return std::make_shared<write_t>(std::move(w));
    }
    //-----------------------------------------------------------------------------
    static auto makeRead(read_t&& r)
    {
        return std::make_shared<read_t>(std::move(r));
    }
    //-----------------------------------------------------------------------------
    static w_t nullWrite()
    {
        static auto w = std::make_shared<write_t>([](const data_t) {
            return status_t::ptr;
        });

        return w;
    }
};


//---------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------
将参数拼接成string
//typename... Args模板参数包，是N个类型的集合， const Args&... args函数参数包，是N个参数的集合。
---------------------------------------------------------------------------------*/
template <typename... Args>
inline std::string stringByArgs(const Args&... args)
{
    auto toStr = [](auto t) -> auto
    {
        std::ostringstream ss;
        ss << t << " ";
        return std::move(ss.str());
    };

    return std::move((toStr(args) + ...));
}

/*---------------------------------------------------------------------------------
通过正则表达式 delim
分隔字符串
---------------------------------------------------------------------------------*/
template <typename E, typename TR = std::char_traits<E>, typename AL = std::allocator<E>, typename _str_type = std::basic_string<E, TR, AL>>
std::vector<_str_type> stringSplit(const std::basic_string<E, TR, AL>& in, const std::basic_string<E, TR, AL>&& delim)
{
    std::basic_regex<E> re{ delim };
    return std::vector<_str_type>{
        //-1,相反的，如果不填，结果是找出delim
        std::regex_token_iterator<typename _str_type::const_iterator>(std::begin(in), std::end(in), re, -1),
        std::regex_token_iterator<typename _str_type::const_iterator>()
    };
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------
GUID
---------------------------------------------------------------------------------*/
int64_t guidGen();

/*---------------------------------------------------------------------------------
rand
---------------------------------------------------------------------------------*/
int32_t rand(int32_t max);