#pragma once

#include "decl_struct.h"
#include "actor/actor.h"
#include "base/log.h"

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
class test_a_t : public oj_actor::actor_t {
public:
    //-----------------------------------------------------------------------------
    int  m_run   = 0;
    int  m_times = 0;

public:
    //-----------------------------------------------------------------------------
    // override actor_t;
    //-----------------------------------------------------------------------------
    virtual status_t onStartup(std::string mark)
    {
        //-----------------------------------------------------------------------------
        m_rpc.register_remote([this](test_a_req&) {
            std::cout << "I am a api!" << std::endl;

            oj_time::elapsed_t ep;
            test_b_req         req;
            call(req);
            logDebug("call test_b_req return! elapsed:{}(ms)", ep.milli());
        });

        return status_t::null;
    }
};