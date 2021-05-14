#include "./daemon.h"
#include "adapter/adapter.h"
#include "app/app.h"
#include "base/file.h"
#include "decl/config.h"


/***************************************  ***************************************/
const char* DIR_DEPLOYMENT = "./deployment/config";
/***************************************  ***************************************/
//[group:app]
//programs={}

const auto SUPERVISOR_CONF = R"(
[program:app-{}]
command={}/demo
directory={}/
user=jiangjun
startsecs=1
startretries=1
autostart=false
autorestart=false
exitcodes=0
stopsignal=INT
stopwaitsecs=10
redirect_stderr=true
stdout_logfile={}/logs/nohup.log
stdout_logfile_maxbytes=50MB
stdout_logfile_backups=10
)";
/***************************************  ***************************************/


/*---------------------------------------------------------------------------------
函数说明：调用fork产生子进程，从子进程中调用/bin/sh -c来执行参数command的指令。

command参数：要执行的shell命令字符串
type参数：只能是读或者写中的一种。如果 type 为 r，那么调用进程读进 command 的标准输出。如果 type 为 w，那么调用进程写到 command 的标准输入
————————————————
popen创建的管道，默认读取的是标准输出stdout，但很多程序（如ffmpeg相关的）输出到stderr上，为了能方便地读取这些输出，需要在执行命令时，对输出做重定向（<cmd> 2>&1）
---------------------------------------------------------------------------------*/
int shell__(std::string strCmd, std::function<void(std::string)> funOut)
{
    FILE* fp         = NULL;
    char  data[1024] = { 0 };
    //std::string buffer(1024, '0');

    strCmd += " 2>&1";
    fp = popen(strCmd.c_str(), "r");
    if (NULL == fp) {
        return -1;
    }
    while (fgets(data, sizeof(data), fp) != NULL) {
        if (NULL != funOut) {
            funOut(data);
        }
    }

    pclose(fp);

    return (('o' == data[0] && 'k' == data[1]) ? 0 : 1);
}


