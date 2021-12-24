#pragma once

#include "base/base.h"
#include "base/queue_write_read.h"
#include "base/time.hpp"
#include "rpc/rpc.h"
#include <typeindex>

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
inline const int CLOSE_WAIT_SEC = 2;

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
class app_t;
class session_t;

namespace oj_actor {
/*---------------------------------------------------------------------------------
actor_t
---------------------------------------------------------------------------------*/
class actor_t : public std::enable_shared_from_this<actor_t> {
public:
    //-----------------------------------------------------------------------------
    friend app_t;
    friend session_t;
    //-----------------------------------------------------------------------------
    using sptr_t         = std::shared_ptr<actor_t>;
    using datas_t        = queue_rw_t<channel_t::data_t>;
    using data_write_t   = channel_t::post_t;
    using type_sptr_t    = std::unordered_map<std::type_index, sptr_t>;
    using data_handler_t = std::function<void(channel_t::data_t&)>;
    using data_bus_t     = std::unordered_map<int, data_handler_t>;
    using timer_cb_t     = std::function<void()>;
    using data_rep_t     = channel_t::data_t;
    using wait_co_t      = std::shared_ptr<void>;
    using data_chan_t    = std::shared_ptr<void>;

private:
    //-----------------------------------------------------------------------------
    long                      m_guid;
    int                       m_useCount;
    int                       m_tickValue;
    std::string               m_name;
    datas_t                   m_datas;
    data_chan_t               m_dataChan;
    data_bus_t                m_dataBus;
    data_write_t              m_channelWrite;
    std::atomic<status_t>     m_status;
    wait_co_t                 m_waitCo;
    inline static long        s_timeSec;
    inline static long        s_timeMs;
    inline static long        s_guidApp;
    inline static int         s_port;
    inline static std::string s_ip;

protected:
    //-----------------------------------------------------------------------------
    oj_time::elapsed_t m_life;
    oj_rpc::rpc_t      m_rpc;

public:
    //-----------------------------------------------------------------------------
    actor_t();
    //-----------------------------------------------------------------------------
    virtual ~actor_t();
    //-----------------------------------------------------------------------------

protected:
    //-----------------------------------------------------------------------------
    //	virtual
    //-----------------------------------------------------------------------------
    virtual status_t onClose() { return status_t::close; }
    virtual status_t onInit() { return status_t::null; }
    virtual status_t onStartup(std::string mark) { return status_t::null; }
    virtual status_t onInterruptcut() { return status_t::null; }
    virtual status_t onStop() { return status_t::null; }
    virtual status_t onTick() { return status_t::null; }
    virtual status_t onHandler(channel_t::data_t& data) { return status_t::unknow; }

public:
    //-----------------------------------------------------------------------------
    status_t          status() { return m_status; }
    status_t          closeSet();
    long              guid() { return m_guid; }
    long              guidApp() { return s_guidApp; }
    long              timesecTick() { return s_timeSec; }
    long              timemsTick() { return s_timeMs; }
    void              tickValueSet(int v);
    int               tickValueGet() { return m_tickValue; }
    int               port() { return s_port; }
    std::string_view  ip() { return s_ip; }
    std::string_view  nameGet() { return m_name; }
    channel_t::post_t channelGet() { return m_channelWrite; }
    status_t          channel(channel_t::data_t data) { return (*m_channelWrite)(data); }
    oj_rpc::rpc_t&    rpcGet() { return m_rpc; }
    void              dataHandler(int type, data_handler_t h);

public:
    //-----------------------------------------------------------------------------
    // rpc 服务, rpc开头;
    //-----------------------------------------------------------------------------
    template <typename t_req, typename... Args>
    int request(t_req&& req, Args&&... args)
    {
        return m_rpc.request(m_channelWrite, std::forward<t_req>(req), std::forward<Args>(args)...);
    }
    //-----------------------------------------------------------------------------
    template <typename t_req, typename... Args>
    int call(t_req&& req, Args&&... args)
    {
        return m_rpc.call(std::forward<t_req>(req), std::forward<Args>(args)...);
    }

private:
    //-----------------------------------------------------------------------------
    void     data_pull();
    void     data_push(const channel_t::data_t& data);
    bool     statusCAS(status_t o, status_t n);
    status_t close();
    status_t init(long guid);
    status_t startup(std::string mark = "");
    status_t tick();
    status_t handler(channel_t::data_t& data);
};


}  // namespace oj_actor

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
using actor_sptr_t = oj_actor::actor_t::sptr_t;


// public:
//     /*---------------------------------------------------------------------------------

//     ---------------------------------------------------------------------------------*/
//     template <typename ComponentT>
//     decltype(auto) ComponentGet()
//     {
//         using t_sptr_t = std::shared_ptr<ComponentT>;

//         auto tindex = std::type_index(typeid(ComponentT));
//         auto exist  = m_ListCType.find(tindex);
//         if (exist == m_ListCType.end()) {
//             return t_sptr_t();
//         }

//         return std::dynamic_pointer_cast<ComponentT>(exist->second);
//     }

//     //-----------------------------------------------------------------------------
//     template <typename ComponentT, typename... TArgs>
//     auto ComponentAdd(TArgs&&... args) -> std::shared_ptr<ComponentT>
//     {
//         using t_sptr_t = std::shared_ptr<ComponentT>;

//         auto ptr = std::make_shared<ComponentT>(std::forward<TArgs>(args)...);
//         if (nullptr == ptr) {
//             logErrorReturn(ptr, logFormatVar(m_ListCType.size()));
//         }

//         auto tindex = std::type_index(typeid(ComponentT));
//         auto exist  = m_ListCType.find(tindex);
//         if (exist != m_ListCType.end()) {
//             logErrorReturn(ptr, logFormatVar(m_ListCType.size()));
//         }

//         m_ListCType.emplace(tindex, ptr);

//         return ptr;
//     }
