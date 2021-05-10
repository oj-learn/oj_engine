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
    int64_t               m_guid;
    task_mgr_t            m_taskMgr;
    inline static int64_t s_guidApp;

private:
    //-----------------------------------------------------------------------------
    auto size() -> int32_t;
    auto close() -> int32_t;
    //-----------------------------------------------------------------------------
    auto invoker(char const* data, int32_t size) -> int32_t;
    //-----------------------------------------------------------------------------
    auto wait(task_ptr_t task, int32_t ms) -> int32_t;
    //-----------------------------------------------------------------------------
    auto taskCancel(task_ptr_t task, head_code_t hc, std::string&& arg) -> int32_t;
    //-----------------------------------------------------------------------------
    auto taskMake(channel_t::post_t& channel, bool sync, int32_t type, std::string& name, zip_t&& reqarg) -> task_ptr_t;
    //-----------------------------------------------------------------------------
    auto taskMake(channel_t::post_t& channel, bool sync, int64_t recvip, int64_t recvport, channel_t::data_t data) -> task_ptr_t;
    //-----------------------------------------------------------------------------
    bool post(task_ptr_t task, channel_t::post_t& channel, channel_t::data_t& data);
    //-----------------------------------------------------------------------------
    template <bool sync, int32_t type_, typename t_cb, typename... Args>
    int32_t request(channel_t::post_t& channel, t_cb&& cb, Args&&... args)
    {
        using args_tuple_t = std::tuple<std::remove_reference_t<Args>...>;
        using req_t        = typename std::tuple_element_t<0, args_tuple_t>;
        using cb_traits_t  = function_traits<std::remove_reference_t<t_cb>>;
        //-----------------------------------------------------------------------------
        std::string name = logFormat("{} {}", type_, typeid(req_t).name());
        if constexpr (cb_traits_t::arg_size == 1) {
            using rep_t = typename cb_traits_t::args<0>::type;
            name += typeid(rep_t).name();
        }

        auto argsBuffer = codec_.packArgs(std::forward<Args>(args)...);
        auto task       = this->taskMake(channel, sync, type_, name, std::move(argsBuffer));

        if (nullptr == task) {
            return 1;
        }

        task->cbOkSet(std::move(cb));
        task->cbErrorSet([](exception_t&& e) {});

        return this->wait(task, 3000);
    }
    //-----------------------------------------------------------------------------
    int32_t callAsync(bool                       sync,
        channel_t::post_t&                       channel,
        int64_t                                  recvip,
        int64_t                                  recvport,
        channel_t::data_t                        data,
        std::function<void(channel_t::data_t)>&& cb);
    //-----------------------------------------------------------------------------
};

}  // namespace oj_rpc