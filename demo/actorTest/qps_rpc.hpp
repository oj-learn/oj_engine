#pragma once

#include "actor/actor.h"
#include "base/log.h"

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
class qps_rpc_t : public oj_actor::actor_t {
public:
    //-----------------------------------------------------------------------------
    long m_pause;
    int  m_run   = 0;
    int  m_times = 0;

public:
    //-----------------------------------------------------------------------------
    virtual status_t onInterruptcut() override
    {
        m_run = 0;
        return status_t::null;
    }
    //-----------------------------------------------------------------------------
    virtual status_t onStartup(std::string config) override
    {
        this->tickValueSet(10000);
        //-----------------------------------------------------------------------------
        auto parms = oj_string::casts(config);
        m_run      = parms.size() > 0 ? parms[0] : 0;
        m_times    = parms.size() > 1 ? parms[1] : 20000;
        //-----------------------------------------------------------------------------
        m_pause = oj_time::timestamp_ms();
        //-----------------------------------------------------------------------------
        return status_t::null;
    }
    //-----------------------------------------------------------------------------
    virtual status_t onTick() override
    {
        if (m_run <= 0) {
            return status_t::null;
        }
        //-----------------------------------------------------------------------------
        if (1 == m_run){
            this->test_rpcAsyncCall();
        }
        if (2 == m_run){
            this->test_rpcSyncCall();
        }
        //-----------------------------------------------------------------------------
        return status_t::null;
    }

public:
    //-----------------------------------------------------------------------------
    int test_rpcAsyncCall()
    {
        // if (oj_time::timestamp_ms() - m_pause < 0) {
        //     return 0;
        // }

        m_pause = oj_time::timestamp_ms();
        static int                s_asyncRep;
        static oj_time::elapsed_t s_asyncEp;
        s_asyncRep = 0;
        s_asyncEp.reset();

        oj_time::elapsed_t ep;
        //-----------------------------------------------------------------------------
        for (auto i = m_times; i > 0; --i) {
            qps_rpc_async_req req1;
            call(req1, [this]() {
                ++s_asyncRep;

                if (s_asyncRep == m_times) {
                    // 计算连所有rep都返回消耗时间
                    logDebug("qps_rpc_t async call rep:{}, elapsed:{}(ms)", s_asyncRep, s_asyncEp.milli());
                }
            });
        }
        //-----------------------------------------------------------------------------
        // 计算连续调 s_times 次消耗时间
        logDebug("qps_rpc_t async　调用了 {} 次, elapsed:{}(ms)", m_times, ep.milli());
        return 0;
    }

    //-----------------------------------------------------------------------------
    int test_rpcSyncCall()
    {
        // if (oj_time::timestamp_ms() - m_pause < 0) {
        //     return 0;
        // }

        m_pause = oj_time::timestamp_ms();

        oj_time::elapsed_t ep;
        //-----------------------------------------------------------------------------
        for (auto i = m_times; i > 0; --i) {
            qps_rpc_sync_req req;
            call(req);
        }
        //-----------------------------------------------------------------------------
        logDebug("qps_rpc_t sync　调用了 {} 次, elapsed:{}(ms)", m_times, ep.milli());

        return 0;
    }
};
