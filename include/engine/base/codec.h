#pragma once

#include "base/base.h"
#include "base/exception_t.h"
#include "base/log.h"
#include "base/string_util.h"
#include "iguana/iguana/json.hpp"
#include "iguana/iguana/msgpack.hpp"
#include <cxxabi.h>
#include <typeindex>

/***************************************  ***************************************/
using zip_t = std::vector<char>;
/***************************************  ***************************************/
struct codec_t;
extern codec_t codec_;
/***************************************  ***************************************/

/*---------------------------------------------------------------------------------
head_code_t
---------------------------------------------------------------------------------*/
enum class head_code_t : int16_t {
    null,
    API_UP,
    API_DOWN,
    API_SYNC,
    REQUEST,
    RPC_ROUTE,
    PKG_ROUTE,
    SYNC_REP_ROUTE,
    ASYNC_REP_ROUTE,
    ROUTETABLE,
    POST,
    PKG,
    OK,
    FAIL,
    UNKNOWN,
    TIMEOUT,
    CANCEL,
    CLOSE,

};

/*---------------------------------------------------------------------------------
header_t
---------------------------------------------------------------------------------*/
struct header_t {
    long        id       = 0;
    head_code_t code     = head_code_t::null;
    uint16_t    len      = 0;
    long        hash     = 0;
    long        recvip   = 0;
    long        recvport = 0;
    long        sendip   = 0;
    long        sendport = 0;
};

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct api_up_t {
    long        appid;
    long        actoid;
    long        hash;
    int         mask;
    std::string name;
};
REFLECTION(api_up_t, appid, actoid, hash, mask, name);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct api_down_t {
    long appid;
    long actoid;
};
REFLECTION(api_down_t, appid, actoid);


/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename T>
concept unpack_requires = requires(T t)
{
    t.data();
    t.size();
};

/*---------------------------------------------------------------------------------
codec_t
---------------------------------------------------------------------------------*/
struct codec_t {
public:
    //-----------------------------------------------------------------------------
    template <typename T>
    zip_t pack(T&& t);
    //-----------------------------------------------------------------------------
    template <typename... T>
    zip_t packs(T&&... ts);
    //-----------------------------------------------------------------------------
    template <typename... Args>
    zip_t packArgs(Args&&... args) const;
    //-----------------------------------------------------------------------------

public:
    //-----------------------------------------------------------------------------
    template <typename T>
    T unpack(char const* data, int length);
    //-----------------------------------------------------------------------------
    template <typename T>
    T unpack(zip_t& data);
    template <typename T>
    T unpack(channel_t::data_t& data);
    //-----------------------------------------------------------------------------
    template <typename T>
    bool fromJson(T&& t, std::string& jsonStr);
    //-----------------------------------------------------------------------------
    template <typename T>
    bool fromJson0(T&& t, std::string& jsonStr);
    //-----------------------------------------------------------------------------
    template <typename T>
    bool toJson(T&& t, std::string& jsonStr);

public:
    //-----------------------------------------------------------------------------
    template <typename... T>
    static auto buffer_t(int type, T&&... ts) -> buffer_t::sptr_t;
};

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename T>
inline zip_t codec_t::pack(T&& t)
{
    iguana::memory_buffer buffer;
    iguana::msgpack::to_msgpack(buffer, std::forward<T>(t));
    return buffer.release();
}

/*---------------------------------------------------------------------------------
copy elison 编译器优化技术，直接返回 buffer
---------------------------------------------------------------------------------*/
template <>
inline zip_t codec_t::pack(header_t& t)
{
    return zip_t{ (char*)(&t), (char*)(&t) + sizeof(header_t) };
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <>
inline zip_t codec_t::pack(zip_t& t)
{
    return std::move(t);
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <>
zip_t codec_t::pack(channel_t::data_t& t);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename... T>
inline zip_t codec_t::packs(T&&... ts)
{
    zip_t ret;
    auto  merge = [&ret](auto&& arg) {
        ret.insert(ret.end(), arg.begin(), arg.end());
    };

    (merge(pack(std::forward<T>(ts))), ...);
    return ret;
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename... Args>
inline zip_t codec_t::packArgs(Args&&... args) const
{
    iguana::memory_buffer buffer;
    auto                  args_tuple = std::make_tuple(std::forward<Args>(args)...);

    iguana::msgpack::to_msgpack(buffer, args_tuple);
    return buffer.release();
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename t_result>
inline t_result codec_t::unpack(char const* data, int length)
{
    try {
        t_result          t;
        msgpack::unpacked msg_;
        iguana::msgpack::from_msgpack(t, msg_, data, length);
        return t;

    } catch (...) {
        using namespace std::string_literals;
        //-----------------------------------------------------------------------------
        char        buff[128] = { 0 };
        std::size_t size      = sizeof(buff);
        int         status    = 0;
        auto        n1        = typeid(t_result).name();
        auto        n2        = abi::__cxa_demangle(n1, buff, &size, &status);
        std::string n3        = n2;
        //-----------------------------------------------------------------------------
        auto message = oj_string::stringByArgs("unpack exception type:", n3, "Args not match!");
        //-----------------------------------------------------------------------------
        throw std::move(exception_t{ int16_t(head_code_t::FAIL), std::move(message) });
    }
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <>
inline header_t* codec_t::unpack(char const* data, int length)
{
    return (header_t*)(data);
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <>
inline channel_t::data_t codec_t::unpack(char const* data, int length)
{
    return buffer_t::create(std::string{ data + sizeof(int), length - sizeof(int) }, *((int*)(data)));
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename T>
inline T codec_t::unpack(zip_t& zip)
{
    return this->unpack<T>(zip.data(), zip.size());
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename T>
inline T codec_t::unpack(channel_t::data_t& data)
{
    return this->unpack<T>(data->data(), data->size());
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename... T>
inline auto codec_t::buffer_t(int type, T&&... ts) -> buffer_t::sptr_t
{
    auto zip  = codec_.packs(std::forward<T>(ts)...);
    auto data = buffer_t::create(std::move(zip), type);
    return data;
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename T>
inline bool codec_t::fromJson(T&& t, std::string& jsonStr)
{
    return iguana::json::from_json(std::forward<T>(t), jsonStr.data(), jsonStr.length());
}

/*---------------------------------------------------------------------------------
对字段，顺序，存在无要求，要更多的计算
---------------------------------------------------------------------------------*/
template <typename T>
inline bool codec_t::fromJson0(T&& t, std::string& jsonStr)
{
    return iguana::json::from_json0(std::forward<T>(t), jsonStr.data(), jsonStr.length());
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename T>
inline bool codec_t::toJson(T&& t, std::string& jsonStr)
{
    iguana::string_stream ss;
    iguana::json::to_json(ss, std::forward<T>(t));

    jsonStr = ss.str();
    return true;
}