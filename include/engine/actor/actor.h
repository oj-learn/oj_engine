#pragma once

#include "./../base/base.h"
#include "./../base/time.hpp"
#include "./../rpc/rpc.h"
#include <typeindex>


/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
inline const int32_t CLOSE_WAIT_SEC = 2;
class app_t;
namespace oj_actor {
/*---------------------------------------------------------------------------------
actor_t
---------------------------------------------------------------------------------*/
class actor_t : public std::enable_shared_from_this<actor_t> {
public:
    //-----------------------------------------------------------------------------
    friend app_t;
    //-----------------------------------------------------------------------------
    using sptr_t         = std::shared_ptr<actor_t>;
    using datas_t        = queue_rw_t<channel_t::data_t>;
    using data_write_t   = channel_t::post_t;
    using type_sptr_t    = std::unordered_map<std::type_index, sptr_t>;
    using data_handler_t = std::function<void(channel_t::data_t&)>;
    using data_bus_t     = std::unordered_map<int32_t, data_handler_t>;
    using timer_cb_t     = std::function<void()>;
    using data_rep_t     = channel_t::data_t;

protected:
    //-----------------------------------------------------------------------------
    int64_t                    m_guid;
    int32_t                    m_useCount;
    time_elapsed_t             m_life;
    std::string                m_name;
    std::atomic<status_t>      m_status;
    datas_t                    m_datas;
    data_write_t               m_channelWrite;
    type_sptr_t                m_ListCType;
    oj_rpc::rpc_t              m_rpc;
    data_bus_t                 m_dataBus;
    inline static int64_t      s_timeSec = 0;
    inline static int64_t      s_timeMs  = 0;
    inline static int64_t      s_guidApp = 0;
    inline static data_write_t s_chRpc;

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
    virtual int32_t  onSleep() { return 100; }
    virtual status_t onInit() { return status_t::null; }
    virtual status_t onStartup(std::string mark) { return status_t::null; }
    virtual status_t onInterruptcut() { return status_t::null; }
    virtual status_t onStop() { return status_t::null; }
    virtual status_t onTick() { return status_t::null; }
    virtual status_t onHandler(channel_t::data_t& data) { return status_t::unknow; }

public:
    //-----------------------------------------------------------------------------
    status_t          status();
    status_t          closeSet();
    int64_t           guid();
    int64_t           guidApp();
    int64_t           timesecTick();
    int64_t           timemsTick();
    std::string_view  nameGet();
    channel_t::post_t channelGet();
    status_t          channel(channel_t::data_t data);
    void              dataHandler(int32_t type, data_handler_t h);

public:
    //-----------------------------------------------------------------------------
    // rpc 服务, rpc开头;
    //-----------------------------------------------------------------------------
    oj_rpc::rpc_t& rpcGet();
    //-----------------------------------------------------------------------------
    template <typename t_req, typename... Args>
    int32_t request(t_req&& req, Args&&... args)
    {
        return m_rpc.request(m_channelWrite, std::forward<t_req>(req), std::forward<Args>(args)...);
    }
    //-----------------------------------------------------------------------------
    template <typename t_fun, typename t_req, typename... Args>
    int32_t call(t_fun fun, t_req&& req, Args&&... args)
    {
        return m_rpc.call(std::move(fun), std::forward<t_req>(req), std::forward<Args>(args)...);
    }

public:
    //-----------------------------------------------------------------------------
    //	定时器 服务, timer开头;
    //-----------------------------------------------------------------------------
    void timerOnce(int32_t delay, timer_cb_t&& cb);

private:
    //-----------------------------------------------------------------------------
    bool     statusCAS(status_t o, status_t n);
    status_t close();
    status_t init(int64_t guid);
    status_t initHandle();
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
