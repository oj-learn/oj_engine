
#pragma once

#include <charconv>
#include <memory>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace oj_string {

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
std::string_view get_shared_string(std::string_view in_str);


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
std::vector<_str_type> stringSplit(const std::basic_string<E, TR, AL>& in, const char* delim)
{
    std::basic_regex<E> re{ delim };
    return std::vector<_str_type>{
        //-1,相反的，如果不填，结果是找出delim
        std::regex_token_iterator<typename _str_type::const_iterator>(std::cbegin(in), std::cend(in), re, -1),
        std::regex_token_iterator<typename _str_type::const_iterator>()
    };
}

/*---------------------------------------------------------------------------------
通过正则表达式 delim
分隔字符串, 返回string_view 注意注意注意!!要保证in的生命周期．．．．．．
---------------------------------------------------------------------------------*/
std::vector<std::string_view> stringSplitView(std::string_view in, const char* delim);


/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
std::string concate_string(const std::vector<std::string>& str_list);
std::string concate_string(const std::vector<std::string_view>& str_list);


/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
std::string_view strip_blank(std::string_view input);


/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
auto cast(long long value) -> std::string_view;

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename T = int>
inline auto cast(std::string_view _text) -> T
{
    T result;
    auto [p, ec] = std::from_chars(_text.data(), _text.data() + _text.size(), result);
    if (ec == std::errc()) {
    }
    return result;
}

template <>
auto cast<bool>(std::string_view _text) -> bool;
template <>
auto cast<float>(std::string_view _text) -> float;
template <>
auto cast<double>(std::string_view _text) -> double;


/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename T = int>
inline auto casts(std::string_view _text, const char* delim = ",") -> std::vector<T>
{
    auto           vecView = stringSplitView(_text, delim);
    std::vector<T> ret;
    ret.reserve(vecView.size());

    for (auto& view : vecView) {
        auto& result = ret.emplace_back();
        auto [p, ec] = std::from_chars(view.data(), view.data() + view.size(), result);
        if (ec == std::errc()) {
        }
    }

    return ret;
}


}  // namespace oj_string
