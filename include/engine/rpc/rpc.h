#pragma once

#include "./client/client.h"
#include "./server/server.h"

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
class rpc_t;
namespace oj_rpc {

/*---------------------------------------------------------------------------------
rpc_t
待解决，
1:需要支持传入options
2:１个调用的结束应该包括完整的调用链的结束，

---------------------------------------------------------------------------------*/
class rpc_t {
public:
    friend ::rpc_t;
    //-----------------------------------------------------------------------------
    using write_t = channel_t::post_t;

private:
    //-----------------------------------------------------------------------------
    rpc_server_t          m_server;
    rpc_client_t          m_client;
    inline static write_t s_post;
    static void           init_s(write_t w, long guidApp);

public:
    //-----------------------------------------------------------------------------
    void init(long guid);
    int  size();
    int  close();
    void invoker(channel_t::data_t data);
    void response(channel_t::data_t data);

public:
    //-----------------------------------------------------------------------------
    template <fun_trait_able t_fun>
    bool register_local(t_fun fun)
    {
        return m_server.register_(std::move(fun));
    }
    //-----------------------------------------------------------------------------
    template <fun_trait_able t_fun>
    bool register_remote(t_fun fun)
    {
        return m_server.register_<int(head_code_t::RPC_ROUTE)>(std::move(fun));
    }
    //-----------------------------------------------------------------------------
    template <int type_ = int(head_code_t::REQUEST), typename t_req, typename... Args>
    int request(write_t w, t_req&& req, Args&&... args)
    {
        using args_tuple_t    = std::tuple<Args...>;
        constexpr int arg_n_t = std::tuple_size_v<args_tuple_t>;
        static_assert(arg_n_t <= 1, "too mutch arg!");
        //-----------------------------------------------------------------------------
        nullptr == w ? w = s_post : nullptr;
        //-----------------------------------------------------------------------------
        try {
            if constexpr (arg_n_t == 1) {
                using arg_t    = std::tuple_element_t<0, args_tuple_t>;
                auto argsTuple = std::tie(args...);

                if constexpr (function_traits<arg_t>::callable == 1) {
                    using cb_traits_t     = function_traits<std::remove_reference_t<arg_t>>;
                    using cb_args_tuple_t = typename cb_traits_t::raw_tuple_type;
                    static_assert(cb_traits_t::arg_size <= 1, "cb arg most!");

                    auto& cb = std::get<0>(argsTuple);
                    if constexpr (cb_traits_t::arg_size == 1) {
                        //异步调用，有返回值
                        using rep_t = typename cb_traits_t::template args<0>::type;
                        rep_t rep;  //这个rep没什么用，而且这是异步调用，这是局部变量，会失效的．．只是为了传过去能调用．
                        return m_client.request<false, type_>(w, std::move(cb), std::forward<t_req>(req), rep);

                    } else {
                        //异步调用，无返回值
                        return m_client.request<false, type_>(w, std::move(cb), std::forward<t_req>(req));
                    }

                } else {
                    //同步调用，有返回值
                    auto& rep = std::get<0>(argsTuple);
                    auto  cb  = [&rep](arg_t& ret) {
                        rep = ret;
                    };

                    return m_client.request<true, type_>(w, std::move(cb), std::forward<t_req>(req), rep);
                }
            }

            if constexpr (arg_n_t == 0) {
                //同步调用，无返回值
                auto cb = []() {
                };
                return m_client.request<true, type_>(w, std::move(cb), std::forward<t_req>(req));
            }

        } catch (std::exception& e) {
            logError("request std::exception e:{} ", e.what());
        }

        return 1;
    }
    //-----------------------------------------------------------------------------
    template <typename t_req, typename... Args>
    int call(t_req&& req, Args&&... args)
    {
        return this->request<int(head_code_t::RPC_ROUTE)>(s_post, std::forward<t_req>(req), std::forward<Args>(args)...);
    }
    //-----------------------------------------------------------------------------
    template <typename Arg>
    int package(long guidApp, long guid, channel_t::data_t req, Arg&& arg)
    {
        using cb_traits_t = function_traits<std::remove_reference_t<Arg>>;
        if constexpr (cb_traits_t::callable == 1) {
            static_assert(cb_traits_t::arg_size == 1, "cb arg need 1!");

            using cb_rep_t = typename cb_traits_t::template args<0>::type;
            static_assert(std::is_same_v<channel_t::data_t, cb_rep_t>, "arg need data_t!!");

            return m_client.callAsync(false, s_post, guidApp, guid, req, std::move(arg));

        } else {
            static_assert(std::is_same_v<channel_t::data_t, std::remove_reference_t<Arg>>, "arg need data_t!!");

            return m_client.callAsync(true, s_post, guidApp, guid, req, [&arg](channel_t::data_t data) {
                arg = data;
            });
        }
    }
    //-----------------------------------------------------------------------------
    template <typename t_req>
    int post(t_req&& req)
    {
        return m_client.post<int(head_code_t::RPC_ROUTE)>(s_post, std::forward<t_req>(req));
    }
    //-----------------------------------------------------------------------------
    // template <fun_trait_able t_fun, typename t_req, typename... Args>
    // [[deprecated("xxx")]] int call(t_fun fun, t_req&& req, Args&&... args)
    // {
    //     using fun_traits_t    = function_traits<std::remove_reference_t<t_fun>>;
    //     using args_tuple_t    = std::tuple<std::remove_reference_t<Args>...>;
    //     constexpr int arg_n_t = std::tuple_size_v<args_tuple_t>;
    //     //-----------------------------------------------------------------------------
    //     static_assert(fun_traits_t::arg_size == 1 || fun_traits_t::arg_size == 2, "fun arg need 1 or 2 arg!");

    //     if constexpr (fun_traits_t::arg_size == 1) {
    //         if constexpr (arg_n_t == 1) {
    //             using arg_t       = std::tuple_element_t<0, args_tuple_t>;
    //             using cb_traits_t = function_traits<std::remove_reference_t<arg_t>>;
    //             static_assert(cb_traits_t::callable == 1, "arg need cb!");
    //         }

    //     } else {
    //         static_assert(arg_n_t == 1, "need 1 arg!");
    //         using fn_rep_t    = typename fun_traits_t::template args<1>::type;
    //         using arg_t       = std::tuple_element_t<0, args_tuple_t>;
    //         using cb_traits_t = function_traits<std::remove_reference_t<arg_t>>;

    //         if constexpr (cb_traits_t::callable == 1) {
    //             static_assert(cb_traits_t::arg_size == 1, "cb need 1 arg!");

    //             using cb_rep_t = typename cb_traits_t::template args<0>::type;
    //             static_assert(std::is_same_v<cb_rep_t, fn_rep_t>, "cb arg and fun arg not match!");

    //         } else {
    //             static_assert(std::is_same_v<arg_t, fn_rep_t>, "arg and fun arg not match!");
    //         }
    //     }

    //     return this->request<int(head_code_t::RPC_ROUTE)>(s_post, std::forward<t_req>(req), std::forward<Args>(args)...);
    // }
};

}  // namespace oj_rpc
