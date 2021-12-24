#pragma once

#include "decl_struct.h"
#include "actor/actor.h"

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
class test_c_t : public oj_actor::actor_t {
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
        m_rpc.register_remote([this](test_c_req& req, test_c_rep& rep) {
            rep.rstr = "who call me!";
            std::cout << "I am c api!" << std::endl;
        });
        //-----------------------------------------------------------------------------
        return status_t::null;
    }
};