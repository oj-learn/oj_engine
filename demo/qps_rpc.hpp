#pragma once

#include "actor/actor.h"

//-----------------------------------------------------------------------------
// actor session_t
//-----------------------------------------------------------------------------
class qps_rpc_t : public oj_actor::actor_t {
public:
    //-----------------------------------------------------------------------------
    int64_t m_pause;
    int32_t m_run = 2;

public:
    //-----------------------------------------------------------------------------
    virtual status_t onClose() override
    {
        m_run = 0;
        return status_t::close;
    }
    //-----------------------------------------------------------------------------
    virtual status_t onStartup(std::string mark) override
    {
        //-----------------------------------------------------------------------------
        m_pause = timesecTick();
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
        switch (m_run) {
            case 1:
                this->test_rpcAsyncCall();
                break;
            case 2:
                this->test_rpcSyncCall();
                break;
            case 3:
                this->test_rpcSyncCall_local();
                break;
        }
        //-----------------------------------------------------------------------------
        return status_t::null;
    }

public:
    //-----------------------------------------------------------------------------
    int test_rpcAsyncCall()
    {
        if (timesecTick() - m_pause < 0) {
            return 0;
        }

        m_pause = timesecTick();

        return 0;
    }

    //-----------------------------------------------------------------------------
    int test_rpcSyncCall()
    {
        if (timesecTick() - m_pause < 0) {
            return 0;
        }

        m_pause = timesecTick();

        qps_rpc_sync_req req;
        call(qpsRpcSyncCall, req);

        qps_rpc_async_req req1;
        call(qpsRpcSyncCallRep, req1, []() {

        });

        return 0;
    }

    //-----------------------------------------------------------------------------
    int test_rpcSyncCall_local()
    {
        if (timesecTick() - m_pause < 0) {
            return 0;
        }

        m_pause = timesecTick();

        return 0;
    }
};
