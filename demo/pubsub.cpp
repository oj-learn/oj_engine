#include "./pubsub.h"
#include "api/pubsub.h"
#include "base/codec.h"

/***************************************  ***************************************/

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
status_t pubsub_t::onStartup(std::string mark)
{
    //-----------------------------------------------------------------------------
    m_rpc.register_remote([this](pubsub_subscribe_req& req) {
        this->subscribe(req);
    });
    //-----------------------------------------------------------------------------
    m_rpc.register_remote([this](pubsub_publish_req& req, pubsub_publish_rep& rep) {
        this->publish(req, rep);
    });
    //-----------------------------------------------------------------------------
    return status_t::null;
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
status_t pubsub_t::onHandler(channel_t::data_t& data)
{
    if (0 == data->type()) {
        auto req = codec_.unpack<pubsub_unsubscribe_req>(data->data(), data->size());
        this->unSubscribe(req);
        return status_t::null;
    }

    //-----------------------------------------------------------------------------
    return status_t::unknow;
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void pubsub_t::subscribe(pubsub_subscribe_req& req)
{
    auto range = m_subscribes.equal_range(req.key);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second.app != req.ip.app) {
            continue;
        }

        if (it->second.actor != req.ip.actor) {
            continue;
        }

        return;  //已经存在了
    }

    m_subscribes.emplace(req.key, req.ip);
    logDebug("pubsub_t::subscribe key:{}, app:{}, actor:{}", req.key, req.ip.app, req.ip.actor);
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void pubsub_t::unSubscribe(pubsub_unsubscribe_req& req)
{
    auto range = m_subscribes.equal_range(req.key);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second.app != req.ip.app) {
            continue;
        }

        if (it->second.actor != req.ip.actor) {
            continue;
        }

        m_subscribes.erase(it);
        logDebug("pubsub_t::unSubscribe key:{}, app:{}, actor:{}", req.key, req.ip.app, req.ip.actor);
        return;
    }
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void pubsub_t::publish(pubsub_publish_req& req, pubsub_publish_rep& rep)
{
    auto              range   = m_subscribes.equal_range(req.key);
    channel_t::data_t dataDup = nullptr;

    rep.key  = req.key;
    rep.size = m_subscribes.count(req.key);

    for (auto it = range.first; it != range.second; ++it) {
        // if (nullptr == dataDup) {
        //     auto data = codec_.unpack<channel_t::data_t>(req.zip);
        //     dataDup   = codec_.buffer_t(int32_t(head_code_t::PKG), it->second.app, it->second.actor, data);

        // } else {
        //     dataDup           = buffer_t::create(dataDup);
        //     auto headreq      = codec_.unpack<head_req_t*>(dataDup);
        //     headreq->recvip   = it->second.app;
        //     headreq->recvport = it->second.actor;
        // }

        //dispatch(it->second.app, it->second.actor, dataDup);


        dataDup = (nullptr == dataDup)
            ? codec_.unpack<channel_t::data_t>(req.zip)
            : buffer_t::create(dataDup);

        m_rpc.package(it->second.app, it->second.actor, dataDup, [key = req.key, ip = it->second, this](channel_t::data_t data) {
            auto status = (nullptr != data ? *((int32_t*)(data->data())) : 1);

            if (int32_t(status_t::null) != status) {
                pubsub_unsubscribe_req req{ key, ip };
                this->channel(codec_t::buffer_t(0, req));
            }
        });
    }
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void api_pubsub_subscribe(pubsub_subscribe_req& req)
{
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void api_pubsub_publish(pubsub_publish_req& req, pubsub_publish_rep& rep)
{
}