#include "./actorShowRpc.hpp"
#include "./console_cb.hpp"
#include "./qps_rpc.hpp"
#include "./test.h"
#include "./daemon.h"
#include "./pubsub.h"
#include "./session_daemon.h"
#include "app/app.h"
#include "base/log.h"
#include <iostream>
#include <thread>

//#pragma comment(lib, "..\\..\\bin\\Debug.x64\\lib_base.lib")

int main(void)
{
    if (auto& App = app_t::singletion(); status_t::null == App->open()) {
        //-----------------------------------------------------------------------------
        //-----------------------------------------------------------------------------
        App->actorBind<test_t>("test");
        App->actorBind<qps_rpc_t>("qpsrpc");
        App->actorBind<actor_showRpc_t>("showrpc");
        App->actorBind<pubsub_t>("pubsub");
        App->actorBind<daemon_t>("daemon");
        App->actorBind<session_daemon_t>("session_daemon");
        //-----------------------------------------------------------------------------
        consoleCbSet("actor", test::actor);
        consoleCbSet("conn", test::conn);
        consoleCbSet("close", test::close);
        consoleCbSet("rpc", test::rpc, "[]，测试 call的注册以及用使用");
        consoleCbSet("req", test::req, "[]，测试request的注册以及用使用");
        consoleCbSet("rpc2", test::rpc2);
        consoleCbSet("rpc3", test::rpc3);
        consoleCbSet("dbquery", test::dbquery);
        consoleCbSet("dbcreate", test::dbcreate);
        consoleCbSet("dbupdate", test::dbupdate);
        consoleCbSet("pubsub", test::pubsub, "(xxx)，［xxx:sub，订阅频道否则publish发布内容]，测试key:pubsub");
        consoleCbSet("daemon1", test::daemon1, "app_info,actor_info，表创建");
        consoleCbSet("daemon2", test::daemon2, "[appid]，不指定则根据app_info启动所有app");
        consoleCbSet("daemon3", test::daemon3, "[appid]，关闭app,-1关闭所有!");
        consoleCbSet("daemon4", test::daemon4, "(appid)，actor make:");
        consoleCbSet("daemon5", test::daemon5, "(appid)[id][name]，actor close 通过id或者make");
        consoleCbSet("loglevel", test::loglevel, "(appid)[debug|info|error]，appid:0，设置自身，设定log输出等级");
        //-----------------------------------------------------------------------------
        App->run();
    } else {
        logError("App open status:{}", int32_t(App->status()));
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
