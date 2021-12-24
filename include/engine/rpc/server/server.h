#pragma once

#include "base/codec.h"
#include "base/function_traits.h"
#include "base/hash.h"
#include "base/log.h"

namespace oj_rpc {

/*---------------------------------------------------------------------------------
rpc_server_t
---------------------------------------------------------------------------------*/
class rpc_server_t {
public:
    friend class rpc_t;
    //-----------------------------------------------------------------------------
    using interface_t      = std::function<channel_t::data_t(char const*, int)>;
    using interface_list_t = std::unordered_map<long, interface_t>;
    using route_t          = std::function<void(char const*, int)>;

private:
    //-----------------------------------------------------------------------------
    long                            m_guid;
    interface_list_t                m_invokers;
    inline static long              s_guidApp;
    inline static channel_t::post_t s_post;

public:
    //-----------------------------------------------------------------------------
    int close();
    //-----------------------------------------------------------------------------
    void invoker(channel_t::data_t& data, channel_t::post_t& channel) const;
    //-----------------------------------------------------------------------------
    bool push(int mask, const std::string& name, interface_t&& interface);
    //-----------------------------------------------------------------------------
    template <int type_ = int(head_code_t::REQUEST), typename t_fun>
    bool register_(t_fun&& fun)
    {
        using fun_traits_t     = function_traits<std::remove_reference_t<t_fun>>;
        using fun_args_tuple_t = typename fun_traits_t::args_tuple_type;
        constexpr int arg_n_t  = fun_traits_t::arg_size;
        //-----------------------------------------------------------------------------
        static_assert(arg_n_t == 1 || arg_n_t == 2, "args not match the fun!");
        //-----------------------------------------------------------------------------
        using req_t = typename fun_traits_t::template args<0>::type;
        //-----------------------------------------------------------------------------
        std::string name;
        if constexpr (arg_n_t == 2) {
            using rep_t = typename fun_traits_t::template args<1>::type;
            name        = logFormat("{}({},{})", type_, meta_hash_t::name_pretty<req_t>(), meta_hash_t::name_pretty<rep_t>());
        } else {
            name = logFormat("{}({})", type_, meta_hash_t::name_pretty<req_t>());
        }
        //-----------------------------------------------------------------------------
        auto invoker = [h = std::forward<t_fun>(fun)](char const* data, int size) -> channel_t::data_t {
            codec_t cp{};
            auto    argdata   = data + sizeof(header_t);
            auto    argsize   = int(size - sizeof(header_t));
            auto    argstuple = cp.unpack<fun_args_tuple_t>(argdata, argsize);
            auto    reqhead   = cp.unpack<header_t*>(data, sizeof(header_t));
            //-----------------------------------------------------------------------------
            std::apply(h, argstuple);
            //-----------------------------------------------------------------------------
            if (head_code_t::POST == reqhead->code) {
                return nullptr;
            }
            //-----------------------------------------------------------------------------
            auto headrep = header_t{};
            if constexpr (arg_n_t == 2) {
                auto& argRep = std::get<1>(argstuple);
                return codec_t::buffer_t(int(reqhead->code), headrep, argRep);

            } else {
                return codec_t::buffer_t(int(reqhead->code), headrep, "");
            }
        };
        //-----------------------------------------------------------------------------
        return push(type_, name, std::move(invoker));
    }
};

}  // namespace oj_rpc