#pragma once

#include "./base.h"
#include "./exception_t.h"
#include "./log.h"
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
    echo,
    REQUEST,
    RPC_ROUTE,
    PKG_ROUTE,
    SYNC_REP_ROUTE,
    ASYNC_REP_ROUTE,
    PKG,
    redirect,
    OK,
    FAIL,
    UNKNOWN,
    TIMEOUT,
    CANCEL,
    CLOSE,
    API_UP,
    API_DOWN,
    API_SYNC,
    INTERFACE_SYNC,
};

/*---------------------------------------------------------------------------------
head_rep_t
---------------------------------------------------------------------------------*/
struct head_rep_t {
    int64_t     id;
    head_code_t code;
    uint16_t    len;
    int64_t     recvip;
    int64_t     recvport;
    int64_t     sendip;
    int64_t     sendport;
};

/*---------------------------------------------------------------------------------
head_req_t
---------------------------------------------------------------------------------*/
struct head_req_t {
    int64_t     id;
    head_code_t code;
    uint16_t    len;
    int64_t     hash;
    int64_t     recvip;
    int64_t     recvport;
    int64_t     sendip;
    int64_t     sendport;
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
    T unpack(char const* data, int32_t length);
    //-----------------------------------------------------------------------------
    template <typename T>
    T unpack(channel_t::data_t& data);
    //-----------------------------------------------------------------------------
    template <typename T>
    T unpack(zip_t& data);
    //-----------------------------------------------------------------------------
    template <typename T>
    bool fromJson(T& t, std::string& jsonStr);
    //-----------------------------------------------------------------------------
    template <typename T>
    bool toJson(T& t, std::string& jsonStr);

public:
    //-----------------------------------------------------------------------------
    template <typename... T>
    static auto buffer_t(int32_t type, T&&... ts) -> buffer_t::sptr_t;
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
zip_t codec_t::pack(head_req_t& t);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <>
zip_t codec_t::pack(head_rep_t& t);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <>
zip_t codec_t::pack(zip_t& t);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <>
zip_t codec_t::pack(channel_t::data_t& t);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename... T>
zip_t codec_t::packs(T&&... ts)
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
template <>
zip_t codec_t::packs(int64_t& guidapp, int64_t& guid, channel_t::data_t& data);

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
inline t_result codec_t::unpack(char const* data, int32_t length)
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
        auto message = stringByArgs("unpack exception type:", n3, "Args not match!");
        //-----------------------------------------------------------------------------
        throw std::move(exception_t{ int16_t(head_code_t::FAIL), std::move(message) });
    }
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <>
head_req_t* codec_t::unpack(char const* data, int32_t length);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <>
head_rep_t* codec_t::unpack(char const* data, int32_t length);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <>
channel_t::data_t codec_t::unpack(char const* data, int32_t length);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename T>
inline T codec_t::unpack(zip_t& zip)
{
    auto buffer = zip.data();
    auto size   = zip.size();
    //-----------------------------------------------------------------------------
    return this->unpack<T>(buffer, size);
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename T>
inline T codec_t::unpack(channel_t::data_t& data)
{
    auto buffer = data->data();
    auto size   = data->size();
    //-----------------------------------------------------------------------------
    return this->unpack<T>(buffer, size);
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename... T>
inline auto codec_t::buffer_t(int32_t type, T&&... ts) -> buffer_t::sptr_t
{
    auto zip  = codec_.packs(std::forward<T>(ts)...);
    auto data = buffer_t::create(std::move(zip), type);
    return data;
}


/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename T>
inline bool codec_t::fromJson(T& t, std::string& jsonStr)
{
    return iguana::json::from_json(t, jsonStr.data(), jsonStr.length());
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename T>
inline bool codec_t::toJson(T& t, std::string& jsonStr)
{
    iguana::string_stream ss;
    iguana::json::to_json(ss, t);

    jsonStr = ss.str();
    return true;
}