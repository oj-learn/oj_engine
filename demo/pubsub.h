#pragma once

#include "actor/actor.h"
#include "decl/decl.h"
#include "decl/pubsub.h"

//-----------------------------------------------------------------------------
// actor session_t
//-----------------------------------------------------------------------------
class pubsub_t : public oj_actor::actor_t {
public:
    //-----------------------------------------------------------------------------
    using subscribe_t = std::unordered_multimap<std::string, ip_actor_t>;

public:
    //-----------------------------------------------------------------------------
    subscribe_t m_subscribes;

protected:
    //-----------------------------------------------------------------------------
    // override actor_t;
    //-----------------------------------------------------------------------------
    virtual status_t onStartup(std::string mark) override;
    virtual status_t onHandler(channel_t::data_t& data) override;

private:
    //-----------------------------------------------------------------------------
    void subscribe(pubsub_subscribe_req&);
    void unSubscribe(pubsub_unsubscribe_req&);
    void publish(pubsub_publish_req&, pubsub_publish_rep&);
};
