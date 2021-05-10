#include "./actorShowRpc.hpp"
#include "./console_cb.hpp"
#include "./qps_rpc.hpp"
#include "./test.h"
#include "app/app.h"
#include "base/log.h"
#include <boost/hana.hpp>
#include <iostream>
#include <thread>


//#pragma comment(lib, "..\\..\\bin\\Debug.x64\\lib_base.lib")

int main(void)
{
    if (auto& app = app_t::singletion(); status_t::null == app.open()) {
        //-----------------------------------------------------------------------------
        //-----------------------------------------------------------------------------
        app.actorBind<actor_showRpc_t>("showrpc");
        app.actorBind<test_t>("test");
        app.actorBind<qps_rpc_t>("qpsrpc");
        //-----------------------------------------------------------------------------
        consoleCbSet("actor", test::actor);
        consoleCbSet("conn", test::conn);
        consoleCbSet("close", test::close);
        //consoleCbSet("gperf", test::gperf);
        consoleCbSet("rpc", test::rpc, "[]，测试 call的注册以及用使用");
        consoleCbSet("req", test::req, "[]，测试request的注册以及用使用");
        consoleCbSet("rpc2", test::rpc2);
        consoleCbSet("rpc3", test::rpc3);
        consoleCbSet("dbquery", test::dbquery);
        consoleCbSet("dbcreate", test::dbcreate);
        consoleCbSet("dbupdate", test::dbupdate);
        consoleCbSet("pubsub", test::pubsub);
        consoleCbSet("daemon1", test::daemon1, "app_info,actor_info，表创建");
        consoleCbSet("daemon2", test::daemon2, "[appid]，不指定则根据app_info启动所有app");
        consoleCbSet("daemon3", test::daemon3, "[appid]，关闭app,-1关闭所有!");
        consoleCbSet("daemon4", test::daemon4, "(appid)，actor make:");
        consoleCbSet("daemon5", test::daemon5, "(appid)[id][name]，actor close 通过id或者make");
        consoleCbSet("loglevel", test::loglevel, "(appid)[debug|info|error]，设定log输出等级");
        //-----------------------------------------------------------------------------
        app.run();
    } else {
        logError("app open status:{}", int32_t(app.status()));
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
