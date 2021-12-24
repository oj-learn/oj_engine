#pragma once

#include "./task.h"
#include "base/function_traits.h"
#include "base/log.h"

namespace oj_rpc {

/*---------------------------------------------------------------------------------
rpc_client_t
---------------------------------------------------------------------------------*/
class rpc_client_t {
public:
    friend class rpc_t;
    //-----------------------------------------------------------------------------
    using task_ptr_t = task_t::sptr;

private:
    //-----------------------------------------------------------------------------
    long               m_guid;
    task_mgr_t         m_taskMgr;
    inline static long s_guidApp;

private:
    //-----------------------------------------------------------------------------
    auto size() -> int;
    auto close() -> int;
    //-----------------------------------------------------------------------------
    auto invoker(char const* data, int size) -> int;
    //-----------------------------------------------------------------------------
    auto wait(task_ptr_t task) -> int;
    //-----------------------------------------------------------------------------
    auto taskCancel(task_ptr_t task, head_code_t hc, std::string&& arg) -> int;
    //-----------------------------------------------------------------------------
    auto taskMake(channel_t::post_t& channel, bool sync, int type, std::string& name, zip_t&& reqarg) -> task_ptr_t;
    //-----------------------------------------------------------------------------
    auto taskMake(channel_t::post_t& channel, bool sync, long recvip, long recvport, channel_t::data_t data) -> task_ptr_t;
    //-----------------------------------------------------------------------------
    auto taskMake(channel_t::post_t& channel, int type, std::string& name, zip_t&& reqarg) -> int;
    //-----------------------------------------------------------------------------
    bool post(task_ptr_t task, channel_t::post_t& channel, channel_t::data_t& data);
    //-----------------------------------------------------------------------------
    template <int type_, typename t_req>
    int post(channel_t::post_t& channel, t_req&& req)
    {
        using req_t            = std::remove_reference_t<t_req>;
        std::string name       = logFormat("{}({})", type_, meta_hash_t::name_pretty<req_t>());
        auto        argsBuffer = codec_.packArgs(std::forward<t_req>(req));
        //-----------------------------------------------------------------------------
        return this->taskMake(channel, type_, name, std::move(argsBuffer));
    }
    //-----------------------------------------------------------------------------
    template <bool sync, int type_, typename t_cb, typename... Args>
    int request(channel_t::post_t& channel, t_cb&& cb, Args&&... args)
    {
        using args_tuple_t = std::tuple<std::remove_reference_t<Args>...>;
        using req_t        = typename std::tuple_element_t<0, args_tuple_t>;
        using cb_traits_t  = function_traits<std::remove_reference_t<t_cb>>;
        //-----------------------------------------------------------------------------
        std::string name;
        if constexpr (cb_traits_t::arg_size == 1) {
            using rep_t = typename cb_traits_t::template args<0>::type;
            name        = logFormat("{}({},{})", type_, meta_hash_t::name_pretty<req_t>(), meta_hash_t::name_pretty<rep_t>());
        } else {
            name = logFormat("{}({})", type_, meta_hash_t::name_pretty<req_t>());
        }

        auto argsBuffer = codec_.packArgs(std::forward<Args>(args)...);
        auto task       = this->taskMake(channel, sync, type_, name, std::move(argsBuffer));

        if (nullptr == task) {
            return 1;
        }

        task->cbOkSet(std::move(cb));
        task->cbErrorSet([](exception_t&& e) {});

        return this->wait(task);
    }
    //-----------------------------------------------------------------------------
    int callAsync(bool sync, channel_t::post_t& channel, long recvip, long recvport, channel_t::data_t data, std::function<void(channel_t::data_t)>&& cb);
    //-----------------------------------------------------------------------------
};

}  // namespace oj_rpc