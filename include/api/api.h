#pragma once

#include "decl/decl.h"

/*---------------------------------------------------------------------------------
qps  rpc
---------------------------------------------------------------------------------*/
void qpsRpcSyncCall(qps_rpc_sync_req&);
void qpsRpcSyncCallRep(qps_rpc_async_req&);

/*---------------------------------------------------------------------------------
---------------------------------------------------------------------------------*/
void api_app_startup(app_startup_req&, app_startup_rep&);
void api_app_close(app_close_req&, app_close_rep&);
void api_app_actor_make(app_actor_make_req&);
void api_app_actor_close(app_actor_close_req&);
void api_app_loglevel(app_loglevel_req&);