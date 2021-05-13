#pragma once

#include "./config.h"
#include "iguana/iguana/json.hpp"


/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct app_startup_req {
    int64_t appid;
};
REFLECTION(app_startup_req, appid);

struct app_startup_rep {
    int64_t     appid;
    std::string reason;
};
REFLECTION(app_startup_rep, appid, reason);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct app_close_req {
    int64_t appid;
};
REFLECTION(app_close_req, appid);

struct app_close_rep {
    int64_t     appid;
    std::string reason;
};
REFLECTION(app_close_rep, appid, reason);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct app_actor_make_req {
    int64_t        appid;
    config_actor_t actor;
};
REFLECTION(app_actor_make_req, appid, actor);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct app_actor_close_req {
    int64_t     appid;
    int64_t     id;
    std::string name;
};
REFLECTION(app_actor_close_req, appid, id, name);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct app_loglevel_req {
    int64_t     appid;
    std::string level;
};
REFLECTION(app_loglevel_req, appid, level);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct parent_connect_ntf {
    int64_t appid;
};
REFLECTION(parent_connect_ntf, appid);

struct parent_disconnect_ntf {
    int64_t appid;
};
REFLECTION(parent_disconnect_ntf, appid);


/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct qps_rpc_sync_req {
    int64_t appid;
};
REFLECTION(qps_rpc_sync_req, appid);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct qps_rpc_async_req {
    int64_t appid;
};
REFLECTION(qps_rpc_async_req, appid);