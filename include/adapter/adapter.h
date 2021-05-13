#pragma once

#include "api/db.h"
#include "app/app.h"

/***************************************  ***************************************/


/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void tcpConnect(std::string addrport, std::string mark = "", int64_t guid = 0);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void consoleCbSet(std::string cmd, std::function<void(std::vector<std::string>&)>&& cb, std::string note = "");

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void adaper_pubsub_subscribe(std::string key, actor_sptr_t actor);
void adaper_pubsub_publish(std::string key, channel_t::data_t data);


/*---------------------------------------------------------------------------------
db 表创建
---------------------------------------------------------------------------------*/
template <typename t_table>
void adaper_dbCreate(std::string key_auto = "", std::string key_unique = "", std::vector<std::string> notnull = {})
{
    static_assert(iguana::is_reflection_v<t_table>, "no decl type!");

    reflect::db_create_req req;
    req.cmd        = "create";
    req.table_name = std::string(iguana::get_name<t_table>());
    req.notnull    = std::move(notnull);
    req.key_auto   = std::move(key_auto);
    req.key_unique = std::move(key_unique);


    auto& App = app_t::singletion();

    reflect::db_create_rep rep;
    App->call(api_db_create, req, rep);

    if (rep.message != "") {
        logError("adaper_dbCreate req.table_name:{} message:{}", req.table_name, rep.message);
    }
}

/*---------------------------------------------------------------------------------
db 数据查询
---------------------------------------------------------------------------------*/
template <typename t_table>
std::vector<t_table> adaper_dbQuery(std::string conditions = "")
{
    using result_t = std::vector<t_table>;
    static_assert(iguana::is_reflection_v<t_table>, "no decl type!");

    reflect::db_query_req req;
    req.cmd        = "query";
    req.table_name = std::string(iguana::get_name<t_table>());
    req.conditions = conditions;

    auto& App = app_t::singletion();

    reflect::db_query_rep rep;
    App->call(api_db_query, req, rep);

    result_t result;
    if (rep.cmd == req.cmd && rep.message == "") {
        result = codec_.unpack<result_t>(rep.zip);

    } else {
        logError("adaper_dbQuery req.table_name:{} message:{}", req.table_name, rep.message);
    }

    return result;
}

/*---------------------------------------------------------------------------------
db 更新
---------------------------------------------------------------------------------*/
template <typename t_table>
void adaper_dbUpdate(std::vector<t_table>& tables)
{
    static_assert(iguana::is_reflection_v<t_table>, "no decl type!");

    reflect::db_update_req req;
    req.cmd        = "update";
    req.table_name = std::string(iguana::get_name<t_table>());
    req.zip        = codec_.pack(tables);

    auto& App = app_t::singletion();

    reflect::db_update_rep rep;
    App->call(api_db_update, req, rep);

    if (rep.message != "") {
        logError("adaper_dbCreate req.table_name:{} message:{}", req.table_name, rep.message);
    }
}

/*---------------------------------------------------------------------------------
db 删除
---------------------------------------------------------------------------------*/
template <typename t_table>
std::vector<t_table> adaper_dbDelete(std::string conditions = "")
{
}