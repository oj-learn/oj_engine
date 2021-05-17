#pragma once

#include "actor/actor.h"
#include "api/api.h"
#include <atomic>

void qpsRpcSyncCall(qps_rpc_sync_req&)
{
}

void qpsRpcSyncCallRep(qps_rpc_async_req&)
{
}


//-----------------------------------------------------------------------------
// actor session_t
//-----------------------------------------------------------------------------
class show_rpc_t : public oj_actor::actor_t {
public:
    //-----------------------------------------------------------------------------
      std::atomic_llong g_connNum  = 0;
      std::atomic_llong g_syncCall = 0, g_syncCallRep = 0;
      std::atomic_llong g_asyncCall = 0, g_asyncCallRep = 0;

public:
    /*---------------------------------------------------------------------------------
	override actor_t;
	---------------------------------------------------------------------------------*/
    virtual status_t onTick() override
    {
        //-----------------------------------------------------------------------------
        this->show_io();
        //-----------------------------------------------------------------------------
        return status_t::null;
    }
    //-----------------------------------------------------------------------------
    virtual status_t onStartup(std::string mark) override
    {
        //-----------------------------------------------------------------------------
        m_rpc.register_remote([this](qps_rpc_sync_req&) {
            g_syncCall++;
        });
        m_rpc.register_remote([this](qps_rpc_async_req&) {
            g_syncCallRep++;
        });
        //-----------------------------------------------------------------------------
        return status_t::null;
    }

public:
    //-----------------------------------------------------------------------------
    void incSyncCall()
    {
        g_syncCall++;
    }
    //-----------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    int show_io()
    {
        using namespace std::chrono;

        static auto pause = timesecTick();
        if (timesecTick() - pause < 1) {
            return 0;
        }

        pause = timesecTick();

        static int     show_title    = 0;
        static int64_t last_syncCall = 0, last_asyncCall = 0;
        static int64_t last_syncCallRep = 0, last_asyncCallRep = 0;
        static auto    start_time = system_clock::now();
        static auto    last_time  = system_clock::now();
        auto           now        = system_clock::now();

        if (show_title++ % 10 == 0) {
            int64_t s = g_syncCall, s1 = g_syncCallRep;
            int64_t r = g_asyncCall, r1 = g_asyncCallRep;
            int     timeDelta = int(duration_cast<milliseconds>(now - start_time).count());

            logDebug("syncCall:({},{}) asyncCall({},{}) timeDelta:{}", s, s1, r, r1, timeDelta);
            logDebug("{:^6} {:^13} {:^13} {:^15} {:^15}", "conn", "syncCall", "asyncCall", "qps AverageQps", "time_delta(ms)");
        }

        //printf("%-6d  %lld,%-9lld  %lld,%-9lld  %-7d  %-7d    %-7d\n",
        logDebug("{:^6} {:>6}-{:<6} {:>6}-{:<6} {:^15} {:^15}",
            (int32_t)g_connNum,
            (g_syncCall - last_syncCall),
            (g_syncCallRep - last_syncCallRep),
            (g_asyncCall - last_asyncCall),
            (g_asyncCallRep - last_asyncCallRep),
            (int)((double)(g_asyncCall - last_asyncCall) / 1024),
            (int)((double)(g_asyncCall / 1024 / std::max<int>(1, duration_cast<seconds>(now - start_time).count() + 1))),
            (int)duration_cast<milliseconds>(now - last_time).count());

        last_time         = now;
        last_syncCall     = g_syncCall;
        last_asyncCall    = g_asyncCall;
        last_syncCallRep  = g_syncCallRep;
        last_asyncCallRep = g_asyncCallRep;
        return 0;
    }
};
