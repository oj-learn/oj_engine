
#pragma once

#include "adapter/adapter.h"
#include "api/pubsub.h"
#include "app/app.h"
#include "base/log.h"
#include "base/file.h"

namespace test {

using console_args_t = std::vector<std::string>;

/*---------------------------------------------------------------------------------
其它定义
---------------------------------------------------------------------------------*/
auto& App = app_t::singletion();

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void conn(console_args_t& args)
{
    auto n = std::atoll(args[0].c_str());
    while (n-- > 0) {
        tcpConnect("127.0.0.1:48554");
    }
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void close(console_args_t& args)
{
    if (args.size() > 0) {
        auto n = 1;
        if (args.size() >= 2) {
            n = std::atoll(args[1].c_str());
        }

        App->actorFor(args[0], [n](auto a) mutable -> int {
            if (--n < 0) {
                return 1;
            }

            a->closeSet();

            return 0;
        });

    } else {
        App->closeSet();
    }
}


/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void actor(console_args_t& args)
{
    auto a = args.size() > 1 ? App->actorMake(args[0], args[1]) : App->actorMake(args[0]);
    if (nullptr != a) {
        //-----------------------------------------------------------------------------
        {
            static auto cb = [](int64_t r) {
                logDebug("callAsync rep registActor", r);
            };

            //actorMain->rpcGet().callAsync(actorMain->channelNet(), cb, "registActor", App->guid(), a->guid());
        }
    }
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void rpc(console_args_t& args)
{
    auto actor = App->actorGet("test");
    if (nullptr == actor) {
        return;
    }

    {  // rpc call 调用测试　只有１个请求

        actor->rpcGet().register_remote([](pubsub_subscribe_req& req) {
            logDebug("register_remote pubsub_subscribe_req key:{}", req.key);
        });

        pubsub_subscribe_req req{ "keykey" };
        actor->rpcGet().call(api_pubsub_subscribe, req);
        logDebug("rpcGet().call api_pubsub_subscribe sync key:{}", req.key);

        actor->rpcGet().call(api_pubsub_subscribe, req, []() {
            logDebug("rpcGet().call api_pubsub_subscribe async rep ");
        });
    }

    {  // 请求返回
        actor->rpcGet().register_remote([](reflect::db_query_req& req, reflect::db_query_rep& rep) {
            logDebug("register_remote db_query_req cmd:{}, rep.message:{}", req.cmd, rep.message);
            rep.message = "xxx";
        });

        reflect::db_query_rep rep{ "ccc" };
        reflect::db_query_req req{ "aaa" };
        actor->rpcGet().call(api_db_query, req, rep);
        logDebug("rpcGet().call api_db_query sync message:{}", rep.message);

        rep.message = "ooo";
        actor->rpcGet().call(api_pubsub_subscribe, req, [](reflect::db_query_rep& rep) {
            logDebug("rpcGet().call api_db_query async rep message:{}", rep.message);
        });
    }
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void req(console_args_t& args)
{
    auto actor = App->actorGet("test");
    if (nullptr == actor) {
        return;
    }

    {
        //测试只有１个req的情况
        actor->rpcGet().register_local([](ip_api_t& req) {
            logDebug("register_local ip_api_treq hash:{}", req.hash);
        });

        // //同步
        ip_api_t req{ 111 };
        actor->rpcGet().request(actor->channelGet(), req);
        logDebug("rpcGet().request sync 1 req ");

        //异步
        actor->rpcGet().request(actor->channelGet(), req, []() {
            logDebug("rpcGet().request async 1 req ");
        });
    }

    {  //测试１个req,1个rep的情况
        actor->rpcGet().register_local([](ip_api_t& req, ip_api_t& rep) {
            logDebug("register_local rep hash:{}", req.hash);
            rep.hash = 444;
        });

        ip_api_t rep{ 11 };
        ip_api_t req{ 222 };
        actor->rpcGet().request(actor->channelGet(), req, rep);
        logDebug("rpcGet().request sync 2 req hash:{} ", rep.hash);

        //异步
        rep.hash = 333;
        actor->rpcGet().request(actor->channelGet(), req, [](ip_api_t& rep) {
            logDebug("rpcGet().request sync 2 req hash:{} ", rep.hash);
        });
    }
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void rpc2(console_args_t& args)
{
    auto actor = App->actorGet("test");
    if (nullptr == actor) {
        return;
    }

    buffer_t::sptr_t data = buffer_t::create(32, int32_t(head_code_t::echo));
    buffer_t::sptr_t rep  = buffer_t::create(23, -1);

    actor->rpcGet().package(0, 0, data, rep);
    logDebug("package sync rep->type:{}, rep->size:{}, content:{}", rep->type(), rep->size(), rep->data());

    actor->rpcGet().package(0, 0, data, [data](channel_t::data_t rep) {
        logDebug("package async rep->type:{}, rep->size:{}, content:{}", rep->type(), rep->size(), rep->data());
    });
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void rpc3(console_args_t& args)
{
    auto actor = App->actorGet("session_parent");
    if (nullptr == actor) {
        return;
    }

    // interfaceQuery_req req{ 3322 };
    // actor->route(req);
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void dbcreate(console_args_t& args)
{
    adaper_dbCreate<reflect::account>("", "account_id", { "account", "age" });
    adaper_dbCreate<reflect::role>("", "role_id", { "account_id", "name" });
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void dbquery(console_args_t& args)
{
    // auto actor = App->actorGet("test");
    // if (nullptr == actor) {
    //     return;
    // }

    //auto results = adaper_dbQuery<reflect::account>("where account_id=1");
    auto results = adaper_dbQuery<reflect::account>();
    for (auto& result : results) {
        logDebug("result.account_id:{}, result.account:{}, result.age:{}", result.account_id, result.account, result.age);
    }

    auto roles = adaper_dbQuery<reflect::role>();
    for (auto& role : roles) {
        logDebug("role.role_i:{}, role.account_id:{}, role.name:{}", role.role_id, role.account_id, role.name);
    }
}


/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void dbupdate(console_args_t& args)
{
    console_args_t                q       = { { "" } };
    reflect::account              record1 = { 1, "account1", 2 };
    reflect::account              record2 = { 2, "account2", 3 };
    reflect::account              record3 = { 3, "account3", 4 };
    std::vector<reflect::account> records{ record1, record2, record3 };
    adaper_dbUpdate(records);
    dbquery(q);

    reflect::role              role1 = { 1, 1, "role1" };
    reflect::role              role2 = { 2, 2, "role2" };
    std::vector<reflect::role> roles{ role1, role2 };
    adaper_dbUpdate(roles);
    dbquery(q);
}


/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void pubsub(console_args_t& args)
{
    if (args.size() <= 0) {
        return;
    }

    if ("sub" == args[0]) {
        adaper_pubsub_subscribe("pubsub", App);

    } else {
        auto content = args[0];
        auto data    = buffer_t::create(std::move(content), int32_t(head_code_t::echo));
        adaper_pubsub_publish("pubsub", data);
    }
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void daemon1(console_args_t& args)
{
    adaper_dbCreate<reflect::app_info>("", "id", { "ip", "port", "exepatch", "parent" });
    adaper_dbCreate<reflect::actor_info>("", "id", { "appid", "make", "config" });

    {
        auto appDir = oj_file::appDir();

        std::vector<reflect::app_info> records;
        records.emplace_back(reflect::app_info{ 1, "main", 1, 0, "127.0.0.1", "7758", "debug", "localhost", appDir + "main" });
        records.emplace_back(reflect::app_info{ 2, "server", 1, 1, "127.0.0.1", "48554", "debug", "localhost", appDir + "server" });
        records.emplace_back(reflect::app_info{ 3, "client", 1, 2, "127.0.0.1", "48555", "debug", "localhost", appDir + "client" });
        adaper_dbUpdate(records);
    }

    {
        std::vector<reflect::actor_info> records;
        records.emplace_back(reflect::actor_info{ 1000, "actor1000", 0, 1, "test", "" });
        records.emplace_back(reflect::actor_info{ 2000, "actor2000", 0, 2, "test", "" });
        records.emplace_back(reflect::actor_info{ 3000, "actor3000", 0, 3, "test", "" });
        records.emplace_back(reflect::actor_info{ 4000, "actor4000", 0, 4, "test", "" });
        adaper_dbUpdate(records);
    }
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void daemon2(console_args_t& args)
{
    auto actor = App->actorGet("test");
    if (nullptr == actor) {
        return;
    }

    auto req = [actor](int64_t id) {
        app_startup_rep rep;
        app_startup_req req{ id };
        actor->call(api_app_startup, req, rep);

        if (req.appid != rep.appid) {
            logError("api_app_startup rep :{}", rep.reason);
        }
    };

    if (args.size() <= 0) {
        auto apps = adaper_dbQuery<reflect::app_info>("where status=0");
        for (auto& appTable : apps) {
            req(appTable.id);
        }

    } else {
        req(std::atoll(args[0].c_str()));
    }
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void daemon3(console_args_t& args)
{
    auto actor = App->actorGet("test");
    if (nullptr == actor) {
        return;
    }

    auto req = [actor](int64_t id) {
        app_close_rep rep;
        app_close_req req{ id };
        actor->call(api_app_close, req, rep);

        if (req.appid != rep.appid) {
            logError("api_app_close rep :{}", rep.reason);
        }
    };

    if (args.size() <= 0) {
        req(-1);

    } else {
        req(std::atoll(args[0].c_str()));
    }
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void daemon4(console_args_t& args)
{
    auto actor = App->actorGet("test");
    if (nullptr == actor) {
        return;
    }

    if (args.size() <= 0) {
        return;
    }

    std::string make = "test";
    if (args.size() >= 2) {
        make = args[1];
    }

    app_actor_make_req req;
    req.appid        = std::atoll(args[0].c_str());
    req.actor.id     = 0;
    req.actor.make   = make;
    req.actor.config = "";

    actor->call(api_app_actor_make, req);
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void daemon5(console_args_t& args)
{
    auto actor = App->actorGet("test");
    if (nullptr == actor) {
        return;
    }

    if (args.size() <= 0) {
        return;
    }

    int64_t id = 0;
    if (args.size() >= 2) {
        id = std::atoll(args[1].c_str());
    }

    std::string make = "test";
    if (args.size() >= 3) {
        make = args[2];
    }

    app_actor_close_req req;
    req.appid = std::atoll(args[0].c_str());
    req.id    = id;
    req.name  = make;

    actor->call(api_app_actor_close, req);
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void loglevel(console_args_t& args)
{
    if (args.size() <= 0) {
        return;
    }

    std::string level = "trace";
    if (args.size() >= 2) {
        level = args[1];
    }

    app_loglevel_req req;
    req.appid = App->guid();
    req.level = level;

    if (auto appid = std::atoll(args[0].c_str()); 0 == appid) {
        App->request(req);

    } else {
        req.appid = appid;
        App->call(api_app_loglevel, req);
    }
}


}  // namespace test
