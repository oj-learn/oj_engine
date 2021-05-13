#pragma once

#include "iguana/iguana/json.hpp"


/*---------------------------------------------------------------------------------
app config
---------------------------------------------------------------------------------*/
struct config_actor_t {
    int64_t     id;
    std::string make;
    std::string config;
};
REFLECTION(config_actor_t, id, make, config);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct config_app_json_t {
    int64_t                     id;
    std::string                 listen;
    std::string                 loglevel;
    std::vector<config_actor_t> actors;
};
REFLECTION(config_app_json_t, id, listen, loglevel, actors);

config_app_json_t configAppJsonGet();

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct config_connect_t {
    std::string ip;
    std::string port;
    int32_t     keep;
};
REFLECTION(config_connect_t, ip, port, keep);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct ip_actor_t {
    int64_t app;
    int64_t actor;
};
REFLECTION(ip_actor_t, app, actor);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct ip_api_t {
    int64_t    hash;
    int64_t    route;
    ip_actor_t ip;
};
REFLECTION(ip_api_t, route, hash, ip);
