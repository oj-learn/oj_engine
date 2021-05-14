#include "./session_daemon.h"
#include "adapter/adapter.h"
#include "base/codec.h"
#include "decl/config.h"


/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
status_t session_daemon_t::onStartup(std::string mark)
{
    session_t::onStartup(mark);
    //-----------------------------------------------------------------------------
    if ("" != mark) {
        config_connect_t config;
        if (!codec_.fromJson(config, mark)) {
            return status_t::make;
        }

        this->connectSet(config.ip + ":" + config.port, "session_daemon", config.keep);
    }
    //-----------------------------------------------------------------------------
    return status_t::null;
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
status_t session_daemon_t::onConnect()
{
    return status_t::null;
}