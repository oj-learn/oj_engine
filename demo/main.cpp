#include "./console_cb.hpp"
#include "actorTest/qps_rpc.hpp"
#include "actorTest/show_rpc.hpp"
#include "actorTest/test_a.hpp"
#include "actorTest/test_b.hpp"
#include "actorTest/test_c.hpp"
#include "app/app.h"
#include "base/log.h"


/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
    if (auto& App = app_t::singletion(); status_t::null == App->open(argc, argv)) {
        //-----------------------------------------------------------------------------
        App->run();

    } else {
        logError("App open status:{}", int(App->status()));
    }
    return 0;
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void app_bind_register()
{
    auto App = app_t::singletion();
    App->actorBind<show_rpc_t>("showrpc");
    App->actorBind<qps_rpc_t>("qpsrpc");
    App->actorBind<test_a_t>("testa");
    App->actorBind<test_b_t>("testb");
    App->actorBind<test_c_t>("testc");
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void app_console_register()
{
    consoleCbSet("actor", test::actor);
    consoleCbSet("close", test::close);
    consoleCbSet("dbquery", test::dbquery);
    consoleCbSet("dbcreate", test::dbcreate);
    consoleCbSet("loglevel", test::loglevel, "(appid)[debug|info|error]，appid:0，设置自身，设定log输出等级");
    consoleCbSet("testabc", test::testabc, "测试 a->b->c的同步调用!");
}