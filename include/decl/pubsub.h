#pragma once

#include "iguana/iguana/json.hpp"

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct pubsub_subscribe_req {
    std::string key;
    ip_actor_t  ip;
};
REFLECTION(pubsub_subscribe_req, key, ip)

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct pubsub_unsubscribe_req {
    std::string key;
    ip_actor_t  ip;
};
REFLECTION(pubsub_unsubscribe_req, key, ip)

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct pubsub_publish_req {
    std::string key;
    zip_t       zip;
};
REFLECTION(pubsub_publish_req, key, zip)

struct pubsub_publish_rep {
    std::string key;
    int32_t     size;
};
REFLECTION(pubsub_publish_rep, key, size)
