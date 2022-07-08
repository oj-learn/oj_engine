#pragma once

#include "iguana/iguana/json.hpp"

namespace reflect {


/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct db_config_t {
    std::string db_ip;
    std::string user_name;
    std::string pwd;
    std::string db_name;
    int32_t     timeout;
    int32_t     db_conn_num;
};
REFLECTION(db_config_t, db_ip, user_name, pwd, db_name, timeout, db_conn_num);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct app_info {
    long        id;
    std::string name;
    int32_t     status;
    long        parent;
    std::string ip;
    int32_t     port;
    std::string loglevel;
    std::string ssh;
    std::string patch;
    std::string supervisor;
};
REFLECTION(app_info, id, name, status, parent, ip, port, loglevel, ssh, patch, supervisor)

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
struct actor_info {
    long        id;
    std::string name;
    int32_t     status;
    long        appid;
    std::string make;
    std::string config;
};
REFLECTION(actor_info, id, name, status, appid, make, config)

/*---------------------------------------------------------------------------------
table account
---------------------------------------------------------------------------------*/
struct account {
    long        account_id;
    std::string account;
    long        age;
};
REFLECTION(account, account_id, account, age)


/*---------------------------------------------------------------------------------
table role
---------------------------------------------------------------------------------*/
struct role {
    long        role_id;
    long        account_id;
    std::string name;
};
REFLECTION(role, role_id, account_id, name)


/*---------------------------------------------------------------------------------
db create
---------------------------------------------------------------------------------*/
struct db_create_req {
    std::string              cmd;
    std::string              table_name;
    std::string              key_auto;
    std::string              key_union;
    std::vector<std::string> key_unique;
    std::vector<std::string> key_index;
    std::vector<std::string> notnull;
};
REFLECTION(db_create_req, cmd, table_name, key_auto, key_union, key_unique, key_index, notnull)

struct db_create_rep {
    std::string message;
    std::string cmd;
};
REFLECTION(db_create_rep, message, cmd)

/*---------------------------------------------------------------------------------
db query
---------------------------------------------------------------------------------*/
struct db_query_req {
    std::string cmd;
    std::string table_name;
    std::string conditions;
};
REFLECTION(db_query_req, cmd, table_name, conditions)

struct db_query_rep {
    std::string       message;
    std::string       cmd;
    std::vector<char> zip;
};
REFLECTION(db_query_rep, message, cmd, zip)

/*---------------------------------------------------------------------------------
db update
---------------------------------------------------------------------------------*/
struct db_update_req {
    std::string       cmd;
    std::string       table_name;
    std::vector<char> zip;
};
REFLECTION(db_update_req, cmd, table_name, zip)

struct db_update_rep {
    std::string message;
    std::string cmd;
};
REFLECTION(db_update_rep, message, cmd)


}  // namespace reflect