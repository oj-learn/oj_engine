#pragma once

#include "base/base.h"
#include "base/codec.h"
#include "base/function_traits.h"
#include "base/log.h"
#include "base/queue_write_read.h"
#include <future>

namespace oj_rpc {


/*---------------------------------------------------------------------------------
task_t
---------------------------------------------------------------------------------*/
class task_t {
public:
    //-----------------------------------------------------------------------------
    using sptr       = std::shared_ptr<task_t>;
    using on_rep_t   = std::function<void(char const*, int32_t)>;
    using on_err_t   = std::function<void(exception_t&&)>;
    using wait_t     = std::promise<void>;
    using wait_ptr_t = std::shared_ptr<wait_t>;
    using wait_co_t  = std::shared_ptr<void>;
    using wait_ntf_t = std::condition_variable;
    using timer_t    = std::shared_ptr<void>;

public:
    //-----------------------------------------------------------------------------
    int64_t          m_id;
    bool             m_sync;
    int64_t          m_hash;
    zip_t            m_reqarg;
    on_rep_t         m_onRepOk;
    on_rep_t         m_onRepError;
    wait_ptr_t       m_wait;
    wait_co_t        m_waitCo;
    timer_t          m_timer;
    std::atomic_bool m_over;

public:
    //-----------------------------------------------------------------------------
    task_t(bool sync, int64_t hash);
    task_t(bool sync, std::string& name, zip_t&& reqarg);
    //-----------------------------------------------------------------------------
    virtual ~task_t();

public:
    //-----------------------------------------------------------------------------
    void    response(char const*, int32_t, head_code_t hc);
    void    error(head_code_t hc, std::string&& arg = "");
    void    cbErrorSet(on_err_t&& f);
    int32_t wait(int32_t ms);
    //-----------------------------------------------------------------------------
    template <typename t_cb>
    void cbOkSet(t_cb&& cb);
};

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename t_cb>
inline void task_t::cbOkSet(t_cb&& cb)
{
    this->m_onRepOk = [func = std::forward<t_cb>(cb)](char const* data, int32_t size) {
        using cb_traits_t = function_traits<std::remove_reference_t<t_cb>>;

        try {
            if constexpr (cb_traits_t::arg_size == 1) {
                using rep_arg_t = typename cb_traits_t::args<0>::type;

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

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
class task_mgr_t {
public:
    //-----------------------------------------------------------------------------
    friend class rpc_client_t;
    //-----------------------------------------------------------------------------
    using task_ptr_t = task_t::sptr;
    using task_map_t = std::unordered_map<int64_t, task_ptr_t>;

private:
    //-----------------------------------------------------------------------------
    int32_t    m_maxSize;
    task_map_t m_tasks;
    std::mutex m_lock;
    //-----------------------------------------------------------------------------
    inline static std::atomic_int64_t s_taskAutoId = 0;

public:
    //-----------------------------------------------------------------------------
    explicit task_mgr_t(int32_t max_size = 10240);

public:
    //-----------------------------------------------------------------------------
    bool empty() const;
    auto close() -> task_map_t;
    bool push(task_ptr_t& task);
    auto pull(int64_t task_id) -> task_ptr_t;
};

}  // namespace oj_rpc