/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
template <typename T>
bool write_config(T&& t, std::string name)
{
    std::string value;
    if (!codec_.toJson(t, value)) {
        return false;
    }

    std::string file = logFormat("{}/{}.json", DIR_DEPLOYMENT, name);
    return oj_file::fileWrite(file, value);
};

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
bool write_supervisor(std::string name, std::string& value)
{
    std::string file = logFormat("{}/app-{}.conf", DIR_DEPLOYMENT, name);
    return oj_file::fileWrite(file, value);
};

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
status_t daemon_t::onClose()
{
    std::vector<int64_t> v;
    v.reserve(m_sessionApp.size());

    for (auto [appid, _] : m_sessionApp) {
        v.push_back(appid);
    }

    for (auto appid : v) {
        app_close_rep rep;
        app_close_req req{ appid };
        this->appClose(req, rep);
    }

    return m_sessionApp.size() > 0 ? status_t::wait : status_t::close;
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
status_t daemon_t::onStartup(std::string mark)
{
    m_scriptPath = mark;
    //-----------------------------------------------------------------------------
    m_rpc.register_remote([this](app_startup_req& req, app_startup_rep& rep) {
        this->appStartup(req, rep);
    });
    //-----------------------------------------------------------------------------
    m_rpc.register_remote([this](app_close_req& req, app_close_rep& rep) {
        if (-1 == req.appid) {
            this->onClose();

            rep.appid = req.appid;

        } else {
            this->appClose(req, rep);
        }
    });
    //-----------------------------------------------------------------------------
    m_rpc.register_remote([this](app_actor_make_req& req) {
        this->appActorMake(req);
    });
    //-----------------------------------------------------------------------------
    m_rpc.register_remote([this](app_actor_close_req& req) {
        this->appActorClose(req);
    });
    //-----------------------------------------------------------------------------
    m_rpc.register_remote([this](app_loglevel_req& req) {
        this->appLoglevel(req);
    });
    //-----------------------------------------------------------------------------
    return status_t::null;
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
status_t daemon_t::onHandler(channel_t::data_t& data)
{
    return status_t::unknow;
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void daemon_t::appStartup(app_startup_req& req, app_startup_rep& rep)
{
    rep.appid = req.appid;

    if (!oj_file::dirMake(DIR_DEPLOYMENT)) {
        logError("appActorMake dirMake {} false appid:{}", DIR_DEPLOYMENT, req.appid);
        rep.reason = "dirMake";
        return;
    }

    auto conditions = logFormat("where id in ({},(select parent from app_info where id={}))", req.appid, req.appid);
    auto apps       = adaper_dbQuery<reflect::app_info>(logFormat("where id={} and status = 0", req.appid));
    auto table      = apps.size() > 0 ? &(apps[0]) : nullptr;

    if (nullptr == table) {
        logError("appActorMake dbquery record null appid:{}", req.appid);
        rep.reason = "dbquery";
        return;
    }

    if ("" == table->supervisor || "" == table->loglevel) {
        "" == table->loglevel ? table->loglevel = "info" : "";

        "" == table->supervisor ? table->supervisor = logFormat(SUPERVISOR_CONF, table->name, table->patch, table->patch, table->patch) : "";

        adaper_dbUpdate(apps);
    }

    config_app_json_t config_app;
    config_app.id       = table->id;
    config_app.listen   = table->ip + ":" + table->port;
    config_app.loglevel = table->loglevel;

    if (0 != table->parent) {
        auto parent = adaper_dbQuery<reflect::app_info>(logFormat("where id={} and status = 0", table->parent));

        if (parent.size() <= 0) {
            logError("appActorMake parent record null parent:{}", table->parent);
            rep.reason = "parent record null";
            return;

        } else {
            std::string      config;
            config_connect_t ipport{ parent[0].ip, parent[0].port, 60 };
            codec_.toJson(ipport, config);
            config_app.actors.emplace_back(config_actor_t{ 0, "session_parent", config });
        }
    }

    {
        auto actors = adaper_dbQuery<reflect::actor_info>(logFormat("where appid={} and status=0", table->id));
        for (auto& actor : actors) {
            config_app.actors.emplace_back(config_actor_t{ actor.id, actor.make, actor.config });
        }
    }

    if (!write_config(config_app, "app") ||
        !write_supervisor(table->name, table->supervisor)) {
        logError("appActorMake write_config appid:{}", table->id);
        rep.reason = "write_config";
        return;
    }

    if (!write_supervisor(table->name, table->supervisor)) {
        logError("appActorMake write_supervisor appid:{}, name:{}, conf:{}", table->id, table->name, table->supervisor);
        rep.reason = "write_config";
        return;
    }

    // appid="$1"，local_dir="$2"， sshname="$3"， remote_path="$4"
    std::string cmd = logFormat("{} {} {} {} {} app-{}.conf",
        m_scriptPath,
        table->id,
        "./deployment",
        table->ssh,
        table->patch,
        table->name);

    auto ok = shell__(cmd, [](std::string s) {
        logDebug("shell echo:{}", s);
    });

    if (0 != ok) {
        logError("appActorMake shell__ appid:{}, ok:{}", table->id, ok);
        rep.reason = "shell__";
        return;
    }

    std::string      strJson;
    config_connect_t config_connect{ table->ip, table->port, 60 };
    codec_.toJson(config_connect, strJson);

    auto itold = m_sessionApp.find(table->id);
    auto old   = (m_sessionApp.end() != itold ? itold->second->guid() : 0);

    auto peer = app_t::singletion()->actorMake("session_daemon", strJson, old);
    if (nullptr == peer) {
        logError("appActorMake session_daemon appid:{}", table->id);
        rep.reason = "session_daemon";
        return;
    }

    if (m_sessionApp.end() != itold) {
        itold->second = peer;

    } else {
        m_sessionApp.emplace(table->id, peer);
    }

    logDebug("appActorMake　appid:{}, peer:{}, size:{}", table->id, peer->guid(), m_sessionApp.size());
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void daemon_t::appClose(app_close_req& req, app_close_rep& rep)
{
    rep.appid = req.appid;

    auto itPeer = m_sessionApp.find(req.appid);
    if (m_sessionApp.end() == itPeer) {
        logError("appClose not found appid:{}", req.appid);
        rep.reason = "not found";
        return;
    }

    auto peer = itPeer->second;
    if (nullptr == peer) {
        logError("appClose peer nullptr appid:{}", req.appid);
        rep.reason = "peer";

        m_sessionApp.erase(req.appid);
        return;
    }

    logDebug("daemon_t::appClose appid:{}, peer:{}", req.appid, peer->guid());

    auto status = peer->request(req, [appid = req.appid, peer](app_close_rep& rep) {
        logDebug("appClose routeAsync rep  req.appid:{}, rep.appid:{}, peer:{}, reason:{}, use_count:{}",
            appid,
            rep.appid,
            peer->guid(),
            rep.reason,
            peer.use_count());

        peer->closeSet();
    });

    if (0 != status) {
        logError("appClose request async appid:{}, peer:{}, status:{}", req.appid, peer->guid(), status);
        rep.reason = "request async status";
        peer->closeSet();
    }

    m_sessionApp.erase(req.appid);
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void daemon_t::appActorMake(app_actor_make_req& req)
{
    auto itPeer = m_sessionApp.find(req.appid);
    if (m_sessionApp.end() == itPeer) {
        logError("appActorMake not found appid:{}", req.appid);
        return;
    }

    auto peer = itPeer->second;
    if (nullptr == peer) {
        logError("appActorMake peer nullptr appid:{}", req.appid);
        return;
    }

    auto status = peer->request(req);
    if (0 != status) {
        logError("appActorMake request sync appid:{}, peer:{}, status:{}", req.appid, peer->guid(), status);
    }
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void daemon_t::appActorClose(app_actor_close_req& req)
{
    auto itPeer = m_sessionApp.find(req.appid);
    if (m_sessionApp.end() == itPeer) {
        logError("appActorClose not found appid:{}", req.appid);
        return;
    }

    auto peer = itPeer->second;
    if (nullptr == peer) {
        logError("appActorClose peer nullptr appid:{}", req.appid);
        return;
    }

    auto status = peer->request(req);
    if (0 != status) {
        logError("appActorClose request sync  appid:{}, peer:{}, status:{}", req.appid, peer->guid(), status);
    }
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void daemon_t::appLoglevel(app_loglevel_req& req)
{
    auto itPeer = m_sessionApp.find(req.appid);
    if (m_sessionApp.end() == itPeer) {
        logError("appLoglevel not found appid:{}", req.appid);
        return;
    }

    auto peer = itPeer->second;
    if (nullptr == peer) {
        logError("appLoglevel peer nullptr appid:{}", req.appid);
        return;
    }

    auto status = peer->request(req);
    if (0 != status) {
        logError("appLoglevel request sync  appid:{}, peer:{}, status:{}", req.appid, peer->guid(), status);
    }
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void api_app_startup(app_startup_req& req, app_startup_rep& rep)
{
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void api_app_close(app_close_req&, app_close_rep&)
{
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void api_app_actor_make(app_actor_make_req&)
{
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void api_app_actor_close(app_actor_close_req&)
{
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void api_app_loglevel(app_loglevel_req&)
{
}