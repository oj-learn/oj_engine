#pragma once

#include "decl_struct.h"
#include "actor/actor.h"
#include "base/log.h"

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
class test_b_t : public oj_actor::actor_t {
public:
    //-----------------------------------------------------------------------------
    // override actor_t;
    //-----------------------------------------------------------------------------
    virtual status_t onStartup(std::string mark)
    {
        m_rpc.register_remote([this](test_b_req&) {
            std::cout << "I am b api!" << std::endl;

            oj_time::elapsed_t ep;
            test_c_req         req;
            test_c_rep         rep;
            call(req, rep);
            logDebug("call test_c_req return! rstr:{} elapsed:{}(ms)", rep.rstr, ep.milli());
        });
        //-----------------------------------------------------------------------------
        return status_t::null;
    }
};