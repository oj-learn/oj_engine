#pragma once

#include "actor/actor.h"
#include "api/api.h"

/*---------------------------------------------------------------------------------
	actor daemon_t 
---------------------------------------------------------------------------------*/
class daemon_t : public oj_actor::actor_t {
public:
    //-----------------------------------------------------------------------------
    using daemon_session_t = std::unordered_map<int64_t, actor_sptr_t>;

private:
    //-----------------------------------------------------------------------------
    std::string      m_scriptPath;
    daemon_session_t m_sessionApp;

public:
    //-----------------------------------------------------------------------------
    // override actor_t;
    //-----------------------------------------------------------------------------
    virtual status_t onClose() override;
    virtual status_t onStartup(std::string mark) override;
    virtual status_t onHandler(channel_t::data_t& data) override;

public:
    //-----------------------------------------------------------------------------
    void appStartup(app_startup_req&, app_startup_rep&);
    void appClose(app_close_req&, app_close_rep&);
    void appActorMake(app_actor_make_req&);
    void appActorClose(app_actor_close_req&);
    void appLoglevel(app_loglevel_req&);
};