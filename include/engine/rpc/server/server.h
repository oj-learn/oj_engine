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
    using interface_t      = std::function<channel_t::data_t(char const*, int32_t)>;
    using interface_list_t = std::unordered_map<int64_t, interface_t>;
    using route_t          = std::function<void(char const*, int32_t)>;

private:
    //-----------------------------------------------------------------------------
    int64_t                         m_guid;
    interface_list_t                m_invokers;
    inline static int64_t           s_guidApp;
    inline static channel_t::post_t s_post;

public:
    //-----------------------------------------------------------------------------
    int32_t close();
    //-----------------------------------------------------------------------------
    void invoker(channel_t::data_t& data, channel_t::post_t& channel) const;
    //-----------------------------------------------------------------------------
    bool push(int32_t mask, const std::string name, interface_t&& interface);
    //-----------------------------------------------------------------------------
    template <int32_t type_ = int32_t(head_code_t::REQUEST), typename t_fun>
    bool register_(t_fun&& fun)
    {
        using fun_traits_t     = function_traits<std::remove_reference_t<t_fun>>;
        using fun_args_tuple_t = typename fun_traits_t::args_tuple_type;
        constexpr int arg_n_t  = fun_traits_t::arg_size;
        //-----------------------------------------------------------------------------
        static_assert(arg_n_t == 1 || arg_n_t == 2, "args not match the fun!");
        //-----------------------------------------------------------------------------
        auto invoker = [h = std::forward<t_fun>(fun)](char const* data, int32_t size) -> channel_t::data_t {
            codec_t cp{};
            auto    argdata   = data + sizeof(head_req_t);
            auto    argsize   = int32_t(size - sizeof(head_req_t));
            auto    argstuple = cp.unpack<fun_args_tuple_t>(argdata, argsize);
            auto    reqhead   = cp.unpack<head_req_t*>(data, sizeof(head_req_t));
            //-----------------------------------------------------------------------------
            std::apply(h, argstuple);
            //-----------------------------------------------------------------------------
            auto headrep = head_rep_t{};
            //-----------------------------------------------------------------------------
            if constexpr (arg_n_t == 2) {
                auto& argRep = std::get<1>(argstuple);
                return codec_t::buffer_t(int32_t(reqhead->code), headrep, argRep);

            } else {
                return codec_t::buffer_t(int32_t(reqhead->code), headrep, "");
            }
        };
        //-----------------------------------------------------------------------------
        using req_t      = typename fun_traits_t::args<0>::type;
        std::string name = logFormat("{} {}", type_, typeid(req_t).name());
        if constexpr (arg_n_t == 2) {
            using rep_t = typename fun_traits_t::args<1>::type;
            name += typeid(rep_t).name();
        }
        return push(type_, name, std::move(invoker));
    }
};

}  // namespace oj_rpc