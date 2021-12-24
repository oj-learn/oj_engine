#pragma once


#include "actor/actor.h"
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// actor session_t
//-----------------------------------------------------------------------------
class session_t : public oj_actor::actor_t {
public:
    friend class tcp_t;
    //-----------------------------------------------------------------------------

protected:
    //-----------------------------------------------------------------------------
    std::string m_ip          = "";
    std::string m_mark        = "";
    int         m_keep        = 0;
    int         m_try         = 0;
    long        m_waitConnect = -1;
    long        m_peer        = 0;

public:
    //-----------------------------------------------------------------------------
    void connectSet(std::string ip, std::string mark, int keep);

protected:
    //-----------------------------------------------------------------------------
    // override actor_t;
    //-----------------------------------------------------------------------------
    virtual status_t onTick() override final;
    virtual status_t onStop() override final;
    virtual status_t onInit() override final;

protected:
    //-----------------------------------------------------------------------------
    // virtual;
    //-----------------------------------------------------------------------------
    virtual status_t onConnect() { return status_t::null; }
    virtual status_t onDisconnect() { return status_t::null; }
    virtual status_t onRefuse() { return status_t::null; }
    virtual status_t onUpdate() { return status_t::null; }

private:
    //-----------------------------------------------------------------------------
    status_t connect(long peer, channel_t::w_t write);
    status_t disconnect();
    status_t refuse();

public:
    //-----------------------------------------------------------------------------
    status_t write(channel_t::data_t data);
    long     peerGet();
};
