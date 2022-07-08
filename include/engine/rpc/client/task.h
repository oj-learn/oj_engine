#pragma once

#include "base/base.h"
#include "base/codec.h"
#include "base/function_traits.h"
#include "base/log.h"
#include <future>

namespace oj_rpc {
/*---------------------------------------------------------------------------------
task_t
---------------------------------------------------------------------------------*/
class task_t final {
public:
    //-----------------------------------------------------------------------------
    using sptr       = std::shared_ptr<task_t>;
    using on_rep_t   = std::function<void(char const*, int)>;
    using on_err_t   = std::function<void(exception_t&&)>;
    using wait_t     = std::promise<void>;
    using wait_ptr_t = std::shared_ptr<wait_t>;
    using wait_ntf_t = std::condition_variable;
    using sm_void_t  = std::shared_ptr<void>;

public:
    //-----------------------------------------------------------------------------
    long             m_id;
    bool             m_sync;
    long             m_hash;
    on_rep_t         m_onRepOk;
    on_rep_t         m_onRepError;
    wait_ptr_t       m_wait;
    sm_void_t        m_wait_;
    sm_void_t        m_timer;
    std::atomic_bool m_over;

public:
    //-----------------------------------------------------------------------------
    task_t(bool sync, long hash);
    //-----------------------------------------------------------------------------
    ~task_t();

public:
    //-----------------------------------------------------------------------------
    void response(char const*, int, head_code_t hc);
    void error(head_code_t hc, std::string&& arg = "");
    void cbErrorSet(on_err_t&& f);
    int  wait(int ms);
    //-----------------------------------------------------------------------------
    template <typename t_cb>
    void cbOkSet(t_cb&& cb)
    {
        this->m_onRepOk = [func = std::forward<t_cb>(cb)](char const* data, int size) {
            using cb_traits_t = function_traits<std::remove_reference_t<t_cb>>;

            try {
                if constexpr (cb_traits_t::arg_size == 1) {
                    using rep_arg_t = typename cb_traits_t::template args<0>::type;

                    auto result = codec_.unpack<rep_arg_t>(data, size);

                    func(result);

                } else {
                    func();
                }

            } catch (exception_t& e) {
                logError("task_t RepOk  exception_t:{}", e.error_message_);
            }
        };
    }
};

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
class task_mgr_t {
public:
    //-----------------------------------------------------------------------------
    friend class rpc_client_t;
    //-----------------------------------------------------------------------------
    using task_ptr_t = task_t::sptr;
    using task_map_t = std::unordered_map<long, task_ptr_t>;

private:
    //-----------------------------------------------------------------------------
    unsigned   m_maxSize;
    task_map_t m_tasks;
    std::mutex m_lock;
    //-----------------------------------------------------------------------------
    inline static std::atomic_int64_t s_taskAutoId = 0;

public:
    //-----------------------------------------------------------------------------
    explicit task_mgr_t(int max_size = 1024000);

public:
    //-----------------------------------------------------------------------------
    bool empty() const;
    auto close() -> task_map_t;
    long idGet();
    bool push(task_ptr_t& task);
    auto pull(long task_id) -> task_ptr_t;
};

}  // namespace oj_rpc