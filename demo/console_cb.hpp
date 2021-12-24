
#pragma once

#include "actorTest/decl_struct.h"
#include "app/app.h"
#include "base/log.h"
#include "base/time.hpp"
#include <gperftools/profiler.h>
#include "adapter/adapter.h"

namespace test {

using console_args_t = std::vector<std::string>;

/*---------------------------------------------------------------------------------
其它定义
---------------------------------------------------------------------------------*/
auto& App = app_t::singletion();

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
    }
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void dbcreate(console_args_t& args)
{
    adaper_dbCreate<reflect::account>("", "account_id");
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void dbquery(console_args_t& args)
{
    auto results = adaper_dbQuery<reflect::account>();
    for (auto& result : results) {
        logDebug("account_id:{}, account:{}, age:{}", result.account_id, result.account, result.age);
    }
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
        App->call(req);
    }
}

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void testabc(console_args_t& args)
{
    oj_time::elapsed_t ep;
    test_a_req         req;
    App->call(req);

    logDebug("call test_a_req return elapsed:{}(ms)", ep.milli());
}

}  // namespace test
